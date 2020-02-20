//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_FileSystem_H__
#define _Rtt_FileSystem_H__

#include "Rtt_Macros.h"

#include <stdio.h>
#include <sys/stat.h>

// Note, using `int` instead of `bool` because C doesn't have bool. And `Rtt_EXPORT` is `extern "C"`

Rtt_EXPORT size_t Rtt_FileRead(void* ptr, size_t size, size_t count, FILE* fileHandle);
Rtt_EXPORT void Rtt_FileRewind(FILE *fileHandle);
Rtt_EXPORT long int Rtt_FileTell(FILE *fileHandle);
Rtt_EXPORT int Rtt_FileSeek(FILE *fileHandle, long int offset, int origin);
Rtt_EXPORT FILE* Rtt_FileOpen(const char *filePath, const char *mode);
Rtt_EXPORT int Rtt_FileClose(FILE *fileHandle);
Rtt_EXPORT int Rtt_FileError(FILE *fileHandle);
Rtt_EXPORT int Rtt_FileGetC(FILE *fileHandle);
Rtt_EXPORT int Rtt_FileUngetC(int c, FILE *fileHandle);
Rtt_EXPORT int Rtt_FileDescriptorOpen(const char *filePath, int oflags, int pmode);
Rtt_EXPORT void Rtt_FileDescriptorClose(int fileDescriptor);
#ifdef Rtt_WIN_ENV
	Rtt_EXPORT int Rtt_FileIsHidden(const char *filePath);
#endif
Rtt_EXPORT int Rtt_FileStatus(const char *filePath, struct stat *buffer);
Rtt_EXPORT int Rtt_FileExists(const char *filePath);
Rtt_EXPORT Rtt_DECL_API int Rtt_FileExistsWithSameCase(const char *filePath, const char *rootPath, char **actualPath);
Rtt_EXPORT void* Rtt_FileMemoryMap(int fileDescriptor, size_t byteOffset, size_t byteCount, int canWrite);
Rtt_EXPORT void Rtt_FileMemoryUnmap(const void *memoryMapPointer, size_t byteCount);
Rtt_EXPORT void Rtt_FileMemoryFlush(const void *memoryMapPointer, size_t byteCount);

Rtt_EXPORT int Rtt_CopyFile(const char *srcFilePath, const char *dstFilePath);
Rtt_EXPORT int Rtt_IsDirectory(const char *dirPath);
Rtt_EXPORT int Rtt_MakeDirectory(const char *dirPath);
Rtt_EXPORT int Rtt_DeleteFile(const char *filePath);
Rtt_EXPORT int Rtt_DeleteDirectory(const char *dirPath);
Rtt_EXPORT char *Rtt_MakeTempDirectory(char *tmpDirTemplate);
Rtt_EXPORT const char *Rtt_GetSystemTempDirectory();

#ifdef __cplusplus
// Only available in C++ contexts due to template usage
#include "Rtt_Data.h"
size_t Rtt_WriteDataToFile(const char *filename, Rtt::Data<const unsigned char> &data);
#endif

#ifdef __cplusplus
// Only available in C++ contexts due to STL usage
#include <vector>
#include <string>
std::vector<std::string> Rtt_ListFiles(const char *directoryName);

	// wraper for NN file system
	#if defined(Rtt_NINTENDO_ENV)
	#include <nn/fs.h>
	struct nnFile
	{
		nnFile(const char *filePath, const char *mode);
		~nnFile();

		bool isOpen() const;
		int getc();
		int nnungetc(int c);
		bool load();

		nn::fs::FileHandle fHandle;
		long int fPos;		// current position
		uint8_t* fData;
		int64_t fSize;
		std::string fPath;
	};
	#endif

#endif

#if !defined(Rtt_WIN_ENV)
// Provide POSIX standard names for Windows compatibility
#define _fileno(fd)				fileno(fd)
#define _lseek(fd, a, b)		lseek(fd, a, b)
#define _write(fd, a, b)		write(fd, a, b)
#endif

// ----------------------------------------------------------------------------

#endif // _Rtt_FileSystem_H__
