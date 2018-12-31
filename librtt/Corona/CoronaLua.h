//-----------------------------------------------------------------------------
//
// Corona Labs
//
// easing.lua
//
// Code is MIT licensed; see https://www.coronalabs.com/links/code/license
//
//-----------------------------------------------------------------------------

#ifndef _CoronaLua_H__
#define _CoronaLua_H__

#include "CoronaMacros.h"
#include "CoronaLog.h"
// ----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

	#include "lua.h"
	#include "lauxlib.h"
	#include "lualib.h"

#ifdef __cplusplus
}
#endif


// Types
// ----------------------------------------------------------------------------

typedef void *CoronaLuaRef;

typedef enum _CoronaLuaFlags
{
	kCoronaLuaFlagNone = 0x0,
	kCoronaLuaFlagOpenStandardLibs = 0x1
}
CoronaLuaFlags;

// C API
// ----------------------------------------------------------------------------

CORONA_API lua_State *CoronaLuaGetCoronaThread( lua_State *coroutine ) CORONA_PUBLIC_SUFFIX;

CORONA_API lua_State *CoronaLuaNew( int flags ) CORONA_PUBLIC_SUFFIX;

CORONA_API void CoronaLuaDelete( lua_State *L ) CORONA_PUBLIC_SUFFIX;

CORONA_API int CoronaLuaNormalize( lua_State *L, int index ) CORONA_PUBLIC_SUFFIX;

CORONA_API void *CoronaLuaGetContext( lua_State *L ) CORONA_PUBLIC_SUFFIX;

CORONA_API void CoronaLuaInitializeContext( lua_State *L, void *context, const char *metatableName ) CORONA_PUBLIC_SUFFIX;

CORONA_API CoronaLuaRef CoronaLuaNewRef( lua_State *L, int index ) CORONA_PUBLIC_SUFFIX;

CORONA_API void CoronaLuaDeleteRef( lua_State *L, CoronaLuaRef ref ) CORONA_PUBLIC_SUFFIX;

CORONA_API int CoronaLuaEqualRef( lua_State *L, CoronaLuaRef ref, int index ) CORONA_PUBLIC_SUFFIX;

CORONA_API void CoronaLuaNewEvent( lua_State *L, const char *eventName ) CORONA_PUBLIC_SUFFIX;

// Invokes listener passing the event at the top of the stack. The event is
// expected to be an event table created by CoronaLuaNewEvent().
// This function pops the event from the stack.
CORONA_API void CoronaLuaDispatchEvent( lua_State *L, CoronaLuaRef listenerRef, int nresults ) CORONA_PUBLIC_SUFFIX;

// Dispatches event at top of stack to the global 'Runtime'. The event is
// expected to be an event table created by CoronaLuaNewEvent().
// This function pops the event from the stack.
CORONA_API void CoronaLuaDispatchRuntimeEvent( lua_State *L, int nresults ) CORONA_PUBLIC_SUFFIX;

CORONA_API int CoronaLuaIsListener( lua_State *L, int index, const char *eventName ) CORONA_PUBLIC_SUFFIX;

CORONA_API void CoronaLuaPushRuntime( lua_State *L ) CORONA_PUBLIC_SUFFIX;

// Deprecated b/c it leaves stack in inconsistent state.
// Use CoronaLuaDispatchRuntimeEvent() instead.
CORONA_API void CoronaLuaRuntimeDispatchEvent( lua_State *L, int index ) CORONA_PUBLIC_SUFFIX;

CORONA_API void CoronaLuaNewGCMetatable( lua_State *L, const char name[], lua_CFunction __gc1 ) CORONA_PUBLIC_SUFFIX;

CORONA_API void CoronaLuaNewMetatable( lua_State *L, const char name[], const luaL_Reg vtable[] ) CORONA_PUBLIC_SUFFIX;

// Same as NewMetatable, but doesn't leave anything on stack
CORONA_API void CoronaLuaInitializeGCMetatable( lua_State *L, const char name[], lua_CFunction __gc1 ) CORONA_PUBLIC_SUFFIX;

CORONA_API void CoronaLuaInitializeMetatable( lua_State *L, const char name[], const luaL_Reg vtable[] ) CORONA_PUBLIC_SUFFIX;

// WARNING: Assumes there will be only one Lua instance of the userdata.
// In other words, when the GC collects the ud, ud will get deleted!
CORONA_API void CoronaLuaPushUserdata( lua_State *L, void *ud, const char metatableName[] ) CORONA_PUBLIC_SUFFIX;

CORONA_API void *CoronaLuaToUserdata( lua_State *L, int index ) CORONA_PUBLIC_SUFFIX;

CORONA_API void *CoronaLuaCheckUserdata( lua_State *L, int index, const char metatableName[] ) CORONA_PUBLIC_SUFFIX;

CORONA_API void CoronaLuaRegisterModuleLoader( lua_State *L, const char *name, lua_CFunction loader, int nupvalues ) CORONA_PUBLIC_SUFFIX;

CORONA_API void CoronaLuaRegisterModuleLoaders( lua_State *L, const luaL_Reg moduleLoaders[], int nupvalues ) CORONA_PUBLIC_SUFFIX;

CORONA_API int CoronaLuaOpenModule( lua_State *L, lua_CFunction loader ) CORONA_PUBLIC_SUFFIX;

// Equivalent to: table.insert( package.loaders, index, loader )
CORONA_API void CoronaLuaInsertPackageLoader( lua_State *L, lua_CFunction loader, int index ) CORONA_PUBLIC_SUFFIX;

CORONA_API lua_CFunction CoronaLuaGetErrorHandler(void) CORONA_PUBLIC_SUFFIX;

CORONA_API void CoronaLuaSetErrorHandler( lua_CFunction newValue ) CORONA_PUBLIC_SUFFIX;

CORONA_API int CoronaLuaDoCall( lua_State *L, int narg, int nresults ) CORONA_PUBLIC_SUFFIX;

CORONA_API int CoronaLuaDoBuffer( lua_State *L, lua_CFunction loader, lua_CFunction pushargs ) CORONA_PUBLIC_SUFFIX;

CORONA_API int CoronaLuaDoFile( lua_State *L, const char* file, int narg, int clear ) CORONA_PUBLIC_SUFFIX;

CORONA_API int CoronaLuaPushModule( lua_State *L, const char *name ) CORONA_PUBLIC_SUFFIX;

#if defined( Rtt_MAC_ENV )
CORONA_API void CoronaLuaLog( lua_State *L, const char *fmt, ... ) __attribute__ ((format (printf, 2, 3))) CORONA_PUBLIC_SUFFIX;
CORONA_API void CoronaLuaLogPrefix( lua_State *L, const char *prefix, const char *fmt, ... ) __attribute__ ((format (printf, 3, 4))) CORONA_PUBLIC_SUFFIX;
CORONA_API void CoronaLuaWarning( lua_State *L, const char *format, ... ) __attribute__ ((format (printf, 2, 3))) CORONA_PUBLIC_SUFFIX;
CORONA_API void CoronaLuaError( lua_State *L, const char *format, ... ) __attribute__ ((format (printf, 2, 3)))  CORONA_PUBLIC_SUFFIX;
#elif defined( Rtt_WIN_ENV ) || defined( Rtt_NINTENDO_ENV )
CORONA_API void CoronaLuaLog(lua_State *L, _Printf_format_string_ const char *fmt, ...) CORONA_PUBLIC_SUFFIX;
CORONA_API void CoronaLuaLogPrefix(lua_State *L, _Printf_format_string_ const char *prefix, const char *fmt, ...) CORONA_PUBLIC_SUFFIX;
CORONA_API void CoronaLuaWarning(lua_State *L, _Printf_format_string_ const char *format, ...) CORONA_PUBLIC_SUFFIX;
CORONA_API void CoronaLuaError(lua_State *L, _Printf_format_string_ const char *format, ...) CORONA_PUBLIC_SUFFIX;
#else
CORONA_API void CoronaLuaLog(lua_State *L, const char *fmt, ...) CORONA_PUBLIC_SUFFIX;
CORONA_API void CoronaLuaLogPrefix(lua_State *L, const char *prefix, const char *fmt, ...) CORONA_PUBLIC_SUFFIX;
CORONA_API void CoronaLuaWarning(lua_State *L, const char *format, ...) CORONA_PUBLIC_SUFFIX;
CORONA_API void CoronaLuaError(lua_State *L, const char *format, ...) CORONA_PUBLIC_SUFFIX;
#endif // Rtt_MAC_ENV

CORONA_API void CoronaLuaLogV(lua_State *L, const char *fmt, va_list arguments) CORONA_PUBLIC_SUFFIX;

CORONA_API void CoronaLuaLogPrefixV(lua_State *L, const char *prefix, const char *fmt, va_list arguments);

#define CORONA_LUA_LOG_WARNING(L, format, ...)	CoronaLuaLogPrefix( (L), "WARNING: ", format "\n", ## __VA_ARGS__ )
#define CORONA_LUA_LOG_ERROR(L, format, ...)	CoronaLuaLogPrefix( (L), "ERROR: ", format "\n", ## __VA_ARGS__ )

#ifndef Rtt_NO_GUI
CORONA_API int CoronaLuaPropertyToJSON(lua_State *L, int idx, const char *key, char *buf, int bufLen, int pos);
#endif

// C++ Wrapper
// ----------------------------------------------------------------------------

#ifdef __cplusplus

namespace Corona
{

// ----------------------------------------------------------------------------

class Lua
{
	public:
		typedef Lua Self;

	public:
		CORONA_INLINE static lua_State *GetCoronaThread( lua_State *coroutine );
		CORONA_INLINE static lua_State *New( int flags );
		CORONA_INLINE static void Delete( lua_State *L );

	public:
		CORONA_INLINE static int Normalize( lua_State *L, int index );

	public:
		CORONA_INLINE static void InitializeContext( lua_State *L, void *context, const char *metatableName = NULL );
		CORONA_INLINE static void *GetContext( lua_State *L );

	public:
		typedef CoronaLuaRef Ref;
		CORONA_INLINE static Ref NewRef( lua_State *L, int index );
		CORONA_INLINE static void DeleteRef( lua_State *L, Ref ref );
		CORONA_INLINE static bool EqualRef( lua_State *L, Ref ref, int index );
		CORONA_INLINE static void NewEvent( lua_State *L, const char *eventName );
		CORONA_INLINE static void DispatchEvent( lua_State *L, Ref listenerRef, int nresults );
		CORONA_INLINE static void DispatchRuntimeEvent( lua_State *L, int nresults );

	public:
		CORONA_INLINE static bool IsListener( lua_State *L, int index, const char *eventName );
		CORONA_INLINE static void PushRuntime( lua_State *L );
		CORONA_INLINE static void RuntimeDispatchEvent( lua_State *L, int index );

	public:
		CORONA_INLINE static void NewGCMetatable( lua_State *L, const char name[], lua_CFunction __gc1 );
		CORONA_INLINE static void NewMetatable( lua_State* L, const char name[], const luaL_Reg vtable[] );

		// Same as NewMetatable, but doesn't leave anything on stack
		CORONA_INLINE static void InitializeGCMetatable( lua_State *L, const char name[], lua_CFunction __gc1 );
		CORONA_INLINE static void InitializeMetatable( lua_State *L, const char name[], const luaL_Reg vtable[] );

	public:
		// WARNING: Assumes there will be only one Lua instance of the userdata.
		// In other words, when the GC collects the ud, ud will get deleted!
		CORONA_INLINE static void PushUserdata( lua_State *L, void *ud, const char metatableName[] );
		CORONA_INLINE static void *ToUserdata( lua_State *L, int index );
		CORONA_INLINE static void *CheckUserdata( lua_State *L, int index, const char metatableName[] );

	public:
		CORONA_INLINE static void RegisterModuleLoader( lua_State *L, const char *name, lua_CFunction loader, int nupvalues = 0 );
		CORONA_INLINE static void RegisterModuleLoaders( lua_State *L, const luaL_Reg moduleLoaders[], int nupvalues = 0 );

	public:
		template < int (*F)( lua_State * ) >
		CORONA_INLINE static int Open( lua_State *L );

		CORONA_INLINE static int OpenModule( lua_State *L, lua_CFunction loader );

	public:
		CORONA_INLINE static void InsertPackageLoader( lua_State *L, lua_CFunction loader, int index );

	public:
		CORONA_INLINE static lua_CFunction GetErrorHandler();
		CORONA_INLINE static void SetErrorHandler( lua_CFunction newValue );

	public:
		CORONA_INLINE static int DoCall( lua_State *L, int narg, int nresults );
		CORONA_INLINE static int DoBuffer( lua_State *L, lua_CFunction loader, lua_CFunction pushargs );
		CORONA_INLINE static int DoFile( lua_State *L, const char* file, int narg, bool clear );

	public:
		CORONA_INLINE static int PushModule( lua_State *L, const char *name );
};

// ----------------------------------------------------------------------------

lua_State *
Lua::GetCoronaThread( lua_State *coroutine )
{
	return CoronaLuaGetCoronaThread( coroutine );
}

lua_State *
Lua::New( int flags )
{
	return CoronaLuaNew( flags );
}

void
Lua::Delete( lua_State *L )
{
	CoronaLuaDelete( L );
}

int
Lua::Normalize( lua_State *L, int index )
{
	return CoronaLuaNormalize( L, index );
}

void
Lua::InitializeContext( lua_State *L, void *context, const char *metatableName )
{
	CoronaLuaInitializeContext( L, context, metatableName );
}

void *
Lua::GetContext( lua_State *L )
{
	return CoronaLuaGetContext( L );
}

Lua::Ref
Lua::NewRef( lua_State *L, int index )
{
	return CoronaLuaNewRef( L, index );
}

void
Lua::DeleteRef( lua_State *L, Ref ref )
{
	CoronaLuaDeleteRef( L, ref );
}

bool
Lua::EqualRef( lua_State *L, Ref ref, int index )
{
	return 0 != CoronaLuaEqualRef( L, ref, index );
}

void
Lua::NewEvent( lua_State *L, const char *eventName )
{
	CoronaLuaNewEvent( L, eventName );
}

void
Lua::DispatchEvent( lua_State *L, Ref listenerRef, int nresults )
{
	CoronaLuaDispatchEvent( L, listenerRef, nresults );
}

void
Lua::DispatchRuntimeEvent( lua_State *L, int nresults )
{
	CoronaLuaDispatchRuntimeEvent( L, nresults );
}

bool
Lua::IsListener( lua_State *L, int index, const char *eventName )
{
	return ( !! CoronaLuaIsListener( L, index, eventName ) );
}

void
Lua::PushRuntime( lua_State *L )
{
	CoronaLuaPushRuntime( L );
}

void
Lua::RuntimeDispatchEvent( lua_State *L, int index )
{
	CoronaLuaRuntimeDispatchEvent( L, index );
}

void
Lua::NewGCMetatable( lua_State *L, const char name[], lua_CFunction __gc1 )
{
	CoronaLuaNewGCMetatable( L, name, __gc1 );
}

void
Lua::NewMetatable( lua_State *L, const char name[], const luaL_Reg vtable[] )
{
	CoronaLuaNewMetatable( L, name, vtable );
}

void
Lua::InitializeGCMetatable( lua_State *L, const char name[], lua_CFunction __gc1 )
{
	CoronaLuaInitializeGCMetatable( L, name, __gc1 );
}

void
Lua::InitializeMetatable( lua_State *L, const char name[], const luaL_Reg vtable[] )
{
	CoronaLuaInitializeMetatable( L, name, vtable );
}

void
Lua::PushUserdata( lua_State *L, void *ud, const char metatableName[] )
{
	CoronaLuaPushUserdata( L, ud, metatableName );
}

void *
Lua::ToUserdata( lua_State *L, int index )
{
	return CoronaLuaToUserdata( L, index );
}

void *
Lua::CheckUserdata( lua_State *L, int index, const char metatableName[] )
{
	return CoronaLuaCheckUserdata( L, index, metatableName );
}

void
Lua::RegisterModuleLoader( lua_State *L, const char *name, lua_CFunction loader, int nupvalues )
{
	CoronaLuaRegisterModuleLoader( L, name, loader, nupvalues );
}

void
Lua::RegisterModuleLoaders( lua_State *L, const luaL_Reg moduleLoaders[], int nupvalues )
{
	CoronaLuaRegisterModuleLoaders( L, moduleLoaders, nupvalues );
}

template < int (*F)( lua_State * ) >
int
Lua::Open( lua_State *L )
{
	return OpenModule( L, F );
}

int
Lua::OpenModule( lua_State *L, lua_CFunction loader )
{
	return CoronaLuaOpenModule( L, loader );
}

void
Lua::InsertPackageLoader( lua_State *L, lua_CFunction loader, int index )
{
	return CoronaLuaInsertPackageLoader( L, loader, index );
}

lua_CFunction
Lua::GetErrorHandler()
{
	return CoronaLuaGetErrorHandler();
}

void
Lua::SetErrorHandler( lua_CFunction newValue )
{
	CoronaLuaSetErrorHandler( newValue );
}

int
Lua::DoCall( lua_State *L, int narg, int nresults )
{
	return CoronaLuaDoCall( L, narg, nresults );
}

int
Lua::DoBuffer( lua_State *L, lua_CFunction loader, lua_CFunction pushargs )
{
	return CoronaLuaDoBuffer( L, loader, pushargs );
}

int
Lua::DoFile( lua_State *L, const char* file, int narg, bool clear )
{
	return CoronaLuaDoFile( L, file, narg, clear );
}

int
Lua::PushModule( lua_State *L, const char *name )
{
	return CoronaLuaPushModule( L, name );
}

// ----------------------------------------------------------------------------

} // namespace Corona

#endif // __cplusplus

// ----------------------------------------------------------------------------

#endif // _CoronaLua_H__
