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

#include "Rtt_FileSystem.h"
#include "Rtt_Assert.h"
#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"
#include "Core/Rtt_Data.h"

#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#if defined(Rtt_NINTENDO_ENV )
	#include <io.h>
	#include <share.h>
	#include <windows.h>
	#include <direct.h>
	#include <map>
	#include <memory>
	std::map< void*, std::shared_ptr<nnFile> > sOpenFiles;

#elif defined( Rtt_WIN_ENV )
	#include <io.h>
	#include <share.h>
	#include <windows.h>
	#include <direct.h>
	#ifdef Rtt_WIN_DESKTOP_ENV
		#include <Shlobj.h>
		#include <Shlwapi.h>
		#include <map>
	#endif
#else
	#include <fcntl.h>
	#include <unistd.h>
	#include <sys/mman.h>
	#include <dirent.h>
	#include <sys/stat.h>
	#include <errno.h>

#if defined(_WIN32)
	#include <direct.h>   // _mkdir
#endif

#endif // Rtt_WIN_ENV

#ifdef Rtt_MAC_ENV
// These Carbon calls are deprecated as of 10.8 but the only viable replacements are Cocoa only
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <Carbon/Carbon.h>

#endif


// ----------------------------------------------------------------------------
// Private Functions
// ----------------------------------------------------------------------------

#if defined( Rtt_WIN_ENV ) || defined(Rtt_NINTENDO_ENV )

#ifdef Rtt_WIN_DESKTOP_ENV
	// Stores a collection of Win32 related memory mapped file info so that it can be unmapped later.
	// Key:		Pointer to the memory mapped file buffer.
	// Value:	HANDLE returned by the Win32 CreateFileMapping() function.
	typedef std::map<const void*, HANDLE> WindowsMemoryMappedInfoCollection;
	typedef std::pair<const void*, HANDLE> WindowsMemoryMappedInfoPair;
	static WindowsMemoryMappedInfoCollection sWindowsMemoryMappedInfoCollection;
#endif // Rtt_WIN_DESKTOP_ENV

static void ConvertUtf16ToUtf8(const wchar_t* utf16String, char* utf8String, size_t utf8StringLength)
{
	int conversionLength;

	if (utf16String && utf8String && (utf8StringLength > 0))
	{
		conversionLength = WideCharToMultiByte(CP_UTF8, 0, utf16String, -1, NULL, 0, NULL, NULL);
		if ((conversionLength > 0) && (conversionLength <= (int)utf8StringLength))
		{
			WideCharToMultiByte(CP_UTF8, 0, utf16String, -1, utf8String, conversionLength, NULL, NULL);
		}
	}
}

static void ConvertUtf8ToUtf16(const char* utf8String, wchar_t* utf16String, size_t utf16StringLength)
{
	size_t utf8StringLength;
	int conversionLength;

	utf8StringLength = strlen(utf8String);
	if (utf16String && (utf8StringLength > 0))
	{
		conversionLength = MultiByteToWideChar(CP_UTF8, 0, utf8String, utf8StringLength + 1, NULL, 0);
		if ((conversionLength > 0) && (conversionLength <= (int)utf16StringLength))
		{
			MultiByteToWideChar(CP_UTF8, 0, utf8String, utf8StringLength + 1, utf16String, conversionLength);
		}
	}
}

static wchar_t* CreateUtf16StringFrom(const char* utf8String)
{
	wchar_t *utf16String = NULL;
	int conversionLength;

	if (utf8String)
	{
		conversionLength = MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, NULL, 0);
		if (conversionLength > 0)
		{
			utf16String = (wchar_t*)malloc((size_t)conversionLength * sizeof(wchar_t));
			MultiByteToWideChar(CP_UTF8, 0, utf8String, -1, utf16String, conversionLength);
		}
	}
	return utf16String;
}

static void DestroyUtf16String(wchar_t *utf16String)
{
	if (utf16String)
	{
		free(utf16String);
	}
}

#endif // Rtt_WIN_ENV


// ----------------------------------------------------------------------------
// Public Functions
// ----------------------------------------------------------------------------

Rtt_EXPORT FILE* Rtt_FileOpen(const char *filePath, const char *mode)
{
	FILE *fileHandle = NULL;

#if defined( Rtt_WIN_ENV )
	wchar_t *utf16Filename = CreateUtf16StringFrom(filePath);
	wchar_t *utf16Mode = CreateUtf16StringFrom(mode);
	errno_t errorCode = _wfopen_s(&fileHandle, utf16Filename, utf16Mode);
	DestroyUtf16String(utf16Filename);
	DestroyUtf16String(utf16Mode);
#elif defined(Rtt_NINTENDO_ENV)
	nnFile* nnf = new nnFile(filePath, mode);
	std::shared_ptr<nnFile> f(nnf);
	if (f->isOpen())
	{
		sOpenFiles[nnf] = f;
		return (FILE*) nnf;
	}
	return NULL;
#else
	fileHandle = fopen(filePath, mode);
#endif // Rtt_WIN_ENV

	return fileHandle;
}

Rtt_EXPORT int Rtt_FileFlush(FILE* fileHandle)
{
#if defined(Rtt_NINTENDO_ENV)
	// TODO
	return 0;
#else
	return fflush(fileHandle);
#endif
}

Rtt_EXPORT size_t Rtt_FileRead(void* ptr, size_t size, size_t count, FILE* fileHandle)
{
#if defined(Rtt_NINTENDO_ENV)
	auto it = sOpenFiles.find(fileHandle);
	if (it != sOpenFiles.end())
	{
		nnFile& f = *it->second;
		f.load();

		int bytesToRead = size * count;
		if (bytesToRead > 0)
		{
			int bytesInFile = f.fSize - f.fPos;
			int n = bytesToRead;
			if (bytesToRead > bytesInFile)
			{
				count = int(bytesInFile / size);
				n = size * count;
			}

			memcpy(ptr, f.fData + f.fPos, n);
//			printf("read %s: pos=%ld size=%lld bytesToRead=%d bytesInFile=%d, n=%d\n", f.fPath.c_str(), f.fPos, f.fSize, bytesToRead, bytesInFile, n);
			f.fPos += n;
			return count;
		}
		// If either size or count is zero, the function returns zero and both the stream state and the content pointed by ptr remain unchanged.
		return 0;
	}
	return -1;
#else
	return fread(ptr, size, count, fileHandle);
#endif
}

Rtt_EXPORT int Rtt_FileEof(FILE* fileHandle)
{
#if defined(Rtt_NINTENDO_ENV)
	auto it = sOpenFiles.find(fileHandle);
	if (it != sOpenFiles.end())
	{
		nnFile& f = *it->second;
		f.load();

		return f.fPos < f.fSize ? 0 : 1;
	}
	return -1;
#else
	return feof(fileHandle);
#endif
}

Rtt_EXPORT int Rtt_FileClose(FILE *fileHandle)
{
#if defined(Rtt_NINTENDO_ENV)
	auto it = sOpenFiles.find(fileHandle);
	if (it != sOpenFiles.end())
	{
		const nnFile& f = *it->second;
//		printf("close %s\n", f.fPath.c_str());
		sOpenFiles.erase(it);
	}
	return 0;
#else
	return fclose(fileHandle);
#endif
}

Rtt_EXPORT int Rtt_FileError(FILE *fileHandle)
{
#if defined(Rtt_NINTENDO_ENV)
	return 0; // todo
#else
	return ferror(fileHandle);
#endif
}

Rtt_EXPORT int Rtt_FileGetC(FILE *fileHandle)
{
#if defined(Rtt_NINTENDO_ENV)
	auto it = sOpenFiles.find(fileHandle);
	if (it != sOpenFiles.end())
	{
		return it->second->getc();
	}
	return -1;
#else
	return getc(fileHandle);
#endif
}

Rtt_EXPORT int Rtt_FileUngetC(int c, FILE *fileHandle)
{
#if defined(Rtt_NINTENDO_ENV)
	auto it = sOpenFiles.find(fileHandle);
	if (it != sOpenFiles.end())
	{
		return it->second->nnungetc(c);
	}
	return -1;
#else
	return ungetc(c, fileHandle);
#endif
}

Rtt_EXPORT void Rtt_FileClearerr(FILE *fileHandle)
{
#if defined(Rtt_NINTENDO_ENV)
	auto it = sOpenFiles.find(fileHandle);
	if (it != sOpenFiles.end())
	{
		nnFile& f = *it->second;
	}
#else
	clearerr(fileHandle);
#endif
}

Rtt_EXPORT long int Rtt_FileTell(FILE *fileHandle)
{
#if defined(Rtt_NINTENDO_ENV)
	auto it = sOpenFiles.find(fileHandle);
	if (it != sOpenFiles.end())
	{
		nnFile& f = *it->second;
		f.load();

		return f.fPos;
	}
	return -1;
#else
	return ftell(fileHandle);
#endif
}

Rtt_EXPORT int Rtt_FileSeek(FILE *fileHandle, long int offset, int origin)
{
#if defined(Rtt_NINTENDO_ENV)
	auto it = sOpenFiles.find(fileHandle);
	if (it != sOpenFiles.end())
	{
		nnFile& f = *it->second;
		f.load();

		switch (origin)
		{
		case 0:	// SEEK_SET
			f.fPos = offset;
			break;
		case 1:	// SEEK_CUR
			f.fPos = f.fPos + offset;
			break;
		case 2:	// SEEK_END
			f.fPos = f.fSize + offset;
			break;
		}

		if (f.fPos<0)
		{
			f.fPos = 0;
		}
//		printf("seek %s: newpos=%ld size=%lld offset=%ld origin=%d\n", f.fPath.c_str(), f.fPos, f.fSize, offset, origin);
		return 0;
	}
	return -1;
#else
	return fseek(fileHandle, offset, origin);
#endif
}

Rtt_EXPORT void Rtt_FileRewind(FILE *fileHandle)
{
#if defined(Rtt_NINTENDO_ENV)
	auto it = sOpenFiles.find(fileHandle);
	if (it != sOpenFiles.end())
	{
		nnFile& f = *it->second;
		f.load();

		f.fPos = 0;
	}
#else
	rewind(fileHandle);
#endif
}

Rtt_EXPORT int Rtt_FileDescriptorOpen(const char *filePath, int oflags, int pmode)
{
	int fileDescriptor = -1;

#ifdef Rtt_WIN_ENV
	wchar_t *utf16Filename = CreateUtf16StringFrom(filePath);
	errno_t errorCode = _wsopen_s(&fileDescriptor, utf16Filename, oflags, SH_DENYWR, pmode);
	DestroyUtf16String(utf16Filename);
#else
	fileDescriptor = open(filePath, oflags, pmode);
#endif // Rtt_WIN_ENV

	return fileDescriptor;
}

Rtt_EXPORT void Rtt_FileDescriptorClose(int fileDescriptor)
{
#ifdef Rtt_WIN_ENV
	_close(fileDescriptor);
#else
	close(fileDescriptor);
#endif
}

#ifdef Rtt_WIN_ENV
Rtt_EXPORT int Rtt_FileIsHidden(const char *filePath)
{
	int isHidden = 0;
	if (filePath && (filePath[0] != '\0'))
	{
		WIN32_FILE_ATTRIBUTE_DATA attributes;
		wchar_t *utf16FilePath = CreateUtf16StringFrom(filePath);
		BOOL wasFound = GetFileAttributesExW(utf16FilePath, GetFileExInfoStandard, &attributes);
		if (wasFound && (attributes.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
		{
			isHidden = 1;
		}
		DestroyUtf16String(utf16FilePath);
	}
	return isHidden;
}
#endif

Rtt_EXPORT int Rtt_FileStatus(const char *filePath, struct stat *buffer)
{
#if defined( Rtt_WIN_ENV ) || defined( Rtt_NINTENDO_ENV )
	wchar_t *utf16FilePath = CreateUtf16StringFrom(filePath);
	int result = _wstat(utf16FilePath, (struct _stat*)buffer);
	DestroyUtf16String(utf16FilePath);
#else
	int result = lstat(filePath, buffer);
#endif // Rtt_WIN_ENV

	return result;
}

Rtt_EXPORT int Rtt_FileExists(const char *filePath)
{
#ifdef Rtt_WIN_ENV
	WIN32_FILE_ATTRIBUTE_DATA attributes;
	wchar_t *utf16Filename = CreateUtf16StringFrom(filePath);
	BOOL wasFound = GetFileAttributesExW(utf16Filename, GetFileExInfoStandard, &attributes);
	DestroyUtf16String(utf16Filename);
	return wasFound ? 1 : 0;
#else
	struct stat buffer;

	return (int(stat (filePath, &buffer) == 0));
#endif // Rtt_WIN_ENV
}

//
// Determine if any part of the file path specified in the Lua code differs
// in case from the actual path on disk.  This is important because filesystems on
// devices are typically case-sensitive while those on desktops are not.  If the
// file on disk does not have the same case, then "actualPath" is set to an allocated
// copy of the actual name (the caller should free this)
//
Rtt_EXPORT int Rtt_FileExistsWithSameCase(const char *filePath, const char *rootPath, char **actualPath )
{
	int result = 1;

#ifdef Rtt_WIN_DESKTOP_ENV

	wchar_t *wRootPath = NULL;
	char filesystemName[MAX_PATH];

	if (rootPath == NULL)
	{
		// Due to the way Windows handles path elements like "Shared Folders" (which has no "filename") 
		// a rootPath must be given so we know when to stop iterating over the filePath.  Default to
		// the filePath minus the last component
		wRootPath = CreateUtf16StringFrom(filePath);
		PathRemoveFileSpecW(wRootPath);
	}
	else
	{
		wRootPath = CreateUtf16StringFrom(rootPath);
	}

	if (filePath == NULL)
	{
		// Degenerate case, assume success
		return result;
	}

	wchar_t *wFilePath = CreateUtf16StringFrom(filePath);

	// Normalize path separators (required to use certain Win32 APIs)
	wchar_t *ptr = wFilePath;
	while ((ptr = wcschr(ptr, L'/')) != NULL)
	{
		*ptr = L'\\';
	}

	// Loop backwards through the elements of the path until we reach the root (typically the project's resource directory)
	while (wcscmp(wRootPath, wFilePath) != 0)
	{
		// Separate the filename from its parent directory's path
		LPWSTR wFileName = ::PathFindFileNameW(wFilePath);
		WIN32_FIND_DATAW data = { };
		SetLastError(ERROR_SUCCESS); // needed to ensure subsequent check of GetLastError() is reliable
		HANDLE hFind = FindFirstFileW(wFilePath, &data);

		FindClose(hFind); // we don't need the iterator

		// If we can't find the file at all, return failure
		if (GetLastError() == ERROR_FILE_NOT_FOUND)
		{
			result = 0;
			break;
		}
		else if (!data.cFileName || (L'\0' == data.cFileName[0]))
		{
			// The string is empty if we've reached the end of the path.
			// This means that the casing of the filename and its subdirectories are correct.
			// Note: We'll only end up here if argument "filePath" is not prefixed with "rootPath".
			break;
		}
		else if ((wcscmp(wFileName, L".") == 0) || (wcscmp(wFileName, L"..") == 0))
		{
			// Skip over a "." or ".." in the path.
			// We only need to validate directory and file names.
		}
		else
		{
			// The magic happens here: the filename returned by FindFirstFile() will have the actual case
			// used in the filesystem, if it doesn't match what was in the path we have a case difference
			if (wcscmp(wFileName, data.cFileName) != 0)
			{
				result = 0;
				ConvertUtf16ToUtf8(data.cFileName, filesystemName, MAX_PATH);
				break;
			}
		}

		// PathRemoveFileSpec is deprecated in favor of PathCchRemoveFileSpec but that isn't in XP
		if (! ::PathRemoveFileSpecW(wFilePath))
		{
			break;
		}
	}

	DestroyUtf16String(wFilePath);
	DestroyUtf16String(wRootPath);

	if (result == 0 && actualPath != NULL)
	{
		*actualPath = strdup(filesystemName);
	}

#elif Rtt_MAC_ENV

	FSRef ref;
	OSStatus status;
	unsigned char filesystemName[PATH_MAX + 1] = { 0 };

	// first get an FSRef for the path
	status = FSPathMakeRef((unsigned char *)filePath, &ref, NULL);

	if (status == 0)
	{
		// then get a path from the FSRef
		status = FSRefMakePath(&ref, filesystemName, PATH_MAX);

		if (status != 0 || strcmp((char *)filesystemName, filePath) != 0)
		{
			result = 0;
		}
	}

	if (result == 0 && status == 0 && actualPath != NULL)
	{
		*actualPath = strdup((char *)filesystemName);
	}

#else
	// Just determine file existence on other platforms
	result = Rtt_FileExists(filePath);

	if (result == 0 && actualPath != NULL)
	{
		*actualPath = strdup(filePath);
	}

#endif // Rtt_WIN_ENV

	return result;
}

#ifndef Rtt_WIN_PHONE_ENV
Rtt_EXPORT void* Rtt_FileMemoryMap(int fileDescriptor, size_t byteOffset, size_t byteCount, int canWrite)
{
	void *memoryMapPointer = NULL;
	if (byteCount > 0)
	{
#if defined( Rtt_WIN_DESKTOP_ENV )
		HANDLE fileHandle = (HANDLE)_get_osfhandle(fileDescriptor);
		if (fileHandle != INVALID_HANDLE_VALUE)
		{
			DWORD protectionMode = canWrite ? PAGE_READWRITE : PAGE_READONLY;
			HANDLE mappingHandle = ::CreateFileMapping(fileHandle, NULL, protectionMode, 0, 0, NULL);
			if (mappingHandle)
			{
				DWORD accessMode = canWrite ? FILE_MAP_WRITE : FILE_MAP_READ;
				memoryMapPointer = ::MapViewOfFile(
					mappingHandle, accessMode, HIWORD(byteOffset), LOWORD(byteOffset), byteCount);
				if (memoryMapPointer)
				{
					sWindowsMemoryMappedInfoCollection.insert(WindowsMemoryMappedInfoPair(memoryMapPointer, mappingHandle));
				}
				else
				{
					::CloseHandle(mappingHandle);
				}
			}
		}
#elif defined( Rtt_NINTENDO_ENV )
		Rtt_LogException("Rtt_FileMemoryMap: TODO\n");
#else
		int accessFlags = PROT_READ;
		if (canWrite)
		{
			accessFlags |= PROT_WRITE;
		}
		memoryMapPointer = mmap(NULL, byteCount, accessFlags, MAP_SHARED, fileDescriptor, byteOffset);
		if (MAP_FAILED == memoryMapPointer)
		{
			memoryMapPointer = NULL;
		}
#endif // Rtt_WIN_DESKTOP_ENV
	}

	return memoryMapPointer;
}

Rtt_EXPORT void Rtt_FileMemoryUnmap(const void *memoryMapPointer, size_t byteCount)
{
	if (memoryMapPointer)
	{
#if defined( Rtt_WIN_DESKTOP_ENV )
		WindowsMemoryMappedInfoCollection::iterator iter = sWindowsMemoryMappedInfoCollection.find(memoryMapPointer);
		if (iter != sWindowsMemoryMappedInfoCollection.end())
		{
			::UnmapViewOfFile(memoryMapPointer);
			::CloseHandle((*iter).second);
			sWindowsMemoryMappedInfoCollection.erase(iter);
		}
#elif defined( Rtt_NINTENDO_ENV )
		Rtt_LogException("Rtt_FileMemoryUnmap: TODO\n");
#else
		munmap((void*)memoryMapPointer, byteCount);
#endif // Rtt_WIN_DESKTOP_ENV
	}
}

Rtt_EXPORT void Rtt_FileMemoryFlush(const void *memoryMapPointer, size_t byteCount)
{
	if (memoryMapPointer != NULL)
	{
#if defined( Rtt_WIN_DESKTOP_ENV )
		FlushViewOfFile(memoryMapPointer, byteCount);
#elif defined( Rtt_NINTENDO_ENV )
		Rtt_LogException("Rtt_FileMemoryFlush: TODO\n");
#else
		msync((void *)memoryMapPointer, byteCount, MS_SYNC);
#endif // Rtt_WIN_DESKTOP_ENV
	}
}

Rtt_EXPORT int Rtt_CopyFile(const char *srcFilePath, const char *dstFilePath)
{
	int result = 1;

#if defined( Rtt_WIN_ENV )
	wchar_t *utf16SrcFilePath = CreateUtf16StringFrom(srcFilePath);
	wchar_t *utf16DstFilePath = CreateUtf16StringFrom(dstFilePath);

	result = CopyFileW(utf16SrcFilePath, utf16DstFilePath, FALSE);

	DestroyUtf16String(utf16SrcFilePath);
	DestroyUtf16String(utf16DstFilePath);
#else
	FILE *inFp = NULL;
	FILE *outFp = NULL;

	if ((inFp = Rtt_FileOpen(srcFilePath, "rb")) == NULL)
	{
		printf("Rtt_CopyFile: failed to open '%s' for reading\n", srcFilePath);

		return 0;
	}
	
	if ((outFp = Rtt_FileOpen(dstFilePath, "wb")) == NULL)
	{
		printf("Rtt_CopyFile: failed to open '%s' for writing\n", dstFilePath);

		return 0;
	}

	char buf[BUFSIZ];

	size_t bytesRead = 0;
	while ((bytesRead = read(fileno(inFp), buf, BUFSIZ)) > 0)
	{
		if (write(fileno(outFp), buf, bytesRead) == 0)
		{
			Rtt_FileClose(inFp);
			Rtt_FileClose(outFp);

			return 0;
		}
	}

	Rtt_FileClose(inFp);
	Rtt_FileClose(outFp);

#endif

	return result;
}

// Adapted from https://stackoverflow.com/questions/675039/how-can-i-create-directory-tree-in-c-linux
Rtt_EXPORT int Rtt_MakeDirectory(const char *dirPath)
{
	int result = false;
	std::string path = dirPath;

#if defined(_WIN32)
		int ret = _mkdir(path.c_str());
#else
		mode_t mode = 0755;
		int ret = mkdir(path.c_str(), mode);
#endif
		if (ret == 0)
		{
			return true;
		}

		switch (errno)
		{
			case ENOENT:  // parent doesn't exist, try to create it
			{
				size_t pos = path.find_last_of('/');

				if (pos == std::string::npos)
#if defined(_WIN32)
				{
					pos = path.find_last_of('\\');
				}

				if (pos == std::string::npos)
#endif
				{
					return false;
				}
				if (!Rtt_MakeDirectory( path.substr(0, pos).c_str() ))
				{
					return false;
				}
			}
				// now, try to create again
#if defined(_WIN32)
				return 0 == _mkdir(path.c_str());
#else
				return 0 == mkdir(path.c_str(), mode);
#endif

			case EEXIST: // done!
				return true;

			default:
				return false;
		}

		return result;
}

Rtt_EXPORT int Rtt_DeleteFile(const char *filePath)
{
	int result = 0;

#ifdef Rtt_WIN_ENV
	std::wstring path(filePath, filePath + strlen(filePath));		// string ==> wstring
	result = DeleteFile(path.c_str());
#else
	result = unlink(filePath) == 0;
#endif

	return result;
}

Rtt_EXPORT int Rtt_IsDirectory(const char *dirPath)
{
	bool result = false;

#if defined( Rtt_NINTENDO_ENV )
	Rtt_LogException("Rtt_IsDirectory: TODO\n");
#else
	#ifdef Rtt_WIN_ENV
		wchar_t *path = CreateUtf16StringFrom(dirPath);
		result = ::PathIsDirectoryW(path) ? true : false;
		DestroyUtf16String(path);
	#else
		DIR *subdp = opendir(dirPath);
		if ( subdp )
		{
			closedir( subdp );
			subdp = NULL;
			result = true;
		}
	#endif
#endif
	return result;
}

Rtt_EXPORT int Rtt_DeleteDirectory(const char *dirPath)
{
	int result = 0;

	std::vector<std::string> fileList = Rtt_ListFiles(dirPath);

	for (std::vector<std::string>::iterator it = fileList.begin(); it != fileList.end(); ++it)
	{
		const char* path = it->c_str();

		if (strncmp(path, ".", 1) == 0 || strncmp(path, "..", 2) == 0)
		{
			continue;
		}

		if (Rtt_IsDirectory(path))
		{
			result = Rtt_DeleteDirectory(path);

			result = rmdir(path) == 0;
		}
		else
		{
			result = Rtt_DeleteFile(path);
		}

		if (! result)
		{
			fprintf(stderr, "Rtt_DeleteDirectory: failed for '%s'\n", path);

			break;
		}
	}

	result = rmdir(dirPath) == 0;

	return result;
}

std::vector<std::string> Rtt_ListFiles(const char *directoryName)
{
	std::vector<std::string> result;
#if defined(Rtt_NINTENDO_ENV )
	Rtt_ASSERT(0 && "TODO");
	return result;
#elif defined( Rtt_WIN_ENV )
	wchar_t *path = CreateUtf16StringFrom(directoryName);
	std::wstring search_path = std::wstring(path) + L"/*.*";
	WIN32_FIND_DATA fd;

	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd); 
	if(hFind != INVALID_HANDLE_VALUE)
	{ 
		do
		{ 
			char filename[MAX_PATH];
			ConvertUtf16ToUtf8(fd.cFileName, filename, MAX_PATH);

			if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0)
			{
				continue;
			}

			std::string path = directoryName;
			path.append("\\");
			path.append(filename);
			result.push_back(path);
	} while(::FindNextFile(hFind, &fd));

		::FindClose(hFind); 
	} 

	DestroyUtf16String(path);

#else
	DIR *dir;
	struct dirent *ent;

	if ((dir = opendir(directoryName)) == NULL)
	{
		return result;
	}

	while ((ent = readdir(dir)) != NULL)
	{
		std::string entry = ent->d_name;

		if (entry == "." || entry == "..")
		{
			continue;
		}

		std::string path = directoryName;

		path.append("/");
		path.append(entry);

		result.push_back( path );
	}

	closedir(dir);
#endif // Rtt_WIN_ENV

	return result;
}

size_t
Rtt_WriteDataToFile(const char *filename, Rtt::Data<const unsigned char> &data)
{
	size_t charsWritten = 0;

	FILE *fp = Rtt_FileOpen(filename, "wb");

	if (fp != NULL)
	{
		charsWritten = fwrite(data.GetData(), 1, data.GetLength(), fp);

		Rtt_ASSERT(charsWritten == (size_t)data.GetLength());
	}

	Rtt_FileClose(fp);

	return charsWritten;
}

#if defined(Rtt_WIN_ENV) && !defined(Rtt_LINUX_ENV)
static void ReplaceString(std::string& subject, const std::string& search, const std::string& replace)
{
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos)
	{
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}
#endif // Rtt_WIN_ENV

#if defined(Rtt_LINUX_ENV)
void ReplaceString(std::string& subject, const std::string& search, const std::string& replace)
{
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos)
	{
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
}
#endif // Rtt_WIN_ENV

Rtt_EXPORT char *Rtt_MakeTempDirectory(char *tmpDirTemplate)
{
#if defined(Rtt_WIN_ENV) || defined(Rtt_NINTENDO_ENV )

	static char utf8FileName[MAX_PATH];
	
	// We need to implement the semantics of POSIX mkdtemp() which doesn't exist on Windows

	/*
	 One option is to use a GUID but these symbols are undefined in some modules for reasons that were not investigated
	UUID  uuid;
	char *uuidStr;
	if (UuidCreate(&uuid) == RPC_S_OK && UuidToStringA(&uuid, (RPC_CSTR *)&uuidStr) == RPC_S_OK)
	{
		ReplaceString(dirname, "XXXXXX", uuidStr);

		RpcStringFreeA((RPC_CSTR *)&uuidStr);
	}
	*/


	TCHAR tempPath[MAX_PATH];
	TCHAR tempFileName[MAX_PATH];


	if (GetTempPath(MAX_PATH, tempPath) && GetTempFileName(tempPath, L"CL", 0, tempFileName))
	{
		// This is not thread safe
		DeleteFile(tempFileName);
		ConvertUtf16ToUtf8(tempFileName, utf8FileName, MAX_PATH);

		_mkdir(utf8FileName);

		return utf8FileName;
	}
	else
	{
		return NULL;
	}


#else
	return mkdtemp(tmpDirTemplate);
#endif
}

Rtt_EXPORT const char *Rtt_GetSystemTempDirectory()
{
#ifdef Rtt_WIN_ENV

	return getenv("TEMP");

#else

	return "/tmp";

#endif
}

#if defined(Rtt_NINTENDO_ENV )

nnFile::nnFile(const char *path, const char *mode)
	: fPos(0)
	, fData(NULL)
	, fSize(0)
{
	fHandle.handle = NULL;
	nn::Result result;

	// '//' ==> '/'
	fPath = path;
	while (true)
	{
		size_t pos = fPath.find("//");
		if (pos != std::string::npos)
		{
			fPath.replace(pos, 2, "/");
			continue;
		}
		pos = fPath.find("\\");
		if (pos != std::string::npos)
		{
			fPath.replace(pos, 1, "/");
			continue;
		}
		pos = fPath.find("./");
		if (pos != std::string::npos)
		{
			// todo
			// for now ignore paths like to "./network.so" ?
			return;
		}
		break;
	}

	if (mode[0] == 'r')
	{
		result = nn::fs::OpenFile(&fHandle, fPath.c_str(), nn::fs::OpenMode_Read);
		if (result.IsSuccess())
		{
//			result = nn::fs::GetFileSize(&fSize, fHandle);
//			fData = (uint8_t*)malloc(fSize);
//			result = nn::fs::ReadFile(fHandle, 0, fData, fSize);
			//printf("open %s: mode=%s, size=%lld\n", fPath.c_str(), mode, fSize);
		}
		else
		{
		//	Rtt_LogException("Failed to open %s\n", fPath.c_str());
		}
	}
	else if (mode[0] == 'w')
	{
		Rtt_ASSERT(0 && "TODO test OpenMode_Write");
		nn::fs::OpenFile(&fHandle, fPath.c_str(), nn::fs::OpenMode_Write);
	}
	else if (mode[0] == 'a')
	{
		Rtt_ASSERT(0 && "TODO test OpenMode_AllowAppend");
		nn::fs::OpenFile(&fHandle, fPath.c_str(), nn::fs::OpenMode_AllowAppend);
	}
	else
	{
		Rtt_ASSERT(0 && "Failed to open nnFile, invalid mode");
	}
}

nnFile::~nnFile()
{
	nn::fs::CloseFile(fHandle);
	free(fData);
}

bool nnFile::load()
{
	if (fData == NULL)
	{
		nn::Result result;
		result = nn::fs::GetFileSize(&fSize, fHandle);
		fData = (uint8_t*)malloc(fSize);
		result = nn::fs::ReadFile(fHandle, 0, fData, fSize);
		return result.IsSuccess();
	}
	return true;
}

bool nnFile::isOpen() const
{
	return fHandle.handle != NULL;
}

int nnFile::getc()
{
	load();
	if (fPos < fSize)
	{
		fPos++;
		return fData[fPos - 1];
	}
	return -1;
}

int nnFile::nnungetc(int c)
{
	load();
	if (fPos > 0)
	{
		fPos--;
		fData[fPos] = c;
	}
	return -1;
}

extern "C"
{
	size_t nFileRead(void* ptr, size_t size, size_t count, FILE* fileHandle)
	{
		return Rtt_FileRead(ptr, size, count, fileHandle);
	}

	size_t nFileWrite(void* ptr, size_t size, size_t count, FILE* fileHandle)
	{
		Rtt_ASSERT(0 && "TODO");
		return -1;
	}

	size_t nFileReopen(void* ptr, size_t size, size_t count, FILE* fileHandle)
	{
		Rtt_ASSERT(0 && "TODO");
		return -1;
	}

	size_t nFileFputS(void* ptr, size_t size, size_t count, FILE* fileHandle)
	{
		Rtt_ASSERT(0 && "TODO");
		return -1;
	}

	size_t nFileTmpfile(void* ptr, size_t size, size_t count, FILE* fileHandle)
	{
		Rtt_ASSERT(0 && "TODO");
		return -1;
	}

	size_t nFileFgetS(void* ptr, size_t size, size_t count, FILE* fileHandle)
	{
		Rtt_ASSERT(0 && "TODO");
		return -1;
	}

	size_t nFileFlush(FILE* fileHandle)
	{
		return Rtt_FileFlush(fileHandle);
	}

	size_t nFileRemove(void* ptr, size_t size, size_t count, FILE* fileHandle)
	{
		Rtt_ASSERT(0 && "TODO");
		return -1;
	}

	size_t nFileRename(void* ptr, size_t size, size_t count, FILE* fileHandle)
	{
		Rtt_ASSERT(0 && "TODO");
		return -1;
	}

	void nFileRewind(FILE *fileHandle)
	{
		Rtt_FileRewind(fileHandle);
	}

	long int nFileTell(FILE *fileHandle)
	{
		return Rtt_FileTell(fileHandle);
	}

	int nFileSeek(FILE *fileHandle, long int offset, int origin)
	{
		return Rtt_FileSeek(fileHandle, offset, origin);
	}

	FILE* nFileOpen(const char *filePath, const char *mode)
	{
		return Rtt_FileOpen(filePath, mode);
	}

	int nFileClose(FILE *fileHandle)
	{
		return Rtt_FileClose(fileHandle);
	}

	int nFileError(FILE *fileHandle)
	{
		return Rtt_FileError(fileHandle);
	}

	int nFileGetC(FILE *fileHandle)
	{
		return Rtt_FileGetC(fileHandle);
	}

	int nFileUngetC(int c, FILE *fileHandle)
	{
		return Rtt_FileUngetC(c, fileHandle);
	}

	int nFileEof(FILE* fileHandle)
	{
		return Rtt_FileEof(fileHandle);
	}

	void nFileClearerr(FILE* fileHandle)
	{
		Rtt_FileClearerr(fileHandle);
	}
}

#endif

#endif // ! Rtt_WIN_PHONE_ENV
