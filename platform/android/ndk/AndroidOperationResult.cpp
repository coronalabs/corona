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


#include "AndroidOperationResult.h"


// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a failure result object without an error message.
AndroidOperationResult::AndroidOperationResult()
:	fHasSucceeded(false),
	fErrorMessageReference(NULL, NULL)
{
}

/// Creates a result object with the given settings.
/// <br>
/// This is a protected constructor that can only be called by this class' static functions
/// or this class' derived classes.
/// @param allocatorPointer Allocator needed to create this object. Cannot be NULL.
/// @param hasSucceeded Set to true to indicate that the operation was a success. Set to false if failed.
/// @param errorMessage Message describing the error that occurred during the operation.
///                     <br>
///                     Set to NULL if no error occurred or if no error message can be provided.
AndroidOperationResult::AndroidOperationResult(
	Rtt_Allocator *allocatorPointer, bool hasSucceeded, const char *errorMessage)
:	fHasSucceeded(hasSucceeded),
	fErrorMessageReference(allocatorPointer, errorMessage ? Rtt_NEW(allocatorPointer, Rtt::String(allocatorPointer, errorMessage)) : NULL)
{
}

/// Destroys this result object.
AndroidOperationResult::~AndroidOperationResult()
{
}


// ----------------------------------------------------------------------------
// Public Member Functions
// ----------------------------------------------------------------------------

/// Determines if the operation has succeeded.
/// @return Returns true if the operation was successful. Returns false if failed.
bool AndroidOperationResult::HasSucceeded() const
{
	return fHasSucceeded;
}

/// Determines if the operation failed.
/// @return Returns false if the operation failed. Returns true if succeeded.
bool AndroidOperationResult::HasFailed() const
{
	return !fHasSucceeded;
}

/// Gets a message describing why the operation failed.
/// <br>
/// This function is only expected to be called if HasSucceeded() returns false.
/// @return Returns a string describing the error.
///         <br>
///         Returns NULL if the operation was successful or if a failure message could not be provided.
const char* AndroidOperationResult::GetErrorMessage() const
{
	if (fErrorMessageReference.IsNull())
	{
		return NULL;
	}
	return (*fErrorMessageReference).GetString();
}


// ----------------------------------------------------------------------------
// Public Static Functions
// ----------------------------------------------------------------------------

/// Creates a success result object whose HasSucceeded() function returns true.
/// @param allocatorPointer Allocator needed to create this object. Cannot be NULL.
/// @return Returns a success result object.
AndroidOperationResult AndroidOperationResult::Succeeded(Rtt_Allocator *allocatorPointer)
{
	return AndroidOperationResult(allocatorPointer, true, NULL);
}

/// Creates a failure result object with the given error message.
/// @param allocatorPointer Allocator needed to create this object. Cannot be NULL.
/// @param errorMessage Message describing the error that occurred during the operation. Can be NULL.
/// @return Returns a failure result object.
AndroidOperationResult AndroidOperationResult::FailedWith(
	Rtt_Allocator *allocatorPointer, const char *errorMessage)
{
	return AndroidOperationResult(allocatorPointer, false, errorMessage);
}
