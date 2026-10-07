//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_Math_H__
#define __Rtt_Math_H__

// ----------------------------------------------------------------------------

#ifdef Rtt_NO_STRICT_ANSI
	#undef _STRICT_ANSI
#endif
#include <math.h>

// ----------------------------------------------------------------------------

Rtt_EXPORT U32 NextPowerOf2( U32 x );

// This function DOESN'T consider zero to be a power of 2.
Rtt_EXPORT U32 IsPowerOf2( U32 value );

Rtt_EXPORT U32 IsAlignedToPowerOf2( U32 value, U32 alignment );
Rtt_EXPORT U32 AlignToPowerOf2( U32 value, U32 alignment );

// ----------------------------------------------------------------------------

#ifdef __cplusplus

namespace Rtt
{

// ----------------------------------------------------------------------------

#ifdef Rtt_SYMBIAN_ENV
	#define Rtt_TYPES_INLINE	Rtt_INLINE
#else
	#define Rtt_TYPES_INLINE	Rtt_FORCE_INLINE
#endif

template < typename T > Rtt_TYPES_INLINE T Max( T a, T b ) { return a > b ? a : b; }
template < typename T > Rtt_TYPES_INLINE T Min( T a, T b ) { return a < b ? a : b; }
template < typename T > Rtt_TYPES_INLINE void Swap( T& a, T& b ) { T c(a); a = b; b = c; }

template < typename T > Rtt_TYPES_INLINE T Clamp( T value, T min, T max )
{
	value = Max( value, min );
	value = Min( value, max );
	return value;
}

template < typename T > Rtt_TYPES_INLINE T Abs( T a ) { return a >= 0 ? a : -a; }

template < > Rtt_TYPES_INLINE float Abs< float >( float a ) { return fabsf( a ); }
template < > Rtt_TYPES_INLINE double Abs< double >( double a ) { return fabs( a ); }

#if !defined( Rtt_NO_LONG_DOUBLE )
template < > Rtt_TYPES_INLINE long double Abs< long double >( long double a ) { return fabsl( a ); }
#endif

// ----------------------------------------------------------------------------

#if defined( Rtt_WIN_ENV )
    Rtt_INLINE unsigned int Rtt_Pop16( U16 v )
    {
		v -= ( ( v >> 1 ) & 0x5555 );

		v = ( ( v >> 2 ) & 0x3333 ) + ( v & 0x3333 );
		v = ( ( v >> 4 ) + v ) & 0x0F0F;
		
		return ( ( v >> 8 ) + v ) & 0x00FF;
    }
	
    Rtt_INLINE unsigned int Rtt_Pop32( U32 v )
    {
        v -= ( v >> 1 ) & 0x55555555;

        v = ( v & 0x33333333 ) + ( ( v >> 2 ) & 0x33333333 );

        return ( ( v + ( v >> 4 ) & 0x0F0F0F0F ) * 0x01010101 ) >> 24;
    }

    Rtt_INLINE unsigned int Rtt_Pop64( U64 v )
    {
        v -= ( v >> 1 ) & 0x5555555555555555ULL;

        v = ( v & 0x3333333333333333ULL ) + ( ( v >> 2 ) & 0x3333333333333333ULL );
        v = ( v + ( v >> 4 ) ) & 0x0F0F0F0F0F0F0F0FULL;

        return ( v * 0x0101010101010101ULL ) >> 56;
    }
#else
	#define Rtt_Pop16 __builtin_popcount
	#define Rtt_Pop32 __builtin_popcount
	#define Rtt_Pop64 __builtin_popcountll
#endif

// ----------------------------------------------------------------------------

} // Rtt

#endif // __cplusplus

// ----------------------------------------------------------------------------

#endif // __Rtt_Math_H__
