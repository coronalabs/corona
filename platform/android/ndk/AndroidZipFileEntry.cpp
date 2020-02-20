//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "AndroidZipFileEntry.h"


// ----------------------------------------------------------------------------
// Constructors/Destructors
// ----------------------------------------------------------------------------

/// Creates a new zip entry information object.
AndroidZipFileEntry::AndroidZipFileEntry(Rtt_Allocator *allocatorPointer)
:	fPackageFilePath(allocatorPointer),
	fEntryName(allocatorPointer),
	fByteOffsetInPackage(0),
	fByteCountInPackage(0),
	fIsCompressed(false)
{
}

/// Destroys this zip entry object.
AndroidZipFileEntry::~AndroidZipFileEntry()
{
}


// ----------------------------------------------------------------------------
// Public Member Functions
// ----------------------------------------------------------------------------

/// Gets the name and path to the zip file which contains the zip entry.
/// @return Returns the zip file's path and file name.
///         <br>
///         Returns NULL or empty string if the zip file has not been specified.
const char* AndroidZipFileEntry::GetPackageFilePath() const
{
	return fPackageFilePath.GetString();
}

/// Sets the path and name of the zip file which contains the zip entry.
/// @param path The path and file name of the zip file. Can be NULL or empty.
void AndroidZipFileEntry::SetPackageFilePath(const char *path)
{
	fPackageFilePath.Set(path);
}

/// Gets the unique name of the entry within the zip file.
/// @return Returns the zip file entry's unique name.
///         <br>
///         Returns NULL or empty string if the entry name was not specified.
const char* AndroidZipFileEntry::GetEntryName() const
{
	return fEntryName.GetString();
}

/// Sets the unique name of the entry within the zip file.
/// @param name The unique name of the zip file entry. Can be NULL or empty.
void AndroidZipFileEntry::SetEntryName(const char *name)
{
	fEntryName.Set(name);
}

/// Gets the index to the first byte within the zip file where the entry's bytes can be found.
/// @return Returns the index to the entry's first byte within the zip file.
long AndroidZipFileEntry::GetByteOffsetInPackage() const
{
	return fByteOffsetInPackage;
}

/// Sets the index to the first byte within the zip file where the entry's bytes can be found.
/// @param value Index to the first byte within the zip file where the entry's bytes can be found.
void AndroidZipFileEntry::SetByteOffsetInPackage(long value)
{
	if (value < 0)
	{
		value = 0;
	}
	fByteOffsetInPackage = value;
}

/// Gets the number of bytes that the entry takes within the zip file.
/// <br>
/// This byte count is expected to be used in conjunction with the byte offset when reading
/// the entry within the zip file.
/// @return Returns the number of bytes the entry takes within the zip file.
///         If the entry is compressed, then this is the compressed byte count.
long AndroidZipFileEntry::GetByteCountInPackage() const
{
	return fByteCountInPackage;
}

/// Sets the number of bytes that the entry takes within the zip file.
/// @param value The number of bytes the entry takes within the zip file.
void AndroidZipFileEntry::SetByteCountInPackage(long value)
{
	if (value < 0)
	{
		value = 0;
	}
	fByteCountInPackage = value;
}

/// Determines if the entry is compressed within the zip file.
/// @return Returns true if the entry is compressed within the zip file.
///         This means that the entry must be decompressed in order to access its bytes.
///         <br>
///         Returns false if the entry is not compressed.
///         This means that the entry can be read directly without a zip decompressor for fast access.
bool AndroidZipFileEntry::IsCompressed() const
{
	return fIsCompressed;
}

/// Sets whether or not the entry is compressed within the zip file.
/// @param Set to true if the entry is compressed. Set to false if not.
void AndroidZipFileEntry::SetIsCompressed(bool value)
{
	fIsCompressed = value;
}

