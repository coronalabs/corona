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

#include "CoronaLua.h"

#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaUserdataProxy.h"
#include "Rtt_FilePath.h"

#include <string.h>

// ----------------------------------------------------------------------------

CORONA_API
lua_State *CoronaLuaGetCoronaThread( lua_State *coroutine )
{
	return Rtt::Lua::GetCoronaThread( coroutine );
}

CORONA_API
lua_State *CoronaLuaNew( int flags )
{
	bool shouldOpenLibs = kCoronaLuaFlagOpenStandardLibs & flags;
	return Rtt::Lua::New( shouldOpenLibs );
}

CORONA_API
void CoronaLuaDelete( lua_State *L )
{
	Rtt::Lua::Delete( L );
}

CORONA_API
int CoronaLuaNormalize( lua_State *L, int index )
{
	return Rtt::Lua::Normalize( L, index );
}

CORONA_API
void CoronaLuaInitializeContext( lua_State *L, void *context, const char *metatableName )
{
	Rtt::Lua::InitializeContext( L, context, metatableName );
}

CORONA_API
void *CoronaLuaGetContext( lua_State *L )
{
	return Rtt::Lua::GetContext( L );
}

CORONA_API
CoronaLuaRef CoronaLuaNewRef( lua_State *L, int index )
{
	return Rtt::Lua::NewRef( L, index );
}

CORONA_API
void CoronaLuaDeleteRef( lua_State *L, CoronaLuaRef ref )
{
	Rtt::Lua::DeleteRef( L, ref );
}

CORONA_API
int CoronaLuaEqualRef( lua_State *L, CoronaLuaRef ref, int index )
{
	return Rtt::Lua::EqualRef( L, ref, index );
}

CORONA_API
void CoronaLuaNewEvent( lua_State *L, const char *eventName )
{
	Rtt::Lua::NewEvent( L, eventName );
}

CORONA_API
void CoronaLuaDispatchEvent( lua_State *L, CoronaLuaRef listenerRef, int nresults )
{
	Rtt::Lua::DispatchEvent( L, listenerRef, nresults );
}

CORONA_API
void CoronaLuaDispatchRuntimeEvent( lua_State *L, int nresults )
{
	Rtt::Lua::DispatchRuntimeEvent( L, nresults );
}

CORONA_API
int CoronaLuaIsListener( lua_State *L, int index, const char *eventName )
{
	return Rtt::Lua::IsListener( L, index, eventName );
}

CORONA_API
void CoronaLuaPushRuntime( lua_State *L )
{
	Rtt::Lua::PushRuntime( L );
}

CORONA_API
void CoronaLuaRuntimeDispatchEvent( lua_State *L, int index )
{
	if ( Rtt_VERIFY( lua_istable( L, index ) ) )
	{
		Rtt::Lua::RuntimeDispatchEvent( L, index, 0 );
	}
	else
	{
		Rtt_LogException( "[ERROR] Lua::RuntimeDispatchEvent() expected the 'event' at index (%d) to be a Lua table.\n", index );
	}
}

CORONA_API
void CoronaLuaNewGCMetatable( lua_State *L, const char name[], lua_CFunction __gc1 )
{
	Rtt::Lua::NewGCMetatable( L, name, __gc1 );
}

CORONA_API
void CoronaLuaNewMetatable( lua_State* L, const char name[], const luaL_Reg vtable[] )
{
	Rtt::Lua::NewMetatable( L, name, vtable );
}

CORONA_API
void CoronaLuaInitializeGCMetatable( lua_State *L, const char name[], lua_CFunction __gc1 )
{
	Rtt::Lua::InitializeGCMetatable( L, name, __gc1 );
}

CORONA_API
void CoronaLuaInitializeMetatable( lua_State *L, const char name[], const luaL_Reg vtable[] )
{
	Rtt::Lua::InitializeMetatable( L, name, vtable );
}

CORONA_API
void CoronaLuaPushUserdata( lua_State *L, void *ud, const char metatableName[] )
{
	Rtt::Lua::PushUserdata( L, ud, metatableName );
}

CORONA_API
void *CoronaLuaToUserdata( lua_State *L, int index )
{
	return Rtt::Lua::ToUserdata( L, index );
}

CORONA_API
void *CoronaLuaCheckUserdata( lua_State *L, int index, const char metatableName[] )
{
	return Rtt::Lua::CheckUserdata( L, index, metatableName );
}

CORONA_API
void CoronaLuaRegisterModuleLoader( lua_State *L, const char *name, lua_CFunction loader, int nupvalues )
{
	Rtt::Lua::RegisterModuleLoader( L, name, loader, nupvalues );
}

CORONA_API
void CoronaLuaRegisterModuleLoaders( lua_State *L, const luaL_Reg moduleLoaders[], int nupvalues )
{
	Rtt::Lua::RegisterModuleLoaders( L, moduleLoaders, nupvalues );
}

CORONA_API
int CoronaLuaOpenModule( lua_State *L, lua_CFunction loader )
{
	return Rtt::Lua::OpenModule( L, loader );
}

CORONA_API
void CoronaLuaInsertPackageLoader( lua_State *L, lua_CFunction loader, int index )
{
	Rtt::Lua::InsertPackageLoader( L, loader, index );
}

CORONA_API
lua_CFunction CoronaLuaGetErrorHandler()
{
	return Rtt::Lua::GetErrorHandler( NULL );
}

CORONA_API
void CoronaLuaSetErrorHandler( lua_CFunction newValue )
{
	Rtt::Lua::SetErrorHandler( newValue );
}

CORONA_API
int CoronaLuaDoCall( lua_State *L, int narg, int nresults )
{
	return Rtt::Lua::DoCall( L, narg, nresults );
}

CORONA_API
int CoronaLuaDoBuffer( lua_State *L, lua_CFunction loader, lua_CFunction pushargs )
{
	return Rtt::Lua::DoBuffer( L, loader, pushargs );
}

CORONA_API
int CoronaLuaDoFile( lua_State *L, const char* file, int narg, int clear )
{
	return Rtt::Lua::DoFile( L, file, narg, clear );
}

CORONA_API
int CoronaLuaPushModule( lua_State *L, const char *name )
{
	return Rtt::Lua::PushModule( L, name );
}

CORONA_API
void CoronaLuaLog( lua_State *L, const char *fmt, ... )
{
	va_list argp;
	va_start( argp, fmt );

	CoronaLuaLogV( L, fmt, argp );

	va_end( argp );
}

CORONA_API
void CoronaLuaLogPrefix( lua_State *L, const char *prefix, const char *fmt, ... )
{
	va_list argp;
	va_start( argp, fmt );

	CoronaLuaLogPrefixV( L, prefix, fmt, argp );

	va_end( argp );
}

CORONA_API
void CoronaLuaLogV( lua_State *L, const char *fmt, va_list arguments )
{
	CoronaLuaLogPrefixV( L, NULL, fmt, arguments );
}

CORONA_API
void CoronaLuaLogPrefixV( lua_State *L, const char *prefix, const char *fmt, va_list arguments )
{
	Rtt_ASSERT(fmt != NULL);

	char *format = NULL;
	const char *where = "";

	if ( prefix == NULL )
	{
		prefix = ""; // this simplifies things later
	}

	if ( L )
	{
		// Include the location of the call from the Lua context
		luaL_where( L, 1 );
		{
			where = lua_tostring( L, -1 );
		}
		lua_pop( L, 1 );
	}

	format = (char *)malloc( strlen(prefix) + strlen(where) + strlen(fmt) + 1 );

	// prefix and where may be empty strings
	strcpy( format, prefix );
	strcat( format, where );
	strcat( format, fmt );

	Rtt_VLogException( format, arguments );

	free( format );
}

//
// EscapeStringForJSON
//
// Escape a string so that it is suitable for use as a JSON rvalue.  This simply means escaping
// backslashes and doublequotes with a backslash.
//
static char *EscapeStringForJSON(const char *str)
{
    static const int bufLen = 10240;
    static char buf[bufLen];
    const char *p = str;
    char *o = buf;

    for ( int count = 0; *p != '\0' && count < (bufLen - 1); count++ )
    {
        if (*p == '\\')
        {
            *o++ = '\\';
            *o++ = '\\';
        }
        else if (*p == '"')
        {
            *o++ = '\\';
            *o++ = '"';
        }
        else
        {
            *o++ = *p;
        }

        ++p;
    }

    *o = '\0';

    return buf;
}

#ifndef  Rtt_NO_GUI
//
// CoronaLuaPropertyToJSON
//
// Get the top item on the Lua stack and put a JSON style representation in the output buffer
// recursing as necessary to handle tables and userdata proxies.
//
CORONA_API
int CoronaLuaPropertyToJSON(lua_State *L, int idx, const char *key, char *buf, int bufLen, int pos)
{
    Rtt_LUA_STACK_GUARD( L );

    int len = 0;

    // Keys must be sane
    Rtt_ASSERT(strchr(key, '"') == NULL);
    Rtt_ASSERT(strchr(key, '\\') == NULL);

    if (lua_isnumber(L, idx))
    {
        // "%g" is not ideal here because it reverts to "%e" when it can represent the number in fewer characters that way
        // but "%f" is no good because it only emits fixed precision strings (i.e. lots of trailing zeroes).
        len = snprintf(&buf[pos], bufLen, "\"%s\": %g", key, lua_tonumber(L, idx));
    }
    else if (lua_isboolean(L, idx))
    {
        len = snprintf(&buf[pos], bufLen, "\"%s\": \"%s\"", key, (lua_toboolean(L, idx) ? "true" : "false"));
    }
    else if (lua_isstring(L, idx))
    {
        len = snprintf(&buf[pos], bufLen, "\"%s\": \"%s\"", key, EscapeStringForJSON(lua_tostring( L, idx )));
    }
    else if (lua_istable(L, idx) && pos == 0) // don't recurse into tables twice
    {
        len = snprintf(&buf[pos], bufLen, "\"%s\": { ", key);
        int origLen = len;

        // enumerate the table
        int index = CoronaLuaNormalize( L, idx );

        lua_pushnil( L );
        while ( lua_next( L, index ) != 0 )
        {
            // printf("table: %s -> %s (%d)\n", lua_tostring(L, -2), lua_typename(L, lua_type(L, -1)), pos+len);

            if (len != origLen)
            {
                // If this isn't the first thing we've added, add a comma (we may remove it later)
                strncat(&buf[pos+len], ", ", 2);
                len += 2;
            }

            /* uses 'key' (at index -2) and 'value' (at index -1) */
            int tmpLen = CoronaLuaPropertyToJSON(L, -1, lua_tostring(L, -2), buf, (bufLen - len), (pos + len));

            if (tmpLen == 0 && len != origLen)
            {
                // Didn't add anything to buf, remove the comma we optimistically provided
                buf[pos+len-2] = '\0';
                len -= 2;
            }
            else
            {
                len += tmpLen;
            }

            /* removes 'value'; keeps 'key' for next iteration */
            lua_pop(L, 1);
        }

        if (len == origLen)
        {
            // The table added nothing so it was likely an internal artifact which we can elide
            buf[pos] = '\0';
        }
        else
        {
            len += snprintf(&buf[pos+len], (bufLen - len), " }");
        }
    }
    else if (lua_isfunction(L, idx))
    {
#if 0
        // TODO
        // It would be nice to emit the file and linenumber for user defined functions overriding
        // default object functions but the "function pointer" we get here isn't one that we can
        // hand off to lua_getinfo()
        lua_Debug ar;
        lua_getstack(L, idx, &ar);
        lua_pushvalue(L, idx);
        lua_getinfo(L, ">Sl", &ar);

        printf("key: %s type: %s\n", key, luaL_typename( L, idx ));
        snprintf(buf, bufLen, "\"%s\": \"%s\"", key, luaL_typename( L, idx ));
#endif
    }
    else if (lua_isuserdata(L, idx) && (strcmp(key, "path") == 0 || strcmp(key, "fill") == 0 || strcmp(key, "stroke") == 0))
    {
        Rtt::LuaUserdataProxy *proxy = Rtt::LuaUserdataProxy::ToProxy( L, idx );

        // Rtt_TRACE(("proxy: %s: %p\n", key, proxy));

        const Rtt::MLuaUserdataAdapter *adapter = proxy->GetAdapter();
        if ( adapter )
        {
            lua_pushvalue(L, idx);
            lua_pushstring(L, "_properties");

            int result = proxy->Index( L, lua_gettop( L ) - 2 );

            if ( result > 0)
            {
                snprintf(buf, bufLen, "\"%s\": { %s }", key, lua_tostring( L, -1 ));
                lua_pop(L, result);
            }

            // pop the arguments we pushed earlier
            lua_pop(L, 2);
        }
    }
    else if (lua_isuserdata(L, idx))
    {
        // Rtt_TRACE(("userdata: key: %s\n", key));
    }
    else if (lua_isnil(L, idx))
    {
        // We can't encode 'nil' in JSON so we just emit the string
        len = snprintf(&buf[pos], bufLen, "\"%s\": \"nil\"", key);
    }

    else
    {
        // Types unhandled so far are generally internal which aren't interesting

        // snprintf(buf, bufLen, "\"%s\": %s", key, luaL_typename( L, idx ));
    }

    return len;
}
#endif // Rtt_NO_GUI

CORONA_API void CoronaLuaWarning( lua_State *L, const char *fmt, ... )
{
	va_list argp;
	va_start( argp, fmt );

	CoronaLuaLogPrefixV( L, "WARNING: ", fmt, argp );

	va_end( argp );
}

CORONA_API void CoronaLuaError( lua_State *L, const char *fmt, ... )
{
	va_list argp;
	va_start( argp, fmt );

	CoronaLuaLogPrefixV( L, "ERROR: ", fmt, argp );

	va_end( argp );
}

// ----------------------------------------------------------------------------

