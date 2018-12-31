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

#ifndef __Rtt_LuaAssert_H__
#define __Rtt_LuaAssert_H__
#
# // We limit this code to the Mac simulator because it depends
# // on libunwind which isn't available for __arm__ on iOS.
#
# // This code is currently BROKEN!!! So we'll disable it until it's fixed.
#//if defined( __APPLE__ ) && defined( __x86_64__ )
# if 0
#
#	ifdef __cplusplus
		extern "C"
		{
#	endif
#			// This function gets the current "lua_State" by walking the call stack.
			void Rtt_NotifyLuaInCallstack( const char *condition_string_prefix,
											const char *condition_string );
#	ifdef __cplusplus
		}
#	endif
#
#	// The "if" statement is used to avoid evaluating "A" more than once.
#	define Rtt_LuaAssert( condition_string_prefix, condition )	\
			do \
			{ \
				if( ! (condition) ) \
				{ \
					Rtt_NotifyLuaInCallstack( condition_string_prefix, #condition ); \
					assert( ! #condition ); \
				} \
			} \
			while( 0 )
#
# else // Not defined( __APPLE__ ) && defined( __x86_64__ ).
#
#	define Rtt_LuaAssert( condition_string_prefix, condition )	assert( condition )
#
# endif // if defined( __APPLE__ ) && defined( __x86_64__ )
#
#endif // __Rtt_LuaAssert_H__
