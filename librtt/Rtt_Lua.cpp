//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


// 
// Design assumptions:
//		These functions should work with any Lua state instance
//

#include "Core/Rtt_Build.h"

#include "Rtt_Lua.h"
#if !defined( Rtt_NO_GUI )
#include "Rtt_LuaContext.h"
#endif
#include "Rtt_MCriticalSection.h"
#include "Core/Rtt_String.h"
#include "Corona/CoronaLog.h"

#include <signal.h>
#include <string.h>

// ----------------------------------------------------------------------------

#include "Rtt_LuaFrameworks.h"

extern "C"
{
	extern int CoronaLuaLoad_re (lua_State *L);

	// We need to load the "re" framework in a C context (rather than CPP) so we
    // provide this support function to do so (there may be a better place for it
    // to live)
    int CoronaLuaLoad_re(lua_State *L)
    {
        lua_getfield( L, LUA_GLOBALSINDEX, "package" );
        lua_getfield( L, -1, "preload" );
        lua_pushcfunction( L, Rtt::Lua::Open< Rtt::luaload_re > );
        lua_setfield( L, -2, "re" );
        
        lua_pop( L, 2 );

        return 0;
    }
}

namespace Rtt
{

// ----------------------------------------------------------------------------

// ----------------------------------------------------------------------------

struct LuaReference
{
	int owner; // Table that owns fRef
	int index; // unique integer key into fOwner
};

// ----------------------------------------------------------------------------

static const char kNameKey[] = "name";

// ----------------------------------------------------------------------------

// Unique keys
//
// NOTE: Using a trick that two string literals next to each other automatically
// become one single string literal via the C preprocessor. Also note __FILE__
// evaluates to a string literal.
static const char kCoronaCriticalSectionKey[] = "_corona.criticalSection." __FILE__;
static const char kCoronaThreadKey[] = "_corona.thread." __FILE__;

// ----------------------------------------------------------------------------

void
Lua::Initialize( lua_State *coronaThread )
{
	lua_State *L = coronaThread;
	Rtt_LUA_STACK_GUARD( L );

	// Store 'coronaThread' as light userdata
	lua_pushstring( L, kCoronaThreadKey ); // push key
	lua_pushlightuserdata( L, coronaThread ); // push value (top of stack)
	lua_settable( L, LUA_REGISTRYINDEX );
}

lua_State*
Lua::GetCoronaThread( lua_State *coroutine )
{
	lua_State *L = coroutine;
	Rtt_LUA_STACK_GUARD( L );

	lua_getfield( L, LUA_REGISTRYINDEX, kCoronaThreadKey ); // push Corona's L
	lua_State *result = (lua_State *)lua_touserdata( L, -1 );
	lua_pop( L, 1 ); // pop Corona's L
	return result;
}

lua_State*
Lua::New( bool shouldOpenLibs )
{
	lua_State *L = luaL_newstate();

	if ( shouldOpenLibs )
	{
		luaL_openlibs( L );
	}
	return L;
}

void
Lua::Delete( lua_State *L )
{
	lua_close( L );
}


void
Lua::SetCriticalSection( lua_State *L, const MCriticalSection *criticalSection )
{
	// NOTE: Lua does not own the criticalSection.
	lua_pushlightuserdata( L, const_cast< MCriticalSection * >( criticalSection ) );
	lua_setfield( L, LUA_REGISTRYINDEX, kCoronaCriticalSectionKey );
}

int
Lua::Normalize( lua_State *L, int index )
{
	int result = index;

	// Map negative indices to positive ones
	if ( index < 0 )
	{
		result = lua_gettop( L ) + index + 1;
	}

	return result;
}

static const char kContextKey[] = "CoronaGlobalContext";

void
Lua::InitializeContext( lua_State *L, void *context, const char *metatableName )
{
	// Only set if non-nil
	if ( context )
	{
		if ( metatableName )
		{
			PushUserdata( L, context, metatableName );
		}
		else
		{
			lua_pushlightuserdata( L, context );
		}
		lua_setfield( L, LUA_REGISTRYINDEX, kContextKey );
	}
}

void *
Lua::GetContext( lua_State *L )
{
	Rtt_LUA_STACK_GUARD( L );

	void *result = NULL;

	lua_getfield( L, LUA_REGISTRYINDEX, kContextKey );
	if ( lua_islightuserdata( L, -1 ) )
	{
		result = lua_touserdata( L, -1 );
	}
	else
	{
		result = ToUserdata( L, -1 );
	}
	lua_pop( L, 1 );

	return result;
}

Lua::Ref
Lua::NewRef( lua_State *L, int index )
{
	int owner = LUA_REGISTRYINDEX;

	LuaReference *result = (LuaReference *)malloc( sizeof( LuaReference ) );
	result->owner = owner;

	lua_pushvalue( L, index );
	result->index = luaL_ref( L, owner );

	return result;
}

void
Lua::DeleteRef( lua_State *L, Ref ref )
{
	if ( ref )
	{
		LuaReference *reference = (LuaReference *)ref;

		luaL_unref( L, reference->owner, reference->index );
		free( reference );
	}
}

bool
Lua::EqualRef( lua_State *L, Ref ref, int index )
{
	bool result = false;

	if ( ref )
	{
		LuaReference *reference = (LuaReference *)ref;

		index = Normalize( L, index );
		lua_rawgeti( L, reference->owner, reference->index ); // Push ref

		result = ( 0 != lua_equal( L, -1, index ) );

		lua_pop( L, 1 ); // Pop ref
	}

	return result;
}

void
Lua::NewEvent( lua_State *L, const char *eventName )
{
	Rtt_ASSERT( NULL != eventName );
	Rtt_ASSERT( NULL != L );

	if (L != NULL && eventName != NULL)
	{
		lua_newtable( L );
		lua_pushstring( L, eventName );
		lua_setfield( L, -2, kNameKey );
	}
	else
	{
		CORONA_LOG_ERROR("Lua::NewEvent: missing parameter (L: %p, eventName: %s)",
				L, (eventName == NULL ? "(null)" : eventName));
	}
}

void
Lua::DispatchEvent( lua_State *L, Ref listenerRef, int nresults )
{
	if ( ! listenerRef )
	{
		return;
	}

	int eventIndex = lua_gettop( L ); // event is at top of stack

	lua_getfield( L, eventIndex, kNameKey ); // push event.name

	if ( Rtt_VERIFY( LUA_TSTRING == lua_type( L, -1 ) ) )
	{
		lua_getfield( L, LUA_REGISTRYINDEX, kCoronaCriticalSectionKey );
		MCriticalSection *criticalSection = (MCriticalSection *)lua_touserdata( L, -1 );
		lua_pop( L, 1 );

		CriticalSectionGuard guard( criticalSection );
		{
			const char *eventName = lua_tostring( L, -1 );

			LuaReference *reference = (LuaReference *)listenerRef;

			lua_rawgeti( L, reference->owner, reference->index ); // 1: reference

			if ( lua_isfunction( L, -1 ) )
			{
				// Corresponds to Lua code: listener( event )
			
				// 1: reference is the function
				lua_pushvalue( L, eventIndex ); // 2: event
				DoCall( L, 1, nresults );
			}
			else if ( lua_istable( L, -1 ) )
			{
				// Corresponds to Lua code: listener.eventName( listener, event )
			
				// reference is a table, so look for table
				lua_getfield( L, -1, eventName ); // listener.eventName
				if ( lua_isfunction( L, -1 ) )
				{
					// swap stack positions for reference and function
					// 1: function (listener.eventName)
					// 2: reference, i.e. table (implicit self arg to function)
					lua_insert( L, -2 );

					lua_pushvalue( L, eventIndex ); // 3: event
					DoCall( L, 2, nresults );
				}
				else
				{
					luaL_error( L, "[Lua::DispatchEvent()] ERROR: Table listener's property '%s' is not a function.\n", eventName );
				}
			}
			else
			{
				luaL_error( L, "[Lua::DispatchEvent()] ERROR: Listener must be a function or a table, not a '%s'.\n", lua_typename( L, lua_type( L, -1 ) ) );
			}
		}
	}
	else
	{
		luaL_error( L, "[Lua::DispatchEvent()] ERROR: Attempt to dispatch malformed event. The event must have a 'name' string property.\n" );
	}
	lua_pop( L, 2 ); // pop event.name and event
}

void
Lua::DispatchRuntimeEvent( lua_State *L, int nresults )
{
	nresults = Max( 0, nresults ); // Ensure nresults >= 0

	int eventIndex = lua_gettop( L ); // event is at top of stack

	bool isError = true;

	lua_getfield( L, eventIndex, kNameKey ); // push event.name
	bool isEvent = Rtt_VERIFY( LUA_TSTRING == lua_type( L, -1 ) );
	lua_pop( L, 1 );

	if ( isEvent )
	{
		lua_getfield( L, LUA_REGISTRYINDEX, kCoronaCriticalSectionKey );
		MCriticalSection *criticalSection = (MCriticalSection *)lua_touserdata( L, -1 );
		lua_pop( L, 1 );

		CriticalSectionGuard guard( criticalSection );
		{
			PushRuntime( L ); Rtt_ASSERT( lua_istable( L, -1 ) );
			
			if ( ! lua_istable( L, -1 ) )
			{
				// This can happen in simulatorAnalytics.lua
				Rtt_LogException( "[Lua::DispatchRuntimeEvent()] ERROR: 'Runtime' is not valid.\n");
				lua_pop( L, 2 ); // pop Runtime and event
				goto exit_gracefully;
			}
			
			lua_getfield( L, -1, "dispatchEvent" ); Rtt_ASSERT( lua_isfunction( L, -1 ) );

			// Reorder so dispatchEvent is below Runtime. And Runtime is below event.
			lua_insert( L, eventIndex ); // move dispatchEvent
			lua_insert( L, eventIndex + 1 ); // move Runtime

			// local dispatchEvent = Runtime.dispatchEvent
			// dispatchEvent( Runtime, event )
			isError = (DoCall(L, 2, nresults) != 0);
		}
	}
	else
	{
		lua_pop( L, 1 ); // pop event

		luaL_error( L, "[Lua::DispatchRuntimeEvent()] ERROR: Attempt to dispatch malformed event. The event must have a 'name' string property.\n" );
	}

exit_gracefully:
	if ( isError )
	{
		// push 'nil' for each result (nresults)
		for ( int i = 0; i < nresults; i++ ) { lua_pushnil( L ); }
	}

	// Check stack: eventIndex was old top, but it was popped by DoCall, so subtract 1
	// and then add nresults
	Rtt_ASSERT( ((eventIndex - 1) + nresults) == lua_gettop( L ) );
}

bool
Lua::IsListener( lua_State *L, int index, const char *eventName )
{
	bool result = lua_isfunction( L, index );
	if ( ! result && lua_istable( L, index ) )
	{
		lua_getfield( L, index, eventName );
		result = lua_isfunction( L, -1 );
		lua_pop( L, 1 );
	}
	return result;
}

void
Lua::PushRuntime( lua_State *L )
{
	lua_getglobal( L, "Runtime" );
}

// Helper method for Lua::PushCachedFunction
static void
Lua_CacheClosure( lua_State *L, lua_CFunction function, int n, bool leaveOnTop )
{
	int top = lua_gettop( L ); // record old top

	lua_pushlightuserdata( L, (void*)function );
	lua_pushcclosure( L, function, n );
	if ( leaveOnTop )
	{
		lua_pushvalue( L, -1 );
		lua_insert( L, ++top );
	}
	lua_settable( L, LUA_REGISTRYINDEX );

	Rtt_ASSERT( lua_gettop( L ) == top );
}

// Helper method for Lua::PushCachedFunction
static void
Lua_PushFunction( lua_State *L, lua_CFunction function )
{
	lua_pushlightuserdata( L, (void*)function );
	lua_gettable( L, LUA_REGISTRYINDEX );
}

// lua_pushcfunction and lua_pushcclosure are very expensive
// Roberto (yes, that Roberto) recommends caching all C functions
// in the global registry using the function ptr value as the key.
void
Lua::PushCachedFunction( lua_State *L, lua_CFunction function )
{
#ifdef Rtt_DEBUG
	int top = lua_gettop( L ); // record old top
#endif

	// Cache hit. The function is already in the global registry
	Lua_PushFunction( L, function );

	// Cache miss. Add function to cache
	if ( ! lua_isfunction( L, -1 ) )
	{
		Rtt_ASSERT( lua_isnil( L, -1 ) );
		lua_pop( L, 1 );

		Lua_CacheClosure( L, function, 0, true );
	}

	Rtt_ASSERT( lua_gettop( L ) == ( top + 1 ) );
	Rtt_ASSERT( lua_isfunction( L, -1 ) );
}

void
Lua::NewGCMetatable( lua_State* L, const char name[], lua_CFunction __gc1 )
{
	const luaL_Reg kVTable[] =
	{
		{ "__gc", __gc1 },		
		{ NULL, NULL }
	};

	Self::NewMetatable( L, name, kVTable );
}

void
Lua::NewMetatable( lua_State* L, const char name[], const luaL_Reg vtable[] )
{
	luaL_newmetatable( L, name );
	luaL_register( L, NULL, vtable );
	}

void
Lua::InitializeGCMetatable( lua_State* L, const char name[], lua_CFunction __gc1 )
{
	Self::NewGCMetatable( L, name, __gc1 );
	lua_pop( L, 1 );
}

void
Lua::InitializeMetatable( lua_State* L, const char name[], const luaL_Reg vtable[] )
{
	Self::NewMetatable( L, name, vtable );
	lua_pop( L, 1 );
}

void
Lua::PushUserdata( lua_State* L, void* ud, const char metatableName[] )
{
	void** p = (void**)lua_newuserdata( L, sizeof( void* ) );
	*p = ud;

	luaL_getmetatable( L, metatableName ); Rtt_ASSERT( lua_istable( L, -1 ) );
	lua_setmetatable( L, -2 );
}

void*
Lua::ToUserdata( lua_State* L, int index )
{
	void *result = NULL;

	void **p = (void**)lua_touserdata( L, index );
	if ( p )
	{
		result = * p;
	}
	
	return result;
}

// IMPORTANT: This code was copy-pasted from luaL_checkudata(),
// minus the call to luaL_typerror().
void*
Lua::ToUserdata( lua_State* L, int ud, const char *tname )
{
  void *p = lua_touserdata(L, ud);
  if (p != NULL) {  /* value is a userdata? */
    if (lua_getmetatable(L, ud)) {  /* does it have a metatable? */
      lua_getfield(L, LUA_REGISTRYINDEX, tname);  /* get correct metatable */
      if (lua_rawequal(L, -1, -2)) {  /* does it have the correct mt? */
        lua_pop(L, 2);  /* remove both metatables */
        return p;
      }
    }
  }
  return NULL;  /* to avoid warnings */
}

void*
Lua::CheckUserdata( lua_State* L, int index, const char metatableName[] )
{
	void *result = NULL;

	void **p = (void**)luaL_checkudata( L, index, metatableName );
	if ( p )
	{
		result = *p;
	}

	return result;
}

void
Lua::RegisterModuleLoader( lua_State *L, const char *name, lua_CFunction loader, int nupvalues )
{
	const luaL_Reg moduleLoaders[] =
	{
		{ name, loader },
		{ NULL, NULL }
	};

	RegisterModuleLoaders( L, moduleLoaders, nupvalues );
}

void
Lua::RegisterModuleLoaders( lua_State *L, const luaL_Reg moduleLoaders[], int nupvalues )
{
	// Preload modules to eliminate dependency
	lua_getfield( L, LUA_GLOBALSINDEX, "package" );
	lua_getfield( L, -1, "preload" );
	for ( const luaL_Reg *lib = moduleLoaders; lib->func; lib++ )
	{
		for ( int i = 0; i < nupvalues; i++ )
		{
			lua_pushvalue( L, -( 2 + nupvalues ) );
		}
		lua_pushcclosure( L, lib->func, nupvalues );
		lua_setfield( L, -2, lib->name );
	}

	lua_pop( L, 2 );
	lua_pop( L, nupvalues );
}

// ----------------------------------------------------------------------------

static int
LuaTraceback( lua_State* L )
{
	int result = 1;

#if !defined( Rtt_NO_GUI )
	// Call the LuaContext version to ensure the same behavior in all circumstances
	LuaContext::traceback( L );
#else
	// In CoronaBuilder we don't care about unhandledError listeners so do it the old way

	if (!lua_isstring(L, 1))  /* 'message' not a string? */
	{
		return 1;  /* keep it intact */
	}

	lua_getfield(L, LUA_GLOBALSINDEX, "debug");
	if (!lua_istable(L, -1))
	{
		lua_pop(L, 1);
		return 1;
	}
	lua_getfield(L, -1, "traceback");
	if (!lua_isfunction(L, -1))
	{
		lua_pop(L, 2);
		return 1;
	}

	lua_remove( L, -2 ); // pop debug

	lua_pushvalue(L, 1);  /* pass error message */
	lua_pushinteger(L, 1);  /* skip this function and traceback */
	lua_call(L, 2, 1);  /* call debug.traceback */

	if ( ! lua_gethook( L ) )
	{
		CORONA_LOG_ERROR( "Runtime error\n%s", lua_tostring( L, -1 ) );

		// Pop message as we have already printed it to console.
		lua_pop( L, 1 );
		result = 0;
	}
#endif

	return result;
}

static void
LuaStop( lua_State* L, lua_Debug *ar )
{
	(void)ar;  /* unused arg. */
	lua_sethook(L, NULL, 0, 0);
	luaL_error(L, "interrupted!");
}

static lua_State *sLuaContext = NULL;

static void
LuaAction( int i )
{
	// if another SIGINT happens before lstop, terminate process (default action)
#if !defined(EMSCRIPTEN)
	signal( i, SIG_DFL );
#endif
	
	lua_sethook( sLuaContext, LuaStop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1 );
}

#ifdef NOT_USED
static void
LuaMessage( const char *pname, const char *msg )
{
	if (pname)
	{
		printf( "%s: ", pname );
	}

	printf( "%s\n", msg );
}
#endif // NOT_USED

static int
LuaReport( lua_State* L, int status )
{
	static const char kLuaRuntimeError[] = "Runtime error";
	static const char kLuaSyntaxError[] = "Syntax error";
	static const char kLuaMemoryError[] = "Out of Memory error";
	static const char kLuaGenericError[] = "Generic error";

	if (status && !lua_isnil(L, -1))
	{
		const char *msg = lua_tostring(L, -1);
		if (msg == NULL)
		{
			msg = "(error object is not a string)";
		}

		const char *errName = kLuaGenericError;
		switch( status )
		{
			case LUA_ERRRUN:
				errName = kLuaRuntimeError;
				break;
			case LUA_ERRSYNTAX:
				errName = kLuaSyntaxError;
				break;
			case LUA_ERRMEM:
				errName = kLuaMemoryError;
				break;
		}

		if (status != LUA_ERRRUN)
		{
			// Runtime errors have already been reported at this point
			CORONA_LOG_ERROR("%s\n%s", errName, msg);
		}

		lua_pop(L, 1);
	}
	return status;
}

// ----------------------------------------------------------------------------

int
Lua::OpenModule( lua_State *L, lua_CFunction loader )
{
	int arg = lua_gettop(L);
	(*loader)( L ); Rtt_ASSERT( lua_isfunction( L, -1 ) );
	lua_insert(L,1);
	lua_call(L,arg,1);
	return 1;
}

// ----------------------------------------------------------------------------

void
Lua::InsertPackageLoader( lua_State *L, lua_CFunction loader, int index, void *loaderData )
{
	Rtt_LUA_STACK_GUARD( L );

	lua_getfield( L, LUA_GLOBALSINDEX, "table" );
	lua_getfield( L, -1, "insert" ); // push function to be called

	lua_getfield( L, LUA_GLOBALSINDEX, "package" );
	lua_getfield( L, -1, "loaders" ); // push 1st arg: "package.loaders" table
	lua_remove( L, -2 ); // pop "package"

	int nargs = 2;
	if ( index > 0 )
	{
		++nargs;
		lua_pushinteger( L, index ); // push optional arg
	}

	int numData = 0;
	if ( loaderData )
	{
		lua_pushlightuserdata( L, loaderData );
		numData++;
	}

	lua_pushcclosure( L, loader, numData );

	lua_call( L, nargs, 0 ); // call table.insert( package.loaders, [index,] loader )
	lua_pop( L, 1 ); // pop "table"
}

// ----------------------------------------------------------------------------

static lua_CFunction sErrorHandler = NULL;

lua_CFunction
Lua::GetErrorHandler( lua_CFunction defaultHandler )
{
	lua_CFunction result = sErrorHandler;
	if ( ! result )
	{
		// If no error handler, then use optionally-provided default
		result = defaultHandler;
	}
	return result;
}

void
Lua::SetErrorHandler( lua_CFunction newValue )
{
	sErrorHandler = newValue;
}

void
Lua::Warning( lua_State *L, const char *fmt, ... )
{
	va_list argp;
	va_start(argp, fmt);
	
	if (L == NULL )
	{
		// No Lua context, just print the message
		Rtt_LogException(fmt, argp);
		
		return;
	}
	
	lua_pushstring(L, "Warning: ");
	luaL_where(L, 1);
	lua_pushvfstring(L, fmt, argp);
	va_end(argp);
	lua_concat(L, 3);

	Rtt_LogException("%s", lua_tostring(L, -1));

	lua_pop(L, 1);
}
	
int
Lua::DoCall( lua_State* L, int narg, int nresults )
{
	int base = lua_gettop(L) - narg;  /* errfunc index */

	lua_CFunction callback = GetErrorHandler( LuaTraceback ); Rtt_ASSERT( callback );

	lua_pushcfunction( L, callback );  /* push errfunc */
	lua_insert( L, base );  /* put it under chunk and args */
	int errfunc = base;

	sLuaContext = L;

#if !defined(EMSCRIPTEN)
	signal(SIGINT, LuaAction);
#endif

    int status = lua_pcall(L, narg, nresults, errfunc);
	
#if !defined(EMSCRIPTEN)
	signal(SIGINT, SIG_DFL);
#endif
	
	lua_remove(L, errfunc);  /* remove errfunc */

	/* force a complete garbage collection in case of errors */
	if (status != 0)
	{
		// If there is no error function, report the message returned by Lua.
		if ( 0 == errfunc )
		{
			Rtt_LogException("Lua Runtime Error: lua_pcall failed with status: %d, error message: %s\n", status, lua_tostring( L, -1) );
		}
		
		lua_gc(L, LUA_GCCOLLECT, 0);

		// Trigger the unhandledError mechanism
		LuaReport( L, status );
	}

	return status;
}

int
Lua::DoBuffer( lua_State *L, lua_CFunction loader, lua_CFunction pushargs )
{
	Rtt_ASSERT( loader );

	int status = loader( L );
	int narg = ( pushargs ? pushargs( L ) : 0 );

	// Load and invoke Lua bytecode
	if ( Rtt_VERIFY( 0 == status ) )
	{
		status = DoCall( L, narg, 0 ); Rtt_ASSERT( 0 == status );
		LuaReport( L, status );
	}

	return status;
}

int
Lua::DoFile( lua_State *L, const char* file, int narg, bool clear, String *errorMesg /* = NULL */ )
{
	int status = luaL_loadfile( L, file );

	if ( 0 == status )
	{
		int base = lua_gettop( L ) - narg;
		lua_insert( L, base ); // move chunk underneath args

		// If the caller supplied a place to return an error message then make a raw Lua API
		// call to run the code which leaves any error message on the stack, otherwise call
		// a higher level routine which traps errors (the trap removes the error from the stack)
		if ( errorMesg != NULL )
		{
			errorMesg->Set("");
			status = lua_pcall(L, narg, 0, 0);
		}
		else
		{
			status = DoCall( L, narg, ( clear ? 0 : LUA_MULTRET ) );
		}
	}

	if ( errorMesg != NULL && status != 0 && lua_isstring(L, -1) )
	{
		errorMesg->Set( lua_tostring(L, -1) );
	}

	// This pops the error off the stack (if there was one)
	return LuaReport( L, status );
}

// ----------------------------------------------------------------------------

int
Lua::PushModule( lua_State *L, const char *name )
{
	int result = 0;

	if ( name )
	{
		// TODO: Should we cache for performance?
		lua_getglobal( L, "require" );
		lua_pushstring( L, name );
		result = ( 0 == DoCall( L, 1, 1 ) );

		Rtt_ASSERT( 0 == result || lua_istable( L, -1 ) );
	}

	return result;
}

int
Lua::VCall( lua_State *L, const char *sig, va_list ap, const char *methodName )
{
	int nres = 0;

	if ( Rtt_VERIFY( L ) && sig )
	{
		int narg = 0;

		int base = lua_gettop( L );

		// Handle object method
		if ( lua_istable( L, base ) )
		{
			lua_getfield( L, base, methodName ); // method = t.methodName
			if ( lua_isfunction( L, -1 ) )
			{
				lua_insert( L, base ); // insert method below t
				++narg;
			}
			else
			{
				Rtt_LogException( "Could not call object method (%s): No method with that name exists in table at top of stack.", methodName );
				lua_pop( L, 1 );
			}
		}
		else
		{
			// Log errors
			if ( ! Rtt_VERIFY( NULL == methodName ) )
			{
				// If no table, then methodName should be NULL
				Rtt_LogException( "Could not call object method (%s): Table expected at top of stack.", methodName );
			}

			if ( ! lua_isfunction( L, base ) )
			{
				Rtt_LogException( "Function expected at top of stack." );
			}
		}

		if ( lua_isfunction( L, base ) )
		{
			for ( bool hasMoreArgs = true; *sig && hasMoreArgs; )
			{
				luaL_checkstack( L, 1, "too many arguments" );
				
				switch ( *sig++ )
				{
					case 'b':
						lua_pushboolean( L, va_arg( ap, int ) );
						break;
					case 'd':
						lua_pushinteger( L, va_arg( ap, int ) );
						break;
					case 'f':
						lua_pushnumber( L, va_arg( ap, double ) );
						break;
					case 's':
						lua_pushstring( L, va_arg( ap, const char * ) );
						break;
					case 't':
						lua_pushvalue( L, va_arg( ap, int ) );
						Rtt_ASSERT( lua_istable( L, -1 ) );
						break;
					case 'p':
						lua_pushvalue( L, va_arg( ap, int ) );
						Rtt_ASSERT( lua_isuserdata( L, -1 ) );
						break;
					case 'l':
						lua_pushlightuserdata( L, va_arg( ap, void * ) );
						break;
					case '>':
						hasMoreArgs = false;
						break;
					default:
						break;
				}

				if ( hasMoreArgs )
				{
					++narg;
				}
			}

			nres = (int) strlen( sig );
			Rtt::Lua::DoCall( L, narg, nres );
		}
	}

	return nres;
}

void
Lua::RuntimeDispatchEvent( lua_State* L, int index, int nresults )
{
	Rtt_LogException( "[Lua::RuntimeDispatchEvent()] WARNING: This function is deprecated. Use Lua::DispatchRuntimeEvent() instead.\n");

	int top = lua_gettop( L );
	int indexNormalized = ( index > 0 ? index : top + index + 1 );
	
	PushRuntime( L ); Rtt_ASSERT( lua_istable( L, -1 ) );
    
    if ( ! lua_istable( L, -1 ) )
    {
        // This can happen in simulatorAnalytics.lua
        Rtt_LogException( "ERROR: no runtime to send event to\n");
        
        return;
    }
	lua_getfield( L, -1, "dispatchEvent" ); Rtt_ASSERT( lua_isfunction( L, -1 ) );
	lua_insert( L, -2 ); // swap stack positions for "Runtime" and "dispatchEvent"
	int nargs = lua_gettop( L ) - top; Rtt_ASSERT( 2 == nargs );
	lua_pushvalue( L, indexNormalized );
	DoCall( L, nargs, nresults );
}
void
Lua::CopyTable( lua_State *L, lua_State *srcL, int srcIndex )
{
	srcIndex = Lua::Normalize( srcL, srcIndex );

	lua_pushnil( srcL ); // first key
	lua_newtable(L);
	while ( lua_next( srcL, srcIndex ) != 0 ) // pushes k,v on srcL
	{
	
		CopyValue(L, srcL, -2 ); // Push copy of key onto dst L
		CopyValue(L, srcL, -1 ); // Push copy of value onto dst L

		lua_settable( L, -3 ); // On dst L, do t[key] = value

		lua_pop( srcL, 1 ); // pop value off srcL
	}	
}

void
Lua::CopyValue( lua_State *L, lua_State *srcL, int srcIndex )
{
	srcIndex = Lua::Normalize( srcL, srcIndex );
	int keyType = lua_type( srcL, srcIndex );

	switch( keyType )
	{
		case LUA_TSTRING:
			{
				const char *value = lua_tostring( srcL, srcIndex );
				lua_pushstring( L, value );
			}
			break;	
		case LUA_TNUMBER:
			{
				lua_Number value = lua_tonumber( srcL, srcIndex );
				lua_pushnumber( L, value );
			}
			break;	
		case LUA_TBOOLEAN:
			{
				int value = lua_toboolean( srcL, srcIndex );
				lua_pushboolean( L, value );
			}
			break;
		case LUA_TTABLE:
			CopyTable( L, srcL, srcIndex );
			break;	
		case LUA_TNIL:
			lua_pushnil( L );
			break;
		default:
			Rtt_ASSERT_NOT_IMPLEMENTED();
			break;	
	}
}

void
Lua::AddCoronaViewListener( lua_State *L, lua_CFunction listener, void *lightuserdata )
{
	Rtt_LUA_STACK_GUARD( L );

	// Push Runtime.addEventListener
	// Push Runtime
	Lua::PushRuntime( L );
	lua_getfield( L, -1, "addEventListener" );
	lua_insert( L, -2 ); // swap table and function

	// Push 'coronaView'
	lua_pushstring( L, "coronaView" );

	// Push 'listener'
	lua_pushlightuserdata( L, lightuserdata );
	lua_pushcclosure( L, listener, 1 );

	// Runtime.addEventListener( Runtime, "coronaView", listener )
	int status = Lua::DoCall( L, 3, 0 ); Rtt_UNUSED( status );
	Rtt_ASSERT( 0 == status );
}

// ----------------------------------------------------------------------------

LuaStackGuard::LuaStackGuard( lua_State *L, const char *label )
	:	fL( L ),
	fTop( lua_gettop( L ) ),
	fLabel( label )
{
}

LuaStackGuard::LuaStackGuard( lua_State *L, const char *label, int ret )
:	fL( L ),
	fTop( lua_gettop( L ) + ret ),
	fLabel( label )
{
}

LuaStackGuard::~LuaStackGuard()
{
	int newTop = lua_gettop( fL );
	if ( fTop != newTop )
	{
		const char *label = fLabel ? fLabel : "";
		Rtt_LogException( "ERROR: %s Unbalanced Lua stack: expectedTop(%d) != newTop(%d)\n", label, fTop, newTop );
	}
}
	

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

