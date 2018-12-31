//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_LuaCoronaBaseLib.h"
#include "Rtt_Assert.h"
#import <Foundation/Foundation.h>
#import "NSString+Extensions.h"
#include "lua.h"
#include "lauxlib.h"

/* This is an override of Lua's stock "print" function to use NSLog
 * because Apple is starting to redirect stdout/stderr to /dev/null.
 * Can't be static because the .c file needs to see this function.
 */
int Rtt_LuaCoronaBaseLib_print(lua_State *L)
{
	// We need an autoreleasepool because we cannot guarantee one already exists,
	// e.g. used in a pure C/C++ project, used on a background thread, used before Obj-C initialization, etc.
	@autoreleasepool
	{
		int n = lua_gettop(L);  /* number of arguments */
		int i;
		NSMutableArray* stringarray = [NSMutableArray arrayWithCapacity:n];
		
		lua_getglobal(L, "tostring");
		for (i=1; i<=n; i++)
		{
			const char *s;
			lua_pushvalue(L, -1);  /* function to be called */
			lua_pushvalue(L, i);   /* value to print */
			lua_call(L, 1, 1);
			s = lua_tostring(L, -1);  /* get result */
			if (s == NULL)
			{
				return luaL_error(L, LUA_QL("tostring") " must return a string to "
								  LUA_QL("print"));
			}
			
			NSString *nss = [NSString stringWithExternalString:s];
			
			[stringarray addObject:nss];
			
			lua_pop(L, 1);  /* pop result */
		}
		// Lua print separates characters with tabs, so do the same.
		NSString* outputstring = [stringarray componentsJoinedByString: @"\t"];
#if Rtt_IPHONE_ENV || defined( Rtt_TVOS_ENV )
		NSLog(@"%@", outputstring); // NSLog automatically includes a newline
#else
		Rtt_LogException("%s", [outputstring UTF8String]);
#endif
	}
	return 0;
}
