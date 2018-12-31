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

#ifndef _Rtt_Fixed_H__
#define _Rtt_Fixed_H__

// Some trickiness here to allow both inline and non-inlining.
// 
// To inline, define Rtt_MATH_INLINE to 1 in Rtt_Build.h. This assumes you
// include Rtt_Build.h *before* this header. In this case, when the compiler
// compiles the source file explicitly, it will "see" an empty file.
// 
// Otherwise, do nothing, i.e. do not define Rtt_MATH_INLINE. This header 
// only includes the source file (to pull the function definitions with it)
// when Rtt_MATH_INLINE is defined. And in this case, the source file also 
// includes a test to ensure that the function definitions are only compiled once.

#if defined( Rtt_MATH_INLINE )
	#define Rtt_FIXED_INLINE Rtt_FORCE_INLINE
#else
	#define Rtt_FIXED_INLINE
#endif

// ----------------------------------------------------------------------------

Rtt_EXPORT_BEGIN

// ----------------------------------------------------------------------------

// Rtt_Fixed is a 16.16 fixed point signed number
typedef S32 Rtt_Fixed;

#define Rtt_FIXED_0				((Rtt_Fixed)0x00000000L)
#define Rtt_FIXED_1				((Rtt_Fixed)0x00010000L)
#define Rtt_FIXED_2				((Rtt_Fixed)0x00020000L)
#define Rtt_FIXED_NEG_1			(-Rtt_FIXED_1)
#define Rtt_FIXED_HALF			((Rtt_Fixed)0x00008000L)
#define Rtt_FIXED_FOURTH		((Rtt_Fixed)0x00004000L)
#define Rtt_FIXED_EIGHTH		((Rtt_Fixed)0x00002000L)
#define Rtt_FIXED_16TH			((Rtt_Fixed)0x00001000L)
#define Rtt_FIXED_32ND			((Rtt_Fixed)0x00000800L)
#define Rtt_FIXED_INF			((Rtt_Fixed)0x7FFFFFFFL)
#define Rtt_FIXED_NEG_INF		((Rtt_Fixed)0x80000000L)
#define Rtt_FIXED_SQRT_2		((Rtt_Fixed)0x00016A0AL)
#define Rtt_FIXED_EPS			((Rtt_Fixed)0x1L)

// ----------------------------------------------------------------------------

Rtt_FIXED_INLINE Rtt_Fixed Rtt_IntToFixed( S32 a );
Rtt_FIXED_INLINE S32 Rtt_FixedToInt( Rtt_Fixed a );
Rtt_FIXED_INLINE Rtt_Fixed Rtt_FloatToFixed( float a );
Rtt_FIXED_INLINE float Rtt_FixedToFloat( Rtt_Fixed a );
Rtt_FIXED_INLINE Rtt_Fixed Rtt_FixedMidpoint( Rtt_Fixed a, Rtt_Fixed b );
Rtt_FIXED_INLINE Rtt_Fixed Rtt_FixedMul2( Rtt_Fixed a );
Rtt_FIXED_INLINE Rtt_Fixed Rtt_FixedDiv2( Rtt_Fixed a );
Rtt_FIXED_INLINE Rtt_Fixed Rtt_FixedDiv4( Rtt_Fixed a );
Rtt_FIXED_INLINE Rtt_Fixed Rtt_FixedDiv8( Rtt_Fixed a );
Rtt_FIXED_INLINE Rtt_Fixed Rtt_FixedDiv16( Rtt_Fixed a );

Rtt_FIXED_INLINE Rtt_Fixed Rtt_FixedAvg( Rtt_Fixed a, Rtt_Fixed b );
Rtt_FIXED_INLINE Rtt_Fixed Rtt_FixedAbs( Rtt_Fixed a );
Rtt_FIXED_INLINE Rtt_Fixed Rtt_FixedMin( Rtt_Fixed a, Rtt_Fixed b );
Rtt_FIXED_INLINE Rtt_Fixed Rtt_FixedMax( Rtt_Fixed a, Rtt_Fixed b );
Rtt_FIXED_INLINE Rtt_Fixed Rtt_FixedMul( Rtt_Fixed a, Rtt_Fixed b );
Rtt_FIXED_INLINE Rtt_Fixed Rtt_FixedDiv( Rtt_Fixed a, Rtt_Fixed b );

Rtt_FIXED_INLINE Rtt_Fixed Rtt_FixedDivNonZeroB( Rtt_Fixed a, Rtt_Fixed b );
Rtt_FIXED_INLINE Rtt_Fixed Rtt_FixedDivNonZeroAB( Rtt_Fixed a, Rtt_Fixed b );

float Rtt_FloatSqrt( float a );
Rtt_Fixed Rtt_FixedSqrt( Rtt_Fixed a );
Rtt_Fixed Rtt_FixedLog2( Rtt_Fixed a ); 
Rtt_Fixed Rtt_FixedCos( Rtt_Fixed a );
Rtt_Fixed Rtt_FixedSin( Rtt_Fixed a );
Rtt_Fixed Rtt_FixedTan( Rtt_Fixed a );


// ----------------------------------------------------------------------------

Rtt_EXPORT_END

// ----------------------------------------------------------------------------

// When inlining, include function definitions
#ifdef Rtt_MATH_INLINE
	#include "Core/Rtt_Fixed.c"
#endif

/*
#ifdef __cplusplus

namespace Rtt
{

typedef ::Rtt_Fixed Fixed;

Rtt_FORCE_INLINE Rtt_Fixed	IntToFixed( S32 a )				{ return Rtt_IntToFixed( a ); }
Rtt_FORCE_INLINE S32		FixedToInt( Rtt_Fixed a )		{ return Rtt_FixedToInt( a ); }
Rtt_FORCE_INLINE Rtt_Fixed	FloatToFixed( float a )			{ return Rtt_FloatToFixed( a ); }
Rtt_FORCE_INLINE float		FixedToFloat( Rtt_Fixed a )		{ return Rtt_FixedToFloat( a ); }
Rtt_FORCE_INLINE Rtt_Fixed	FixedMidpoint( Rtt_Fixed a, Rtt_Fixed b ) { return Rtt_FixedMidpoint( a, b ); }
Rtt_FORCE_INLINE Rtt_Fixed	FixedDiv2( Rtt_Fixed a )		{ return Rtt_FixedDiv2( a ); }

Rtt_FORCE_INLINE Rtt_Fixed	FixedAbs( Rtt_Fixed a )				{ return Rtt_FixedAbs( a ); }
Rtt_FORCE_INLINE Rtt_Fixed	FixedMin( Rtt_Fixed a, Rtt_Fixed b ) { return Rtt_FixedMin( a, b ); }
Rtt_FORCE_INLINE Rtt_Fixed	FixedMax( Rtt_Fixed a, Rtt_Fixed b ) { return Rtt_FixedMax( a, b ); }
Rtt_FORCE_INLINE Rtt_Fixed	FixedMul( Rtt_Fixed a, Rtt_Fixed b ) { return Rtt_FixedMul( a, b ); }

} // Rtt

#endif // __cplusplus
*/
// ----------------------------------------------------------------------------

#endif // _Rtt_Fixed_H__
