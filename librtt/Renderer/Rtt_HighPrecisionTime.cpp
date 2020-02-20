//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Config.h" // TODO: Cleanup header include dependencies
#include "Renderer/Rtt_HighPrecisionTime.h"

// ----------------------------------------------------------------------------

Rtt_AbsoluteTime
Rtt_GetPreciseAbsoluteTime()
{
	return Rtt_GetAbsoluteTime();
}

Rtt::Real 
Rtt_PreciseAbsoluteToMilliseconds( Rtt_AbsoluteTime absoluteTime )
{
	return static_cast<Rtt::Real>( Rtt_AbsoluteToMilliseconds( absoluteTime ) );
}

Rtt::Real 
Rtt_PreciseAbsoluteToMicroseconds( Rtt_AbsoluteTime absoluteTime )
{
	return static_cast<Rtt::Real>( Rtt_AbsoluteToMicroseconds( absoluteTime ) );
}

// ----------------------------------------------------------------------------
