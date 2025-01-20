//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "DispatchCoronaLuaEventResult.h"
#include "ICoronaBoxedData.h"


namespace CoronaLabs { namespace Corona { namespace WinRT {

#pragma region Consructors/Destructors
DispatchCoronaLuaEventResult::DispatchCoronaLuaEventResult(
	bool hasSucceeded, Platform::String^ message, ICoronaBoxedData^ returnedValue)
{
	fHasSucceeded = hasSucceeded;
	fMessage = message;
	fReturnedValue = returnedValue;
}

#pragma endregion


#pragma region Public Instance Methods/Properties
bool DispatchCoronaLuaEventResult::HasSucceeded::get()
{
	return fHasSucceeded;
}

bool DispatchCoronaLuaEventResult::HasFailed::get()
{
	return !fHasSucceeded;
}

Platform::String^ DispatchCoronaLuaEventResult::Message::get()
{
	return fMessage;
}

ICoronaBoxedData^ DispatchCoronaLuaEventResult::ReturnedValue::get()
{
	return fReturnedValue;
}

#pragma endregion


#pragma region Public Static Functions
DispatchCoronaLuaEventResult^ DispatchCoronaLuaEventResult::SucceededWith(ICoronaBoxedData^ returnedValue)
{
	return ref new DispatchCoronaLuaEventResult(true, nullptr, returnedValue);
}

DispatchCoronaLuaEventResult^ DispatchCoronaLuaEventResult::FailedWith(Platform::String^ message)
{
	return ref new DispatchCoronaLuaEventResult(false, message, nullptr);
}

#pragma endregion

} } }	// namespace CoronaLabs::Corona::WinRT
