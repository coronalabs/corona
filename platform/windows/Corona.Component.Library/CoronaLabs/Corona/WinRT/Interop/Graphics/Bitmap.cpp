// ----------------------------------------------------------------------------
// 
// Bitmap.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "Bitmap.h"
#include "BitmapInfo.h"
#include "BitmapSettings.h"
#include "PixelFormat.h"
#include "CoronaLabs\WinRT\OperationResult.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

#pragma region Consructors/Destructors
Bitmap::Bitmap()
{
	ReleaseByteBuffer();
}

#pragma endregion


#pragma region Public Properties/Methods
CoronaLabs::WinRT::OperationResult^ Bitmap::FormatUsing(BitmapInfo^ info)
{
	// Throw an exception if given null.
	if (!info)
	{
		throw ref new Platform::NullReferenceException();
	}

	// Check if we were given a zero width and height bitmap settings.
	// If so, then destroy the previous bitmap bytes and stop here.
	if ((info->PixelWidth <= 0) && (info->PixelWidth <= 0))
	{
		ReleaseByteBuffer();
		return CoronaLabs::WinRT::OperationResult::Succeeded;
	}

	// Validate the given bitmap settings.
	if (info->PixelWidth <= 0)
	{
		Platform::String^ message = "Cannot create a bitmap with a pixel width less than or equal to zero.";
		return CoronaLabs::WinRT::OperationResult::FailedWith(message);
	}
	if (info->PixelHeight <= 0)
	{
		Platform::String^ message = "Cannot create a bitmap with a pixel height less than or equal to zero.";
		return CoronaLabs::WinRT::OperationResult::FailedWith(message);
	}
	if ((PixelFormat::Undefined == info->PixelFormat) || (info->PixelFormat->ByteCount <= 0))
	{
		Platform::String^ message = "Cannot create a bitmap with an 'undefined' pixel format.";
		return CoronaLabs::WinRT::OperationResult::FailedWith(message);
	}

	// Create the bitmap's byte array used to store all pixel data.
	// Note: The stride is the number of bytes per row, rounded up to the next 4 byte increment.
	//       This is needed to fit within the 4 byte packing alignment required by the rendering system.
	int byteCount = info->StrideInBytes * (info->PixelHeight * info->PixelFormat->ByteCount);
	fBitmapBytes = ref new Platform::Array<uint8>(byteCount);
	fBitmapInfo = info;

	// Return a success result.
	return CoronaLabs::WinRT::OperationResult::Succeeded;
}

void Bitmap::ReleaseByteBuffer()
{
	// Reset this bitmap's information back to zero width and height.
	fBitmapInfo = BitmapInfo::Empty;

	// Replace the byte array with a zero length array if not already done.
	if (!fBitmapBytes || (fBitmapBytes->Length > 0))
	{
		fBitmapBytes = ref new Platform::Array<uint8>(0);
	}
}

BitmapInfo^ Bitmap::Info::get()
{
	return fBitmapInfo;
}

int64 Bitmap::ByteBufferMemoryAddress::get()
{
	int64 memoryAddress = 0;
	if (fBitmapBytes)
	{
		memoryAddress = (int64)(fBitmapBytes->Data);
	}
	return memoryAddress;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
