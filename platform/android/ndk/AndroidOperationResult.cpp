//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
