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
#include "Rtt_AndroidRuntimeDelegate.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Runtime.h"
#include "NativeToJavaBridge.h"

#include "Rtt_DependencyLoader.h"

#include <android/log.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// Lua Loader for flattened directories
// This allows .so plugins to load specifically on Android.
extern "C" int loader_Cflat (lua_State *L);

// ----------------------------------------------------------------------------

static int
JavaLuaLoader( lua_State *L )
{
	const char *libName = luaL_checkstring( L, 1 );
	const char kClassName[] = "LuaLoader";

	NativeToJavaBridge *bridge = (NativeToJavaBridge*)lua_touserdata( L, lua_upvalueindex(1) );

	int result = bridge->LoadClass( L, libName, kClassName );

	return result;
}

static int
FileLoader( lua_State *L )
{
	const char *fileName = luaL_checkstring( L, 1 );

	NativeToJavaBridge *bridge = (NativeToJavaBridge*)lua_touserdata( L, lua_upvalueindex(1) );

	int result = bridge->LoadFile( L, fileName );

	return result;
}

// ----------------------------------------------------------------------------

/// Creates a new delegate used to receive events from the Corona runtime.
AndroidRuntimeDelegate::AndroidRuntimeDelegate(NativeToJavaBridge *ntjb, bool isCoronaKit)
	: RuntimeDelegatePlayer(), fNativeToJavaBridge(ntjb), fIsCoronaKit(isCoronaKit)
{
}

/// Destructor. Cleans up allocated resources.
AndroidRuntimeDelegate::~AndroidRuntimeDelegate()
{
}

void
AndroidRuntimeDelegate::DidInitLuaLibraries( const Runtime& sender ) const
{
	RuntimeDelegatePlayer::DidInitLuaLibraries( sender );

	lua_State *L = sender.VMContext().L();

	//Loads the path for the so files
	fNativeToJavaBridge->OnRuntimeLoaded(L);

	// Insert lua loader for flattened directories
	// This allows .so plugins to load specifically on Android.
	Lua::InsertPackageLoader( L, & loader_Cflat, -1, fNativeToJavaBridge );

	if (fIsCoronaKit)
	{
		// Lua::InsertPackageLoader( L, & (Corona::DependencyLoader::CCDataLoader), -1 );
		//When require is called in corona kit this will read the file form the assets dir instead of resource.car
		Lua::InsertPackageLoader( L, & FileLoader, -1, fNativeToJavaBridge );
	}
	Lua::InsertPackageLoader( L, & JavaLuaLoader, -1, fNativeToJavaBridge );
}

bool
AndroidRuntimeDelegate::HasDependencies( const Runtime& sender ) const
{
	return ( !fIsCoronaKit ) || ( Corona::DependencyLoader::CCDependencyCheck( sender ) );
}

/// Called just before the "main.lua" file has been loaded.
/// This is the application's opportunity to register custom APIs into Lua.
void AndroidRuntimeDelegate::WillLoadMain(const Runtime& sender) const
{
	// Notify the Java side of Corona that the runtime has been loaded.
	lua_State *L = sender.VMContext().L();

	fNativeToJavaBridge->OnRuntimeWillLoadMain();

	// Show this right before main.lua is loaded and is shown until main.lua ends
	fNativeToJavaBridge->ShowSplashScreen();

	// Push the Corona activity's intent as launch arguments to Lua.
	// Note: This must be done after the OnRuntimeLoaded() call so that it can
	//       create the LuaState Java object wrapper needed by this function call.
	if (Rtt_VERIFY(const_cast<Runtime&>(sender).PushLaunchArgs(true) > 0))
	{
		fNativeToJavaBridge->PushLaunchArgumentsToLuaTable(L);
		lua_pop(L, 1);
	}
}

/// Called after the "main.lua" file has been loaded and executed
void AndroidRuntimeDelegate::DidLoadMain(const Runtime& sender) const
{
	fNativeToJavaBridge->OnRuntimeStarted();
}

/// Called when the "config.lua" file has been loaded into Lua, but before it has been read.
void AndroidRuntimeDelegate::WillLoadConfig(const Runtime& sender, lua_State *L) const
{
}

/// Called after the "config.lua" file has been read.
void AndroidRuntimeDelegate::DidLoadConfig(const Runtime& sender, lua_State *L) const
{
	// Attempt to fetch the Google Push Notifications project number string from the "config.lua" file.
	// If it is there, then register for push notifications.
	const char *googleProjectNumber = NULL;
	int top = lua_gettop( L );
	lua_getglobal( L, "application" );
	if ( lua_istable( L, -1 ) )
	{
		lua_getfield( L, -1, "notification" );
		if ( lua_istable( L, -1 ) )
		{
			lua_getfield( L, -1, "google" );
			if ( lua_istable( L, -1 ) )
			{
				lua_getfield( L, -1, "projectNumber" );
				if ( lua_isstring( L, -1 ) )
				{
					googleProjectNumber = lua_tostring( L, -1 );
				}
			}
		}
	}
	if ( googleProjectNumber != NULL ) {
		// Register for push notifications.
		fNativeToJavaBridge->GooglePushNotificationsRegister( googleProjectNumber );
	}
	else {
		// Unregister from Google's push notification service, if not done already.
		fNativeToJavaBridge->GooglePushNotificationsUnregister();
	}
	lua_settop( L, top );

	const char *googleMapsAPIKey = NULL;
	lua_getglobal( L, "application" );
	if ( lua_istable( L, -1 ) )
	{
		lua_getfield( L, -1, "license" );
		if ( lua_istable( L, -1 ) )
		{
			lua_getfield( L, -1, "google" );
			if ( lua_istable( L, -1 ) )
			{
				lua_getfield( L, -1, "mapsKey" );
				if ( lua_isstring( L, -1 ) )
				{
					googleMapsAPIKey = lua_tostring( L, -1 );
				}
			}
		}
	}
	if ( googleMapsAPIKey != NULL ) {
		fNativeToJavaBridge->SetGoogleMapsAPIKey( googleMapsAPIKey );
	}
	lua_settop( L, top );
}

// Called when the config is pushed to ensure we have something there
void AndroidRuntimeDelegate::InitializeConfig( const Runtime& sender, lua_State *L ) const
{
	static const char kApplication[] = "application";
	static const char kMetadata[] = "metadata";
	static const char kAppId[] = "appId";

	Rtt_LUA_STACK_GUARD( L );

	lua_getglobal( L, kApplication ); // application
	{
		// [Lua] if not application then
		// [Lua]     application = {}
		// [Lua] end
		if ( ! lua_istable( L, -1 ) )
		{
			lua_pop( L, 1 ); // pop non-nil value
			lua_createtable( L, 0, 1 );

			lua_setglobal( L, kApplication ); // application
		}
		
		int index = lua_gettop( L );

		lua_getfield( L, index, kMetadata );
		{
			// [Lua] if not application.metadata then
			// [Lua]     application.metadata = {}
			// [Lua] end
			if ( ! lua_istable( L, -1 ) )
			{
				lua_pop( L, 1 ); // pop non-nil value
				lua_createtable( L, 0, 1 );

				lua_pushvalue( L, -1 ); // leave metadata on stack, so push extra
				lua_setfield( L, index, kMetadata );
			}
		}
		lua_pop( L, 1 );
	}
	lua_pop( L, 1 );
}


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
