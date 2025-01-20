//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __Rtt_HighPrecisionTime_H__
#define __Rtt_HighPrecisionTime_H__

#include "Core/Rtt_Macros.h" // TODO: Fix header dependency issue
#include "Core/Rtt_Types.h"  // TODO: Fix header dependency issue

#include "Core/Rtt_Real.h"
#include "Core/Rtt_Time.h"

// ----------------------------------------------------------------------------

Rtt_AbsoluteTime Rtt_GetPreciseAbsoluteTime();
Rtt::Real Rtt_PreciseAbsoluteToMilliseconds( Rtt_AbsoluteTime absoluteTime );
Rtt::Real Rtt_PreciseAbsoluteToMicroseconds( Rtt_AbsoluteTime absoluteTime );

// ----------------------------------------------------------------------------

#endif // __Rtt_HighPrecisionTime_H__
