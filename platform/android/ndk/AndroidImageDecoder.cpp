//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "AndroidImageDecoder.h"
#include "AndroidJavaImageDecoder.h"
#include "AndroidNativePngDecoder.h"
#include "AndroidOperationResult.h"
#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"


// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a new image decoder.
/// @param allocatorPointer Allocator this decoder needs to creates its objects. Cannot be NULL.
AndroidImageDecoder::AndroidImageDecoder(Rtt_Allocator *allocatorPointer, NativeToJavaBridge * ntjb)
:	AndroidBaseImageDecoder(allocatorPointer), fNativeToJavaBridge(ntjb)
{
}

/// Destroys this decoder and its resources.
AndroidImageDecoder::~AndroidImageDecoder()
{
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
AndroidOperationResult AndroidImageDecoder::OnDecodeFromFile(const char *filePath)
{
	AndroidOperationResult result;

	// First, attempt to load the image file natively on the C/C++ side.
	if (Rtt_StringEndsWithNoCase(filePath, ".png"))
	{
		AndroidNativePngDecoder pngDecoder(*this, fNativeToJavaBridge);
		result = pngDecoder.DecodeFromFile(filePath);
	}

	// If we're unable to decode the image natively, then load it on the Java side.
	if (result.HasFailed())
	{
		AndroidJavaImageDecoder javaImageDecoder(*this, fNativeToJavaBridge);
		result = javaImageDecoder.DecodeFromFile(filePath);
	}

	// Return the result.
	return result;
}
