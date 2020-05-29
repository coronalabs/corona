//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _CoronaLuaIObjCAdapter_H__
#define _CoronaLuaIObjCAdapter_H__

#include "CoronaMacros.h"


struct lua_State;
@class NSDictionary;

// ----------------------------------------------------------------------------

CORONA_API NSDictionary *CoronaLuaCreateDictionary( lua_State *L, int index );

CORONA_API int CoronaLuaPushValue( lua_State *L, id value);

// ----------------------------------------------------------------------------

#endif // _CoronaLuaIObjCAdapter_H__
