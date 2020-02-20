//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Foundation/Foundation.h>
#import "NSString+Extensions.h"

struct lua_State;

// ----------------------------------------------------------------------------

// CoronaLuaObjC
// 
// This is a collection of utility functions for bridging Lua and Obj-C
// 
@interface CoronaLuaObjC : NSObject

+ (id)toValue:(struct lua_State *)L forLuaValue:(int)index;

+ (NSMutableDictionary *)toDictionary:(struct lua_State *)L forTable:(int)index;

+ (void)copyEntries:(struct lua_State *)L fromDictionary:(NSDictionary *)src toTable:(int)index;

+ (BOOL)pushTable:(struct lua_State *)L forValue:(NSDictionary *)src;

+ (BOOL)pushArray:(struct lua_State* )L forValue:(NSArray *)src;

+ (BOOL)pushLuaValue:(struct lua_State *)L forValue:(NSArray *)src;

@end

// ----------------------------------------------------------------------------
