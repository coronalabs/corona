//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "CoronaLuaObjC+NSObject.h"

#import "CoronaLuaObjC.h"

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

// ----------------------------------------------------------------------------

@implementation NSObject(CoronaLuaObjC)

- (int)pushLuaValue:(lua_State *)L forKey:(NSString *)key
{
	int result = 0;

	@try
	{
		id value = [self valueForKey:key];
		if ( value && [CoronaLuaObjC pushLuaValue:L forValue:value] )
		{
			result = 1;
		}
	}
	@catch (NSException *exception)
	{
#ifdef Rtt_DEBUG
		NSLog( @"[%@] %@", NSStringFromSelector(_cmd), exception );
#endif
	}

	return result;
}

- (BOOL)set:(lua_State *)L luaValue:(int)index forKey:(NSString *)key
{
	BOOL result = NO;

	id value = [CoronaLuaObjC toValue:L forLuaValue:index];

	if ( value )
	{
		@try
		{
			[self setValue:value forKey:key];
			result = YES;
		}
		@catch (NSException *exception)
		{
#ifdef Rtt_DEBUG
			NSLog( @"[%@] %@", NSStringFromSelector(_cmd), exception );
#endif
		}
	}

	return result;
}

@end

// ----------------------------------------------------------------------------
