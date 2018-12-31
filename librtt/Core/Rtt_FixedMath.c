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

#include "Core/Rtt_Build.h"

//#undef Rtt_MATH_INLINE
#include "Core/Rtt_Fixed.h"

// TODO: Optimize for fixed instead of converting to/from floats

#include "Core/Rtt_Math.h"

// ----------------------------------------------------------------------------

Rtt_EXPORT_BEGIN

// ----------------------------------------------------------------------------

/*
// For Symbian...
#ifdef _STRICT_ANSI
	#define sqrtf		(float)sqrt
	#define M_LOG2_E	0.693147180559945309417
	#define log2f(a)	(float)(log(a)/M_LOG2_E)
	#define cosf		(float)cos
	#define sinf		(float)sin
	#define tanf		(float)tan
#endif
*/
#if defined( Rtt_WIN_ENV ) || defined( Rtt_POWERVR_ENV ) || defined( Rtt_ANDROID_ENV )
#define M_LOG2_E	0.693147180559945309417
#define log2f(a)	(float)(logf(a)/M_LOG2_E)
#endif

float Rtt_FloatSqrt( float a )
{
	return sqrtf( a );
}

Rtt_Fixed Rtt_FixedSqrt( Rtt_Fixed a )
{
	return Rtt_FloatToFixed( sqrtf( Rtt_FixedToFloat( a ) ) );
}

Rtt_Fixed Rtt_FixedLog2( Rtt_Fixed a )
{
	return Rtt_FloatToFixed( log2f( Rtt_FixedToFloat( a ) ) );
}

Rtt_Fixed Rtt_FixedCos( Rtt_Fixed a )
{
	return Rtt_FloatToFixed( cosf( Rtt_FixedToFloat( a ) ) );
}

Rtt_Fixed Rtt_FixedSin( Rtt_Fixed a )
{
	return Rtt_FloatToFixed( sinf( Rtt_FixedToFloat( a ) ) );
}

Rtt_Fixed Rtt_FixedTan( Rtt_Fixed a )
{
	return Rtt_FloatToFixed( tanf( Rtt_FixedToFloat( a ) ) );
}

// ----------------------------------------------------------------------------

Rtt_EXPORT_END

// ----------------------------------------------------------------------------
