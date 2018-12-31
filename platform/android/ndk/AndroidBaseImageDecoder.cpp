//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include "AndroidBaseImageDecoder.h"
#include "AndroidImageData.h"
#include "AndroidOperationResult.h"
#include "Core/Rtt_Build.h"
#include "Core/Rtt_Types.h"


// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a new image decoder.
/// @param allocatorPointer Allocator this decoder needs to creates its objects. Cannot be NULL.
AndroidBaseImageDecoder::AndroidBaseImageDecoder(Rtt_Allocator *allocatorPointer)
:	fAllocatorPointer(allocatorPointer),
	fImageDataPointer(NULL),
	fIsPixelFormatGrayscale(false),
	fIsDecodingImageInfoOnly(false),
	fMaxWidth(0),
	fMaxHeight(0)
{
}

/// Destroys this decoder and its resources.
AndroidBaseImageDecoder::~AndroidBaseImageDecoder()
{
}


// ----------------------------------------------------------------------------
// Public Member Functions
// ----------------------------------------------------------------------------

/// Gets the allocator this object uses.
/// @return Returns a pointer to the allocator.
///         <br>
///         Returns NULL if this object was never assigned an allocator.
Rtt_Allocator* AndroidBaseImageDecoder::GetAllocator() const
{
	return fAllocatorPointer;
}

/// Gets the data object that this decoder will write all image information and pixels to
/// when the DecodeFrom() function is called.
/// @return Returns a pointer to the data object assigned to this decoder by the SetTarget() function.
///         <br>
///         Returns NULL if this decoder does not have data object to write information to.
AndroidImageData* AndroidBaseImageDecoder::GetTarget() const
{
	return fImageDataPointer;
}

/// Sets the data object that this decoder will write all image information and pixels to
/// when the DecodeFrom() function is called.
/// @param imageDataPointer Pointer to the image data object that this decoder will write to.
void AndroidBaseImageDecoder::SetTarget(AndroidImageData *imageDataPointer)
{
	fImageDataPointer = imageDataPointer;
}

/// Determines if the decoded image pixels will be converted to 32-bit RGBA when
/// writing to the targeted image data object.
/// @return Returns true if decoded pixels will be converted to 32-bit RGBA.
///         <br>
///         Returns false if the decoded pixels will be converted to an 8-bit grayscale format.
bool AndroidBaseImageDecoder::IsPixelFormatRGBA() const
{
	return !fIsPixelFormatGrayscale;
}

/// Determines if the decoded image pixels will be converted to 8-bit grayscale when
/// writing to the targeted image data object.
/// @return Returns true if the decoded pixels will be converted to an 8-bit grayscale format.
///         <br>
///         Returns false if decoded pixels will be converted to 32-bit RGBA.
bool AndroidBaseImageDecoder::IsPixelFormatGrayscale() const
{
	return fIsPixelFormatGrayscale;
}

/// Sets up this decoder to convert the decoded image's pixels to 32-bit RGBA when writing
/// to the targeted image data object.
void AndroidBaseImageDecoder::SetPixelFormatToRGBA()
{
	fIsPixelFormatGrayscale = false;
}

/// Sets up this decoder to convert the decoded image's pixels to 8-bit grayscale when writing
/// to the targeted image data object.
void AndroidBaseImageDecoder::SetPixelFormatToGrayscale()
{
	fIsPixelFormatGrayscale = true;
}

/// Determines if this decoder will only read the image's information such as width and height.
/// @return Returns true if this decoder will only read the image's information such as its
///         width, height, and orientation. It will not decode the pixels in this case.
///         <br>
///         Returns false if all image data (information and pixels) will be read.
bool AndroidBaseImageDecoder::IsDecodingImageInfoOnly() const
{
	return fIsDecodingImageInfoOnly;
}

/// Determines if this decoder will read the image's information and pixels.
/// @return Returns true if this decoder will read all pixel data and information from the image.
///         <br>
///         Returns false if this decoder will only read the image's information such as its
///         width, height, and orientation. It will not decode the pixels in this case.
bool AndroidBaseImageDecoder::IsDecodingAllImageData() const
{
	return fIsDecodingImageInfoOnly;
}

/// Sets up this decoder to only read the image's information such as width, height, and orientation
/// when the DecodeFrom() function is called. The image's pixels will not be decoded.
void AndroidBaseImageDecoder::SetToDecodeImageInfoOnly()
{
	fIsDecodingImageInfoOnly = true;
}

/// Sets up this decoder to read all image data, which includes its pixels and image information.
/// <br>
/// This is the default.
void AndroidBaseImageDecoder::SetToDecodeAllImageData()
{
	fIsDecodingImageInfoOnly = false;
}

/// Gets the maximum pixel width that this decoder will set the targeted image data to.
/// <br>
/// If the decoded image is larger than this maximum, then the image will be downscaled or
/// downsampled when the decoded pixels are copied to the targeted image data object.
/// @return Returns the maximum pixel width that this decoder will set the targeted image data to.
///         <br>
///         Returns 0 if no maximum will be applied while decoding, meaning that the image
///         will not be downscaled/downsampled and be loaded as is.
U32 AndroidBaseImageDecoder::GetMaxWidth() const
{
	return fMaxWidth;
}

/// Sets the maximum pixel width that this decoder will set the targeted image data to.
/// <br>
/// If the decoded image is larger than this maximum, then the image will be downscaled or
/// downsampled when the decoded pixels are copied to the targeted image data object.
/// <br>
/// If you intend to decode images to be used as OpenGL textures, then you are expected
/// to set this to OpenGL's max texture size.
/// @param value The maximum pixel width to be applied while decoding.
///              <br>
///              Set to zero to not apply a maximum and load the image as is.
void AndroidBaseImageDecoder::SetMaxWidth(U32 value)
{
	fMaxWidth = value;
}

/// Gets the maximum pixel height that this decoder will set the targeted image data to.
/// <br>
/// If the decoded image is larger than this maximum, then the image will be downscaled or
/// downsampled when the decoded pixels are copied to the targeted image data object.
/// @return Returns the maximum pixel height that this decoder will set the targeted image data to.
///         <br>
///         Returns 0 if no maximum will be applied while decoding, meaning that the image
///         will not be downscaled/downsampled and be loaded as is.
U32 AndroidBaseImageDecoder::GetMaxHeight() const
{
	return fMaxHeight;
}

/// Sets the maximum pixel height that this decoder will set the targeted image data to.
/// <br>
/// If the decoded image is larger than this maximum, then the image will be downscaled or
/// downsampled when the decoded pixels are copied to the targeted image data object.
/// <br>
/// If you intend to decode images to be used as OpenGL textures, then you are expected
/// to set this to OpenGL's max texture size.
/// @param value The maximum pixel height to be applied while decoding.
///              <br>
///              Set to zero to not apply a maximum and load the image as is.
void AndroidBaseImageDecoder::SetMaxHeight(U32 value)
{
	fMaxHeight = value;
}

/// Decodes the given image file and copies its data to the targeted image data object
/// assigned to this decoder via the SetTarget() function.
/// @param filePath The name and path of the image file to decode. Cannot be NULL or empty.
/// @return Returns the result of the image decoding operation.
///         <br>
///         If the result object's HasSucceeded() function returns true, then this decoder
///         has successfully loaded the image and copied its data to the target image data object.
///         <br>
///         If the result object's HasSucceed() function returns false, then this decoder
///         has failed to decode the image and the targeted image data object might not have been
///         written to or might have incomplete information. The returned result object's
///         GetErrorMessage() function will typically provide a reason why it failed.
AndroidOperationResult AndroidBaseImageDecoder::DecodeFromFile(const char *filePath)
{
	// Validate.
	if (Rtt_StringIsEmpty(filePath))
	{
		return AndroidOperationResult::FailedWith(fAllocatorPointer, "Invalid image file path.");
	}
	if (!fImageDataPointer)
	{
		char message[512];
		snprintf(message, sizeof(message), "Image decoder was not provided a target for file \"%s\".", filePath);
		return AndroidOperationResult::FailedWith(fAllocatorPointer, message);
	}

	// Decode the image file and return the result.
	return OnDecodeFromFile(filePath);
}
