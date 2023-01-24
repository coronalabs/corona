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
#include <string.h>

// ----------------------------------------------------------------------------

static const int DataKey = 1;
static const int InfoKey = 2;
static const int VersionKey = 3;
static const int DummyMTKey = 4;

#define CORONA_DUMMY_MEMORY_METATABLE "DummyMemoryMetatable"

// ----------------------------------------------------------------------------

static const int MemoryVersion = 0;

CORONA_API
int CoronaMemoryCreateInterface( lua_State *L, const CoronaMemoryInterfaceInfo *info )
{
	if ( !info->callbacks.getReadableBytes && !info->callbacks.getWriteableBytes )
	{
		CORONA_LOG_WARNING( "Interface must have either `getReadableBytes()` or `getWriteableBytes()` callback (or both)" );

		return 0;
	}

	else if ( !info->callbacks.getByteCount )
	{
		CORONA_LOG_WARNING( "Interface must have `getByteCount()` callback" );

		return 0;
	}

	else if ( !info->getObject )
	{
		CORONA_LOG_WARNING( "Interface must have `getObject()`" );

		return 0;
	}

	else if ( info->dataSize < 0 && !lua_isuserdata( L, -1 ) )
	{
		CORONA_LOG_WARNING( "`dataSize` < 0 but item on top of stack is not a userdata" );

		return 0;
	}

	lua_createtable( L, 3, 0 ); // ...[, data], env

	if ( info->dataSize < 0 )
	{
		lua_insert( L, -2 ); // ..., env, data
		lua_rawseti( L, -2, DataKey ); // ..., env = { data }
	}

	void* mii = lua_newuserdata( L, sizeof( CoronaMemoryInterfaceInfo ) ); // ..., env, info

	*static_cast< CoronaMemoryInterfaceInfo* >( mii ) = *info;

	lua_rawseti( L, -2, InfoKey ); // ..., env = { ..., info }
	lua_pushinteger( L, MemoryVersion ); // ..., env, version
	lua_rawseti( L, -2, VersionKey ); // ..., env = { ..., version }
	luaL_newmetatable( L, CORONA_DUMMY_MEMORY_METATABLE ); // ..., env, dummy_mt
	lua_pushlightuserdata( L, ( void* )lua_topointer( L, -1 ) ); // ..., env, dummy_mt, dummy_mt_ptr
	lua_rawseti( L, -3, DummyMTKey ); // ..., env = { ..., dummy_mt_ptr }, dummy_mt
	lua_pop( L, 1 ); // ..., env

	size_t proxy_size = info->dataSize > 0 ? ( size_t )info->dataSize : 0;

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
	U16 fGuardBits1 : 2;
	U16 fID : 12;
	U16 fGuardBits2 : 2;
	U16 fContext;

	static U16 MaxID() { return ( 1 << 12 ) - 1; }
	static U16 GuardBitPattern1() { return ( 1 << 0 ) + ( 0 << 1 ); }
	static U16 GuardBitPattern2() { return ( 0 << 0 ) + ( 1 << 1 ); }
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

		*id = static_cast< U16 >( frontier );

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
	int top = lua_gettop( L ), exists = 0;

	lua_getfield( L, LUA_REGISTRYINDEX, CORONA_MEMORY_LOOKUP_SLOTS ); // ..., slots?

	if ( !lua_isnil( L, -1 ) )
	{
		lua_rawgeti( L, -1, ( int )id + 1 ); // ..., slots, true, exists

		exists = !lua_isnil( L, -1 );
	}

	if ( exists )
	{
		lua_pushnil( L ); // ..., slots, true, nil
		lua_rawseti( L, -3, ( int )id + 1 ); // ..., slots = { ..., [slot] = nil }, true
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
	int top = lua_gettop( L ), exists = 0;

	lua_getfield( L, LUA_REGISTRYINDEX, CORONA_MEMORY_LOOKUP_SLOTS ); // ..., slots?

	if ( !lua_isnil( L, -1 ) )
	{
		lua_rawgeti( L, -1, ( int )id + 1 ); // ..., slots, exists

		exists = !lua_isnil( L, -1 );
	}

	lua_settop( L, top ); // ...

	if ( exists )
	{
		LightUserdataEncoding encoding;

		encoding.fID = id;
		encoding.fContext = context;
		encoding.fGuardBits1 = LightUserdataEncoding::GuardBitPattern1();
		encoding.fGuardBits2 = LightUserdataEncoding::GuardBitPattern2();

		// See:
		// https://stackoverflow.com/a/51228315, "How do we Type Pun correctly?"
		// https://tttapa.github.io/Pages/Programming/Cpp/Practices/type-punning.html#std-memcpy
		void* ptr = NULL; // since it might be > encoding size

		memcpy( &ptr, &encoding, sizeof( LightUserdataEncoding ) );

		lua_pushlightuserdata( L, ptr ); // ..., encoding
	}

	else
	{
		CORONA_LOG_WARNING( "Attempt to push lookup encoding (context = %u), but id = %u not bound", ( unsigned int )context, ( unsigned int )id );
	}

	return exists;
}

// ----------------------------------------------------------------------------

static const void* NullRead( CoronaMemoryAcquireState* ) { return NULL; }
static const void* NullReadOfSize( CoronaMemoryAcquireState*, size_t ) { return NULL; }
static void NullCopyTo( CoronaMemoryAcquireState *, void*, size_t, int ) {}
static void* NullWrite( CoronaMemoryAcquireState* ) { return NULL; }
static void* NullWriteOfSize( CoronaMemoryAcquireState*, size_t ) { return NULL; }
static int NullResize( CoronaMemoryAcquireState*, size_t, int ) { return 0; }
static size_t NullByteCount( CoronaMemoryAcquireState* ) { return 0; }
static size_t NullGetAlignment( CoronaMemoryAcquireState* ) { return 0; }
static int NullGetIndexed( CoronaMemoryAcquireState*, unsigned int, size_t* ) { return 0; }

// ----------------------------------------------------------------------------

#define STATE_CALLBACK( NAME ) state->callbacks->NAME( &state->workspace )
#define STATE_CALLBACK_WITH_ARGS( NAME, ... ) state->callbacks->NAME( &state->workspace, __VA_ARGS__ )

static const void* Read( CoronaMemoryAcquireState *state )
{
	return STATE_CALLBACK( getReadableBytes );
}

static const void* ReadOfSize( CoronaMemoryAcquireState *state, size_t n )
{
	size_t count = STATE_CALLBACK( getByteCount );

	if ( count >= n )
	{
		return STATE_CALLBACK( getReadableBytes );
	}
	
	if ( STATE_CALLBACK_WITH_ARGS( resize, n, 0 ) )
	{
		return STATE_CALLBACK( getReadableBytes );
	}
	
	else
	{
		return NULL;
	}
}

static void CopyTo( CoronaMemoryAcquireState *state, void* output, size_t n, int ignoreExtra )
{
	size_t count = STATE_CALLBACK( getByteCount );

	if ( count > n )
	{
		count = n;
	}

	if ( count > 0 )
	{
		memcpy( output, STATE_CALLBACK( getReadableBytes ), count );

		if ( count < n && !ignoreExtra )
		{
			memset( static_cast<unsigned char*>( output ) + count, 0, n - count );
		}
	}
}

static void* Write( CoronaMemoryAcquireState *state )
{
	return STATE_CALLBACK( getWriteableBytes );
}

static void* WriteOfSize( CoronaMemoryAcquireState *state, size_t n )
{
	size_t count = STATE_CALLBACK( getByteCount );

	if ( count >= n )
	{
		return STATE_CALLBACK( getWriteableBytes );
	}
	
	if ( STATE_CALLBACK_WITH_ARGS( resize, n, 1 ) )
	{
		return STATE_CALLBACK( getWriteableBytes );
	}
	
	else
	{
		return NULL;
	}
}

static int Resize( CoronaMemoryAcquireState *state, size_t size, int writeable )
{
	return STATE_CALLBACK_WITH_ARGS( resize, size, writeable );
}

static size_t GetByteCount( CoronaMemoryAcquireState *state )
{
	return STATE_CALLBACK( getByteCount );
}

static size_t GetAlignment( CoronaMemoryAcquireState *state )
{
	return STATE_CALLBACK( getAlignment );
}

static int GetSize( CoronaMemoryAcquireState *state, unsigned int index, size_t *size )
{
	return STATE_CALLBACK_WITH_ARGS( getSize, index, size );
}

static int GetStride( CoronaMemoryAcquireState *state, unsigned int index, size_t *stride )
{
	return STATE_CALLBACK_WITH_ARGS( getStride, index, stride );
}

#undef STATE_CALLBACK
#undef STATE_CALLBACK_WITH_ARGS

// ----------------------------------------------------------------------------

static void AddReadableBytes( CoronaMemoryInterface *mi, const CoronaMemoryInterfaceInfo *mii )
{
	if ( mii->callbacks.getReadableBytes )
	{
		mi->getReadableBytes = Read;
		mi->getReadableBytesOfSize = ReadOfSize;
		mi->copyBytesTo = CopyTo;
	}

	else
	{
		mi->getReadableBytes = NullRead;
		mi->getReadableBytesOfSize = NullReadOfSize;
		mi->copyBytesTo = NullCopyTo;
	}
}

static void AddWriteableBytes( CoronaMemoryInterface *mi, const CoronaMemoryInterfaceInfo *mii )
{
	if ( mii->callbacks.getWriteableBytes )
	{
		mi->getWriteableBytes = Write;
		mi->getWriteableBytesOfSize = WriteOfSize;
	}

	else
	{
		mi->getWriteableBytes = NullWrite;
		mi->getWriteableBytesOfSize = NullWriteOfSize;
	}
}

static void AddResize( CoronaMemoryInterface *mi, const CoronaMemoryInterfaceInfo *mii )
{
	mi->resize = mii->callbacks.resize ? Resize : NullResize;
}

static void AddAlignment( CoronaMemoryInterface *mi, const CoronaMemoryInterfaceInfo *mii )
{
	mi->getAlignment = mii->callbacks.getAlignment ? GetAlignment : NullGetAlignment;
}

static void AddIndexed( CoronaMemoryInterface *mi, const CoronaMemoryInterfaceInfo *mii )
{
	mi->getSize = mii->callbacks.getSize ? GetSize : NullGetIndexed;
	mi->getStride = mii->callbacks.getStride ? GetStride : NullGetIndexed;
}

// ----------------------------------------------------------------------------

#define CORONA_MEMORY_STRING_PROXY "MemoryStringProxy"

static const void* GetStringReadableBytes( CoronaMemoryWorkspace *ws )
{
	return ws->vars[0].cp;
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

static bool GetStringInterfaceProxy( lua_State *L )
{
	lua_getfield( L, LUA_REGISTRYINDEX, CORONA_MEMORY_STRING_PROXY ); // ..., object, ..., proxy?

	if ( lua_isnil( L, -1 ) )
	{
		CoronaMemoryInterfaceInfo string_interface_info = {};

		string_interface_info.callbacks.getReadableBytes = GetStringReadableBytes;
		string_interface_info.callbacks.getByteCount = GetStringByteCount;
		string_interface_info.getObject = GetStringObject;

		int result = CoronaMemoryCreateInterface( L, &string_interface_info ); // ..., object, ..., nil, proxy

		Rtt_VERIFY( result );

		lua_pushvalue( L, -1 ); // ..., object, ..., nil, proxy, proxy
		lua_setfield( L, LUA_REGISTRYINDEX, CORONA_MEMORY_STRING_PROXY ); // ..., object, ..., nil, proxy; registry[STRING_PROXY] = proxy
		lua_remove( L, -2 ); // ..., object, ..., proxy
	}

	return true;
}

// ----------------------------------------------------------------------------

static void PopulateInterface( CoronaMemoryAcquireState *state, CoronaMemoryInterfaceInfo *mii )
{
	state->methods.getByteCount = GetByteCount;
	
	AddReadableBytes( &state->methods, mii );
	AddWriteableBytes( &state->methods, mii );
	AddResize( &state->methods, mii );
	AddAlignment( &state->methods, mii );
	AddIndexed( &state->methods, mii );
}

static void DummyInterface( CoronaMemoryAcquireState *state )
{
	state->methods.getReadableBytes = NullRead;
	state->methods.getReadableBytesOfSize = NullReadOfSize;
	state->methods.copyBytesTo = NullCopyTo;
	state->methods.getWriteableBytes = NullWrite;
	state->methods.getWriteableBytesOfSize = NullWriteOfSize;
	state->methods.resize = NullResize;
	state->methods.getByteCount = NullByteCount;
	state->methods.getAlignment = NullGetAlignment;
	state->methods.getSize = NullGetIndexed;
	state->methods.getStride = NullGetIndexed;
}

// ----------------------------------------------------------------------------

CORONA_API
int CoronaMemoryAcquireInterface( lua_State *L, int arg, CoronaMemoryAcquireState *state )
{
	int type = lua_type( L, arg ), top = lua_gettop( L );
	bool found = false;

	if ( LUA_TSTRING == type )
	{
		found = GetStringInterfaceProxy( L ); // ..., object, ...[, proxy]
	}

	else if ( LUA_TLIGHTUSERDATA == type )
	{
		// see notes in CoronaMemoryPushLookupEncoding()
		void* ptr = lua_touserdata( L, arg );

		LightUserdataEncoding encoding;

		memcpy( &encoding, &ptr, sizeof( LightUserdataEncoding ) );

		if ( LightUserdataEncoding::GuardBitPattern1() == encoding.fGuardBits1 && LightUserdataEncoding::GuardBitPattern2() == encoding.fGuardBits2 )
		{
			lua_getfield( L, LUA_REGISTRYINDEX, CORONA_MEMORY_LOOKUP_SLOTS ); // ..., object, ..., slots?

			if ( !lua_isnil( L, -1 ) )
			{
				lua_rawgeti( L, -1, ( int )encoding.fID + 1 ); // ..., object, ..., slots, proxy?
				
				found = !lua_isnil( L, -1 );

				if ( found )
				{
					state->workspace.vars[0].u = 1;
					state->workspace.vars[1].u = encoding.fID;
					state->workspace.vars[2].u = encoding.fContext;
				}
			}
		}
	}

	else if ( luaL_getmetafield( L, arg, "__memory" ) ) // ..., object, ...[, proxy]
	{
		state->workspace.vars[0].u = 0;
		
		found = IsMemoryProxy( L, -1 );
	}
	
	CoronaMemoryInterfaceInfo *interface_info = NULL;
	
	if ( found )
	{
		Rtt_VERIFY( IsMemoryProxy( L, -1 ) );
		
		lua_getfenv( L, -1 ); // ..., object, ..., proxy, env
		lua_rawgeti( L, -1, InfoKey ); // ..., object, ..., proxy, env, info
		lua_rawgeti( L, -2, VersionKey ); // ..., object, ..., proxy, env, info, version
	
		interface_info = ( CoronaMemoryInterfaceInfo* )lua_touserdata( L, -2 );
		state->callbacks = &interface_info->callbacks;
		state->version = ( int )lua_tointeger( L, -1 );

		size_t dataSize = lua_objlen( L, -4 );
		
		if ( dataSize > 0 )
		{
			state->workspace.data = lua_touserdata( L, -4 );
			state->workspace.dataSize = dataSize;
		}
		
		else
		{
			lua_rawgeti( L, -3, DataKey ); // ..., object, ..., proxy, env, info, version, data?

			state->workspace.data = lua_touserdata( L, -1 );
			state->workspace.dataSize = lua_objlen( L, -1 );
		}
	}
	
	else
	{
		CORONA_LOG_WARNING( "Unable to find memory interface proxy" );
	}
	
	lua_settop( L, top ); // ..., object, ...
	
	int result = 0;
	
	if ( found )
	{
		*state->workspace.error = '\0';

		result = interface_info->getObject( L, arg, &state->workspace ); // n.b. might adjust stack

		if ( result )
		{
			PopulateInterface( state, interface_info );
		}
		
		else
		{
			CORONA_LOG_WARNING( "Failed to get object memory" );
		}
	}

	if ( !result )
	{
		DummyInterface( state );
	}

	return result;
}

// ----------------------------------------------------------------------------
