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

#include "stdafx.h"
#include "Rtt_WinExitCallback.h"
#include "Core\Rtt_Build.h"
#include "Interop\MDeviceSimulatorServices.h"
#include "Interop\RuntimeEnvironment.h"


namespace Rtt
{

#pragma region Constructors/Destructors
WinExitCallback::WinExitCallback(Interop::RuntimeEnvironment& environment)
:	fEnvironment(environment)
{
}

WinExitCallback::~WinExitCallback()
{
}

#pragma endregion


#pragma region Public Methods
void WinExitCallback::operator()()
{
	(*this)(0);
}

void WinExitCallback::operator()(int code)
{
	// If we're simulating a device via the Corona Simulator, then do not let os.exit() force quit the application.
	// Instead, gracefully terminate the Corona runtime and keep the application window alive.
	// Note: If the simulator was started with command line argument "-allowLuaExit", then we allow force quits.
	//       This is used by Corona Labs' automated test system.
	auto deviceSimulatorServicesPointer = fEnvironment.GetDeviceSimulatorServices();
	if (deviceSimulatorServicesPointer && (deviceSimulatorServicesPointer->IsLuaExitAllowed() == false))
	{
		Rtt_LogException("Simulation Terminated: Lua script called os.exit() with status: %d\n", code);
		deviceSimulatorServicesPointer->RequestTerminate();
		return;
	}

	// For all other cases, force quit the application process. This matches Lua's original os.exit() behavior.
	PlatformExitCallback::operator()(code);
}

#pragma endregion

} // namespace Rtt
