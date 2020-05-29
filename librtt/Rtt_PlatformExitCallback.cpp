//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_PlatformExitCallback.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// Note: These implementations are only expected to be directly used by the Simulators.
// These functions are activated via shell.lua which is Simulator only.
// Non-simulators should never actually hit this code.
// Simulators are expected to override the behavior to trap the exit() calls as necessary.

// ----------------------------------------------------------------------------
void
PlatformExitCallback::operator()()
{
	exit(0);
}

void
PlatformExitCallback::operator()(int code)
{
	exit(code);
}


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

