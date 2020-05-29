//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_Lua_H__
#define _Rtt_Lua_H__

// ----------------------------------------------------------------------------

Rtt_EXPORT_BEGIN

	#define lua_c

	#include "lua.h"
	#include "lauxlib.h"
	#include "lualib.h"

Rtt_EXPORT_END

// ----------------------------------------------------------------------------

namespace Rtt
{

class MCriticalSection;
class String;

// ----------------------------------------------------------------------------

class Lua
{
	public:
		typedef Lua Self;

	public:
		static void Initialize( lua_State *coronaThread );

		// Given the Lua state for a 'coroutine', returns Corona's main Lua state.
		// Returns NULL if the coroutine is not a child/descendent of Corona's main
		// Lua state. If 'coroutine' happens to be Corona's main Lua state, then
		// returns itself.
		static lua_State* GetCoronaThread( lua_State *coroutine );

	public:
		static lua_State* New( bool shouldOpenLibs );
		static void Delete( lua_State *L );

	public:
		static void SetCriticalSection( lua_State *L, const MCriticalSection *criticalSection );

	public:
		static int Normalize( lua_State *L, int index );

	public:
		static void InitializeContext( lua_State *L, void *context, const char *metatableName = NULL );
		static void *GetContext( lua_State *L );

	public:
		typedef void *Ref;
		static Ref NewRef( lua_State *L, int index );
		static void DeleteRef( lua_State *L, Ref ref );
		static bool EqualRef( lua_State *L, Ref ref, int index );
		static void NewEvent( lua_State *L, const char *eventName );
		static void DispatchEvent( lua_State *L, Ref listenerRef, int nresults );
		static void DispatchRuntimeEvent( lua_State *L, int nresults );

	public:
		static bool IsListener( lua_State *L, int index, const char *eventName );
		static void PushRuntime( lua_State *L );

	public:
		static void PushCachedFunction( lua_State *L, lua_CFunction function );

	public:
		static void NewGCMetatable( lua_State* L, const char name[], lua_CFunction __gc1 );
		static void NewMetatable( lua_State* L, const char name[], const luaL_Reg vtable[] );

		// Same as NewMetatable, but doesn't leave anything on stack
		static void InitializeGCMetatable( lua_State* L, const char name[], lua_CFunction __gc1 );
		static void InitializeMetatable( lua_State* L, const char name[], const luaL_Reg vtable[] );

	public:
		// WARNING: Assumes there will be only one Lua instance of the userdata.
		// In other words, when the GC collects the ud, ud will get deleted!
		static void PushUserdata( lua_State* L, void* ud, const char metatableName[] );
		static void* ToUserdata( lua_State* L, int index );
		//! This is the same as CheckUserdata(), except
		//! it doesn't generate a lua error when it fails.
		static void* ToUserdata( lua_State* L, int ud, const char *tname );
		static void* CheckUserdata( lua_State* L, int index, const char metatableName[] );

	public:
		static void RegisterModuleLoader( lua_State *L, const char *name, lua_CFunction loader, int nupvalues = 0 );
		static void RegisterModuleLoaders( lua_State *L, const luaL_Reg moduleLoaders[], int nupvalues = 0 );

	public:
		template < int (*F)( lua_State * ) >
		static int Open( lua_State *L );

		static int OpenModule( lua_State *L, lua_CFunction loader );

	public:
		// Equivalent to: table.insert( package.loaders, index, loader )
		static void InsertPackageLoader( lua_State *L, lua_CFunction loader, int index, void *loaderData = NULL );

	public:
		static lua_CFunction GetErrorHandler( lua_CFunction defaultHandler );
		static void SetErrorHandler( lua_CFunction newValue );
		static void Warning( lua_State *L, const char *fmt, ... );
	
	public:
		static int DoCall( lua_State* L, int narg, int nresults );
		static int DoBuffer( lua_State *L, lua_CFunction loader, lua_CFunction pushargs );
		static int DoFile( lua_State *L, const char* file, int narg, bool clear, String *errorMesg = NULL );

	public:
		static int PushModule( lua_State *L, const char *name );
		static int VCall( lua_State *L, const char *sig, va_list ap, const char *methodName = NULL );

		// Deprecated b/c inconsistencies like leaving event on stack. Use DispatchRuntimeEvent() instead.
		static void RuntimeDispatchEvent( lua_State* L, int index, int nresults );
		
	public:
		static void CopyTable( lua_State *L, lua_State *srcL, int srcIndex );
		static void CopyValue( lua_State *L, lua_State *srcL, int srcIndex );

	// Internal (not public)
	public:
		static void AddCoronaViewListener( lua_State *L, lua_CFunction listener, void *lightuserdata );
};

template < int (*F)( lua_State * ) >
int
Lua::Open( lua_State *L )
{
	return OpenModule( L, F );
}

// ----------------------------------------------------------------------------

#ifdef Rtt_DEBUG
	#define Rtt_LUA_STACK_GUARD( L, ... ) ::Rtt::LuaStackGuard luaStackGuard ## __LINE__ ( ( L ), __FUNCTION__ ,##__VA_ARGS__ )
#else
	#define Rtt_LUA_STACK_GUARD( L, ... )
#endif

class LuaStackGuard
{
	Rtt_CLASS_NO_DYNAMIC_ALLOCATION

	public:
		LuaStackGuard( lua_State *L, const char *label );
		LuaStackGuard( lua_State *L, const char *label, int ret );
		~LuaStackGuard();

	private:
		lua_State *fL;
		int fTop;
		const char *fLabel;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_Lua_H__
