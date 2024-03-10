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

#include "Display/Rtt_Paint.h"
#include "Display/Rtt_DisplayObject.h"
#include "Renderer/Rtt_RenderData.h"
#include "Renderer/Rtt_Program.h"
#include "Display/Rtt_ShaderComposite.h"
#include "Rtt_LuaUserdataProxy.h"
#include "Renderer/Rtt_FrameBufferObject.h"
#include "Renderer/Rtt_Renderer.h"
#include "Renderer/Rtt_Matrix_Renderer.h"

#include "Renderer/Rtt_TextureBitmap.h"
#include "Display/Rtt_ShaderCompositeAdapter.h"
#include "Display/Rtt_ShaderResource.h"
#include "Display/Rtt_ShaderData.h"

#include <string.h>

#include "Display/Rtt_ShaderInput.h"
#include "Display/Rtt_ShaderProxy.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

ShaderComposite::ShaderComposite(
	Rtt_Allocator *allocator, const SharedPtr< ShaderResource >& resource, ShaderData *data )
:	Super( allocator, resource, data ),
	fProxy(NULL),
	fGeometry(NULL),
	fInitializedAsComposite(false)
{

}

ShaderComposite::~ShaderComposite()
{
	Rtt_DELETE( fGeometry ); // Renderer should generate its own copy, so can delete right away

	fMap.clear();
	
	if ( fProxy )
	{
		Paint *owner = GetPaint();
		if ( owner )
		{
			owner->GetObserver()->QueueRelease( fProxy ); // Release native ref to Lua-side proxy
		}
		fProxy->DetachUserdata(); // Notify proxy that object is invalid
	}
}
void ShaderComposite::Log(std::string preprend, bool last)
{
	printf( "%sShaderComposite::Log(%p)::(Effect:%s)\n", preprend.c_str(), this, fResource->GetName().c_str() );

//	if (this->fInput0.NotNull())
//	{
//		printf( "\t|Input0 ");
//		this->fInput0->Log(preprend+"\t ");
//	}
//	if (this->fInput1.NotNull())
//	{
//		printf( "\t|Input1 ");
//		this->fInput1->Log(preprend+"\t ");
//	}

	printf( "%s",preprend.c_str());
	if (last)
	{
		printf( "\\-");
		preprend += "  ";
	}
	else
	{
		printf( "|-");
		preprend += "| ";
	}
	printf( "[%s %p] map_size:%d\n", fResource->GetName().c_str(), this, (int)fMap.size() );
	
	if (this->fInput0.NotNull())
	{
		this->fInput0->Log(preprend,this->fInput0->HasChildren());
		
	}
	if (this->fInput1.NotNull())
	{
		this->fInput1->Log(preprend,this->fInput1->HasChildren());
	}
}

Shader *
ShaderComposite::Clone( Rtt_Allocator *allocator) const
{
	ShaderComposite *result = Rtt_NEW( allocator, ShaderComposite( allocator, fResource, fData->Clone( allocator ) ) );
	return result;
}

void ShaderComposite::SetInput(int inputChannel, SharedPtr<Shader> shader)
{
	if (fInitializedAsComposite == false)
	{
		if (inputChannel == 0)
		{
			fInput0 = shader;
		}
		else
		{
			fInput1 = shader;
		}
	}
	else //Talking to this node as a composite
	{
//		//Now talk to this like a composite node

		ShaderProxy *proxy = NULL;
		if (inputChannel == 0)
		{
			proxy = (ShaderProxy*)GetNamedShader("paint1");
		}
		else if (inputChannel == 1)
		{
			proxy = (ShaderProxy*)GetNamedShader("paint2");
		}
		if (proxy)
		{
			proxy->SetInput(inputChannel,shader);
		}
		


//		ShaderProxy *proxyP1 = (ShaderProxy*)GetNamedShader("paint1");
//		ShaderProxy *proxyP2 = (ShaderProxy*)GetNamedShader("paint2");
//		if (proxyP1)
//		{
//			proxyP1->SetInput(inputChannel,shader);
//		}
//		
//		if (proxyP2)
//		{
//			proxyP2->SetInput(inputChannel,shader);
//		}

		
	}
}
// protected (not publicly called)
void
ShaderComposite::UpdateCache( const TextureInfo& textureInfo, const RenderData& objectData )
{
	Super::UpdateCache(textureInfo, objectData );

	// Recursively UpdateCache
	if ( fInput0.NotNull() )
	{
		fInput0->UpdateCache(textureInfo, objectData );
	}

	if ( fInput1.NotNull() )
	{
		fInput1->UpdateCache(textureInfo, objectData );
	}

	if ( fInput0.NotNull() )
	{
		fInput0->SetTextureBounds( textureInfo );
		fRenderData->fFillTexture0 = fInput0->GetTexture();
	}

	if ( fInput1.NotNull() )
	{
		fInput1->SetTextureBounds( textureInfo );
		fRenderData->fFillTexture1 = fInput1->GetTexture();
	}
}

Texture *
ShaderComposite::GetTexture() const
{
	return Super::GetTexture();
}

void
ShaderComposite::RenderToTexture(Renderer& renderer, Geometry& cache ) const
{
	if ( fInput0.NotNull())
	{
		fInput0->RenderToTexture( renderer, cache );
	}

	if ( fInput1.NotNull())
	{
		fInput1->RenderToTexture( renderer, cache );
	}

	Super::RenderToTexture( renderer, cache );
}

void
ShaderComposite::UpdateGeometry(const TextureInfo& textureInfo)
{
	if ( ! fGeometry )
	{
		fGeometry = Rtt_NEW( fAllocator, Geometry( fAllocator, Geometry::kTriangleStrip, 4, 0, false ) );
	}

	fGeometry->SetVerticesUsed( 0 );

	// Update geometry with 4 vertices for a quad
	int w = textureInfo.fWidth;
	int h = textureInfo.fHeight;

	Geometry::Vertex v;
	v.Zero();

	// Same for all vertices
	v.z = 0.f;
	v.q = 1.f;
	v.rs = 0xFF;
	v.gs = 0xFF;
	v.bs = 0xFF;
	v.as = 0xFF;
	//				v.ux = ux;
	//				v.uy = uy;
	//				v.uz = uz;
	//				v.uw = uw;
	
	v.x = 0.f; v.y = 0.f;
	v.u = 0.f; v.v = 0.f;
	fGeometry->Append( v );
	
	v.x = 0.f; v.y = h;
	v.u = 0.f; v.v = 1.f;
	fGeometry->Append( v );
	
	v.x = w; v.y = 0.f;
	v.u = 1.f; v.v = 0.f;
	fGeometry->Append( v );
	
	v.x = w; v.y = h;
	v.u = 1.f; v.v = 1.f;
	fGeometry->Append( v );

}

// public: ShapeObject calls this.
void
ShaderComposite::Prepare( RenderData& objectData, int w, int h, ShaderResource::ProgramMod mod )
{
	Texture *objectTexture = objectData.fFillTexture0;

	bool hasInput0 = fInput0.NotNull();
	bool hasInput1 = fInput1.NotNull();

	if ( hasInput0 || hasInput1 )
	{
		TextureInfo textureInfo;
		
		textureInfo.fWidth = objectTexture->GetWidth();
		textureInfo.fHeight = objectTexture->GetHeight();
		textureInfo.fFormat = objectTexture->GetFormat();
		textureInfo.fFilter = objectTexture->GetFilter();
		textureInfo.fWrap = Texture::kClampToEdge;

		// To avoid cache w,h, just pass it in here.
		UpdateGeometry(textureInfo); // Optional for single node case

		// Pass struct into UpdateCache
		UpdateCache(textureInfo, objectData ); // Optional for single node case
		
	}

	Super::Prepare( objectData, w, h, mod );

	// To avoid a race condition, assign texture at very end
	// b/c ShaderInput nodes will grab fFillTexture0 to get input textures

	if ( hasInput0 )
	{
		objectData.fFillTexture0 = fInput0->GetTexture();
	}
	
	if ( hasInput1 )
	{
		objectData.fFillTexture1 = fInput1->GetTexture();
	}

}

// public: ShapeObject calls this to get final output
void
ShaderComposite::Draw( Renderer& renderer, const RenderData& objectData, const GeometryWriter* writers, U32 n ) const
{
    if ( !renderer.CanAddGeometryWriters() ) // ignore during raw draws
    {
        renderer.SetGeometryWriters( writers, n );
    }

    DrawState state( fResource->GetEffectCallbacks(), fIsDrawing );

    if (DoAnyBeforeDrawAndThenOriginal( state, renderer, objectData ))
    {
        // Create geometry for a quad (based on texture bounds of fFillTexture0/fFillTexture1)
        Geometry& cache = GetGeometry();

        RenderToTexture( renderer, cache );
        
        renderer.TallyTimeDependency( fResource->UsesTime() );
        renderer.Insert( & objectData, GetData() );
    }

    DoAnyAfterDraw( state, renderer, objectData );
}	

void
ShaderComposite::SetNamedShader(std::string key, Shader* shader)
{
	fMap[key] = shader;
}
Shader *ShaderComposite::GetNamedShader(std::string key) const
{
	std::map<std::string,Shader*>::const_iterator i = fMap.find(key);
	
	if ( i != fMap.end() )
		return i->second;
		
	return NULL;
}
bool
ShaderComposite::IsTerminal(Shader *shader) const
{
	return (fMap.size() != 0 && this == shader);
}
void
ShaderComposite::PushProxy( lua_State *L ) const
{
	if ( fMap.size() == 0 )
	{
		Super::PushProxy( L );
	}
	else
	{
		if ( ! fProxy )
		{
			fProxy = LuaUserdataProxy::New( L, const_cast< Self * >( this ) );
			fProxy->SetAdapter( & ShaderCompositeAdapter::Constant() );
		}

		fProxy->Push( L );
	}
}

void
ShaderComposite::DetachProxy()
{
	Super::DetachProxy();

	fProxy = NULL;
}
// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

