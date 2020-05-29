//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

/**
 * Encapsulates a Windows read/only file, opened for read using memory mapping.
 */

#ifndef _WinFile_H_
#define _WinFile_H_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define MAP_FAILED NULL  // check return value of GetContents() against this.

class WinFile
{
	HANDLE mFileHandle;
	HANDLE mFileMapping;
	LPVOID mFileData;
	DWORD mFileSize;

public:
	WinFile();
	~WinFile();

	void Open(const char * path, const char *mode = "r", DWORD size_high = 0, DWORD size_low = 0 );
	void Close();

	DWORD GetFileSize() const
	{
		return mFileSize;
	}

	const void * GetContents() const
	{
		return mFileData;
	}

	HANDLE GetFileHandle() const
	{
		return mFileHandle;
	}

	bool IsOpen() const
	{
		return mFileData != NULL;
	}
};

#endif
