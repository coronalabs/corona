//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "CoronaLuaEventArgs.h"
#include "CoronaLuaEventProperties.h"
#include "CoronaBoxedString.h"
#include "ReadOnlyCoronaLuaEventProperties.h"


namespace CoronaLabs { namespace Corona { namespace WinRT {

#pragma region Consructors/Destructors
CoronaLuaEventArgs::CoronaLuaEventArgs(CoronaLuaEventProperties^ properties)
{
	// Validate the given properties collection.
	if (nullptr == properties)
	{
		throw ref new Platform::NullReferenceException();
	}

	// Fetch the event "name" from the given properties collection.
	// This property is required. Throw an exception if missing.
	auto boxedString = dynamic_cast<CoronaBoxedString^>(properties->Get(L"name"));
	if (boxedString)
	{
		fEventName = boxedString->ToUtf16String();
	}
	if (fEventName->IsEmpty())
	{
		throw ref new Platform::InvalidArgumentException(
				L"The 'CoronaLuaEventProperties' argument must provide a property named 'name' with a non-empty string value.");
	}

	// Wrap the given properties collection in a read-only container.
	fReadOnlyProperties = ref new ReadOnlyCoronaLuaEventProperties(properties);
}

#pragma endregion


#pragma region Public Functions/Properties
Platform::String^ CoronaLuaEventArgs::EventName::get()
{
	return fEventName;
}

ReadOnlyCoronaLuaEventProperties^ CoronaLuaEventArgs::Properties::get()
{
	return fReadOnlyProperties;
}

#pragma endregion

} } }	// namespace CoronaLabs::Corona::WinRT
