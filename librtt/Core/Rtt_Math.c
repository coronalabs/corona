//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
