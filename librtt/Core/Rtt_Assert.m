//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Config.h"

#include "Core/Rtt_Macros.h"

#include "Core/Rtt_Assert.h"

// ----------------------------------------------------------------------------

Rtt_EXPORT_BEGIN

// ----------------------------------------------------------------------------

//#if defined( Rtt_MAC_ENV ) || defined( Rtt_WIN_ENV ) || defined( Rtt_IPHONE_ENV )
//	#define Rtt_VPRINTF_SUPPORTED
//#endif

#include <stdio.h>
#include <stdarg.h>
#include <Foundation/Foundation.h>

#include <wchar.h>

//
// vfprintf_utf8
//
// This gets around a deficiency in stdargs which corrupts UTF8 characters like "😃" to "üòÉ" when
// they appear in printf arguments other than the format string
//
static int vfprintf_utf8(FILE *fp, const char *format, va_list ap)
{
	long n = mbstowcs(0, format, 0);  // determine wide length of format

	if (n==-1)
	{
		return -1;
	}

	wchar_t wfmt[n+1];
	mbstowcs(wfmt, format, n+1);

	int result = vfwprintf(fp, wfmt, ap);

	va_end(ap);

	return result;
}

static int vasprintf_utf8(char **buf, const char *format, va_list ap)
{
	long n = mbstowcs(0, format, 0);  // determine wide length of format

	if (n == -1)
	{
		return -1;
	}

	wchar_t wfmt[n+1];
	mbstowcs(wfmt, format, n+1);

	int result = -1;

	for (size_t wBufLen = 1024; wBufLen < (10 * 1024 * 1024); wBufLen *= 2)
	{
		// alloc memory because using runtime sized arrays on the stack seems to fail at about 2MB
		wchar_t *wBuf = malloc((wBufLen+1) * sizeof(wchar_t));
		va_list ap2;

		// make a copy of the arg list or it wont be any good the next time round the loop
		va_copy(ap2, ap);

		result = vswprintf(wBuf, wBufLen, wfmt, ap2);

		if (result != -1)
		{
			long bufLen = wcstombs(0, wBuf, 0); // determine length required for multibyte string

			if (bufLen == -1)
			{
				return -1;
			}
			
			*buf = calloc((bufLen + 1), sizeof(char));

			wcstombs(*buf, wBuf, bufLen);

			free(wBuf);
			break;
		}

		va_end(ap2);
		free(wBuf);
	}

	va_end(ap);

	return result;
}

#ifdef NOT_USED
static int fprintf_utf8(FILE *fp, const char *format, ...)
{
	va_list ap;
	va_start(ap, format);

	int result = vfprintf_utf8(fp, format, ap);

	va_end(ap);

	return result;
}

static int vsnprintf_utf8(char *buf, size_t bufLen, const char *format, va_list ap)
{
	long n = mbstowcs(0, format, 0);

	if (n==-1)
	{
		return -1;
	}

	wchar_t wfmt[n+1];
	mbstowcs(wfmt, format, n+1);

	size_t wbuflen = 10240;
	wchar_t wbuf[10204];

	int result = vswprintf(wbuf, wbuflen, wfmt, ap);

	wcstombs(buf, wbuf, bufLen);

	va_end(ap);

	return result;
}
#endif // NOT_USED

int
Rtt_LogException( const char *format, ... )
{
	int result = 0;
    va_list ap;

    va_start( ap, format );
    result = Rtt_VLogException( format, ap );
    va_end( ap );

	return result;
}

int Rtt_VLogException_UseStdout = -1;

int
Rtt_VLogException( const char *format, va_list ap )
{
	int result = 0;

	if (Rtt_LogIsEnabled())
	{
		/* With general purpose functions low level functions like this to be used in C code, we can't
		 * guarantee the existence of an autorelease pool. We must create one every single time for safety.
		 */
        @autoreleasepool
		{
			// Log output generally goes to stdout which is attached to either the builtin console logger or a tty.
			// Optionally, by setting a default, it can be sent to the system log.
			if (Rtt_VLogException_UseStdout == -1)
			{
				Rtt_VLogException_UseStdout = true;

#ifndef Rtt_AUTHORING_SIMULATOR
				// Debug output should always go to stdout in the Simulator because it's captured by
				// CoronaConsole but in OS X desktop apps we need to detect whether we're being run
				// from the command line so we direct the output to either the terminal or the system console
				// (shells set the environment variable "_" to the name of the last process they started)
				NSDictionary *env = [[NSProcessInfo processInfo] environment];
				NSString* underscore = [env objectForKey:@"_"];

				if (underscore == nil || [underscore isEqualToString:@"/usr/bin/open"])
				{
					Rtt_VLogException_UseStdout = false;
				}
#endif // Rtt_AUTHORING_SIMULATOR

				if ([[NSUserDefaults standardUserDefaults] boolForKey:@"useSystemLog"])
				{
					Rtt_VLogException_UseStdout = false;
				}
			}

			if (Rtt_VLogException_UseStdout)
			{
				if (isatty(STDOUT_FILENO))
				{
					// Output a timestamp
					NSDateFormatter *dateFormat = [[[NSDateFormatter alloc] init] autorelease];
					[dateFormat setDateFormat:@"MMM dd HH:mm:ss.SSS: "];
					NSString *timestamp = [dateFormat stringFromDate:[NSDate date]];

					fputs([timestamp UTF8String], stdout);
				}

				// Include the process name if it's not "Corona Simulator" (distinguishes output
				// from OS X apps run from build dialog)
				NSString *processName = [[NSProcessInfo processInfo] processName];

				if (! [processName isEqualToString:@"Corona Simulator"])
				{
					fputs([processName UTF8String], stdout);
					fputs(": ", stdout);
				}

				if (isatty(STDOUT_FILENO))
				{
					result = vfprintf_utf8(stdout, format, ap);

					// For result to be greater than 0, format must be at least one character long
					if (result == 0 || format[strlen(format)-1] != '\n')
					{
						fputs("\n", stdout);
					}
				}
				else
				{
					char *outBuf = NULL;
					long bufLen = -1;

					// Optimize the degenerate case where the "format" is just "print the string" to
					// avoid printf() overhead (which can be considerable)
					if (strcmp(format, "%s") == 0)
					{
						// Point at the first argument
						format = va_arg(ap, char *);

						if (strchr(format, '\n') == NULL)
						{
							// No newlines in the string, just get a pointer to it
							outBuf = (char *) format;
						}
						else
						{
							// String has newlines that we'll have to overwrite, dupe it
							outBuf = strdup(format);
						}

						bufLen = strlen(outBuf);
					}
					else
					{
						// Get the result of the printf
						bufLen = vasprintf_utf8(&outBuf, format, ap);
					}

					if (bufLen != -1 && outBuf != NULL)
					{
						// The builtin console logger understands that "\r"s are embedded newlines
						// (this is necessary because multiple log entries can get aggregated into
						// one buffer when traversing the pipe) so we translate newlines to carriage
						// returns

						char *s = outBuf;
						while ((s = strchr(outBuf, '\n')) != NULL)
						{
							*s = '\r';
						}

						// If we zapped a trailing newline, restore it
						if (outBuf[bufLen-1] == '\r')
						{
							outBuf[bufLen-1] = '\n';
						}

						fputs(outBuf, stdout);

						// If there wasn't a trailing newline, emit a newline as a separator
						if (outBuf[bufLen-1] != '\n')
						{
							fputc('\n', stdout);
						}

						if (outBuf != format)
						{
							free(outBuf);
						}
					}
					else
					{
						// Punt on trying to make continuation lines right and just output the values (happens
						// for really large strings)
						bufLen = vfprintf_utf8(stdout, format, ap);

						// For result to be greater than 0, format must be at least one character long
						if (bufLen == 0 || format[strlen(format)-1] != '\n')
						{
							fputs("\n", stdout);
						}
					}
				}

				fflush(stdout);
			}
			else
			{
				NSString *fmtStr = [NSString stringWithUTF8String:format];

				if (fmtStr == nil)
				{
					fmtStr = [NSString stringWithCString:format encoding:NSASCIIStringEncoding];
				}

				NSLogv(fmtStr, ap);
			}
		}
	}

	return result;
}

int
Rtt_Log( const char *format, ... )
{
	int result = 0;
    va_list ap;

    va_start( ap, format );
    result = Rtt_VLogException( format, ap );
    va_end( ap );

	return result;
}

// ----------------------------------------------------------------------------

Rtt_EXPORT_END

// ----------------------------------------------------------------------------
