//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////


#include "Core/Rtt_Build.h"
#include "Rtt_AndroidStore.h"
#include "Rtt_LuaContext.h"
#include "NativeToJavaBridge.h"


// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

/// Creates a new store accessing object for Android.
/// @param handle The Lua state wrapper needed to create objects in the runtime and raise event in Lua.
AndroidStore::AndroidStore( const ResourceHandle<lua_State>& handle, NativeToJavaBridge *ntjb )
:	PlatformStore( handle ), fNativeToJavaBridge(ntjb)
{
}

/// Destructor. Cleans up allocated resources.
AndroidStore::~AndroidStore()
{
}

/// Sets the type of store to be used such as Google Marketplace, Amazon, etc.
/// @param storeName The unique name of the store to be used such as "google".
///                  Set to NULL or empty string to have Corona attempt to select a default store.
void AndroidStore::UseStore( const char *storeName )
{
	fNativeToJavaBridge->StoreInit(storeName);
}

/// Enables store transaction events to be sent to the Lua listener.
void AndroidStore::StartObservingTransactions()
{
}

/// Disables store transaction events from being sent to the Lua listener.
void AndroidStore::StopObservingTransactions()
{
}

/// Fetches all purchased products from the store.
/// Note: This is not currently supported on Android.
void AndroidStore::LoadProducts( lua_State *L, int index )
{
}

/// Sends a purchase request to the store.
/// @param L The Lua state providing a table or array of unique product names to be purchased. Cannot be NULL.
/// @param index Index to the Lua state where parameters were provided in the store.purchase() function in Lua.
void AndroidStore::Purchase( lua_State *L, int index )
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
		lua_rawgeti( L, index, productIndex + 1);
		if (lua_istable(L, -1))
		{
			// We've received a table for the product. Fetch the table's product ID.
			lua_getfield(L, -1, "productIdentifier");
			const char *productName = lua_tostring(L, -1);
			if (productName)
			{
				fNativeToJavaBridge->StorePurchase(productName);
			}
			lua_pop( L, 1 );
		}
		else if (lua_isstring(L, -1))
		{
			// We've received a product ID string.
			const char *productName = lua_tostring(L, -1);
			if (productName)
			{
				fNativeToJavaBridge->StorePurchase(productName);
			}
		}
		lua_pop(L, 1);
	}
}

/// Determines if the currently selected store can make purchases and the current user is authorized to make purchases.
/// Note: This is currenty locked to true until we can find a nice way to retrieve this value.
/// @return Returns true if store.purchase() requests are allowed. Returns false if not.
bool AndroidStore::CanMakePurchases() const
{
	return true;
}

/// Determines if the currently selected store can load previously purchased products.
/// Note: This is currenty locked to false since Google does not support this.
///       Will remain this way until we have an Android in-app purchasing system that supports it.
/// @return Returns true if store.loadProducts() will work with the currently selected store.
///         Returns false if that function will do nothing.
bool AndroidStore::CanLoadProducts() const
{
	return false;
}

/// Sends a confirmation to the store to finish a purchase or restore transaction.
/// @param transaction Pointer to the transaction object to be confirmed. Cannot be NULL.
void AndroidStore::FinishTransaction( PlatformStoreTransaction *transaction )
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
	fNativeToJavaBridge->StoreFinishTransaction(transactionStringId);
}

/// Sends a "restore" request to the store for fetching all previously purchased products.
void AndroidStore::RestoreCompletedTransactions()
{
	fNativeToJavaBridge->StoreRestoreCompletedTransactions();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
