/*
 * zlib license.
 */
/*
   Copyright (c) 2003 Eric Wing <ewing . public @ playcontrol.net>

   This software is provided 'as-is', without any express or implied warranty.
   In no event will the authors be held liable for any damages arising from
   the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software in a
   product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source distribution.

*/
/**
 * @file
 * This is a Thread Safe Error handling Library. (How safe is still to be
 * determined.) The name ErrorLib was already taken. TSError might work, but
 * Terror was more fun to use. Named tErrorLib just in case of 
 * any conflicts with others who use "terror".
 *
 * This library is a generalized error flagging library. When an error 
 * occurs, this library allows you to set an error condition (a number 
 * and/or string) which can be fetched by at a later time (perhaps in a 
 * different module). It is analgous to perror or glGetError(), though
 * perhaps a closer analogy might be to SDL_SetError() as this library 
 * allows you to deal with both integer numbers as well as printf-style
 * formatted strings.
 *
 * This library also attempts to implement a certain degree of thread 
 * safety. The problem with a general access error system used 
 * by multiple threads is the risk of clobbering error messages set 
 * in one thread by error messages set in other threads before you 
 * get a chance to read them. This library solves that problem by 
 * creating separate error message structures for every unique thread.
 *
 * This library is unique in several ways. First, this is an entirely
 * self-contained, error-handling-only library. Most people seem to at 
 * best rewrite their own error handling system for each library they write 
 * (and at worst, don't write any error handling system at all).
 * 
 * Second, because this library is intended for just error handling, 
 * it was designed with the idea that it could be dropped into any 
 * library you choose and you are allowed to have separate/isolated 
 * error "pools". So for example, if you were writing multiple modules,
 * such as a graphics library, a sound library, and an application core, 
 * you might desire to have separate error pools for each system so 
 * they don't collide.
 * In order to accommodate the possible multiple instance of pools, the 
 * library has been factored into an object-oriented structure. Since 
 * this library is C-based, it does impose an additional parameter to 
 * be passed around than what people might be used to, but this 
 * API could be wrapped easily that hides the object pointer using a
 * static or global variable for your module if you choose.
 *
 * Finally, this library allows the use of either/both integer or string
 * errors. Integer error numbers allow for quick access and easy comparison.
 * It may also allow for easier internationalization of error messages 
 * if you maintain a look-up-table.
 * String error messages are nice because you know what the problem is 
 * immediately (don't have to hunt down or figure out what error 427 means).
 * They also help make the code self documenting. And if you have errors 
 * nested down in function call layers, you can easily report the 
 * function call stack by appending information to the error string
 * at each function call.
 * 
 * Keep in mind that this library isn't meant to replace other error 
 * reporting conventions, but to supplement them. Functions that return 
 * error values (true/false,int) and logging systems (Logger) are still
 * good systems that may address other issues. You might continue 
 * using error codes for your functions and then use tErrorLib to 
 * fetch the error string when you need them. And to log the error, 
 * you might pass the string into Logger.
 *
 * There are two different ways to use this library with respect to 
 * retrieving errors. You can treat each thread separately and let each
 * live in its own separate world. When you retrieve the error, you only 
 * retrieve the error for the current thread. You should use the
 * TError_GetErrorOnCurrentThread family of functions for this scenario.
 * Alternatively, you can use this library to always return you the last
 * set error regardless of the thread it was set in. You should use the
 * TError_GetLastError family of functions for this scenario.
 * 
 * Sample Usage:
 * @code
 * TErrorPool* err_pool;
 * TErrorStatus error_status;
 * 
 * err_pool = TError_CreateErrorPool();
 * if(NULL == err_pool)
 * {
 *	 	fprintf(stderr, "Error, could not create error pool\n");
 *  	exit(1);
 * }
 *
 * // Set a hypothetical error
 * TError_SetError(err_pool, -1234, "Demo error #%d: %s", 222, "Hello");
 *
 * // Check/get the error using the isolated thread usage model.
 * // Might use TError_GetErrorNumOnCurrentThread or TError_GetErrorStrOnCurrentThread instead
 * error_status = TError_GetErrorOnCurrentThread(err_pool);
 * // Make sure the string is not NULL before printing it with printf.
 * // (Some systems let you print NULL, but on Solaris, it seg faults.)
 * if(NULL != error_status.errorString)
 * {
 * 		printf("%d: %s", error_status.errorNumber, error_status.errorString);
 * }
 *
 * // not really necessary to call since single threaded and
 * // going to delete the entire pool right after, but here for show.
 * TError_DeleteEntryOnCurrentThread(err_pool);
 *
 * TError_FreeErrorPool(err_pool);
 *
 * @endcode
 *
 * Sample API Wrapping:
 * This is an example of how you might want to wrap this library into
 * your own API code so you don't have to expose the error pool.
 * This example uses the last set error usage model.
 * (If you didn't want to use both integers and strings, you could 
 * also make that decision here.)
 *
 * @code
 * #include "tErrorLib.h"
 * static TErrorPool* s_ErrorPool = NULL; // static error pool for this module 
 * 
 * int MyLibraryInit()
 * {
 * 		// Presumably you have you application specific code here too.
 * 		s_ErrorPool = TError_CreateErrorPool();
 * 		if(NULL == s_ErrorPool)
 * 		{
 * 			return 0; // couldn't allocate memory
 * 		}
 * 		return 1;
 * }
 * 
 * void MyLibraryQuit()
 * {
 * 		TError_FreeErrorPool(s_ErrorPool);
 * 		s_ErrorPool = NULL;
 * }
 * 
 * void MyLibrarySetError(int err_num, const char* err_str, ...)
 * {
 * 		va_list argp;
 * 		va_start(argp, err_str);
 * 		TError_SetErrorv(s_ErrorPool, err_num, err_str, argp);
 * 		va_end(argp);
 * }
 *
 * const char* MyLibraryGetError()
 * {
 * 		const char* ret_error = TError_GetLastErrorStr(s_ErrorPool);
 * 		if(NULL == ret_error)
 *      {
 * 		 		return ""; // provide an empty string to make it safe for people using printf without checking.
 * 		}
 * 		return ret_error;
 * }
 * @endcode
 *
 * @note By default, this library attempts to use vasprintf to generate
 * the printf-style formatted strings. vasprintf is a GNU extension 
 * which solves the problem of having enough memory allocated in a buffer
 * to handle an arbitrary length formatted string which you don't know
 * in advance. I recommend you use this implementation if your library
 * can support it as it will allow you to always generate correct strings.
 * (Stack traces can become long and exceed your preallocated buffer sizes.)
 * For compatibility, an alternative vsnprintf implementation is provided.
 * If a string is too large, it will be truncated. vsnprintf is specified 
 * in C99, but it critcal for avoid security related issues surrounding 
 * sprintf and vsprintf. If your library lacks vsnprintf, you're asking 
 * for trouble. I currently do not try to handle this case.
 * 
 * @note By default, this library assumes 0 is the no-error value.
 * In addition, if you are using the string-only based APIs, the integer
 * components will be automatically filled to 0 (for no-error) and 
 * 1 (for error). If these numbers conflict with your conventions,
 * you may be able to change these values in the implementation file
 * and recompile the library. Look for the defines for TERROR_ERROR_VALUE
 * and TERROR_NOERROR_VALUE.
 *
 * @note This version attempts to provide enough thread safety to get by
 * but it may not be totally safe during creation and destruction of 
 * instances (partly since locking is done inside the object-level),
 * so don't create or destroy pools when there is possible contention.
 * Strings you pass into the functions are not necessarily locked so
 * be careful if you are modifying strings that are shared among
 * your threads.
 *
 * @note Error strings returned are pointers to tErrorLib's internal 
 * copies of strings. Do not modify these or delete them. Also keep in 
 * mind that the pointers to these strings may become invalid when
 * a new error is set (on a per-thread basis). So if you need a copy 
 * of the error string, you should make your own copy.
 *
 * @warning For code that frequently generates and destroys many threads,
 * be aware that you should pay attention to memory management with this 
 * library. This library works by creating a unique error message 
 * structure for each thread. When the thread dies, the error pool 
 * will still contain a structure for that thread (if it had called 
 * this library in that thread). Just before the thread dies (but after 
 * any final error calls), you should call TError_DeleteEntryOnCurrentThread()
 * to free the memory for that thread. Otherwise you will have a 
 * pseudo-memory-leak. (Pseudo in the sense that once you free the error pool,
 * all memory will be freed, regardless of whether you remembered to call
 * this function.)
 * @see TERROR_NOERROR_VALUE, TError_DeleteEntryOnCurrentThread
 * 
 * @author Eric Wing
 */

#ifndef TERRORLIB_H
#define TERRORLIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h> /* for va_list */

/**
 * This library determines if there is an error by checking 
 * both the error number and error string. If the error string is NULL
 * and the error number is 0 (TERROR_NOERROR_VALUE), then it 
 * is considered a non-error. Because this library allows you to
 * use just numbers or just strings, a value must be filled in internally
 * as a place holder. It also must return some default/no-error values
 * for GetError if there was no error. In these situations,
 * NULL is set for strings and 0 (TERROR_NOERROR_VALUE)
 * is set for numbers. This will become a point of confusion if you use 
 * 0 as an error code to denote a legitimate error and have a NULL error 
 * string. 
 * 
 * To accommodate this problem, this define is provided to let you 
 * redefine what the no-error value is (though this is untested). 
 * If you have the opportunity to write code that doesn't rely on 0 denoting
 * an error, I recommend using this library as is, instead of trying 
 * to change this. If you do change this value, remember you must recompile
 * the entire library. Also make sure that TERROR_ERROR_VALUE (in 
 * implementation file) is not equal to your changed value.
 * 
 * For most cases, if you just want to check if there was an 
 * error, you can check if the error_number == 0. But if
 * you are thinking that you may want to redefine what the no-error
 * value is in the future (say -99999), then you can use this 
 * constant name instead (e.g. error_number == TERROR_NOERROR_VALUE).
 */
#define TERROR_NOERROR_VALUE 0


#ifndef DOXYGEN_SHOULD_IGNORE_THIS
/** @cond DOXYGEN_SHOULD_IGNORE_THIS */

/* Note: For Doxygen to produce clean output, you should set the 
 * PREDEFINED option to remove TERROR_TERROR_DECLSPEC, TERROR_CALL, and
 * the DOXYGEN_SHOULD_IGNORE_THIS blocks.
 * PREDEFINED = DOXYGEN_SHOULD_IGNORE_THIS=1 TERROR_TERROR_DECLSPEC= TERROR_CALL=
 */

/** Windows needs to know explicitly which functions to export in a DLL. */
#if defined(_WIN32) || defined(WINAPI_FAMILY)
	#if defined(TERROR_BUILD_LIBRARY)
		#define TERROR_DECLSPEC __declspec(dllexport)
	#else
		#define TERROR_DECLSPEC
	#endif
#else
	#if defined(TERROR_BUILD_LIBRARY)
		#if defined (__GNUC__) && __GNUC__ >= 4
			#define TERROR_DECLSPEC __attribute__((visibility("default")))
		#else
			#define TERROR_DECLSPEC
		#endif
	#else
		#define TERROR_DECLSPEC
	#endif
#endif

/* For Windows, by default, use the C calling convention */
#if defined(_WIN32) || defined(WINAPI_FAMILY)
	#define TERROR_CALL __cdecl
#else
	#define TERROR_CALL
#endif

	
/* Version number is set here.
 * Printable format: "%d.%d.%d", MAJOR, MINOR, PATCHLEVEL
 */
#define TERROR_MAJOR_VERSION		0
#define TERROR_MINOR_VERSION		1
#define TERROR_PATCH_VERSION		0

/** @endcond DOXYGEN_SHOULD_IGNORE_THIS */
#endif /* DOXYGEN_SHOULD_IGNORE_THIS */


/**
 * Struct that contains the version information of this library.
 * This represents the library's version as three levels: major revision
 * (increments with massive changes, additions, and enhancements),
 * minor revision (increments with backwards-compatible changes to the
 * major revision), and patchlevel (increments with fixes to the minor
 * revision).
 * @see TERROR_GET_COMPILED_VERSION, TError_GetLinkedVersion
 */
typedef struct
{
	int major; /**< major revision. */
	int minor; /**< minor revision. */
	int patch; /**< patch revision. */
} TErrorVersion;


/**
 * Struct that contains all the data needed to represent an error (pool)
 * instance. This is the object you pass around to all instance
 * based error functions. Don't touch the data in the struct directly.
 * This should be considered an opaque data type.
 */
typedef struct
{
	void* mutexLock; /**< You probably shouldn't touch this either. */
	void* opaqueData; /**< Don't touch this. */
} TErrorPool;

/**
 * Struct that contains both the error number and error string.
 * This is returned by the TError_GetError functions which 
 * allows you to fetch both the errorNumber and errorString.
 * (Other API functions only let you get one or the other).
 * You may read the values directly, but do not modify the string.
 * Keep in mind that once a SetError is called again for your thread,
 * the pointer may become invalid. Copy this data out if you need
 * to keep it.
 */
typedef struct
{
	int errorNumber; /**< The error number. */
	const char* errorString; /**< The error string (read-only). */
} TErrorStatus;


/**
 * This macro fills in a TError_Version structure with the version of the
 * library you compiled against. This is determined by what header the
 * compiler uses. Note that if you dynamically linked the library, you might
 * have a slightly newer or older version at runtime. That version can be
 * determined with TError_GetLinkedVersion(), which, unlike 
 * TERROR_GET_COMPILED_VERSION, is not a macro.
 *
 * @param X A pointer to a TError_Version struct to initialize.
 *
 * @see TErrorVersion, TError_GetLinkedVersion
 */
#define TERROR_GET_COMPILED_VERSION(X) 		\
{											\
	if(NULL != (X))							\
	{										\
		(X)->major = TERROR_MAJOR_VERSION;	\
		(X)->minor = TERROR_MINOR_VERSION;	\
		(X)->patch = TERROR_PATCH_VERSION;	\
	}										\
}

/**
 * Gets the library version of tErrorLib you are using.
 * This gets the version of tErrorLib that is linked against your program.
 * If you are using a shared library (DLL) version of tError, then it is
 * possible that it will be different than the version you compiled against.
 *
 * This is a real function; the macro TERROR_GET_COMPILED_VERSION 
 * tells you what version of tErrorLib you compiled against:
 *
 * @code
 * TErrorVersion compiled;
 * TErrorVersion linked;
 *
 * TERROR_GET_COMPILED_VERSION(&compiled);
 * TError_GetLinkedVersion(&linked);
 * printf("We compiled against tError version %d.%d.%d ...\n",
 *           compiled.major, compiled.minor, compiled.patch);
 * printf("But we linked against tError version %d.%d.%d.\n",
 *           linked.major, linked.minor, linked.patch);
 * @endcode
 *
 * @see TErrorVersion, TERROR_GET_COMPILED_VERSION
 */
extern TERROR_DECLSPEC void TERROR_CALL TError_GetLinkedVersion(TErrorVersion* ver);

/**
 * This creates a new error pool instance.
 * An error pool is a self-contained object that holds its own 
 * errors. You may have multiple error pools to isolate errors for 
 * different subsystems if you choose.
 *
 * For most (if not all) other tErrorLib functions, you will
 * pass this instance to each function when you call them.
 * 
 * @return Returns a pointer to an error pool which is the 
 * instance variable (if successful) or NULL on failure.
 *
 * @see TError_FreeErrorPool
 */
extern TERROR_DECLSPEC TErrorPool* TERROR_CALL TError_CreateErrorPool(void);

/**
 * This frees an error pool instance.
 * This properly frees the memory of an error pool instance created by
 * CreateErrorPool. Whenever you create a TErrorPool
 * instance, you should always remember to balance it with a 
 * FreeErrorPool() call.
 *
 * @param err_pool A pointer to the error pool instance you want to free.
 *
 * @see TError_CreateErrorPool
 */
extern TERROR_DECLSPEC void TERROR_CALL TError_FreeErrorPool(TErrorPool* err_pool);

/**
 * This function will delete the error message memory that has been
 * allocated for the thread you call this function in. 
 * This reflects a deficiency in this library's design.
 * If a thread terminates, this library will still have 
 * allocated memory for it if it had set an error.
 * If you plan on killing a thread, call this function in that 
 * thread before it dies so the memory can be freed.
 * Do not call SetError* again in this thread or memory
 * will be reallocated.
 * It is safe to call if no memory has actually be allocated for the 
 * error for the current thread.
 * If you cannot use this function to free the memory for some reason, 
 * this is a pseudo-memory-leak. By pseudo, I mean that you won't
 * completely leak. When you delete the entire memory pool, it will 
 * be able to free all error message structures that were associated 
 * with the pool so you can recover this memory.
 *
 * @param err_pool The error pool instance you want to use.
 */
extern TERROR_DECLSPEC void TError_DeleteEntryOnCurrentThread(TErrorPool* err_pool);


/**
 * This function sets an error.
 * Calling this function will set an error (for this thread)
 * with the specified error number and error string which 
 * can later be retrieved by a GetError call.
 * The function usage is similar to printf.
 * If both the err_num is set to 0 (see other notes TERROR_NOERROR_VALUE)
 * and the err_str is set to NULL, then this is considered clearing an 
 * error and no error will be marked.
 * 
 * @param err_pool The error pool instance you want to use.
 *
 * @param err_num The error number you want to use for this error.
 * The value for this number is up to you, based on your own 
 * conventions. But this library reserves one number to denote "no error".
 * This number for initial implementation and documentation purposes is 0,
 * and is defined in the implementation as TERROR_NOERROR_VALUE. 
 * You can still specify this number to be anything you choose 
 * if and only if you also always specify an error string that is not NULL.
 * As long as an error string exists, the system will recognize there
 * is an error. But if you have a NULL string and use 0 to denote an
 * error, this will be interpreted as a clear error message.
 * So I recommend you treat 0 as a no-error value and avoid using it 
 * as an error value in your code to avoid confusion.
 *
 * @param err_str This is where your error text goes. It is compatible with
 * printf style format strings. There is no imposed limit on how long 
 * the strings can be if you are using the vasprintf backend. The
 * vsnprintf implemention will automatically truncate the string if it
 * is too long.
 * You if don't wish to specify an error string, you may enter NULL here.
 * But remember to note the effect if this string is NULL and the err_num is
 * set to 0 (TERROR_NOERROR_VALUE).
 *
 * @param ... This is the variable argument list used to accomodate 
 * printf style format strings.
 *
 * @see TError_SetErrorv, TError_SetErrorNoFormat, TError_SetErrorNum,
 * TError_SetErrorStr, TError_SetErrorStrv, TError_SetErrorStrNoFormat,
 * TERROR_NOERROR_VALUE
 */
extern TERROR_DECLSPEC void TERROR_CALL TError_SetError(TErrorPool* err_pool, int err_num, const char* err_str, ...);


/**
 * This function sets an error.
 * This is the va_list version of TError_SetError.
 * The same rules apply to this as with TError_SetError.
 * 
 * @see TError_SetError
 */
extern TERROR_DECLSPEC void TERROR_CALL TError_SetErrorv(TErrorPool* err_pool, int err_num, const char* err_str, va_list argp);

/**
 * This is a "No Format Strings Allowed" version of SetError.
 * This version of SetError disallows the use of format strings.
 * This was written if you needed to expose the SetError function to
 * an untrusted source because it is quite easy to crash a system (or worse)
 * with an invalid format string. An untrusted source might include 
 * arguments passed through the command line, any user input that 
 * gets fed to Logger, or strings taken from runtime generated sources
 * like scripts.
 * I was probably being overly paranoid when I created this function,
 * and there may be ways to achive this level of safety without 
 * this function, but here it is anyway.
 * In addition, if your compiler has problems with both vasprintf and 
 * vsnprintf, (you will have to modify some source code) you might 
 * consider basing all SetError functions around this function because 
 * since the strings are known length because there is no argument 
 * expansion.
 * 
 * @see TError_SetError
 */
extern TERROR_DECLSPEC void TERROR_CALL TError_SetErrorNoFormat(TErrorPool* err_pool, int err_num, const char* err_str);

/**
 * This function sets an error.
 * This version only lets you set the error number. The backend
 * will automatically set the error string to NULL. 
 * This API call is intended to be used only if you don't plan on using
 * any error strings in your code.
 * Also be aware of the notes about TERROR_NOERROR_VALUE.
 * 
 * @see TError_SetError, TERROR_NOERROR_VALUE
 */
extern TERROR_DECLSPEC void TERROR_CALL TError_SetErrorNum(TErrorPool* err_pool, int err_num);

/**
 * This function sets an error.
 * This version only lets you set the error string. The backend
 * will automatically set the error number to TERROR_NOERROR_VALUE
 * which is currently implemented as 0. 
 * This API call is intended to be used only if you don't plan on using
 * any error numbers in your code.
 * Also be aware of the notes about TERROR_NOERROR_VALUE if you use NULL
 * strings.
 * 
 * @see TError_SetError, TERROR_NOERROR_VALUE
 */
extern TERROR_DECLSPEC void TERROR_CALL TError_SetErrorStr(TErrorPool* err_pool, const char* err_str, ...);

/**
 * This function sets an error.
 * This is the va_list version of TError_SetErrorStr.
 * 
 * @see TError_SetError, TError_SetErrorStr, TERROR_NOERROR_VALUE
 */
extern TERROR_DECLSPEC void TERROR_CALL TError_SetErrorStrv(TErrorPool* err_pool, const char* err_str, va_list argp);

/**
 * This function sets an error.
 * This is the "No Format Strings Allowed" version of TError_SetErrorStr.
 * 
 * @see TError_SetError, TError_SetErrorNoFormat,
 * TError_SetErrorStr, TERROR_NOERROR_VALUE
 */
extern TERROR_DECLSPEC void TERROR_CALL TError_SetErrorStrNoFormat(TErrorPool* err_pool, const char* err_str);

/**
 * This function gets the last error to be set by one of the SetError 
 * functions (from within your current thread). This version of the function
 * returns just the error number.
 * After this function is called, the error will be cleared, so your 
 * next call to a GetError function (with no SetError calls in between)
 * will return a struct set with no-error values.
 *
 * @param err_pool The error pool instance you want to use.
 *
 * @return Returns the error number.
 * If no error was set, the error number will be set 
 * to 0 (TERROR_NOERROR_VALUE).
 *
 * @see TError_GetErrorOnCurrentThread, TError_GetErrorStrOnCurrentThread, TERROR_NOERROR_VALUE
 */
extern TERROR_DECLSPEC int TERROR_CALL TError_GetErrorNumOnCurrentThread(TErrorPool* err_pool);

/**
 * This function gets the last error to be set by one of the SetError 
 * functions (from within your current thread). This version of the function
 * returns a pointer to the error string.
 * After this function is called, the error will be cleared, so your 
 * next call to a GetError function (with no SetError calls in between)
 * will return a struct set with no-error values.
 *
 * @param err_pool The error pool instance you want to use.
 *
 * @return Returns the pointer to the error string. 
 * The pointer is to tErrorLib's own copy
 * of the error message so you should not modify this string. Furthermore,
 * the pointer may become invalid at the next call to SetError within this
 * same thread, so if you need to keep the string, you must make your
 * own copy of it.
 * If no error was set, the error string will be set to NULL.
 *
 * @see TError_GetErrorNumOnCurrentThread, TError_GetErrorOnCurrentThread, TERROR_NOERROR_VALUE
 */
extern TERROR_DECLSPEC const char* TERROR_CALL TError_GetErrorStrOnCurrentThread(TErrorPool* err_pool);

/**
 * This function gets the last error to be set by one of the SetError 
 * functions (from within your current thread). This version of the function
 * returns a struct containing the error number and a pointer to the 
 * error string.
 * After this function is called, the error will be cleared, so your 
 * next call to a GetError function (with no SetError calls in between)
 * will return a struct set with no-error values.
 *
 * @param err_pool The error pool instance you want to use.
 *
 * @return Returns (by-value) a struct containing the error number and
 * pointer to the error string. The pointer is to tErrorLib's own copy
 * of the error message so you should not modify this string. Furthermore,
 * the pointer may become invalid at the next call to SetError within this
 * same thread, so if you need to keep the string, you must make your
 * own copy of it.
 * If no error was set, the error number will be set to 0 (TERROR_NOERROR_VALUE)
 * and the error string will be set to NULL.
 *
 * @see TError_GetErrorNumOnCurrentThread, TError_GetErrorStrOnCurrentThread, TERROR_NOERROR_VALUE
 */
extern TERROR_DECLSPEC TErrorStatus TERROR_CALL TError_GetErrorOnCurrentThread(TErrorPool* err_pool);


/**
 * This function gets the last error to be set by one of the SetError 
 * functions (regardless of thread).
 * This version of the function
 * returns just the error number.
 * After this function is called, the error will be cleared, so your 
 * next call to a GetError function (with no SetError calls in between)
 * will return a struct set with no-error values.
 *
 * @param err_pool The error pool instance you want to use.
 *
 * @return Returns the error number.
 * If no error was set, the error number will be set 
 * to 0 (TERROR_NOERROR_VALUE).
 *
 * @see TError_GetLastError, TError_GetLastErrorStr, TERROR_NOERROR_VALUE
 */
extern TERROR_DECLSPEC int TERROR_CALL TError_GetLastErrorNum(TErrorPool* err_pool);

/**
 * This function gets the last error to be set by one of the SetError 
 * functions (regardless of thread).
 * This version of the function
 * returns a pointer to the error string.
 * After this function is called, the error will be cleared, so your 
 * next call to a GetError function (with no SetError calls in between)
 * will return a struct set with no-error values.
 *
 * @param err_pool The error pool instance you want to use.
 *
 * @return Returns the pointer to the error string. 
 * The pointer is to tErrorLib's own copy
 * of the error message so you should not modify this string. Furthermore,
 * the pointer may become invalid at the next call to SetError within this
 * same thread, so if you need to keep the string, you must make your
 * own copy of it.
 * If no error was set, the error string will be set to NULL.
 *
 * @see TError_GetLastErrorNum, TError_GetLastError, TERROR_NOERROR_VALUE
 */
extern TERROR_DECLSPEC const char* TERROR_CALL TError_GetLastErrorStr(TErrorPool* err_pool);

/**
 * This function gets the last error to be set by one of the SetError 
 * functions (regardless of thread). 
 * This version of the function
 * returns a struct containing the error number and a pointer to the 
 * error string.
 * After this function is called, the error will be cleared, so your 
 * next call to a GetError function (with no SetError calls in between)
 * will return a struct set with no-error values.
 *
 * @param err_pool The error pool instance you want to use.
 *
 * @return Returns (by-value) a struct containing the error number and
 * pointer to the error string. The pointer is to tErrorLib's own copy
 * of the error message so you should not modify this string. Furthermore,
 * the pointer may become invalid at the next call to SetError within this
 * same thread, so if you need to keep the string, you must make your
 * own copy of it.
 * If no error was set, the error number will be set to 0 (TERROR_NOERROR_VALUE)
 * and the error string will be set to NULL.
 *
 * @see TError_GetLastErrorNum, TError_GetLastErrorStr, TERROR_NOERROR_VALUE
 */
extern TERROR_DECLSPEC TErrorStatus TERROR_CALL TError_GetLastError(TErrorPool* err_pool);


#ifdef __cplusplus
}
#endif


#endif /* TERRORLIB_H */

