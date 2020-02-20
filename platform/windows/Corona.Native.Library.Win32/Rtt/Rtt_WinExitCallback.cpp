//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
