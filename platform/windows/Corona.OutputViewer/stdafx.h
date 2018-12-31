// ----------------------------------------------------------------------------
// 
// stdafx.h
// Copyright (c) 2015 Corona Labs Inc. All rights reserved.
//
// This is a pre-compiled header which must be included in all source files
// in this library.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once


// Exclude rarely-used stuff from Windows headers.
#ifndef VC_EXTRALEAN
#	define VC_EXTRALEAN
#endif

// Makes CString class use explicit constructors.
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS

// Turn off MFC's hiding of some common and often safely ignored warning messages.
#define _AFX_ALL_WARNINGS


#include "targetver.h"
#include <afxwin.h>
#include <afxext.h>
#include <afxdisp.h>
#ifndef _AFX_NO_OLE_SUPPORT
#	include <afxdtctl.h>
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#	include <afxcmn.h>
#endif
#include <afxcontrolbars.h>


// Enable memory leak tracking if building a debug version.
#ifdef _DEBUG
#	ifndef _CRTDBG_MAP_ALLOC
#		define _CRTDBG_MAP_ALLOC
#	endif
#	include <stdlib.h>
#	include <crtdbg.h>
#endif


// Explicitly link to a newer version of the "ComCtl32.dll" to support UI themes on Windows XP and newer OS versions.
// If we don't do this, then the app will link to a "ComCtl32.dll" version that is older than Windows XP.
#ifdef _UNICODE
#	if defined _M_IX86
#		pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#	elif defined _M_X64
#		pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#	else
#		pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#	endif
#endif
