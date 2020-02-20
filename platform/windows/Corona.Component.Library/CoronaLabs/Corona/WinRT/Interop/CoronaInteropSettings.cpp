//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "CoronaInteropSettings.h"
#include "Graphics\IImageServices.h"
#include "Input\IInputDeviceServices.h"
#include "Networking\INetworkServices.h"
#include "Storage\IResourceServices.h"
#include "UI\IUserInterfaceServices.h"
#include "IReadOnlyWeakReference.h"
#include "ITimerServices.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop {

#pragma region Constructors/Destructors
CoronaInteropSettings::CoronaInteropSettings()
{
}

#pragma endregion


#pragma region Public Methods/Properties
void CoronaInteropSettings::CopyFrom(CoronaInteropSettings^ settings)
{
	// Do not continue if given null or a reference to this object.
	if ((nullptr == settings) || (this == settings))
	{
		return;
	}

	// Copy the given settings.
	this->ApplicationServices = settings->ApplicationServices;
	this->ImageServices = settings->ImageServices;
	this->InputDeviceServices = settings->InputDeviceServices;
	this->NetworkServices = settings->NetworkServices;
	this->ResourceServices = settings->ResourceServices;
	this->TimerServices = settings->TimerServices;
	this->UserInterfaceServices = settings->UserInterfaceServices;
	this->WeakCoronaControlReference = settings->WeakCoronaControlReference;
}

#pragma endregion

} } } }	// namespace CoronaLabs::Corona::WinRT::Interop
