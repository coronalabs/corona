#ifndef ALMIXER_COMPILED_WITH_SDL

#include "ALmixer_RWops.h"
#include <stdlib.h> /* malloc, free */
#include <stdio.h> /* fopen, fseek, fread, fclose */
#include <string.h> /* strerror */
#include <errno.h> /* errno */
#if defined(_WIN32) || defined(WINAPI_FAMILY)
#	include <Windows.h>
#endif


/* (Note this is different than stdio's seek. This returns ftell.)
 */
static long stdio_seek(ALmixer_RWops* the_context, long offset, int whence)
{
	if(0 == fseek(the_context->hidden.stdio.fp, offset, whence))
	{
		return(ftell(the_context->hidden.stdio.fp));
	}
	else
   	{
/*		ALmixer_SetError("ALmixer_RWops seek failed: %s", strerror(errno)); */
		return (-1);
	}
}

static size_t stdio_read(ALmixer_RWops* the_context, void* ptr, size_t size, size_t nitems)
{
	size_t bytes_read;

	bytes_read = fread(ptr, size, nitems, the_context->hidden.stdio.fp);
	if(0 == bytes_read && ferror(the_context->hidden.stdio.fp))
	{
		/* not sure if strerror can convert ferror */
/*		ALmixer_SetError("ALmixer_RWops read failed: %s", strerror(ferror(the_context->hidden.stdio.fp))); */
	}
	return bytes_read;
}

static size_t stdio_write(ALmixer_RWops* the_context, const void* ptr, size_t size, size_t nitems)
{
	size_t bytes_written;

	bytes_written = fwrite(ptr, size, nitems, the_context->hidden.stdio.fp);
	if(0 == bytes_written && ferror(the_context->hidden.stdio.fp))
	{
/*		ALmixer_SetError("ALmixer_RWops write failed: %s", strerror(ferror(the_context->hidden.stdio.fp))); */
	}
	return bytes_written;
}

static int stdio_close(ALmixer_RWops* the_context)
{
	int return_status = 0;
	if(NULL != the_context)
	{
		if(0 != the_context->hidden.stdio.autoclose)
		{
			if(0 != fclose(the_context->hidden.stdio.fp))
			{
/*				ALmixer_SetError("ALmixer_RWops close failed: %s", strerror(errno)); */
				return_status = -1;
			}
		}
		free(the_context);
	}
	return return_status;
}

ALmixer_RWops* ALmixer_RWFromFP(FILE* file_pointer, char autoclose_flag)
{
    ALmixer_RWops* rw_ops = NULL;

	rw_ops = (ALmixer_RWops*)malloc(sizeof(ALmixer_RWops));
	if(NULL == rw_ops)
	{
/*		ALmixer_SetError("ALmixer_RWFromFP: Out of memory"); */
		return NULL;
	}

	rw_ops->seek = stdio_seek;
	rw_ops->read = stdio_read;
	rw_ops->write = stdio_write;
	rw_ops->close = stdio_close;
	rw_ops->hidden.stdio.fp = file_pointer;
	rw_ops->hidden.stdio.autoclose = autoclose_flag;
	return rw_ops;
}

ALmixer_RWops* ALmixer_RWFromFile(const char* file_name, const char* file_mode)
{
    ALmixer_RWops* rw_ops = NULL;
    FILE* file_pointer = NULL;
    if ((NULL == file_name) || ('\0' == file_name[0]) || (NULL == file_mode) || ('\0' == file_mode[0]))
	{
/*		ALmixer_SetError("ALmixer_RWFromFile: No file or mode specified"); */
        return NULL;
    }
#if defined(NXS_LIB)
		file_pointer = fopen(file_name, file_mode);
#elif defined(_WIN32) || defined(WINAPI_FAMILY)
	{
		/* On Windows, we must convert the given path to UTF-16 in order to access a file whose path contains
		   unicode characters. This is because the Win32 char* APIs do not support UTF-8 encodings.
		 */
		wchar_t* utf16FilePath = NULL;
		wchar_t* utf16ModeString = NULL;
		int conversionLength = MultiByteToWideChar(CP_UTF8, 0, file_name, -1, NULL, 0);
		if (conversionLength > 0)
		{
			utf16FilePath = (wchar_t*)malloc((size_t)conversionLength * sizeof(wchar_t));
			MultiByteToWideChar(CP_UTF8, 0, file_name, -1, utf16FilePath, conversionLength);
		}
		conversionLength = MultiByteToWideChar(CP_UTF8, 0, file_mode, -1, NULL, 0);
		if (conversionLength > 0)
		{
			utf16ModeString = (wchar_t*)malloc((size_t)conversionLength * sizeof(wchar_t));
			MultiByteToWideChar(CP_UTF8, 0, file_mode, -1, utf16ModeString, conversionLength);
		}
		if (utf16FilePath && utf16ModeString)
		{
			_wfopen_s(&file_pointer, utf16FilePath, utf16ModeString);
		}
		if (utf16FilePath)
		{
			free(utf16FilePath);
		}
		if (utf16ModeString)
		{
			free(utf16ModeString);
		}
	}
#else
	file_pointer = fopen(file_name, file_mode);
#endif
	if(NULL == file_pointer)
	{
/*		ALmixer_SetError("ALmixer_RWFromFile: Could not open file: %s", strerror(errno)); */
		return NULL;
    }

	rw_ops = ALmixer_RWFromFP(file_pointer, 1);
    return rw_ops;
}


#endif /* ALMIXER_COMPILED_WITH_SDL */
