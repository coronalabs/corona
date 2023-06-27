//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Display/Rtt_EmitterObject.h"

#include "Rtt_LuaLibSystem.h"
#include "Core/Rtt_AutoPtr.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_DisplayDefaults.h"
#include "Display/Rtt_Shader.h"
#include "Display/Rtt_ShaderData.h"
#include "Display/Rtt_ShaderFactory.h"
#include "Display/Rtt_Scene.h"
#include "Display/Rtt_TextureFactory.h"
#include "Display/Rtt_TextureResource.h"

#include "Renderer/Rtt_Geometry_Renderer.h"
#include "Renderer/Rtt_Renderer.h"

#include "Rtt_LuaUserdataProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_LuaAux.h"

#include "Box2D/Box2D.h"
#include "Rtt_LuaContainer.h"
#include <algorithm>

#include "Rtt_Profiling.h"

// -----------------------------------------------------------------------------

#define VERTICES_PER_QUADS	( 6 )

#define MIN_PARTICLE_LIFESPAN_IN_SECONDS	( 0.05f )

// -----------------------------------------------------------------------------

float clamp( float lower, float upper, float n )
{
	return std::max( lower, std::min( n, upper ) );
}

// -----------------------------------------------------------------------------

namespace Rtt
{

// -----------------------------------------------------------------------------

bool Util_GetBool( LuaMap &lua_map, const char *field_name )
{
	LuaBool *ld = static_cast< LuaBool * >( lua_map.GetData( field_name ) );
	if( ld )
	{
		return ld->GetBool();
	}
	else
	{
		return false;
	}
}

double Util_GetDouble( LuaMap &lua_map, const char *field_name )
{
	LuaDouble *ld = static_cast< LuaDouble * >( lua_map.GetData( field_name ) );
	if( ld )
	{
		return *(double *)ld->GetData();
	}
	else
	{
		return 0.0;
	}
}

#define GET_FLOAT( lua_map, field_name )	( (float)Util_GetDouble( lua_map, field_name ) )

#define GET_INT( lua_map, field_name )		( (int)Util_GetDouble( lua_map, field_name ) )

const char *Util_GetString( LuaMap &lua_map, const char *field_name )
{
	LuaString *ls = static_cast< LuaString * >( lua_map.GetData( field_name ) );
	if( ls )
	{
		return static_cast< std::string * >( ls->GetData() )->c_str();
	}
	else
	{
		return "";
	}
}

// -----------------------------------------------------------------------------

// Particle type
enum kParticleTypes
{
	kParticleTypeGravity,
	kParticleTypeRadial
};

// RAND_MAX is the entire range of rand(). Dividing the result of rand() by
// half its range produces a result twice as large (0..2).
#define GET_RANDOM_0_TO_1( ... )		( (float)rand() / (float)RAND_MAX )
#define GET_RANDOM_MINUS_1_TO_1( ... )	( ( (float)rand() / (float)( RAND_MAX / 2 ) ) - 1.0f )

// Structure used to hold particle specific information
struct EmitterObjectParticle
{
	void Reset();
	void Init( EmitterObject *eo, const Matrix &spawnTimeTransform );
	void Update( EmitterObject *eo, float time_delta );
	void UpdateVertices( EmitterObject *eo,
							EmitterObjectParticle &particle,
							Geometry::Vertex *output_vertices );

	Matrix fSpawnTimeTransform;
	b2Vec2 fPosition;
	b2Vec2 fDirection;
//    b2Vec2 fStartPos;
	Vector4 fColor;
	Vector4 fDeltaColor;
    float fRotation;
    float fRotationDelta;
    float fRadialAcceleration;
    float fTangentialAcceleration;
	float fRadius;
	float fRadiusDelta;
	float fRotationInRadians;
	float fRadiansPerSecond;
	float fParticleSize;
	float fParticleSizeDelta;
	float fTimeToLiveInSeconds;
};

void EmitterObjectParticle::Reset()
{
	fTimeToLiveInSeconds = 0.0f;
}

void EmitterObjectParticle::Init( EmitterObject *eo, const Matrix &spawnTimeTransform )
{
	fSpawnTimeTransform = spawnTimeTransform;

	// Init the position of the particle.  This is based on the source position of the particle emitter
	// plus a configured variance.  The GET_RANDOM_MINUS_1_TO_1 macro allows the number to be both positive
	// and negative
	fPosition.x = eo->fSourcePositionVariance.x * GET_RANDOM_MINUS_1_TO_1();
	fPosition.y = eo->fSourcePositionVariance.y * GET_RANDOM_MINUS_1_TO_1();
//    fStartPos.x = 0.0f;
//    fStartPos.y = 0.0f;


	// Init the direction of the particle.  The newAngleInRadians is calculated using the angle passed in and the
	// angle variance.
	float newAngleInRadians = Rtt_RealDegreesToRadians(eo->fRotationInDegrees + eo->fRotationInDegreesVariance * GET_RANDOM_MINUS_1_TO_1());

	// Create a new GLKVector2 using the newAngleInRadians
	b2Vec2 vector( cosf(newAngleInRadians), sinf(newAngleInRadians) );

	// Calculate the vectorSpeed using the speed and speedVariance which has been passed in
	float vectorSpeed = std::max( 0.0f, ( eo->fSpeed + eo->fSpeedVariance * GET_RANDOM_MINUS_1_TO_1() ) );

	// The particles direction vector is calculated by taking the vector calculated above and
	// multiplying that by the speed
	fDirection = vector;
	fDirection *= vectorSpeed;

    // Calculate the particles life span using the life span and variance passed in
	fTimeToLiveInSeconds = std::max( 0.0f, ( eo->fParticleLifespanInSeconds + eo->fParticleLifespanInSecondsVariance * GET_RANDOM_MINUS_1_TO_1() ) );

    float startRadius = std::max( 0.0f, ( eo->fMaxRadius + eo->fMaxRadiusVariance * GET_RANDOM_MINUS_1_TO_1() ) );
    float endRadius = std::max( 0.0f, ( eo->fMinRadius + eo->fMinRadiusVariance * GET_RANDOM_MINUS_1_TO_1() ) );

	// Set the default diameter of the particle from the source position
	fRadius = startRadius;
	fRadiusDelta = (endRadius - startRadius) / fTimeToLiveInSeconds;

	fRotationInRadians = Rtt_RealDegreesToRadians(eo->fRotationInDegrees + eo->fRotationInDegreesVariance * GET_RANDOM_MINUS_1_TO_1());
	fRadiansPerSecond = Rtt_RealDegreesToRadians(eo->fRotateDegreesPerSecond + eo->fRotateDegreesPerSecondVariance * GET_RANDOM_MINUS_1_TO_1());

    fRadialAcceleration = eo->fRadialAcceleration + eo->fRadialAccelerationVariance * GET_RANDOM_MINUS_1_TO_1();
    fTangentialAcceleration = eo->fTangentialAcceleration + eo->fTangentialAccelerationVariance * GET_RANDOM_MINUS_1_TO_1();

	// Calculate the particle size using the start and finish particle sizes
	float particleStartSize = std::max( 0.0f, eo->fStartParticleSize + eo->fStartParticleSizeVariance * GET_RANDOM_MINUS_1_TO_1() );
	float particleFinishSize = std::max( 0.0f, eo->fFinishParticleSize + eo->fFinishParticleSizeVariance * GET_RANDOM_MINUS_1_TO_1() );
	fParticleSizeDelta = ((particleFinishSize - particleStartSize) / fTimeToLiveInSeconds);
	fParticleSize = particleStartSize;

	// Calculate the color the particle should have when it starts its life.  All the elements
	// of the start color passed in along with the variance are used to calculate the star color
	Vector4 start = {0.0f, 0.0f, 0.0f, 0.0f};
	start.r = clamp( 0.0f, 1.0f, ( eo->fStartColor.r + eo->fStartColorVariance.r * GET_RANDOM_MINUS_1_TO_1() ) );
	start.g = clamp( 0.0f, 1.0f, ( eo->fStartColor.g + eo->fStartColorVariance.g * GET_RANDOM_MINUS_1_TO_1() ) );
	start.b = clamp( 0.0f, 1.0f, ( eo->fStartColor.b + eo->fStartColorVariance.b * GET_RANDOM_MINUS_1_TO_1() ) );
	start.a = clamp( 0.0f, 1.0f, ( eo->fStartColor.a + eo->fStartColorVariance.a * GET_RANDOM_MINUS_1_TO_1() ) );

	// Calculate the color the particle should be when its life is over.  This is done the same
	// way as the start color above
	Vector4 end = {0.0f, 0.0f, 0.0f, 0.0f};
	end.r = clamp( 0.0f, 1.0f, ( eo->fFinishColor.r + eo->fFinishColorVariance.r * GET_RANDOM_MINUS_1_TO_1() ) );
	end.g = clamp( 0.0f, 1.0f, ( eo->fFinishColor.g + eo->fFinishColorVariance.g * GET_RANDOM_MINUS_1_TO_1() ) );
	end.b = clamp( 0.0f, 1.0f, ( eo->fFinishColor.b + eo->fFinishColorVariance.b * GET_RANDOM_MINUS_1_TO_1() ) );
	end.a = clamp( 0.0f, 1.0f, ( eo->fFinishColor.a + eo->fFinishColorVariance.a * GET_RANDOM_MINUS_1_TO_1() ) );

	// Calculate the delta which is to be applied to the particles color during each cycle of its
	// life.  The delta calculation uses the life span of the particle to make sure that the
	// particles color will transition from the start to end color during its life time.  As the game
	// loop is using a fixed delta value we can calculate the delta color once saving cycles in the
	// update method

    fColor = start;
	fDeltaColor.r = ((end.r - start.r) / fTimeToLiveInSeconds);
	fDeltaColor.g = ((end.g - start.g) / fTimeToLiveInSeconds);
	fDeltaColor.b = ((end.b - start.b) / fTimeToLiveInSeconds);
	fDeltaColor.a = ((end.a - start.a) / fTimeToLiveInSeconds);

    // Calculate the rotation
    float startA = eo->fRotationStart + eo->fRotationStartVariance * GET_RANDOM_MINUS_1_TO_1();
    float endA = eo->fRotationEnd + eo->fRotationEndVariance * GET_RANDOM_MINUS_1_TO_1();
    fRotation = startA;
    fRotationDelta = (endA - startA) / fTimeToLiveInSeconds;
}

void EmitterObjectParticle::Update( EmitterObject *eo, float time_delta )
{
    // Reduce the life span of the particle
    fTimeToLiveInSeconds -= time_delta;

	// If the current particle is alive then update it
	if( fTimeToLiveInSeconds <= 0.0f )
	{
		return;
	}

	// If maxRadius is greater than 0 then the particles are going to spin otherwise they are effected by speed and gravity
	if( eo->fEmitterType == kParticleTypeRadial )
	{
        // FIX 2
        // Update the angle of the particle from the sourcePosition and the radius.  This is only done of the particles are rotating
		fRotationInRadians += fRadiansPerSecond * time_delta;
		fRadius += fRadiusDelta * time_delta;

		fPosition.x = - cosf(fRotationInRadians) * fRadius;
		fPosition.y = - sinf(fRotationInRadians) * fRadius;
	}
	else
	{
		b2Vec2 tmp, radial, tangential;

        radial = b2Vec2_zero;

        // By default this emitters particles are moved relative to the emitter node position
//        b2Vec2 positionDifference = fStartPos - b2Vec2_zero;
//        fPosition = fPosition - positionDifference;

        if( fPosition.x || fPosition.y )
		{
            radial = fPosition;
            radial.Normalize();
		}

        tangential = radial;
        radial *= fRadialAcceleration;

        float newy = tangential.x;
        tangential.x = ( - tangential.y );
        tangential.y = newy;
        tangential *= fTangentialAcceleration;

		tmp = ( radial + tangential + eo->fGravity);
        tmp *= time_delta;
		fDirection += tmp;
		tmp = fDirection;
		tmp *= time_delta;
		fPosition += tmp;

        // Now apply the difference calculated early causing the particles to be relative in position to the emitter position
//        fPosition += positionDifference;
	}

	// Update the particles color
	fColor.r += fDeltaColor.r * time_delta;
	fColor.g += fDeltaColor.g * time_delta;
	fColor.b += fDeltaColor.b * time_delta;
	fColor.a += fDeltaColor.a * time_delta;

	// Update the particle size
	fParticleSize += fParticleSizeDelta * time_delta;
    fParticleSize = std::max(0.0f, fParticleSize);

    // Update the rotation of the particle
    fRotation += fRotationDelta * time_delta;
}

bool EmitterObject::ValidateEmitterParent()
{
	if( fAbsolutePositionParent == EMITTER_ABSOLUTE_PARENT || fAbsolutePositionParent == NULL)
		return true;
	DisplayObject *pIt = this;
	while( (pIt = pIt->GetParent()))
	{
		if(pIt == fAbsolutePositionParent)
		{
			return true;
		}
	}
	fAbsolutePositionParent = EMITTER_ABSOLUTE_PARENT;
	return false;
}

inline void EmitterObject::TransformParticlePosition(const Matrix &spawnTimeTransform, Vertex2 &v) const
{
	if( fAbsolutePositionParent == EMITTER_ABSOLUTE_PARENT)
	{
		spawnTimeTransform.Apply(v);
	}
	else if(fAbsolutePositionParent == NULL)
	{
		GetSrcToDstMatrix().Apply(v);
	}
	else
	{
		const Matrix &curParentTransform = fAbsolutePositionParent->GetSrcToDstMatrix();
		spawnTimeTransform.Apply(v);
		curParentTransform.Apply(v);
	}
}

static void SetVertex( Geometry::Vertex& output, const Vertex2& v, const Vector4& color )
{
	output.SetPos( v.x, v.y );
	Geometry::Vertex::SetColor( 1,
								&output,
								color.r,
								color.g,
								color.b,
								color.a );
}

static void EmitRect( Geometry::Vertex *output_vertices, const Vertex2& topLeft, const Vertex2& topRight, const Vertex2& bottomLeft, const Vertex2& bottomRight, const Vector4& color )
{
	SetVertex( output_vertices[0], bottomLeft, color );
	SetVertex( output_vertices[1], topRight, color );
	SetVertex( output_vertices[2], topLeft, color );
	SetVertex( output_vertices[3], bottomLeft, color );
	SetVertex( output_vertices[4], bottomRight, color );
	SetVertex( output_vertices[5], topRight, color );
}

static void Legacy( Geometry::Vertex *output_vertices, const Vertex2& base_position, float rotation, float halfSize, const Vector4& color )
{
	Vertex2 topLeft, topRight, bottomLeft, bottomRight;

    // If a rotation has been defined for this particle then apply the rotation to the vertices that define
    // the particle
    if( rotation )
	{
        float x1 = -halfSize;
        float y1 = -halfSize;
        float x2 = halfSize;
        float y2 = halfSize;
        float x = base_position.x;
        float y = base_position.y;
        float r = Rtt_RealDegreesToRadians( rotation );
        float cr = cosf( r );
        float sr = sinf( r );
        float ax = x1 * cr - y1 * sr + x;
        float ay = x1 * sr + y1 * cr + y;
        float bx = x2 * cr - y1 * sr + x;
        float by = x2 * sr + y1 * cr + y;
        float cx = x2 * cr - y2 * sr + x;
        float cy = x2 * sr + y2 * cr + y;
        float dx = x1 * cr - y2 * sr + x;
        float dy = x1 * sr + y2 * cr + y;

		bottomLeft = { dx, dy };
		topRight = { bx, by };
		topLeft = { ax, ay };
		bottomRight = { cx, cy };
    }
	else
	{
        // Using the position of the particle, work out the four vertices for the quad that will hold the particle
        // and load those into the quads array.

		float xmin = ( base_position.x - halfSize );
		float ymin = ( base_position.y - halfSize );
		float xmax = ( base_position.x + halfSize );
		float ymax = ( base_position.y + halfSize );

		bottomLeft = { xmin, ymax };
		topRight = { xmax, ymin };
		topLeft = { xmin, ymin };
		bottomRight = { xmax, ymax };
	}

	EmitRect( output_vertices, topLeft, topRight, bottomLeft, bottomRight, color );
}

static Vertex2
MapPoint( const Vertex2& basePosition, const Vertex2& xDir, const Vertex2& yDir, float x, float y )
{
	Vertex2 output = basePosition;

	output.x += xDir.x * x + yDir.x * y;
	output.y += xDir.y * x + yDir.y * y;

	return output;
}

static void
RotatePoint( Vertex2& dir, float ca, float sa )
{
	Vertex2 tmp = dir;

	dir.x = tmp.x * ca - tmp.y * sa;
	dir.y = tmp.y * ca + tmp.x * sa;
}

static Vertex2
DiffPoints( const Vertex2& a, const Vertex2& b )
{
	Vertex2 diff = { a.x - b.x, a.y - b.y };
	return diff;
}

static void
ScaleVertex( Vertex2& v, float scale )
{
	v.x *= scale;
	v.y *= scale;
}

static Vertex2
PerpFromXDir( const Vertex2& xDir ) // below, from right
{
	Vertex2 yDir = { -xDir.y, xDir.x };
	return yDir;
}

static Vertex2
PerpFromYDir( const Vertex2& yDir ) // right, from below
{
	Vertex2 xDir = { yDir.y, -yDir.x };
	return xDir;
}

static void Rescale( Geometry::Vertex *output_vertices, EmitterObject::Mapping mapping, const Vertex2& base_position, const Vertex2& right, const Vertex2& below, float rotation, float halfSize, const Vector4& color )
{
	Vertex2 xDir = DiffPoints( right, base_position ), yDir = DiffPoints( below, base_position );
	
	switch ( mapping )
	{
	case EmitterObject::kMapping_Rescale:
		break;
	case EmitterObject::kMapping_RescaleX:
		yDir = PerpFromXDir( xDir );
		break;
	case EmitterObject::kMapping_RescaleY:
		xDir = PerpFromYDir( yDir );
		break;
	default:
		{
			Rtt_Real xLength = Rtt_RealSqrt( xDir.x * xDir.x + xDir.y * xDir.y );
			Rtt_Real yLength = Rtt_RealSqrt( yDir.x * yDir.x + yDir.y * yDir.y );

			if ( EmitterObject::kMapping_RescaleMean == mapping && !Rtt_RealIsZero( xLength ) && !Rtt_RealIsZero( yLength ) ) // if length of 0, fall back to max
			{
				Rtt_Real meanLength = Rtt_RealDiv2( xLength + yLength );

				ScaleVertex( xDir, meanLength / xLength );
				ScaleVertex( xDir, meanLength / yLength );
			}
			else
			{
				bool xIsShorter = xLength < yLength;
				bool fromXDir = ( EmitterObject::kMapping_RescaleMin == mapping ) ? xIsShorter : !xIsShorter;

				if ( fromXDir )
				{
					yDir = PerpFromXDir( xDir );
				}
				else
				{
					xDir = PerpFromYDir( yDir );
				}
			}
		}
	}

    // If a rotation has been defined for this particle then apply the rotation to the vertices that define
    // the particle

	if ( rotation )
	{
		float r = Rtt_RealDegreesToRadians( rotation );
        float cr = cosf( r );
        float sr = sinf( r );

		RotatePoint( xDir, cr, sr );
		RotatePoint( yDir, cr, sr );
	}

	ScaleVertex( xDir, halfSize );
	ScaleVertex( yDir, halfSize );

	Vertex2 topLeft = MapPoint( base_position, xDir, yDir, -1.f, -1.f );
	Vertex2 topRight = MapPoint( base_position, xDir, yDir, +1.f, -1.f );
	Vertex2 bottomLeft = MapPoint( base_position, xDir, yDir, -1.f, +1.f );
	Vertex2 bottomRight = MapPoint( base_position, xDir, yDir, +1.f, +1.f );

	EmitRect( output_vertices, topLeft, topRight, bottomLeft, bottomRight, color );
}

// Rect: output_vertices, top_left, top_right, bottom_left, bottom_right, color
// Legacy: base_position, rotation, halfSize, output_vertices, color

void EmitterObjectParticle::UpdateVertices( EmitterObject *eo,
											EmitterObjectParticle &particle,
											Geometry::Vertex *output_vertices )
{

    // As we are rendering the particles as quads, we need to define 6 vertices for each particle
	// We assume that all particles have a square aspect ratio.
    float halfSize = ( fParticleSize * 0.5f );

	//// Base position.
	//
	Vertex2 base_position = {fPosition.x, fPosition.y};
	Vertex2 right = {base_position.x + 1, base_position.y};
	Vertex2 below = {base_position.x, base_position.y + 1};

	eo->TransformParticlePosition(fSpawnTimeTransform, base_position);

	//// Color.
	//
	Vector4 color;

	float cumulative_alpha = ( (float)eo->AlphaCumulative() * ( 1.0f / 255.0f ) );

	if( eo->fTextureResource->GetBitmap()->IsPremultiplied() )
	{
		color.r = ( fColor.r * cumulative_alpha );
		color.g = ( fColor.g * cumulative_alpha );
		color.b = ( fColor.b * cumulative_alpha );
		color.a = ( fColor.a * cumulative_alpha );
	}
	else
	{
		color.r = fColor.r;
		color.g = fColor.g;
		color.b = fColor.b;
		color.a = ( fColor.a * cumulative_alpha );
	}
	//
	////

	EmitterObject::Mapping mapping = eo->GetMapping();

	if ( EmitterObject::kMapping_Legacy != mapping )
	{
		if ( EmitterObject::kMapping_RescaleY != mapping )
		{
			eo->TransformParticlePosition(fSpawnTimeTransform, right);
		}

		if ( EmitterObject::kMapping_RescaleX != mapping )
		{
			eo->TransformParticlePosition(fSpawnTimeTransform, below);
		}

		Rescale( output_vertices, mapping, base_position, right, below, fRotation, halfSize, color );
	}
	else
	{
		Legacy( output_vertices, base_position, fRotation, halfSize, color );
	}
}

// -----------------------------------------------------------------------------

EmitterObject::EmitterObject()
: Super()
, MUpdatable()
, fShouldUpdate( false )
, fAbsolutePositionParent( NULL )
, fGravity( 0.0f, 0.0f )
, fStartColor( kVector4Zero )
, fStartColorVariance( kVector4Zero )
, fFinishColor( kVector4Zero )
, fFinishColorVariance( kVector4Zero )
, fStartParticleSize( 0.0f )
, fStartParticleSizeVariance( 0.0f )
, fFinishParticleSize( 0.0f )
, fFinishParticleSizeVariance( 0.0f )
, fMaxRadius( 0.0f )
, fMaxRadiusVariance( 0.0f )
, fMinRadius( 0.0f )
, fMinRadiusVariance( 0.0f )
, fRotateDegreesPerSecond( 0.0f )
, fRotateDegreesPerSecondVariance( 0.0f )
, fRotationStart( 0.0f )
, fRotationStartVariance( 0.0f )
, fRotationEnd( 0.0f )
, fRotationEndVariance( 0.0f )
, fSpeed( 0.0f )
, fSpeedVariance( 0.0f )
, fEmissionRateInParticlesPerSeconds( 0.0f )
, fRadialAcceleration( 0.0f )
, fRadialAccelerationVariance( 0.0f )
, fTangentialAcceleration( 0.0f )
, fTangentialAccelerationVariance( 0.0f )
, fSourcePositionVariance( 0.0f, 0.0f )
, fRotationInDegrees( 0.0f )
, fRotationInDegreesVariance( 0.0f )
, fParticleLifespanInSeconds( 0.0f )
, fParticleLifespanInSecondsVariance( 0.0f )
, fEmitterType( 0 )
, fMaxParticles( 0 )
, fDuration( 0.0f )
, fBlendFuncSource( 0 )
, fBlendFuncDestination( 0 )
, fEmitCounter( 0.0f )
, fElapsedTime( 0.0f )
, fTextureFileName()
, fParticles( NULL )
, fParticleCount( 0 )
, fMapping( kMapping_Legacy )
, fState( kState_Playing )
, fTextureResource()
, fData()
, fShader( NULL )
{
    SetObjectDesc("EmitterObject"); // for introspection
}

EmitterObject::~EmitterObject()
{
	_Cleanup();
}

void EmitterObject::_Cleanup()
{
	StageObject *stage = GetStage();
	if( stage )
	{
		stage->GetScene().RemoveActiveUpdatable( this );
	}

	if( fParticles )
	{
		Rtt_FREE( fParticles );

		fParticles = NULL;
	}

	if( fData.fGeometry )
	{
		QueueRelease( fData.fGeometry );

		fData.fGeometry = NULL;
	}
}

// display.newEmitter( parms, [baseDir] )
bool EmitterObject::Initialize( lua_State *L, Display &display )
{
	Rtt_ASSERT( ! fParticles );
    
    int index = 1;
	LuaMap params( L, index++ );

	//// Simulation (tweakable from Lua).
	//
	fAbsolutePositionParent = Util_GetBool( params, "absolutePosition" )?NULL:EMITTER_ABSOLUTE_PARENT;

	fGravity.x = GET_FLOAT( params, "gravityx" );
	fGravity.y = GET_FLOAT( params, "gravityy" );

	fStartColor.r = GET_FLOAT( params, "startColorRed" );
	fStartColor.g = GET_FLOAT( params, "startColorGreen" );
	fStartColor.b = GET_FLOAT( params, "startColorBlue" );
	fStartColor.a = GET_FLOAT( params, "startColorAlpha" );

	fStartColorVariance.r = GET_FLOAT( params, "startColorVarianceRed" );
	fStartColorVariance.g = GET_FLOAT( params, "startColorVarianceGreen" );
	fStartColorVariance.b = GET_FLOAT( params, "startColorVarianceBlue" );
	fStartColorVariance.a = GET_FLOAT( params, "startColorVarianceAlpha" );

	fFinishColor.r = GET_FLOAT( params, "finishColorRed" );
	fFinishColor.g = GET_FLOAT( params, "finishColorGreen" );
	fFinishColor.b = GET_FLOAT( params, "finishColorBlue" );
	fFinishColor.a = GET_FLOAT( params, "finishColorAlpha" );

	fFinishColorVariance.r = GET_FLOAT( params, "finishColorVarianceRed" );
	fFinishColorVariance.g = GET_FLOAT( params, "finishColorVarianceGreen" );
	fFinishColorVariance.b = GET_FLOAT( params, "finishColorVarianceBlue" );
	fFinishColorVariance.a = GET_FLOAT( params, "finishColorVarianceAlpha" );

	fStartParticleSize = GET_FLOAT( params, "startParticleSize" );
	fStartParticleSizeVariance = GET_FLOAT( params, "startParticleSizeVariance" );
	fFinishParticleSize = GET_FLOAT( params, "finishParticleSize" );
	fFinishParticleSizeVariance = GET_FLOAT( params, "finishParticleSizeVariance" );
	//
	////

	// These paramters are used when you want to have the particles spinning around the source location
	fMaxRadius = GET_FLOAT( params, "maxRadius" );
	fMaxRadiusVariance = GET_FLOAT( params, "maxRadiusVariance" );
	fMinRadius = GET_FLOAT( params, "minRadius" );
	fMinRadiusVariance = GET_FLOAT( params, "minRadiusVariance" );
	fRotateDegreesPerSecond = GET_FLOAT( params, "rotatePerSecond" );
	fRotateDegreesPerSecondVariance = GET_FLOAT( params, "rotatePerSecondVariance" );
	fRotationStart = GET_FLOAT( params, "rotationStart" );
	fRotationStartVariance = GET_FLOAT( params, "rotationStartVariance" );
	fRotationEnd = GET_FLOAT( params, "rotationEnd" );
	fRotationEndVariance = GET_FLOAT( params, "rotationEndVariance" );

	fSpeed = GET_FLOAT( params, "speed" );
	fSpeedVariance = GET_FLOAT( params, "speedVariance" );

	fEmitterType = GET_INT( params, "emitterType" );

	fSourcePositionVariance.x = GET_FLOAT( params, "sourcePositionVariancex" );
	fSourcePositionVariance.y = GET_FLOAT( params, "sourcePositionVariancey" );

	fParticleLifespanInSeconds = GET_FLOAT( params, "particleLifespan" );
	fParticleLifespanInSecondsVariance = GET_FLOAT( params, "particleLifespanVariance" );
	fRotationInDegrees = GET_FLOAT( params, "angle" );
	fRotationInDegreesVariance = GET_FLOAT( params, "angleVariance" );

	fRadialAcceleration = GET_FLOAT( params, "radialAcceleration" );
	fRadialAccelerationVariance = GET_FLOAT( params, "radialAccelVariance" );
	fTangentialAcceleration = GET_FLOAT( params, "tangentialAcceleration" );
	fTangentialAccelerationVariance = GET_FLOAT( params, "tangentialAccelVariance" );

	fMaxParticles = GET_INT( params, "maxParticles" );
	fDuration = GET_FLOAT( params, "duration" );

	fBlendFuncSource = GET_INT( params, "blendFuncSource" );
	fBlendFuncDestination = GET_INT( params, "blendFuncDestination" );

	if( ! fMaxParticles )
	{
		fMaxParticles = 1;
		CoronaLuaWarning( L, "Invalid Particle Designer file - \"maxParticles\" MUST be non-zero. We'll set this to 1 for now. Please adjust this value in Particle Designer and re-export the effect" );
	}

	if( ! fParticleLifespanInSeconds )
	{
		fParticleLifespanInSeconds = MIN_PARTICLE_LIFESPAN_IN_SECONDS;
		CoronaLuaWarning( L, "Invalid Particle Designer file - \"particleLifespan\" MUST be non-zero. We'll set this to %g for now. Please adjust this value in Particle Designer and re-export the effect", MIN_PARTICLE_LIFESPAN_IN_SECONDS );
	}

	// Calculate the emission rate
	fEmissionRateInParticlesPerSeconds = ( (float)fMaxParticles / fParticleLifespanInSeconds );
	fEmitCounter = 0.0f;

	fParticles = (EmitterObjectParticle *)Rtt_MALLOC( display.GetAllocator(),
													sizeof( EmitterObjectParticle ) * (int)fMaxParticles );

	// Get any mapping, else use the display default.
	lua_getfield( L, index - 1, "emitterMapping" );

	if ( lua_isstring( L, -1 ) )
	{
		fMapping = GetMappingForString( lua_tostring( L, -1 ), kMapping_Rescale );
	}
	else if ( lua_toboolean( L, -1 ) )
	{
		fMapping = kMapping_Rescale;
	}
	else
	{
		fMapping = (Mapping)display.GetDefaults().GetEmitterMapping();
	}

	lua_pop( L, 1 );

	// Texture
	{
		fTextureFileName = Util_GetString( params, "textureFileName" );
		if( fTextureFileName.empty() )
		{
			/*
			std::string textureImageData = Util_GetString( params, "textureImageData" );
			if( textureImageData.empty() )
			{
				Rtt_LogException( "NO textureFileName, and NO textureImageData, specified." );
				_Cleanup();
				return false;
			}
			else
			{
				// Base64-decode the content of textureImageData HERE!!!
				// We can use luasocket (b64decode) or OpenSSL (BIO_f_base64) for this.
			}
			*/

			// We DON'T support "textureImageData" yet, so we have to
			// early-out if "textureFileName" HASN'T been specified.
			_Cleanup();
            CoronaLuaError(L, "ERROR: emitter initialization: textureFileName hasn't been specified");
			return false;
		}
		else
		{
			// A fTextureFileName HAS been specified.

            // Check for a supplied baseDir
            MPlatform::Directory baseDir = MPlatform::kResourceDir;
            
            if ( lua_islightuserdata( L, index ) )
            {
                void* p = lua_touserdata( L, index );
                baseDir = (MPlatform::Directory)EnumForUserdata(
                            LuaLibSystem::Directories(),
                            p,
                            MPlatform::kNumDirs,
                            MPlatform::kResourceDir );
                index++;
            }
			fTextureResource = display.GetTextureFactory().FindOrCreate( fTextureFileName.c_str(),
																			baseDir,
																			( PlatformBitmap::kIsNearestAvailablePixelDensity |
																				PlatformBitmap::kIsBitsFullResolution ), //!< Image sheets should be loaded at full resolution AND support dynamic image resolution
																			false );
			if( fTextureResource.IsNull() )
			{
                _Cleanup();
                CoronaLuaError(L, "ERROR: emitter initialization: can't find texture name: %s",
									fTextureFileName.c_str() );
				return false;
			}
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

		fData.fGeometry->Resize( ( fMaxParticles * VERTICES_PER_QUADS ),
									false );
		fData.fGeometry->SetVerticesUsed( fParticleCount * VERTICES_PER_QUADS );

		// Set every member of Vertex that NEVER change.
		{
			Geometry::Vertex *output_vertices = fData.fGeometry->GetVertexData();

			for( int i = 0;
					i < fMaxParticles;
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

	// Set the color.
	Geometry::Vertex::SetColor( ( fParticleCount * VERTICES_PER_QUADS ),
								fData.fGeometry->GetVertexData(),
								1.0f,//color.r,
								1.0f,//color.g,
								1.0f,//color.b,
								1.0f );//color.a

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

	// Blend mode.
	{
		// Source
		fData.fBlendMode.fSrcColor = BlendMode::ParamForGLenum( fBlendFuncSource );
		fData.fBlendMode.fSrcAlpha = BlendMode::ParamForGLenum( fBlendFuncSource );

		// Destination.
		fData.fBlendMode.fDstColor = BlendMode::ParamForGLenum( fBlendFuncDestination );
		fData.fBlendMode.fDstAlpha = BlendMode::ParamForGLenum( fBlendFuncDestination );
	}

	/**/ //SET OR UNSET ANY VALIDITY FLAGS HERE???
	//SetValid( kBlendFlag );

	// We'll leave the default to:
	//fData.fBlendEquation = RenderTypes::kAddEquation;
	// Other valid values are:
	//fData.fBlendEquation = RenderTypes::kSubtractEquation;
	//fData.fBlendEquation = RenderTypes::kReverseSubtractEquation;
	//fData.fBlendEquation = RenderTypes::kDisabledEquation;

	display.GetScene().AddActiveUpdatable( this );

	return true;
}

void EmitterObject::Draw( Renderer& renderer ) const
{
	Rtt_ASSERT( fParticles );

	if( ! ShouldDraw() )
	{
		return;
	}

	SUMMED_TIMING( ed, "Emitter: Draw" );

#if defined(Rtt_EMSCRIPTEN_ENV)
	if (fData.fGeometry->GetStoredOnGPU())
	{
		fData.fGeometry->Invalidate();
	}
#endif

	// This does the renderer.Insert( &fData ).
	fShader->Draw( renderer, fData );
}

void EmitterObject::GetSelfBounds( Rect& rect ) const
{
	Rtt_ASSERT( fParticles );

	rect.xMin = -0.5f;
	rect.yMin = -0.5f;
	rect.xMax = 0.5f;
	rect.yMax = 0.5f;
}

bool EmitterObject::HitTest( Real contentX, Real contentY )
{
	Rtt_ASSERT( fParticles );

	return false;
}

ShaderResource::ProgramMod EmitterObject::GetProgramMod() const
{
	Rtt_ASSERT( fParticles );

	return ShaderResource::kDefault;
}

void EmitterObject::_AddParticle(const Matrix &spawnTimeTransform)
{
	// If we have already reached the maximum number of particles then do nothing
	if( fParticleCount >= fMaxParticles )
	{
		return;
	}

	// Take the next particle out of the particle pool we have created and initialize it
	fParticles[ fParticleCount ].Init( this, spawnTimeTransform );

	// Increment the particle count
	fParticleCount++;

	fData.fGeometry->SetVerticesUsed( fParticleCount * VERTICES_PER_QUADS );
}

void EmitterObject::_RemoveParticle( int particleIndex )
{
	// As the particle is not alive anymore replace it with the last active particle
	// in the array and reduce the count of particles by one.  This causes all active particles
	// to be packed together at the start of the array so that a particle which has run out of
	// life will only drop into this clause once
	if( particleIndex != fParticleCount - 1 )
	{
		fParticles[particleIndex] = fParticles[fParticleCount - 1];
	}

	fParticleCount--;

	fData.fGeometry->SetVerticesUsed( fParticleCount * VERTICES_PER_QUADS );
}

bool
EmitterObject::CanCull() const
{
	Rtt_ASSERT( fParticles );

	return false;
}

const LuaProxyVTable &EmitterObject::ProxyVTable() const
{
	Rtt_ASSERT( fParticles );
	return LuaEmitterObjectProxyVTable::Constant();
}

void EmitterObject::Prepare( const Display &display )
{
	SUMMED_TIMING( ep, "Emitter: Prepare" );

	Rtt_ASSERT( fParticles );

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

void EmitterObject::_Update( const Display &display )
{
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

	////
	//
	if( fState == kState_Paused )
	{
		// Nothing to do.
		return;
	}

	// The current fState is either kState_Playing or kState_Stopped.
	//
	////

	// It's possible for the application to be put on hold for more
	// than a second by the OS.
	//
	// For effects containing short-lived particles, this has the
	// side-effect of killing and respawning all particles on the
	// same frame, when the application becomes active.
	//
	// Visually, with a repeating effect, this makes the emitter
	// look like its spawning particles in waves.
	//
	// Our solution is to override time_delta when it's greater
	// than 1 second.
	float time_delta = display.GetDeltaTimeInSeconds();
	if( time_delta > 1.0f )
	{
		// 30 FPS.
		time_delta = ( 1.0f / 30.0f );
	}

	/**/ //SET OR UNSET ANY VALIDITY FLAGS HERE???
	ValidateEmitterParent();

	// If the emitter is active and the emission rate
	// is greater than zero then emit particles
	if( fState != kState_Stopped )
	{
		// We should still be potentially emitting particles.

		float rate = ( 1.0f / fEmissionRateInParticlesPerSeconds );

		if( fParticleCount < fMaxParticles )
		{
			// There's still room to add more particles.

            fEmitCounter += time_delta;
		}

		Matrix spawnTimeTransform;
		bool transformInited = false;
		while( ( fParticleCount < fMaxParticles ) &&
				( fEmitCounter > rate ) )
		{
			if(!transformInited)
			{
				transformInited = true;
				if(fAbsolutePositionParent != NULL && fAbsolutePositionParent != EMITTER_ABSOLUTE_PARENT)
				{
					Matrix::Invert(fAbsolutePositionParent->GetSrcToDstMatrix(), spawnTimeTransform);
					spawnTimeTransform.Concat(GetSrcToDstMatrix());
				}
				else
				{
					spawnTimeTransform = GetSrcToDstMatrix();
				}
			}
			_AddParticle(spawnTimeTransform);
			fEmitCounter -= rate;
		}

		fElapsedTime += time_delta;

		if( ( fDuration != -1 ) &&
			( fDuration < fElapsedTime ) )
		{
			Stop();
		}
	}

	// Reset the particle index before updating the particles in this emitter
	int particleIndex = 0;

	Geometry::Vertex *output_vertices = fData.fGeometry->GetVertexData();

    // Loop through all the particles updating their location and color
	while( particleIndex < fParticleCount )
	{
		// Get the particle for the current particle index
		EmitterObjectParticle &particle = fParticles[particleIndex];

		particle.Update( this, time_delta );

		if( particle.fTimeToLiveInSeconds > 0.0f )
		{
			particle.UpdateVertices( this,
										particle,
										&( output_vertices[ particleIndex * VERTICES_PER_QUADS ] ) );

			// Update the particle and vertex counters
			particleIndex++;
		}
		else
		{
			_RemoveParticle( particleIndex );
		}
	}
}

void EmitterObject::SetEmissionRateInParticlesPerSeconds( float v )
{
	fEmissionRateInParticlesPerSeconds = std::max( FLT_EPSILON, v );
}

void EmitterObject::SetParticleLifespanInSeconds( float v )
{
	fParticleLifespanInSeconds = std::max( MIN_PARTICLE_LIFESPAN_IN_SECONDS, v );
}

void EmitterObject::Start()
{
	if( fState == kState_Stopped )
	{
		// kState_Stopped -> kState_Playing.

		// Reset all the particles.
		for( int i = 0;
				i < fParticleCount;
				++i )
		{
			fParticles[i].Reset();
		}

		fElapsedTime = 0.0f;
		fEmitCounter = 0.0f;
	}

	fState = kState_Playing;
}

void EmitterObject::Stop()
{
	fState = kState_Stopped;
}

void EmitterObject::Pause()
{
	fState = kState_Paused;
}

// -----------------------------------------------------------------------------

const char* EmitterObject::GetStringForMapping( Mapping mapping )
{
	switch ( mapping )
	{
		kMapping_Rescale:
			return "rescale";
		kMapping_RescaleX:
			return "rescaleX";
		kMapping_RescaleY:
			return "rescaleY";
		kMapping_RescaleMin:
			return "rescaleMin";
		kMapping_RescaleMax:
			return "rescaleMax";
		kMapping_RescaleMean:
			return "rescaleMean";
		default:
			return "legacy";
	}
}

EmitterObject::Mapping EmitterObject::GetMappingForString( const char* string, Mapping def )
{
	if ( 0 == Rtt_StringCompare( string, "rescale" ) )
	{
		return kMapping_Rescale;
	}
	else if ( 0 == Rtt_StringCompare( string, "rescaleX" ) )
	{
		return kMapping_RescaleX;
	}
	else if ( 0 == Rtt_StringCompare( string, "rescaleY" ) )
	{
		return kMapping_RescaleY;
	}
	else if ( 0 == Rtt_StringCompare( string, "rescaleMin" ) )
	{
		return kMapping_RescaleMin;
	}
	else if ( 0 == Rtt_StringCompare( string, "rescaleMax" ) )
	{
		return kMapping_RescaleMax;
	}
	else if ( 0 == Rtt_StringCompare( string, "rescaleMean" ) )
	{
		return kMapping_RescaleMean;
	}
	else if ( 0 == Rtt_StringCompare( string, "legacy" ) )
	{
		return kMapping_Legacy;
	}
	else
	{
		return def;
	}
}

const char *EmitterObject::GetStringForState( int state )
{
	if( state == kState_Playing )
	{
		return "playing";
	}
	else if( state == kState_Stopped )
	{
		return "stopped";
	}
	else if( state == kState_Paused )
	{
		return "paused";
	}
	else
	{
		return "UNKNOWN";
	}
}

// -----------------------------------------------------------------------------

} // namespace Rtt

// -----------------------------------------------------------------------------
