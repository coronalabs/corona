// ----------------------------------------------------------------------------
// 
// BitmapInfoResult.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "BitmapInfoResult.h"
#include "BitmapInfo.h"
#include "CoronaLabs\WinRT\OperationResult.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

#pragma region Consructors/Destructors
BitmapInfoResult::BitmapInfoResult(
	bool hasSucceeded, Platform::String^ message, CoronaLabs::Corona::WinRT::Interop::Graphics::BitmapInfo^ info)
{
	if (hasSucceeded && info)
	{
		fBaseResult = CoronaLabs::WinRT::OperationResult::SucceededWith(message);
		fBitmapInfo = info;
	}
	else
	{
		fBaseResult = CoronaLabs::WinRT::OperationResult::FailedWith(message);
		fBitmapInfo = nullptr;
	}
}

#pragma endregion


#pragma region Public Instance Methods/Properties
bool BitmapInfoResult::HasSucceeded::get()
{
	return fBaseResult->HasSucceeded;
}

bool BitmapInfoResult::HasFailed::get()
{
	return fBaseResult->HasFailed;
}

Platform::String^ BitmapInfoResult::Message::get()
{
	return fBaseResult->Message;
}

CoronaLabs::Corona::WinRT::Interop::Graphics::BitmapInfo^ BitmapInfoResult::BitmapInfo::get()
{
	return fBitmapInfo;
}

#pragma endregion


#pragma region Public Static Functions
BitmapInfoResult^ BitmapInfoResult::SucceededWith(
	CoronaLabs::Corona::WinRT::Interop::Graphics::BitmapInfo^ info)
{
	if (!info)
	{
		throw ref new Platform::NullReferenceException();
	}
	return ref new BitmapInfoResult(true, nullptr, info);
}

BitmapInfoResult^ BitmapInfoResult::FailedWith(Platform::String^ message)
{
	return ref new BitmapInfoResult(false, message, nullptr);
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
