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

#include "Core/Rtt_Math.h"

// ----------------------------------------------------------------------------

Rtt_EXPORT U32
NextPowerOf2( U32 x )
{
	x = x - 1;
	x |= (x >> 1);
	x |= (x >> 2);
	x |= (x >> 4);
	x |= (x >> 8);
	x |= (x >> 16);
	return x + 1;
}

// This function DOESN'T consider zero to be a power of 2.
Rtt_EXPORT U32
IsPowerOf2( U32 value )
{
	return ( ( ! ( value & ( value - 1 ) ) ) && value );
}

Rtt_EXPORT U32
IsAlignedToPowerOf2( U32 value, U32 alignment )
{
	Rtt_ASSERT( IsPowerOf2( alignment ) );

	return ( ! ( value & ( alignment - 1 ) ) );
}

Rtt_EXPORT U32
AlignToPowerOf2( U32 value, U32 alignment )
{
	Rtt_ASSERT( IsPowerOf2( alignment ) );

	return ( ( value + ( alignment - 1 ) ) & ( ~ ( alignment - 1 ) ) );
}

// ----------------------------------------------------------------------------
