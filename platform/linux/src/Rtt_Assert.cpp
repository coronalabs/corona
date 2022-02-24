//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

//#include "Core/Rtt_Build.h"
#include "Core/Rtt_Config.h"
#include "Core/Rtt_Macros.h"
#include "Core/Rtt_Types.h"
#include "Core/Rtt_Assert.h"

#ifdef Rtt_LINUX_ENV 
void LinuxLog(const char* buf, int len);
#endif

#ifdef Rtt_EMSCRIPTEN_ENV
#undef Rtt_Log
#undef Rtt_LogException

#if EMSCRIPTEN
#include "emscripten/emscripten.h"
#endif

#endif

// ----------------------------------------------------------------------------

Rtt_EXPORT_BEGIN

// ----------------------------------------------------------------------------

//#if defined( Rtt_MAC_ENV ) || defined( Rtt_WIN_ENV ) || defined( Rtt_IPHONE_ENV )
//	#define Rtt_VPRINTF_SUPPORTED
//#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#if defined( Rtt_ANDROID_ENV )
#include <android/log.h>
#elif defined( Rtt_WIN_ENV ) || defined( Rtt_POWERVR_ENV )
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#ifdef Rtt_LINUX_ENV
static bool linuxIsErrorMsg = false;
#endif
/// Static variable set to non-zero if logging is enabled. Set to zero if logging is disabled.
static int fIsLoggingEnabled = 1;

/// Enables the logging system.
void
Rtt_LogEnable()
{
	fIsLoggingEnabled = 1;
}

/// Disables the logging system.
void
Rtt_LogDisable()
{
#ifdef Rtt_DEBUG
	// Don't ever disable logging in debug builds
	fIsLoggingEnabled = 1;
#else
	fIsLoggingEnabled = 0;
#endif // Rtt_DEBUG
}

/// Determines if the logging system is enabled or disabled.
/// @return Returns non-zero if the logging system is enabled. Returns zero if disabled.
int
Rtt_LogIsEnabled()
{
	return fIsLoggingEnabled;
}

#ifdef Rtt_WIN_PHONE_ENV
/// Optional callback to be invoked when the system wants to override logging in Rtt_Log() and Rtt_LogException().
static Rtt_LogHandlerCallback fLogHandler = NULL;

/// Set a callback used to override logging when Rtt_Log() or Rtt_LogException() gets called.
/// @param callback Pointer to the function to be invoked. Set to null to not override logging.
void
Rtt_LogSetHandler(Rtt_LogHandlerCallback callback)
{
	fLogHandler = callback;
}

/// Gets the callback set via Rtt_LogSetHandler() used to override logging.
/// @returns Returns a pointer to the function callback. Returns null if no callback has been assigned.
Rtt_LogHandlerCallback
Rtt_LogGetHandler(void)
{
	return fLogHandler;
}

#endif


#if ! defined( Rtt_APPLE_ENV ) // Apple platforms need to use Obj-C for now so the implementation is done elsewhere.
// This is duplicated because Rtt_Log is disabled in too many configurations and we need a function that is available everywhere.
// This is a temporary function until I rewrite the entire logging/reporting system.
int
Rtt_LogException( const char *format, ... )
{
#if defined(Rtt_LINUX_ENV) && defined(Rtt_SIMULATOR)
	linuxIsErrorMsg = true;
#endif

	int result = 0;
	va_list ap;

	va_start( ap, format );
	result = Rtt_VLogException( format, ap );
	va_end( ap );

	return result;
}

int
Rtt_VLogException(const char* format, va_list ap)
{
	int result = 0;

#ifdef Rtt_VPRINTF_SUPPORTED

#if defined( Rtt_WIN_ENV ) || defined( Rtt_POWERVR_ENV )
	if (format != NULL)
	{
		// Acquire a string buffer that will fit the formatted text.
		// For best performance, attempt to use a small string buffer on the stack.
		char stringBufferOnStack[512];
		char* stringPointer = stringBufferOnStack;
		size_t stringLength = _vscprintf(format, ap);
		size_t bufferSize = stringLength + 2;	// Assume we need to append:  \n\0
		if (bufferSize > sizeof(stringBufferOnStack))
		{
			// The string variable on the stack is too small. Allocate a larger string on the heap.
			stringPointer = malloc(bufferSize);
			if (!stringPointer)
			{
				stringPointer = stringBufferOnStack;
				bufferSize = sizeof(stringBufferOnStack);
				stringLength = bufferSize - 3;
			}
		}

		// Format and output the string.
		result = vsnprintf_s(stringPointer, bufferSize, stringLength, format, ap);
		if (result >= 0)
		{
			// Update our string length with number of characters actually copied to the buffer.
			// Should never differ, but we should check just in case.
			if (result < (int)stringLength)
			{
				stringLength = result;
			}
			else if (result > (int)stringLength)
			{
				result = stringLength;
			}

			// Make sure the string is null terminated.
			stringPointer[stringLength] = '\0';

			// Replace all "\r\n" pairs with "\n" in the string.
			// We do this because stdout is in "text mode" by default, which means it'll replace all "\n" with "\r\n".
			// If we don't remove the '\r' characters, then the outputted string will contain "\r\r\n" line endings.
			{
				size_t destinationIndex;
				size_t sourceIndex;
				for (sourceIndex = 0, destinationIndex = 0; sourceIndex < stringLength; sourceIndex++)
				{
					if (('\r' == stringPointer[sourceIndex]) &&
						((sourceIndex + 1) < stringLength) && ('\n' == stringPointer[sourceIndex + 1]))
					{
						result--;
						continue;
					}
					stringPointer[destinationIndex] = stringPointer[sourceIndex];
					destinationIndex++;
				}
				stringLength = result;
				stringPointer[stringLength] = '\0';
			}

			// Add a newline character to the end of the message if it doesn't have one.
			if ((stringLength <= 0) || (stringPointer[stringLength - 1] != '\n'))
			{
				stringPointer[stringLength] = '\n';
				stringPointer[stringLength + 1] = '\0';
				stringLength++;
				result++;
			}

			// Output the string to stdout and the Visual Studio debugger.
#if defined(Rtt_NINTENDO_ENV)
			fputs(stringPointer, stdout);
#elif defined(Rtt_WIN_PHONE_ENV)
			if (fLogHandler)
			{
				fLogHandler(stringPointer);
			}
#else
			if (IsDebuggerPresent())
			{
				OutputDebugStringA(stringPointer);
			}
			fputs(stringPointer, stdout);
			fflush(stdout);
#endif
		}

		// Delete the string buffer if allocated on the heap.
		if (stringPointer != stringBufferOnStack)
		{
			free(stringPointer);
		}
	}
#elif defined( Rtt_ANDROID_ENV )
	result = __android_log_vprint(ANDROID_LOG_INFO, "Corona", format, ap);
#elif defined(EMSCRIPTEN)
	char buffer[4096];
	int n = vsnprintf(buffer, 4096, format, ap);
	if (n > 0)
	{
		EM_ASM(
			{
				var msg = UTF8ToString($0);
				Module.printErr(msg);
			}, buffer);
	}
#else
#if defined(Rtt_LINUX_ENV) && defined(Rtt_SIMULATOR)
#if !defined(CORONABUILDER_LINUX)
	char buffer[4096];
	va_list apCopy;
	va_copy(apCopy, ap);

	int n = vsnprintf(buffer, sizeof(buffer) - 1, format, apCopy);
	if (n > 0)
	{
		LinuxLog(buffer, n);
	}
#endif
#endif

	result = vfprintf( stderr, format, ap );
	fflush( stderr );
#endif

#else

	Rtt_ASSERT_NOT_IMPLEMENTED();
	result = fprintf( stderr, "vfprintf not supported\n" );

#endif

	return result;
}

int
Rtt_Log( const char *format, ... )
{
	int result;

	if (Rtt_LogIsEnabled())
	{
#if defined(Rtt_LINUX_ENV) && defined(Rtt_SIMULATOR)
		linuxIsErrorMsg = false;
#endif

		va_list ap;
		va_start(ap, format);
		result = Rtt_VLogException(format, ap);
		va_end(ap);
	}

	return result;
}

#endif // ! defined ( Rtt_APPLE_ENV )
// ----------------------------------------------------------------------------

#ifdef Rtt_DEBUG

#if defined( Rtt_MAC_ENV ) || defined( Rtt_IPHONE_ENV ) || defined( Rtt_TVOS_ENV )
#define Rtt_TRAP_WITH_SIGNAL	1
#include <signal.h>
#if 0 // see stacktrace code below
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#endif // 0
#elif defined( Rtt_EMSCRIPTEN_ENV )
#define Rtt_Log printf
#endif

static void
Rtt_UserBreak( )
{
#if defined( Rtt_TRAP_WITH_SIGNAL )
	raise( SIGINT );
#elif defined( Rtt_WIN_ENV )
	__debugbreak();
#elif defined( __ARMCC_VERSION )
	__breakpoint( 1 );
#endif
}

static void
Rtt_LogStack()
{
#if defined( Rtt_EMSCRIPTEN_ENV )
#if EMSCRIPTEN
	emscripten_log( EM_LOG_C_STACK | EM_LOG_JS_STACK | EM_LOG_DEMANGLE, "" );
#endif
#elif defined( Rtt_MAC_ENV ) || defined( Rtt_IPHONE_ENV )
	/*

	Rtt_LogEnable();

	void* callstack[128];
	int i, frames = backtrace(callstack, 128);
	char** strs = backtrace_symbols(callstack, frames);
	Rtt_Log("stack trace:\n");
	for (i = 0; i < frames; ++i)
	{
		Rtt_Log("\t%s\n", strs[i]);
	}
	free(strs);
	*/
#endif
}

int
Rtt_Verify( int cond, const char *reason, const char *func, const char *file, int line )
{
	if ( ! cond )
	{
		const char kNullStr[] = "(null)";

		if ( ! reason )	{ reason = kNullStr; }
		if ( ! file )	{ file = kNullStr; }
		if ( ! func )	{ func = kNullStr; }

		Rtt_TRACE( ( "Rtt_ASSERT [%s] at %s:%d (%s)\n", func, file, line, reason ) );
		Rtt_LogStack();

		Rtt_UserBreak( );
	}

	return cond;
}

/*
#include <assert.h>

static void
Rtt_Panic( )
{
	assert( 0 );
}
*/

#endif // Rtt_DEBUG

// ----------------------------------------------------------------------------

Rtt_EXPORT_END

// ----------------------------------------------------------------------------
