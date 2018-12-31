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

#include "Rtt_AppleInAppStore.h"
#import "Rtt_AppleInAppStoreApple.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Runtime.h"
#include "Core/Rtt_String.h"

#import <Foundation/Foundation.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

AppleProductList::AppleProductList( SKProductsResponse *response )
:	fProducts( [response.products retain] )
{
}

AppleProductList::~AppleProductList()
{
	[fProducts release];
}

int
AppleProductList::GetLength() const
{
	return (int)[fProducts count];
}

const char*
AppleProductList::GetTitle( int index ) const
{
	SKProduct *product = [fProducts objectAtIndex:index];
	return [product.localizedTitle UTF8String];
}

const char*
AppleProductList::GetDescription( int index ) const
{
	SKProduct *product = [fProducts objectAtIndex:index];
	return [product.localizedDescription UTF8String];
}

const char*
AppleProductList::GetProductIdentifier( int index ) const
{
	SKProduct *product = [fProducts objectAtIndex:index];
	return [product.productIdentifier UTF8String];
}

double
AppleProductList::GetPrice( int index ) const
{
	SKProduct *product = [fProducts objectAtIndex:index];
	return ( [product.price doubleValue] );
}

const char*
AppleProductList::GetPriceLocaleIdentifier( int index ) const
{
	SKProduct* product = [fProducts objectAtIndex:index];
	return [[[product priceLocale] objectForKey:NSLocaleIdentifier] UTF8String];
}
	
Rtt_STRONG const char*
AppleProductList::GetLocalizedPrice( int index ) const
{
	SKProduct* product = [fProducts objectAtIndex:index];
	NSNumberFormatter* formatter = [[NSNumberFormatter alloc] init];
	[formatter setFormatterBehavior:NSNumberFormatterBehavior10_4];
	[formatter setNumberStyle:NSNumberFormatterCurrencyStyle];
	[formatter setLocale:product.priceLocale];
	NSString* localizedstring = [formatter stringFromNumber:product.price];
	[formatter release];
	return [localizedstring UTF8String];
}

// ----------------------------------------------------------------------------

AppleProductListEvent::AppleProductListEvent( AppleProductList *productList, NSArray *invalidProductIdentifiers )
:	Super( productList ),
	fInvalidProductIdentifiers( [invalidProductIdentifiers retain] )
{
}

AppleProductListEvent::~AppleProductListEvent()
{
	[fInvalidProductIdentifiers release];
}

int
AppleProductListEvent::Push( lua_State *L ) const
{
	if ( Rtt_VERIFY( Super::Push( L ) ) )
	{
		// Next, we need a field called invalidProducts for those nasty failure conditions.
		// Again, this will be an array, but this time only an array of strings.
		lua_newtable( L );
		for( int i = 0, iMax = (int)[fInvalidProductIdentifiers count]; i < iMax; i++ )
		{
			// Each element in the array is going to be a string
			/*
			 for i=1, #event.products do
				print(event.invalidProducts[i])
			 end
			 */
			NSString *identifier = (NSString*)[fInvalidProductIdentifiers objectAtIndex:i];
			Rtt_ASSERT( [identifier isKindOfClass:[NSString class]] );
			lua_pushstring( L, [identifier UTF8String] );
			lua_rawseti( L, -2, i+1); // add 1 for arrays start at index 1 in Lua
		}
		lua_setfield( L, -2, "invalidProducts" );

		if(!fInvalidProductIdentifiers)
		{
			lua_pushboolean(L, true);
			lua_setfield(L, -2, kIsErrorKey);
		}
	}

	return 1;
}

// ----------------------------------------------------------------------------

AppleStore::AppleStore( const ResourceHandle<lua_State>& handle )
:	Super( handle )
{
	// TODO: Support multiple store backends
	fStoreManager = [[AppleStoreManager alloc] initWithOwner:this];
}

AppleStore::~AppleStore()
{
	[fStoreManager release];
}

void
AppleStore::StartObservingTransactions()
{
	[fStoreManager startObservingTransactions];
}

void
AppleStore::StopObservingTransactions()
{
	[fStoreManager stopObservingTransactions];
}

void
AppleStore::LoadProducts( lua_State *L, int index )
{
	if ( lua_istable( L, index )	)
	{
		int length = (int)lua_objlen( L, index );
		if ( length > 0 )
		{
			NSMutableSet *products = [NSMutableSet setWithCapacity:length];
			for ( int i = 0; i < length; i++ )
			{
				lua_rawgeti( L, index, i + 1 );
				NSString* s = [[NSString alloc] initWithUTF8String:lua_tostring( L, -1 )];
				[products addObject:s];
				[s release];
				lua_pop( L, 1 );
			}

			[fStoreManager loadProducts:products];
		}
	}
}

void
AppleStore::Purchase( lua_State *L, int index )
{
	if ( lua_istable( L, index )	)
	{
		int length = (int)lua_objlen( L, index );
		if ( length > 0 )
		{
		    NSMutableArray* products = [NSMutableArray arrayWithCapacity:length];
			for ( int i = 0; i < length; i++ )
			{
				lua_rawgeti( L, index, i + 1 );
				
				// Check if a product table entry was passed in
				if( lua_istable( L, -1) )
				{
					// if so, we expect a key called "productIdentifier"
					lua_getfield( L, -1, "productIdentifier");
					NSString* s = [[NSString alloc] initWithUTF8String:lua_tostring( L, -1 )];
					[products addObject:s];
					[s release];
					lua_pop( L, 1 );
				}
				// If not a transaction object, then expect a product identifier string
				else if( lua_isstring( L, -1) )
				{
					NSString* s = [[NSString alloc] initWithUTF8String:lua_tostring( L, -1 )];
					[products addObject:s];
					[s release];
				}
				lua_pop( L, 1 );
			}

			[fStoreManager purchase:products];
		}
	}
	else if ( lua_type( L, index ) == LUA_TSTRING )
	{
		[fStoreManager purchase:@[[NSString stringWithUTF8String:lua_tostring( L, index )]]];
	}
}

bool
AppleStore::CanMakePurchases() const
{
	return [fStoreManager canMakePurchases];
}

bool
AppleStore::CanLoadProducts() const
{
	return true;
}

void
AppleStore::FinishTransaction( PlatformStoreTransaction *transaction )
{
	[fStoreManager finishTransaction:transaction];		
}

void
AppleStore::RestoreCompletedTransactions()
{
	[fStoreManager restoreCompletedTransactions];		
}

// ----------------------------------------------------------------------------

/// Unique name assigned to the iOS/Mac app store.
static const char* kAppleStoreName = "apple";

/// Creates a new object used to provide an appropriate store accessing object for the current platform.
/// @param handle The Lua state wrapper needed to create objects in the runtime and raise event in Lua.
AppleStoreProvider::AppleStoreProvider( const ResourceHandle<lua_State>& handle )
:	PlatformStoreProvider( handle ),
	fActiveStore( NULL )
{
	// Determine if the "apple" app store is available on this system.
	// If so, then add that store name to the "Available Stores" array.
	// (Note: We do not support in-app purchases within the simulator.)
#ifndef Rtt_AUTHORING_SIMULATOR
	Runtime *runtime = LuaContext::GetRuntime(GetLuaHandle().Dereference());
	if (runtime)
	{
		GetAvailableStores().Append(Rtt_NEW(runtime->Allocator(), String(runtime->Allocator(), kAppleStoreName)));
	}
#endif
}

/// Destructor. Cleans up allocated resources.
AppleStoreProvider::~AppleStoreProvider()
{
	Rtt_DELETE( fActiveStore );
}

/// Assign a store to the provider to be returned by the GetActiveStore() function.
/// @param storeName The unique name of the store to provide access to.
///                  Set to NULL or empty string to have the platform automatically select a default store, if possible.
void
AppleStoreProvider::SetActiveStore( const char *storeName )
{
	// Do not allow the store to be changed once one has been activated.
	if (fActiveStore)
	{
		return;
	}
	
	// Automatically select the Apple store if a name was not provided.
	if (!storeName || (strlen(storeName) < 1))
	{
		storeName = kAppleStoreName;
	}
	
	// Delete access to the last store, if assigned.
	Rtt_DELETE( fActiveStore );
	fActiveStore = NULL;
	
	// Create the specified store accessing object if available on this system/device.
	if (IsStoreAvailable(storeName))
	{
		Runtime *runtime = LuaContext::GetRuntime(GetLuaHandle().Dereference());
		if (runtime)
		{
			fActiveStore = Rtt_NEW(runtime->Allocator(), AppleStore(GetLuaHandle()));
		}
	}
}

/// Gets the store accessing object that was assigned to this provider via the SetActiveStore() function.
/// @return Returns a pointer to a store accessing object.
///         Returns NULL if a store was not assigned to this provider or if a store is not available on this platform.
PlatformStore*
AppleStoreProvider::GetActiveStore()
{
	return fActiveStore;
}

/// Gets the unique name of the store this application is targeting.
/// @return Returns the unique name of the targetd store such as "apple", "google", "amazon", etc.
///         Returns NULL if this application is not targeting a store.
const char*
AppleStoreProvider::GetTargetedStoreName()
{
	return kAppleStoreName;
}

// ----------------------------------------------------------------------------

} // namespace Rtt
