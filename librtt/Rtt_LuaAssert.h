//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
