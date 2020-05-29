//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import <Foundation/NSObject.h>

struct lua_State;

// ----------------------------------------------------------------------------

// NSObject(CoronaLuaObjC)
// 
// This is an informal protocol that allows properties of Obj-C objects
// to be accessible via Lua.
// 
// The principal use case is when we wrap an Obj-C type (e.g. native display object)
// and want to provide a generic way to access its "raw" properties. We say "raw"
// because we normally define a cross-platform property name, but in this use case,
// we would be exposing the actual underlying Obj-C property name.
// 
@interface NSObject(CoronaLuaObjC)

// Uses KVC to get receiver's Obj-C property value corresponding to 'key', and 
// then pushes the corresponding Lua value on the stack. Returns 1 if success; 
// 0 otherwise.
- (int)pushLuaValue:(struct lua_State *)L forKey:(NSString *)key;

// Converts Lua value at 'index' to equivalent Obj-C value, and then uses
// KVC to set receiver property value for the given 'key'. Returns YES 
// if a value was set successfully; NO otherwise.
- (BOOL)set:(struct lua_State *)L luaValue:(int)index forKey:(NSString *)key;

@end

// ----------------------------------------------------------------------------
