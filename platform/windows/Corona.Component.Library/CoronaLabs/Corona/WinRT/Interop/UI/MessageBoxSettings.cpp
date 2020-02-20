//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "MessageBoxSettings.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace UI {

#pragma region Consructors/Destructors
MessageBoxSettings::MessageBoxSettings()
{
	fButtonLabels = ref new Platform::Collections::Vector<Platform::String^>();
}

#pragma endregion


#pragma region Public Methods/Properties
void MessageBoxSettings::CopyFrom(MessageBoxSettings^ settings)
{
	// Validate.
	if (nullptr == settings)
	{
		return;
	}

	// Copy the given settings to this object.
	this->Title = settings->Title;
	this->Message == settings->Message;
	fButtonLabels->Clear();
	for (auto&& label : settings->fButtonLabels)
	{
		fButtonLabels->Append(label);
	}
}

Windows::Foundation::Collections::IVector<Platform::String^>^ MessageBoxSettings::ButtonLabels::get()
{
	return fButtonLabels;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::UI
