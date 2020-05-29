//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

// This is a pre-compiled header which must be included in all source files
// in this library.

#pragma once


#include "targetver.h"
#include <Windows.h>


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
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "ComCtl32.lib")
