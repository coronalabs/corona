//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "AndroidBinaryReadResult.h"


// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a failure result object without an error message.
AndroidBinaryReadResult::AndroidBinaryReadResult()
:	AndroidOperationResult(),
	fBytesRead(0),
	fHasBytesRemaining(false)
{
}

/// Creates a result object with the given settings.
/// <br>
/// This is a protected constructor that can only be called by this class' static functions.
/// @param allocatorPointer Allocator needed to create this object. Cannot be NULL.
/// @param hasSucceeded Set to true to indicate that the operation was a success. Set to false if failed.
/// @param bytesRead Set to the number of bytes read.
/// @param hasBytesRemaining Set to true if there are more bytes to be read from the binary stream.
///                          <br>
///                          Set to false if no more bytes are available.
/// @param errorMessage Message describing the error that occurred during the binary operation.
///                     <br>
///                     Set to NULL if no error occurred or if no error message can be provided.
AndroidBinaryReadResult::AndroidBinaryReadResult(
	Rtt_Allocator *allocatorPointer, bool hasSucceeded,
	U32 bytesRead, bool hasBytesRemaining, const char *errorMessage)
:	AndroidOperationResult(allocatorPointer, hasSucceeded, errorMessage),
	fBytesRead(bytesRead),
	fHasBytesRemaining(hasBytesRemaining)
{
}

/// Destroys this result object.
AndroidBinaryReadResult::~AndroidBinaryReadResult()
{
}


// ----------------------------------------------------------------------------
// Public Member Functions
// ----------------------------------------------------------------------------

/// Determines if the binary stream has more bytes available to be read.
/// @return Returns true if more bytes can be read from the binary stream.
///         <br>
///         Returns false if there are no more bytes to be read.
bool AndroidBinaryReadResult::HasBytesRemaining() const
{
	return fHasBytesRemaining;
}

/// Gets the number of bytes read from the stream.
/// @return Returns the number of bytes read. Note that this might be less than the number of bytes
///         requested to be read from the stream, which typically happens if there are no more bytes
///         available. For example, this can happen when you reach the end of a file.
///         <br>
///         Returns zero if no bytes were read from the stream.
U32 AndroidBinaryReadResult::GetBytesRead() const
{
	return fBytesRead;
}


// ----------------------------------------------------------------------------
// Public Static Functions
// ----------------------------------------------------------------------------

/// Creates a success result object whose HasSucceeded() function returns true.
/// @param allocatorPointer Allocator needed to create this object. Cannot be NULL.
/// @param bytesRead Set to the number of bytes read.
/// @param hasBytesRemaining Set to true if there are more bytes to be read from the binary stream.
///                          <br>
///                          Set to false if no more bytes are available.
/// @return Returns a success result object.
AndroidBinaryReadResult AndroidBinaryReadResult::SucceededWith(
	Rtt_Allocator *allocatorPointer, U32 bytesRead, bool hasBytesRemaining)
{
	return AndroidBinaryReadResult(allocatorPointer, true, bytesRead, hasBytesRemaining, NULL);
}

/// Creates a failure result object with the given error message.
/// @param allocatorPointer Allocator needed to create this object. Cannot be NULL.
/// @param errorMessage Message describing the error that occurred during the operation. Can be NULL.
/// @return Returns a failure result object.
AndroidBinaryReadResult AndroidBinaryReadResult::FailedWith(
	Rtt_Allocator *allocatorPointer, const char *errorMessage)
{
	return AndroidBinaryReadResult(allocatorPointer, false, 0, false, errorMessage);
}
