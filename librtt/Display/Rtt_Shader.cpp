//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_Shader.h"

#include "Display/Rtt_DisplayObject.h"
#include "Display/Rtt_Paint.h"
#include "Display/Rtt_ShaderData.h"
#include "Display/Rtt_ShaderResource.h"
#include "Renderer/Rtt_RenderData.h"
#include "Renderer/Rtt_Program.h"

#include "Renderer/Rtt_FrameBufferObject.h"
#include "Renderer/Rtt_TextureVolatile.h"
#include "Renderer/Rtt_Renderer.h"
#include "Renderer/Rtt_Matrix_Renderer.h"

#include <string.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

Shader::Shader( Rtt_Allocator *allocator, const SharedPtr< ShaderResource >& resource, ShaderData *data )
:	fResource( resource ),
	fAllocator(allocator),
	fData( data ),
	fCategory( fResource->GetCategory() ),
	fOwner( NULL ),
	fFBO( NULL ),
	fTexture( NULL ),
	fRoot( NULL ),
	fRenderData( NULL ),
	fOutputReady( false ),
	fDirty(false)
{
	Rtt_ASSERT( resource.NotNull() );
	if ( data )
	{
		data->SetOwner( this );
	}
}

//This should not be called externally
Shader::Shader()
:	fResource( ),
	fAllocator(NULL),
	fData( NULL ),
	fCategory(ShaderTypes::kCategoryDefault),
	fOwner( NULL ),
	fFBO( NULL ),
	fTexture( NULL ),
	fRoot( NULL ),
	fRenderData( NULL ),
	fOutputReady( false ),
	fDirty(false)
{

}
Shader::~Shader()
{
	Paint *owner = GetPaint();
	if ( owner )
	{
		DisplayObject *observer = owner->GetObserver();
		if ( fData )
		{
			fData->QueueRelease( observer );
		}
		
		observer->QueueRelease( fTexture );
		observer->QueueRelease( fFBO );
	}
	Rtt_DELETE( fRenderData );
	Rtt_DELETE( fData );
}

Shader *
Shader::Clone( Rtt_Allocator *allocator ) const
{
	ShaderData *data = NULL;
	if ( fData )
	{
		data = fData->Clone( allocator );
	}
	return Rtt_NEW( allocator, Shader(fAllocator, fResource, data ) );
}
void Shader::Log(std::string preprend, bool last)
{
	printf( "Shader::Log(%p)::(Effect:%s)\n", this, fResource->GetName().c_str() );
}
void Shader::Log()
{
	Log("",false);
}
void
Shader::SetTextureBounds( const TextureInfo& textureInfo )
{
	if ( ! fTexture )
	{
		fTexture = Rtt_NEW( fAllocator, TextureVolatile( fAllocator, textureInfo.fWidth, textureInfo.fHeight,
														textureInfo.fFormat, textureInfo.fFilter, textureInfo.fWrap, textureInfo.fWrap ) );
								
		fFBO = Rtt_NEW( fAllocator, FrameBufferObject( fAllocator, fTexture ) );
	}
}
Texture *
Shader::GetTexture() const
{
	return fTexture;
}
void
Shader::UpdateCache( const TextureInfo& textureInfo, const RenderData& objectData )
{
	if ( fOutputReady ) { return; }
	fOutputReady = true;

	if ( ! fRenderData )
	{
		fRenderData = Rtt_NEW( fAllocator, RenderData );
	}
	//Uses default because we're using axis aligned regions in the intermediate Render calls
	fRenderData->fProgram = fResource->GetProgramMod(ShaderResource::kDefault);
}

void
Shader::RenderToTexture( Renderer& renderer, Geometry& cache ) const
{
	//if ( ! fOutputReady ) { return; }
	fOutputReady = false;

	//if ( fDirty )
	if ( fTexture )
	{
		// ????: (optionally???) Copy geometry
		
		// Save current state so we can restore it later
		FrameBufferObject *fbo = renderer.GetFrameBufferObject();
		Rtt::Real viewMatrix[16];
		Rtt::Real projMatrix[16];
		renderer.GetFrustum( viewMatrix, projMatrix );
		
		S32 x, y, width, height;
		renderer.GetViewport( x, y, width, height );
		{
			
			
			//Assumes textures are same w/h format/filter
			Real w = Rtt_IntToReal( fTexture->GetWidth() );
			Real h = Rtt_IntToReal( fTexture->GetHeight() );
			
			Rect bounds;
			bounds.xMin = 0;
			bounds.xMax = w;
			bounds.yMin = 0;
			bounds.yMax = h;
			
			
			
			// Update shader params
			if ( UsesUniforms() )
			{
				// Update uniforms in fRenderData
				UpdatePaint( * fRenderData );
			}
			else
			{
				// Update geometry for per-vertex shader params
				Real ux, uy, uz, uw;
				GetData()->CopyVertexData( ux, uy, uz, uw );
				
				Geometry::Vertex *dstVertices = cache.GetVertexData();
				for ( U32 i = 0, iMax = cache.GetVerticesUsed(); i < iMax; i++ )
				{
					Geometry::Vertex& v = dstVertices[i];
					
					v.ux = ux;
					v.uy = uy;
					v.uz = uz;
					v.uw = uw;
				}
			}
			
			fRenderData->fGeometry = & cache; // Use same geometry
			
			Rtt::Real offscreenViewMatrix[16];
			Rtt::CreateViewMatrix( 0.0f, 0.0f, 0.5f,
								  0.0f, 0.0f, 0.0f,
								  0.0f, 1.0f, 0.0f,
								  offscreenViewMatrix );
			
			
			Rtt::Real offscreenProjMatrix[16];
			Rtt::CreateOrthoMatrix(
								   bounds.xMin, bounds.xMax,
								   bounds.yMin, bounds.yMax,
								   0.0f, 1.0f, offscreenProjMatrix );
			
			
			renderer.SetFrameBufferObject( fFBO );
			renderer.PushMaskCount();
			{
				renderer.SetFrustum( offscreenViewMatrix, offscreenProjMatrix );
				renderer.SetViewport( 0, 0, w, h );
				renderer.Clear( 0.0f, 0.0f, 0.0f, 0.0f );
				
				renderer.Insert( fRenderData );
			}
			renderer.PopMaskCount();
		}

		// Restore state so further rendering is unaffected
		renderer.SetViewport( x, y, width, height );
		renderer.SetFrustum( viewMatrix, projMatrix );
		
		renderer.SetFrameBufferObject( fbo );
		
		//fDirty = true;
	}
}
void
Shader::UpdatePaint( RenderData& data ) const
{
	data.fUserUniform0 = fData->GetUniform( ShaderData::kData0 );
	data.fUserUniform1 = fData->GetUniform( ShaderData::kData1 );
	data.fUserUniform2 = fData->GetUniform( ShaderData::kData2 );
	data.fUserUniform3 = fData->GetUniform( ShaderData::kData3 );
}

void
Shader::Prepare( RenderData& objectData, int w, int h, ShaderResource::ProgramMod mod )
{
	Program *program = fResource->GetProgramMod(mod);
	
	objectData.fProgram = program;
}

void
Shader::Draw( Renderer& renderer, const RenderData& objectData ) const
{
	// No-op
	renderer.TallyTimeDependency( fResource->UsesTime() );
	renderer.Insert( & objectData );
}

void
Shader::PushProxy( lua_State *L ) const
{
	fData->PushProxy( L );
}

void
Shader::DetachProxy()
{
	fData->DetachProxy();
	fData = NULL;
}

Paint *
Shader::GetPaint() const
{
	Paint *result = fOwner; // Outermost node should have something here...
	if ( ! result )
	{
		// Recurse up hierarchies to get the paint object to invalidate
		if ( fRoot )
		{
			result = fRoot->GetPaint();
		}
	}
	return result;
}

bool
Shader::UsesUniforms() const
{
	return fResource->UsesUniforms();
}
bool
Shader::IsTerminal(Shader *shader) const
{
	return (this==shader);
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

