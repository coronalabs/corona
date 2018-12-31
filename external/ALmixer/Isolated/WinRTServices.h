/*
 * Author:  Joshua Quick
 *
 * Description:
 *    Provides APIs needed on a WinRT platform for Windows Phone and Windows Store apps.
 */

#pragma once

#include <windows.h>

/* Trigger a compiler error if this header file was included on a non-WinRT platform. */
#if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
	#error This header file can only be included on a WinRT platform.
#endif


#ifdef __cplusplus
extern "C" {
#endif


void WinRTServices_Sleep(int milliseconds);


/* Ends C function definitions when using C++ */
#ifdef __cplusplus
}
#endif

