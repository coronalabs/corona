//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Rtt_LuaCoronaBaseLib.h"
#include "Rtt_Assert.h"
#import <Foundation/Foundation.h>
#import "NSString+Extensions.h"
#include "lua.h"
#include "lauxlib.h"

#if Rtt_IPHONE_ENV || defined( Rtt_TVOS_ENV )
#import <os/log.h>
#endif

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
        os_log_with_type(OS_LOG_DEFAULT, OS_LOG_TYPE_DEFAULT, "%{public}s", [outputstring UTF8String]);
		//NSLog(@"%@", outputstring); // NSLog automatically includes a newline
#else
		Rtt_LogException("%s", [outputstring UTF8String]);
#endif
	}
	return 0;
}
