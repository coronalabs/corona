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

#include "Core/Rtt_Build.h"

#include "Rtt_LuaAssert.h"

#include "Rtt_PhysicsWorld.h"

// We limit this code to the Mac simulator because it depends
// on libunwind which isn't available for __arm__ on iOS.
#if defined( __APPLE__ ) && defined( __x86_64__ )
#
#	include <assert.h>
#	include <stdio.h>
#	include <string.h>
#
#	include <dlfcn.h> // dlsym().
#	include <libunwind.h> // unw_step().
#	include <mach-o/dyld.h> // _NSGetExecutablePath().
#
#	ifdef __cplusplus
		extern "C"
		{
#	endif
#
#			define lua_c
#
#			include "lua.h"
#			include "lauxlib.h"
#			include "lualib.h"
#
#	ifdef __cplusplus
		}
#	endif
#
#	include "Core/Rtt_Build.h"
#	include "Rtt_LuaContext.h"
#	include "Rtt_Runtime.h"

#	define ENABLE_MY_PRINTF		( 0 )
#
#	if ENABLE_MY_PRINTF
#
#		define MY_PRINTF( ... )		printf( __VA_ARGS__ )
#
#	else // NOT ENABLE_MY_PRINTF
#
#		define MY_PRINTF( ... )
#
#	endif // ENABLE_MY_PRINTF

	namespace // anonymous namespace.
	{
		bool is_a_lua_C_function( unw_cursor_t &cursor )
		{
			// 128 : We only need the first 5 characters, but we'll grab the
			// entire name for debugging purpose. This is the length of the
			// string necessary to find the prefix ("lua?_").
			// "+ 1" : The trailing null character.
			//
			// We don't need the entire function name because we only need to
			// verify its prefix.
			//
			// For function names starting with "_Z", we can demangle the
			// name and verify that the first parameter, following the first
			// "(", is of type "lua_State *". See GCC's abi::__cxa_demangle()
			// in cxxabi.h.
			//
			// We can avoid demangling the function name by parsing it ourselves.
			// We can look for "P9lua_State" immediately after the function name.
			//
			// Then we can get the "lua_State *" by skipping over the implicit
			// "this" parameter.

			char name[ 128 + 1 ];

			unw_word_t unused_ip_offset;
			if( unw_get_proc_name( &cursor,
									name,
									sizeof( name ),
									&unused_ip_offset ) )
			{
				// Can't get the name.
				MY_PRINTF( "%s : Error: unw_get_proc_name() failed.\n",
							__FUNCTION__ );
				return false;
			}

			// We want any functions that start with either "lua_", or "lua?_",
			// where "?" is any character. It's simpler to explicitly look at
			// each character than using regular expressions.

			if( ( name[ 0 ] == '_' ) &&
				( name[ 1 ] == 'Z' ) )
			{
				// This is a mangled C++ function name.
				// Ignore these for now.
				MY_PRINTF( "%s : Skip over C++ functions: %s\n",
							__FUNCTION__,
							name );
				return false;
			}

			if( ( name[ 0 ] == 'l' ) &&
				( name[ 1 ] == 'u' ) &&
				( name[ 2 ] == 'a' ) &&
				( name[ 3 ] == '_' ) )
			{
				// We found a C function name prefixed with "lua_".
				MY_PRINTF( "%s : Found: %s\n",
							__FUNCTION__,
							name );
				return true;
			}

			if( ( name[ 0 ] == 'l' ) &&
				( name[ 1 ] == 'u' ) &&
				( name[ 2 ] == 'a' ) &&
				( name[ 4 ] == '_' ) )
			{
				// We found a C function name prefixed with "lua?_".
				MY_PRINTF( "%s : Found: %s\n",
							__FUNCTION__,
							name );
				return true;
			}

			// Not a function we're interested in.
			MY_PRINTF( "%s : Skip over: %s\n",
						__FUNCTION__,
						name );
			return false;
		}

		bool get_lua_State_from_the_first_C_function_parameter( unw_cursor_t &cursor,
																lua_State *&state )
		{
			// RBP is a 64 bit register that points to the next stack frame.
			// ie: The function that called the current function.
			// ie: The parent caller of the current function.
			//
			// The values preceding the address in RBP are the current
			// function's parameters.
			// ie: To get the first pointer parameter to a function, take
			// the RBP address and subtract the size of one pointer.
			//
			// The values preceding the function's parameters, are the
			// function's local variables. These are the closest to the
			// value of the SP pointer.
			//
			// The function's parameters, in the stack frame, are presented in
			// reverse order of declaration.
			// ie: The first parameter has the largest address.
			// The last parameter has the smallest address.
			//
			// Some of this is discussed here:
			// http://llvm.org/docs/CodeGenerator.html#frame-layout

			unw_word_t rbp;
			if( unw_get_reg( &cursor,
								UNW_X86_64_RBP,
								&rbp ) )
			{
				MY_PRINTF( "%s : Error: unw_get_reg() failed.\n",
							__FUNCTION__ );
				state = NULL;
				return false;
			}

			// "sizeof( ptrdiff_t )" : The size of the first parameter of the
			// function, which should be a "lua_State *".
			//
			// If this was a C++ function, we would need to skip over the first
			// implicit "this" parameter.

#			ifdef Rtt_DEBUG
#
#				define FIRST_ARGUMENT_OFFSET	( 1 )
#
#			else // NOT Rtt_DEBUG.
#
#				// In non-debug builds (release builds), "compact unwind
#				// encoding" (introduced in OSX 10.6) causes the start
#				// of the local function arguments to be offset by 2
#				// extra pointers.
#				define FIRST_ARGUMENT_OFFSET	( 3 )
#
#			endif // Rtt_DEBUG

			state = *(lua_State **)( rbp - ( FIRST_ARGUMENT_OFFSET * sizeof( ptrdiff_t ) ) );
			MY_PRINTF( "%s : rbp: 0x%llx, L derived from rbp location: %p\n",
						__FUNCTION__,
						rbp,
						state );

			if( (ptrdiff_t)state < (ptrdiff_t)0x10000 )
			{
				// This is a sanity-check for bad pointers.
				// This should NEVER happen.
				MY_PRINTF( "%s : Error, L is bad: %p\n",
							__FUNCTION__,
							state );
				state = NULL;
				return false;
			}

			return true;
		}

		lua_State *get_lua_State_from_current_callstack()
		{
			unw_context_t context;
			if( unw_getcontext( &context ) )
			{
				MY_PRINTF( "%s : Error: unw_getcontext() failed.\n",
							__FUNCTION__ );
				return NULL;
			}

			unw_cursor_t cursor;
			// "cursor" is now pointing to the current function.
			// We're NOT interested in the current function, so
			// we'll unw_step() to the next (parent) function
			// before we start to inspect the stack frames.
			if( unw_init_local( &cursor,
								&context ) )
			{
				MY_PRINTF( "%s : Error: unw_init_local() failed.\n",
							__FUNCTION__ );
				return NULL;
			}

			while( unw_step( &cursor ) > 0 )
			{
				// Most Lua functions follow the same pattern:
				//
				//		(1) They're C-based (NOT C++, so there's no "this"
				//		pointer passed as the first argument).
				//
				//		(2) Their name is prefixed with "lua_" or  "lua?_".
				//
				//		(3) Their first parameter is a "lua_State *".

				if( ! is_a_lua_C_function( cursor ) )
				{
					// Not what we're looking for.
					continue;
				}

				lua_State *state;
				if( get_lua_State_from_the_first_C_function_parameter( cursor,
																		state ) )
				{
					// We found the what we want.
					return state;
				}

				// We DIDN'T find what we're looking for.
			}

			return NULL;
		}

	} // anonymous namespace.

	void Rtt_NotifyLuaInCallstack( const char *condition_string_prefix,
									const char *condition_string )
	{
		lua_State *L = get_lua_State_from_current_callstack();
		if( ! L )
		{
			// Nothing to do.
			return;
		}

		if( ! Rtt::LuaContext::GetRuntime( L )->GetPhysicsWorld().GetLuaAssertEnabled() )
		{
			// Nothing to do.
			return;
		}

		// This typedef MUST stay in sync with any changes made to luaL_error().
		typedef int (*luaL_error_t) (lua_State *L, const char *fmt, ...);

		// RTLD_MAIN_ONLY isn't POSIX. It's specific to __APPLE__.
		// It avoids having to dlopen() / dlclose() the current executable.
		luaL_error_t luaL_error_ptr = (luaL_error_t)dlsym( RTLD_MAIN_ONLY,
															"luaL_error" );
		if( ! luaL_error_ptr )
		{
			fprintf( stderr,
						"%s\n",
						dlerror() );

			// Nothing to do.
			return;
		}

		// Report the error to Lua.
		{
			// Concatenate the condition string with its prefix.
			char *complete_condition_string = (char *)malloc( strlen( condition_string_prefix ) +
																strlen( condition_string ) +
																1 ); // Trailing NULL character.
			strcpy( complete_condition_string,
					condition_string_prefix );
			strcat( complete_condition_string,
					condition_string );

			// Call luaL_error().
			(*luaL_error_ptr)( L,
								complete_condition_string );

			free( complete_condition_string );
		}
	}

#endif // defined( __APPLE__ ) && defined( __x86_64__ ).
