//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "AndroidBinaryReader.h"
#include "Core/Rtt_Build.h"


// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a new reader.
AndroidBinaryReader::AndroidBinaryReader(Rtt_Allocator *allocatorPointer)
:	fAllocatorPointer(allocatorPointer)
{
}

/// Destroys this reader.
AndroidBinaryReader::~AndroidBinaryReader()
{
}


// ----------------------------------------------------------------------------
// Public Member Functions
// ----------------------------------------------------------------------------

Rtt_Allocator* AndroidBinaryReader::GetAllocator() const
{
	return fAllocatorPointer;
}

bool AndroidBinaryReader::IsClosed()
{
	return !IsOpen();
}

AndroidBinaryReadResult AndroidBinaryReader::StreamTo(U8 *value)
{
	return StreamTo(value, 1);
}

AndroidBinaryReadResult AndroidBinaryReader::StreamTo(U8 *bytes, U32 count)
{
	// Validate arguments.
	if (!bytes)
	{
		return AndroidBinaryReadResult::FailedWith(fAllocatorPointer, "Argument 'bytes' cannot be NULL.");
	}
	if (0 == count)
	{
		return AndroidBinaryReadResult::FailedWith(fAllocatorPointer, "Argument 'count' must be >= 0.");
	}

	// Do not continue if this reader is closed.
	if (IsClosed())
	{
		return AndroidBinaryReadResult::FailedWith(fAllocatorPointer, "Reader is closed.");
	}

	// Read the requested bytes.
	return OnStreamTo(bytes, count);
}
