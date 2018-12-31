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

#include "stdafx.h"
#include "WinFile.h"
#include "WinString.h"

WinFile::WinFile()
{
	mFileHandle = INVALID_HANDLE_VALUE;
	mFileMapping = INVALID_HANDLE_VALUE;
	mFileData = NULL;
	mFileSize = 0;
}

WinFile::~WinFile()
{
	Close();
}

void 
WinFile::Open(const char * path, const char *mode /* = "r" */, 
              DWORD size_high /* = 0 */, DWORD size_low /* = 0 */ )
{
	WinString	filePath;

	filePath.SetUTF8( path );

//	BY_HANDLE_FILE_INFORMATION fileInfo;  unused

    // defaults set for reading, mode == "r"
    DWORD dwDesiredAccess = GENERIC_READ;
    DWORD dwShareMode = FILE_SHARE_READ;
    DWORD flProtect = PAGE_READONLY;
    DWORD dwMapDesiredAccess = FILE_MAP_READ;

    if( mode == "w" )
	{
		dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
		dwShareMode = FILE_SHARE_READ;  // other processes can open for read
		flProtect = PAGE_READWRITE;
		dwMapDesiredAccess = FILE_MAP_WRITE;

	}

	mFileHandle = ::CreateFile(
		filePath.GetTCHAR(),
		dwDesiredAccess,
		dwShareMode,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL
		);
	if ( mFileHandle == INVALID_HANDLE_VALUE ) {
		DWORD err = ::GetLastError();
		return;
	}

    if( mode == "w" )
 	{
	}

	mFileMapping = ::CreateFileMapping(
		mFileHandle,
		NULL,
		flProtect,
		size_high,
		size_low,
		NULL
		);
	if ( mFileMapping == NULL ) {
		DWORD err = ::GetLastError();
		return;
	}

	mFileData = ::MapViewOfFile(
		mFileMapping,
		dwMapDesiredAccess,
		0,
		0,
		0 );
	if ( mFileData == NULL ) {
		DWORD err = ::GetLastError();
		return;
	}

	mFileSize = ::GetFileSize(
		mFileHandle,
		0 );
}

void
WinFile::Close()
{
	if ( mFileData != NULL ) {
		::UnmapViewOfFile(mFileData);
		mFileData = NULL;
	}
	if ( mFileMapping != INVALID_HANDLE_VALUE ) {
		::CloseHandle(mFileMapping);
		mFileMapping = INVALID_HANDLE_VALUE;
	}
	if ( mFileHandle != INVALID_HANDLE_VALUE ) {
		::CloseHandle(mFileHandle);
		mFileHandle = INVALID_HANDLE_VALUE;
	}
}

