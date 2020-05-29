//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "CoronaIOSLoader.h"
#include "Rtt_CKWorkFlowConstants.h"

#include <dlfcn.h>

#import <Foundation/Foundation.h>

// ----------------------------------------------------------------------------

namespace Corona
{

// ----------------------------------------------------------------------------

int
IOSLoader::SymbolLoader( lua_State *L )
{
	const char *libName = luaL_checkstring( L, 1 );

	// push libNameFlattened: replace '.' with '_'
	const char *libNameFlattened = luaL_gsub( L, libName, ".", "_" );

	// push funcName
	const char *funcName = lua_pushfstring( L, "luaopen_%s", libNameFlattened );

//#ifdef Rtt_TVOS_ENV
//	// NOTE: static libraries must be linked into CoronaCards.framework
//	// via the -force_load flag, e.g. libnetwork.a
//
//	// On TVOS, Corona is not linked in the main exe, so
//	void *handle = RTLD_SELF; // Search the image containing this function
//#else
//	void *handle = RTLD_MAIN_ONLY; // Search only the main exe image
//#endif
	void *handle = RTLD_DEFAULT; // Search everywhere

	lua_CFunction openLib = (lua_CFunction)dlsym( handle, funcName );

	// pop libNameFlattened and funcName
	lua_pop( L, 2 );

	if ( openLib )
	{
		lua_pushcfunction( L, openLib );
	}

	return 1;
}

// Library names passed to `require()` can be used to locate the
// corresponding framework and function loader
//
// * Naming convention
//    + Framework: "Corona_" + libNameFlattened + ".framework"
//    + Function:  "luaopen_" + libNameFlattened
// * Example
//    + Framework: Corona_plugin_zip.framework
//    + Function:  luaopen_plugin_zip
//
// NOTE: Returns NULL when the bundle/framework doesn't exist
//       or when the bundle's code fails to load.
static const char *
GetExecutableCodePath( const char *libNameFlattened, bool shouldLoad )
{
	const char *result = NULL;

	// Corona_plugin_X
	NSString *frameworkName = [NSString stringWithFormat:@"Corona_%s", libNameFlattened];
	
	// Corona_plugin_X.framework
	NSString *bundleName = [frameworkName stringByAppendingPathExtension:@"framework"];
	
	// A.app/Frameworks/Corona_plugin_X.framework
	NSString *bundlePath = [[[NSBundle mainBundle] privateFrameworksPath] stringByAppendingPathComponent:bundleName];

	NSBundle *bundle = [NSBundle bundleWithPath:bundlePath];
	if ( bundle )
	{
		bool isLoaded = [bundle isLoaded];

		// For some reason, dlopen/dlsym will return a bogus function pointer
		// if the framework does NOT contain Obj-C code (e.g. a pure C/C++ plugin).
		// Apparently, this is b/c dlopen will not automatically load the code
		// in that framework, so we load the code manually if it's not loaded.
		if ( shouldLoad && ! isLoaded )
		{
			isLoaded = [bundle load];

			if ( ! isLoaded )
			{
				CORONA_LOG_WARNING( "Could not load code in framework (%s)", [bundlePath UTF8String] );
			}
		}

		if ( isLoaded )
		{
			// A.app/Frameworks/Corona_plugin_X.framework/Corona_plugin_X
			NSString *executableCodePath = [bundlePath stringByAppendingPathComponent:frameworkName];
			
			result = [executableCodePath UTF8String];
		}
	}

	return result;
}

int
IOSLoader::FrameworkSymbolLoader( lua_State *L )
{
	const char *libName = luaL_checkstring( L, 1 );

	// push libNameFlattened: replace '.' with '_'
	const char *libNameFlattened = luaL_gsub( L, libName, ".", "_" );

	// push funcName
	const char *funcName = lua_pushfstring( L, "luaopen_%s", libNameFlattened );

	const char *path = GetExecutableCodePath( libNameFlattened, true );
	if ( path )
	{
		void *handle = dlopen( path, RTLD_LAZY );

		if ( handle )
		{
			lua_CFunction openLib = (lua_CFunction)dlsym( handle, funcName );

			// pop libNameFlattened and funcName
			lua_pop( L, 2 );

			if ( openLib )
			{
				lua_pushcfunction( L, openLib );
			}

			dlclose( handle );
		}
	}

	return 1;
}

void
IOSLoader::Register( lua_State *L, void *platformContext )
{
	CoronaLuaInitializeContext( L, platformContext, NULL );

	// table.insert( package.loaders, -1, SymbolLoader )
	Lua::InsertPackageLoader( L, & SymbolLoader, -1 );

	// table.insert( package.loaders, -1, FrameworkSymbolLoader )
	Lua::InsertPackageLoader( L, & FrameworkSymbolLoader, -1 );
}

// ----------------------------------------------------------------------------

} // namespace Corona

// ----------------------------------------------------------------------------

