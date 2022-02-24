//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"
#include "Rtt_EmscriptenStore.h"
#include "Rtt_LuaContext.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

#pragma region Constructors/Destructors
/// Creates a new store accessing object.
/// @param handle The Lua state wrapper needed to create objects in the runtime and raise event in Lua.
EmscriptenStore::EmscriptenStore(const ResourceHandle<lua_State>& handle)
:	PlatformStore(handle)
{
}

/// Destructor. Cleans up allocated resources.
EmscriptenStore::~EmscriptenStore()
{
}

#pragma endregion


#pragma region Public Member Functions
/// Sets the type of store to be used.
/// @param storeName The unique name of the store to be used such as "windows".
///                  Set to NULL or empty string to have Corona attempt to select a default store.
void EmscriptenStore::UseStore(const char *storeName)
{
}

/// Enables store transaction events to be sent to the Lua listener.
void EmscriptenStore::StartObservingTransactions()
{
}

/// Disables store transaction events from being sent to the Lua listener.
void EmscriptenStore::StopObservingTransactions()
{
}

/// Fetches all purchased products from the store.
/// Note: This is not currently supported on Emscripten.
void EmscriptenStore::LoadProducts(lua_State *L, int index)
{
}

/// Sends a purchase request to the store.
/// @param L The Lua state providing a table or array of unique product names to be purchased. Cannot be NULL.
/// @param index Index to the Lua state where parameters were provided in the store.purchase() function in Lua.
void EmscriptenStore::Purchase(lua_State *L, int index)
{
	// Validate.
	if (!L || (index < 0))
	{
		return;
	}

	// Make sure that the given Lua state contains an array of products.
	if (!lua_istable(L, index))
	{
		return;
	}

	// Send a purchase request for every product in the Lua array.
	int productCount = lua_objlen(L, index);
	for (int productIndex = 0; productIndex < productCount; productIndex++)
	{
		lua_rawgeti(L, index, productIndex + 1);
		if (lua_istable(L, -1))
		{
			// We've received a table for the product. Fetch the table's product ID.
			lua_getfield(L, -1, "productIdentifier");
			const char *productName = lua_tostring(L, -1);
			if (productName)
			{
//TODO: Send the purchase request.
			}
			lua_pop(L, 1);
		}
		else if (lua_isstring(L, -1))
		{
			// We've received a product ID string.
			const char *productName = lua_tostring(L, -1);
			if (productName)
			{
//TODO: Send the purchase request.
			}
		}
		lua_pop(L, 1);
	}
}

/// Determines if the currently selected store can make purchases and the current user is authorized to make purchases.
/// Note: This is currenty locked to true until we can find a nice way to retrieve this value.
/// @return Returns true if store.purchase() requests are allowed. Returns false if not.
bool EmscriptenStore::CanMakePurchases() const
{
	return true;
}

/// Determines if the currently selected store can load previously purchased products.
/// Note: This is currenty locked to false since Google does not support this.
///       Will remain this way until we have an Emscripten in-app purchasing system that supports it.
/// @return Returns true if store.loadProducts() will work with the currently selected store.
///         Returns false if that function will do nothing.
bool EmscriptenStore::CanLoadProducts() const
{
	return false;
}

/// Sends a confirmation to the store to finish a purchase or restore transaction.
/// @param transaction Pointer to the transaction object to be confirmed. Cannot be NULL.
void EmscriptenStore::FinishTransaction(PlatformStoreTransaction *transaction)
{
	// Validate argument.
	if (!transaction)
	{
		return;
	}

	// Get the transaction's string ID and validate it.
	const char *transactionStringId = transaction->GetIdentifier();
	if (!transactionStringId || (strlen(transactionStringId) <= 0)) {
		return;
	}

	// Send the "finish transaction" request.
}

/// Sends a "restore" request to the store for fetching all previously purchased products.
void EmscriptenStore::RestoreCompletedTransactions()
{
}

#pragma endregion

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
