//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_AppleInAppStore_H__
#define _Rtt_AppleInAppStore_H__

#include "Rtt_PlatformInAppStore.h"

@class AppleStoreManager;
@class NSArray;
@class SKProductsResponse;

// ----------------------------------------------------------------------------

namespace Rtt
{

class Runtime;

// ----------------------------------------------------------------------------

class AppleProductList : public PlatformProductList
{
	public:
		AppleProductList( SKProductsResponse *response );
		virtual ~AppleProductList();

	public:
		virtual int GetLength() const  override;
		virtual const char* GetTitle( int index ) const override;
		virtual const char* GetDescription( int index ) const override;
		virtual const char* GetProductIdentifier( int index ) const override;
		virtual double GetPrice( int index ) const override;
		virtual Rtt_STRONG const char* GetLocalizedPrice( int index ) const override;
		virtual const char* GetPriceLocaleIdentifier( int index ) const override;
		virtual const char* GetPriceLocaleCurrencyCode( int index ) const override;

	private:
		NSArray *fProducts; // array of SKProduct objects
};

// ----------------------------------------------------------------------------

class AppleProductListEvent : public ProductListEvent
{
	public:
		typedef ProductListEvent Super;
		typedef AppleProductListEvent Self;
		
	public:
		AppleProductListEvent( AppleProductList *productList, NSArray *invalidProductIdentifiers );
		virtual ~AppleProductListEvent();

	public:
		virtual int Push( lua_State *L ) const;

	private:
		NSArray *fInvalidProductIdentifiers;		
};

// ----------------------------------------------------------------------------

class AppleStore : public PlatformStore
{
	public:
		typedef PlatformStore Super;

	public:
		AppleStore( const ResourceHandle<lua_State>& handle );
		virtual ~AppleStore();

	public:
		virtual void StartObservingTransactions();
		virtual void StopObservingTransactions();
		virtual void LoadProducts( lua_State *L, int index );
		virtual void Purchase( lua_State *L, int index );
		virtual bool CanMakePurchases() const;
		virtual bool CanLoadProducts() const;
		virtual void FinishTransaction( PlatformStoreTransaction *transaction );
		virtual void RestoreCompletedTransactions();

	protected:
		// TODO: Refactor to allow multiple store backends
		AppleStoreManager* fStoreManager;
};

// ----------------------------------------------------------------------------

class AppleStoreProvider : public PlatformStoreProvider
{
	public:
		AppleStoreProvider( const ResourceHandle<lua_State>& handle );
		virtual ~AppleStoreProvider();
		
		virtual void SetActiveStore( const char *storeName );
		virtual PlatformStore* GetActiveStore();
		virtual const char* GetTargetedStoreName();
		
	private:
		AppleStore *fActiveStore;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AppleInAppStore_H__
