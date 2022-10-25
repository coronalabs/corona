//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "CoronaMemory.h"
#include "CoronaLua.h"
#include "Core/Rtt_Types.h"

// ----------------------------------------------------------------------------

static const int DataKey = 1;
static const int InfoKey = 2;
static const int InterfaceKey = 3;
static const int VersionKey = 4;
static const int DummyMTKey = 5;

#define CORONA_DUMMY_MEMORY_METATABLE "DummyMemoryMetatable"

// ----------------------------------------------------------------------------

static void AddReadableBytes( CoronaMemoryInterface *mi, const CoronaMemoryInterfaceInfo *mii )
{
}

static void AddReadableBytes( CoronaMemoryInterface *mi, const CoronaMemoryInterfaceInfo *mii )
{
}

// etc.

// ----------------------------------------------------------------------------

static const int MemoryVersion = 0;

CORONA_API
int CoronaMemoryCreateInterface( lua_State *L, const CoronaMemoryInterfaceInfo *mii )
{
	if (!mii->callbacks.getReadableBytes && !mii->callbacks.getWriteableBytes)
	{
		CORONA_LOG_WARNING( "Interface must have either `getReadableBytes()` or `getWriteableBytes()` callback (or both)" );

		return 0;
	}

	else if ( !mii->callbacks.getByteCount )
	{
		CORONA_LOG_WARNING( "Interface must have `getByteCount()` callback" );

		return 0;
	}

	else if ( !mii->getObject )
	{
		CORONA_LOG_WARNING( "Interface must have `getObject()`" );

		return 0;
	}

	else if ( mii->dataSize < 0 && !lua_isuserdata( L, -1 ) )
	{
		CORONA_LOG_WARNING( "`dataSize` < 0 but item on top of stack is not a userdata" );

		return 0;
	}

	lua_createtable( L, 3, 0 ); // ...[, data], env

	if ( mii->dataSize < 0 )
	{
		lua_insert( L, -2 ); // ..., env, data
		lua_rawseti( L, -2, DataKey ); // ..., env = { data }
	}

	void* info = lua_newuserdata( L, sizeof( CoronaMemoryInterfaceInfo ) ); // ..., env, info

	memcpy( info, &mii, sizeof( CoronaMemoryInterfaceInfo ) );

	lua_rawseti( L, -2, InfoKey ); // ..., env = { ..., info }

	// TODO: interface

	lua_pushinteger( L, MemoryVersion ); // ..., env, version
	lua_rawseti( L, -2, VersionKey ); // ..., env = { ..., version }
	luaL_newmetatable( L, CORONA_DUMMY_MEMORY_METATABLE ); // ..., env, dummy_mt
	lua_pushlightuserdata( L, ( void* )lua_topointer( L, -1 ) ); // ..., env, dummy_mt, dummy_mt_ptr
	lua_rawseti( L, -3, DummyMTKey ); // ..., env = { ..., dummy_mt_ptr }, dummy_mt
	lua_pop( L, 1 ); // ..., env

	size_t proxy_size = mii->dataSize > 0 ? ( size_t )mii->dataSize : 0;

	lua_newuserdata( L, ( size_t )proxy_size ); // ..., env, proxy
	lua_insert( L, -2 ); // ..., proxy, env
	lua_setfenv( L, -2 ); // ..., proxy; proxy.environment = env

	return 1;
}

// ----------------------------------------------------------------------------

static bool IsMemoryProxy( lua_State *L, int arg )
{
	if ( !lua_isuserdata( L, arg ) )
	{
		return false;
	}

	int top = lua_gettop( L ), is_proxy = 0;

	lua_getfenv( L, arg ); // ..., object, ..., env
	lua_rawgeti( L, -1, DummyMTKey ); // ..., object, ..., env, dummy_mt_ptr?

	if ( !lua_isnil( L, -1 ) )
	{
		luaL_getmetatable( L, CORONA_DUMMY_MEMORY_METATABLE ); // ..., object, ..., env, dummy_mt_ptr, dummy_mt

		is_proxy = lua_touserdata( L, -2 ) == lua_topointer( L, -1 );
	}

	lua_settop( L, top ); // ..., object, ...

	return is_proxy;
}

// ----------------------------------------------------------------------------

struct LightUserdataEncoding {
	U16 mGuardBits1 : 2;
	U16 mID : 12;
	U16 mGuardBits2 : 2;
	U16 mContext;

	static U16 MaxID() { return (1 << 12) - 1; }
	static U16 GuardBitPattern1() { return (1 << 0) + (0 << 1); }
	static U16 GuardBitPattern2() { return (0 << 0) + (1 << 1); }
};

union LightUserdataPunning {
	void * mPtr;
	LightUserdataEncoding mEncoding;
};

static_assert( sizeof( LightUserdataEncoding ) <= sizeof( void* ), "Lossy encoding as light userdata" );

// ----------------------------------------------------------------------------

#define CORONA_MEMORY_LOOKUP_SLOTS "MemoryBindLookupSlots"

CORONA_API
int CoronaMemoryBindLookupSlot( lua_State *L, unsigned short *id )
{
	if ( !IsMemoryProxy( L, -1 ) )
	{
		CORONA_LOG_WARNING( "Expected memory proxy on top of stack" );

		return 0;
	}

	lua_getfield( L, LUA_REGISTRYINDEX, CORONA_MEMORY_LOOKUP_SLOTS ); // ..., proxy, slots?

	if ( lua_isnil( L, -1 ) )
	{
		lua_pop( L, 1 ); // ..., proxy
		lua_newtable( L ); // ..., proxy, slots
		lua_pushvalue( L, -1 ); // ..., proxy, slots, slots
		lua_setfield( L, LUA_REGISTRYINDEX, CORONA_MEMORY_LOOKUP_SLOTS ); // ..., proxy, slots; registry[SLOTS] = slots
	}

	size_t frontier = lua_objlen( L, -1 ); // find a position before a free slot: N.B. that might be a hole, per behavior of Lua's # operator

	if ( frontier < LightUserdataEncoding::MaxID() )
	{
		lua_insert( L, -2 ); // ..., slots, proxy
		lua_rawseti( L, -2, ( int )frontier + 1 ); // ..., slots = { ..., [slot] = proxy }
		lua_pop( L, 1 ); // ...

		*id = static_cast< unsigned short >( frontier );

		return 1;
	}

	else
	{
		lua_pop( L, 2 ); // ...

		CORONA_LOG_WARNING( "Lookup slots all bound" );

		return 0;
	}
}

// ----------------------------------------------------------------------------

CORONA_API
int CoronaMemoryReleaseLookupSlot( lua_State *L, unsigned short id )
{
	lua_getfield( L, LUA_REGISTRYINDEX, CORONA_MEMORY_LOOKUP_SLOTS ); // ..., slots?

	int top = lua_gettop( L ), exists = 0;

	if ( !lua_isnil( L, -1 ) )
	{
		lua_rawgeti( L, -1, ( int )id + 1 ); // ..., slots, exists

		exists = !lua_isnil( L, -1 );
	}

	if ( exists )
	{
		lua_pushnil( L ); // ..., slots, nil
		lua_rawseti( L, -2, ( int )id + 1 ); // ..., slots = { ..., [slot] = nil }
	}

	else
	{
		CORONA_LOG_WARNING( "Attempt to release lookup slot %u, but not bound", ( unsigned int )id );
	}
	
	lua_settop( L, top ); // ...

	return exists;
}

// ----------------------------------------------------------------------------

CORONA_API
int CoronaMemoryPushLookupEncoding( lua_State *L, unsigned short id, unsigned short context )
{
	lua_getfield( L, LUA_REGISTRYINDEX, CORONA_MEMORY_LOOKUP_SLOTS ); // ..., slots?

	int top = lua_gettop( L ), exists = 0;

	if ( !lua_isnil( L, -1 ) )
	{
		lua_rawgeti( L, -1, ( int )id + 1 ); // ..., slots, exists

		exists = !lua_isnil( L, -1 );
	}

	else
	{
		CORONA_LOG_WARNING( "Attempt to push lookup encoding (context = %u), but id = %u not bound", ( unsigned int )context, ( unsigned int )id );
	}

	lua_settop( L, top ); // ...

	if ( exists )
	{
		LightUserdataPunning punning;

		punning.mEncoding.mID = id;
		punning.mEncoding.mContext = context;
		punning.mEncoding.mGuardBits1 = LightUserdataEncoding::GuardBitPattern1();
		punning.mEncoding.mGuardBits2 = LightUserdataEncoding::GuardBitPattern2();

		lua_pushlightuserdata(L, punning.mPtr ); // ..., encoding
	}

	return exists;
}

// ----------------------------------------------------------------------------

#define CORONA_MEMORY_STRING_INTERFACE "MemoryStringInterface"

static const unsigned char* GetStringReadableBytes( CoronaMemoryWorkspace *ws )
{
	return static_cast< const unsigned char* >( ws->vars[0].cp );
}

static size_t GetStringByteCount( CoronaMemoryWorkspace *ws )
{
	return ws->vars[1].size;
}

static int GetStringObject( lua_State *L, int arg, CoronaMemoryWorkspace *ws )
{
	ws->vars[0].cp = lua_tostring( L, arg );
	ws->vars[1].size = lua_objlen( L, arg );

	return 1;
}

// ----------------------------------------------------------------------------

static void GetStringInterface( lua_State *L, int arg, CoronaMemoryCallbacksInfo *info )
{
	lua_getfield( L, LUA_REGISTRYINDEX, CORONA_MEMORY_STRING_INTERFACE ); // ..., object, ..., string_interface?

	if ( lua_isnil( L, -1 ) )
	{
		CoronaMemoryInterfaceInfo string_interface_info = {};

		string_interface_info.callbacks.getReadableBytes = GetStringReadableBytes;
		string_interface_info.callbacks.getByteCount = GetStringByteCount;
		string_interface_info.getObject = GetStringObject;

		int result = CoronaMemoryCreateInterface( L, &string_interface_info ); // ..., object, ..., nil, string_interface

		Rtt_VERIFY( result );

		lua_pushvalue( L, -1 ); // ..., object, ..., nil, string_interface, string_interface
		lua_setfield( L, LUA_REGISTRYINDEX, CORONA_MEMORY_STRING_INTERFACE ); // ..., object, ..., nil, string_interface; registry[STRING_INTERFACE] = string_interface
		lua_remove( L, -2 ); // ..., object, ..., string_interface
	}

	// TODO:
	info->interface;
	info->callbacks;
}

// ----------------------------------------------------------------------------

CORONA_API
int CoronaMemoryAcquireInterface( lua_State *L, int arg, CoronaMemoryCallbacksInfo *info )
{
	int type = lua_type( L, arg );

	if ( LUA_TSTRING == type )
	{
		GetStringInterface( L, arg, info );
	}

	else if ( LUA_TLIGHTUSERDATA == type )
	{
		LightUserdataPunning punning;

		punning.mPtr = lua_touserdata( L, arg );

		if ( LightUserdataEncoding::GuardBitPattern1() == punning.mEncoding.mGuardBits1 && LightUserdataEncoding::GuardBitPattern2() == punning.mEncoding.mGuardBits2 )
		{
			// TODO: exists?
				// do acquire()
			// TODO special case
		}

		else
		{
			// ERROR
		}
	}

	else if (luaL_getmetafield( L, arg, "__memory" ) ) // ..., object, ...[, memory]
	{
		 if ( IsMemoryProxy( L, -1 ) )
		 {
			 // TODO rig up interface
		 }

		 else
		 {
			 // ERROR
		 }
	}

	else
	{
		// ERROR
	}
	
	info->version = MemoryVersion;

	return 0;
}

// ----------------------------------------------------------------------------