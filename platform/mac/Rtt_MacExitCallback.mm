//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_MacExitCallback.h"
#import "AppDelegate.h"
#include <stdlib.h>
// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------
void
MacExitCallback::operator()()
{
	(*this)(0);
}

void
MacExitCallback::operator()(int code)
{
	AppDelegate* appdelegate = (AppDelegate*)[[NSApplication sharedApplication] delegate];
	if( NO == appdelegate.allowLuaExit )
	{
		// We want to simulate the user calling close with the menu to close the simulator.
		// Calling close directly causes crashes because I think there are too many assumptions in the system
		// about calling close while in the middle of a lua event loop.
		// performSelector seems to schedule the close on the event loop so it will be run at a safe time.
		Rtt_LogException("Simulation Terminated: Lua script called os.exit() with status: %d\n", code);
		[appdelegate performSelectorOnMainThread:@selector(close:) withObject:nil waitUntilDone:NO];
	}
	else
	{
		// call exit?, call terminate?, do nothing (design code to let Lua do the normal thing)?
		exit(code);
	}
}


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

