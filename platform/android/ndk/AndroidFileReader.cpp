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
#include <errno.h>
#include "AndroidFileReader.h"
#include "Core/Rtt_Build.h"


// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a new file reader.
AndroidFileReader::AndroidFileReader(Rtt_Allocator *allocatorPointer)
:	AndroidBinaryReader(allocatorPointer),
	fFilePath(allocatorPointer),
	fFilePointer(NULL)
{
}

/// Destroys this reader.
AndroidFileReader::~AndroidFileReader()
{
	Close();
}


// ----------------------------------------------------------------------------
// Public Member Functions
// ----------------------------------------------------------------------------

AndroidOperationResult AndroidFileReader::Open(const char *filePath)
{
	// Validate argument.
	if (Rtt_StringIsEmpty(filePath))
	{
		return AndroidOperationResult::FailedWith(GetAllocator(), "Invalid argument.");
	}

	// If this reader is currently open, then close it.
	Close();

	// Attempt to open the given file.
	fFilePointer = fopen(filePath, "rb");
	if (!fFilePointer)
	{
		int errorNumber = errno;
		if (errorNumber != 0)
		{
			return AndroidOperationResult::FailedWith(GetAllocator(), strerror(errorNumber));
		}
		else
		{
			char message[512];
			snprintf(message, sizeof(message), "Failed to open file \"%s\".", filePath);
			return AndroidOperationResult::FailedWith(GetAllocator(), message);
		}
	}

	// File was opened successfully.
	// Store the given path and return a success result.
	fFilePath.Set(filePath);
	return AndroidOperationResult::Succeeded(GetAllocator());
}

AndroidOperationResult AndroidFileReader::Open(FILE *filePointer)
{
	// Validate argument.
	if (!filePointer)
	{
		return AndroidOperationResult::FailedWith(GetAllocator(), "Invalid argument.");
	}

	// If this reader is currently open, then close its connection with the last file,
	// but only if given a new a file.
	if (fFilePointer && (fFilePointer != filePointer))
	{
		Close();
	}

	// Store the given file pointer.
	fFilePointer = filePointer;

	// Return a success result.
	return AndroidOperationResult::Succeeded(GetAllocator());
}

void AndroidFileReader::Close()
{
	// Close the file if this reader opened it.
	// Note: This reader only owns the file if the path is known.
	if (fFilePointer && !fFilePath.IsEmpty())
	{
		fclose(fFilePointer);
	}

	// Reset member variable.
	fFilePath.Set(NULL);
	fFilePointer = NULL;
}

bool AndroidFileReader::IsOpen()
{
	return fFilePointer ? true : false;
}


// ----------------------------------------------------------------------------
// Protected Member Functions
// ----------------------------------------------------------------------------

AndroidBinaryReadResult AndroidFileReader::OnStreamTo(U8 *bytes, U32 count)
{
	// Read the requested bytes from file.
	U32 bytesRead = (U32)fread(bytes, 1, (size_t)count, fFilePointer);

	// If less bytes were read than expected, then check if an error occurred.
	// Note: This can also happen if we've reached the end of the file, which is not an error.
	if (bytesRead < (size_t)count)
	{
		int errorNumber = errno;
		if (ferror(fFilePointer) && errorNumber)
		{
			Close();
			return AndroidBinaryReadResult::FailedWith(GetAllocator(), strerror(errorNumber));
		}
	}

	// Check if there are more bytes to read from the file.
	bool hasBytesRemaining = (feof(fFilePointer) == 0) ? true : false;

	// Return the read result.
	return AndroidBinaryReadResult::SucceededWith(GetAllocator(), bytesRead, hasBytesRemaining);
}
