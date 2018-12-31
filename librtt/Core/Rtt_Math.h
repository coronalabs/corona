//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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

} // Rtt

#endif // __cplusplus

// ----------------------------------------------------------------------------

#endif // __Rtt_Math_H__
