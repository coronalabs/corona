//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "NativeStringServices.h"
#include "Utf8String.h"


namespace CoronaLabs { namespace WinRT {

#pragma region Constructors/Destructors
NativeStringServices::NativeStringServices()
{
}

#pragma endregion


#pragma region Public Static Functions
Utf8String^ NativeStringServices::Utf8From(Platform::String^ utf16String)
{
	return Utf8String::From(utf16String);
}

Utf8String^ NativeStringServices::Utf8From(const wchar_t* utf16String)
{
	return Utf8String::From(utf16String);
}

Platform::String^ NativeStringServices::Utf16FromUtf8(const char* utf8String)
{
	if (!utf8String)
	{
		return nullptr;
	}
	return Utf8String(utf8String).ToString();
}

Platform::String^ NativeStringServices::Utf16From(Utf8String^ utf8String)
{
	if (!utf8String)
	{
		return nullptr;
	}
	return utf8String->ToString();
}

const char* NativeStringServices::GetStringPointerFrom(Utf8String^ utf8String)
{
	if (!utf8String)
	{
		return NULL;
	}
	return utf8String->Data;
}

#pragma endregion

} }	// namespace CoronaLabs::WinRT
