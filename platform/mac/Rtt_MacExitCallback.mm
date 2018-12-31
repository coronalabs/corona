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

