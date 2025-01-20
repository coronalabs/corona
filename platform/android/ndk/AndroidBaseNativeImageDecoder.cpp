//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include <stdio.h>
#include "AndroidBaseNativeImageDecoder.h"
#include "AndroidBinaryReader.h"
#include "AndroidFileReader.h"
#include "AndroidOperationResult.h"
#include "AndroidZipFileEntry.h"
#include "NativeToJavaBridge.h"


// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a new image decoder.
/// @param allocatorPointer Allocator this decoder needs to creates its objects. Cannot be NULL.
AndroidBaseNativeImageDecoder::AndroidBaseNativeImageDecoder(Rtt_Allocator *allocatorPointer, NativeToJavaBridge *ntjb)
:	AndroidBaseImageDecoder(allocatorPointer), fNativeToJavaBridge(ntjb)
{
}

/// Creates a new image decoder with settings copied from the given decoder.
/// @param decoder The decoder to copy settings from.
AndroidBaseNativeImageDecoder::AndroidBaseNativeImageDecoder(const AndroidBaseImageDecoder &decoder, NativeToJavaBridge *ntjb)
:	AndroidBaseImageDecoder(decoder), fNativeToJavaBridge(ntjb)
{
}

/// Destroys this decoder and its resources.
AndroidBaseNativeImageDecoder::~AndroidBaseNativeImageDecoder()
{
}


// ----------------------------------------------------------------------------
// Public Member Functions
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
AndroidOperationResult AndroidBaseNativeImageDecoder::DecodeFrom(AndroidBinaryReader &reader)
{
	// Do not continue if the reader/stream is not open.
	if (reader.IsClosed())
	{
		return AndroidOperationResult::FailedWith(GetAllocator(), "Invalid reader given to native image decoder.");
	}

	// Do not continue if we do not have an image data object to decode to.
	if (GetTarget() == NULL)
	{
		return AndroidOperationResult::FailedWith(GetAllocator(), "Image decoder was not provided a target.");
	}

	// Decode the image and return the result.
	return OnDecodeFrom(reader);
}


// ----------------------------------------------------------------------------
// Protected Member Functions
// ----------------------------------------------------------------------------

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
AndroidOperationResult AndroidBaseNativeImageDecoder::OnDecodeFromFile(const char *filePath)
{
	// Check if the given file path references an asset within the APK or Google Play expansion file.
	// If so, then the below function will find its package file and byte offset within the file.
	AndroidZipFileEntry zipFileEntry(GetAllocator());
	bool isAssetFile = fNativeToJavaBridge->GetAssetFileLocation(filePath, zipFileEntry);

	// Do not continue if the given file is a compressed asset.
//TODO: Add support for decompressing assets.
	if (isAssetFile && zipFileEntry.IsCompressed())
	{
		char message[512];
		snprintf(message, sizeof(message), "Unable to read image file \"%s\" because it is compressed.", filePath);
		return AndroidOperationResult::FailedWith(GetAllocator(), message);
	}

	// Open the given file.
	FILE *filePointer = NULL;
	if (isAssetFile)
	{
		filePointer = fopen(zipFileEntry.GetPackageFilePath(), "rb");
		if (filePointer)
		{
			int seekResult = fseek(filePointer, zipFileEntry.GetByteOffsetInPackage(), SEEK_SET);
			if (seekResult)
			{
				fclose(filePointer);
				filePointer = NULL;
			}
		}
	}
	else
	{
		filePointer = fopen(filePath, "rb");
	}
	if (NULL == filePointer)
	{
		char message[512];
		snprintf(message, sizeof(message), "Failed to open file \"%s\".", filePath);
		return AndroidOperationResult::FailedWith(GetAllocator(), message);
	}

	// Decode the given image file.
	AndroidFileReader reader(GetAllocator());
	reader.Open(filePointer);
	AndroidOperationResult result = DecodeFrom(reader);
	reader.Close();
	fclose(filePointer);
	return result;
}
