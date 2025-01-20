//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_LuaAux.h"
#include "Rtt_MLuaProxyable.h"

#include <string.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

// LuaProxyVTableTracer wraps LuaProxyVTable instances
// 
#ifdef Rtt_DEBUG
	// Uncomment the following line to enable this functionality:
	//#define Rtt_TRACE_LUAPROXYDELEGATE
#endif

#ifdef Rtt_TRACE_LUAPROXYDELEGATE

class LuaProxyVTableTracer : public LuaProxyVTable
{
	public:
		LuaProxyVTableTracer( const LuaProxyVTable& delegate, void* proxy );

	protected:
		void PrintCommon( lua_State *L, const char metamethod[], const char key[] ) const;

	public:
		virtual int ValueForKey( lua_State *L, const char key[] ) const;
		virtual bool SetValueForKey( lua_State *L, const char key[], int valueIndex ) const;

	private:
		const LuaProxyVTable& fDelegate;
		void* fProxy;
};

LuaProxyVTableTracer::LuaProxyVTableTracer( const LuaProxyVTable& delegate, void* proxy )
:	fDelegate( delegate ),
	fProxy( proxy )
{
}

void
LuaProxyVTableTracer::PrintCommon( lua_State *L, const char metamethod[], const char key[] ) const
{
	Rtt_ASSERT( metamethod );
	Rtt_ASSERT( key );

	Rtt_TRACE( ( "META %s( (this=%p) (key=%s) ", metamethod, fProxy, key ) );
}

int
LuaProxyVTableTracer::ValueForKey( lua_State *L, const char key[] ) const
{
	PrintCommon( L, "   __index", key );
	Rtt_TRACE( ( ")\n" ) );

	return fDelegate.ValueForKey( L, key );
}

bool
LuaProxyVTableTracer::SetValueForKey( lua_State *L, const char key[], int valueIndex ) const
{
	PrintCommon( L, "__newindex", key );
	Rtt_TRACE( ( "(value=" ) );

	switch( lua_type( L, valueIndex ) )
	{
		case LUA_TNIL:
			Rtt_TRACE( ( "nil" ) );
			break;
		case LUA_TBOOLEAN:
			Rtt_TRACE( ( "%s", lua_toboolean( L, valueIndex ) ? "true" : "false" ) );
			break;
		case LUA_TNUMBER:
			Rtt_TRACE( ( "%g", lua_tonumber( L, valueIndex ) ) );
			break;
		case LUA_TSTRING:
			Rtt_TRACE( ( "%s", lua_tostring( L, valueIndex ) ) );
			break;
		case LUA_TLIGHTUSERDATA:
			Rtt_TRACE( ( "%p [light userdata]", lua_topointer( L, valueIndex ) ) );
			break;
		case LUA_TTABLE:
			Rtt_TRACE( ( "%p [table]", lua_topointer( L, valueIndex ) ) );
			break;
		case LUA_TFUNCTION:
			Rtt_TRACE( ( "%p [function]", lua_topointer( L, valueIndex ) ) );
			break;
		case LUA_TUSERDATA:
			Rtt_TRACE( ( "%p [userdata]", lua_topointer( L, valueIndex ) ) );
			break;
		case LUA_TTHREAD:
			Rtt_TRACE( ( "%p [thread]", lua_topointer( L, valueIndex ) ) );
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			Rtt_TRACE( ( "THAT'S RIGHT! NOTHING!" ) );
			break;
	}

	Rtt_TRACE( ( ") )\n" ) );
	
	return fDelegate.SetValueForKey( L, key, valueIndex );
}

#endif // Rtt_TRACE_LUAPROXYDELEGATE

// ----------------------------------------------------------------------------

class NullProxyable : public MLuaProxyable
{
	public:
        NullProxyable() : MLuaProxyable() {}
		virtual void InitProxy( lua_State *L );
		virtual LuaProxy* GetProxy() const;
		virtual void ReleaseProxy();
};

void
NullProxyable::InitProxy( lua_State * )
{
	Rtt_ASSERT_NOT_REACHED();
}

LuaProxy*
NullProxyable::GetProxy() const
{
	Rtt_ASSERT_NOT_REACHED();
	return NULL;
}

void
NullProxyable::ReleaseProxy()
{
	Rtt_ASSERT_NOT_REACHED();
}

// ----------------------------------------------------------------------------

const char LuaProxyConstant::kProxyKey[] = "ProxyConstant";

const luaL_Reg LuaProxyConstant::kMetatable[] =
{
	{ "__index", LuaProxyConstant::__index },
	{ "__gc", LuaProxyConstant::__gcMeta },
	{ NULL, NULL }
};

LuaProxyConstant*
LuaProxyConstant::GetProxy( lua_State *L, int index )
{
	LuaProxyConstant** p = (LuaProxyConstant**)luaL_checkudata( L, index, kProxyKey );
	return ( p ? *p : NULL );
}

const MLuaProxyable&
LuaProxyConstant::NullProxyableObject()
{
	static const NullProxyable sInstance;
	return sInstance;
}

int
LuaProxyConstant::__index( lua_State *L )
{
	int result = 0;

	LuaProxyConstant* proxy = GetProxy( L, 1 );
	if ( proxy )
	{
		const char* key = lua_tostring( L, 2 );

		if ( key )
		{
			const LuaProxyVTable& delegate = proxy->Delegate();
			const LuaProxyVTable* pDelegate = & delegate;

			#ifdef Rtt_TRACE_LUAPROXYDELEGATE
				const LuaProxyVTableTracer tracer( delegate, proxy );
				pDelegate = & tracer;
			#endif

			result = pDelegate->ValueForKey( L, NullProxyableObject(), key ); // Search for key in C++ delegate
		}
	}

	return result;
}

int
LuaProxyConstant::__gcMeta( lua_State *L )
{
	LuaProxyConstant* p = GetProxy( L, 1 );

	Rtt_DELETE( p );

	return 0;
}

void
LuaProxyConstant::Initialize( lua_State *L )
{
	luaL_newmetatable( L, kProxyKey );
	luaL_register( L, NULL, kMetatable );
	lua_pop( L, 1 );
}

LuaProxyConstant::LuaProxyConstant( lua_State *L, const LuaProxyVTable& delegate )
:	fDelegate( delegate )
{
}

LuaProxyConstant::~LuaProxyConstant()
{
}

void
LuaProxyConstant::Push( lua_State *L ) const
{
	const LuaProxyConstant** userdata = (const LuaProxyConstant**)lua_newuserdata( L, sizeof(LuaProxyConstant*) );
	* userdata = this;

	// Set userdata's metatable = mt
	luaL_getmetatable( L, kProxyKey ); // Fetch mt
	Rtt_ASSERT( ! lua_isnil( L, -1 ) );
	lua_setmetatable( L, -2 ); // implicitly pops mt

	Rtt_ASSERT( lua_isuserdata( L, -1 ) );
}

// ----------------------------------------------------------------------------

// LuaProxy and LuaProxyVTables are used to export C++ objects to Lua.
// 
// LuaProxy instances represent C++ objects (e.g. Rtt::DisplayObject).
// Each instance stores a lua reference (see luaL_ref) to a Lua table.
// These Lua tables are the objects that a Lua coder uses to manipulate
// C++ objects.  They should never interact with lua_userdata.
// 
// These Lua tables undergo initialization in two stages --- first in C++
// and second in Lua.
// 
// On the C++ side, Lua tables are initialized (see LuaProxy::CreateTable)
// to have a property "_proxy" whose value is LuaProxy instance (stored
// as Lua userdata).
// 
// On the Lua side, all tables must set the following C++ methods as the
// metamethods in their metatable:
//     * LuaProxy::__proxyindex
//     * LuaProxy::__proxynewindex
// 
// In addition, all Lua tables should have a "_class" property whose value 
// is the Lua table that contains the class definition for the Lua class 
// corresponding to the C++ class that the LuaProxy represents.


// Optimization:
// Uncommenting this facilitates a lazy optimization for Lua-to-C function
// dispatches. During a dispatch, Lua must first obtain a function value using 
// the function name as the key. Since userdata isn't a table, the __index 
// metamethod is invoked. This metamethod is in the userdata's metatable.
// 
// We exploit this by ensuring that each subclass of LuaProxy has its own 
// metatable. As functions in a particular class are fetched by __index, we
// add them to that class' metatable. Subsequent calls for the same function
// will hit the cache.
#define Rtt_OPTIMIZE_LUAPROXY

const char LuaProxy::kProxyKey[] = "Proxy";

const luaL_Reg LuaProxy::kMetatable[] =
{
	{ "__gc", LuaProxy::__gcMeta },
	
	{ NULL, NULL }
};

static const char kProxyFieldKey[] = "_proxy";
static const char kClassFieldKey[] = "_class";


bool LuaProxy::IsProxy(lua_State *L, int index)
{
	bool result = false;
	if ( lua_istable( L, index ) )
	{
		lua_pushlstring( L, kProxyFieldKey, sizeof( kProxyFieldKey ) - 1 );
		lua_rawget( L, index );

		result = (lua_isuserdata( L, -1 ) == 1);

		lua_pop( L, 1 );
	}
	return result;
}

LuaProxy*
LuaProxy::GetProxy( lua_State *L, int index )
{
	LuaProxy* result = NULL;

	if ( lua_istable( L, index ) )
	{
		lua_pushlstring( L, kProxyFieldKey, sizeof( kProxyFieldKey ) - 1 );
		lua_rawget( L, index );
		// lua_getfield( L, index, kProxyFieldKey );
		Rtt_ASSERT( lua_isuserdata( L, -1 ) );

		#ifdef Rtt_DEVICE_ENV
			// On devices, disable checking
			LuaProxy** p =(LuaProxy**)lua_touserdata( L, -1 ); Rtt_ASSERT( luaL_checkudata( L, -1, kProxyKey ) );
		#else
			// On non-devices, enforce checking
			LuaProxy** p =(LuaProxy**)luaL_checkudata( L, -1, kProxyKey );

			Rtt_WARN_SIM(
				( p || lua_isnil( L, -1 ) ),
				( "ERROR: This object is not a display object but is using a display object metamethod.\n" ) );
		#endif

		if ( p )
		{
			result = *p;
		}

		lua_pop( L, 1 );
	}
	else
	{
		luaL_error( L, "ERROR: table expected. If this is a function call, you might have used '.' instead of ':'" );
	}


	return result;
}

LuaProxy*
LuaProxy::GetProxyMeta( lua_State *L, int index )
{
	LuaProxy** p = (LuaProxy**)luaL_checkudata( L, index, kProxyKey );
	return ( p ? *p : NULL );
}

MLuaProxyable*
LuaProxy::GetProxyableObject( lua_State *L, int index )
{
	MLuaProxyable* result = NULL;

	LuaProxy* proxy = GetProxy( L, index );

	if ( proxy )
	{
		result = proxy->Object();
	}

	return result;
}

// Does the equivalent of the following pseudo-code:
// 
//   function gettable_event( table, eventName )
//      return valueForKey( table.proxy_, eventName ) or table._class[eventName]
//   end
// 
// where valueForKey( ud, key ) would be a C function exported to Lua:
// 
//   int valueForKey( L, ud, key )
//   {
//      return ((LuaProxy*)ud)->ValueForKey( L, key )
//   }
// 
int
LuaProxy::__proxyindex( lua_State *L )
{
#if 1
	int result = 0;

	const MLuaProxyable *object = NULL;
	LuaProxy *proxy = LuaProxy::GetProxy( L, 1 ); // proxy = table.proxy_
	if ( Rtt_VERIFY( proxy ) )
	{
		object = proxy->Object();
	}

	if ( object == NULL )
	{
		// Proxy has no object, which means a DO was removed but a reference to the table remains,
		// and someone is trying to set a DO property on it.
		Rtt_WARN_SIM( object == NULL,
			( "WARNING: Attempting to get property (%s) on a removed DisplayObject\n", lua_tostring( L, 2 ) ) );
	}
	else
	{
		// Prevent lua_tostring from converting numbers on the stack to strings
		// const char* key = ! lua_isnumber( L, 2 ) ? lua_tostring( L, 2 ) : NULL;
		const char* key = LUA_TSTRING == lua_type( L, 2 ) ? lua_tostring( L, 2 ) : NULL;

		Rtt_LUA_ASSERT( L, lua_isnumber( L, 2 ) || key, "__proxyindex was passed a NULL key" );
		if ( lua_isnil( L, 2 ) )
		{
			luaL_error( L, "ERROR: nil key supplied for property lookup." );
		}

		const LuaProxyVTable& delegate = proxy->Delegate();
		const LuaProxyVTable* pDelegate = & delegate;

		#ifdef Rtt_TRACE_LUAPROXYDELEGATE
			const LuaProxyVTableTracer tracer( delegate, proxy );
			pDelegate = & tracer;
		#endif

		result = pDelegate->ValueForKey( L, * object, key ); // Search for key in C++ delegate

#ifdef Rtt_PHYSICS
		// Call extensions delegate if it exists. 
		if ( ! result )
		{
			const LuaProxyVTable *extensions = proxy->GetExtensionsDelegate();
			if ( extensions )
			{
				result = extensions->ValueForKey( L, *object, key );
			}
		}
#endif

		if ( ! result )
		{
			lua_getfield( L, 1, kClassFieldKey ); // table._class
			lua_pushvalue( L, 2 ); // key
			lua_gettable( L, -2 ); // push table._class[key]
			lua_remove( L, -2 ); // pop table._class
			result = 1; // Return even if nil
		}
	}

	return result;

#else
	int result = 1;

	// Search for cached functions in metatable
	lua_getmetatable( L, 1 ); // mt = metatable(table)
	lua_pushvalue( L, 2 ); // key
	lua_rawget( L, -2 ); // fetch mt[key]

	if ( lua_isnil( L, -1 ) )
	{
		lua_pop( L, 1 ); // pop nil

		result = 0;

		LuaProxy* proxy = LuaProxy::GetProxy( L, 1 ); // proxy = table.proxy_
		if ( Rtt_VERIFY( proxy ) )
		{
			// Prevent lua_tostring from converting numbers on the stack to strings
			const char* key = ! lua_isnumber( L, 2 ) ? lua_tostring( L, 2 ) : NULL;

			Rtt_LUA_ASSERT( L, lua_isnumber( L, 2 ) || key, "__proxyindex was passed a NULL key" );

			const LuaProxyVTable& delegate = proxy->Delegate();
			const LuaProxyVTable* pDelegate = & delegate;

			#ifdef Rtt_TRACE_LUAPROXYDELEGATE
				const LuaProxyVTableTracer tracer( delegate, proxy );
				pDelegate = & tracer;
			#endif

			result = pDelegate->ValueForKey( L, key ); // Search for key in C++ delegate

			if ( result )
			{
				// Cache functions in metatable
				if ( lua_iscfunction( L, -1 ) )
				{
					lua_pushvalue( L, 2 ); // key
					lua_pushvalue( L, -2 ); // function
					lua_rawset( L, -4 ); // mt[key] = function
				}
			}
			else
			{
				lua_getfield( L, 1, kClassFieldKey ); // table._class
				lua_pushvalue( L, 2 ); // key
				lua_gettable( L, -2 ); // push table._class[key]
				lua_remove( L, -2 ); // pop table._class
				result = 1; // Return even if nil
			}
		}
	}

	lua_remove( L, -2 ); // pop metatable
	return result;
#endif
}

int
LuaProxy::__proxynewindex( lua_State *L )
{
	MLuaProxyable *object = NULL;
	LuaProxy* proxy = LuaProxy::GetProxy( L, 1 ); // proxy = table.proxy_
	if ( Rtt_VERIFY( proxy ) )
	{
		object = proxy->Object();
	}

	if ( object == NULL )
	{
		// Proxy has no object, which means a DO was removed but a reference to the table remains,
		// and someone is trying to set a DO property on it.
		Rtt_WARN_SIM( object == NULL,
			( "WARNING: Attempting to set property (%s) on a removed DisplayObject\n", lua_tostring( L, 2 ) ) );
	}
	else
	{
		const char* key = lua_tostring( L, 2 );

		Rtt_LUA_ASSERT( L, key, "__proxynewindex was passed a NULL key" );

		const LuaProxyVTable& delegate = proxy->Delegate();
		const LuaProxyVTable* pDelegate = & delegate;

		#ifdef Rtt_TRACE_LUAPROXYDELEGATE
			const LuaProxyVTableTracer tracer( delegate, proxy );
			pDelegate = & tracer;
		#endif

		// Warn whenever the key corresponds to a proxy property (ValueForKey != 0 )
		// and the newValue is none; ignore otherwise. Some properties can be set to 'nil'.
		Rtt_WARN_SIM(
			pDelegate->ValueForKey( L, * object, key ) == 0 || ! lua_isnone( L, 3 ),
			( "WARNING: Attempting to set property(%s) but no value was provided\n", key ) );

		bool didSetValue = false;

#ifdef Rtt_PHYSICS
		// Call extensions delegate if it exists. This should be add-on behavior,
		// not modify original behavior.  Therefore, it should return false for all
		// keys that the original pDelegate handles.  It should return true only when
		// it handles a new key that's not handled by the traditional LuaProxyVTable hierarchy.
		const LuaProxyVTable *extensions = proxy->GetExtensionsDelegate();
		if ( extensions )
		{
			didSetValue = extensions->SetValueForKey( L, *object, key, 3 );
		}
#endif

		// Set value in LuaProxy for valid keys
		if ( ! didSetValue )
		{
			didSetValue = pDelegate->SetValueForKey( L, *object, key, 3 );
		}

		if ( ! didSetValue )
		{
			// TODO: Check whether key is available, i.e. not used
			// by the delegate as in ValueForKey().  To be efficient,
			// this requires a perfect hash:
			// 
			// if ( Rtt_VERIFY( pDelegate->IsKeyAvailable( key ) ) )

			// Otherwise set it in the Lua table
			lua_pushvalue( L, 2 ); // key
			lua_pushvalue( L, 3 ); // value
			lua_rawset( L, 1 ); // table[key] = value
		}
	}

	return 0;
}

int
LuaProxy::__proxyregister( lua_State *L )
{
	bool result = Rtt_VERIFY( lua_isstring( L, 1 ) )
				  && Rtt_VERIFY( lua_istable( L, 2 ) );
	if ( result )
	{
		// TODO: Put these in a single table that lives in the global registry
		// instead of the global registry itself

		#ifdef Rtt_DEBUG
			lua_pushvalue( L, 1 );
			lua_gettable( L, LUA_REGISTRYINDEX );
			Rtt_ASSERT( lua_isnil( L, -1 ) );
			lua_pop( L, 1 );
		#endif
		lua_settable( L, LUA_REGISTRYINDEX );
	}

	lua_pushboolean( L, result );
	return 1;
}

/*
int
LuaProxy::__proxylen( lua_State *L )
{
	LuaProxy* proxy = LuaProxy::GetProxy( L, 1 ); // proxy = table.proxy_
	return ( Rtt_VERIFY( proxy ) ? proxy->Delegate().Length( L ) : 0 );
}
*/
int
LuaProxy::__gcMeta( lua_State *L )
{
// Rtt_TRACE( ( "__gcMeta( %x )\n", lua_topointer( L, 1 ) ) );
	LuaProxy* p = GetProxyMeta( L, 1 );

	if ( Rtt_VERIFY( p ) )
	{
		p->Release();
		Rtt_DELETE( p );
	}

	return 0;
}

void
LuaProxy::Initialize( lua_State *L )
{
	luaL_newmetatable( L, kProxyKey );
	luaL_register( L, NULL, kMetatable );
	lua_pop( L, 1 );
}

// Need to wrap up Initialize into lua_pcall
//#define PROXY_SHARED_ENV

#ifdef PROXY_SHARED_ENV
static int
SetSharedEnvironment( lua_State *L )
{
	lua_pushstring( L, kInstanceTablesKey );
	lua_gettable( L, LUA_REGISTRYINDEX );

	if ( lua_isnil( L, -1 ) )
	{
		lua_pop( L, 1 ); // pop nil

		lua_newtable( L ); // push new table

		lua_pushstring( L, kInstanceTablesKey ); // push key
		lua_pushvalue( L, -2 ); // push value (extra ref of table)
		lua_settable( L, LUA_REGISTRYINDEX ); // table[key] = value (pops key and value)
	}
	Rtt_ASSERT( lua_istable( L, -1 ) );
	lua_replace( L, LUA_ENVIRONINDEX );

	return 0;
}
#endif // PROXY_SHARED_ENV


LuaProxy::LuaProxy( lua_State *L, MLuaProxyable& object, const LuaProxyVTable& delegate, const char* className )
:	fObject( & object ),
	fDelegate( delegate ),
#ifdef Rtt_PHYSICS
	fExtensionsDelegate( NULL ),
#endif
	fTableRef( LUA_NOREF )
{
	LuaProxy** userdata = (LuaProxy**)lua_newuserdata( L, sizeof(LuaProxy*) );
	* userdata = this;

	// Set userdata's metatable = mt
	luaL_getmetatable( L, kProxyKey ); // Fetch mt
	lua_setmetatable( L, -2 ); // implicitly pops mt

	Rtt_ASSERT( lua_isuserdata( L, -1 ) );

	CreateTable( L, className );

	Rtt_ASSERT( lua_isuserdata( L, -1 ) );
	lua_pop( L, 1 ); // pop userdata
}

LuaProxy::~LuaProxy()
{
//	luaL_unref( L, LUA_REGISTRYINDEX, TableRef() );
}

void
LuaProxy::CreateTable( lua_State *L, const char* className )
{
	Rtt_ASSERT( lua_isuserdata( L, -1 ) );
	lua_newtable( L ); // t

	AcquireTableRef( L );

	// t[proxy_] = userdata (this)
	Rtt_ASSERT( lua_isuserdata( L, -2 ) );
	lua_pushvalue( L, -2 );
	lua_setfield( L, -2, kProxyFieldKey );

	// t[class_] = LUA_REGISTRY[className]
	lua_getfield( L, LUA_REGISTRYINDEX, className ); Rtt_ASSERT( ! lua_isnil( L, -1 ) );
	lua_setfield( L, -2, kClassFieldKey );

	// setmetatable( t, Runtime._proxy )
	lua_getglobal( L, "Runtime" );
	lua_getfield( L, -1, kProxyFieldKey );
	lua_setmetatable( L, -3 );
	lua_pop( L, 1 ); // pop Runtime

	lua_pop( L, 1 ); // pop t
}

void
LuaProxy::RestoreTable( lua_State *L )
{
	int index = TableRef();

	// Restore table, stripping away all the proxy baggage
	if ( Rtt_VERIFY( LUA_NOREF != index ) )
	{
		// push t on top of stack
		PushTable( L );

		// t[proxy_] = nil
		lua_pushnil( L );
		lua_setfield( L, -2, kProxyFieldKey );

		// t[class_] = nil
		lua_pushnil( L );
		lua_setfield( L, -2, kClassFieldKey );

		// setmetatable( t, nil )
		lua_pushnil( L );
		lua_setmetatable( L, -2 );

		lua_pop( L, 1 ); // pop t
	}

	// Release proxy's reference to the MLuaProxyable (e.g. the DisplayObject)
	Release();
}

void
LuaProxy::Release()
{
	if ( fObject ) { fObject->ReleaseProxy(); }
}

void
LuaProxy::AcquireTableRef( lua_State *L )
{
	// TODO: Use environindex instead of global registry...
	// Assign a Lua ref to table at top of stack
	if ( LUA_NOREF == fTableRef )
	{
		Rtt_ASSERT( lua_istable( L, -1 ) );

		// Rtt_TRACE( ( "acquire table ref(%x)\n", lua_topointer( L, -1 ) ) );
		lua_pushvalue( L, -1 );
		fTableRef = luaL_ref( L, LUA_REGISTRYINDEX ); // implicitly pops extra table
		Rtt_ASSERT( LUA_NOREF != fTableRef );
	}
}

void
LuaProxy::ReleaseTableRef( lua_State *L )
{
	luaL_unref( L, LUA_REGISTRYINDEX, TableRef() );
	fTableRef = LUA_NOREF;
}

bool
LuaProxy::PushTable( lua_State *L ) const
{
	const int index = TableRef();
	Rtt_ASSERT( LUA_NOREF != index );

	lua_rawgeti( L, LUA_REGISTRYINDEX, index ); Rtt_ASSERT( ! lua_isnil( L, -1 ) );

	// Always return what's on the top of stack
	return true;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
