//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_PlatformTimer.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

PlatformTimer::PlatformTimer( MCallback& callback )
:	fCallback( callback )
{
}

PlatformTimer::~PlatformTimer()
{
}

void
PlatformTimer::SwitchToForegroundTimer()
{
}

void
PlatformTimer::SwitchToBackgroundTimer()
{
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

