//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Real_H__
#define _Rtt_Real_H__

// ----------------------------------------------------------------------------

#include "Core/Rtt_Config.h"
#include "Core/Rtt_Macros.h"
#include "Core/Rtt_Fixed.h"

#if defined( Rtt_WIN_ENV ) || defined( Rtt_POWERVR_ENV ) || defined( Rtt_NINTENDO_ENV )
#define M_PI 3.1415926535897932384626f
#endif

// ----------------------------------------------------------------------------

Rtt_FORCE_INLINE float Rtt_FloatMul( float a, float b ) { return a * b; }

// ----------------------------------------------------------------------------

#if defined( Rtt_REAL_FIXED )

	typedef Rtt_Fixed Rtt_Real;

	#define Rtt_REAL_0			Rtt_FIXED_0
	#define Rtt_REAL_1			Rtt_FIXED_1
	#define Rtt_REAL_2			Rtt_FIXED_2
	#define Rtt_REAL_NEG_1		Rtt_FIXED_NEG_1
	#define Rtt_REAL_HALF		Rtt_FIXED_HALF
	#define Rtt_REAL_FOURTH		Rtt_FIXED_FOURTH
	#define Rtt_REAL_EIGHTH		Rtt_FIXED_EIGHTH
	#define Rtt_REAL_16TH		Rtt_FIXED_16TH
	#define Rtt_REAL_32ND		Rtt_FIXED_32ND
	#define Rtt_REAL_SQRT_2		Rtt_FIXED_SQRT_2
	#define Rtt_REAL_INF		Rtt_FIXED_INF

	#define Rtt_RealMul			Rtt_FixedMul
	#define Rtt_RealDiv			Rtt_FixedDiv
	#define Rtt_RealAvg			Rtt_FixedAvg
	#define Rtt_RealMul2		Rtt_FixedMul2
	#define Rtt_RealDiv2		Rtt_FixedDiv2
	#define Rtt_RealDiv4		Rtt_FixedDiv4
	#define Rtt_RealDiv8		Rtt_FixedDiv8
	#define Rtt_RealDiv16		Rtt_FixedDiv16

	#define Rtt_RealDivNonZeroB		Rtt_FixedDivNonZeroB
	#define Rtt_RealDivNonZeroAB	Rtt_FixedDivNonZeroAB

	#define Rtt_RealAbs			Rtt_FixedAbs

	#define Rtt_RealSqrt		Rtt_FixedSqrt
	#define Rtt_RealLog2		Rtt_FixedLog2
	#define Rtt_RealCos			Rtt_FixedCos
	#define Rtt_RealSin			Rtt_FixedSin
	#define Rtt_RealTan			Rtt_FixedTan

	#define Rtt_IntToReal		Rtt_IntToFixed
	#define Rtt_RealToInt		Rtt_FixedToInt
	#define Rtt_FloatToReal		Rtt_FloatToFixed
	#define Rtt_RealToFloat		Rtt_FixedToFloat

	#define Rtt_RealEqualEps( a, b, eps )	(Rtt_FixedAbs( (a) - (b) ) < (eps))
	#define Rtt_RealEqual( a, b )	( (a) == (b) )

#else

	#ifdef Rtt_USE_LIMITS
		#include <limits>
	#endif

	#include <math.h>

	typedef float Rtt_Real;

	#define Rtt_REAL_0			((Rtt_Real)0.f)
	#define Rtt_REAL_1			((Rtt_Real)1.f)
	#define Rtt_REAL_2			((Rtt_Real)2.f)
	#define Rtt_REAL_NEG_1		((Rtt_Real)-1.f)
	#define Rtt_REAL_HALF		((Rtt_Real)0.5f)
	#define Rtt_REAL_FOURTH		((Rtt_Real)0.25f)
	#define Rtt_REAL_EIGHTH		((Rtt_Real)0.125f)
	#define Rtt_REAL_16TH		((Rtt_Real)0.0625f)
	#define Rtt_REAL_32ND		((Rtt_Real)0.03125f)
	#define Rtt_REAL_SQRT_2		((Rtt_Real)1.4142135623731f)
	#ifdef Rtt_USE_LIMITS
		#define Rtt_REAL_INF	((Rtt_Real)std::numeric_limits<float>::infinity())
	#else
		#if defined( Rtt_ANDROID_ENV )
			// TODO: gack
			#define Rtt_REAL_INF	((Rtt_Real)INFINITY)
		#else
			#define Rtt_REAL_INF	((Rtt_Real)infinityf())
		#endif
	#endif

	//#undef Rtt_INLINE
	//#undef Rtt_FORCE_INLINE
	//#define Rtt_INLINE inline
	//#define Rtt_FORCE_INLINE Rtt_INLINE

	#define Rtt_RealMul			Rtt_FloatMul
	Rtt_INLINE Rtt_Real Rtt_RealDiv( Rtt_Real a, Rtt_Real b )	{ return a / b; }
	Rtt_INLINE Rtt_Real Rtt_RealAvg( Rtt_Real a, Rtt_Real b )	{ return (a + b) * Rtt_REAL_HALF; }
	Rtt_INLINE Rtt_Real Rtt_RealMul2( Rtt_Real a )			{ return a * Rtt_REAL_2; }
	Rtt_INLINE Rtt_Real Rtt_RealDiv2( Rtt_Real a )			{ return a * Rtt_REAL_HALF; }
	Rtt_INLINE Rtt_Real Rtt_RealDiv4( Rtt_Real a )			{ return a * Rtt_REAL_FOURTH; }
	Rtt_INLINE Rtt_Real Rtt_RealDiv8( Rtt_Real a )			{ return a * Rtt_REAL_EIGHTH; }
	Rtt_INLINE Rtt_Real Rtt_RealDiv16( Rtt_Real a )			{ return a * Rtt_REAL_16TH; }

	#define Rtt_RealDivNonZeroB		Rtt_RealDiv
	#define Rtt_RealDivNonZeroAB	Rtt_RealDiv

	Rtt_FORCE_INLINE Rtt_Real Rtt_RealAbs( Rtt_Real a ) { return fabsf( a ); }

	Rtt_FORCE_INLINE Rtt_Real Rtt_RealSqrt( Rtt_Real a ) { return sqrtf( a ); }
	Rtt_FORCE_INLINE Rtt_Real Rtt_RealLog2( Rtt_Real a ) { 
#if defined( Rtt_ANDROID_ENV ) || defined ( Rtt_WIN_ENV ) || defined( Rtt_POWERVR_ENV )
		return logf( a ) / logf( 2 ); 
#else
		return log2f( a ); 
#endif
	}
	Rtt_FORCE_INLINE Rtt_Real Rtt_RealCos( Rtt_Real a ) { return cosf( a ); }
	Rtt_FORCE_INLINE Rtt_Real Rtt_RealSin( Rtt_Real a ) { return sinf( a ); }
	Rtt_FORCE_INLINE Rtt_Real Rtt_RealTan( Rtt_Real a ) { return tanf( a ); }

	Rtt_FORCE_INLINE Rtt_Real Rtt_IntToReal( S32 a )		{ return (Rtt_Real)a; }
	Rtt_FORCE_INLINE S32      Rtt_RealToInt( Rtt_Real a )	{ return (S32)a; }
	Rtt_FORCE_INLINE Rtt_Real Rtt_FloatToReal( float a )	{ return a; }
	Rtt_FORCE_INLINE float    Rtt_RealToFloat( Rtt_Real a )	{ return a; }

	Rtt_INLINE Rtt_Real Rtt_RealDegreesToRadians( Rtt_Real degrees ) { return degrees * M_PI / 180.0f; }
	Rtt_INLINE Rtt_Real Rtt_RealRadiansToDegrees( Rtt_Real radians ) { return radians * 180.0f / M_PI; }

	#define Rtt_RealEqualEps( a, b, eps )	(fabsf( (a) - (b) ) < (eps))
#ifdef Rtt_USE_LIMITS
	#define Rtt_RealEqual( a, b )			Rtt_RealEqualEps( (a), (b), std::numeric_limits<float>::epsilon() )
#else
	// TODO: gack
	// Walter says use FLT_EPS from limits.h or float.h
	#define Rtt_RealEqual( a, b )			Rtt_RealEqualEps( (a), (b), .0000001 )
#endif

	//#undef Rtt_INLINE
	//#undef Rtt_FORCE_INLINE
	//#define Rtt_INLINE 
	//#define Rtt_FORCE_INLINE Rtt_INLINE

#endif // Rtt_REAL_FIXED

#define Rtt_RealIsZero( n )		Rtt_RealEqual( (n), Rtt_REAL_0 )
#define Rtt_RealIsOne( n )		Rtt_RealEqual( (n), Rtt_REAL_1 )

// ----------------------------------------------------------------------------

#ifdef __cplusplus

namespace Rtt
{

typedef Rtt_Real Real;

}

#endif // __cplusplus
	
// ----------------------------------------------------------------------------

#endif // _Rtt_Real_H__
