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
