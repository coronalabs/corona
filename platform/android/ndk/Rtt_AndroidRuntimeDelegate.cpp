//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
#include <dlfcn.h>

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

#if !defined(RTLD_NODELETE)
	#define RTLD_NODELETE 0
#endif

static int
AndroidZipSoLoader( lua_State *L )
{
	lua_pushliteral(L, "");
	const char *libName = luaL_checkstring( L, 1 );
    const char *libNameFlattened = luaL_gsub(L, libName, ".", "_");
    const char *funcName = lua_pushfstring(L, "luaopen_%s", libNameFlattened);

    lua_getglobal(L, "package");

	lua_CFunction res = (lua_CFunction) dlsym(RTLD_DEFAULT, funcName);
    if (res == NULL) {
        lua_pushvalue(L, 2);
        lua_pushfstring(L, "\n\tno global loaded symbol '%s'", funcName);
        lua_concat(L, 2);
        lua_replace(L, 2);
    }

	lua_getfield(L, -1, "APKs");
	if(lua_istable(L, -1) && res == NULL)
	{
	    lua_getfield(L, -1, "abi");
	    const char* abi = lua_tostring(L, -1);
	    if(abi != NULL) {
            int length = (int) lua_objlen(L, -2);
            for (int i = 0; i < length && res == NULL; i++) {
                lua_rawgeti(L, -2, i + 1);
                const char *apk = luaL_checkstring(L, -1);
                const char *dlopenName = lua_pushfstring(L, "%s!/lib/%s/lib%s.so", apk, abi, libName);
                void *handle = dlopen(dlopenName, RTLD_NOW | RTLD_GLOBAL | RTLD_NODELETE);
                if (handle) {
                    res = (lua_CFunction) dlsym(handle, funcName);
                    // dlclose(handle); // this crashes Android
                }
                if (res == NULL) {
                    lua_pushvalue(L, 2);
                    lua_pushfstring(L, "\n\tno zipped .so symbol at '%s'", dlopenName);
                    lua_concat(L, 2);
                    lua_replace(L, 2);
                }
                lua_pop(L, 2);
            }
        }
		lua_pop(L, 1); // abi
	}
	lua_pop(L, 4);

	if ( res )
	{
	    lua_pop(L, 1); // pop error string
		lua_pushcfunction( L, res );
	}

	return 1;
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
	Lua::InsertPackageLoader( L, & AndroidZipSoLoader, -1 );
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
