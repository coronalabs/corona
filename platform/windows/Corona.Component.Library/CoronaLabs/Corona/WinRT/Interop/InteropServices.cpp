// ----------------------------------------------------------------------------
// 
// InteropServices.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "InteropServices.h"
#include "Graphics\IImageServices.h"
#include "Input\IInputDeviceServices.h"
#include "Networking\INetworkServices.h"
#include "Storage\IResourceServices.h"
#include "UI\IUserInterfaceServices.h"
#include "CoronaInteropSettings.h"
#include "IApplicationServices.h"
#include "ITimerServices.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop {

#pragma region Constructors/Destructors
InteropServices::InteropServices(CoronaInteropSettings^ settings)
{
	// Validate the given settings.
	if (nullptr == settings)
	{
		throw ref new Platform::NullReferenceException(L"settings");
	}
	if (nullptr == settings->ApplicationServices)
	{
		throw ref new Platform::NullReferenceException(L"settings->ApplicationServices");
	}
	if (nullptr == settings->ImageServices)
	{
		throw ref new Platform::NullReferenceException(L"settings->ImageServices");
	}
	if (nullptr == settings->InputDeviceServices)
	{
		throw ref new Platform::NullReferenceException(L"settings->InputDeviceServices");
	}
	if (nullptr == settings->NetworkServices)
	{
		throw ref new Platform::NullReferenceException(L"settings->NetworkServices");
	}
	if (nullptr == settings->ResourceServices)
	{
		throw ref new Platform::NullReferenceException(L"settings->ResourceServices");
	}
	if (nullptr == settings->TimerServices)
	{
		throw ref new Platform::NullReferenceException(L"settings->TimerServices");
	}

	// Create a copy of the given settings.
	fSettings = ref new CoronaInteropSettings();
	fSettings->CopyFrom(settings);
}

#pragma endregion


#pragma region Public Methods/Properties
IApplicationServices^ InteropServices::ApplicationServices::get()
{
	return fSettings->ApplicationServices;
}

Graphics::IImageServices^ InteropServices::ImageServices::get()
{
	return fSettings->ImageServices;
}

Input::IInputDeviceServices^ InteropServices::InputDeviceServices::get()
{
	return fSettings->InputDeviceServices;
}

Networking::INetworkServices^ InteropServices::NetworkServices::get()
{
	return fSettings->NetworkServices;
}

Storage::IResourceServices^ InteropServices::ResourceServices::get()
{
	return fSettings->ResourceServices;
}

ITimerServices^ InteropServices::TimerServices::get()
{
	return fSettings->TimerServices;
}

UI::IUserInterfaceServices^ InteropServices::UserInterfaceServices::get()
{
	return fSettings->UserInterfaceServices;
}

#pragma endregion

} } } }	// namespace CoronaLabs::Corona::WinRT::Interop
