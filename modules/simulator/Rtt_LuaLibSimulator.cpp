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

#include "Rtt_LuaLibSimulator.h"

#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Event.h"
#include "Rtt_MSimulatorServices.h"

#include <string.h>
#include "Rtt_Array.h"
#include "Rtt_SimulatorRecents.h"
#include "Rtt_FileSystem.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static int
show( lua_State *L )
{
	const char *arg = lua_tostring( L, 1 );

	if ( arg )
	{
		Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
		const MSimulatorServices *simulator =
			(const MSimulatorServices *)lua_touserdata( L, lua_upvalueindex( 1 ) );

		if ( strcmp( "buildstatus", arg ) == 0 )
		{
			simulator->SetBuildMessage( lua_tostring( L, 2 ) );
		}
		else if ( strcmp( "new", arg ) == 0 )
		{
			simulator->NewProject();
		}
		else if ( strcmp( "open", arg ) == 0 )
		{
			//const char *open = lua_tostring( L, 2 );
			simulator->OpenProject( lua_tostring( L, 2 ) );
		}
		else if ( strcmp( "build", arg ) == 0 )
		{
			simulator->BuildProject( lua_tostring( L, 2 ) );
		}
		else if ( strcmp( "sampleCode", arg ) == 0 )
		{
			simulator->ShowSampleCode();
		}
		else if ( strcmp( "selectOpenFilename", arg ) == 0 )
		{
            // Usage:   simulator.show( "selectOpenFilename", [fileExtn, [directory,]] listener )
            // Example: simulator.show( "selectOpenFilename", "ext", "~/Desktop", selectOpenFilenameListener )

            LuaResource *resource = NULL;
            int nextArg = 2;
            const char *fileExtn = NULL;
            const char *directory = NULL;
            
            if ( lua_isstring( L, nextArg ) )
            {
                fileExtn = lua_tostring( L, nextArg++ );
            }
            
            if ( lua_isstring( L, nextArg ) )
            {
                directory = lua_tostring( L, nextArg++ );
            }
            
            if ( Lua::IsListener( L, nextArg, CompletionEvent::kName ) )
            {
                // This will be freed by simulator->SelectOpenFilename() when the callback is complete
                resource = Rtt_NEW( & platform.GetAllocator(), LuaResource( LuaContext::GetContext( L )->LuaState(), nextArg ) );
                
                simulator->SelectOpenFilename( directory, fileExtn, resource );
            }
            else
            {
                luaL_error( L, "ERROR: simulator.show('selectOpenFilename', [fileExtn,] listener) requires a listener as the last argument" );
            }
        }
		else if ( strcmp( "selectSaveFilename", arg ) == 0 )
		{
            // Usage:   simulator.show( "selectOpenFilename", [newFilename, [fileExtn, [directory,]]] listener )
            // Example: simulator.show( "selectSaveFilename", "myNewFile", "ext", "~/Desktop", selectSaveFilenameListener )
            
            LuaResource *resource = NULL;
            int nextArg = 2;
            const char *newFilename = NULL;
            const char *fileExtn = NULL;
            const char *directory = NULL;
            
            if ( lua_isstring( L, nextArg ) )
            {
                newFilename = lua_tostring( L, nextArg++ );
            }
            
            if ( lua_isstring( L, nextArg ) )
            {
                fileExtn = lua_tostring( L, nextArg++ );
            }
            
            if ( lua_isstring( L, nextArg ) )
            {
                directory = lua_tostring( L, nextArg++ );
            }
            
            if ( Lua::IsListener( L, nextArg, CompletionEvent::kName ) )
            {
                // This will be freed by simulator->SelectSaveFilename() when the callback is complete
                resource = Rtt_NEW( & platform.GetAllocator(), LuaResource( LuaContext::GetContext( L )->LuaState(), nextArg ) );
                
                simulator->SelectSaveFilename( newFilename, directory, fileExtn, resource );
            }
            else
            {
                luaL_error( L, "ERROR: simulator.show('selectSaveFilename', [newFilename, [fileExtn, [directory,]]] listener) requires a listener as the last argument" );
            }
		}
		else if ( strcmp( "relaunchProject", arg ) == 0 )
		{
			simulator->RelaunchProject( );
		}
		else if ( strcmp( "editProject", arg ) == 0 )
		{
			simulator->EditProject( lua_tostring( L, 2 ) );
		}
		else if ( strcmp( "showFiles", arg ) == 0 )
		{
			simulator->ShowProjectFiles( lua_tostring( L, 2 ) );
		}
		else if ( strcmp( "showSandbox", arg ) == 0 )
		{
			simulator->ShowProjectSandbox( lua_tostring( L, 2 ) );
		}
    }

	return 0;
}

static int
getSubscription( lua_State *L )
{
	Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
	const MSimulatorServices *simulator =
		(const MSimulatorServices *)lua_touserdata( L, lua_upvalueindex( 1 ) );

	S32 timestamp = 0;
	const char *subscription = simulator->GetSubscription( & timestamp );

	lua_pushstring( L, subscription );
	lua_pushinteger( L, timestamp );

	return 2;
}

static int
getCurrProjectPath( lua_State *L )
{
    Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
    const MSimulatorServices *simulator = (const MSimulatorServices *)lua_touserdata( L, lua_upvalueindex( 1 ) );
    
    const char *currProjectPath = simulator->GetCurrProjectPath();
    
    lua_pushstring( L, currProjectPath );
    
    return 1;
}

static int
runExtension( lua_State *L )
{
    Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
    const MSimulatorServices *simulator = (const MSimulatorServices *)lua_touserdata( L, lua_upvalueindex( 1 ) );
    
    simulator->RunExtension(lua_tostring( L, 1));
    
    return 0;
}

static int
setProjectResourceDirectory( lua_State *L )
{
    Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
    MSimulatorServices *simulator = (MSimulatorServices *)lua_touserdata( L, lua_upvalueindex( 1 ) );
    
    simulator->SetProjectResourceDirectory(lua_tostring( L, 1));
    
    return 0;
}

static int
setWindowCloseListener( lua_State *L )
{
    Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
    MSimulatorServices *simulator = (MSimulatorServices *)lua_touserdata( L, lua_upvalueindex( 1 ) );
    
    if ( Lua::IsListener( L, 1, CompletionEvent::kName ) )
    {
        LuaResource *resource = NULL;
        
        // This will be freed by simulator->SelectSaveFilename() when the callback is complete
        resource = Rtt_NEW( & platform.GetAllocator(), LuaResource( LuaContext::GetContext( L )->LuaState(), 1 ) );
        
        simulator->SetWindowCloseListener( resource );
    }
    else if (lua_isnil(L, -1))
    {
        simulator->SetWindowCloseListener( NULL );
    }
    else
    {
        luaL_error( L, "ERROR: simulator.setWindowCloseListener(listener) requires a listener as an argument" );
    }
    
    return 0;
}

static int
closeWindow( lua_State *L )
{
    Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
    MSimulatorServices *simulator = (MSimulatorServices *)lua_touserdata( L, lua_upvalueindex( 1 ) );
    
    simulator->CloseWindow( );
    
    return 0;
}

static int
getRecentProjects( lua_State *L )
{
	Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
	const MSimulatorServices *simulator =
		(const MSimulatorServices *)lua_touserdata( L, lua_upvalueindex( 1 ) );
	
	//Runtime* runtime = LuaContext::GetRuntime( L );
	Rtt::LightPtrArray<RecentProjectInfo> recentDocs(Rtt_AllocatorCreate());
	
	simulator->GetRecentDocs(&recentDocs);
		
	lua_createtable( L, 0, 0);
	{
		int luaHeaderTableStackIndex = lua_gettop( L );

		for (int i = 0, count = 1; i < (int)recentDocs.Length(); i++)
		{
			const char *url		= recentDocs[i]->formattedString.c_str();
			const char *fullUrl = recentDocs[i]->fullURLString.c_str();

			// Only consider paths that still exist
			if (Rtt_FileExists(fullUrl))
			{
				lua_newtable(L);
				{
					int index = lua_gettop( L );
					lua_pushstring( L, url);
					lua_setfield(L, index, "formattedString");

					lua_pushstring( L, fullUrl);
					lua_setfield(L,index, "fullURLString");

				}
				lua_rawseti( L, luaHeaderTableStackIndex, count );

				++count;
			}

			delete recentDocs[i];
		}
	}
	
	return 1;
}

static int
getPreference( lua_State *L )
{
    Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
    const MSimulatorServices *simulator = (const MSimulatorServices *)lua_touserdata( L, lua_upvalueindex( 1 ) );
    
    const char *prefName = NULL;

    if ( lua_isstring( L, 1 ) )
    {
        prefName = lua_tostring( L, 1 );
    }
    else
    {
        luaL_error( L, "ERROR: simulator.getPreference(name) requires a string as the first argument" );
    }

    const char *prefValue = simulator->GetPreference(prefName);
    
    lua_pushstring( L, prefValue );
    
    return 1;
}

static int
setPreference( lua_State *L )
{
    Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
    const MSimulatorServices *simulator = (const MSimulatorServices *)lua_touserdata( L, lua_upvalueindex( 1 ) );
    
    const char *prefName = lua_tostring( L, 1);
    const char *prefValue = NULL;
    
    if ( lua_isstring( L, 1 ) )
    {
        prefName = lua_tostring( L, 1 );
    }
    else
    {
        luaL_error( L, "ERROR: simulator.setPreference(name, value) requires a string as the first argument" );
    }
    
    if ( lua_isstring( L, 2 ) )
    {
        prefValue = lua_tostring( L, 2 );
    }
    else
    {
        luaL_error( L, "ERROR: simulator.setPreference(name, value) requires a string as the second argument" );
    }
    
    simulator->SetPreference(prefName, prefValue);
    
    return 0;
}

static int
setDocumentEdited( lua_State *L )
{
    Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
    const MSimulatorServices *simulator = (const MSimulatorServices *)lua_touserdata( L, lua_upvalueindex( 1 ) );
    
    bool value = false;
    
    if ( lua_isboolean( L, 1 ) )
    {
        value = lua_toboolean( L, 1 );
    }
    else
    {
        luaL_error( L, "ERROR: simulator.setDocumentEdited(value) requires a boolean as its argument" );
    }
    
    simulator->SetDocumentEdited(value);
    
    return 0;
}

static int
getDocumentEdited( lua_State *L )
{
    Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
    const MSimulatorServices *simulator = (const MSimulatorServices *)lua_touserdata( L, lua_upvalueindex( 1 ) );
    
    bool value = simulator->GetDocumentEdited();
    
    lua_pushboolean( L, value );
    
    return 1;

}

    
static int
setCursorRect( lua_State *L )
{
    Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
    const MSimulatorServices *simulator = (const MSimulatorServices *)lua_touserdata( L, lua_upvalueindex( 1 ) );

    // Default values for options.
    const char* cursorName = NULL;
    int x = 0, y = 0, w = 0, h = 0;
    
    if( lua_istable( L, 1 ) )
    {
        // cursor is required
        lua_getfield( L, -1, "cursor" );
        cursorName = luaL_checkstring( L, -1 );
        if( ! Rtt_VERIFY( cursorName ) )
        {
            // Nothing to do.
            lua_pop( L, 1 );
            return 0;
        }
        lua_pop( L, 1 );
        
        lua_getfield( L, -1, "x" );
        if (lua_type( L, -1 ) == LUA_TNUMBER)
        {
            x = (int) luaL_checkreal( L, -1 );
        }
        lua_pop( L, 1 );
        
        lua_getfield( L, -1, "y" );
        if (lua_type( L, -1 ) == LUA_TNUMBER)
        {
			y = (int) luaL_checkreal(L, -1);
        }
        lua_pop( L, 1 );
        
        lua_getfield( L, -1, "width" );
        if (lua_type( L, -1 ) == LUA_TNUMBER)
        {
			w = (int) luaL_checkreal(L, -1);
        }
        lua_pop( L, 1 );
        
        lua_getfield( L, -1, "height" );
        if (lua_type( L, -1 ) == LUA_TNUMBER)
        {
            h = (int) luaL_checkreal( L, -1 );
        }
        lua_pop( L, 1 );
        
    }
    
    simulator->SetCursorRect(cursorName, x, y, w, h);
    
    return 0;
}

static int
setWindowResizeListener( lua_State *L )
{
    Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
    MSimulatorServices *simulator = (MSimulatorServices *)lua_touserdata( L, lua_upvalueindex( 1 ) );
    
    if ( Lua::IsListener( L, 1, CompletionEvent::kName ) )
    {
        LuaResource *resource = NULL;
        
        // This will be freed by simulator->SelectSaveFilename() when the callback is complete
        resource = Rtt_NEW( & platform.GetAllocator(), LuaResource( LuaContext::GetContext( L )->LuaState(), 1 ) );
        
        simulator->SetWindowResizeListener( resource );
    }
    else if (lua_isnil(L, -1))
    {
        simulator->SetWindowResizeListener( NULL );
    }
    else
    {
        luaL_error( L, "ERROR: simulator.setWindowResizeListener(listener) requires a listener as an argument" );
    }
    
    return 0;
}

static int
setWindowTitle( lua_State *L )
{
    Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
    MSimulatorServices *simulator = (MSimulatorServices *)lua_touserdata( L, lua_upvalueindex( 1 ) );
    const char *windowTitle = NULL;
    
    if ( lua_isstring( L, 1 ) )
    {
        windowTitle = lua_tostring( L, 1 );
        
        simulator->SetWindowTitle(windowTitle);
    }
    else
    {
        luaL_error( L, "ERROR: simulator.setWindowTitle(windowTitle) requires a string as the first argument" );
    }
    
    return 0;
}

static int
openTextEditor( lua_State *L )
{
    Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
    MSimulatorServices *simulator = (MSimulatorServices *)lua_touserdata( L, lua_upvalueindex( 1 ) );
    const char *filename = NULL;

    if ( lua_isstring( L, 1 ) )
    {
        filename = lua_tostring( L, 1 );

        simulator->OpenTextEditor(filename);
    }
    else
    {
        luaL_error( L, "ERROR: simulator.openTextEditor(filename) requires a string as the first argument" );
    }
    
    return 0;
}

static int
openColorPanel( lua_State *L )
{
    Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
    MSimulatorServices *simulator = (MSimulatorServices *)lua_touserdata( L, lua_upvalueindex( 1 ) );
    bool gotColorPart = false;
    double r = 0, g = 0, b = 0, a = 1;

    if( lua_istable( L, 1 ) )
    {
        // It's a color table

        lua_getfield( L, 1, "r" );
        if (lua_type( L, -1 ) == LUA_TNUMBER)
        {
            r = luaL_checkreal( L, -1 );
            gotColorPart = true;
        }
        lua_pop( L, 1 );

        lua_getfield( L, 1, "g" );
        if (lua_type( L, -1 ) == LUA_TNUMBER)
        {
            g = luaL_checkreal( L, -1 );
            gotColorPart = true;
        }
        lua_pop( L, 1 );

        lua_getfield( L, 1, "b" );
        if (lua_type( L, -1 ) == LUA_TNUMBER)
        {
            b = luaL_checkreal( L, -1 );
            gotColorPart = true;
        }
        lua_pop( L, 1 );

        lua_getfield( L, 1, "a" );
        if (lua_type( L, -1 ) == LUA_TNUMBER)
        {
            a = luaL_checkreal( L, -1 );
            gotColorPart = true;
        }
        lua_pop( L, 1 );
    }

    if ( Lua::IsListener( L, 2, CompletionEvent::kName ) )
    {
        LuaResource *resource = NULL;

        if (! gotColorPart)
        {
            luaL_error( L, "ERROR: simulator.openColorPanel(color, listener) requires a color table as argument #1" );
        }

        resource = Rtt_NEW( & platform.GetAllocator(), LuaResource( LuaContext::GetContext( L )->LuaState(), 2 ) );

        simulator->OpenColorPanel(r, g, b, a, resource);
    }
    else if (lua_isnil(L, 2))
    {
        simulator->OpenColorPanel(0, 0, 0, 0, NULL);
    }
    else
    {
        luaL_error( L, "ERROR: simulator.openColorPanel(color, listener) requires a listener as argument #2" );
    }

    return 0;
}

static int
analytics( lua_State *L )
{
	Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
	MSimulatorServices *simulator = (MSimulatorServices *)lua_touserdata( L, lua_upvalueindex( 1 ) );
	const char *eventName = NULL;
	const char *keyName = NULL;
	const char *value = NULL;

	if ( lua_isstring( L, 1 ) && lua_isstring( L, 2 ) && lua_isstring( L, 3 ) )
	{
		eventName = lua_tostring( L, 1 );
		keyName = lua_tostring( L, 2 );
		value = lua_tostring( L, 3 );

		simulator->SendAnalytics(eventName, keyName, value);
	}
	else
	{
#ifdef Rtt_DEBUG
		luaL_error( L, "ERROR: simulator.analytics(event, key, value) requires 3 string arguments" );
#endif
	}

	return 0;
}

// ----------------------------------------------------------------------------

const char LuaLibSimulator::kName[] = "simulator";

int
LuaLibSimulator::Open( lua_State *L )
{
	const luaL_Reg kVTable[] =
	{
		{ "show", show },
		{ "getSubscription", getSubscription },
		{ "getRecentProjects", getRecentProjects },
		{ "getCurrentProjectPath", getCurrProjectPath },
		{ "runExtension", runExtension },
		{ "setProjectResourceDirectory", setProjectResourceDirectory },
		{ "setWindowCloseListener", setWindowCloseListener },
		{ "closeWindow", closeWindow },
		{ "getPreference", getPreference },
		{ "setPreference", setPreference },
		{ "setDocumentEdited", setDocumentEdited },
		{ "getDocumentEdited", getDocumentEdited },
		{ "setCursorRect", setCursorRect },
		{ "setWindowResizeListener", setWindowResizeListener },
		{ "setWindowTitle", setWindowTitle },
		{ "openTextEditor", openTextEditor },
		{ "openColorPanel", openColorPanel },
		{ "analytics", analytics },

		{ NULL, NULL }
	};

	Rtt_ASSERT( lua_islightuserdata( L, lua_upvalueindex( 1 ) ) );
	void *context = lua_touserdata( L, lua_upvalueindex( 1 ) );
	lua_pushlightuserdata( L, context );
	luaL_openlib( L, kName, kVTable, 1 ); // leave "simulator" on top of stack

	return 1;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

