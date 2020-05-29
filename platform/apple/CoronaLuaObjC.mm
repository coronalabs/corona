//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import "CoronaLuaObjC.h"

#include "Rtt_Lua.h"

#if !defined( Rtt_NO_GUI )
	#include "Rtt_LuaLibSystem.h"

	#ifdef Rtt_IPHONE_ENV
		#import <UIKit/UIImage.h>
	#endif
#endif

// ----------------------------------------------------------------------------

// If a supported value exists in srcObject for key 'srcKey', then set it in 
// the table at 'index' using key 'dstKey'. Returns false if could not add
// the value to the table.
static bool
SetField( lua_State *L, int index, const char *dstKey, id srcObject, NSString *srcKey )
{
	Rtt_ASSERT( index > 0 );

	bool result = false;

	id value = [srcObject valueForKey:srcKey];
	if ( value )
	{
		result = [CoronaLuaObjC pushLuaValue:L forValue:value];
		if ( result )
		{
			lua_setfield( L, index, dstKey );
		}
	}

	return result;
}

// ----------------------------------------------------------------------------

@implementation CoronaLuaObjC

+ (id)toValue:(lua_State *)L forLuaValue:(int)index
{
	using namespace Rtt;

	Rtt_LUA_STACK_GUARD( L );

	index = Lua::Normalize( L, index );

	id result = nil;
	
	int t = lua_type( L, index );

	if ( LUA_TSTRING == t )
	{
		const char *value = lua_tostring( L, index );
		result = [NSString stringWithExternalString:value];
	}
	else if ( LUA_TNUMBER == t )
	{
		double value = lua_tonumber( L, index );
		result = [NSNumber numberWithDouble:value];
	}
	else if ( LUA_TBOOLEAN == t )
	{
		BOOL value = lua_toboolean( L, index );
		result = [NSNumber numberWithBool:value];
	}
	else if ( LUA_TTABLE == t )
	{
#ifdef Rtt_NO_GUI
#else
			LuaLibSystem::FileType fileType;
			int numResults = LuaLibSystem::PathForTable( L, index, fileType );
			if ( 0 == numResults )
			{
				result = [CoronaLuaObjC toDictionary:L forTable:index];
			}
			else
			{
				// PathForTable will leave the result at the top of the stack
				const char *path = lua_tostring( L, -1 );
				if ( path )
				{
					NSString *p = [[NSString alloc] initWithUTF8String:path];
#ifdef Rtt_IPHONE_ENV
					if ( LuaLibSystem::kImageFileType == fileType )
					{
						result = [UIImage imageWithContentsOfFile:p];
					}
					else
#endif // Rtt_IPHONE_ENV
					{
						result = [NSData dataWithContentsOfFile:p];
					}
					[p release];
				}

				lua_pop( L, numResults );
			}
#endif // Rtt_NO_GUI

	}
	else
	{
		// Unsupported type
		Rtt_ASSERT_NOT_IMPLEMENTED();
	}

	return result;
}


+ (NSMutableDictionary *)toDictionary:(lua_State *)L forTable:(int)t
{
	Rtt_LUA_STACK_GUARD( L );

	Rtt_ASSERT( t > 0 ); // TODO: Map to positive, absolute index
	if ( 0 >= t ) { return nil; }

	NSMutableDictionary *params = [NSMutableDictionary dictionaryWithCapacity:2];

	lua_checkstack( L, 3 );

	lua_pushnil( L );
	while ( lua_next( L, t ) != 0 )
	{
		id key = [CoronaLuaObjC toValue:L forLuaValue:-2];
		if ( key )
		{
			id value = [CoronaLuaObjC toValue:L forLuaValue:-1];
			if ( value )
			{
				[params setValue:value forKey:key];
			}
		}

		lua_pop( L, 1 ); // pop value; keeps key for next iteration
	}

	return params;
}

+ (void)copyEntries:(lua_State *)L fromDictionary:(NSDictionary *)src toTable:(int)index
{
	using namespace Rtt;

	Rtt_LUA_STACK_GUARD( L );

	if ( lua_istable( L, index ) )
	{
		index = Lua::Normalize( L, index );

		for ( NSString *key in src )
		{
			SetField( L, index, [key UTF8String], src, key );
		}
	}
}

+ (BOOL)pushTable:(lua_State *)L forValue:(NSDictionary *)src
{
	NSUInteger count = [src count];
	lua_createtable( L, 0, (int) count );

	[CoronaLuaObjC copyEntries:L fromDictionary:src toTable:-1];

	return true;
}

+ (BOOL)pushArray:(lua_State *)L forValue:(NSArray *)src
{
	bool result = true;
	
	lua_checkstack(L, 3); // is it really 3? table+key+value?
	lua_newtable(L);
	int table_index = lua_gettop(L);
	int current_lua_array_index = 1;
	for(id an_element in src)
	{
		// recursively add elements
		if ( ! [CoronaLuaObjC pushLuaValue:L forValue:an_element] )
		{
			result = false;
		}
		lua_rawseti(L, table_index, current_lua_array_index);
		current_lua_array_index++;
	}

	return result;
}

+ (BOOL)pushLuaValue:(lua_State *)L forValue:(id)src
{
	bool result = true;

	if ( [src isKindOfClass:[NSString class]] )
	{
		const char *str = [(NSString*)src UTF8String];
		lua_pushstring( L, str );
	}
	else if ( [src isKindOfClass:[NSNumber class]] )
	{
		NSNumber *n = (NSNumber*)src;
		if ( [NSNumber numberWithBool:YES] == n || [NSNumber numberWithBool:NO] == n )
		{
			BOOL b = [n boolValue];
			lua_pushboolean( L, b );
		}
		else
		{
			double v = [n doubleValue];
			lua_pushnumber( L, v );
		}
	}
	else if ( [src isKindOfClass:[NSArray class]] )
	{
		NSArray* a = (NSArray*)src;
		result = [CoronaLuaObjC pushArray:L forValue:a];
	}
	else if ( [src isKindOfClass:[NSDictionary class]] )
	{
		NSDictionary *d = (NSDictionary*)src;
		result = [CoronaLuaObjC pushTable:L forValue:d];
	}
	else
	{
		result = false;
	}

	return result;
}

@end

// ----------------------------------------------------------------------------
