//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_ParticleSystemObject.h"
#include "Rtt_PhysicsWorld.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_Runtime.h"

#include "Core/Rtt_AutoPtr.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_Shader.h"
#include "Display/Rtt_ShaderData.h"
#include "Display/Rtt_ShaderFactory.h"
#include "Display/Rtt_Scene.h"
#include "Display/Rtt_TextureFactory.h"
#include "Display/Rtt_TextureResource.h"
#include "Display/Rtt_LuaLibDisplay.h"

#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Renderer/Rtt_Renderer.h"

#include "Rtt_LuaUserdataProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "b2Separator.h"

#include "Box2D/Box2D.h"
#include "Rtt_LuaContainer.h"
#include <algorithm>
#include <list>

# define ENABLE_DEBUG_PRINT	( 0 )
#
# if ENABLE_DEBUG_PRINT
#
#	define DEBUG_PRINT( ... )	Rtt_Log( __VA_ARGS__ )
#
# else // Not ENABLE_DEBUG_PRINT
#
#	define DEBUG_PRINT( ... )
#
# endif // ENABLE_DEBUG_PRINT

// -----------------------------------------------------------------------------

#if 0
	#define TRACE_CALL \
	{ \
		static int TRACE_CALL ## __LINE__ = 0; ++TRACE_CALL ## __LINE__; printf( "%s %d\n", __FUNCTION__, TRACE_CALL ## __LINE__ ); \
	}
#else
	#define TRACE_CALL
#endif

// -----------------------------------------------------------------------------

#define VERTICES_PER_QUADS	( 6 )

#define MIN_LIFETIME_GRANULARITY	( 0.01f )

// -----------------------------------------------------------------------------

namespace Rtt
{

// -----------------------------------------------------------------------------

ParticleSystemObject::ParticleSystemObject()
: Super()
, MUpdatable()
, fShouldUpdate( false )
, fWorldScaleInPixelsPerMeter( 1.0f )
, fWorldScaleInMetersPerPixel( 1.0f )
, fRenderRadiusInPixels( 1.0f, 1.0f )
, fWorld( NULL )
, fPhysics( NULL )
, fParticleSystem( NULL )
, fTextureResource()
, fData()
, fShader( NULL )
{
TRACE_CALL;

    SetObjectDesc("ParticleSystemObject"); // for introspection
}

ParticleSystemObject::~ParticleSystemObject()
{
TRACE_CALL;
	_Cleanup();
}

void ParticleSystemObject::_Cleanup()
{
TRACE_CALL;
	StageObject *stage = GetStage();
	if( stage )
	{
		stage->GetScene().RemoveActiveUpdatable( this );
	}

	if( fParticleSystem && fWorld )
	{
		fWorld->DestroyParticleSystem( fParticleSystem );
		fParticleSystem = NULL;
	}

	if( fData.fGeometry )
	{
		QueueRelease( fData.fGeometry );

		fData.fGeometry = NULL;
	}
}

void ParticleSystemObject::_InitializeFromLua_ParticleSystemDef( lua_State *L,
																	b2ParticleSystemDef &particleSystemDef )
{
TRACE_CALL;

	lua_getfield( L, -1, "strictContactCheck" );
	if( lua_isboolean( L, -1 ) )
	{
		particleSystemDef.strictContactCheck = lua_toboolean( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "density" );
	if( lua_isnumber( L, -1 ) )
	{
		particleSystemDef.density = lua_tonumber( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "gravityScale" );
	if( lua_isnumber( L, -1 ) )
	{
		particleSystemDef.gravityScale = lua_tonumber( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "radius" );
	if( lua_isnumber( L, -1 ) )
	{
		particleSystemDef.radius = ( lua_tonumber( L, -1 ) * fWorldScaleInMetersPerPixel );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "maxCount" );
	if( lua_isnumber( L, -1 ) )
	{
		particleSystemDef.maxCount = lua_tonumber( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "pressureStrength" );
	if( lua_isnumber( L, -1 ) )
	{
		particleSystemDef.pressureStrength = lua_tonumber( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "dampingStrength" );
	if( lua_isnumber( L, -1 ) )
	{
		particleSystemDef.dampingStrength = lua_tonumber( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "elasticStrength" );
	if( lua_isnumber( L, -1 ) )
	{
		particleSystemDef.elasticStrength = lua_tonumber( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "springStrength" );
	if( lua_isnumber( L, -1 ) )
	{
		particleSystemDef.springStrength = lua_tonumber( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "viscousStrength" );
	if( lua_isnumber( L, -1 ) )
	{
		particleSystemDef.viscousStrength = lua_tonumber( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "surfaceTensionPressureStrength" );
	if( lua_isnumber( L, -1 ) )
	{
		particleSystemDef.surfaceTensionPressureStrength = lua_tonumber( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "surfaceTensionNormalStrength" );
	if( lua_isnumber( L, -1 ) )
	{
		particleSystemDef.surfaceTensionNormalStrength = lua_tonumber( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "repulsiveStrength" );
	if( lua_isnumber( L, -1 ) )
	{
		particleSystemDef.repulsiveStrength = lua_tonumber( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "powderStrength" );
	if( lua_isnumber( L, -1 ) )
	{
		particleSystemDef.powderStrength = lua_tonumber( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "ejectionStrength" );
	if( lua_isnumber( L, -1 ) )
	{
		particleSystemDef.ejectionStrength = lua_tonumber( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "staticPressureStrength" );
	if( lua_isnumber( L, -1 ) )
	{
		particleSystemDef.staticPressureStrength = lua_tonumber( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "staticPressureRelaxation" );
	if( lua_isnumber( L, -1 ) )
	{
		particleSystemDef.staticPressureRelaxation = lua_tonumber( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "staticPressureIterations" );
	if( lua_isnumber( L, -1 ) )
	{
		particleSystemDef.staticPressureIterations = (int)lua_tointeger( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "colorMixingStrength" );
	if( lua_isnumber( L, -1 ) )
	{
		particleSystemDef.colorMixingStrength = lua_tonumber( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "destroyByAge" );
	if( lua_isboolean( L, -1 ) )
	{
		particleSystemDef.destroyByAge = lua_toboolean( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "lifetimeGranularity" );
	if( lua_isnumber( L, -1 ) )
	{
		particleSystemDef.lifetimeGranularity = lua_tonumber( L, -1 );
		if( particleSystemDef.lifetimeGranularity <= 0.0f )
		{
			particleSystemDef.lifetimeGranularity = MIN_LIFETIME_GRANULARITY;
			Rtt_LogException( "Invalid lifetimeGranularity (this value MUST be non-zero). We'll set this to 0.01 for now." );
		}
	}
	lua_pop( L, 1 );

	// Blend equation.
	lua_getfield( L, -1, "blendEquation" );
	if( lua_isstring( L, -1 ) )
	{
		fData.fBlendEquation = RenderTypes::BlendEquationForString( lua_tostring( L, -1 ) );
	}
	lua_pop( L, 1 );

	// Blend mode.
	lua_getfield( L, -1, "blendMode" );
	if( lua_isstring( L, -1 ) )
	{
		RenderTypes::BlendType value = RenderTypes::BlendTypeForString( lua_tostring( L, -1 ) );

		bool isPremultiplied = false;
		fData.fBlendMode = BlendMode( value, isPremultiplied );
		/*
		//??? RenderTypes::BlendTypeForBlendMode( blend, IsPremultiplied() );
		//??? data.fBlendEquation = (RenderTypes::BlendEquation)fBlendEquation;
		*/
	}
	else if( lua_istable( L, -1 ) )
	{
		const char *tmpStr;

		// Required values
		lua_getfield( L, -1, "srcColor" );
		tmpStr = lua_tostring( L, -1 );
		fData.fBlendMode.fSrcColor = BlendMode::ParamForString( tmpStr );
		lua_pop( L, 1 );

		lua_getfield( L, -1, "dstColor" );
		tmpStr = lua_tostring( L, -1 );
		fData.fBlendMode.fDstColor = BlendMode::ParamForString( tmpStr );
		lua_pop( L, 1 );

		// Optional values
		lua_getfield( L, -1, "srcAlpha" );
		tmpStr = lua_tostring( L, -1 );
		BlendMode::Param srcAlpha = BlendMode::ParamForString( tmpStr );
		if( srcAlpha != BlendMode::kUnknown )
		{
			fData.fBlendMode.fSrcAlpha = srcAlpha;
		}
		lua_pop( L, 1 );

		lua_getfield( L, -1, "dstAlpha" );
		tmpStr = lua_tostring( L, -1 );
		BlendMode::Param dstAlpha = BlendMode::ParamForString( tmpStr );
		if( dstAlpha != BlendMode::kUnknown )
		{
			fData.fBlendMode.fDstAlpha = dstAlpha;
		}
		lua_pop( L, 1 );
	}
	else
	{
		// Nothing to do.
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "filter" );
	if( lua_istable( L, -1 ) )
	{
		b2Filter& filter = particleSystemDef.filter;

		lua_getfield( L, -1, "categoryBits" );
		if ( ! lua_isnil( L, -1 ) )
		{
			uint16 categoryBits = (uint16)lua_tonumber( L, -1 );
			filter.categoryBits = categoryBits;
		}
		lua_pop( L, 1 );

		lua_getfield( L, -1, "maskBits" );
		if ( ! lua_isnil( L, -1 ) )
		{
			uint16 maskBits = (uint16)lua_tonumber( L, -1 );
			filter.maskBits = maskBits;
		}
		lua_pop( L, 1 );

		lua_getfield( L, -1, "groupIndex" );
		if ( ! lua_isnil( L, -1 ) )
		{
			int16 groupIndex = (int16)lua_tonumber( L, -1 );
			filter.groupIndex = groupIndex;
		}
		lua_pop( L, 1 );
	}
	lua_pop( L, 1 );
}

bool ParticleSystemObject::Initialize( lua_State *L,
										Display &display )
{
TRACE_CALL;
	Rtt_ASSERT( ! fParticleSystem );

	fPhysics = &( LuaContext::GetRuntime( L )->GetPhysicsWorld() );
	fWorld = fPhysics->GetWorld();
	if( ! fWorld )
	{
		_Cleanup();
		luaL_error( L,
					"No physics world present." );
		return false;
	}

	fWorldScaleInPixelsPerMeter = fPhysics->GetPixelsPerMeter();
	fWorldScaleInMetersPerPixel = fPhysics->GetMetersPerPixel();

	// Default values for options.
	const char* textureFileName = NULL;
	MPlatform::Directory baseDir = MPlatform::kResourceDir;

	b2ParticleSystemDef particleSystemDef;
	if( lua_istable( L, 1 ) )
	{
		// It's a table of options.

		_InitializeFromLua_ParticleSystemDef( L, particleSystemDef );

		// filename is required.
		lua_getfield( L, -1, "filename" );
		textureFileName = luaL_checkstring( L, -1 );
		if( ! Rtt_VERIFY( textureFileName ) )
		{
			// Nothing to do.
			lua_pop( L, 1 );
			_Cleanup();
			luaL_error( L,
						"\"filename\" is missing." );
			return false;
		}
		lua_pop( L, 1 );

		lua_getfield( L, -1, "baseDir" );
		baseDir = LuaLibSystem::ToDirectory( L, -1, MPlatform::kResourceDir );
		lua_pop( L, 1 );

		lua_getfield( L, -1, "imageRadius" );
		if( lua_isnumber( L, -1 ) )
		{
			fRenderRadiusInPixels.x = lua_tonumber( L, -1 );
			fRenderRadiusInPixels.y = fRenderRadiusInPixels.x;
		}
		else
		{
			// Default to the same value set for fParticleSystem.
			fRenderRadiusInPixels.x = ( particleSystemDef.radius * fWorldScaleInPixelsPerMeter );
			fRenderRadiusInPixels.y = fRenderRadiusInPixels.x;
		}
		lua_pop( L, 1 );
	}

	fParticleSystem = fWorld->CreateParticleSystem( &particleSystemDef );

	// Texture
	{
		fTextureResource = display.GetTextureFactory().FindOrCreate( textureFileName,
																		baseDir,
																		( PlatformBitmap::kIsNearestAvailablePixelDensity |
																		PlatformBitmap::kIsBitsFullResolution ), //!< Image sheets should be loaded at full resolution AND support dynamic image resolution
																		false );
		if( fTextureResource.IsNull() )
		{
			_Cleanup();
			luaL_error( L,
						"CAN'T find texture name: %s",
								textureFileName );
			return false;
		}
	}

	// Init fData.
	{
		fData.fGeometry = Rtt_NEW( display.GetAllocator(),
									Geometry( display.GetAllocator(),
												Geometry::kTriangleFan,
												0, // Vertex count.
												0, // Index count.
												false ) ); // Store on GPU.

		ShaderFactory &factory = display.GetShaderFactory();
		fShader = &factory.GetDefault();

		fData.fFillTexture0 = NULL;
		fData.fFillTexture1 = NULL;
		fData.fMaskTexture = NULL;
		fData.fMaskUniform = NULL;

		fData.fUserUniform0 = NULL;
		fData.fUserUniform1 = NULL;
		fData.fUserUniform2 = NULL;
		fData.fUserUniform3 = NULL;

		_ResizeRenderBuffers();
	}

	fData.fGeometry->SetPrimitiveType( Geometry::kTriangles );

	// Shader.
	{
		fData.fFillTexture0 = &fTextureResource->GetTexture();

		ShaderData *d = ( fShader ? fShader->GetData() : NULL );
		if( d )
		{
			fData.fUserUniform0 = d->GetUniform( ShaderData::kData0 );
			fData.fUserUniform1 = d->GetUniform( ShaderData::kData1 );
			fData.fUserUniform2 = d->GetUniform( ShaderData::kData2 );
			fData.fUserUniform3 = d->GetUniform( ShaderData::kData3 );
		}
		else
		{
			fData.fUserUniform0 = NULL;
			fData.fUserUniform1 = NULL;
			fData.fUserUniform2 = NULL;
			fData.fUserUniform3 = NULL;
		}
	}

	display.GetScene().AddActiveUpdatable( this );

	return true;
}

void ParticleSystemObject::Draw( Renderer& renderer ) const
{
TRACE_CALL;
	Rtt_ASSERT( fParticleSystem );

	if( ! ShouldDraw() )
	{
		return;
	}

#if defined(Rtt_EMSCRIPTEN_ENV)
	if (fData.fGeometry->GetStoredOnGPU())
	{
		fData.fGeometry->Invalidate();
	}
#endif

	// This does the renderer.Insert( &fData ).
	fShader->Draw( renderer, fData );
}

void ParticleSystemObject::GetSelfBounds( Rect& rect ) const
{
TRACE_CALL;
	Rtt_ASSERT( fParticleSystem );

	rect.xMin = -0.5f;
	rect.yMin = -0.5f;
	rect.xMax = 0.5f;
	rect.yMax = 0.5f;
}

bool ParticleSystemObject::HitTest( Real contentX, Real contentY )
{
TRACE_CALL;
	Rtt_ASSERT( fParticleSystem );

	return false;
}

ShaderResource::ProgramMod ParticleSystemObject::GetProgramMod() const
{
TRACE_CALL;
	Rtt_ASSERT( fParticleSystem );

	return ShaderResource::kDefault;
}

bool
ParticleSystemObject::CanCull() const
{
TRACE_CALL;
	Rtt_ASSERT( fParticleSystem );

	return false;
}

const LuaProxyVTable &ParticleSystemObject::ProxyVTable() const
{
TRACE_CALL;
	Rtt_ASSERT( fParticleSystem );

	return LuaParticleSystemObjectProxyVTable::Constant();
}

void ParticleSystemObject::Prepare( const Display &display )
{
TRACE_CALL;
	Rtt_ASSERT( fParticleSystem );

	if( ! ShouldHitTest() )
	{
		// Nothing to do.
		return;
	}

	_Update( display );

	Super::Prepare( display );

	fShader->Prepare( fData, 0, 0, ShaderResource::kDefault );

	SetValid( kGeometryFlag |
				kPaintFlag |
				kColorFlag |
				kProgramFlag |
				kProgramDataFlag );
}

void ParticleSystemObject::_CopyParticleFromParticleSystemToVertexData( b2Vec2 &base_position,
																		b2ParticleColor &color,
																		Geometry::Vertex *output_vertices )
{
    // As we are rendering the particles as quads, we need to define 6 vertices for each particle
	// We assume that all particles have a square aspect ratio.

	Geometry::Vertex &output_vert0 = output_vertices[ 0 ];
	Geometry::Vertex &output_vert1 = output_vertices[ 1 ];
	Geometry::Vertex &output_vert2 = output_vertices[ 2 ];
	Geometry::Vertex &output_vert3 = output_vertices[ 3 ];
	Geometry::Vertex &output_vert4 = output_vertices[ 4 ];
	Geometry::Vertex &output_vert5 = output_vertices[ 5 ];

	Vertex2 base_position_in_pixels = { ( base_position.x * fWorldScaleInPixelsPerMeter ),
										( base_position.y * fWorldScaleInPixelsPerMeter ) };

	// Transform from local-space to content-space.
	GetSrcToDstMatrix().Apply( base_position_in_pixels );

    // Using the position of the particle, work out the four vertices for the
	// quad that will hold the particle and load those into the quads array.

	// Meters to pixels.
	Vertex2 min_edge = { ( base_position_in_pixels.x - fRenderRadiusInPixels.x ),
							( base_position_in_pixels.y - fRenderRadiusInPixels.y ) };
	Vertex2 max_edge = { ( base_position_in_pixels.x + fRenderRadiusInPixels.x ),
							( base_position_in_pixels.y + fRenderRadiusInPixels.y ) };

	// Bottom left.
	output_vert0.SetPos( min_edge.x, max_edge.y );

	// Top right.
	output_vert1.SetPos( max_edge.x, min_edge.y );

	// Top left.
	output_vert2.SetPos( min_edge.x, min_edge.y );

	// Bottom left.
	output_vert3.SetPos( min_edge.x, max_edge.y );

	// Bottom right.
	output_vert4.SetPos( max_edge.x, max_edge.y );

	// Top right.
	output_vert5.SetPos( max_edge.x, min_edge.y );

	// Calculate vertex color, modulating by display object's alpha
	RGBA rgba = { color.r, color.g, color.b, color.a };
	rgba.ModulateAlpha( AlphaCumulative() );
	if ( fTextureResource->GetBitmap()->IsPremultiplied() )
	{
		rgba.PremultiplyAlpha();
	}

	// Set the color.
	Geometry::Vertex::SetColor4ub( VERTICES_PER_QUADS, output_vertices,
									rgba.r, rgba.g, rgba.b, rgba.a );
}

void ParticleSystemObject::_CopyAllParticlesFromParticleSystemToVertexData()
{
TRACE_CALL;
	Geometry::Vertex *input_vertices = fData.fGeometry->GetVertexData();
	//ANY TIME THE PARTICLE RADIUS CHANGES, WE NEED TO UPDATE ALL THE RENDERING DATA!!!!!!!

	int32 max_box2d_particleCount = fParticleSystem->GetParticleCount();

	fData.fGeometry->SetVerticesUsed( max_box2d_particleCount * VERTICES_PER_QUADS );

	// Copy vertex data from Box2D to our own renderer format.
	b2ParticleColor *colorBuffer = fParticleSystem->GetColorBuffer();
	b2Vec2 *positionBuffer = fParticleSystem->GetPositionBuffer();
	for( int i = 0;
			i < max_box2d_particleCount;
			++i )
	{
		_CopyParticleFromParticleSystemToVertexData( positionBuffer[ i ],
														colorBuffer[ i ],
														&( input_vertices[ i * VERTICES_PER_QUADS ] ) );
	}
}

void ParticleSystemObject::_ResizeRenderBuffers()
{
TRACE_CALL;
	int max_box2d_particleCount = fParticleSystem->GetInternalAllocatedCapacity();
	int max_render_particles = ( fData.fGeometry->GetVerticesAllocated() / VERTICES_PER_QUADS );

	if( max_box2d_particleCount <= max_render_particles )
	{
		// Nothing to do.
		return;
	}

	max_render_particles = max_box2d_particleCount;

	fData.fGeometry->Resize( ( max_render_particles * VERTICES_PER_QUADS ),
								false );

	// Set every member of Vertex that NEVER change.
	{
		Geometry::Vertex *output_vertices = fData.fGeometry->GetVertexData();

		for( int i = 0;
				i < max_render_particles;
				++i )
		{
			Geometry::Vertex &output_vert0 = output_vertices[ ( i * VERTICES_PER_QUADS ) + 0 ];
			Geometry::Vertex &output_vert1 = output_vertices[ ( i * VERTICES_PER_QUADS ) + 1 ];
			Geometry::Vertex &output_vert2 = output_vertices[ ( i * VERTICES_PER_QUADS ) + 2 ];
			Geometry::Vertex &output_vert3 = output_vertices[ ( i * VERTICES_PER_QUADS ) + 3 ];
			Geometry::Vertex &output_vert4 = output_vertices[ ( i * VERTICES_PER_QUADS ) + 4 ];
			Geometry::Vertex &output_vert5 = output_vertices[ ( i * VERTICES_PER_QUADS ) + 5 ];

			output_vert0.Zero();
			output_vert1.Zero();
			output_vert2.Zero();
			output_vert3.Zero();
			output_vert4.Zero();
			output_vert5.Zero();

			// Bottom left.
			output_vert0.u = 0.0f;
			output_vert0.v = 1.0f;

			// Top right.
			output_vert1.u = 1.0f;
			output_vert1.v = 0.0f;

			// Top left.
			output_vert2.u = 0.0f;
			output_vert2.v = 0.0f;

			// Bottom left.
			output_vert3.u = 0.0f;
			output_vert3.v = 1.0f;

			// Bottom right.
			output_vert4.u = 1.0f;
			output_vert4.v = 1.0f;

			// Top right.
			output_vert5.u = 1.0f;
			output_vert5.v = 0.0f;
	    }
	}
}

void ParticleSystemObject::_Update( const Display &display )
{
TRACE_CALL;
	//// Prevent doing more than one update per frame.
	//
	if( ! fShouldUpdate )
	{
		// Nothing to do.
		return;
	}

	fShouldUpdate = false;
	//
	////

	//SET OR UNSET ANY VALIDITY FLAGS HERE???

	_ResizeRenderBuffers();

	_CopyAllParticlesFromParticleSystemToVertexData();
}

static uint32 _FromStringToParticleFlag( const char *str )
{
	if ( ! str )
	{
		return 0;
	}

	if ( ! strcmp( str, "water" ) )
	{
		return b2_waterParticle;
	}
	else if ( ! strcmp( str, "zombie" ) )
	{
		return b2_zombieParticle;
	}
	else if ( ! strcmp( str, "wall" ) )
	{
		return b2_wallParticle;
	}
	else if ( ! strcmp( str, "spring" ) )
	{
		return b2_springParticle;
	}
	else if ( ! strcmp( str, "elastic" ) )
	{
		return b2_elasticParticle;
	}
	else if ( ! strcmp( str, "viscous" ) )
	{
		return b2_viscousParticle;
	}
	else if ( ! strcmp( str, "powder" ) )
	{
		return b2_powderParticle;
	}
	else if ( ! strcmp( str, "tensile" ) )
	{
		return b2_tensileParticle;
	}
	else if ( ! strcmp( str, "colorMixing" ) )
	{
		return b2_colorMixingParticle;
	}
	else if ( ! strcmp( str, "destructionListener" ) )
	{
		return b2_destructionListenerParticle;
	}
	else if ( ! strcmp( str, "barrier" ) )
	{
		return b2_barrierParticle;
	}
	else if ( ! strcmp( str, "staticPressure" ) )
	{
		return b2_staticPressureParticle;
	}
	else if ( ! strcmp( str, "reactive" ) )
	{
		return b2_reactiveParticle;
	}
	else if ( ! strcmp( str, "repulsive" ) )
	{
		return b2_repulsiveParticle;
	}
	else if ( ! strcmp( str, "fixtureContactListener" ) )
	{
		return b2_fixtureContactListenerParticle;
	}
	else if ( ! strcmp( str, "particleContactListener" ) )
	{
		return b2_particleContactListenerParticle;
	}
	else if ( ! strcmp( str, "fixtureContactFilter" ) )
	{
		return b2_fixtureContactFilterParticle;
	}
	else if ( ! strcmp( str, "particleContactFilter" ) )
	{
		return b2_particleContactFilterParticle;
	}

	return 0;
}

static uint32 _FromStringToParticleGroupFlag( const char *str )
{
	if ( ! str )
	{
		return 0;
	}

	if ( ! strcmp( str, "solid" ) )
	{
		return b2_solidParticleGroup;
	}
	else if ( ! strcmp( str, "rigid" ) )
	{
		return b2_rigidParticleGroup;
	}
	else if ( ! strcmp( str, "canBeEmpty" ) )
	{
		return b2_particleGroupCanBeEmpty;
	}
	else if ( ! strcmp( str, "willBeDestroyed" ) )
	{
		return b2_particleGroupWillBeDestroyed;
	}
	else if ( ! strcmp( str, "needsUpdateDepth" ) )
	{
		return b2_particleGroupNeedsUpdateDepth;
	}
	else if ( ! strcmp( str, "internalMask" ) )
	{
		return b2_particleGroupInternalMask;
	}

	return 0;
}

void ParticleSystemObject::_InitializeFromLua_ParticleDef( lua_State *L,
															b2ParticleDef &particleDef )
{
TRACE_CALL;

	particleDef.userData = this;

	// Flags.
	{
		lua_getfield( L, -1, "flags" );
		int table_index = lua_gettop( L );
		if( lua_istable( L, -1 ) )
		{
			particleDef.flags = 0;

			// 1: Because Lua is 1-based.
			int start = 1;
			int end = (int)lua_objlen( L, table_index );
			for ( ;
					start <= end;
					++start )
			{
				lua_rawgeti( L, table_index, start );

				if( ! lua_isnil( L, -1 ) )
				{
					particleDef.flags = ( particleDef.flags |
											_FromStringToParticleFlag( lua_tostring( L, -1 ) ) );
				}

				lua_pop( L, 1 );
			}
		}
		else if( lua_isstring( L, -1 ) )
		{
			particleDef.flags = _FromStringToParticleFlag( lua_tostring( L, -1 ) );
		}
		lua_pop( L, 1 );
	}

	// Position.
	{
		particleDef.position.SetZero();

		lua_getfield( L, -1, "x" );
		if( lua_isnumber( L, -1 ) )
		{
			particleDef.position.x = lua_tonumber( L, -1 );
		}
		lua_pop( L, 1 );

		lua_getfield( L, -1, "y" );
		if( lua_isnumber( L, -1 ) )
		{
			particleDef.position.y = lua_tonumber( L, -1 );
		}
		lua_pop( L, 1 );

		// Pixels to meters.
		particleDef.position *= fWorldScaleInMetersPerPixel;
	}

	// Velocity.
	{
		particleDef.velocity.SetZero();

		lua_getfield( L, -1, "velocityX" );
		if( lua_isnumber( L, -1 ) )
		{
			particleDef.velocity.x = lua_tonumber( L, -1 );
		}
		lua_pop( L, 1 );

		lua_getfield( L, -1, "velocityY" );
		if( lua_isnumber( L, -1 ) )
		{
			particleDef.velocity.y = lua_tonumber( L, -1 );
		}
		lua_pop( L, 1 );

		// Pixels to meters.
		particleDef.velocity *= fWorldScaleInMetersPerPixel;
	}

	lua_getfield( L, -1, "color" );
	if( lua_istable( L, -1 ) )
	{
		ColorUnion c;
		c.rgba.Clear();
		LuaLibDisplay::ArrayToColor( L, -1, c.pixel, false );

		particleDef.color.r = c.rgba.r;
		particleDef.color.g = c.rgba.g;
		particleDef.color.b = c.rgba.b;
		particleDef.color.a = c.rgba.a;
	}
	else
	{
		particleDef.color.Set( 255,
								255,
								255,
								255 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "lifetime" );
	if( lua_isnumber( L, -1 ) )
	{
		particleDef.lifetime = lua_tonumber( L, -1 );
	}
	lua_pop( L, 1 );
}

void ParticleSystemObject::_InitializeFromLua_ParticleGroupDef( lua_State *L,
																b2ParticleGroupDef &particleGroupDef,
																b2PolygonShape &polygonDef,
																b2CircleShape &circleDef,
																b2PolygonShapePtrVector &polyVec )
{
TRACE_CALL;

	particleGroupDef.userData = this;

	// Flags.
	{
		lua_getfield( L, -1, "flags" );
		int table_index = lua_gettop( L );
		if( lua_istable( L, -1 ) )
		{
			particleGroupDef.flags = 0;

			// 1: Because Lua is 1-based.
			int start = 1;
			int end = (int)lua_objlen( L, table_index );
			for ( ;
					start <= end;
					++start )
			{
				lua_rawgeti( L, table_index, start );

				if( ! lua_isnil( L, -1 ) )
				{
					particleGroupDef.flags = ( particleGroupDef.flags |
												_FromStringToParticleFlag( lua_tostring( L, -1 ) ) );
				}

				lua_pop( L, 1 );
			}
		}
		else if( lua_isstring( L, -1 ) )
		{
			particleGroupDef.flags = _FromStringToParticleFlag( lua_tostring( L, -1 ) );
		}
		lua_pop( L, 1 );
	}

	// Group flags.
	{
		lua_getfield( L, -1, "groupFlags" );
		int table_index = lua_gettop( L );
		if( lua_istable( L, -1 ) )
		{
			particleGroupDef.groupFlags = 0;

			// 1: Because Lua is 1-based.
			int start = 1;
			int end = (int)lua_objlen( L, table_index );
			for ( ;
					start <= end;
					++start )
			{
				lua_rawgeti( L, table_index, start );

				if( ! lua_isnil( L, -1 ) )
				{
					particleGroupDef.groupFlags = ( particleGroupDef.groupFlags |
													_FromStringToParticleGroupFlag( lua_tostring( L, -1 ) ) );
				}

				lua_pop( L, 1 );
			}
		}
		else if( lua_isstring( L, -1 ) )
		{
			particleGroupDef.groupFlags = _FromStringToParticleGroupFlag( lua_tostring( L, -1 ) );
		}
		lua_pop( L, 1 );
	}

	// Position.
	{
		particleGroupDef.position.SetZero();

		lua_getfield( L, -1, "x" );
		if( lua_isnumber( L, -1 ) )
		{
			particleGroupDef.position.x = lua_tonumber( L, -1 );
		}
		lua_pop( L, 1 );

		lua_getfield( L, -1, "y" );
		if( lua_isnumber( L, -1 ) )
		{
			particleGroupDef.position.y = lua_tonumber( L, -1 );
		}
		lua_pop( L, 1 );

		// Pixels to meters.
		particleGroupDef.position *= fWorldScaleInMetersPerPixel;
	}

	lua_getfield( L, -1, "angle" );
	if( lua_isnumber( L, -1 ) )
	{
		particleGroupDef.angle = Rtt_RealDegreesToRadians( lua_tonumber( L, -1 ) );
	}
	lua_pop( L, 1 );

	// Linear velocity.
	{
		particleGroupDef.linearVelocity.SetZero();

		lua_getfield( L, -1, "linearVelocityX" );
		if( lua_isnumber( L, -1 ) )
		{
			particleGroupDef.linearVelocity.x = lua_tonumber( L, -1 );
		}
		lua_pop( L, 1 );

		lua_getfield( L, -1, "linearVelocityY" );
		if( lua_isnumber( L, -1 ) )
		{
			particleGroupDef.linearVelocity.y = lua_tonumber( L, -1 );
		}
		lua_pop( L, 1 );

		// Pixels to meters.
		particleGroupDef.linearVelocity *= fWorldScaleInMetersPerPixel;
	}

	lua_getfield( L, -1, "angularVelocity" );
	if( lua_isnumber( L, -1 ) )
	{
		particleGroupDef.angularVelocity = Rtt_RealDegreesToRadians( lua_tonumber( L, -1 ) );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "color" );
	if( lua_istable( L, -1 ) )
	{
		ColorUnion c;
		c.rgba.Clear();
		LuaLibDisplay::ArrayToColor( L, -1, c.pixel, false );

		particleGroupDef.color.r = c.rgba.r;
		particleGroupDef.color.g = c.rgba.g;
		particleGroupDef.color.b = c.rgba.b;
		particleGroupDef.color.a = c.rgba.a;
	}
	else
	{
		particleGroupDef.color.Set( 255,
									255,
									255,
									255 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "strength" );
	if( lua_isnumber( L, -1 ) )
	{
		particleGroupDef.strength = lua_tonumber( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "stride" );
	if( lua_isnumber( L, -1 ) )
	{
		particleGroupDef.stride = lua_tonumber( L, -1 );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "lifetime" );
	if( lua_isnumber( L, -1 ) )
	{
		particleGroupDef.lifetime = lua_tonumber( L, -1 );
	}
	lua_pop( L, 1 );

	// Box shape.
	if( ! particleGroupDef.shape )
	{
		b2Vec2 half_size( b2Vec2_zero );
		float radians = 0.0f;

		lua_getfield( L, -1, "halfWidth" );
		if( lua_isnumber( L, -1 ) )
		{
			half_size.x = lua_tonumber( L, -1 );
			particleGroupDef.shape = &polygonDef;
		}
		lua_pop( L, 1 );

		lua_getfield( L, -1, "halfHeight" );
		if( lua_isnumber( L, -1 ) )
		{
			half_size.y = lua_tonumber( L, -1 );
			particleGroupDef.shape = &polygonDef;
		}
		lua_pop( L, 1 );

		lua_getfield( L, -1, "angle" );
		if( lua_isnumber( L, -1 ) )
		{
			radians = Rtt_RealDegreesToRadians( lua_tonumber( L, -1 ) );
			particleGroupDef.shape = &polygonDef;
		}
		lua_pop( L, 1 );

		if( particleGroupDef.shape )
		{
			// Pixels to meters.
			half_size *= fWorldScaleInMetersPerPixel;

			polygonDef.SetAsBox( half_size.x,
									half_size.y,
									b2Vec2_zero,
									radians );
		}
	}

	// Circle shape.
	if( ! particleGroupDef.shape )
	{
		lua_getfield( L, -1, "radius" );
		if( lua_isnumber( L, -1 ) )
		{
			// This is a circular shape.

			circleDef.m_radius = lua_tonumber( L, -1 );
			particleGroupDef.shape = &circleDef;
		}
		lua_pop( L, 1 );

		if( particleGroupDef.shape )
		{
			// Pixels to meters.
			circleDef.m_radius *= fWorldScaleInMetersPerPixel;

			// Default to 1/16th of a meter.
			circleDef.m_radius = std::max( circleDef.m_radius,
											Rtt_REAL_16TH );
		}
	}

	// Convex shape.
	if( ! particleGroupDef.shape )
	{
		lua_getfield( L, -1, "shape" );
		if( lua_istable( L, -1 ) )
		{
			// This is an arbitrary polygonal shape.

			// Prevent magic number from getting too high (Box2D max polygons set in Common/b2Settings.h)
			Rtt_STATIC_ASSERT( b2_maxPolygonVertices <= 8 );
			b2Vec2 vertexList[ b2_maxPolygonVertices ];

			int numVertices = std::min( b2_maxPolygonVertices,
										(int)lua_objlen( L, -1 ) >> 1 );
			if( numVertices >= 3 )
			{
				for( int i = 0;
						i < numVertices;
						i++ )
				{
					int n = (i<<1) + 1;

					b2Vec2 &p = vertexList[i];

					// X.
					lua_rawgeti( L, -1, n );
					p.x = luaL_toreal( L, -1 );
					lua_pop( L, 1 );

					// Y.
					lua_rawgeti( L, -1, n + 1 );
					p.y = luaL_toreal( L, -1 );
					lua_pop( L, 1 );

					// Pixels to meters.
					p *= fWorldScaleInMetersPerPixel;
				}

				polygonDef.Set( vertexList, numVertices );

				particleGroupDef.shape = &polygonDef;
			}
		}
		lua_pop( L, 1 );
	}

	// Outline.
	if( ! particleGroupDef.shape )
	{
		lua_getfield( L, -1, "outline" );
		if( lua_istable( L, -1 ) )
		{
			// This is an outline.

			// Get the input vertices.
			b2Vec2Vector shape_outline_in_texels;
			{
				int table_index = lua_gettop( L );
				int numVertices = (int)( lua_objlen( L, table_index ) / 2 );

				for ( int i = 0; i < numVertices; i++ )
				{
					// Get X and Y coordinates.
					{
						// Lua is one-based, so the first element must be at index 1.
						lua_rawgeti( L, table_index, ( ( i * 2 ) + 1 ) );

						// Lua is one-based, so the second element must be at index 2.
						lua_rawgeti( L, table_index, ( ( i * 2 ) + 2 ) );

						shape_outline_in_texels.push_back( b2Vec2( luaL_toreal( L, -2 ),
																	luaL_toreal( L, -1 ) ) );

						lua_pop( L, 2 );
					}

					DEBUG_PRINT( "Outline from Lua: %f, %f\n",
									shape_outline_in_texels.back().x,
									shape_outline_in_texels.back().y );
				}
			}

			//// Tessellate the outline.
			//
			b2Separator sep;

			int error = sep.Validate( shape_outline_in_texels );
			if( error )
			{
				DEBUG_PRINT( "b2Separator::Validate() : %d\nNote: 0 is ok. 1 is overlapping lines. 2 is counter-clockwise points. 3 is both 1 and 2.\n",
								error );
			}
			else
			{
				//// Tessellate.
				//
				b2Vec2VectorVector tessellated_outline;

				sep.calcShapes( shape_outline_in_texels, tessellated_outline );

				DEBUG_PRINT( "*** %s()\nnumber of bodies tesselated: %d\n",
								__FUNCTION__,
								tessellated_outline.size() );
				//
				////

				// Position the outline.
				b2Vec2 translate( shape_outline_in_texels[0] * -1.0f );

				//// Copy the output to polyVec.
				//
				for( size_t i = 0;
						i < tessellated_outline.size();
						++i )
				{
				    b2Vec2Vector &vertices = tessellated_outline[i];

				    size_t m = vertices.size();

					// Verify the vertex count is valid.
					if( m < 3 )
					{
						// Discard.
						DEBUG_PRINT( "skipped shape index: %d\n", i );
						continue;
					}

					if( m > b2_maxPolygonVertices )
					{
						// Clamp the maximum number of vertices to b2_maxPolygonVertices.
						// THIS SHOULD NEVER HAPPEN, BUT IT DOES!!!
						DEBUG_PRINT( "clamp the number of vertices in shape index: %d\n", i );
						m = b2_maxPolygonVertices;
					}

					// Scale and translate all the vertices in tessellated_outline.
				    for( size_t j = 0;
							j < m;
							++j )
					{
						b2Vec2 &v = vertices[j];

						v += translate;
						// Pixels to meters.
						v *= fWorldScaleInMetersPerPixel;

						DEBUG_PRINT( "*** Fixture %03d : Vertex %03d : x, y: %f, %f\n", i, j, v.x, v.y );
				    }

					// This is released in the caller of this function.
					polyVec.push_back( new b2PolygonShape );
				    bool ok = polyVec.back()->Set( &( vertices[ 0 ] ), (int)m );
					if( ! ok )
					{
						// Set() failed. Remove the polygon.
						delete polyVec.back();
						polyVec.pop_back();
					}
				}
				//
				////
			}
			//
			////
		}
		lua_pop( L, 1 );
	}
}

void ParticleSystemObject::CreateParticle( lua_State *L )
{
TRACE_CALL;
	Rtt_ASSERT( fParticleSystem );

	b2ParticleDef particleDef;
	_InitializeFromLua_ParticleDef( L, particleDef );

	fParticleSystem->CreateParticle( particleDef );
}

void ParticleSystemObject::CreateGroup( lua_State *L )
{
TRACE_CALL;
	Rtt_ASSERT( fParticleSystem );

	b2PolygonShape polygonDef;
	b2CircleShape circleDef;

	// This is ONLY used for outlines.
	b2PolygonShapePtrVector polyVec;

	b2ParticleGroupDef particleGroupDef;

	_InitializeFromLua_ParticleGroupDef( L,
											particleGroupDef,
											polygonDef,
											circleDef,
											polyVec );

	if( polyVec.size() )
	{
		// This is an outline.

		particleGroupDef.shapes = (b2Shape **)&polyVec[ 0 ];
		particleGroupDef.shapeCount = (int)polyVec.size();
	}

	fParticleSystem->CreateParticleGroup( particleGroupDef );

	if( polyVec.size() )
	{
		// Clean-up.
		for( b2PolygonShapePtrVectorIter ptr = polyVec.begin();
				ptr != polyVec.end();
				++ptr )
		{
			delete *ptr;
		}
	}
}

void ParticleSystemObject::ApplyForce( lua_State *L )
{
TRACE_CALL;
	Rtt_ASSERT( fParticleSystem );

	b2Vec2 force_in_newtons( lua_tonumber( L, 2 ),
								lua_tonumber( L, 3 ) );

	int32 max_box2d_particleCount = fParticleSystem->GetParticleCount();

	for( int i = 0;
			i < max_box2d_particleCount;
			++i )
	{
		fParticleSystem->ParticleApplyForce( i,
												force_in_newtons );
	}
}

void ParticleSystemObject::ApplyLinearImpulse( lua_State *L )
{
TRACE_CALL;
	Rtt_ASSERT( fParticleSystem );

	b2Vec2 impulse_in_newton_seconds( lua_tonumber( L, 2 ),
										lua_tonumber( L, 3 ) );

	int32 max_box2d_particleCount = fParticleSystem->GetParticleCount();

	for( int i = 0;
			i < max_box2d_particleCount;
			++i )
	{
		fParticleSystem->ParticleApplyLinearImpulse( i,
														impulse_in_newton_seconds );
	}
}

int ParticleSystemObject::DestroyParticlesInShape( lua_State *L )
{
TRACE_CALL;
	Rtt_ASSERT( fParticleSystem );

	b2PolygonShape polygonDef;
	b2CircleShape circleDef;
	b2Shape *shape = NULL;

	// Position.
	b2Vec2 position( b2Vec2_zero );
	{
		lua_getfield( L, -1, "x" );
		if( lua_isnumber( L, -1 ) )
		{
			position.x = lua_tonumber( L, -1 );
		}
		lua_pop( L, 1 );

		lua_getfield( L, -1, "y" );
		if( lua_isnumber( L, -1 ) )
		{
			position.y = lua_tonumber( L, -1 );
		}
		lua_pop( L, 1 );

		// Pixels to meters.
		position *= fWorldScaleInMetersPerPixel;
	}

	// Rotation.
	float radians = 0.0f;
	{
		lua_getfield( L, -1, "angle" );
		if( lua_isnumber( L, -1 ) )
		{
			radians = Rtt_RealDegreesToRadians( lua_tonumber( L, -1 ) );
		}
		lua_pop( L, 1 );
	}

	// Box shape.
	if( ! shape )
	{
		b2Vec2 half_size( b2Vec2_zero );

		lua_getfield( L, -1, "halfWidth" );
		if( lua_isnumber( L, -1 ) )
		{
			half_size.x = lua_tonumber( L, -1 );
			shape = &polygonDef;
		}
		lua_pop( L, 1 );

		lua_getfield( L, -1, "halfHeight" );
		if( lua_isnumber( L, -1 ) )
		{
			half_size.y = lua_tonumber( L, -1 );
			shape = &polygonDef;
		}
		lua_pop( L, 1 );

		if( shape )
		{
			// Pixels to meters.
			half_size *= fWorldScaleInMetersPerPixel;

			polygonDef.SetAsBox( half_size.x,
									half_size.y );
		}
	}

	// Circle shape.
	if( ! shape )
	{
		lua_getfield( L, -1, "radius" );
		if( lua_isnumber( L, -1 ) )
		{
			// This is a circular shape.

			circleDef.m_radius = lua_tonumber( L, -1 );
			shape = &circleDef;
		}
		lua_pop( L, 1 );

		if( shape )
		{
			// Pixels to meters.
			circleDef.m_radius *= fWorldScaleInMetersPerPixel;

			// Default to 1/16th of a meter.
			circleDef.m_radius = std::max( circleDef.m_radius,
											Rtt_REAL_16TH );
		}
	}

	// Convex shape.
	if( ! shape )
	{
		lua_getfield( L, -1, "shape" );
		if( lua_istable( L, -1 ) )
		{
			// This is an arbitrary polygonal shape.

			// Prevent magic number from getting too high (Box2D max polygons set in Common/b2Settings.h)
			Rtt_STATIC_ASSERT( b2_maxPolygonVertices <= 8 );
			b2Vec2 vertexList[ b2_maxPolygonVertices ];

			int numVertices = std::min( b2_maxPolygonVertices,
										(int)lua_objlen( L, -1 ) >> 1 );
			if( numVertices >= 3 )
			{
				for( int i = 0;
						i < numVertices;
						i++ )
				{
					int n = (i<<1) + 1;

					b2Vec2 &p = vertexList[i];

					// X.
					lua_rawgeti( L, -1, n );
					p.x = luaL_toreal( L, -1 );
					lua_pop( L, 1 );

					// Y.
					lua_rawgeti( L, -1, n + 1 );
					p.y = luaL_toreal( L, -1 );
					lua_pop( L, 1 );

					// Pixels to meters.
					p *= fWorldScaleInMetersPerPixel;
				}

				polygonDef.Set( vertexList, numVertices );

				shape = &polygonDef;
			}
		}
		lua_pop( L, 1 );
	}

	////
	//
	int count = 0;

	b2Transform xf;

	if( shape )
	{
		xf.Set( position, radians );

		count = fParticleSystem->DestroyParticlesInShape( *shape, xf );
	}

	lua_pushnumber( L, count );
	//
	////

	return 1;
}

namespace // anonymous namespace.
{
	void add_hit_to_table_of_raycasting_results( lua_State *L,
													float pixels_per_meter,
													size_t &result_index,
													const b2Vec2& point,
													const b2Vec2& normal,
													const float32 fraction )
	{
		if( ! result_index )
		{
			// We're handling the first result.
			lua_newtable( L );
		}

		// Meters to pixels.
		b2Vec2 positionInPixels( point );
		positionInPixels *= pixels_per_meter;

		// Add the result.
		//
		// results[n].x : The X coordinate in content-space of this hit.
		// results[n].y : The Y coordinate in content-space of this hit.
		// results[n].normalX : The X coordinate in content-space of this hit.
		// results[n].normalY : The Y coordinate in content-space of this hit.
		// results[n].fraction : The fraction (0..1), along the ray, where the hit is located.
		{
			// We can use lua_newtable() here, but we know exactly how
			// many records we'll put in the table (5). So we use
			// lua_createtable() for better performance.
			lua_createtable( L, 0, 5 );
			{
				// 1st element: Hit position.
				//
				// -2 : This is used to refer to the table that was
				// created above, using lua_createtable().
				//
				// lua_createtable() pushes a table to the top of the
				// stack (at position "-1"). Then lua_pushnumber()
				// pushes a number to the top of the stack (the new
				// element at position "-1"). Now the table is right
				// below the top of the stack (at position "-2").
				//
				// An alternative is to use lua_gettop() immediately
				// after the creation of the above table. The index
				// returned will refer to the table, and can be passed
				// to every calls of lua_setfield() instead of "-2".
				{
					lua_pushnumber( L, positionInPixels.x );
					lua_setfield( L, -2, "x" );

					lua_pushnumber( L, positionInPixels.y );
					lua_setfield( L, -2, "y" );
				}

				// 2nd element: Hit surface normal.
				//
				// "normal" is a unit vector (vector of length one)
				// without units (just a direction).
				{
					lua_pushnumber( L, normal.x );
					lua_setfield( L, -2, "normalX" );

					lua_pushnumber( L, normal.y );
					lua_setfield( L, -2, "normalY" );
				}

				// 3rd element: Hit fraction along the ray.
				{
					lua_pushnumber( L, fraction );
					lua_setfield( L, -2, "fraction" );
				}

				// Important: If we modify the total number of items
				// pushed on this Lua table, we HAVE to update the
				// number of pre-allocated records in the
				// lua_createtable() call above.
			}

			// Lua is one-based, so the first result must be at index 1.
			++result_index;
			lua_rawseti( L, -2, (int) result_index );
		}
	}

	class AnyHitAlongRay : public b2RayCastCallback
	{
	public:

		AnyHitAlongRay( b2ParticleSystem *particleSystem,
						lua_State *L,
						float pixelsPerMeter )
		: b2RayCastCallback()
		, fParticleSystem( particleSystem )
		, fL( L )
		, fPixelsPerMeter( pixelsPerMeter )
		{
		}

		virtual ~AnyHitAlongRay()
		{
		}

		float32 ReportFixture(	b2Fixture* fixture, const b2Vec2& point,
								const b2Vec2& normal, float32 fraction )
		{
			return 0;
		}

		// fixture the fixture hit by the ray
		// point the point of initial intersection
		// normal the normal vector at the point of intersection
		float32 ReportParticle( const b2ParticleSystem* particleSystem,
								int32 index, const b2Vec2& point,
								const b2Vec2& normal, float32 fraction )
		{
			if( fParticleSystem != particleSystem )
			{
				// 0.0f: Immediately stop the RayCast().
				return 0.0f;
			}

			size_t result_index = 0;
			add_hit_to_table_of_raycasting_results( fL,
													fPixelsPerMeter,
													result_index,
													point,
													normal,
													fraction );

			// We only need one result, regardless of its "fraction",
			// therefore return 0.0f to immediately stop RayCast().
			return 0.0f;
		}

		b2ParticleSystem *fParticleSystem;
		lua_State *fL;
		float fPixelsPerMeter;
	};

	class ClosestHitAlongRay : public b2RayCastCallback
	{
	public:

		ClosestHitAlongRay( b2ParticleSystem *particleSystem,
							lua_State *L,
							float pixelsPerMeter )
		: b2RayCastCallback()
		, fParticleSystem( particleSystem )
		, fL( L )
		, fInitialTopIndexOfLuaStack( lua_gettop( fL ) )
		, fPixelsPerMeter( pixelsPerMeter )
		{
		}

		virtual ~ClosestHitAlongRay()
		{
		}

		float32 ReportFixture( b2Fixture* fixture, const b2Vec2& point,
								const b2Vec2& normal, float32 fraction )
		{
			return 0;
		}

		// fixture the fixture hit by the ray
		// point the point of initial intersection
		// normal the normal vector at the point of intersection
		float32 ReportParticle( const b2ParticleSystem* particleSystem,
								int32 index, const b2Vec2& point,
								const b2Vec2& normal, float32 fraction )
		{
			if( fParticleSystem != particleSystem )
			{
				// Next closest.
				return fraction;
			}

			// We only want to return a single hit in the result table.
			// We reset the top of the Lua stack to always write the result
			// in the same position on the stack.
			lua_settop( fL, fInitialTopIndexOfLuaStack );

			size_t result_index = 0;
			add_hit_to_table_of_raycasting_results( fL,
													fPixelsPerMeter,
													result_index,
													point,
													normal,
													fraction );

			// "fraction" represents the fraction along the ray where the
			// hit is located. By returning "fraction", we clip the next
			// queries to this position. This has the effect of moving the
			// end point closer and closer to the starting point. This also
			// has the effect of giving us the closest possible hit point
			// from the starting point.
			return fraction;
		}

		b2ParticleSystem *fParticleSystem;
		lua_State *fL;
		int fInitialTopIndexOfLuaStack;
		float fPixelsPerMeter;
	};

	class UnsortedHitsAlongRay : public b2RayCastCallback
	{
	public:

		UnsortedHitsAlongRay( b2ParticleSystem *particleSystem,
								lua_State *L,
								float pixelsPerMeter )
		: b2RayCastCallback()
		, fParticleSystem( particleSystem )
		, fL( L )
		, fResultIndex( 0 )
		, fPixelsPerMeter( pixelsPerMeter )
		{
		}

		virtual ~UnsortedHitsAlongRay()
		{
		}

		float32 ReportFixture( b2Fixture* fixture, const b2Vec2& point,
								const b2Vec2& normal, float32 fraction )
		{
			return 0;
		}

		// fixture the fixture hit by the ray
		// point the point of initial intersection
		// normal the normal vector at the point of intersection
		float32 ReportParticle( const b2ParticleSystem* particleSystem,
								int32 index, const b2Vec2& point,
								const b2Vec2& normal, float32 fraction )
		{
			if( fParticleSystem != particleSystem )
			{
				// Continue RayCast() until all possible results are found.
				return 1.0f;
			}

			add_hit_to_table_of_raycasting_results( fL,
													fPixelsPerMeter,
													fResultIndex,
													point,
													normal,
													fraction );

			// Continue RayCast() until all possible results are found.
			return 1.0f;
		}

		b2ParticleSystem *fParticleSystem;
		lua_State *fL;
		size_t fResultIndex;
		float fPixelsPerMeter;
	};

	struct hit
	{
		// This is used to hold a "raw" (unscaled) result as reported by
		// ReportParticle(). We sort them by their "fraction" in PushResultsToLua().
		// We scale them by the pixels_per_meter ratio in add_hit_to_table_of_raycasting_results().

		hit( b2Vec2 point_,
				b2Vec2 normal_,
				float32 fraction_ )
		: point( point_ )
		, normal( normal_ )
		, fraction( fraction_ )
		{
		}

		b2Vec2 point;
		b2Vec2 normal;
		float32 fraction;

		bool operator < ( const hit &other ) const
		{
			return ( fraction < other.fraction );
		}
	};

	typedef std::list< hit > ListHit;
	typedef std::list< hit >::iterator ListHitIter;

	class SortedHitsAlongRay : public b2RayCastCallback
	{
	public:

		SortedHitsAlongRay( b2ParticleSystem *particleSystem,
							lua_State *L,
							float pixelsPerMeter )
		: b2RayCastCallback()
		, fParticleSystem( particleSystem )
		, fL( L )
		, fListHit()
		, fPixelsPerMeter( pixelsPerMeter )
		{
		}

		virtual ~SortedHitsAlongRay()
		{
		}

		float32 ReportFixture( b2Fixture* fixture, const b2Vec2& point,
								const b2Vec2& normal, float32 fraction )
		{
			return 0;
		}

		// fixture the fixture hit by the ray
		// point the point of initial intersection
		// normal the normal vector at the point of intersection
		float32 ReportParticle( const b2ParticleSystem* particleSystem,
								int32 index, const b2Vec2& point,
								const b2Vec2& normal, float32 fraction )
		{
			if( fParticleSystem != particleSystem )
			{
				// Continue RayCast() until all possible results are found.
				return 1.0f;
			}

			// We don't want to push the result to Lua immediately because
			// we want to sort them by distance from the starting point.
			fListHit.push_back( hit( point,
										normal,
										fraction ) );

			// Continue RayCast() until all possible results are found.
			return 1.0f;
		}

		bool PushResultsToLua()
		{
			if( fListHit.empty() )
			{
				// There's no result.
				// Nothing to do.
				return false;
			}

			// Sort the results from closest to farthest.
			fListHit.sort();

			size_t result_index = 0;

			for( ListHitIter iter = fListHit.begin();
					iter != fListHit.end();
					++iter )
			{
				add_hit_to_table_of_raycasting_results( fL,
														fPixelsPerMeter,
														result_index,
														iter->point,
														iter->normal,
														iter->fraction );
			}

			return true;
		}

		b2ParticleSystem *fParticleSystem;
		lua_State *fL;
		ListHit fListHit;
		float fPixelsPerMeter;
	};
} // anonymous namespace.

int ParticleSystemObject::_CommonRayCast( lua_State *L,
											b2RayCastCallback *callback )
{
	b2Vec2 from_in_meters( lua_tonumber( L, 2 ), lua_tonumber( L, 3 ) );
	b2Vec2 to_in_meters( lua_tonumber( L, 4 ), lua_tonumber( L, 5 ) );

	// Pixels to meters.
	from_in_meters *= fWorldScaleInMetersPerPixel;
	to_in_meters *= fWorldScaleInMetersPerPixel;

	// Important: If any results are found, "callback" will leave
	// a table at the top of the Lua stack. This table at the top
	// of the Lua stack is the result we return from this function.
	//
	// Exception: For SortedHitsAlongRay, the results are accumulated
	// so they can be sorted before they're pushed to the Lua stack.
	int top_index_before_RayCast = lua_gettop( L );
	fParticleSystem->RayCast( callback, from_in_meters, to_in_meters );

	// Any hits returned by RayCast() are pushed into a table that's
	// on the stack. We want to return true if we're returning a result.
	// Therefore we can compare the top index of the Lua stack before
	// and after RayCast() to know if we're returning a table of hits.
	return ( top_index_before_RayCast != lua_gettop( L ) );
}

int ParticleSystemObject::RayCast( lua_State *L )
{
TRACE_CALL;
	Rtt_ASSERT( fParticleSystem );

    if( ( ! lua_isnumber(L, 2) ) ||
		( ! lua_isnumber(L, 3) ) ||
		( ! lua_isnumber(L, 4) ) ||
		( ! lua_isnumber(L, 5) ) )
    {
		CoronaLuaError(L, "physics.rayCast() requires at least 4 parameters (number, number, number, number)");

		return 0;
    }

	const char *behavior = lua_tostring( L, 6 );

    if( ! Rtt_StringCompare( "any", behavior ) )
    {
        AnyHitAlongRay callback( fParticleSystem,
									L,
									fWorldScaleInPixelsPerMeter );

        return _CommonRayCast( L,
                                &callback );
    }
    else if( ! Rtt_StringCompare( "unsorted", behavior ) )
    {
        UnsortedHitsAlongRay callback( fParticleSystem,
										L,
										fWorldScaleInPixelsPerMeter );

        return _CommonRayCast( L,
                                &callback );
    }
    else if( ! Rtt_StringCompare( "sorted", behavior ) )
    {
        SortedHitsAlongRay callback( fParticleSystem,
										L,
										fWorldScaleInPixelsPerMeter );

        _CommonRayCast( L,
                        &callback );

        // This leaves a table at the top of the Lua stack. This table is
        // the result we return from this function.
        return callback.PushResultsToLua();
    }
    else // if( ! Rtt_StringCompare( "closest", behavior ) )
    {
        // This MUST be the "else" case because it's the default option.

        ClosestHitAlongRay callback( fParticleSystem,
										L,
										fWorldScaleInPixelsPerMeter );

        return _CommonRayCast( L,
                                &callback );
    }
}

namespace // anonymous namespace.
{
	class HitsInRegion : public b2QueryCallback
	{
	public:

		HitsInRegion( b2ParticleSystem *particleSystem,
						lua_State *L,
						float pixelsPerMeter,
						b2Vec2 *optionalPositionDeltaInMeters,
						b2Vec2 *optionalVelocityInMetersPerSecond,
						b2Vec2 *optionalVelocityDeltaInMetersPerSecond )
		: b2QueryCallback()
		, fParticleSystem( particleSystem )
		, fL( L )
		, fInitialTopIndexOfLuaStack( lua_gettop( fL ) )
		, fPixelsPerMeter( pixelsPerMeter )
		, fResultCount( 0 )
		, fOptionalPositionDeltaInMeters( optionalPositionDeltaInMeters )
		, fOptionalVelocityInMetersPerSecond( optionalVelocityInMetersPerSecond )
		, fOptionalVelocityDeltaInMetersPerSecond( optionalVelocityDeltaInMetersPerSecond )
		{
		}

		virtual ~HitsInRegion()
		{
		}

		bool ReportFixture( b2Fixture* fixture )
		{
			return false;
		}

		// particles overlapping the region.
		bool ReportParticle( const b2ParticleSystem* particleSystem,
								int32 index )
		{
			if( fParticleSystem != particleSystem )
			{
				// true: Continue the query.
				return true;
			}

			if( ! fResultCount )
			{
				// We're handling the first result.
				lua_newtable( fL );
			}

			b2Vec2 &positionInMeters = fParticleSystem->GetPositionBuffer()[ index ];
			b2Vec2 &velocityInMetersPerSecond = fParticleSystem->GetVelocityBuffer()[ index ];

			// Apply fOptionalPositionDeltaInMeters.
			if( fOptionalPositionDeltaInMeters )
			{
				positionInMeters += *fOptionalPositionDeltaInMeters;
			}

			// Apply fOptionalVelocityInMetersPerSecond.
			if( fOptionalVelocityInMetersPerSecond )
			{
				velocityInMetersPerSecond = *fOptionalVelocityInMetersPerSecond;
			}

			// Apply fOptionalVelocityDeltaInMetersPerSecond.
			if( fOptionalVelocityDeltaInMetersPerSecond )
			{
				velocityInMetersPerSecond += *fOptionalVelocityDeltaInMetersPerSecond;
			}

			// Add the result.
			//
			// results[n].x : The X coordinate in content-space of this hit.
			// results[n].y : The Y coordinate in content-space of this hit.
			{
				// Meters to pixels.
				b2Vec2 positionInPixels( positionInMeters );
				positionInPixels *= fPixelsPerMeter;

				// Hit position.
				//
				// We can use lua_newtable() here, but we know exactly how
				// many records we'll put in the table (2). So we use
				// lua_createtable() for better performance.
				lua_createtable( fL, 0, 2 );
				{
					// -2 : This is used to refer to the table that was
					// created above, using lua_createtable().
					//
					// lua_createtable() pushes a table to the top of the
					// stack (at position "-1"). Then lua_pushnumber()
					// pushes a number to the top of the stack (the new
					// element at position "-1"). Now the table is right
					// below the top of the stack (at position "-2").
					//
					// An alternative is to use lua_gettop() immediately
					// after the creation of the above table. The index
					// returned will refer to the table, and can be passed
					// to every calls of lua_setfield() instead of "-2".
					lua_pushnumber( fL, positionInPixels.x );
					lua_setfield( fL, -2, "x" );

					lua_pushnumber( fL, positionInPixels.y );
					lua_setfield( fL, -2, "y" );
				}

				// Lua is one-based, so the first result must be at index 1.
				++fResultCount;
				lua_rawseti( fL, -2, fResultCount );
			}

			// true: Continue the query.
			return true;
		}

		b2ParticleSystem *fParticleSystem;
		lua_State *fL;
		int fInitialTopIndexOfLuaStack;
		float fPixelsPerMeter;
		int fResultCount;
		b2Vec2 *fOptionalPositionDeltaInMeters;
		b2Vec2 *fOptionalVelocityInMetersPerSecond;
		b2Vec2 *fOptionalVelocityDeltaInMetersPerSecond;
	};
} // anonymous namespace.

int ParticleSystemObject::QueryRegion( lua_State *L )
{
TRACE_CALL;
	Rtt_ASSERT( fParticleSystem );

    if( ( ! lua_isnumber(L, 2) ) ||
		( ! lua_isnumber(L, 3) ) ||
		( ! lua_isnumber(L, 4) ) ||
		( ! lua_isnumber(L, 5) ) )
    {
		CoronaLuaError(L, "physics.queryRegion() requires 4 parameters (number, number, number, number)");

		return 0;
    }

	b2AABB aabb;
	aabb.lowerBound.Set( lua_tonumber( L, 2 ), lua_tonumber( L, 3 ) );
	aabb.upperBound.Set( lua_tonumber( L, 4 ), lua_tonumber( L, 5 ) );

	// Pixels to meters.
	aabb.lowerBound *= fWorldScaleInMetersPerPixel;
	aabb.upperBound *= fWorldScaleInMetersPerPixel;

	b2Vec2 positionDeltaInMeters;
	b2Vec2 velocityInMetersPerSecond;
	b2Vec2 velocityDeltaInMetersPerSecond;
	b2Vec2 *optionalPositionDeltaInMeters = NULL;
	b2Vec2 *optionalVelocityInMetersPerSecond = NULL;
	b2Vec2 *optionalVelocityDeltaInMetersPerSecond = NULL;

	if( lua_istable( L, 6 ) )
	{
		// Position delta.
		{
			positionDeltaInMeters.SetZero();

			lua_getfield( L, -1, "deltaX" );
			if( lua_isnumber( L, -1 ) )
			{
				positionDeltaInMeters.x = lua_tonumber( L, -1 );

				optionalPositionDeltaInMeters = &positionDeltaInMeters;
			}
			lua_pop( L, 1 );

			lua_getfield( L, -1, "deltaY" );
			if( lua_isnumber( L, -1 ) )
			{
				positionDeltaInMeters.y = lua_tonumber( L, -1 );

				optionalPositionDeltaInMeters = &positionDeltaInMeters;
			}
			lua_pop( L, 1 );

			// Pixels to meters.
			positionDeltaInMeters *= fWorldScaleInMetersPerPixel;
		}

		// Velocity.
		{
			velocityInMetersPerSecond.SetZero();

			lua_getfield( L, -1, "velocityX" );
			if( lua_isnumber( L, -1 ) )
			{
				velocityInMetersPerSecond.x = lua_tonumber( L, -1 );

				optionalVelocityInMetersPerSecond = &velocityInMetersPerSecond;
			}
			lua_pop( L, 1 );

			lua_getfield( L, -1, "velocityY" );
			if( lua_isnumber( L, -1 ) )
			{
				velocityInMetersPerSecond.y = lua_tonumber( L, -1 );

				optionalVelocityInMetersPerSecond = &velocityInMetersPerSecond;
			}
			lua_pop( L, 1 );

			// Pixels to meters.
			velocityInMetersPerSecond *= fWorldScaleInMetersPerPixel;
		}

		// Velocity Delta.
		{
			velocityDeltaInMetersPerSecond.SetZero();

			lua_getfield( L, -1, "deltaVelocityX" );
			if( lua_isnumber( L, -1 ) )
			{
				velocityDeltaInMetersPerSecond.x = lua_tonumber( L, -1 );

				optionalVelocityDeltaInMetersPerSecond = &velocityDeltaInMetersPerSecond;
			}
			lua_pop( L, 1 );

			lua_getfield( L, -1, "deltaVelocityY" );
			if( lua_isnumber( L, -1 ) )
			{
				velocityDeltaInMetersPerSecond.y = lua_tonumber( L, -1 );

				optionalVelocityDeltaInMetersPerSecond = &velocityDeltaInMetersPerSecond;
			}
			lua_pop( L, 1 );

			// Pixels to meters.
			velocityDeltaInMetersPerSecond *= fWorldScaleInMetersPerPixel;
		}

	}

    HitsInRegion callback( fParticleSystem,
							L,
							fWorldScaleInPixelsPerMeter,
							optionalPositionDeltaInMeters,
							optionalVelocityInMetersPerSecond,
							optionalVelocityDeltaInMetersPerSecond );

    // Important: If any results are found, "callback" will leave
    // a table at the top of the Lua stack. This table at the top
    // of the Lua stack is the result we return from this function.
    int top_index_before_QueryAABB = lua_gettop( L );
    fWorld->QueryAABB( &callback, aabb );

    // Any hits returned by QueryAABB() are pushed into a table that's
    // on the stack. We want to return true if we're returning a result.
    // Therefore we can compare the top index of the Lua stack before
    // and after QueryAABB() to know if we're returning a table of hits.
    return ( top_index_before_QueryAABB != lua_gettop( L ) );
}

// -----------------------------------------------------------------------------

} // namespace Rtt

// -----------------------------------------------------------------------------
