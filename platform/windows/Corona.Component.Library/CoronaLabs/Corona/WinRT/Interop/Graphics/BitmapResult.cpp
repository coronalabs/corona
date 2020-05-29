//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "BitmapResult.h"
#include "IBitmap.h"
#include "CoronaLabs\WinRT\OperationResult.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

#pragma region Consructors/Destructors
BitmapResult::BitmapResult(bool hasSucceeded, Platform::String^ message, IBitmap^ bitmap)
{
	if (hasSucceeded && bitmap)
	{
		fBaseResult = CoronaLabs::WinRT::OperationResult::SucceededWith(message);
		fBitmap = bitmap;
	}
	else
	{
		fBaseResult = CoronaLabs::WinRT::OperationResult::FailedWith(message);
		fBitmap = nullptr;
	}
}

#pragma endregion


#pragma region Public Instance Methods/Properties
bool BitmapResult::HasSucceeded::get()
{
	return fBaseResult->HasSucceeded;
}

bool BitmapResult::HasFailed::get()
{
	return fBaseResult->HasFailed;
}

Platform::String^ BitmapResult::Message::get()
{
	return fBaseResult->Message;
}

IBitmap^ BitmapResult::Bitmap::get()
{
	return fBitmap;
}

#pragma endregion


#pragma region Public Static Functions
BitmapResult^ BitmapResult::SucceededWith(IBitmap^ bitmap)
{
	if (!bitmap)
	{
		throw ref new Platform::NullReferenceException();
	}
	return ref new BitmapResult(true, nullptr, bitmap);
}

BitmapResult^ BitmapResult::FailedWith(Platform::String^ message)
{
	return ref new BitmapResult(false, message, nullptr);
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
