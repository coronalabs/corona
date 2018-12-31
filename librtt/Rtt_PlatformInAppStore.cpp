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
#include "Rtt_Runtime.h"

#include "Rtt_PlatformInAppStore.h"
#include "Rtt_LuaLibInAppStore.h"
#include "Rtt_LuaContext.h"


// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

PlatformProductList::~PlatformProductList()
{
}

// ----------------------------------------------------------------------------

const char kTransactionStateUndefinedString[] = "undefined";
const char kTransactionStatePurchasedString[] = "purchased";
const char kTransactionStateFailedString[] = "failed";
const char kTransactionStateRestoredString[] = "restored";
const char kTransactionStateCancelledString[] = "cancelled";
const char kTransactionStatePurchasingString[] = "purchasing";
const char kTransactionStateRefundedString[] = "refunded";

const char*
PlatformStoreTransaction::StringForState( State s )
{
	const char *result = kTransactionStateUndefinedString;
	switch ( s )
	{
		case kTransactionStatePurchased:
		{
			result = kTransactionStatePurchasedString;
			break;
		}
		case kTransactionStateFailed:
		{
			result = kTransactionStateFailedString;
			break;
		}
		case kTransactionStateRestored:
		{
			result = kTransactionStateRestoredString;
			break;
		}
		case kTransactionStateCancelled:
		{
			result = kTransactionStateCancelledString;
			break;
		}
		case kTransactionStatePurchasing:
		{
			// Note: Currently not used
			result = kTransactionStatePurchasingString;
			break;
		}
		case kTransactionStateRefunded:
		{
			result = kTransactionStateRefundedString;
			break;
		}
		default:
		{
			break;			
		}
	}

	return result;
}


const char kTransactionErrorNoneString[] = "none";
const char kTransactionErrorUnknownString[] = "unknown";
const char kTransactionErrorPaymentCancelledString[] = "cancelled"; // Note: We promoted cancelled to a full 'state', but it may be possible that this state still sneaks through the API such as in some kind of unexpected error condition. So I leave the constant available.
const char kTransactionErrorClientInvalidString[] = "invalidClient";
const char kTransactionErrorPaymentInvalidString[] = "invalidPayment";
const char kTransactionErrorPaymentNotAllowedString[] = "paymentNotAllowed";
const char kTransactionErrorRestoreFailedString[] = "restoreFailed";

const char*
PlatformStoreTransaction::StringForErrorType( ErrorType t )
{
	const char *result = kTransactionErrorUnknownString;

	switch( t )
	{
		case kTransactionErrorNone:
		{
			result = kTransactionErrorNoneString;
			break;
		}
		case kTransactionErrorPaymentCancelled:
		{
			result = kTransactionErrorPaymentCancelledString;
			break;
		}
		case kTransactionErrorClientInvalid:
		{
			result = kTransactionErrorClientInvalidString;
			break;
		}
		case kTransactionErrorPaymentInvalid:
		{
			result = kTransactionErrorPaymentInvalidString;
			break;
		}
		case kTransactionErrorPaymentNotAllowed:
		{
			result = kTransactionErrorPaymentNotAllowedString;
			break;
		}
		case kTransactionErrorRestoreFailed:
		{
			result = kTransactionErrorRestoreFailedString;
			break;
		}
		default:
		{
			break;			
		}
	}

	return result;
}

PlatformStoreTransaction::~PlatformStoreTransaction() 
{
}

// ----------------------------------------------------------------------------

const char ProductListEvent::kName[] = "productList";

ProductListEvent::ProductListEvent( PlatformProductList *productList )
:	fProductList( productList )
{
}

ProductListEvent::~ProductListEvent()
{
	Rtt_DELETE( fProductList );
}

const char*
ProductListEvent::Name() const
{
	return Self::kName;
}

int
ProductListEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		// We're going to make a field called event.products which is an array
		lua_newtable( L );

		PlatformProductList *products = fProductList;
		if(products) for( int i = 0, iMax = products->GetLength(); i < iMax; i++ )
		{
			// Each element in the products array is going to be a table containing
			// the the following fields:
			// 
			// 	for i=1, #event.products do
			// 		print(event.products[i].title)
			// 		print(event.products[i].description)
			// 		print(event.products[i].price)
			// 		print(event.products[i].productIdentifier)
			// 	end
			lua_newtable( L );

			lua_pushstring( L, products->GetTitle( i ) );
			lua_setfield( L, -2, "title" );
			
			lua_pushstring( L, products->GetDescription( i ) );
			lua_setfield( L, -2, "description" );
			
			lua_pushnumber( L, products->GetPrice( i ) );
			lua_setfield( L, -2, "price" );

			lua_pushstring( L, products->GetLocalizedPrice( i ) );
			lua_setfield( L, -2, "localizedPrice" );
						
			lua_pushstring( L, products->GetProductIdentifier( i ) );
			lua_setfield( L, -2, "productIdentifier" );

			lua_pushstring( L, products->GetPriceLocaleIdentifier( i ) );
			lua_setfield( L, -2, "priceLocale" );
			
			lua_rawseti( L, -2, i+1); // add 1 for arrays start at index 1 in Lua
		}
		lua_setfield( L, -2, "products" );
	}
	return 1;
}

// ----------------------------------------------------------------------------

const char StoreTransactionEvent::kName[] = "storeTransaction";

StoreTransactionEvent::StoreTransactionEvent( PlatformStoreTransaction *transaction )
:	fTransaction( transaction )
{
}

StoreTransactionEvent::~StoreTransactionEvent()
{
	// Just in case the event is never dispatched
	Rtt_DELETE( fTransaction );
}

const char*
StoreTransactionEvent::Name() const
{
	return Self::kName;
}

int
StoreTransactionEvent::Push( lua_State *L ) const
{
	// Check for additional calls
	if ( Rtt_VERIFY( fTransaction ) )
	{
		if ( Rtt_VERIFY( Super::Push( L ) ) )
		{
			// event.transaction

			Lua::PushUserdata( L, fTransaction, LuaLibStore::kTransactionMetaTableName );
			fTransaction = NULL; // Lua owns the transaction now

			lua_setfield( L, -2, "transaction" );
		}
	}

	return 1;
}

// ----------------------------------------------------------------------------

PlatformStore::PlatformStore( const ResourceHandle<lua_State> & handle )
:	fTransactionNotifier( handle, true ),
	fProductListNotifier( handle )
{
}

PlatformStore::~PlatformStore()
{
}

void
PlatformStore::StartObservingTransactions()
{
	// Default implementation
}

void
PlatformStore::StopObservingTransactions()
{
	// Default implementation
}

void
PlatformStore::LoadProducts( lua_State *L, int index )
{
	Rtt_TRACE_SIM( ( "WARNING: store.loadProducts() is only supported on device.\n" ) );
	Rtt_ASSERT_NOT_IMPLEMENTED();
}

void
PlatformStore::Purchase( lua_State *L, int index )
{
	Rtt_TRACE_SIM( ( "WARNING: store.purchase() is only supported on device.\n" ) );
	Rtt_ASSERT_NOT_IMPLEMENTED();
}

bool
PlatformStore::CanMakePurchases() const
{
	return false;
}

bool
PlatformStore::CanLoadProducts() const
{
	return false;
}

void
PlatformStore::FinishTransaction( PlatformStoreTransaction* transaction )
{
	Rtt_TRACE_SIM( ( "WARNING: store.finishTransaction() is only supported on device.\n" ) );
	Rtt_ASSERT_NOT_IMPLEMENTED();
}

void
PlatformStore::RestoreCompletedTransactions()
{
	Rtt_TRACE_SIM( ( "WARNING: store.restore() is only supported on device.\n" ) );
	Rtt_ASSERT_NOT_IMPLEMENTED();
}

// ----------------------------------------------------------------------------

/// Creates a new object used to provide an appropriate store accessing object for the current platform.
/// @param handle The Lua state wrapper needed to create objects in the runtime and raise event in Lua.
PlatformStoreProvider::PlatformStoreProvider( const ResourceHandle<lua_State> & handle )
:	fLuaState( handle ),
	fAvailableStoreNames( LuaContext::GetRuntime(GetLuaHandle().Dereference())->Allocator() )
{
}

/// Destructor. Cleans up allocated resources.
PlatformStoreProvider::~PlatformStoreProvider()
{
}

/// Assign a store to the provider to be returned by its GetActiveStore() function.
/// This base implementation ignores this call, causing GetActiveStore() to continue to return NULL.
/// It is up to the derived class to override this behavior and assign this provider a real store object.
/// @param storeName The unique name of the store to provide access to.
///                  Set to NULL or empty string to have the platform automatically select a default store, if possible.
void
PlatformStoreProvider::SetActiveStore( const char *storeName )
{
}

/// Gets the store accessing object that was assigned to this provider via the SetActiveStore() function.
/// @return Returns a pointer to a store accessing object.
///         Returns NULL if a store was not assigned to this provider or if a store is not available on this platform.
PlatformStore*
PlatformStoreProvider::GetActiveStore()
{
	return NULL;
}

/// Determines if this provider has been assigned a store.
/// @return Returns true if this provider's GetActiveStore() will return a valid store object. Returns false if not.
bool
PlatformStoreProvider::HasActiveStore()
{
	return (GetActiveStore() != NULL);
}

/// Determines if the given store is available on the current platform.
/// @param storeName The unique name of the store to check for.
/// @return Returns true if the store is available and can be made the active store.
///         Returns false if the store is not available or if given an unknown or invalid store name.
bool
PlatformStoreProvider::IsStoreAvailable( const char *storeName )
{
	// Do not continue if given a NULL or empty string.
	if (!storeName || (strlen(storeName) < 1))
	{
		return false;
	}
	
	// Check if the given store name exists in the stored "available stores" array.
	int count = fAvailableStoreNames.Length();
	for (int index = 0; index < count; index++)
	{
		if (strcmp(storeName, fAvailableStoreNames[index]->GetString()) == 0)
		{
			// Store name found in the array. This means it is available/accessible.
			return true;
		}
	}
	
	// Store name not found.
	return false;
}

/// Gets the unique name of the store this application is targeting.
/// @return Returns the unique name of the targetd store such as "apple", "google", "amazon", etc.
///         Returns NULL if this application is not targeting a store.
const char*
PlatformStoreProvider::GetTargetedStoreName()
{
	return NULL;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
