//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_LuaLibInAppStore.h"

#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_MPlatform.h"
#include "Rtt_Runtime.h"
#include "Core/Rtt_String.h"
#include "Rtt_PlatformInAppStore.h"
#include "CoronaLua.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

/// Fetches the currently selected store that was set via store.init() from the given runtime object.
/// Logs a warning if a store could not be provided by this function.
/// @param runtime Pointer to the runtime object to fetch the store from.
/// @return Returns a pointer to the currently selected/active store.
///         Returns NULL if a store has not been selected via store.init() in Lua or if the platform does not support in-app purchases.
static PlatformStore*
GetStoreFrom( lua_State* L )
{
    Runtime *runtime = LuaContext::GetRuntime( L );
	PlatformStore *store = NULL;
	
	if (runtime)
	{
		PlatformStoreProvider *storeProvider = runtime->Platform().GetStoreProvider(runtime->VMContext().LuaState());
		if (storeProvider)
		{
			store = storeProvider->GetActiveStore();
			if (!store)
			{
				CoronaLuaError(L, "an available store has not been loaded by store.init()");
				Rtt_ASSERT_NOT_IMPLEMENTED();
			}
		}
		else
		{
			CoronaLuaWarning(L, "the store API is not supported on this platform");
			Rtt_ASSERT_NOT_IMPLEMENTED();
		}
	}
	return store;
}

// store.init( [type, ] transactionListener )
static int
init( lua_State* L )
{
	// Fetch the runtime's platform object.
	Runtime *runtime = LuaContext::GetRuntime( L );
	const MPlatform& platform = runtime->Platform();
	
	// Fetch the store provider.
	PlatformStoreProvider *storeProvider = platform.GetStoreProvider(runtime->VMContext().LuaState());
	if (!storeProvider)
	{
		return 0;
	}
	
	// Fetch the store name argument, if provided.
	int nextArg = 1;
	const char *storeName = NULL;
	if (lua_isstring( L, nextArg ))
	{
		storeName = lua_tostring( L, nextArg );
		++nextArg;
	}
	
	// Select and activate the specified store. Setting this to NULL tells the system to attempt to select a default store.
	storeProvider->SetActiveStore(storeName);
	PlatformStore* store = storeProvider->GetActiveStore();
	if (!store)
	{
		// The specified store is not available or the platform does not support in-app purchases.
		return 0;
	}
	
	// Get the transaction listener argument, if provided.
	// --------------------------------------------------------------------------------
	// Because Apple is aggressive about handling interrupted transactions, 
	// we should to setup the callbacks at the same time when we init.
	// --------------------------------------------------------------------------------
	if (Lua::IsListener( L, nextArg, StoreTransactionEvent::kName ))
	{
		store->GetTransactionNotifier().SetListenerRef( nextArg );
		store->StartObservingTransactions();
	}
	else
	{
		// Need to clear callback from previous calls
		store->StopObservingTransactions();
		store->GetTransactionNotifier().CleanupNotifier();			
	}
	
	return 0;
}

// store.loadProducts( identifiers, listener )
// Callback will return a list of items on the store with other information like pricing
// so a UI can be constructed and displayed
static int
loadProducts( lua_State* L )
{
	PlatformStore *store = GetStoreFrom(L);
	if (store)
	{
		// Handle the listner callback
		if( Lua::IsListener( L, 2, ProductListEvent::kName ) )
		{
			store->GetProductListNotifier().SetListenerRef( 2 );			
		}
		else
		{
			// Need to clear callback from previous calls
			store->GetProductListNotifier().CleanupNotifier();			
		}
		
		// Do this last so we can directly call LoadProducts asserting everything else has been setup
		if( lua_istable( L, 1 )	)
		{
			store->LoadProducts( L, 1 );
		}
        else
        {
            CoronaLuaError(L, "store.loadProducts() requires a product identifiers table");
        }
	}

	return 0;
}

static int
isActive( lua_State* L )
{
	PlatformStore *store = GetStoreFrom(L);
	lua_pushboolean( L, store ? 1 : 0 );
	return 1;
}

static int
canMakePurchases( lua_State* L )
{
	PlatformStore *store = GetStoreFrom(L);
	bool canMakePurchases = ( store ? store->CanMakePurchases() : false );
	lua_pushboolean( L, canMakePurchases ? 1 : 0 );
	return 1;
}

static int
canLoadProducts( lua_State* L )
{
	PlatformStore *store = GetStoreFrom(L);
	bool canLoadProducts = ( store ? store->CanLoadProducts() : false );
	lua_pushboolean( L, canLoadProducts ? 1 : 0 );
	return 1;
}

static int
purchase( lua_State* L )
{
	PlatformStore *store = GetStoreFrom(L);
	if (store)
	{
		if (lua_istable( L, 1 ) || lua_type(L, 1) == LUA_TSTRING)
		{
			store->Purchase( L, 1 );
		}
        else
        {
            CoronaLuaWarning(L, "store.purchase() expected a table as first argument (got %s)", lua_typename(L, lua_type(L, 1)));
        }
	}
	
	return 0;
}

static int
finishTransaction( lua_State* L )
{
	PlatformStore *store = GetStoreFrom(L);
	if (store)
	{
		PlatformStoreTransaction *transaction =
			(PlatformStoreTransaction*)Lua::CheckUserdata( L, 1, LuaLibStore::kTransactionMetaTableName );
		store->FinishTransaction( transaction );
	}
	
	return 0;	
}

static int
restoreCompletedTransactions( lua_State* L )
{
	PlatformStore *store = GetStoreFrom(L);
	if (store)
	{
		store->RestoreCompletedTransactions();
	}
	return 0;
}

static int
storeValueForKey( lua_State *L )
{
	const char *key = luaL_checkstring( L, 2 );		

	if ( 0 == strcmp( "isActive", key ) )
	{
		return isActive( L );
	}
	else if ( 0 == strcmp( "canMakePurchases", key ) )
	{
		return canMakePurchases( L );
	}
	else if ( 0 == strcmp( "canLoadProducts", key ) )
	{
		return canLoadProducts( L );
	}

	return 0;
}

static int
transactionValueForKey( lua_State *L )
{
	int result = 1;

	PlatformStoreTransaction *transaction =
		(PlatformStoreTransaction*)Lua::CheckUserdata( L, 1, LuaLibStore::kTransactionMetaTableName );

	const char *key = luaL_checkstring( L, 2 );		

	if ( 0 == strcmp( "state", key ) )  // read-only
	{
		lua_pushstring( L, PlatformStoreTransaction::StringForState( transaction->GetState() ) );
	}
	else if ( 0 == strcmp( "errorType", key ) )  // read-only
	{
		lua_pushstring( L, PlatformStoreTransaction::StringForErrorType( transaction->GetErrorType() ) );
	}
	else if ( 0 == strcmp( "errorString", key ) )  // read-only
	{
		lua_pushstring( L, transaction->GetErrorString() );
	}
	else if ( 0 == strcmp( "productIdentifier", key ) )  // read-only
	{
		lua_pushstring( L, transaction->GetProductIdentifier() );
	}
	else if ( 0 == strcmp( "receipt", key ) )  // read-only
	{
		lua_pushstring( L, transaction->GetReceipt() );
	}
	else if ( 0 == strcmp( "signature", key ) )  // read-only
	{
		lua_pushstring( L, transaction->GetSignature() );
	}
	else if ( 0 == strcmp( "identifier", key ) )  // read-only
	{
		lua_pushstring( L, transaction->GetIdentifier() );
	}
	else if ( 0 == strcmp( "date", key ) )  // read-only
	{
		lua_pushstring( L, transaction->GetDate() );
	}
	else if ( 0 == strcmp( "originalReceipt", key ) )  // read-only
	{
		lua_pushstring( L, transaction->GetOriginalReceipt() );
	}
	else if ( 0 == strcmp( "originalIdentifier", key ) )  // read-only
	{
		lua_pushstring( L, transaction->GetOriginalIdentifier() );
	}
	else if ( 0 == strcmp( "originalDate", key ) )  // read-only
	{
		lua_pushstring( L, transaction->GetOriginalDate() );
	}
	else
	{
		result = 0;
	}

	return result;
}

static int
gcStoreTransaction( lua_State* L )
{
	PlatformStoreTransaction *transaction =
		(PlatformStoreTransaction*)Lua::CheckUserdata( L, 1, LuaLibStore::kTransactionMetaTableName );
    
    if (transaction != NULL)
    {
        Rtt_DELETE( transaction );
    }

	return 0;
}

// ----------------------------------------------------------------------------

const char LuaLibStore::kTransactionMetaTableName[] = "store.transaction";

int
LuaLibStore::Open( lua_State* L )
{
	Runtime *runtime = LuaContext::GetRuntime( L );
	const MPlatform& platform = runtime->Platform();
	
	// Register all "store" functions.
	const luaL_Reg kVTable[] =
	{
		{ "init", init },
		{ "loadProducts", loadProducts },
		{ "purchase", purchase },
		{ "finishTransaction", finishTransaction },
		{ "restore", restoreCompletedTransactions },
//		{ "__index", valueForKeyForStore },
		
		{ NULL, NULL }
	};
	luaL_register( L, "store", kVTable );
	
	// Set up the "store" property handlers to be registerd into Lua.
	const luaL_Reg kStoreVTable[] = 
	{
		{ "__index", storeValueForKey },
		{ NULL, NULL }
	};
	
	// To provide access to queries via dot-notation (like store.canMakePurchases)
	// I need a metatable on the main audio table.
	// And that metatable needs to have the __index metamethod defined.
	luaL_register( L, "LuaLibStore", kStoreVTable );
	lua_setmetatable( L, -2 );
	
	// Create the store provider, if available.
	PlatformStoreProvider *storeProvider = platform.GetStoreProvider(runtime->VMContext().LuaState());

	// Fetch the "store" table so that we can add properties to it down below.
	lua_getglobal( L, "store" );

	// Add a "store.availableStores" table which indicates what stores are available on the system/device.
	lua_newtable( L );
	if (storeProvider)
	{
		String *stringPointer;
		for (int index = 0; index < storeProvider->GetAvailableStores().Length(); index++)
		{
			stringPointer = storeProvider->GetAvailableStores()[index];
			if ((stringPointer != NULL) && (stringPointer->IsEmpty() == false))
			{
				lua_pushboolean( L, 1 );
				lua_setfield( L, -2, stringPointer->GetString() );
			}
		}
	}
	lua_setfield( L, -2, "availableStores" );

	// Add a "store.target" property which indicates which store this application was built for.
	const char *storeName = storeProvider ? storeProvider->GetTargetedStoreName() : NULL;
	if (!storeName)
	{
		storeName = "none";
	}
	lua_pushstring( L, storeName );
	lua_setfield( L, -2, "target" );

	// We're now done adding properties to the "store" table. Pop it off of the stack.
	lua_pop( L, 1 );
	
	// Create a metatable for transaction object userdata wrappers
	const luaL_Reg kTransactionVTable[] =
	{
		{ "__index", transactionValueForKey },
		{"__gc", gcStoreTransaction },
		
		{ NULL, NULL }
	};

	Lua::InitializeMetatable( L, kTransactionMetaTableName, kTransactionVTable );

	// Note: Apple's store wants to do some immediate setup to handle purchase interruption cases
	// User should call store.init() early.
	return 1;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

