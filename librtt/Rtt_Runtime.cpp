//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Solar2D game engine.
// With contributions from Dianchu Technology
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_Runtime.h"

#include "Core/Rtt_Allocator.h"
#include "Core/Rtt_String.h"
#include "Display/Rtt_Display.h"
#include "Display/Rtt_GroupObject.h"
#include "Display/Rtt_SpritePlayer.h"
#include "Display/Rtt_StageObject.h"
#include "Rtt_Archive.h"
#include "Display/Rtt_BufferBitmap.h"
#include "Rtt_Event.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaFile.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_MPlatform.h"
#include "Rtt_MPlatformDevice.h"
#include "Rtt_MRuntimeDelegate.h"
#include "Rtt_PhysicsWorld.h"
#include "Rtt_PlatformExitCallback.h"
#include "Rtt_PlatformTimer.h"
#include "Rtt_Scheduler.h"
#include "Display/Rtt_TextObject.h"
#include "Rtt_LuaFrameworks.h"
#include "Rtt_HTTPClient.h"

#ifdef Rtt_USE_ALMIXER
	#include "Rtt_PlatformOpenALPlayer.h"
#endif
#ifdef Rtt_USE_WEBMIXER
#include "Rtt_PlatformWebAudioPlayer.h"
#endif
#ifdef Rtt_USE_OPENSLES
#include "Rtt_PlatformOpenSLESPlayer.h"
#endif

// ----------------------------------------------------------------------------

namespace Rtt
{
#if defined( Rtt_AUTHORING_SIMULATOR )
extern "C" int luaopen_coronabaselib(lua_State * L);

void Runtime::FinalizeWorkingThreadWithEvent(Runtime* runtime, lua_State* L)
{
	std::thread* curWorkingThread = runtime->m_fAsyncThread.load();
	if (curWorkingThread) {
		curWorkingThread->join();
		std::string* strResult = runtime->m_fAsyncResultStr.load();
		Lua::Ref listener = runtime->m_fAsyncListener.load();

		if (L) {
			Lua::NewEvent(L, "async");
			if (strResult->empty()) {
				lua_pushnil(L);
			} else {
				lua_pushstring(L, strResult->c_str());
			}
			lua_setfield(L, -2, "result");
			Lua::DispatchEvent(L, listener, 0);
			Lua::DeleteRef(L, listener);
		}

		delete curWorkingThread;
		delete strResult;

		runtime->m_fAsyncListener.store(nullptr);
		runtime->m_fAsyncResultStr.store(nullptr);
		runtime->m_fAsyncThread.store(nullptr);
	}
}

int Runtime::ShellPluginCollector_Async(lua_State* L)
{
	Runtime *runtime = (Runtime*)lua_touserdata(L, lua_upvalueindex(1));

	std::string args(lua_tostring(L, 1));
	Lua::Ref listener = nullptr;
	if (lua_type(L, 2) == LUA_TFUNCTION) {
		listener = Lua::NewRef(L, 2);
	}

	FinalizeWorkingThreadWithEvent(runtime, L);
	
	std::thread* thread = new std::thread([](std::string args, Lua::Ref listener, Runtime* runtime) {
		lua_State* L = Rtt::Lua::New(true);
		lua_pushcfunction(L, luaopen_coronabaselib);
		lua_pushstring(L, "coronabaselib");
		lua_call(L, 1, 0);
		const char* debugBuildProcess = getenv("DEBUG_BUILD_PROCESS");
		if (debugBuildProcess) {
			lua_pushstring(L, debugBuildProcess);
			lua_setglobal(L, "debugBuildProcess");
		}

		HTTPClient::registerFetcherModuleLoaders(L);
		std::string result;
		lua_getglobal(L, "require");
		lua_pushstring(L, "CoronaBuilderPluginCollector");
		int res = lua_pcall(L, 1, 1, NULL);
		lua_getfield(L, -1, "collect");
		lua_pushstring(L, args.c_str());
		res = lua_pcall(L, 1, 1, NULL);
		if (lua_type(L, -1) == LUA_TSTRING) {
			result = lua_tostring(L, -1);
		}
		Rtt::Lua::Delete(L);
		runtime->m_fAsyncListener.store(listener);
		runtime->m_fAsyncResultStr.store(new std::string(result));
	}, args, listener, runtime);

	if (listener) {
		lua_pushnil(L);
		runtime->m_fAsyncListener.store(listener);
		runtime->m_fAsyncThread.store(thread);
	}
	else {
		thread->join();
		std::string* result = runtime->m_fAsyncResultStr.load();
		if (result->empty()) {
			lua_pushnil(L);
		}
		else {
			lua_pushstring(L, result->c_str());
		}
		FinalizeWorkingThreadWithEvent(runtime, NULL);
	}
	return 1;
}

#endif
// ----------------------------------------------------------------------------

// These iterations are reasonable default values. See http://www.box2d.org/forum/viewtopic.php?f=8&t=4396 for discussion.
// NOT USED: const S32 kVelocityIterations = 8;
// NOT_USED const S32 kPositionIterations = 3;

#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
static const char kFusePluginName[] = "plugin.fuse";
static const char kFusePublisherId[] = "com.coronalabs";
#endif

// ----------------------------------------------------------------------------

Runtime::Runtime(const MPlatform& platform, MCallback* viewCallback)
	: fAllocator(platform.GetAllocator()),
	fPlatform(platform),
	fStartTime(Rtt_GetAbsoluteTime()),
	fStartTimeCorrection(0),
	fSuspendTime(0),
	fResourcesHead(Rtt_NEW(&fAllocator, CachedResource(*this, NULL))),
	fDisplay(Rtt_NEW(&fAllocator, Display(*this))),
	fVMContext(LuaContext::New(Allocator(), platform, this)),
	fTimer(platform.CreateTimerWithCallback(viewCallback ? *viewCallback : *this)),
	fScheduler(Rtt_NEW(&fAllocator, Scheduler(*this))),
	fArchive(NULL),
	fPhysicsWorld(Rtt_NEW(&fAllocator, PhysicsWorld(fAllocator))),
	fBackend("glBackend"),
	fBackendState(nullptr),
#ifdef Rtt_USE_ALMIXER
	fOpenALPlayer(NULL),
#endif
	fFPS(30),
	fIsSuspended(-1), // uninitialized
	fProperties(0),
	fSuspendOverrideProperties(kSuspendAll),
	fFrame(0),
	fLaunchArgsRef(LUA_NOREF),
	fSimulatorPlatformName(NULL),
	fDownloadablePluginsRef(LUA_NOREF),
	fDownloadablePluginsCount(0),
	fDelegate(NULL),
	fShowingTrialMessages(false),
#ifdef Rtt_AUTHORING_SIMULATOR
	m_fAsyncListener(nullptr),
	m_fAsyncResultStr(nullptr),
	m_fAsyncThread(nullptr),
#endif
	fErrorHandlerRecursionGuard( false )
{
	Rtt_TRACE_SIM( ( "\n%s\n", Rtt_STRING_COPYRIGHT ) );
	Rtt_TRACE_SIM( ( "\tVersion: %s\n", Rtt_STRING_VERSION ) );
	Rtt_TRACE_SIM( ( "\tBuild: %s\n", Rtt_STRING_BUILD ) );

	fResourcesHead->Retain();

	if ( ! Rtt_VERIFY( fVMContext )
		 || ! Rtt_VERIFY( fTimer ) )
	{
		platform.RaiseError( MPlatform::kOutOfMemoryError, NULL );
	}

	// TODO: Move this to Mac/Win simulator-side of code
	#ifdef Rtt_AUTHORING_SIMULATOR
		// Simulator does not use resource.car file
		SetProperty( kIsApplicationNotArchived, true );
		SetProperty( kIsLuaParserAvailable, true );
		SetProperty( kShouldVerifyLicense, true );
	#endif
}

Runtime::~Runtime()
{
	// Notify the owner that this runtime object is about to be destroyed.
	if (fDelegate)
	{
		fDelegate->WillDestroy( *this );
	}

	OnSystemEvent( SystemEvent::kOnAppExit );

#ifdef Rtt_USE_ALMIXER
	if ( fOpenALPlayer )
	{
		PlatformOpenALPlayer::SharedInstance()->RuntimeWillTerminate( * this );
		PlatformOpenALPlayer::ReleaseInstance();
		fOpenALPlayer = NULL;
	}	
#endif

	Rtt_DELETE( fArchive );
	Rtt_DELETE( fScheduler );
	fTimer->Stop();
	Rtt_DELETE( fTimer );
	
	// Notify textures that everything is about to be destroyed. Clean up all dependencies.
	// This is required because Textures and Objects may be dependent on Lua state, or have to notify plugins
	fDisplay->Teardown();

	LuaContext::Delete( fVMContext );

	// Marking the Lua context as NULL invalidates the resource cache.
	// We do this before we delete the display list to prevent resources in the
	// display list from unnecessarily removing themselves from a bogus cache!
	fVMContext = NULL;

	// Lua VM no longer exists, so Corona app is technically no longer executing.
	// This also stops TextureFactory::GetTextureMemoryUsed() from going negative,
	// since it will count images loaded by shell.lua as being removed.
	SetProperty( kIsApplicationExecuting, false );

	fPhysicsWorld->WillDestroyDisplay();

	// Must delete this *after* Lua is destroyed, since certain Lua finalizers
	// rely on fDisplay being valid
	Rtt_DELETE( fDisplay );
	
	// Technically, C++ rules say we should destroy fWorld before fDisplayList,
	// but fDisplayList has display objects that own b2Body objects and require
	// the fWorld to exist in order to call DestroyBody properly.  Therefore, we
	// postpone destroying the world.  Muhahahaha!
#ifdef Rtt_PHYSICS
	fPhysicsWorld->StopWorld();
	Rtt_DELETE( fPhysicsWorld );
#endif

	fResourcesHead->Release();
#if defined(Rtt_AUTHORING_SIMULATOR)
	FinalizeWorkingThreadWithEvent(this, nullptr);
#endif
}

// ----------------------------------------------------------------------------

class LoadMainGuard
{
	Rtt_CLASS_NO_DYNAMIC_ALLOCATION

	public:
		LoadMainGuard( const Runtime& runtime );
		~LoadMainGuard();

	private:
		const Runtime& fRuntime;
		const MRuntimeDelegate *fDelegate;
};

LoadMainGuard::LoadMainGuard( const Runtime& runtime )
:	fRuntime( runtime ),
	fDelegate( runtime.GetDelegate() )
{
	if ( fDelegate )
	{
		LuaStackGuard guard( runtime.VMContext().L(), "WillLoadMain()" );

		fDelegate->WillLoadMain( runtime );
	}
}

LoadMainGuard::~LoadMainGuard()
{
	if ( fDelegate )
	{
		LuaStackGuard guard( fRuntime.VMContext().L(), "DidLoadMain()" );

		fDelegate->DidLoadMain( fRuntime );
	}
}

// ----------------------------------------------------------------------------


// DEPRECATED: the current splash screen does not use this mechanism at all
// (see AppDelegate.mm on iOS or CoronaActivity.java on Android)

// LoadMainTask is used by the authoring simulator only. Its main purpose is to
// wrap up code in shell.lua for displaying splash screen and status bars.
class LoadMainTask : public Task
{
	public:
		LoadMainTask( DisplayObject *splashScreen );

	public:
		virtual void operator()( Scheduler& sender );

	private:
		DisplayObject *fSplashScreen;
};

LoadMainTask::LoadMainTask( DisplayObject *splashScreen )
:	fSplashScreen( splashScreen )
{
}

void
LoadMainTask::operator()( Scheduler& sender )
{
	Runtime& runtime = sender.GetOwner();

	if ( fSplashScreen )
	{
		DisplayObject& child = *fSplashScreen;

		// No need to add to Orphanage b/c this object is not visible to main.lua
		GroupObject *parent = child.GetParent();
		S32 index = parent->Find( child );
		if ( Rtt_VERIFY( index >= 0 ) )
		{
			parent->Remove( index );
		}
	}

	// Rtt_ASSERT( ! runtime.IsProperty( Runtime::kIsApplicationExecuting ) );
	runtime.SetProperty( Runtime::kIsApplicationExecuting, true );

	LoadMainGuard guard( runtime );

	int narg = runtime.PushLaunchArgs( false );

	if ( runtime.IsProperty( Runtime::kIsApplicationNotArchived ) )
	{
		// Load from individual Lua file
		const char kMain[] = Rtt_LUA_SCRIPT_FILE( "main" );
		String filePath( runtime.GetAllocator() );
		runtime.Platform().PathForFile( kMain, MPlatform::kResourceDir, MPlatform::kDefaultPathFlags, filePath );
		if ( Rtt_VERIFY( filePath.GetString() ) )
		{
			bool connectToDebugger = runtime.IsProperty( Runtime::kIsDebuggerConnected );
			runtime.VMContext().DoFile( filePath.GetString(), connectToDebugger, narg );
		}
	}
	else
	{
		// Load from resource.car
		const char kMain[] = Rtt_LUA_OBJECT_FILE( "main" );
		(void) Rtt_VERIFY( 0 == runtime.GetArchive()->DoResource( runtime.VMContext().L(), kMain, narg ) );
	}

	runtime.OnSystemEvent( SystemEvent::kOnAppStart );
}


static int
onShellComplete( lua_State* L )
{
	Runtime* runtime = LuaContext::GetRuntime( L );
	Scheduler& scheduler = runtime->GetScheduler();

	DisplayObject* splashScreen = ( lua_isnoneornil( L, 1 ) ? NULL : (DisplayObject*)LuaProxy::GetProxyableObject( L, 1 ) );
	LoadMainTask* e = Rtt_NEW( runtime->Allocator(), LoadMainTask( splashScreen ) );

	scheduler.Append( e );

	return 0;
}

#if defined( Rtt_AUTHORING_SIMULATOR )
static int
exitCallback( lua_State* L )
{
	Runtime* runtime = LuaContext::GetRuntime( L );
	int code = luaL_optint( L, 1, 0 );	
	PlatformExitCallback* callback = runtime->GetExitCallback();
	if ( callback )
	{
		(*callback)(code);
	}
	return 0;
}



int luaload_config_require(lua_State* L);

// restrictLibs( subscription )
static int
restrictLibs( lua_State *L )
{
	// Create application.metadata global table that luaload_config_require
	// assumes already exists
	lua_newtable( L ); // t1
	{
		lua_newtable( L ); // t2

		lua_pushstring( L, "developer" ); // push 'developer'
		lua_setfield( L, -2, "mode" ); // t2.mode = 'developer'

		lua_pushvalue( L, 1 ); // push subscription
		lua_setfield( L, -2, "subscription" ); // t2.subscription = subscription

	}
	lua_setfield( L, -2, "metadata" ); // t1 = t2

	lua_setglobal( L, "application" ); // application = t2

	(void) Rtt_VERIFY( 0 == Lua::DoBuffer( L, luaload_config_require, NULL ) );

	lua_pushnil( L );
	lua_setglobal( L, "application" ); // application = nil

	return 0;
}

static int
pushShellArgs( lua_State* L )
{
	Runtime* runtime = LuaContext::GetRuntime( L );
	const MPlatform& platform = runtime->Platform();
	Rtt::String value( runtime->GetAllocator() );

	lua_createtable( L, 0, 8 ); // params
	{
		// Pass overlay as first argument to shell.lua
		GroupObject *overlay = runtime->GetDisplay().Overlay(); Rtt_ASSERT( overlay );
		overlay->InitProxy( L );
		overlay->GetProxy()->PushTable( L );
		lua_setfield( L, -2, "overlay" ); // params.overlay

		// Pass function to schedule LoadMainTask
		lua_pushcfunction( L, onShellComplete );
		lua_setfield( L, -2, "onShellComplete" ); // params.onShellComplete

		// Pass table of status bar file names
		lua_newtable( L );
		{
			platform.GetPreference( MPlatform::kDefaultStatusBarFile, &value );
			if ( ! value.IsEmpty() )
			{
				lua_pushstring( L, value.GetString() );
				lua_setfield( L, -2, "default" );
			}

			platform.GetPreference( MPlatform::kDarkStatusBarFile, &value );
			if ( ! value.IsEmpty() )
			{
				lua_pushstring( L, value.GetString() );
				lua_setfield( L, -2, "dark" );
			}

			platform.GetPreference( MPlatform::kTranslucentStatusBarFile, &value );
			if ( ! value.IsEmpty() )
			{
				lua_pushstring( L, value.GetString() );
				lua_setfield( L, -2, "translucent" );
			}

			platform.GetPreference( MPlatform::kLightTransparentStatusBarFile, &value );
			if ( ! value.IsEmpty() )
			{
				lua_pushstring( L, value.GetString() );
				lua_setfield( L, -2, "lightTransparent" );
			}

			platform.GetPreference( MPlatform::kDarkTransparentStatusBarFile, &value );
			if ( ! value.IsEmpty() )
			{
				lua_pushstring( L, value.GetString() );
				lua_setfield( L, -2, "darkTransparent" );
			}

			platform.GetPreference( MPlatform::kScreenDressingFile, &value );
			if ( ! value.IsEmpty() )
			{
				lua_pushstring( L, value.GetString() );
				lua_setfield( L, -2, "screenDressing" );
			}
		}
		lua_setfield( L, -2, "statusBarFiles" ); // params.statusBarFiles

		lua_pushboolean( L, runtime->IsProperty( Runtime::kIsDebuggerConnected ) );
		lua_setfield( L, -2, "isDebuggerConnected" ); // params.isDebuggerConnected

		if ( runtime->GetExitCallback() )
		{
			lua_pushcfunction( L, exitCallback );
			lua_setfield( L, -2, "exitCallback" ); // params.exitCallback
		}

		runtime->PushDownloadablePlugins( L );
		lua_setfield( L, -2, "plugins" );
		
#if defined( Rtt_AUTHORING_SIMULATOR )
		lua_pushlightuserdata(L, runtime);
		lua_pushcclosure(L, &Runtime::ShellPluginCollector_Async, 1);
		lua_setfield( L, -2, "shellPluginCollector");
#endif

		const char *name = runtime->GetSimulatorPlaformName();
		if ( name )
		{
			lua_pushstring( L, name );
			lua_setfield( L, -2, "platform" );
		}
        
        lua_pushboolean( L, runtime->IsProperty( Runtime::kIsSimulatorExtension ) );
		lua_setfield( L, -2, "isSimulatorExtension" ); // params.isSimulatorExtension
	}

	return 1; // params
}

#else // ! defined( Rtt_AUTHORING_SIMULATOR )

static int
pushShellArgs( lua_State* L )
{
	int result = 1;
	
	Runtime* runtime = LuaContext::GetRuntime( L );
	Rtt::String value( runtime->GetAllocator() );

	lua_createtable( L, 0, 2 ); // params
	{
		// Pass overlay as first argument to shell.lua
		GroupObject *overlay = runtime->GetDisplay().Overlay();
		if (overlay)
		{
			overlay->InitProxy( L );
			overlay->GetProxy()->PushTable( L );
			lua_setfield( L, -2, "overlay" ); // params.overlay
		
			bool showTrialMessage = runtime->IsShowingTrialMessage();
			lua_pushboolean(L, showTrialMessage);
			lua_setfield(L, -2, "showMessage");
		}
		

		// Pass function to schedule LoadMainTask
		lua_pushcfunction( L, onShellComplete );
		lua_setfield( L, -2, "onShellComplete" ); // params.onShellComplete

	}

	return result;
}

#endif // defined( Rtt_AUTHORING_SIMULATOR )

// shell.lua is pre-compiled into bytecodes and then placed in a byte array
// constant in shell.cpp. The following function defined in shell.cpp loads
// the bytecodes via luaL_loadbuffer. The .cpp file is dynamically generated.
int luaload_shell(lua_State* L);




// ----------------------------------------------------------------------------

// TODO: Remove this wrapper
S32
Runtime::ViewableContentWidth() const
{
	return fDisplay->ViewableContentWidth();
}

// TODO: Remove this wrapper
S32
Runtime::ViewableContentHeight() const
{
	return fDisplay->ViewableContentHeight();
}

// TODO: Remove this wrapper
S32
Runtime::WindowWidth() const
{
	return fDisplay->WindowWidth();
}

// TODO: Remove this wrapper
S32
Runtime::WindowHeight() const
{
	return fDisplay->WindowHeight();
}

void
Runtime::InitializeArchive( const char *filePath )
{
	Rtt_ASSERT( ! fArchive );

	fArchive = Rtt_NEW( & fAllocator, Archive( fAllocator, filePath ) );

	// Add loader
	lua_State *L = fVMContext->L();
	Lua::InsertPackageLoader( L, & Archive::ResourceLoader, 1 );
}

static const char kApplication[] = "application";

#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN

// Extracts the metadata, and pass it to Fuse:
//    local fuse = require "plugin.fuse"
//    local params = ... -- from config.lua
//    fuse._start( params )
//
// Assumes "application" table from config.lua is at index
//
/*
-- config.lua
application =
{
	-- This is a PRIVATE override mechanism
	-- This is for situations where we need to provide the developer a manual
	-- override mechanism, e.g. the publisher relations team needs to do a one-off
	fuse =
	{
		-- Format 1: single string
		appKey = "string",

		-- Format 2: table of strings
		appKey = {
			ios = "key",
			android = "key",
		}
	},

	-- Standard use case for Corona
	-- This is generated by the build server
	metadata =
	{
		appKey = nil,

		-- In the future, we could change this to be a table (as above)
		-- since plugins will have to recognize both formats
	},
	
	-- Standard use case for CoronaEnterprise
	-- Nothing. Just manually pass appKey:
	--
	-- [main.lua]
	-- fuse.init( appKey, listener )
}
*/
void
Runtime::InitializeFuse( lua_State *L, int index )
{
	int top = lua_gettop( L );

	index = Lua::Normalize( L, index );

	// [Lua] local fuse = require "plugin.fuse"
	// NOTE: Normally we'd use Lua::PushModule( "plugin.fuse" )
	// but we want this to silently fail so we have to drop down
	// and use low level calls instead.
	lua_getglobal( L, "require" );
	lua_pushstring( L, kFusePluginName );
	int status = lua_pcall( L, 1, 1, 0 );

	if ( 0 == status )
	{
		lua_getfield( L, -1, "_start" );

		if ( LUA_TFUNCTION == lua_type( L, -1 ) )
		{
			// Enable manual-override
			// [Lua] local params = application.fuse
			lua_getfield( L, index, "fuse" ); // application.fuse from config.lua

			if ( lua_isnil( L,  -1 ) )
			{
				// None found
				lua_pop( L, 1 ); // pop nil

				// Use server-provided metadata instead
				// [Lua] local params = application.metadata
				lua_getfield( L, index, "metadata" ); // application.metadata from config.lua
			}
			
			// [Lua] fuse._start( params )
			Lua::DoCall( L, 1, 0 );
		}
	}

	lua_settop( L, top );
}
#endif // AUTO_INCLUDE_MONETIZATION_PLUGIN

int
Runtime::InitializeMetadataShim( lua_State *L )
{
	Runtime *runtime = (Runtime *)lua_touserdata( L, lua_upvalueindex( 1 ) );
	runtime->InitializeMetadata( L, 1 );
	return 0;
}

void
Runtime::InitializeMetadata( lua_State *L, int index )
{
	index = Lua::Normalize( L, index );
	lua_getfield( L, index, "permissions" );
	{
		if ( lua_istable( L, -1 ) )
		{
			lua_getfield( L, -1, "advancedGraphics" );
			{
				bool canUseAdvancedGraphics = (bool)lua_toboolean( L, -1 );

				// TODO: SetRestricted() is deprecated in favor of SetRestrictedFeature().
				// Once we migrate all calls from ShouldRestrict => ShouldRestrictFeature,
				// we can remove this call, as it's no longer needed.
				GetDisplay().SetRestricted( ! canUseAdvancedGraphics );

				// TODO: As part of migrating to ShouldRestrictFeature(),
				// we need to verify this works!!! Below is untested!!!
				if ( ! canUseAdvancedGraphics )
				{
					// Set 'false' for *all* features
					for ( int i = Display::kDisplayNewSnapshot; i < Display::kNumFeatures; i++ )
					{
						Display::Feature f = (Display::Feature)i;
						GetDisplay().SetRestrictedFeature( f, false );
					}
				}
			}
			lua_pop( L, 1 );

			lua_getfield( L, -1, "customFragment" );
			{
				bool canUseCustomShaders = (bool)lua_toboolean( L, -1 );
				GetDisplay().SetRestrictedFeature( Display::kGraphicsDefineEffectFragment, ! canUseCustomShaders );
			}
			lua_pop( L, 1 );

			lua_getfield( L, -1, "customVertex" );
			{
				bool canUseCustomShaders = (bool)lua_toboolean( L, -1 );
				GetDisplay().SetRestrictedFeature( Display::kGraphicsDefineEffectVertex, ! canUseCustomShaders );
			}
			lua_pop( L, 1 );
		}
	}
	lua_pop( L, 1 );
}

bool
Runtime::PushConfig( lua_State *L, bool shouldRestrictLibs )
{
	bool result = false;

	int status = LUA_ERRFILE; // assume file does not exist

	Rtt_ASSERT( 0 == lua_gettop( L ) );

	// [Lua] initializeMetadata = & Runtime::initializeMetadataShim
	lua_pushlightuserdata( L, this );
	lua_pushcclosure( L, InitializeMetadataShim, 1 );
	lua_setglobal( L, "initializeMetadata" );
	{
	#ifdef Rtt_AUTHORING_SIMULATOR
		if ( shouldRestrictLibs )
		{
			// NOTE: The build server Execute restrictLibs and config_require
			Rtt::String value( GetAllocator() );
			fPlatform.GetPreference( MPlatform::kSubscription, &value );

			// [Lua] restrictLibs( value )
			lua_pushcfunction( L, restrictLibs );
			lua_pushstring( L, value.GetString() );
			VMContext().DoCall( L, 1, 0 );
		}
	#endif

		if ( IsProperty( kIsApplicationNotArchived ) )
		{
			// Load from individual Lua file
			const char kConfig[] = Rtt_LUA_SCRIPT_FILE( "config" );

			String filePath( GetAllocator() );
			fPlatform.PathForFile( kConfig, MPlatform::kResourceDir, MPlatform::kTestFileExists, filePath );
			const char *path = filePath.GetString();
			if ( path )
			{
				status = VMContext().DoFile( path, 0, true );
			}
			else
			{
				// other cases assume a non-zero status means there's an error msg on the stack
				// so push a "fake" error msg on the stack so we are consistent with those cases
				lua_pushnil( L );
			}
		}
		else
		{
			// Load from resource.car
			const char kConfig[] = Rtt_LUA_OBJECT_FILE( "config" );
			status = GetArchive()->DoResource( L, kConfig, 0 );
		}
	}
	// [Lua] initializeMetadata = nil
	lua_pushnil( L );
	lua_setglobal( L, "initializeMetadata" );

	// Check config.lua was loaded successfully
	if ( 0 == status )
	{
		if ( fDelegate )
		{
			fDelegate->InitializeConfig( * this, L );
		}

		lua_getglobal( L, kApplication ); // application

		if ( lua_istable( L, -1 ) )
		{
#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
			// Read server-generated metadata before popping application
			InitializeFuse( L, -1 );
#endif
			
			lua_getfield( L, -1, "showRuntimeErrors" );
            bool runtimeErrorsExplicitlySet = ! lua_isnil( L, -1 );
            SetProperty( kShowRuntimeErrorsSet, runtimeErrorsExplicitlySet );

            if ( lua_toboolean( L, -1 ) )
			{
				SetProperty( kShowRuntimeErrors, true );
			}
			lua_pop( L, 1 );

			lua_getfield( L, -1, "content" ); // application.content
			result = lua_istable( L, -1 );

			if ( result )
			{
				lua_remove( L, -2 ); // application
			}
			else
			{
				lua_pop( L, 2 ); // pop application and application.content
			}
		}
		else
		{
			lua_pop( L, 1 ); // pop application
		}
	}
	else
	{
		lua_pop( L, lua_gettop( L ) ); // pop and ignore error msg (we don't have a config.lua file)
	}

	Rtt_ASSERT( ( ! result && 0 == lua_gettop( L ) )
				|| ( result && 1 == lua_gettop( L ) ) );

	return result;
}

void
Runtime::ReadConfig( lua_State *L )
{
	// Rtt_ASSERT( ! fDisplay );
	Rtt_ASSERT( 1 == lua_gettop( L ) );	
	Rtt_ASSERT( lua_istable( L, -1 ) );

	lua_getfield( L, -1, "multisample" );
	if ( lua_toboolean( L, -1 ) != 0 )
	{
		fDisplay->SetAntialiased( true );
	}
	lua_pop( L, 1 );

	lua_getfield( L, -1, "fps" );
	int fps = (int) lua_tointeger( L, -1 );
	if ( 60 == fps )	// Besides default (30), only 60 fps is supported
	{
		Rtt_ASSERT( ! IsProperty( kIsApplicationLoaded ) );
		fFPS = 60;
	}
	lua_pop( L, 1 );

	// Apparently this is used for automated testing (set application.content.exitOnError in config.lua)
	lua_getfield( L, -1, "exitOnError" );
	if ( lua_toboolean( L, -1 ) )
	{
		SetProperty( kUseExitOnErrorHandler, true );
	}
	lua_pop( L, 1 );
	
#ifdef Rtt_USE_ALMIXER
	lua_getfield( L, -1, "audioPlayFrequency" );
	int frequency = (int) lua_tointeger( L, -1 );
	PlatformOpenALPlayer::SetFrequency( frequency );
	lua_pop( L, 1 );

	lua_getfield( L, -1, "maxSources" );
	int sources = (int) lua_tointeger( L, -1 );
	PlatformOpenALPlayer::SetMaxSources( sources );
	lua_pop( L, 1 );	
#endif
	

	Rtt_ASSERT( 1 == lua_gettop( L ) );	
}

void
Runtime::PopAndClearConfig( lua_State *L )
{
	Rtt_ASSERT( 1 == lua_gettop( L ) );
	Rtt_ASSERT( lua_istable( L, -1 ) );

	// pop "application.content"
	lua_pop( L, 1 );

	// set global table "application" to nil
	lua_pushnil( L );
	lua_setglobal( L, kApplication );

	Rtt_ASSERT( 0 == lua_gettop( L ) );
}

void
Runtime::AddDownloadablePlugin(
	lua_State *L, const char *pluginName, const char *publisherId,
	int downloadablePluginsIndex, bool isSupportedOnThisPlatform, const char *pluginEntryJSON)
{
	Rtt_LUA_STACK_GUARD( L );

	lua_createtable( L, 0, 2 );
	{
		lua_pushstring( L, pluginName );
		lua_setfield( L, -2, "pluginName" );

		lua_pushstring( L, publisherId );
		lua_setfield( L, -2, "publisherId" );

		lua_pushboolean( L, isSupportedOnThisPlatform ? 1 : 0 );
		lua_setfield( L, -2, "isSupportedOnThisPlatform" );
		
		lua_pushstring(L, pluginEntryJSON);
		lua_setfield(L, -2, "json");
	}
	++fDownloadablePluginsCount;
	lua_rawseti( L, downloadablePluginsIndex, fDownloadablePluginsCount );
}

// Determine list of plugins that the project is using based on build.settings
void
Runtime::FindDownloadablePlugins( const char *simPlatformName )
{
	fSimulatorPlatformName = simPlatformName;

	// Create temporary Lua state to prevent build.settings from having
	// any side effects from the Runtime's L state. Note the separate
	// 'runtimeL' variable for Runtime's L below.
	lua_State *L = luaL_newstate();
	luaL_openlibs(L);
	
	Lua::RegisterModuleLoader( L, "lpeg", luaopen_lpeg );
	Lua::RegisterModuleLoader( L, "dkjson", Lua::Open< luaload_dkjson > );
	Lua::RegisterModuleLoader( L, "json", Lua::Open< luaload_json > );
	lua_getglobal(L, "require");
	lua_pushstring(L, "json");
	lua_pcall(L, 1, 1, 0);
	lua_getfield(L, -1, "encode");
	lua_setglobal(L, "jsonEncode");
	lua_pop(L, 1); // pop json

	const char kBuildSettings[] = "build.settings";
	
	String filePath( & fPlatform.GetAllocator() );
	fPlatform.PathForFile( kBuildSettings, MPlatform::kResourceDir, MPlatform::kTestFileExists, filePath );

	const char *p = filePath.GetString();
	if ( p
		 && 0 == luaL_loadfile( L, p )
		 && 0 == lua_pcall( L, 0, 0, 0 ) )
	{
		lua_getglobal( L, "settings" );
		if ( lua_istable( L, -1 ) )
		{
			lua_getfield(L, -1, "plugins");
			if (lua_istable(L, -1))
			{
				lua_State *runtimeL = fVMContext->L();
				Rtt_LUA_STACK_GUARD(runtimeL);

				// Create table to store downloadable plugins
				lua_createtable(runtimeL, 0, 0);
				lua_pushvalue(runtimeL, -1); // luaL_ref pops so push extra reference
				fDownloadablePluginsRef = luaL_ref(runtimeL, LUA_REGISTRYINDEX);
				int downloadablePluginsIndex = lua_gettop(runtimeL);

				int pluginsIndex = lua_gettop(L);
				lua_pushnil(L); // push first key
				while (lua_next(L, pluginsIndex) != 0)
				{
					bool availableOnPlatform = true;

					if (lua_type(L, -2) != LUA_TSTRING)
					{
						Rtt_LogException("ERROR: Skipping invalid plugin entry in %s file", kBuildSettings);
						lua_pop(L, 1); // pop value; keep 'key' for next iteration
						continue;
					}

					// key at -2, value at -1
					const char *pluginName = lua_tostring(L, -2);

					// these names are compatible with those used in things like "excludeFiles"
					// (they should probably be defined in a central place but are not)
#ifdef Rtt_MAC_ENV
					const char *primaryOSKey = "osx";
#elif Rtt_WIN_DESKTOP_ENV
					const char *primaryOSKey = "win32";
#elif Rtt_LINUX_ENV
					const char *primaryOSKey = "linux";
#else
					// this code shouldn't run on any other platform
					const char *primaryOSKey = "not-used";
#endif

					// ensure the stack contains a table at top
					if (lua_istable(L, -1))
					{
						lua_getglobal(L, "jsonEncode");
						lua_pushvalue(L, -2);
						lua_pcall(L, 1, 1, 0);
						const char *pluginJson = lua_tostring(L, -1);
						String pluginJsonStr;
						pluginJsonStr.Set(pluginJson);
						lua_pop(L, 1);
						
						// Honor the settings for "supportedPlatforms", if any
						lua_getfield(L, -1, "supportedPlatforms");
						{
							if (lua_istable(L, -1))
							{
								// If "supportedPlatforms" exists, the default for platforms changes to false
								// (if it doesn't exist at all, platforms are assumed to be supported)
								availableOnPlatform = false;

								lua_getfield(L, -1, primaryOSKey);
								if (lua_isboolean(L, -1))
								{
									availableOnPlatform = (bool)lua_toboolean(L, -1);
								}
								lua_pop(L, 1); // pop platform
							}
						}
						lua_pop(L, 1); // pop supportedPlatforms

						// Add the plugin to our download list if supported.
						// Note: Ignore the "supportedPlatforms" in the simulator, because we always want to
						//       download the plugin's simulator stub if available.
#ifndef Rtt_AUTHORING_SIMULATOR
						if (availableOnPlatform)
#endif
						{
							lua_getfield(L, -1, "publisherId");
							{
								const char *publisherId = lua_tostring(L, -1); // push publisherId

								// Transfer information to Runtime's list of downloadable plugins.
								// Note, we use runtimeL instead of L here!
								AddDownloadablePlugin(
									runtimeL, pluginName, publisherId, downloadablePluginsIndex, availableOnPlatform, pluginJsonStr.GetString());
							}
							lua_pop(L, 1); // pop publisherId
						}
					}
					else
					{
						Rtt_LogException("ERROR: Plugin entry '%s' must be a table in %s file", pluginName, kBuildSettings);
					}

					lua_pop(L, 1); // pop value; keep 'key' for next iteration
				}

#ifdef AUTO_INCLUDE_MONETIZATION_PLUGIN
				// Always download the fuse plugin stub
				AddDownloadablePlugin(runtimeL, kFusePluginName, kFusePublisherId, downloadablePluginsIndex, false, NULL);
#endif

				lua_pop(runtimeL, 1); // pop downloadablePlugins table
			}
			lua_pop(L, 1); // pop plugins
		}
	}

	lua_close( L );
}

void
Runtime::PushDownloadablePlugins( lua_State *L )
{
	lua_rawgeti( L, LUA_REGISTRYINDEX, fDownloadablePluginsRef );
}

bool
Runtime::RequiresDownloadablePlugins() const
{
	return ( fDownloadablePluginsCount > 0 );
}

int
Runtime::PushLaunchArgs( bool createIfNotExists )
{
	lua_State *L = fVMContext->L(); Rtt_ASSERT( L );

	if ( LUA_NOREF == fLaunchArgsRef )
	{
		if ( createIfNotExists )
		{
			lua_createtable( L, 0, 0 );

			lua_pushvalue( L, -1 ); // luaL_ref pops so push extra reference
			fLaunchArgsRef = luaL_ref( L, LUA_REGISTRYINDEX );
		}
		else
		{
			lua_pushnil( L );
		}
	}
	else
	{
		lua_rawgeti( L, LUA_REGISTRYINDEX, fLaunchArgsRef );
	}

	return 1;
}

void
Runtime::ClearLaunchArgs()
{
	lua_State *L = fVMContext->L(); Rtt_ASSERT( L );

	if ( LUA_NOREF != fLaunchArgsRef )
	{
		luaL_unref( L, LUA_REGISTRYINDEX, fLaunchArgsRef );
		fLaunchArgsRef = LUA_NOREF;
	}
}

Runtime::LoadParameters::LoadParameters()
:	launchOptions( kDefaultLaunchOption ),
	orientation( DeviceOrientation::kUpright ),
	contentWidth( -1 ),
	contentHeight( -1 )
{
}

void
Runtime::LoadParameters::UpdateConfig( lua_State *L, int configIndex ) const
{
	Rtt_ASSERT( configIndex > 0 );

	if ( contentWidth > 0 && contentHeight > 0 )
	{
		lua_pushinteger( L, contentWidth );
		lua_setfield( L, configIndex, "width" );

		lua_pushinteger( L, contentHeight );
		lua_setfield( L, configIndex, "height" );
	}
}

Runtime::LoadApplicationReturnCodes
Runtime::LoadApplication( const LoadParameters& parameters )
{
	U32 launchOptions = parameters.launchOptions;
	DeviceOrientation::Type orientation = parameters.orientation;

	LoadApplicationReturnCodes result = Runtime::kGeneralFail;

	const char kArchive[] = Rtt_CAR_FILE( "resource" );
	const char *basename = kArchive;

	// Use kSystemResourceDir b/c resource.car should always live inside the .app bundle
	String filePath( GetAllocator() );
#if defined( Rtt_ANDROID_ENV )
    fPlatform.PathForFile( basename, MPlatform::kResourceDir, MPlatform::kDefaultPathFlags, filePath );
#else
	fPlatform.PathForFile( basename, MPlatform::kSystemResourceDir, MPlatform::kDefaultPathFlags, filePath );
#endif

	{
		// Init VM
		fVMContext->Initialize( fPlatform, this );
		if ( ! IsProperty( kIsLuaParserAvailable ) )
		{
			fVMContext->DisableParser( ( launchOptions & kConnectToDebugger ) );
		}

		if ( ! IsProperty( kIsCustomEffectsAvailable ) )
		{
			
		}

		if ( ! IsProperty( kIsApplicationNotArchived ) )
		{
			if ( filePath.IsEmpty() )
			{
				goto exit_gracefully;
			}
			InitializeArchive( filePath.GetString() );
		}
		
		// ---------------------------------------------------------------

		lua_State *L = VMContext().L();
		bool shouldRestrictLibs = ! ( launchOptions & kUnlockFeatures );
		bool hasConfig = PushConfig( L, shouldRestrictLibs );
		if ( hasConfig )
		{
			if ( fDelegate )
			{
				fDelegate->WillLoadConfig( * this, L );
			}
			ReadConfig( L ); // assumes fDisplay is NULL
		}
		else
		{
			// Always create a table to simplify logic later
			lua_createtable( L, 0, 2 );
		}
		int configIndex = lua_gettop( L );
		parameters.UpdateConfig( L, configIndex );

		// We need a runtime guard to lock in the OpenGL context.
		// The runtime guard is re-entrant safe. It doesn't look like we invoke multiple runtime guards (though I didn't verify),
		// but it should be safe to do.
		RuntimeGuard guard( * this );

		fDisplay->Initialize( L, configIndex, orientation, fBackend, fBackendState );

		if ( fDelegate )
		{
			fDelegate->DidLoadConfig( * this, L );
		}

		// Send launch analytics.  The function is defined in init.lua so that its lifetime is
		// sufficient to make asynchronous network requests.  We call it here because the
		// "application" table from "config.lua" is needed for some data items.
		lua_getglobal( L, "sendLaunchAnalytics" ); Rtt_ASSERT( lua_isfunction( L, -1 ) );

		if ( Lua::DoCall(L, 0, 1) == 0)
		{
			if(lua_isstring(L, -1))
			{
				fBuildId.Set(lua_tostring(L,-1));
			}
			lua_pop(L, 1);
		}
		else
		{
			// Intended to be silent on failure in release builds
			Rtt_TRACE(("init: launchAnalytics failed"));
		}

		PopAndClearConfig( L );
		
		// ---------------------------------------------------------------
		
		// Now that the config.lua has been read, startup any libraries that depend on those values
#if defined ( Rtt_USE_ALMIXER )
		if ( launchOptions & kDisableAudio )
		{
			fOpenALPlayer = NULL;
		}
		else
		{
			fOpenALPlayer = PlatformOpenALPlayer::RetainInstance();

			fOpenALPlayer->AttachNotifier( Rtt_NEW( GetAllocator(), PlatformNotifier( VMContext().LuaState() ) ) );
		}
#endif
		bool connectToDebugger = ( launchOptions & kConnectToDebugger );
		SetProperty( kIsDebuggerConnected, connectToDebugger );
		
		//If there's no delegate to check "dependencies" for CoronaKit, then exit
		if ( fDelegate )
		{
			// Obfuscated name - for CoronaKit we check "dependencies", everywhere else always returns true
			if ( fDelegate->HasDependencies( *this ) ) // license check
			{
				// If kLaunchDeviceShell is set, run shell.lua which will, in turn, run main.lua (via onShellComplete())
				if ( launchOptions & kLaunchDeviceShell )
				{
					// Load and invoke shell.lua bytecodes
					bool retVal = Rtt_VERIFY( 0 == fVMContext->DoBuffer( luaload_shell, false, pushShellArgs ) );
					result = retVal ? Runtime::kSuccess : Runtime::kGeneralFail;
				}
				else
				{
					// If kLaunchDeviceShell is not set, just schedule main.lua
					LoadMainTask* e = Rtt_NEW( Allocator(), LoadMainTask( NULL ) );
					GetScheduler().Append( e );
					
					result = Runtime::kSuccess;
				}
			}
			else
			{
				result = Runtime::kSecurityIssue;
			}
		}
	}

	if ( result == Runtime::kSuccess )
	{
		SetProperty( kIsApplicationLoaded, true );
	}

exit_gracefully:
	return result;
}

Runtime::LoadApplicationReturnCodes
Runtime::LoadApplication( U32 launchOptions, DeviceOrientation::Type orientation )
{
	LoadParameters parameters;
	parameters.launchOptions = launchOptions;
	parameters.orientation = orientation;
	
	return LoadApplication( parameters );
}

// TODO: Remove this wrapper
void
Runtime::SetContentOrientation( DeviceOrientation::Type newOrientation )
{
	Rtt_ASSERT( fDisplay );

	fDisplay->SetContentOrientation( newOrientation );
}

// TODO: Remove this wrapper
void
Runtime::WindowDidRotate( DeviceOrientation::Type newOrientation, bool isNewOrientationSupported )
{
	Rtt_ASSERT( fDisplay );

	fDisplay->WindowDidRotate( newOrientation, isNewOrientationSupported );
}

// TODO: Remove this wrapper
void
Runtime::WindowSizeChanged()
{
	fDisplay->WindowSizeChanged();
}

void
Runtime::BeginRunLoop()
{
	const U32 kFps = fFPS;
	const U32 kInterval = 1000 / kFps;

	fPhysicsWorld->Initialize( GetFrameInterval() );

	fDisplay->SetDelegate( this );
	fDisplay->Start();

	// PlatformTimer
	fTimer->SetInterval( kInterval );

	// Initial call to main.lua can cause runtime to be suspended,
	// e.g. via PlatformModalInteraction
	// Otherwise, it's < 0 (i.e. uninitialized)
	if ( fIsSuspended < 0 )
	{
		fTimer->Start();
		fIsSuspended = 0;
	}

	// Update screen until timer fires to yield better launch time.
	// This may cause problems if the OpenGL context is not ready.

	//Platforms like Mac need to defer the initial update
 	if ( ! IsProperty( kDeferUpdate ) )
	{
		(*this)();
	}
}

void
Runtime::OnSystemEvent( SystemEvent::Type t )
{
	// Do not dispatch a system event if the Corona application has not been loaded yet.
	// This can happen if the destructor was called before LoadApplication() or if the application failed to load.
	if (IsProperty(kIsApplicationLoaded) == false)
	{
		return;
	}

	SystemEvent e( t );
	DispatchEvent( e );
}

void
Runtime::OnInternalSystemEvent( SystemEvent::Type t )
{
	InternalSystemEvent e( t );
	DispatchEvent( e );
}
	
	
void
Runtime::CoronaInvokeSystemSuspendEvent()
{
	// If Corona (core) has already been suspended, don't trigger a callback.
	// This only looks at the Corona Core suspend flag.
	// This might need to be adjusted to deal with the possibility that only some subsystems may be suspended at a time.
	if ( 0 == fIsSuspended )
	{
		// Invoke the user system event
		OnSystemEvent( SystemEvent::kOnAppSuspend );
		// Always do our suspend after the user so we don't get in the user's way, 
		// e.g. shut down systems so their calls fail,
		// e.g. suspend video objects which they may want to handle different (destroy objects)
		OnInternalSystemEvent( SystemEvent::kOnAppSuspend );
	}
}

void
Runtime::CoronaInvokeSystemResumeEvent()
{
	// Assumption: Corona has already been resumed at this point. This means the flag check can't look for being suspended.
	Rtt_ASSERT( fIsSuspended <= 0 );
	if ( 0 == fIsSuspended )
	{
		// Always do our resume first so the user is oblvious that we were actually doing things.
		OnInternalSystemEvent( SystemEvent::kOnAppResume );
		// Invoke the user system event
		OnSystemEvent( SystemEvent::kOnAppResume );
	}
}
	
void
Runtime::CoronaAllSuspend()
{
	CoronaCoreSuspend();
#ifdef Rtt_USE_ALMIXER
	PlatformOpenALPlayer* player = PlatformOpenALPlayer::SharedInstance();
	if( NULL != player)
	{
		player->SuspendPlayer();
	}
#endif
#ifdef Rtt_USE_WEBMIXER
	PlatformWebAudioPlayer* player = getSDLAudio();
	player->SuspendPlayer();
#endif
#ifdef Rtt_USE_OPENSLES
	getOpenSLES_player()->SuspendPlayer();
#endif

	// Suspend platform specific items (like video players)
	fPlatform.Suspend();
}
	
void
Runtime::CoronaCoreSuspend()
{
	if ( 0 == fIsSuspended )
	{
		fTimer->Stop();
	}
	if ( fIsSuspended <= 0 )
	{
		fIsSuspended = 1;
		fSuspendTime = Rtt_GetAbsoluteTime();
	}
}

void
Runtime::Suspend(bool sendApplicationEvents /* = true */)
{
	// Determine if the runtime is currently running.
	// Note: -1 means not started. 0 means running. 1 means suspended.
	bool wasRunning = (0 == fIsSuspended);

	// Notify the runtime's owner that the runtime is about to be suspended.
	if ( wasRunning && fDelegate )
	{
		fDelegate->WillSuspend( *this );
	}

	// Only invoke the system suspend event if required (the Simulator sets this to false when suspending before a build)
	if (sendApplicationEvents)
	{
		// We need to make sure this event is called before any of the subsystems have been suspended.
		// We may need to track additional information about which specific subsystems were suspended and pass that as additional data to the event.
		CoronaInvokeSystemSuspendEvent();
	}

	// Easy case: 
	if ( kSuspendAll == fSuspendOverrideProperties )
	{
		CoronaAllSuspend();
	}
	else
	{
		bool coronaCoreShouldSuspend = true;

#ifdef Rtt_USE_ALMIXER
		if ( IsSuspendOverrideProperty( kBackgroundAudio ) )
		{
			// TODO: We need a better story about what to do about Corona event callbacks during background.
			// For initial testing, switch to a background timer if available which currently has lower CPU utilization.
#ifdef Rtt_USE_BACKGROUND_TIMER_EXPERIMENT
			fTimer->SwitchToBackgroundTimer();
			coronaCoreShouldSuspend = false;
#endif
		}
		else
		{
			PlatformOpenALPlayer::SharedInstance()->SuspendPlayer();
		}
#endif
#ifdef Rtt_USE_WEBMIXER
		if ( IsSuspendOverrideProperty( kBackgroundAudio ) )
		{
			// TODO: We need a better story about what to do about Corona event callbacks during background.
			// For initial testing, switch to a background timer if available which currently has lower CPU utilization.
		}
		else
		{
			PlatformWebAudioPlayer* player = getSDLAudio();
			player->SuspendPlayer();
		}
#endif


		// TODO: Right now this breaks the lua audio callback system too.
		// So selectively suspend so we can do further testing/experiments for now.
		if ( coronaCoreShouldSuspend )
		{
			CoronaCoreSuspend();
		}
	}

	// Notify the runtime's owner that the runtime has just been suspended.
	if ( wasRunning && fDelegate && (fIsSuspended > 0) )
	{
		fDelegate->DidSuspend( *this );
	}
}

void
Runtime::CoronaCoreResume()
{
	Rtt_ASSERT( 0 != fIsSuspended );
	if ( 1 == fIsSuspended )
	{

		Rtt_AbsoluteTime currentTime = Rtt_GetAbsoluteTime();
		Rtt_AbsoluteTime suspendTime = fSuspendTime; Rtt_ASSERT( suspendTime > 0 );
		if ( currentTime > suspendTime )
		{
			fStartTimeCorrection += (currentTime - suspendTime);
		}
		fSuspendTime = 0;

#ifdef Rtt_USE_BACKGROUND_TIMER_EXPERIMENT
		fTimer->SwitchToForegroundTimer();
#endif

		fTimer->Start();

		fIsSuspended = 0;
	}
	else
	{
#ifdef Rtt_USE_BACKGROUND_TIMER_EXPERIMENT
		fTimer->SwitchToForegroundTimer();
#endif
	}
}

void
Runtime::CoronaResumeAll()
{
	// Determine if the runtime is currently suspended.
	// Note: -1 means not started. 0 means running. 1 means suspended.
	bool wasSuspended = (fIsSuspended > 0);

	// Notify the runtime's owner that the runtime is about to be resumed.
	if ( wasSuspended && fDelegate )
	{
		fDelegate->WillResume( *this );
	}

	// Resume the runtime, if currently suspended.
	CoronaCoreResume();

#ifdef Rtt_USE_ALMIXER
	// As an optimization, we may have just shutdown the audio system if it wasn't needed. Resume it to be safe.
	if( PlatformOpenALPlayer::SharedInstance()->IsPlayerSuspended() )
	{
		PlatformOpenALPlayer::SharedInstance()->ResumePlayer();
	}
	// If iOS entered an interruption state and backgrounding is enabled (think phone call), it could be possible that
	// resuming the app avoids the internal AudioSession callback to end the interruption.
	// So we should try to recover from an interruption event.
	// This should be done after the suspended check because the current implementation has suspend call interruption
	// so we want to avoid ending an interruption without resuming the player.
	else if( PlatformOpenALPlayer::SharedInstance()->IsInInterruption() )
	{
		PlatformOpenALPlayer::SharedInstance()->EndInterruption();
	}
#endif
#ifdef Rtt_USE_WEBMIXER
	PlatformWebAudioPlayer* player = getSDLAudio();
	player->ResumePlayer();
#endif
#ifdef Rtt_USE_OPENSLES
	getOpenSLES_player()->ResumePlayer();
#endif

	// Resume platform specific items (like video players)
	fPlatform.Resume();

	// Notify the runtime's owner that the runtime has just been resumed.
	if ( wasSuspended && fDelegate && !fIsSuspended )
	{
		fDelegate->DidResume( *this );
	}
}
	
void
Runtime::Resume(bool sendApplicationEvents /* = true */)
{
	CoronaResumeAll();
	// reset for the next time
	fSuspendOverrideProperties = kSuspendAll;

	// Only invoke the system resume event if required (the Simulator sets this to false when resuming after a build)
	if (sendApplicationEvents)
	{
		// We need to make sure this event is called only after all the subsystems have been resumed.
		// We may need to track additional information about which specific subsystems were resumed and pass that as additional data to the event.
		CoronaInvokeSystemResumeEvent();
	}
}

void
Runtime::DispatchEvent( const MEvent& e )
{
	RuntimeGuard guard( * this );

	e.Dispatch( fVMContext->L(), * this );
}

double
Runtime::GetElapsedMS() const
{
	Rtt_AbsoluteTime elapsed = GetElapsedTime();
	return ((double)Rtt_AbsoluteToMicroseconds( elapsed )) / 1000.;
}

Rtt_AbsoluteTime
Runtime::GetElapsedTime() const
{
	// During a suspend, use fSuspendTime as current time; otherwise, fetch absolute time
	Rtt_AbsoluteTime currentTime = ( 1 != fIsSuspended ? Rtt_GetAbsoluteTime() : fSuspendTime );
	Rtt_AbsoluteTime elapsed = currentTime - fStartTime;

	// Subtract out correction, but protect against subtracting past 0
	if ( elapsed > fStartTimeCorrection )
	{
		elapsed -= fStartTimeCorrection;
	}

	return elapsed;
}

void
Runtime::Collect()
{
	LuaContext& vm = VMContext();
	
	// Lua
	vm.Collect();

	// Display objects
	GroupObject::CollectUnreachables( vm.L(), GetDisplay().GetScene(), * GetDisplay().Orphanage() );
}

void
Runtime::SetProperty( U16 mask, bool value )
{
	const U16 p = fProperties;
	fProperties = ( value ? p | mask : p & ~mask );
}

void
Runtime::SetSuspendOverrideProperty( U32 mask, bool value )
{
	const U32 p = fSuspendOverrideProperties;
	fSuspendOverrideProperties = ( value ? p | mask : p & ~mask );
}

void
Runtime::SetShowingTrialMessage( bool showMsg ) const
{
	fShowingTrialMessages = showMsg;
}

bool
Runtime::IsShowingTrialMessage()
{
	return fShowingTrialMessages;
}

PlatformExitCallback*
Runtime::GetExitCallback()
{
	MPlatform* platform = const_cast<MPlatform*>(&fPlatform);
	return platform->GetExitCallback();
}

void 
Runtime::UnloadResources()
{
#if defined( Rtt_ANDROID_ENV ) && defined( Rtt_DEBUG )
	Rtt_TRACE( ( "> ReleaseTextures" ) );
#endif

	fDisplay->UnloadResources();

	// Iterate through all resources
	lua_State *L = PushResourceRegistry(); Rtt_ASSERT( L );
	int t = lua_gettop( L ); // Need stable index for table

	lua_pushnil( L );
	while ( lua_next( L, t ) )
	{
		CachedResource* resource = (CachedResource*)lua_touserdata( L, -1 ); Rtt_ASSERT( ! resource || lua_type( L, -1 ) == LUA_TLIGHTUSERDATA );

#ifdef OLD_GRAPHICS
		resource->Unload();
#endif
		lua_pop( L, 1 );
	}

#if defined( Rtt_ANDROID_ENV ) && defined( Rtt_DEBUG )
	Rtt_TRACE( ( "< ReleaseTextures" ) );
#endif
}

void 
Runtime::ReloadResources()
{
#if defined( Rtt_ANDROID_ENV ) && defined( Rtt_DEBUG )
	Rtt_TRACE( ( "> ReloadTextures" ) );
#endif

	fDisplay->ReloadResources();

	// Iterate through all resources
	lua_State *L = PushResourceRegistry(); Rtt_ASSERT( L );
	int t = lua_gettop( L ); // Need stable index for table

	lua_pushnil( L );
	while ( lua_next( L, t ) )
	{
		CachedResource* resource = (CachedResource*)lua_touserdata( L, -1 ); Rtt_ASSERT( ! resource || lua_type( L, -1 ) == LUA_TLIGHTUSERDATA );
#ifdef OLD_GRAPHICS
		resource->Reload();
#endif
		lua_pop( L, 1 );
	}

	// Force rebuild of entire display
	fDisplay->GetStage()->Invalidate( DisplayObject::kRenderDirty );
	
#if defined( Rtt_ANDROID_ENV ) && defined( Rtt_DEBUG )
	Rtt_TRACE( ( "< ReloadTextures" ) );
#endif
}

// TODO: Remove this wrapper
void
Runtime::RestartRenderer()
{
	fDisplay->Restart();
}

void
Runtime::RestartRenderer( DeviceOrientation::Type orientation )
{
	fDisplay->Restart( orientation );
}

CachedResource*
Runtime::LookupResource( const char key[] )
{
// Null key is valid
//	Rtt_ASSERT( key );
	if ( !key )
		return NULL;

	lua_State *L = PushResourceRegistry(); Rtt_ASSERT( L );
	lua_getfield( L, -1, key );
	CachedResource* result = (CachedResource*)lua_touserdata( L, -1 ); Rtt_ASSERT( ! result || lua_type( L, -1 ) == LUA_TLIGHTUSERDATA );
	lua_pop( L, 2 );

	return result;
}

void
Runtime::SetResource( CachedResource* resource, const char key[] )
{
	lua_State *L = PushResourceRegistry();

#if defined( Rtt_ANDROID_ENV ) && defined( Rtt_DEBUG )
	Rtt_TRACE( ( "> Runtime::SetResource" ) );
#endif

	// Library is only valid when the Lua state exists. No-op for invalid lib.
	if ( L )
	{
		Rtt_ASSERT( key );

		if ( resource )
		{
			// Add
			lua_pushlightuserdata( L, resource );

#if defined( Rtt_ANDROID_ENV ) && defined( Rtt_DEBUG )
	Rtt_TRACE( ( "+ Runtime::Set" ) );
#endif
			CachedResource* r = resource;
			fResourcesHead->Prepend( r ); // Put at end of list (MRU)
		}
		else
		{
			CachedResource* r = LookupResource( key );
			if ( r )
			{
#if defined( Rtt_ANDROID_ENV ) && defined( Rtt_DEBUG )
	Rtt_TRACE( ( "- Runtime::Set" ) );
#endif
				r->Remove();
			}
		
			// Remove
			lua_pushnil( L );
		}

		lua_setfield( L, -2, key );
		lua_pop( L, 1 );
	}
#if defined( Rtt_ANDROID_ENV ) && defined( Rtt_DEBUG )
	Rtt_TRACE( ( "< Runtime::SetResource" ) );
#endif
}

void
Runtime::MarkRecentlyUsed( CachedResource& resource )
{
	// Move to end of list (MRU).
	fResourcesHead->Prepend( & resource );
}

Rtt_Allocator*
Runtime::GetAllocator() const
{
	return Allocator();
}

// TODO: Remove this wrapper
void
Runtime::GetImageSuffix( String& outSuffix )
{
	fDisplay->GetImageSuffix( outSuffix );
}

// TODO: Remove this wrapper
bool
Runtime::GetImageFilename( const char *filename, MPlatform::Directory baseDir, String& outFilename )
{
	return fDisplay->GetImageFilename( filename, baseDir, outFilename );
}

lua_State*
Runtime::PushResourceRegistry()
{
	lua_State *L = fVMContext ? fVMContext->L() : NULL;

	if ( L )
	{
		lua_pushlightuserdata( L, this );
		lua_gettable( L, LUA_REGISTRYINDEX );

		// Create resource registry
		if ( lua_isnil( L, -1 ) )
		{
			lua_pop( L, 1 ); // pop nil

			lua_newtable( L ); // t

			lua_pushlightuserdata( L, this );
			lua_pushvalue( L, -2 );		
			lua_settable( L, LUA_REGISTRYINDEX ); // LUA_REGISTRY[this] = t
		}
	}

	return L;
}
/*
void
Runtime::BeginOrientationListener()
{
	fPlatform.GetDevice().SetOrientationCallback( & fListener );
}

void
Runtime::EndOrientationListener()
{
	fPlatform.GetDevice().SetOrientationCallback( NULL );
}
*/
void
Runtime::operator()()
{
	RuntimeGuard guard( * this );

	if ( ! Rtt_VERIFY( fDisplay ) )
	{
		return;
	}

	const bool wasSuspended = IsSuspended();
	fScheduler->Run();
	const bool isSuspended = IsSuspended();
	if( wasSuspended != isSuspended && isSuspended )
	{
		// This condition is written inverse for better understanding
		// Sometimes (Splash Screen is shown) scheduled tasks can suspend Runtime
		// In that case (suspension state is changed and it is suspended), skip Display update
	}
	else
	{
#if defined(Rtt_AUTHORING_SIMULATOR)
		if (m_fAsyncResultStr.load()) {
			FinalizeWorkingThreadWithEvent(this, fVMContext->L());
		}
#endif
		fDisplay->Update();

		++fFrame;
	}

	if ( ! IsProperty( kRenderAsync ) )
	{
		fDisplay->Render();
	}
	
}
void
Runtime::Render()
{
	RuntimeGuard guard( * this );

	if ( ! Rtt_VERIFY( fDisplay ) )
	{
		return;
	}

	fDisplay->Render();
}

void
Runtime::Blit()
{
	Rtt_ASSERT( fDisplay );

	fDisplay->Blit();
}

void
Runtime::Begin() const
{
	fPlatform.BeginRuntime( * this );
}

void
Runtime::End() const
{
	fPlatform.EndRuntime( * this );
}

void
Runtime::WillDispatchFrameEvent( const Display& sender )
{
	fPhysicsWorld->StepWorld( GetElapsedMS() );
}

// ----------------------------------------------------------------------------

RuntimeGuard::RuntimeGuard( Runtime& runtime )
:	fRuntime( runtime ),
	fPlatform( runtime.Platform() )
{
	fPlatform.BeginRuntime( runtime );
}

RuntimeGuard::~RuntimeGuard()
{
	// Collect orphaned display objects
	fRuntime.GetDisplay().Collect( fRuntime.VMContext().L() );

	fPlatform.EndRuntime( fRuntime );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

