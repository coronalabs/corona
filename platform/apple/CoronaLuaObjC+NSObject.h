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
