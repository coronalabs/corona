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
