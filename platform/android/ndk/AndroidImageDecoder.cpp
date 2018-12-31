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
