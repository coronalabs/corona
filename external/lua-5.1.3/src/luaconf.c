/* ----------------------------------------------------------------------------
// 
// luaconf.c
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------
*/


#define LUA_CORE
#include "luaconf.h"
#ifdef LUA_WIN
	#include <windows.h>
#endif


#if defined(LUA_WIN)
LUA_API int lua_convert_utf16_to_utf8(const wchar_t* utf16String, char* utf8String, size_t utf8StringLength)
{
	int conversionLength;
	int bytesCopied = 0;

	if (utf16String && utf8String && (utf8StringLength > 0))
	{
		conversionLength = WideCharToMultiByte(CP_UTF8, 0, utf16String, -1, NULL, 0, NULL, NULL);
		if ((conversionLength > 0) && (conversionLength <= (int)utf8StringLength))
		{
			bytesCopied = WideCharToMultiByte(
							CP_UTF8, 0, utf16String, -1, utf8String, conversionLength, NULL, NULL);
			if (bytesCopied <= 0)
			{
				bytesCopied = 0;
			}
		}
	}
	return bytesCopied;
}

LUA_API wchar_t* lua_create_utf16_string_from(const char* utf8String)
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

LUA_API void lua_destroy_utf16_string(wchar_t *utf16String)
{
	if (utf16String)
	{
		free(utf16String);
	}
}

LUA_API char* lua_create_utf8_string_from(const wchar_t* utf16String)
{
	char *utf8String = NULL;
	int conversionLength;

	if (utf16String)
	{
		conversionLength = WideCharToMultiByte(CP_UTF8, 0, utf16String, -1, NULL, 0, NULL, NULL);
		if (conversionLength > 0)
		{
			utf8String = (char*)malloc((size_t)conversionLength * sizeof(char));
			WideCharToMultiByte(CP_UTF8, 0, utf16String, -1, utf8String, conversionLength, NULL, NULL);
		}
	}
	return utf8String;
}

LUA_API void lua_destroy_utf8_string(char *utf8String)
{
	if (utf8String)
	{
		free(utf8String);
	}
}


LUA_API FILE* lua_wfopen(const char *filename, const char *mode)
{
	wchar_t *utf16Filename = lua_create_utf16_string_from(filename);
	wchar_t *utf16Mode = lua_create_utf16_string_from(mode);
	FILE *fileHandle = NULL;
	errno_t errorCode;

	errorCode = _wfopen_s(&fileHandle, utf16Filename, utf16Mode);
	lua_destroy_utf16_string(utf16Filename);
	lua_destroy_utf16_string(utf16Mode);
	return fileHandle;
}

LUA_API FILE* lua_wfreopen(const char *filename, const char *mode, FILE *stream)
{
	wchar_t *utf16Filename = lua_create_utf16_string_from(filename);
	wchar_t *utf16Mode = lua_create_utf16_string_from(mode);
	FILE *fileHandle = NULL;

	fileHandle = _wfreopen(utf16Filename, utf16Mode, stream);
	lua_destroy_utf16_string(utf16Filename);
	lua_destroy_utf16_string(utf16Mode);
	return fileHandle;
}

LUA_API int lua_wremove(const char *filename)
{
	wchar_t *utf16Filename = lua_create_utf16_string_from(filename);
	int result = _wremove(utf16Filename);
	lua_destroy_utf16_string(utf16Filename);
	return result;
}

LUA_API int lua_wrename(const char *oldFilename, const char *newFilename)
{
	wchar_t *utf16OldFilename = lua_create_utf16_string_from(oldFilename);
	wchar_t *utf16NewFilename = lua_create_utf16_string_from(newFilename);
	int result = _wrename(utf16OldFilename, utf16NewFilename);
	lua_destroy_utf16_string(utf16OldFilename);
	lua_destroy_utf16_string(utf16NewFilename);
	return result;
}

LUA_API int lua_wchdir(const char *directoryName)
{
#ifdef LUA_WIN_PHONE
	return -1;
#else
	wchar_t *utf16DirectoryName = lua_create_utf16_string_from(directoryName);
	int result = _wchdir(utf16DirectoryName);
	lua_destroy_utf16_string(utf16DirectoryName);
	return result;
#endif
}

LUA_API char* lua_wgetcwd(char *buffer, int maxLength)
{
#ifdef LUA_WIN_PHONE
	return NULL;
#else
	wchar_t *wcharBuffer = NULL;
	wchar_t *resultPointer = NULL;
	
	/* Do not continue if given a buffer with an invalid length. */
	if (buffer && (maxLength <= 0))
	{
		return NULL;
	}
	
	/* Create the UTF-16 buffer. */
	if (NULL == buffer)
	{
		maxLength = MAX_PATH;
	}
	wcharBuffer = (wchar_t*)malloc(maxLength * sizeof(wchar_t));

	/* Copy the current directory path name to the UTF-16 buffer. */
	resultPointer = _wgetcwd(wcharBuffer, maxLength);
	if (NULL == resultPointer)
	{
		return NULL;
	}

	/* Copy the UTF-16 characters to the UTF-8 buffer. */
	if (buffer)
	{
		lua_convert_utf16_to_utf8(wcharBuffer, buffer, maxLength);
	}
	else
	{
		buffer = lua_create_utf8_string_from(wcharBuffer);
	}

	/* Destroy the UTF-16 buffer. */
	free(wcharBuffer);

	/* Return the result. */
	return buffer;
#endif
}

LUA_API int lua_wstati64(const char *path, struct _stati64 *buffer)
{
	wchar_t *utf16Path = lua_create_utf16_string_from(path);
	int result = _wstati64(utf16Path, buffer);
	lua_destroy_utf16_string(utf16Path);
	return result;
}

LUA_API int lua_wsystem(const char *command)
{
#ifdef LUA_WIN_PHONE
	return 0;
#else
	wchar_t *utf16Command = lua_create_utf16_string_from(command);
	int result = _wsystem(utf16Command);
	lua_destroy_utf16_string(utf16Command);
	return result;
#endif
}

#ifdef Rtt_LUA_COMPILER
extern int Rtt_LuaCompile(lua_State *L, int numSources, const char** sources, const char* dstFile, int stripDebug);
LUA_API int lua_compile_files(lua_State *L, int numSources, const char **sources, const char *dstFile, int stripDebug)
{
	return Rtt_LuaCompile(L, numSources, sources, dstFile, stripDebug);
}
#endif

#endif


#ifdef LUA_WIN_PHONE
LUA_API char* lua_getenv(const char *varname)
{
	return NULL;
}

#endif

