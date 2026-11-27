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
#include "Display/Rtt_EmitterSIMD.h"

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
#include <limits>

#include "Rtt_Profiling.h"
#include "Rtt_Runtime.h"

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

// Particle state, as a structure of arrays. Live particles are packed densely
// in [0, fParticleCount), so every pass is a forward walk over a few arrays.
//
// Only the fields used by the emitter's type are allocated: gravity and radial
// emitters share the slots following kCommonFieldCount.
struct EmitterObjectParticles
{
	enum Field
	{
		kPositionX,
		kPositionY,
		kTimeToLiveInSeconds,
		kColorR,
		kColorG,
		kColorB,
		kColorA,
		kDeltaColorR,
		kDeltaColorG,
		kDeltaColorB,
		kDeltaColorA,
		kParticleSize,
		kParticleSizeDelta,
		kRotation,
		kRotationDelta,

		kCommonFieldCount,

		// Gravity emitters.
		kDirectionX = kCommonFieldCount,
		kDirectionY,
		kRadialAcceleration,
		kTangentialAcceleration,

		// Radial emitters.
		kRotationInRadians = kCommonFieldCount,
		kRadiansPerSecond,
		kRadius,
		kRadiusDelta,

		// Spawn-time transform: rows ( a, b, c ) and ( d, e, f ), as in Matrix,
		// with a = infinity for the identity (and the rest 0).
		kSpawnA = kCommonFieldCount + 4,
		kSpawnB,
		kSpawnC,
		kSpawnD,
		kSpawnE,
		kSpawnF,

		kFieldCount
	};

	static EmitterObjectParticles *Create( Rtt_Allocator *allocator, int capacity );
	static void Destroy( EmitterObjectParticles *particles );

	void Init( int index, EmitterObject *eo, const Matrix &spawnTimeTransform );
	void Move( int to, int from );

	// Same result as Matrix::Apply() with the particle's spawn-time transform.
	void ApplySpawnTimeTransform( int index, Vertex2 &v ) const;

	float *fFields[kFieldCount];
};

EmitterObjectParticles *
EmitterObjectParticles::Create( Rtt_Allocator *allocator, int capacity )
{
	// Round each array up to a multiple of 4 floats, keeping them 16-byte aligned
	// relative to one another.
	size_t stride = ( ( (size_t)capacity + 3 ) & ~(size_t)3 );
	size_t headerSize = ( ( sizeof( EmitterObjectParticles ) + 15 ) & ~(size_t)15 );
	size_t fieldsSize = ( kFieldCount * stride * sizeof( float ) );

	U8 *storage = (U8 *)Rtt_MALLOC( allocator, headerSize + fieldsSize );
	EmitterObjectParticles *particles = (EmitterObjectParticles *)storage;
	float *fields = (float *)( storage + headerSize );

	for( int i = 0; i < kFieldCount; ++i )
	{
		particles->fFields[i] = fields + ( i * stride );
	}

	return particles;
}

void
EmitterObjectParticles::Destroy( EmitterObjectParticles *particles )
{
	Rtt_FREE( particles );
}

void
EmitterObjectParticles::Move( int to, int from )
{
	for( int i = 0; i < kFieldCount; ++i )
	{
		fFields[i][to] = fFields[i][from];
	}
}

void
EmitterObjectParticles::ApplySpawnTimeTransform( int index, Vertex2 &v ) const
{
	float a = fFields[kSpawnA][index];

	if ( Rtt_REAL_INF != a )
	{
		// As MatrixRowDotVertex2() computes it.
		Real x = fFields[kSpawnC][index];
		x += a * v.x;
		x += fFields[kSpawnB][index] * v.y;

		Real y = fFields[kSpawnF][index];
		y += fFields[kSpawnD][index] * v.x;
		y += fFields[kSpawnE][index] * v.y;

		v.x = x;
		v.y = y;
	}
}

void EmitterObjectParticles::Init( int index, EmitterObject *eo, const Matrix &spawnTimeTransform )
{
	float **f = fFields;

	if ( spawnTimeTransform.IsIdentity() )
	{
		f[kSpawnA][index] = Rtt_REAL_INF;
		f[kSpawnB][index] = f[kSpawnC][index] = f[kSpawnD][index] = f[kSpawnE][index] = f[kSpawnF][index] = 0.0f;
	}
	else
	{
		const Real *row0 = spawnTimeTransform.Row0(), *row1 = spawnTimeTransform.Row1();

		f[kSpawnA][index] = row0[0];
		f[kSpawnB][index] = row0[1];
		f[kSpawnC][index] = row0[2];
		f[kSpawnD][index] = row1[0];
		f[kSpawnE][index] = row1[1];
		f[kSpawnF][index] = row1[2];
	}

	// NOTE: Every random value below is drawn, in this order, whatever the emitter
	// type. This keeps the sequence of rand() calls the same as it has always been.

	// Init the position of the particle.  This is based on the source position of the particle emitter
	// plus a configured variance.  The GET_RANDOM_MINUS_1_TO_1 macro allows the number to be both positive
	// and negative
	f[kPositionX][index] = eo->fSourcePositionVariance.x * GET_RANDOM_MINUS_1_TO_1();
	f[kPositionY][index] = eo->fSourcePositionVariance.y * GET_RANDOM_MINUS_1_TO_1();

	// Init the direction of the particle.  The newAngleInRadians is calculated using the angle passed in and the
	// angle variance.
	float newAngleInRadians = Rtt_RealDegreesToRadians(eo->fRotationInDegrees + eo->fRotationInDegreesVariance * GET_RANDOM_MINUS_1_TO_1());

	// Calculate the vectorSpeed using the speed and speedVariance which has been passed in
	float vectorSpeed = std::max( 0.0f, ( eo->fSpeed + eo->fSpeedVariance * GET_RANDOM_MINUS_1_TO_1() ) );

	// Calculate the particles life span using the life span and variance passed in
	float timeToLive = std::max( 0.0f, ( eo->fParticleLifespanInSeconds + eo->fParticleLifespanInSecondsVariance * GET_RANDOM_MINUS_1_TO_1() ) );

	// A particle whose sampled life span is 0 dies on its first update, before it is
	// drawn. Give it zero rates, rather than dividing by 0.
	bool hasLifetime = timeToLive > 0.0f;

	float startRadius = std::max( 0.0f, ( eo->fMaxRadius + eo->fMaxRadiusVariance * GET_RANDOM_MINUS_1_TO_1() ) );
	float endRadius = std::max( 0.0f, ( eo->fMinRadius + eo->fMinRadiusVariance * GET_RANDOM_MINUS_1_TO_1() ) );

	float rotationInRadians = Rtt_RealDegreesToRadians(eo->fRotationInDegrees + eo->fRotationInDegreesVariance * GET_RANDOM_MINUS_1_TO_1());
	float radiansPerSecond = Rtt_RealDegreesToRadians(eo->fRotateDegreesPerSecond + eo->fRotateDegreesPerSecondVariance * GET_RANDOM_MINUS_1_TO_1());

	float radialAcceleration = eo->fRadialAcceleration + eo->fRadialAccelerationVariance * GET_RANDOM_MINUS_1_TO_1();
	float tangentialAcceleration = eo->fTangentialAcceleration + eo->fTangentialAccelerationVariance * GET_RANDOM_MINUS_1_TO_1();

	if( eo->fEmitterType == kParticleTypeRadial )
	{
		// Set the default diameter of the particle from the source position
		f[kRadius][index] = startRadius;
		f[kRadiusDelta][index] = hasLifetime ? (endRadius - startRadius) / timeToLive : 0.0f;
		f[kRotationInRadians][index] = rotationInRadians;
		f[kRadiansPerSecond][index] = radiansPerSecond;
	}
	else
	{
		// The particles direction vector is calculated by taking the unit vector of the angle
		// above and multiplying that by the speed
		f[kDirectionX][index] = cosf(newAngleInRadians) * vectorSpeed;
		f[kDirectionY][index] = sinf(newAngleInRadians) * vectorSpeed;
		f[kRadialAcceleration][index] = radialAcceleration;
		f[kTangentialAcceleration][index] = tangentialAcceleration;
	}

	f[kTimeToLiveInSeconds][index] = timeToLive;

	// Calculate the particle size using the start and finish particle sizes
	float particleStartSize = std::max( 0.0f, eo->fStartParticleSize + eo->fStartParticleSizeVariance * GET_RANDOM_MINUS_1_TO_1() );
	float particleFinishSize = std::max( 0.0f, eo->fFinishParticleSize + eo->fFinishParticleSizeVariance * GET_RANDOM_MINUS_1_TO_1() );
	f[kParticleSizeDelta][index] = hasLifetime ? ((particleFinishSize - particleStartSize) / timeToLive) : 0.0f;
	f[kParticleSize][index] = particleStartSize;

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
	// particles color will transition from the start to end color during its life time.
	f[kColorR][index] = start.r;
	f[kColorG][index] = start.g;
	f[kColorB][index] = start.b;
	f[kColorA][index] = start.a;
	f[kDeltaColorR][index] = hasLifetime ? ((end.r - start.r) / timeToLive) : 0.0f;
	f[kDeltaColorG][index] = hasLifetime ? ((end.g - start.g) / timeToLive) : 0.0f;
	f[kDeltaColorB][index] = hasLifetime ? ((end.b - start.b) / timeToLive) : 0.0f;
	f[kDeltaColorA][index] = hasLifetime ? ((end.a - start.a) / timeToLive) : 0.0f;

	// Calculate the rotation
	float startA = eo->fRotationStart + eo->fRotationStartVariance * GET_RANDOM_MINUS_1_TO_1();
	float endA = eo->fRotationEnd + eo->fRotationEndVariance * GET_RANDOM_MINUS_1_TO_1();
	f[kRotation][index] = startA;
	f[kRotationDelta][index] = hasLifetime ? (endA - startA) / timeToLive : 0.0f;
}

// The update kernels below process 4 particles at a time when SIMD is available,
// with the same operations, in the same order, as their scalar loops, which also
// handle the last count % 4 particles. The exception is trigonometry, which uses
// EmitterSIMD::SinCos(): within a couple of units in the last place of sinf() /
// cosf(), but not identical.
#ifdef Rtt_EMITTER_SIMD
using namespace EmitterSIMD;
#endif

// Advances the state shared by both emitter types.
static void
UpdateCommonParticleState( float **f, int count, float time_delta )
{
	float * Rtt_RESTRICT r = f[EmitterObjectParticles::kColorR];
	float * Rtt_RESTRICT g = f[EmitterObjectParticles::kColorG];
	float * Rtt_RESTRICT b = f[EmitterObjectParticles::kColorB];
	float * Rtt_RESTRICT a = f[EmitterObjectParticles::kColorA];
	const float * Rtt_RESTRICT dr = f[EmitterObjectParticles::kDeltaColorR];
	const float * Rtt_RESTRICT dg = f[EmitterObjectParticles::kDeltaColorG];
	const float * Rtt_RESTRICT db = f[EmitterObjectParticles::kDeltaColorB];
	const float * Rtt_RESTRICT da = f[EmitterObjectParticles::kDeltaColorA];
	float * Rtt_RESTRICT size = f[EmitterObjectParticles::kParticleSize];
	const float * Rtt_RESTRICT sizeDelta = f[EmitterObjectParticles::kParticleSizeDelta];
	float * Rtt_RESTRICT rotation = f[EmitterObjectParticles::kRotation];
	const float * Rtt_RESTRICT rotationDelta = f[EmitterObjectParticles::kRotationDelta];

	int i = 0;

#ifdef Rtt_EMITTER_SIMD
	V4 dt = Splat( time_delta );

	for( ; i + 4 <= count; i += 4 )
	{
		Store( r + i, Add( Load( r + i ), Mul( Load( dr + i ), dt ) ) );
		Store( g + i, Add( Load( g + i ), Mul( Load( dg + i ), dt ) ) );
		Store( b + i, Add( Load( b + i ), Mul( Load( db + i ), dt ) ) );
		Store( a + i, Add( Load( a + i ), Mul( Load( da + i ), dt ) ) );
		Store( size + i, MaxZero( Add( Load( size + i ), Mul( Load( sizeDelta + i ), dt ) ) ) );
		Store( rotation + i, Add( Load( rotation + i ), Mul( Load( rotationDelta + i ), dt ) ) );
	}
#endif

	for( ; i < count; ++i )
	{
		// Update the particles color
		r[i] += dr[i] * time_delta;
		g[i] += dg[i] * time_delta;
		b[i] += db[i] * time_delta;
		a[i] += da[i] * time_delta;

		// Update the particle size
		size[i] = std::max( 0.0f, size[i] + sizeDelta[i] * time_delta );

		// Update the rotation of the particle
		rotation[i] += rotationDelta[i] * time_delta;
	}
}

static void
UpdateRadialParticles( float **f, int count, float time_delta )
{
	float * Rtt_RESTRICT x = f[EmitterObjectParticles::kPositionX];
	float * Rtt_RESTRICT y = f[EmitterObjectParticles::kPositionY];
	float * Rtt_RESTRICT angle = f[EmitterObjectParticles::kRotationInRadians];
	const float * Rtt_RESTRICT radiansPerSecond = f[EmitterObjectParticles::kRadiansPerSecond];
	float * Rtt_RESTRICT radius = f[EmitterObjectParticles::kRadius];
	const float * Rtt_RESTRICT radiusDelta = f[EmitterObjectParticles::kRadiusDelta];

	int i = 0;

#ifdef Rtt_EMITTER_SIMD
	V4 dt = Splat( time_delta );

	for( ; i + 4 <= count; i += 4 )
	{
		V4 theta = Add( Load( angle + i ), Mul( Load( radiansPerSecond + i ), dt ) );
		V4 r = Add( Load( radius + i ), Mul( Load( radiusDelta + i ), dt ) );
		V4 sines, cosines;

		SinCos( theta, sines, cosines ); // NOT bit-identical to sinf() / cosf()

		Store( angle + i, theta );
		Store( radius + i, r );
		Store( x + i, Mul( Neg( cosines ), r ) );
		Store( y + i, Mul( Neg( sines ), r ) );
	}
#endif

	for( ; i < count; ++i )
	{
		// Update the angle of the particle from the sourcePosition and the radius.
		angle[i] += radiansPerSecond[i] * time_delta;
		radius[i] += radiusDelta[i] * time_delta;

		x[i] = - cosf(angle[i]) * radius[i];
		y[i] = - sinf(angle[i]) * radius[i];
	}
}

static void
UpdateGravityParticles( float **f, int count, float time_delta, const b2Vec2 &gravity )
{
	float * Rtt_RESTRICT x = f[EmitterObjectParticles::kPositionX];
	float * Rtt_RESTRICT y = f[EmitterObjectParticles::kPositionY];
	float * Rtt_RESTRICT dx = f[EmitterObjectParticles::kDirectionX];
	float * Rtt_RESTRICT dy = f[EmitterObjectParticles::kDirectionY];
	const float * Rtt_RESTRICT radialAcceleration = f[EmitterObjectParticles::kRadialAcceleration];
	const float * Rtt_RESTRICT tangentialAcceleration = f[EmitterObjectParticles::kTangentialAcceleration];

	int i = 0;

#ifdef Rtt_EMITTER_SIMD
	V4 dt = Splat( time_delta ), gx = Splat( gravity.x ), gy = Splat( gravity.y );
	V4 zero = Splat( 0.0f ), one = Splat( 1.0f ), epsilon = Splat( b2_epsilon );

	for( ; i + 4 <= count; i += 4 )
	{
		V4 px = Load( x + i ), py = Load( y + i );

		// radial = position, normalized unless its length is below b2_epsilon
		// (see b2Vec2::Normalize()), or 0 if position is 0.
		V4 length = Sqrt( Add( Mul( px, px ), Mul( py, py ) ) );
		V4 invLength = Div( one, length );
		V4 isShort = Less( length, epsilon );
		V4 rx = Select( isShort, px, Mul( px, invLength ) );
		V4 ry = Select( isShort, py, Mul( py, invLength ) );
		V4 isNonZero = Or( NotEqual( px, zero ), NotEqual( py, zero ) );

		rx = And( isNonZero, rx );
		ry = And( isNonZero, ry );

		// tangential = perpendicular of radial.
		V4 ta = Load( tangentialAcceleration + i ), ra = Load( radialAcceleration + i );
		V4 tx = Mul( Neg( ry ), ta ), ty = Mul( rx, ta );

		rx = Mul( rx, ra );
		ry = Mul( ry, ra );

		V4 ddx = Mul( Add( Add( rx, tx ), gx ), dt );
		V4 ddy = Mul( Add( Add( ry, ty ), gy ), dt );
		V4 dirx = Add( Load( dx + i ), ddx ), diry = Add( Load( dy + i ), ddy );

		Store( dx + i, dirx );
		Store( dy + i, diry );
		Store( x + i, Add( px, Mul( dirx, dt ) ) );
		Store( y + i, Add( py, Mul( diry, dt ) ) );
	}
#endif

	for( ; i < count; ++i )
	{
		b2Vec2 position( x[i], y[i] ), direction( dx[i], dy[i] );
		b2Vec2 tmp, radial = b2Vec2_zero, tangential;

		if( position.x || position.y )
		{
			radial = position;
			radial.Normalize();
		}

		tangential = radial;
		radial *= radialAcceleration[i];

		float newy = tangential.x;
		tangential.x = ( - tangential.y );
		tangential.y = newy;
		tangential *= tangentialAcceleration[i];

		tmp = ( radial + tangential + gravity );
		tmp *= time_delta;
		direction += tmp;
		tmp = direction;
		tmp *= time_delta;
		position += tmp;

		x[i] = position.x;
		y[i] = position.y;
		dx[i] = direction.x;
		dy[i] = direction.y;
	}
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

// -----------------------------------------------------------------------------

// Everything the vertex pass needs that is the same for every particle in a frame.
struct EmitterVertexContext
{
	enum TransformMode
	{
		kTransform_SpawnTime, // Default: particles keep the transform they spawned with.
		kTransform_Current, // "absolutePosition" is true: particles follow the emitter.
		kTransform_Parent, // Particles keep their spawn transform within a parent group.
	};

	const Matrix *fCurrentTransform; // kTransform_Current: the emitter's; kTransform_Parent: the parent's.
	Real fColorScale[4]; // Applied to particle color: cumulative alpha, premultiplied if the texture is.
	EmitterObject::Mapping fMapping;
	bool fUsesIndexedQuads;
};

template < int kTransformMode >
static inline void
TransformParticlePoint( const EmitterVertexContext &context, const EmitterObjectParticles &particles, int index, Vertex2 &v )
{
	switch ( kTransformMode )
	{
	case EmitterVertexContext::kTransform_SpawnTime:
		particles.ApplySpawnTimeTransform( index, v );
		break;
	case EmitterVertexContext::kTransform_Current:
		context.fCurrentTransform->Apply( v );
		break;
	default:
		particles.ApplySpawnTimeTransform( index, v );
		context.fCurrentTransform->Apply( v );
		break;
	}
}

static inline void
SetVertex( Geometry::Vertex& output, const Vertex2& v, const U8 color[4] )
{
	output.x = v.x;
	output.y = v.y;
	output.rs = color[0];
	output.gs = color[1];
	output.bs = color[2];
	output.as = color[3];
}

static inline void
EmitRect( Geometry::Vertex *output_vertices, bool indexed, const Vertex2& topLeft, const Vertex2& topRight, const Vertex2& bottomLeft, const Vertex2& bottomRight, const U8 color[4] )
{
	if ( indexed )
	{
		// See the indices in EmitterObject::Initialize().
		SetVertex( output_vertices[0], topLeft, color );
		SetVertex( output_vertices[1], topRight, color );
		SetVertex( output_vertices[2], bottomLeft, color );
		SetVertex( output_vertices[3], bottomRight, color );
	}
	else
	{
		SetVertex( output_vertices[0], bottomLeft, color );
		SetVertex( output_vertices[1], topRight, color );
		SetVertex( output_vertices[2], topLeft, color );
		SetVertex( output_vertices[3], bottomLeft, color );
		SetVertex( output_vertices[4], bottomRight, color );
		SetVertex( output_vertices[5], topRight, color );
	}
}

static inline void
Legacy( Vertex2 corners[4], const Vertex2& base_position, float rotation, float halfSize )
{
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

		corners[0] = { ax, ay }; // Top left.
		corners[1] = { bx, by }; // Top right.
		corners[2] = { dx, dy }; // Bottom left.
		corners[3] = { cx, cy }; // Bottom right.
    }
	else
	{
        // Using the position of the particle, work out the four vertices for the quad that will hold the particle
        // and load those into the quads array.

		float xmin = ( base_position.x - halfSize );
		float ymin = ( base_position.y - halfSize );
		float xmax = ( base_position.x + halfSize );
		float ymax = ( base_position.y + halfSize );

		corners[0] = { xmin, ymin };
		corners[1] = { xmax, ymin };
		corners[2] = { xmin, ymax };
		corners[3] = { xmax, ymax };
	}
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

static void
Rescale( Vertex2 corners[4], EmitterObject::Mapping mapping, const Vertex2& base_position, const Vertex2& right, const Vertex2& below, float rotation, float halfSize )
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
				ScaleVertex( yDir, meanLength / yLength );
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

	corners[0] = MapPoint( base_position, xDir, yDir, -1.f, -1.f ); // Top left.
	corners[1] = MapPoint( base_position, xDir, yDir, +1.f, -1.f ); // Top right.
	corners[2] = MapPoint( base_position, xDir, yDir, -1.f, +1.f ); // Bottom left.
	corners[3] = MapPoint( base_position, xDir, yDir, +1.f, +1.f ); // Bottom right.
}

#ifdef Rtt_EMITTER_SIMD

// A transform for 4 points: either one matrix for all 4 or, for spawn-time
// transforms, one per particle. Lanes with an identity matrix are left alone.
struct TransformV4
{
	V4 a, b, c, d, e, f;
	V4 isIdentity;
	bool isUniformIdentity;
};

static inline TransformV4
LoadSpawnTimeTransforms( float **f, int index )
{
	TransformV4 t;

	t.a = Load( f[EmitterObjectParticles::kSpawnA] + index );
	t.b = Load( f[EmitterObjectParticles::kSpawnB] + index );
	t.c = Load( f[EmitterObjectParticles::kSpawnC] + index );
	t.d = Load( f[EmitterObjectParticles::kSpawnD] + index );
	t.e = Load( f[EmitterObjectParticles::kSpawnE] + index );
	t.f = Load( f[EmitterObjectParticles::kSpawnF] + index );
	t.isIdentity = Equal( t.a, Splat( Rtt_REAL_INF ) );
	t.isUniformIdentity = false;

	return t;
}

static inline TransformV4
SplatTransform( const Matrix &m )
{
	TransformV4 t;

	t.isUniformIdentity = m.IsIdentity();

	const Real *row0 = m.Row0(), *row1 = m.Row1();
	bool isIdentity = t.isUniformIdentity;

	t.a = Splat( isIdentity ? 0.0f : row0[0] );
	t.b = Splat( isIdentity ? 0.0f : row0[1] );
	t.c = Splat( isIdentity ? 0.0f : row0[2] );
	t.d = Splat( isIdentity ? 0.0f : row1[0] );
	t.e = Splat( isIdentity ? 0.0f : row1[1] );
	t.f = Splat( isIdentity ? 0.0f : row1[2] );
	t.isIdentity = Splat( 0.0f );

	return t;
}

// Same operations as Matrix::Apply().
static inline void
ApplyTransform( const TransformV4 &t, V4 &x, V4 &y )
{
	if ( !t.isUniformIdentity )
	{
		V4 tx = Add( Add( t.c, Mul( t.a, x ) ), Mul( t.b, y ) );
		V4 ty = Add( Add( t.f, Mul( t.d, x ) ), Mul( t.e, y ) );

		x = Select( t.isIdentity, x, tx );
		y = Select( t.isIdentity, y, ty );
	}
}

template < int kTransformMode >
static inline void
TransformParticlePoints( const TransformV4 &spawnTimeTransforms, const TransformV4 &current, V4 &x, V4 &y )
{
	if ( EmitterVertexContext::kTransform_Current != kTransformMode )
	{
		ApplyTransform( spawnTimeTransforms, x, y );
	}

	if ( EmitterVertexContext::kTransform_SpawnTime != kTransformMode )
	{
		ApplyTransform( current, x, y );
	}
}

// The cosine and sine of each lane's rotation, in degrees; lanes with no rotation
// get 1 and 0. Returns the mask of rotated lanes. (SinCos() is NOT bit-identical
// to the scalar code's cosf() / sinf().)
static inline V4
CosSinOfRotations( V4 rotation, V4 &cosines, V4 &sines )
{
	V4 isRotated = NotEqual( rotation, Splat( 0.0f ) );

	cosines = Splat( 1.0f );
	sines = Splat( 0.0f );

	if ( Any( isRotated ) )
	{
		V4 s, c;

		SinCos( Mul( rotation, Splat( (float)( M_PI / 180.0 ) ) ), s, c );

		cosines = Select( isRotated, c, cosines );
		sines = Select( isRotated, s, sines );
	}

	return isRotated;
}

// See Legacy(). Corners: top left, top right, bottom left, bottom right; x then y.
static inline void
LegacyCorners( V4 corners[8], V4 x, V4 y, V4 rotation, V4 halfSize )
{
	V4 cr, sr;
	V4 isRotated = CosSinOfRotations( rotation, cr, sr );

	V4 xmin = Sub( x, halfSize ), ymin = Sub( y, halfSize );
	V4 xmax = Add( x, halfSize ), ymax = Add( y, halfSize );

	corners[0] = xmin; corners[1] = ymin;
	corners[2] = xmax; corners[3] = ymin;
	corners[4] = xmin; corners[5] = ymax;
	corners[6] = xmax; corners[7] = ymax;

	if ( Any( isRotated ) )
	{
		V4 x1 = Neg( halfSize ), y1 = x1;
		V4 x2 = halfSize, y2 = halfSize;
		V4 ax = Add( Sub( Mul( x1, cr ), Mul( y1, sr ) ), x );
		V4 ay = Add( Add( Mul( x1, sr ), Mul( y1, cr ) ), y );
		V4 bx = Add( Sub( Mul( x2, cr ), Mul( y1, sr ) ), x );
		V4 by = Add( Add( Mul( x2, sr ), Mul( y1, cr ) ), y );
		V4 cx = Add( Sub( Mul( x2, cr ), Mul( y2, sr ) ), x );
		V4 cy = Add( Add( Mul( x2, sr ), Mul( y2, cr ) ), y );
		V4 dx = Add( Sub( Mul( x1, cr ), Mul( y2, sr ) ), x );
		V4 dy = Add( Add( Mul( x1, sr ), Mul( y2, cr ) ), y );

		corners[0] = Select( isRotated, ax, corners[0] ); corners[1] = Select( isRotated, ay, corners[1] );
		corners[2] = Select( isRotated, bx, corners[2] ); corners[3] = Select( isRotated, by, corners[3] );
		corners[4] = Select( isRotated, dx, corners[4] ); corners[5] = Select( isRotated, dy, corners[5] );
		corners[6] = Select( isRotated, cx, corners[6] ); corners[7] = Select( isRotated, cy, corners[7] );
	}
}

// See Rescale().
static inline void
RescaleCorners( V4 corners[8], EmitterObject::Mapping mapping, V4 x, V4 y, V4 rightX, V4 rightY, V4 belowX, V4 belowY, V4 rotation, V4 halfSize )
{
	V4 xDirX = Sub( rightX, x ), xDirY = Sub( rightY, y );
	V4 yDirX = Sub( belowX, x ), yDirY = Sub( belowY, y );

	switch ( mapping )
	{
	case EmitterObject::kMapping_Rescale:
		break;
	case EmitterObject::kMapping_RescaleX:
		yDirX = Neg( xDirY ); // PerpFromXDir()
		yDirY = xDirX;
		break;
	case EmitterObject::kMapping_RescaleY:
		xDirX = yDirY; // PerpFromYDir()
		xDirY = Neg( yDirX );
		break;
	default:
		{
			V4 xLength = Sqrt( Add( Mul( xDirX, xDirX ), Mul( xDirY, xDirY ) ) );
			V4 yLength = Sqrt( Add( Mul( yDirX, yDirX ), Mul( yDirY, yDirY ) ) );

			// Min or max (also mean's fallback): rebuild one axis from the other.
			V4 xIsShorter = Less( xLength, yLength );
			V4 fromXDir = xIsShorter;

			if ( EmitterObject::kMapping_RescaleMin != mapping )
			{
				V4 zero = Splat( 0.0f );

				fromXDir = AndNot( Equal( zero, zero ), xIsShorter ); // !xIsShorter
			}

			V4 newXDirX = Select( fromXDir, xDirX, yDirY ), newXDirY = Select( fromXDir, xDirY, Neg( yDirX ) );
			V4 newYDirX = Select( fromXDir, Neg( xDirY ), yDirX ), newYDirY = Select( fromXDir, xDirX, yDirY );

			if ( EmitterObject::kMapping_RescaleMean == mapping )
			{
				float xl[4], yl[4];

				Store( xl, xLength );
				Store( yl, yLength );

				V4 useMean = MaskFromBools( !Rtt_RealIsZero( xl[0] ) && !Rtt_RealIsZero( yl[0] ),
											!Rtt_RealIsZero( xl[1] ) && !Rtt_RealIsZero( yl[1] ),
											!Rtt_RealIsZero( xl[2] ) && !Rtt_RealIsZero( yl[2] ),
											!Rtt_RealIsZero( xl[3] ) && !Rtt_RealIsZero( yl[3] ) );

				if ( Any( useMean ) )
				{
					V4 meanLength = Mul( Add( xLength, yLength ), Splat( Rtt_REAL_HALF ) ); // Rtt_RealDiv2()
					V4 xScale = Div( meanLength, xLength ), yScale = Div( meanLength, yLength );

					newXDirX = Select( useMean, Mul( xDirX, xScale ), newXDirX );
					newXDirY = Select( useMean, Mul( xDirY, xScale ), newXDirY );
					newYDirX = Select( useMean, Mul( yDirX, yScale ), newYDirX );
					newYDirY = Select( useMean, Mul( yDirY, yScale ), newYDirY );
				}
			}

			xDirX = newXDirX; xDirY = newXDirY;
			yDirX = newYDirX; yDirY = newYDirY;
		}
	}

	V4 cr, sr;
	V4 isRotated = CosSinOfRotations( rotation, cr, sr );

	if ( Any( isRotated ) )
	{
		// RotatePoint()
		V4 rxx = Sub( Mul( xDirX, cr ), Mul( xDirY, sr ) ), rxy = Add( Mul( xDirY, cr ), Mul( xDirX, sr ) );
		V4 ryx = Sub( Mul( yDirX, cr ), Mul( yDirY, sr ) ), ryy = Add( Mul( yDirY, cr ), Mul( yDirX, sr ) );

		xDirX = Select( isRotated, rxx, xDirX ); xDirY = Select( isRotated, rxy, xDirY );
		yDirX = Select( isRotated, ryx, yDirX ); yDirY = Select( isRotated, ryy, yDirY );
	}

	xDirX = Mul( xDirX, halfSize ); xDirY = Mul( xDirY, halfSize );
	yDirX = Mul( yDirX, halfSize ); yDirY = Mul( yDirY, halfSize );

	// MapPoint() with ( -1, -1 ), ( +1, -1 ), ( -1, +1 ), ( +1, +1 ).
	V4 plus = Splat( +1.f ), minus = Splat( -1.f );

	corners[0] = Add( x, Add( Mul( xDirX, minus ), Mul( yDirX, minus ) ) ); corners[1] = Add( y, Add( Mul( xDirY, minus ), Mul( yDirY, minus ) ) );
	corners[2] = Add( x, Add( Mul( xDirX, plus ), Mul( yDirX, minus ) ) ); corners[3] = Add( y, Add( Mul( xDirY, plus ), Mul( yDirY, minus ) ) );
	corners[4] = Add( x, Add( Mul( xDirX, minus ), Mul( yDirX, plus ) ) ); corners[5] = Add( y, Add( Mul( xDirY, minus ), Mul( yDirY, plus ) ) );
	corners[6] = Add( x, Add( Mul( xDirX, plus ), Mul( yDirX, plus ) ) ); corners[7] = Add( y, Add( Mul( xDirY, plus ), Mul( yDirY, plus ) ) );
}

#endif // Rtt_EMITTER_SIMD

// Writes the vertices of particles [0, count). The transform mode and the
// legacy / rescale choice are fixed for the whole loop.
template < int kTransformMode, bool kIsLegacy >
static void
WriteParticleVertices( const EmitterVertexContext &context, EmitterObjectParticles &particles, int count, Geometry::Vertex *output_vertices )
{
	float **f = particles.fFields;
	const float * Rtt_RESTRICT px = f[EmitterObjectParticles::kPositionX];
	const float * Rtt_RESTRICT py = f[EmitterObjectParticles::kPositionY];
	const float * Rtt_RESTRICT size = f[EmitterObjectParticles::kParticleSize];
	const float * Rtt_RESTRICT rotation = f[EmitterObjectParticles::kRotation];
	const float * Rtt_RESTRICT r = f[EmitterObjectParticles::kColorR];
	const float * Rtt_RESTRICT g = f[EmitterObjectParticles::kColorG];
	const float * Rtt_RESTRICT b = f[EmitterObjectParticles::kColorB];
	const float * Rtt_RESTRICT a = f[EmitterObjectParticles::kColorA];

	const bool indexed = context.fUsesIndexedQuads;
	const int verticesPerParticle = indexed ? 4 : VERTICES_PER_QUADS;
	const EmitterObject::Mapping mapping = context.fMapping;

	int i = 0;

#ifdef Rtt_EMITTER_SIMD
	TransformV4 current = SplatTransform( EmitterVertexContext::kTransform_SpawnTime != kTransformMode ? *context.fCurrentTransform : Matrix() );

	V4 half = Splat( 0.5f ), one = Splat( 1.0f ), maxByte = Splat( 255.0f );
	V4 colorScaleR = Splat( context.fColorScale[0] ), colorScaleG = Splat( context.fColorScale[1] );
	V4 colorScaleB = Splat( context.fColorScale[2] ), colorScaleA = Splat( context.fColorScale[3] );

	for( ; i + 4 <= count; i += 4 )
	{
		TransformV4 spawnTimeTransforms = LoadSpawnTimeTransforms( f, i );

		V4 halfSize = Mul( Load( size + i ), half );
		V4 x = Load( px + i ), y = Load( py + i );
		V4 rightX = Add( x, one ), rightY = y;
		V4 belowX = x, belowY = Add( y, one );

		TransformParticlePoints< kTransformMode >( spawnTimeTransforms, current, x, y );

		uint32_t colors[4];

		PackBytes( colors,
					Mul( Mul( Load( r + i ), colorScaleR ), maxByte ),
					Mul( Mul( Load( g + i ), colorScaleG ), maxByte ),
					Mul( Mul( Load( b + i ), colorScaleB ), maxByte ),
					Mul( Mul( Load( a + i ), colorScaleA ), maxByte ) );

		V4 corners[8];

		if ( kIsLegacy )
		{
			LegacyCorners( corners, x, y, Load( rotation + i ), halfSize );
		}
		else
		{
			if ( EmitterObject::kMapping_RescaleY != mapping )
			{
				TransformParticlePoints< kTransformMode >( spawnTimeTransforms, current, rightX, rightY );
			}

			if ( EmitterObject::kMapping_RescaleX != mapping )
			{
				TransformParticlePoints< kTransformMode >( spawnTimeTransforms, current, belowX, belowY );
			}

			RescaleCorners( corners, mapping, x, y, rightX, rightY, belowX, belowY, Load( rotation + i ), halfSize );
		}

		float lanes[8][4];

		for ( int k = 0; k < 8; ++k )
		{
			Store( lanes[k], corners[k] );
		}

		for ( int j = 0; j < 4; ++j, output_vertices += verticesPerParticle )
		{
			Vertex2 topLeft = { lanes[0][j], lanes[1][j] }, topRight = { lanes[2][j], lanes[3][j] };
			Vertex2 bottomLeft = { lanes[4][j], lanes[5][j] }, bottomRight = { lanes[6][j], lanes[7][j] };
			U8 color[4];

			memcpy( color, &colors[j], sizeof( color ) ); // Bytes r, g, b, a, in order.

			EmitRect( output_vertices, indexed, topLeft, topRight, bottomLeft, bottomRight, color );
		}
	}
#endif

	for( ; i < count; ++i, output_vertices += verticesPerParticle )
	{
		// As we are rendering the particles as quads, we need to define 4 corners for each particle
		// We assume that all particles have a square aspect ratio.
		float halfSize = ( size[i] * 0.5f );

		//// Base position.
		//
		Vertex2 base_position = { px[i], py[i] };
		Vertex2 right = { base_position.x + 1, base_position.y };
		Vertex2 below = { base_position.x, base_position.y + 1 };

		TransformParticlePoint< kTransformMode >( context, particles, i, base_position );

		//// Color.
		//
		U8 color[4] = {
			(U8)( ( r[i] * context.fColorScale[0] ) * 255.0f ),
			(U8)( ( g[i] * context.fColorScale[1] ) * 255.0f ),
			(U8)( ( b[i] * context.fColorScale[2] ) * 255.0f ),
			(U8)( ( a[i] * context.fColorScale[3] ) * 255.0f )
		};

		Vertex2 corners[4];

		if ( kIsLegacy )
		{
			Legacy( corners, base_position, rotation[i], halfSize );
		}
		else
		{
			if ( EmitterObject::kMapping_RescaleY != mapping )
			{
				TransformParticlePoint< kTransformMode >( context, particles, i, right );
			}

			if ( EmitterObject::kMapping_RescaleX != mapping )
			{
				TransformParticlePoint< kTransformMode >( context, particles, i, below );
			}

			Rescale( corners, mapping, base_position, right, below, rotation[i], halfSize );
		}

		EmitRect( output_vertices, indexed, corners[0], corners[1], corners[2], corners[3], color );
	}
}

template < bool kIsLegacy >
static void
WriteParticleVertices( int transformMode, const EmitterVertexContext &context, EmitterObjectParticles &particles, int count, Geometry::Vertex *output_vertices )
{
	switch ( transformMode )
	{
	case EmitterVertexContext::kTransform_SpawnTime:
		WriteParticleVertices< EmitterVertexContext::kTransform_SpawnTime, kIsLegacy >( context, particles, count, output_vertices );
		break;
	case EmitterVertexContext::kTransform_Current:
		WriteParticleVertices< EmitterVertexContext::kTransform_Current, kIsLegacy >( context, particles, count, output_vertices );
		break;
	default:
		WriteParticleVertices< EmitterVertexContext::kTransform_Parent, kIsLegacy >( context, particles, count, output_vertices );
		break;
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
, fUsesIndexedQuads( false )
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
		EmitterObjectParticles::Destroy( fParticles );

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

	fParticles = EmitterObjectParticles::Create( display.GetAllocator(), fMaxParticles );

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
		// Each particle is a quad. If the indices fit, store 4 vertices per quad
		// on the GPU and draw them with static indices; otherwise, fall back to
		// 6 vertices (2 triangles) per quad.
		//
		// The vertices are re-sent every frame. That is cheap on GL, but on Vulkan
		// each update of GPU-stored geometry is a blocking transfer, so use the
		// renderer's own (batched) vertex streaming there instead.
		const U32 kMaxIndexedVertices = (U32)std::numeric_limits< Geometry::Index >::max() + 1;
		bool updatesGPUGeometryCheaply = 0 == strcmp( display.GetRuntime().GetBackend(), "glBackend" );

		fUsesIndexedQuads = updatesGPUGeometryCheaply && (U32)fMaxParticles * 4 <= kMaxIndexedVertices;

		if ( fUsesIndexedQuads )
		{
			fData.fGeometry = Rtt_NEW( display.GetAllocator(),
										Geometry( display.GetAllocator(),
													Geometry::kIndexedTriangles,
													0, // Vertex count.
													0, // Index count.
													true ) ); // Store on GPU.
		}
		else
		{
			fData.fGeometry = Rtt_NEW( display.GetAllocator(),
										Geometry( display.GetAllocator(),
													Geometry::kTriangles,
													0, // Vertex count.
													0, // Index count.
													false ) ); // Store on GPU.
		}

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

		// Set every member of Vertex that NEVER changes, and the indices.
		Geometry::Vertex blank;

		blank.Zero();

		if ( fUsesIndexedQuads )
		{
			fData.fGeometry->Resize( fMaxParticles * 4, fMaxParticles * 6, false );

			Geometry::Vertex *output_vertices = fData.fGeometry->GetVertexData();
			Geometry::Index *indices = fData.fGeometry->GetIndexData();

			for( int i = 0;
					i < fMaxParticles;
					++i, output_vertices += 4, indices += 6 )
			{
				// Top left, top right, bottom left, bottom right.
				for ( int j = 0; j < 4; ++j )
				{
					output_vertices[j] = blank;
					output_vertices[j].u = (Real)( j & 1 );
					output_vertices[j].v = (Real)( j >> 1 );
				}

				// Same triangles, in the same order, as the 6-vertex form:
				// (bottom left, top right, top left), (bottom left, bottom right, top right).
				Geometry::Index base = (Geometry::Index)( i * 4 );

				indices[0] = base + 2;
				indices[1] = base + 1;
				indices[2] = base + 0;
				indices[3] = base + 2;
				indices[4] = base + 3;
				indices[5] = base + 1;
			}
		}
		else
		{
			fData.fGeometry->Resize( ( fMaxParticles * VERTICES_PER_QUADS ),
										false );

			Geometry::Vertex *output_vertices = fData.fGeometry->GetVertexData();

			// Bottom left, top right, top left, bottom left, bottom right, top right.
			static const Real kU[] = { 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f };
			static const Real kV[] = { 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f };

			for( int i = 0;
					i < fMaxParticles;
					++i, output_vertices += VERTICES_PER_QUADS )
			{
				for ( int j = 0; j < VERTICES_PER_QUADS; ++j )
				{
					output_vertices[j] = blank;
					output_vertices[j].u = kU[j];
					output_vertices[j].v = kV[j];
				}
			}
		}

		_SetParticleCount( fParticleCount );
	}

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
	fParticles->Init( fParticleCount, this, spawnTimeTransform );

	// Increment the particle count
	_SetParticleCount( fParticleCount + 1 );
}

void EmitterObject::_SetParticleCount( int count )
{
	fParticleCount = count;

	if ( fUsesIndexedQuads )
	{
		fData.fGeometry->SetVerticesUsed( fParticleCount * 4 );
		fData.fGeometry->SetIndicesUsed( fParticleCount * 6 );
	}
	else
	{
		fData.fGeometry->SetVerticesUsed( fParticleCount * VERTICES_PER_QUADS );
	}
}

void EmitterObject::_UpdateParticles( float time_delta )
{
	float **f = fParticles->fFields;
	float * Rtt_RESTRICT timeToLive = f[EmitterObjectParticles::kTimeToLiveInSeconds];
	int count = fParticleCount;

	// Reduce the life span of every particle.
	for( int i = 0; i < count; ++i )
	{
		timeToLive[i] -= time_delta;
	}

	// Remove dead particles, moving the last live particle into each hole. Live
	// particles stay packed together at the start of the arrays. (This leaves
	// them in the same order as removing each dead one as it is found.)
	int hole = 0, tail = count;

	for( ;; )
	{
		while( hole < tail && timeToLive[hole] > 0.0f )
		{
			++hole;
		}

		while( hole < tail && !( timeToLive[tail - 1] > 0.0f ) )
		{
			--tail;
		}

		if( hole >= tail )
		{
			break;
		}

		fParticles->Move( hole++, --tail );
	}

	if( tail != count )
	{
		_SetParticleCount( tail );
	}

	// Update the survivors.
	if( fEmitterType == kParticleTypeRadial )
	{
		UpdateRadialParticles( f, fParticleCount, time_delta );
	}
	else
	{
		UpdateGravityParticles( f, fParticleCount, time_delta, fGravity );
	}

	UpdateCommonParticleState( f, fParticleCount, time_delta );
}

void EmitterObject::_UpdateVertices()
{
	EmitterVertexContext context;
	int transformMode;

	if( fAbsolutePositionParent == EMITTER_ABSOLUTE_PARENT )
	{
		transformMode = EmitterVertexContext::kTransform_SpawnTime;
		context.fCurrentTransform = NULL;
	}
	else if( fAbsolutePositionParent == NULL )
	{
		transformMode = EmitterVertexContext::kTransform_Current;
		context.fCurrentTransform = &GetSrcToDstMatrix();
	}
	else
	{
		transformMode = EmitterVertexContext::kTransform_Parent;
		context.fCurrentTransform = &fAbsolutePositionParent->GetSrcToDstMatrix();
	}

	float cumulative_alpha = ( (float)AlphaCumulative() * ( 1.0f / 255.0f ) );
	bool premultiplied = fTextureResource->GetBitmap()->IsPremultiplied();

	context.fColorScale[0] = premultiplied ? cumulative_alpha : 1.0f;
	context.fColorScale[1] = context.fColorScale[0];
	context.fColorScale[2] = context.fColorScale[0];
	context.fColorScale[3] = cumulative_alpha;
	context.fMapping = fMapping;
	context.fUsesIndexedQuads = fUsesIndexedQuads;

	Geometry::Vertex *output_vertices = fData.fGeometry->GetVertexData();

	if ( kMapping_Legacy == fMapping )
	{
		WriteParticleVertices< true >( transformMode, context, *fParticles, fParticleCount, output_vertices );
	}
	else
	{
		WriteParticleVertices< false >( transformMode, context, *fParticles, fParticleCount, output_vertices );
	}

	if ( fData.fGeometry->GetStoredOnGPU() )
	{
		fData.fGeometry->Invalidate();
	}
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

	_UpdateParticles( time_delta );
	_UpdateVertices();
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
		float *timeToLive = fParticles->fFields[EmitterObjectParticles::kTimeToLiveInSeconds];

		for( int i = 0;
				i < fParticleCount;
				++i )
		{
			timeToLive[i] = 0.0f;
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
		case kMapping_Rescale:
			return "rescale";
		case kMapping_RescaleX:
			return "rescaleX";
		case kMapping_RescaleY:
			return "rescaleY";
		case kMapping_RescaleMin:
			return "rescaleMin";
		case kMapping_RescaleMax:
			return "rescaleMax";
		case kMapping_RescaleMean:
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
