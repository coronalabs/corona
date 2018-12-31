// ----------------------------------------------------------------------------
// 
// NativeDebuggerLogger.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "NativeDebuggerLogger.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Logging {

#pragma region Constructors/Destructors
NativeDebuggerLogger::NativeDebuggerLogger()
{
}

#pragma endregion


#pragma region Public Instance Methods/Properties
bool NativeDebuggerLogger::CanWrite::get()
{
	return IsDebuggerPresent() ? true : false;
}

void NativeDebuggerLogger::Write(Platform::String^ message)
{
	if (message->IsEmpty() == false)
	{
		OutputDebugStringW(message->Data());
	}
}

void NativeDebuggerLogger::Write(const Platform::Array<wchar_t>^ message)
{
	if (message && message->Data)
	{
		auto stringPointer = message->Data;
		OutputDebugStringW(stringPointer);
	}
}

#pragma endregion


#pragma region Public Static Functions/Properties
NativeDebuggerLogger^ NativeDebuggerLogger::Instance::get()
{
	static NativeDebuggerLogger sInstance;
	return %sInstance;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Logging
