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
