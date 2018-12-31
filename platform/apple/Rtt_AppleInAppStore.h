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
		virtual int GetLength() const;
		virtual const char* GetTitle( int index ) const;
		virtual const char* GetDescription( int index ) const;
		virtual const char* GetProductIdentifier( int index ) const;
		virtual double GetPrice( int index ) const;
		virtual Rtt_STRONG const char* GetLocalizedPrice( int index ) const;
		virtual const char* GetPriceLocaleIdentifier( int index ) const override;

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
