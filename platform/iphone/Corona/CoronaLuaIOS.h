//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _CoronaLuaIOS_H__
#define _CoronaLuaIOS_H__

#include "CoronaMacros.h"
#include "CoronaLuaObjCHelper.h"

struct lua_State;

// ----------------------------------------------------------------------------

CORONA_API int CoronaLuaPushImage( lua_State *L, UIImage *image );

// ----------------------------------------------------------------------------

#endif // _CoronaLuaIOS_H__
