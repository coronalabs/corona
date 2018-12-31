// ----------------------------------------------------------------------------
// 
// CoronaLuaObjCHelper.h
// Copyright (c) 2015 Corona Labs Inc. All rights reserved.
// 
// ----------------------------------------------------------------------------

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
