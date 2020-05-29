//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include "AndroidNativePngDecoder.h"
#include "AndroidBaseImageDecoder.h"
#include "AndroidBinaryReader.h"
#include "AndroidBinaryReadResult.h"
#include "AndroidImageData.h"
#include "AndroidOperationResult.h"
#include "Core/Rtt_Build.h"
#include "Core/Rtt_Types.h"
extern "C"
{
	#include "png.h"
}


// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a new image decoder.
/// @param allocatorPointer Allocator this decoder needs to creates its objects. Cannot be NULL.
AndroidNativePngDecoder::AndroidNativePngDecoder(Rtt_Allocator *allocatorPointer, NativeToJavaBridge *ntjb)
:	AndroidBaseNativeImageDecoder(allocatorPointer, ntjb)
{
}

/// Creates a new image decoder with settings copied from the given decoder.
/// @param decoder The decoder to copy settings from.
AndroidNativePngDecoder::AndroidNativePngDecoder(const AndroidBaseImageDecoder &decoder, NativeToJavaBridge *ntjb)
:	AndroidBaseNativeImageDecoder(decoder, ntjb)
{
}

/// Destroys this decoder and its resources.
AndroidNativePngDecoder::~AndroidNativePngDecoder()
{
}


// ----------------------------------------------------------------------------
// Private C Functions
// ----------------------------------------------------------------------------

/// Called when the "libpng" library's png_error() function gets called.
/// Typically indicates that there was an error decoding a PNG image and the decoder needs to give up.
/// @param pngReaderPointer The PNG reader that the error occurred on.
/// @param message The error message describing what went wrong.
static void PngErrorCallback(png_structp pngReaderPointer, png_const_charp message)
{
	Rtt_LogException(Rtt_StringIsEmpty(message) ? "Error decoding PNG." : message);
}

/// Called when the "libpng" library's png_warning() function gets called.
/// A warning will not prevent the PNG reader from decoding the image.
/// It is intended to provide information to the developer to help during debugging.
/// @param pngReaderPointer The PNG reader that the warning occurred on.
/// @param message The warning message to be logged.
static void PngWarningCallback(png_structp pngReaderPointer, png_const_charp message)
{
	if (!Rtt_StringIsEmpty(message))
	{
		Rtt_LogException(message);
	}
}

/// Called when the PNG reader requests more bytes to be streamed to it for decoding.
/// <br>
/// Every call to this function is expected to advanced the stream's position in the PNG's file or byte stream.
/// @param pngReaderPointer The PNG reader requesting more bytes.
/// @param byteBuffer The byte array that the PNG's bytes need to be copied to.
/// @param bytesToRead The number of bytes that the PNG reader is requesting.
static void PngInputRequestCallback(png_structp pngReaderPointer, png_bytep byteBuffer, png_size_t bytesToRead)
{
	// Validate.
	if (!pngReaderPointer || !(pngReaderPointer->io_ptr) || !byteBuffer || (bytesToRead <= 0))
	{
		return;
	}

	// Fetch the file stream reader.
	AndroidBinaryReader *fileReaderPointer = (AndroidBinaryReader*)(pngReaderPointer->io_ptr);

	// Read the next batch of bytes from the file to the PNG decoder.
	AndroidBinaryReadResult result = fileReaderPointer->StreamTo((U8*)byteBuffer, (U32)bytesToRead);
	if (result.HasFailed())
	{
		const char *message = result.GetErrorMessage() ? result.GetErrorMessage() : "Error reading file.";
		png_error(pngReaderPointer, message);
	}
	else if (result.GetBytesRead() != (U32)bytesToRead)
	{
		png_error(pngReaderPointer, "PNG file does not contain the expected amount of bytes.");
	}
}


// ----------------------------------------------------------------------------
// Protected Member Functions
// ----------------------------------------------------------------------------

/// Decodes an image from the given reader/stream and copies its data to the targeted image data object
/// assigned to this decoder via the SetTarget() function.
/// @param reader The reader which stream's the image's bytes to this decoder.
/// @return Returns the result of the image decoding operation.
///         <br>
///         If the result object's HasSucceeded() function returns true, then this decoder
///         has successfully loaded the image and copied its data to the target image data object.
///         <br>
///         If the result object's HasSucceed() function returns false, then this decoder
///         has failed to decode the image and the targeted image data object might not have been
///         written to or might have incomplete information. The returned result object's
///         GetErrorMessage() function will typically provide a reason why it failed.
AndroidOperationResult AndroidNativePngDecoder::OnDecodeFrom(AndroidBinaryReader &reader)
{
	AndroidBinaryReadResult readResult;

	// Ensure that the given file is really a PNG file.
	// We do this by checking if the PNG's signature is at the head of the file.
	U8 pngSignatureBytes[8];
	U32 pngSignatureLength = (U32)sizeof(pngSignatureBytes);
	readResult = reader.StreamTo(pngSignatureBytes, pngSignatureLength);
	if (readResult.GetBytesRead() != pngSignatureLength)
	{
		return AndroidOperationResult::FailedWith(GetAllocator(), "The given file is not a PNG or is corrupted.");
	}
	if (png_sig_cmp(pngSignatureBytes, 0, pngSignatureLength) != 0)
	{
		return AndroidOperationResult::FailedWith(GetAllocator(), "The given file is not a PNG or is corrupted.");
	}

	// Create the PNG reader and its information object.
	png_structp pngReaderPointer = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!pngReaderPointer)
	{
		return AndroidOperationResult::FailedWith(GetAllocator(), "Failed to create libpng read struct.");
	}
	png_infop pngInfoPointer = png_create_info_struct(pngReaderPointer);
	if (!pngInfoPointer)
	{
		png_destroy_read_struct(&pngReaderPointer, NULL, NULL);
		return AndroidOperationResult::FailedWith(GetAllocator(), "Failed to create libpng info struct.");
	}

	// Set up the PNG reader's error/warning handlers.
	// Note that the PNG reader will long jump inside of this "if" block if an error occurs.
	png_set_error_fn(pngReaderPointer, NULL, PngErrorCallback, PngWarningCallback);
	if (setjmp(png_jmpbuf(pngReaderPointer)))
	{
		png_destroy_read_struct(&pngReaderPointer, &pngInfoPointer, NULL);
		return AndroidOperationResult::FailedWith(GetAllocator(), "Error decoding PNG.");
	}

	// Set up a callback with the PNG reader to request bytes from our file stream reader.
	// This is needed so that we can read a PNG inside of a package file, such as the APK.
	png_set_read_fn(pngReaderPointer, &reader, PngInputRequestCallback);

	// Inform the PNG reader that we have already read past the signature at the head of the file.
	png_set_sig_bytes(pngReaderPointer, pngSignatureLength);

	// Fetch the image's dimensions and other info.
	png_uint_32 sourcePixelWidth = 0;
	png_uint_32 sourcePixelHeight = 0;
	int colorChannelBitCount = 0;
	int colorType = -1;
	int interlaceMethod = PNG_INTERLACE_NONE;
	png_read_info(pngReaderPointer, pngInfoPointer);
	png_uint_32 getHeaderResult = png_get_IHDR(
									pngReaderPointer, pngInfoPointer, &sourcePixelWidth, &sourcePixelHeight,
									&colorChannelBitCount, &colorType, &interlaceMethod, NULL, NULL);
	if (!getHeaderResult || !sourcePixelWidth || !sourcePixelHeight || (colorChannelBitCount <= 0))
	{
		png_destroy_read_struct(&pngReaderPointer, &pngInfoPointer, NULL);
		return AndroidOperationResult::FailedWith(GetAllocator(), "Error decoding PNG header.");
	}

	// If this decoder has been given a max width or height (ie: a max value greater than zero),
	// then use the smallest of the maximum lengths given to downsample with below.
	int maxTargetPixelWidth = GetMaxWidth();
	int maxTargetPixelHeight = GetMaxHeight();
	int maxTargetPixelLength = 0;
	if ((maxTargetPixelWidth > 0) && (maxTargetPixelHeight > 0))
	{
		if (maxTargetPixelWidth < maxTargetPixelHeight)
		{
			maxTargetPixelLength = maxTargetPixelWidth;
		}
		else
		{
			maxTargetPixelLength = maxTargetPixelHeight;
		}
	}
	else if (maxTargetPixelWidth > 0)
	{
		maxTargetPixelLength = maxTargetPixelWidth;
	}
	else if (maxTargetPixelHeight > 0)
	{
		maxTargetPixelLength = maxTargetPixelHeight;
	}

	// Downsample the image if it is larger than the given max width or height.
	// Note: Downsampling involves skipping every other decoded pixel, which downscales
	//       the image in powers of 2 in a fast and memory efficient manner.
	int targetPixelWidth = sourcePixelWidth;
	int targetPixelHeight = sourcePixelHeight;
	float scale = 1.0f;
	int pixelSampleFrequency = 1;
	if (maxTargetPixelLength > 0)
	{
		int longestImageLength = (sourcePixelWidth > sourcePixelHeight) ? sourcePixelWidth : sourcePixelHeight;
		float percentDelta = (float)longestImageLength / (float)maxTargetPixelLength;
		while (percentDelta > 1.0f)
		{
			pixelSampleFrequency++;
			percentDelta /= 2.0f;
		}
		if (pixelSampleFrequency > 1)
		{
			targetPixelWidth = sourcePixelWidth / pixelSampleFrequency;
			if ((sourcePixelWidth % pixelSampleFrequency) > 0)
			{
				targetPixelWidth++;
			}
			targetPixelHeight = sourcePixelHeight / pixelSampleFrequency;
			if ((sourcePixelHeight % pixelSampleFrequency) > 0)
			{
				targetPixelHeight++;
			}
			scale = (float)targetPixelWidth / (float)sourcePixelWidth;
		}
	}

	// Copy the image dimensions to the targeted image data object.
	// This must be done after we've calculated the downsampled size, if applicable.
	AndroidImageData *imageDataPointer = GetTarget();
	imageDataPointer->SetWidth(targetPixelWidth);
	imageDataPointer->SetHeight(targetPixelHeight);
	imageDataPointer->SetScale(Rtt_FloatToReal(scale));
	if (IsPixelFormatGrayscale())
	{
		imageDataPointer->SetPixelFormatToGrayscale();
	}
	else
	{
		imageDataPointer->SetPixelFormatToRGBA();
	}

	// Do not continue if the caller only wants image information and not the decoded pixels.
	if (IsDecodingImageInfoOnly())
	{
		png_destroy_read_struct(&pngReaderPointer, &pngInfoPointer, NULL);
		return AndroidOperationResult::Succeeded(GetAllocator());
	}

	// Do not attempt to load the PNG here if it is interlaced.
	// This is a significantly more difficult format to load via the "libpng" library.
	if (interlaceMethod != PNG_INTERLACE_NONE)
	{
		png_destroy_read_struct(&pngReaderPointer, &pngInfoPointer, NULL);
		return AndroidOperationResult::FailedWith(GetAllocator(), "Interlaced PNG files are not supported.");
	}

	// Have the PNG reader convert the color channels to 8-bit, if necessary.
	if (16 == colorChannelBitCount)
	{
		png_set_strip_16(pngReaderPointer);
	}
	else if ((colorChannelBitCount < 8) && (PNG_COLOR_TYPE_GRAY == colorType))
	{
		png_set_gray_1_2_4_to_8(pngReaderPointer);
	}
	colorChannelBitCount = 8;

	// If the PNG is using a color palette, then have it converted to RGB.
	if (PNG_COLOR_TYPE_PALETTE == colorType)
	{
		png_set_palette_to_rgb(pngReaderPointer);
		colorType = PNG_COLOR_TYPE_RGB;
	}

	// Have the PNG reader provide an alpha channel if set in the image's palette.
	if (png_get_valid(pngReaderPointer, pngInfoPointer, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(pngReaderPointer);
		if (PNG_COLOR_TYPE_GRAY == colorType)
		{
			colorType = PNG_COLOR_TYPE_GRAY_ALPHA;
		}
		else if (PNG_COLOR_TYPE_RGB == colorType)
		{
			colorType = PNG_COLOR_TYPE_RGB_ALPHA;
		}
	}

	// Have the PNG reader transform the decoded pixels to the expected RGB or grayscale format.
	if (IsPixelFormatGrayscale())
	{
		// Caller is expecting an 8-bit grayscaled image to be loaded.
		// If the PNG is not grayscale, then have the PNG reader convert it to grayscale.
		if ((PNG_COLOR_TYPE_RGB == colorType) || (PNG_COLOR_TYPE_RGB_ALPHA == colorType))
		{
			png_set_rgb_to_gray_fixed(pngReaderPointer, 1, -1, -1);
			colorType = PNG_COLOR_TYPE_GRAY | (colorType & PNG_COLOR_MASK_ALPHA);
		}
	}
	else
	{
		// Caller is expecting a 32-bit RGBA image to be loaded.
		// If the PNG is grayscale, then have the PNG reader convert it to RGB or RGBA.
		if ((PNG_COLOR_TYPE_GRAY == colorType) || (PNG_COLOR_TYPE_GRAY_ALPHA == colorType))
		{
			png_set_gray_to_rgb(pngReaderPointer);
			colorType = PNG_COLOR_TYPE_RGB | (colorType & PNG_COLOR_MASK_ALPHA);
		}
	}

	// Apply the above transformations.
	png_read_update_info(pngReaderPointer, pngInfoPointer);

	// Determine the number of bytes in one pixel.
	int sourcePixelSizeInBytes;
	switch (colorType)
	{
		case PNG_COLOR_TYPE_GRAY:
			sourcePixelSizeInBytes = 1;
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			sourcePixelSizeInBytes = 2;
			break;
		case PNG_COLOR_TYPE_RGB:
			sourcePixelSizeInBytes = 3;
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
		default:
			sourcePixelSizeInBytes = 4;
			break;
	}

	// Create the image buffer that will store the image in decompressed bitmap form.
	imageDataPointer->CreateImageByteBuffer();
	char *imageBytes = (char*)imageDataPointer->GetImageByteBuffer();
	if (!imageBytes)
	{
		png_destroy_read_struct(&pngReaderPointer, &pngInfoPointer, NULL);
		char message[] = "Not enough memory to decode the given PNG.";
		return AndroidOperationResult::FailedWith(GetAllocator(), message);
	}

	// Decode the PNG to an uncompressed bitmap.
	int bytesPerRow = png_get_rowbytes(pngReaderPointer, pngInfoPointer);
	png_byte *byteBuffer = new png_byte[bytesPerRow];
	for (int sourceRowIndex = 0, targetRowIndex = 0;
	     (sourceRowIndex < sourcePixelHeight) && (targetRowIndex < targetPixelHeight);
	     sourceRowIndex++)
	{
		// Decode the next row of pixels.
		png_read_row(pngReaderPointer, byteBuffer, NULL);

		// Do not copy this row's pixels to the bitmap if it is downsampled out.
		if ((sourceRowIndex % pixelSampleFrequency) > 0)
		{
			continue;
		}

		// Copy the row of pixels to the bitmap.
		targetRowIndex++;
		for (int byteIndex = 0, sourcePixelIndex = 0, targetPixelIndex = 0;
		     (byteIndex < bytesPerRow) && (targetPixelIndex < targetPixelWidth);
		     byteIndex += sourcePixelSizeInBytes, sourcePixelIndex++)
		{
			// Skip this pixel if it is downsampled out.
			if ((sourcePixelIndex % pixelSampleFrequency) > 0)
			{
				continue;
			}

			// Convert the decoded pixel to the expected color format and copy it to the bitmap.
			targetPixelIndex++;
			if (IsPixelFormatGrayscale())
			{
				if (PNG_COLOR_TYPE_RGB_ALPHA == colorType)
				{
					int alpha = byteBuffer[byteIndex + 3];
					int grayscaledColor = (int)(
							(0.30f * byteBuffer[byteIndex]) +
							(0.59f * byteBuffer[byteIndex + 1]) +
							(0.11f * byteBuffer[byteIndex + 2]));
					*imageBytes = (char)((grayscaledColor * alpha) >> 8);
				}
				else if (PNG_COLOR_TYPE_RGB == colorType)
				{
					*imageBytes = (char)(
							(0.30f * byteBuffer[byteIndex]) +
							(0.59f * byteBuffer[byteIndex + 1]) +
							(0.11f * byteBuffer[byteIndex + 2]));
				}
				else if (PNG_COLOR_TYPE_GRAY_ALPHA == colorType)
				{
					*imageBytes = (char)(((int)byteBuffer[byteIndex] * (int)byteBuffer[byteIndex + 1]) >> 8);
				}
				else if (PNG_COLOR_TYPE_GRAY == colorType)
				{
					*imageBytes = byteBuffer[byteIndex];
				}
				imageBytes++;
			}
			else
			{
				if (PNG_COLOR_TYPE_RGB_ALPHA == colorType)
				{
					int alpha = byteBuffer[byteIndex + 3];
					imageBytes[0] = (char)(((int)byteBuffer[byteIndex] * alpha) >> 8);
					imageBytes[1] = (char)(((int)byteBuffer[byteIndex + 1] * alpha) >> 8);
					imageBytes[2] = (char)(((int)byteBuffer[byteIndex + 2] * alpha) >> 8);
					imageBytes[3] = (char)alpha;
				}
				else
				{
					imageBytes[0] = byteBuffer[byteIndex];
					imageBytes[1] = byteBuffer[byteIndex + 1];
					imageBytes[2] = byteBuffer[byteIndex + 2];
					imageBytes[3] = (char)255;
				}
				imageBytes += 4;
			}
		}
	}

	// Destroy the reader and its objects.
	delete [] byteBuffer;
	png_destroy_read_struct(&pngReaderPointer, &pngInfoPointer, NULL);

	// Return a success result.
	return AndroidOperationResult::Succeeded(GetAllocator());
}
