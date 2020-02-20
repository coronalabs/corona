//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "BitmapConverter.h"
#include "BitmapInfo.h"
#include "BitmapSettings.h"
#include "Bitmap.h"
#include "IImageDecoder.h"
#include "ImageDecoderSettings.h"
#include "PixelFormat.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core\Rtt_Time.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


#define LOG_BITMAP_CONVERTER_PERFORMANCE 0


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Graphics {

#pragma region Consructors/Destructors
BitmapConverter::BitmapConverter()
:	fMaxPixelWidth(0),
	fMaxPixelHeight(0),
	fPremultiplyAlphaEnabled(false),
	fPixelFormat(CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat::RGBA)
{
}

#pragma endregion


#pragma region Public Properties/Methods
int BitmapConverter::MaxPixelWidth::get()
{
	return fMaxPixelWidth;
}

void BitmapConverter::MaxPixelWidth::set(int value)
{
	if (value < 0)
	{
		value = 0;
	}
	fMaxPixelWidth = value;
}

int BitmapConverter::MaxPixelHeight::get()
{
	return fMaxPixelHeight;
}

void BitmapConverter::MaxPixelHeight::set(int value)
{
	if (value < 0)
	{
		value = 0;
	}
	fMaxPixelHeight = value;
}

bool BitmapConverter::PremultiplyAlphaEnabled::get()
{
	return fPremultiplyAlphaEnabled;
}

void BitmapConverter::PremultiplyAlphaEnabled::set(bool value)
{
	fPremultiplyAlphaEnabled = value;
}

CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat^ BitmapConverter::PixelFormat::get()
{
	return fPixelFormat;
}

void BitmapConverter::PixelFormat::set(CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat^ value)
{
	// Validate.
	if (!value)
	{
		throw ref new Platform::NullReferenceException();
	}
	if (CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat::Undefined == value)
	{
		throw ref new Platform::InvalidArgumentException("Pixel format cannot be set to 'Undefined'.");
	}

	// Store the given pixel format.
	fPixelFormat = value;
}

void BitmapConverter::CopySettingsFrom(IImageDecoder^ decoder)
{
	// Validate.
	if (!decoder)
	{
		throw ref new Platform::NullReferenceException();
	}

	// Copy the decoder's settings that are relevant to this class.
	this->MaxPixelWidth = decoder->Settings->MaxPixelWidth;
	this->MaxPixelHeight = decoder->Settings->MaxPixelHeight;
	this->PremultiplyAlphaEnabled = decoder->Settings->PremultiplyAlphaEnabled;
	this->PixelFormat = decoder->Settings->PreferredPixelFormat;
}

bool BitmapConverter::CanConvert(IBitmap^ bitmap)
{
	// Validate argument.
	if (!bitmap)
	{
		return false;
	}

	// Check if the given bitmap has zero width and height.
	if ((bitmap->Info->PixelWidth <= 0) || (bitmap->Info->PixelHeight <= 0))
	{
		return false;
	}

	// Check if the given bitmap must be downsampled.
	if ((bitmap->Info->PixelWidth > fMaxPixelWidth) || (bitmap->Info->PixelHeight > fMaxPixelHeight))
	{
		return false;
	}

	// Check if the given bitmap's pixel size will have to change.
	if (bitmap->Info->PixelFormat->ByteCount != fPixelFormat->ByteCount)
	{
		return false;
	}

	// The given bitmap's bytes can be changed by this object's Convert() method.
	return true;
}

bool BitmapConverter::Convert(IBitmap^ bitmap)
{
	// Do not continue if the given bitmap's bytes cannot be converted by this method.
	if (CanConvert(bitmap) == false)
	{
		return false;
	}

	// Fetch the given bitmap's byte buffer.
	uint8 *byteBufferPointer = (uint8*)(bitmap->ByteBufferMemoryAddress);
	if (!byteBufferPointer)
	{
		return false;
	}

	// Do not continue if the given bitmap is grayscale. There is nothing for us to convert in this case.
	if (bitmap->Info->PixelFormat->ByteCount <= 1)
	{
		return true;
	}

	// Convert the given bitmap's pixels.
	for (int rowIndex = 0; rowIndex < bitmap->Info->PixelHeight; rowIndex++)
	{
		for (int columnIndex = 0; columnIndex < bitmap->Info->PixelWidth; columnIndex++)
		{
			// Fetch the next pixel's color values.
			uint8 red = 0;
			uint8 green = 0;
			uint8 blue = 0;
			uint8 alpha = 255;
			if (CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat::BGRA == bitmap->Info->PixelFormat)
			{
				blue = byteBufferPointer[columnIndex];
				green = byteBufferPointer[columnIndex + 1];
				red = byteBufferPointer[columnIndex + 2];
				alpha = byteBufferPointer[columnIndex + 3];
			}
			else if (CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat::RGBA == bitmap->Info->PixelFormat)
			{
				red = byteBufferPointer[columnIndex];
				green = byteBufferPointer[columnIndex + 1];
				blue = byteBufferPointer[columnIndex + 2];
				alpha = byteBufferPointer[columnIndex + 3];
			}

			// Apply premultiplied alpha, if enabled.
			if (fPremultiplyAlphaEnabled && (false == bitmap->Info->PremultipliedAlphaApplied))
			{
				red = (uint8)(((int)red * (int)alpha) >> 8);
				green = (uint8)(((int)green * (int)alpha) >> 8);
				blue = (uint8)(((int)blue * (int)alpha) >> 8);
			}

			// Update the pixel's color channels using this converter's given pixel format.
			if (CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat::RGBA == fPixelFormat)
			{
				byteBufferPointer[0] = red;
				byteBufferPointer[1] = green;
				byteBufferPointer[2] = blue;
				byteBufferPointer[3] = alpha;
			}
			else if (CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat::BGRA == fPixelFormat)
			{
				byteBufferPointer[0] = blue;
				byteBufferPointer[1] = green;
				byteBufferPointer[2] = red;
				byteBufferPointer[3] = alpha;
			}
		}
		byteBufferPointer += bitmap->Info->StrideInBytes;
	}
	return true;
}

IBitmap^ BitmapConverter::CreateBitmapFrom(IBitmap^ bitmap)
{
	// Validate.
	if (!bitmap)
	{
		return nullptr;
	}

	// Do not continue if the given bitmap has an undefined/unknown pixel format.
	if (CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat::Undefined == bitmap->Info->PixelFormat)
	{
		return nullptr;
	}

	// Fetch the given bitmap's byte buffer.
	uint8 *sourceByteBufferPointer = (uint8*)(bitmap->ByteBufferMemoryAddress);
	if (!sourceByteBufferPointer)
	{
		return nullptr;
	}

	// If this converter has been given a max width or height (ie: a max value greater than zero),
	// then use the smallest of the maximum lengths given to downsample with below.
	int maxTargetPixelWidth = fMaxPixelWidth;
	int maxTargetPixelHeight = fMaxPixelHeight;
	int maxTargetPixelLength = 0;
	if ((maxTargetPixelWidth > 0) && (maxTargetPixelHeight > 0))
	{
		maxTargetPixelLength = min(maxTargetPixelWidth, maxTargetPixelHeight);
	}
	else if (maxTargetPixelWidth > 0)
	{
		maxTargetPixelLength = maxTargetPixelWidth;
	}
	else if (maxTargetPixelHeight > 0)
	{
		maxTargetPixelLength = maxTargetPixelHeight;
	}

	// Downsample the bitmap if it is larger than the max width or height.
	// Note: Downsampling involves skipping every other pixel, which downscales
	//       the image in powers of 2 in a fast and memory efficient manner.
	int targetPixelWidth = bitmap->Info->PixelWidth;
	int targetPixelHeight = bitmap->Info->PixelHeight;
	float scale = 1.0f;
	int pixelSampleFrequency = 1;
	if (maxTargetPixelLength > 0)
	{
		int longestImageLength = max(bitmap->Info->PixelWidth, bitmap->Info->PixelHeight);
		float percentDelta = (float)longestImageLength / (float)maxTargetPixelLength;
		while (percentDelta > 1.0f)
		{
			pixelSampleFrequency++;
			percentDelta /= 2.0f;
		}
		if (pixelSampleFrequency > 1)
		{
			targetPixelWidth = bitmap->Info->PixelWidth / pixelSampleFrequency;
			if ((bitmap->Info->PixelWidth % pixelSampleFrequency) > 0)
			{
				targetPixelWidth++;
			}
			targetPixelHeight = bitmap->Info->PixelHeight / pixelSampleFrequency;
			if ((bitmap->Info->PixelHeight % pixelSampleFrequency) > 0)
			{
				targetPixelHeight++;
			}
			scale = (float)targetPixelWidth / (float)(bitmap->Info->PixelWidth);
		}
	}

	// Create a new bitmap that will contain the converted pixels.
	auto settings = ref new BitmapSettings();
	settings->Orientation = bitmap->Info->Orientation;
	settings->PixelFormat = fPixelFormat;
	settings->PixelWidth = targetPixelWidth;
	settings->PixelHeight = targetPixelHeight;
	settings->Scale = bitmap->Info->Scale * scale;
	auto targetBitmap = ref new Bitmap();
	targetBitmap->FormatUsing(ref new BitmapInfo(settings));
	uint8 *targetByteBufferPointer = (uint8*)(targetBitmap->ByteBufferMemoryAddress);

	// Do not continue if the resulting bitmap has zero width or height.
	if (!targetByteBufferPointer || (targetBitmap->Info->PixelWidth <= 0) || (targetBitmap->Info->PixelHeight <= 0))
	{
		return targetBitmap;
	}

#if LOG_BITMAP_CONVERTER_PERFORMANCE
	auto startTime = Rtt_GetAbsoluteTime();
#endif

	// Fetch the memory addresses of all pixel format constants this converter supports.
	// Note: Comparing pixel format types by memory address in the loop below provides a *huge* performance boost.
	const void* kPixelFormatGrayscalePointer = (void*)CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat::Grayscale;
	const void* kPixelFormatBgraPointer = (void*)CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat::BGRA;
	const void* kPixelFormatRgbaPointer = (void*)CoronaLabs::Corona::WinRT::Interop::Graphics::PixelFormat::RGBA;

	// Fetch information about the source and target bitmaps to be used by the loop below.
	// Note: Avoiding accessing the reference object's "->" accessors provides a *huge* performance boost.
	int sourceStrideInBytes = bitmap->Info->StrideInBytes;
	int sourceRowBytePadding = sourceStrideInBytes - (bitmap->Info->PixelWidth * bitmap->Info->PixelFormat->ByteCount);
	int targetRowBytePadding = targetBitmap->Info->StrideInBytes - (targetPixelWidth * fPixelFormat->ByteCount);
	int sourcePixelByteCount = bitmap->Info->PixelFormat->ByteCount;
	int targetPixelByteCount = fPixelFormat->ByteCount;
	const void* sourcePixelFormatPointer = (void*)(bitmap->Info->PixelFormat);
	const void* targetPixelFormatPointer = (void*)fPixelFormat;

	// Traverse all pixels in the given bitmap.
	for (int sourceRowIndex = 0, targetRowIndex = 0;
	     (sourceRowIndex < bitmap->Info->PixelHeight) && (targetRowIndex < targetPixelHeight);
		 sourceRowIndex++, sourceByteBufferPointer += sourceStrideInBytes)
	{
		// Do not copy this row's pixels to the bitmap if it is downsampled out.
		if ((pixelSampleFrequency > 1) && ((sourceRowIndex % pixelSampleFrequency) > 0))
		{
			continue;
		}

		// Traverse all pixels in the next row of the source bitmap.
		targetRowIndex++;
		for (int sourceByteIndex = 0, sourcePixelIndex = 0, targetPixelIndex = 0;
		     (sourceByteIndex < sourceStrideInBytes) && (targetPixelIndex < targetPixelWidth);
			 sourceByteIndex += sourcePixelByteCount, sourcePixelIndex++)
		{
			// Skip this pixel if it is downsampled out.
			if ((pixelSampleFrequency > 1) && ((sourcePixelIndex % pixelSampleFrequency) > 0))
			{
				continue;
			}

			// Convert the source pixel to the expected format and copy it to the target bitmap.
			if (kPixelFormatGrayscalePointer == targetPixelFormatPointer)
			{
				// Fetch the source bitmap's next pixel, converting it to a grayscale color if necessary.
				uint8 alpha = 0;
				if (kPixelFormatGrayscalePointer == sourcePixelFormatPointer)
				{
					alpha = sourceByteBufferPointer[sourceByteIndex];
				}
				else if (kPixelFormatBgraPointer == sourcePixelFormatPointer)
				{
					int grayscaledColor =
							(11 * (int)sourceByteBufferPointer[sourceByteIndex]) +		// Blue
							(59 * (int)sourceByteBufferPointer[sourceByteIndex + 1]) +	// Green
							(30 * (int)sourceByteBufferPointer[sourceByteIndex + 2]);	// Red
					grayscaledColor /= 100;
					alpha = sourceByteBufferPointer[sourceByteIndex + 3];
					alpha = (uint8)((grayscaledColor * (int)alpha) >> 8);
				}
				else if (kPixelFormatRgbaPointer == sourcePixelFormatPointer)
				{
					int grayscaledColor =
							(30 * (int)sourceByteBufferPointer[sourceByteIndex]) +		// Red
							(59 * (int)sourceByteBufferPointer[sourceByteIndex + 1]) +	// Green
							(11 * (int)sourceByteBufferPointer[sourceByteIndex + 2]);	// Blue
					grayscaledColor /= 100;
					alpha = sourceByteBufferPointer[sourceByteIndex + 3];
					alpha = (uint8)((grayscaledColor * (int)alpha) >> 8);
				}

				// Write the grayscale color to the target bitmap's next pixel.
				*targetByteBufferPointer = alpha;
			}
			else
			{
				// Fetch the color values from the source bitmap's next pixel.
				uint8 red = 0;
				uint8 green = 0;
				uint8 blue = 0;
				uint8 alpha = 255;
				if (kPixelFormatBgraPointer == sourcePixelFormatPointer)
				{
					blue = sourceByteBufferPointer[sourceByteIndex];
					green = sourceByteBufferPointer[sourceByteIndex + 1];
					red = sourceByteBufferPointer[sourceByteIndex + 2];
					alpha = sourceByteBufferPointer[sourceByteIndex + 3];
				}
				else if (kPixelFormatRgbaPointer == sourcePixelFormatPointer)
				{
					red = sourceByteBufferPointer[sourceByteIndex];
					green = sourceByteBufferPointer[sourceByteIndex + 1];
					blue = sourceByteBufferPointer[sourceByteIndex + 2];
					alpha = sourceByteBufferPointer[sourceByteIndex + 3];
				}
				else if (kPixelFormatGrayscalePointer == sourcePixelFormatPointer)
				{
					alpha = sourceByteBufferPointer[sourceByteIndex];
					red = alpha;
					green = alpha;
					blue = alpha;
				}

				// Apply premultiplied alpha, if enabled.
				if (fPremultiplyAlphaEnabled && (false == bitmap->Info->PremultipliedAlphaApplied))
				{
					red = (uint8)(((int)red * (int)alpha) >> 8);
					green = (uint8)(((int)green * (int)alpha) >> 8);
					blue = (uint8)(((int)blue * (int)alpha) >> 8);
				}

				// Write the color values to the target bitmap's next pixel.
				if (kPixelFormatRgbaPointer == targetPixelFormatPointer)
				{
					targetByteBufferPointer[0] = red;
					targetByteBufferPointer[1] = green;
					targetByteBufferPointer[2] = blue;
					targetByteBufferPointer[3] = alpha;
				}
				else if (kPixelFormatBgraPointer == targetPixelFormatPointer)
				{
					targetByteBufferPointer[0] = blue;
					targetByteBufferPointer[1] = green;
					targetByteBufferPointer[2] = red;
					targetByteBufferPointer[3] = alpha;
				}
			}

			// Increment the pointer to the next pixel.
			targetByteBufferPointer += targetPixelByteCount;
		}

		// Increment the pointer to the 1st pixel in the next row of the bitmap.
		targetByteBufferPointer += targetRowBytePadding;
	}

#if LOG_BITMAP_CONVERTER_PERFORMANCE
	int durationInMilliseconds = (int)Rtt_AbsoluteToMilliseconds(Rtt_GetAbsoluteTime() - startTime);
	wchar_t message[256];
	swprintf_s(message, 256, L"BitmapConverter::CreateBitmapFrom() Duration = %d ms\r\n", durationInMilliseconds);
	OutputDebugStringW(message);
#endif

	// Return the newly created bitmap.
	return targetBitmap;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Graphics
