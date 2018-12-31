// ----------------------------------------------------------------------------
// 
// Rtt_WinRTBitmap.cpp
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "Rtt_WinRTBitmap.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\BitmapInfo.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\BitmapInfoResult.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\BitmapResult.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\IImageDecoder.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\ImageDecoderSettings.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\PixelFormat.h"
#include "CoronaLabs\Corona\WinRT\Interop\Graphics\TextRendererSettings.h"
#include "CoronaLabs\WinRT\NativeStringServices.h"
#include "CoronaLabs\WinRT\Utf8String.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core/Rtt_Build.h"
#	include "Rtt_PlatformFont.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

#pragma region WinRTImageFileBitmap Class

#pragma region Constructors/Destructors
WinRTImageFileBitmap::WinRTImageFileBitmap(
	Rtt_Allocator &allocator, Platform::String^ filePath,
	CoronaLabs::Corona::WinRT::Interop::Graphics::IImageDecoder^ imageDecoder)
:	PlatformBitmap(),
	fImageDecoder(imageDecoder),
	fBitmapInfo(CoronaLabs::Corona::WinRT::Interop::Graphics::BitmapInfo::Empty),
	fBitmapData(nullptr),
	fAllocatorPointer(&allocator),
	fFilePath(filePath),
	fProperties(0)
{
	// Validate arguments.
	if (!fFilePath || !imageDecoder)
	{
		throw ref new Platform::NullReferenceException();
	}

	// Attempt to fetch the image file's information such as pixel width, height, orientation, etc.
	auto result = fImageDecoder->DecodeInfoFromFile(fFilePath);
	if (result->HasSucceeded && result->BitmapInfo)
	{
		// Store the decoded image information.
		fBitmapInfo = result->BitmapInfo;
	}
	else
	{
		// The decoder failed to fetch the image file's information. This can happen if the decoder does not support
		// loading an image file's header. In this case, attempt to decode the entire image now to acquire this information.
		GetBits(fAllocatorPointer);
	}

	// Flag whether or not the bitmap the decoder will produce will have premultiplied alpha.
	SetProperty(kIsPremultiplied, fBitmapInfo->PremultipliedAlphaApplied);
}

WinRTImageFileBitmap::~WinRTImageFileBitmap()
{
	FreeBits();
}

#pragma endregion


#pragma region Public Member Functions
const void* WinRTImageFileBitmap::GetBits(Rtt_Allocator *allocatorPointer) const
{
	// Decode the image from file, if not done already.
	if ((nullptr == fBitmapData) || (0 == fBitmapData->ByteBufferMemoryAddress))
	{
		// Decode the image file to an uncompressed bitmap.
		auto result = fImageDecoder->DecodeFromFile(fFilePath);
		fBitmapData = result->Bitmap;

		// Update this object's bitmap information such as pixel width, height, orientation, etc.
		fBitmapInfo = fBitmapData ? fBitmapData->Info : CoronaLabs::Corona::WinRT::Interop::Graphics::BitmapInfo::Empty;
		(const_cast<WinRTImageFileBitmap*>(this))->SetProperty(kIsPremultiplied, fBitmapInfo->PremultipliedAlphaApplied);

		// If we've failed to decode the image file, then log the reason why.
		if (nullptr == fBitmapData)
		{
			auto message = CoronaLabs::WinRT::NativeStringServices::Utf8From(result->Message);
			if (message && message->IsNotEmpty)
			{
				Rtt_LogException(message->Data);
			}
			else
			{
				auto utf8FilePath = CoronaLabs::WinRT::NativeStringServices::Utf8From(fFilePath);
				Rtt_LogException("Failed to load image file: %s", utf8FilePath->Data);
			}
		}
	}

	// Fetch the memory address of the bitmap's byte buffer, if available.
	int64 byteBufferMemoryAddress = 0;
	if (fBitmapData)
	{
		byteBufferMemoryAddress = fBitmapData->ByteBufferMemoryAddress;
	}

	// Return the memory address as a pointer.
	return (void*)byteBufferMemoryAddress;
}

void WinRTImageFileBitmap::FreeBits() const
{
	if (fBitmapData)
	{
		fBitmapData->ReleaseByteBuffer();
	}
}

U32 WinRTImageFileBitmap::Width() const
{
	int width;

	if (IsPropertyInternal(kIsBitsAutoRotated))
	{
		width = UprightWidth();
	}
	else if (fBitmapInfo->PixelFormat->ByteCount == 1)
	{
		width = fBitmapInfo->StrideInBytes;
	}
	else
	{
		width = fBitmapInfo->PixelWidth;
	}
	return (U32)width;
}

U32 WinRTImageFileBitmap::Height() const
{
	return IsPropertyInternal(kIsBitsAutoRotated) ? UprightHeight() : (U32)fBitmapInfo->PixelHeight;
}

U32 WinRTImageFileBitmap::UprightWidth() const
{
	// Determine if this bitmap is being displayed vertically.
	S32 angle = DegreesToUpright();
	bool isUprightOrUpsideDown = ((0 == angle) || (180 == Abs(angle)));

	// Fetch the pixel width of the bitmap when it is shown upright.
	int length;
	if (isUprightOrUpsideDown)
	{
		// The bitmap is upright or upside down in memory.
		if (fBitmapInfo->PixelFormat->ByteCount == 1)
		{
			// For 8-bit grayscale bitmaps, we must return the stride for the width.
			// Note: The stride is the number of bytes used per row, which might be padded at
			//       the end with additional bytes to fit within the 4 byte packing alignment.
			length = fBitmapInfo->StrideInBytes;
		}
		else
		{
			// For 32-bit color bitmaps, use the pixel width.
			length = fBitmapInfo->PixelWidth;
		}
	}
	else
	{
		// The bitmap is sideways in memory. This means that the height is the width when displayed upright.
		length = fBitmapInfo->PixelHeight;
	}
	return (U32)length;
}

U32 WinRTImageFileBitmap::UprightHeight() const
{
	// Determine if this bitmap is being displayed vertically.
	S32 angle = DegreesToUpright();
	bool isUprightOrUpsideDown = ((0 == angle) || (180 == Abs(angle)));

	// Fetch the pixel width of the bitmap when it is shown upright.
	int length;
	if (isUprightOrUpsideDown)
	{
		// The bitmap is upright or upside down in memory.
		length = fBitmapInfo->PixelHeight;
	}
	else
	{
		// The bitmap is sideways in memory. This means that the width is the height when displayed upright.
		if (fBitmapInfo->PixelFormat->ByteCount == 1)
		{
			// For 8-bit grayscale bitmaps, we must return the stride for the width.
			// Note: The stride is the number of bytes used per row, which might be padded at
			//       the end with additional bytes to fit within the 4 byte packing alignment.
			length = fBitmapInfo->StrideInBytes;
		}
		else
		{
			// For 32-bit color bitmaps, use the pixel width.
			length = fBitmapInfo->PixelWidth;
		}
	}
	return (U32)length;
}

Real WinRTImageFileBitmap::GetScale() const
{
	return Rtt_FloatToReal(fBitmapInfo->Scale);
}

PlatformBitmap::Format WinRTImageFileBitmap::GetFormat() const
{
	// Use the pixel format that was decoded from the image file, if loaded.
	if (fBitmapInfo)
	{
		return fBitmapInfo->PixelFormat->CoronaBitmapFormat;
	}

	// We have not attempted to access the image file yet.
	// Return the pixel format we expect to decode the image to.
	return fImageDecoder->Settings->PreferredPixelFormat->CoronaBitmapFormat;
}

bool WinRTImageFileBitmap::IsProperty(PropertyMask mask) const
{
	return IsPropertyInternal(mask);
}

void WinRTImageFileBitmap::SetProperty(PropertyMask mask, bool newValue)
{
	if (!PlatformBitmap::IsPropertyReadOnly(mask))
	{
		const U8 p = fProperties;
		const U8 propertyMask = (U8)mask;
		fProperties = (newValue ? p | propertyMask : p & ~propertyMask);
	}
}

#pragma endregion

#pragma endregion


#pragma region WinRTTextBitmap Class

#pragma region Constructors/Destructors
WinRTTextBitmap::WinRTTextBitmap(
	Rtt_Allocator &allocator, CoronaLabs::Corona::WinRT::Interop::Graphics::ITextRenderer^ textRenderer)
:	PlatformBitmap(),
	fTextRenderer(textRenderer),
	fBitmapInfo(CoronaLabs::Corona::WinRT::Interop::Graphics::BitmapInfo::Empty),
	fBitmapData(nullptr),
	fAllocatorPointer(&allocator),
	fProperties(0)
{
//JLQ: We can only create the text bitmap on Windows Phone while running on the main UI thread.
//     So, we must create the bitmap immediately here.
#if 0
	// Attempt to aquire the text's measurements without generating a bitmap.
	// Note: This provides a significant performance boost if text is updated several time between render passes.
	auto result = fTextRenderer->MeasureBitmap();
	if (result->HasSucceeded && result->BitmapInfo)
	{
		// Store the text's measurements.
		fBitmapInfo = result->BitmapInfo;
	}
	else
	{
		// Failed to predict the text bitmap's measurements. This can happen if the text renderer does not support
		// this feature. In this case, we have no choice but to generate the text bitmap now.
		GetBits(fAllocatorPointer);
	}
#else
	GetBits(fAllocatorPointer);
#endif
}

WinRTTextBitmap::~WinRTTextBitmap()
{
//JLQ: We currently only release the byte buffer in the destructor to prevent the rendering system from recreating
//     the text bitmap after a suspend/resume (ie: reload) which causes a threading deadlock issue. This issue occurs
//     because Microsoft's "WriteableBitmap" class in .NET hangs when calling its render thread while Corona's
//     has the rendering thread locked.
#if 0
	FreeBits();
#else
	if (fBitmapData)
	{
		fBitmapData->ReleaseByteBuffer();
	}
#endif
}

#pragma endregion


#pragma region Public Member Functions
const void* WinRTTextBitmap::GetBits(Rtt_Allocator *allocatorPointer) const
{
	// Render text to a bitmap, if not done already.
	if ((nullptr == fBitmapData) || (0 == fBitmapData->ByteBufferMemoryAddress))
	{
		// Render the text.
		auto result = fTextRenderer->CreateBitmap();
		fBitmapData = result->Bitmap;

		// Update this object's bitmap information such as pixel width, height, orientation, etc.
		fBitmapInfo = fBitmapData ? fBitmapData->Info : CoronaLabs::Corona::WinRT::Interop::Graphics::BitmapInfo::Empty;
		(const_cast<WinRTTextBitmap*>(this))->SetProperty(kIsPremultiplied, fBitmapInfo->PremultipliedAlphaApplied);

		// If we've failed to render text, then log the reason why.
		if (nullptr == fBitmapData)
		{
			auto message = CoronaLabs::WinRT::NativeStringServices::Utf8From(result->Message);
			if (message && message->IsNotEmpty)
			{
				Rtt_LogException(message->Data);
			}
			else
			{
				Rtt_LogException("Failed to render text.\n");
			}
		}
	}

	// Fetch the memory address of the bitmap's byte buffer, if available.
	int64 byteBufferMemoryAddress = 0;
	if (fBitmapData)
	{
		byteBufferMemoryAddress = fBitmapData->ByteBufferMemoryAddress;
	}

	// Return the memory address as a pointer.
	return (void*)byteBufferMemoryAddress;
}

void WinRTTextBitmap::FreeBits() const
{
//JLQ: We currently only release the byte buffer in the destructor to prevent the rendering system from recreating
//     the text bitmap after a suspend/resume (ie: reload) which causes a threading deadlock issue. This issue occurs
//     because Microsoft's "WriteableBitmap" class in .NET hangs when calling its render thread while Corona's
//     has the rendering thread locked.
#if 0
	if (fBitmapData)
	{
		fBitmapData->ReleaseByteBuffer();
	}
#endif
}

U32 WinRTTextBitmap::Width() const
{
	return (U32)(fBitmapInfo->StrideInBytes);
}

U32 WinRTTextBitmap::Height() const
{
	return (U32)(fBitmapInfo->PixelHeight);
}

PlatformBitmap::Format WinRTTextBitmap::GetFormat() const
{
	return PlatformBitmap::kMask;
}

bool WinRTTextBitmap::IsProperty(PropertyMask mask) const
{
	return (fProperties & mask) ? true : false;
}

void WinRTTextBitmap::SetProperty(PropertyMask mask, bool newValue)
{
	if (!PlatformBitmap::IsPropertyReadOnly(mask))
	{
		const U8 p = fProperties;
		const U8 propertyMask = (U8)mask;
		fProperties = (newValue ? p | propertyMask : p & ~propertyMask);
	}
}

#pragma endregion

#pragma endregion

} // namespace Rtt
