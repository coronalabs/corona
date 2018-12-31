// ----------------------------------------------------------------------------
// 
// CoronaLuaIOS.h
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// ----------------------------------------------------------------------------

#ifndef _CoronaLuaIOS_H__
#define _CoronaLuaIOS_H__

#include "CoronaMacros.h"
#include "CoronaLuaObjCHelper.h"

struct lua_State;

// ----------------------------------------------------------------------------

CORONA_API int CoronaLuaPushImage( lua_State *L, UIImage *image );

// ----------------------------------------------------------------------------

#endif // _CoronaLuaIOS_H__
