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

#ifndef _Rtt_PlatformStore_H__
#define _Rtt_PlatformStore_H__

#include "Core/Rtt_Build.h"
#include "Core/Rtt_String.h"
#include "Core/Rtt_Array.h"
#include "Core/Rtt_ResourceHandle.h"
#include "Rtt_Event.h"
#include "Rtt_PlatformNotifier.h"


// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class PlatformProductList
{
	public:
		typedef PlatformProductList Self;

	public:	
		virtual ~PlatformProductList() = 0;

	public:
		virtual int GetLength() const = 0;
		virtual const char* GetTitle( int index ) const = 0;
		virtual const char* GetDescription( int index ) const = 0;
		virtual const char* GetProductIdentifier( int index ) const = 0;
		virtual double GetPrice( int index ) const = 0;
		virtual Rtt_STRONG const char* GetLocalizedPrice( int index ) const = 0;
		virtual const char* GetPriceLocaleIdentifier( int index ) const = 0;

};

// ----------------------------------------------------------------------------

// Class used to wrap native transaction object if necessary.
// Platforms should subclass this as necessary
class PlatformStoreTransaction
{
	public:
		enum State
		{
			kTransactionStateUndefined = 0,
			kTransactionStatePurchasing, // not currently used
			kTransactionStatePurchased,
			kTransactionStateFailed,
			kTransactionStateRestored,
			kTransactionStateCancelled,
			kTransactionStateRefunded,
			kNumTransactionStates
		};
		enum ErrorType
		{
			kTransactionErrorNone = 0,
			kTransactionErrorUnknown,
			kTransactionErrorClientInvalid,
			kTransactionErrorPaymentCancelled,
			kTransactionErrorPaymentInvalid,
			kTransactionErrorPaymentNotAllowed,
			kTransactionErrorRestoreFailed, // Note: This is an invented value which does not have a mapping to an Apple counterpart (unlike the others)
			kNumTransactionErrors
		};

	public:
		static const char* StringForState( State s );
		static const char* StringForErrorType( ErrorType t );

	public:
		virtual ~PlatformStoreTransaction();

	public:
		virtual State GetState() const = 0;
		virtual ErrorType GetErrorType() const = 0;
		virtual const char* GetErrorString() const = 0;
		virtual const char* GetProductIdentifier() const = 0;
		virtual const char* GetReceipt() const = 0;
		virtual const char* GetSignature() const = 0;
		virtual const char* GetIdentifier() const = 0;
		virtual const char* GetDate() const = 0;
		virtual const char* GetOriginalReceipt() const = 0;
		virtual const char* GetOriginalIdentifier() const = 0;
		virtual const char* GetOriginalDate() const = 0;
};

// ----------------------------------------------------------------------------

// Local event
class ProductListEvent : public VirtualEvent
{
	public:
		typedef VirtualEvent Super;
		typedef ProductListEvent Self;

	public:
		static const char kName[];

	public:
		ProductListEvent( PlatformProductList *productList );
		virtual ~ProductListEvent();
		
	public:
		virtual const char* Name() const;

		// Pushes all products in fProductList into the event table
		virtual int Push( lua_State *L ) const;

	private:
		PlatformProductList *fProductList;
};

// ----------------------------------------------------------------------------

class StoreTransactionEvent : public VirtualEvent
{
	public:
		typedef VirtualEvent Super;
		typedef StoreTransactionEvent Self;

	public:
		static const char kName[];
		
	public:
		StoreTransactionEvent( PlatformStoreTransaction *transaction );
		virtual ~StoreTransactionEvent();

	public:
		virtual const char* Name() const;
		virtual int Push( lua_State *L ) const;

	protected:
		mutable PlatformStoreTransaction *fTransaction;
};

// ----------------------------------------------------------------------------

class PlatformStore
{
	public:
		enum StoreFront
		{
			kAppleAppStore = 0,
			kUrbanAirShip,
		};

	public:
		PlatformStore( const ResourceHandle<lua_State> & handle );
		virtual ~PlatformStore();
		
	public:
		PlatformNotifier& GetTransactionNotifier() { return fTransactionNotifier; }
		PlatformNotifier& GetProductListNotifier() { return fProductListNotifier; }

	public:
		virtual void StartObservingTransactions();
		virtual void StopObservingTransactions();
		virtual void LoadProducts( lua_State *L, int index );
		virtual void Purchase( lua_State *L, int index );
		virtual bool CanMakePurchases() const;
		virtual bool CanLoadProducts() const;
		virtual void FinishTransaction( PlatformStoreTransaction* transaction );
		virtual void RestoreCompletedTransactions();

	protected:
		PlatformNotifier fTransactionNotifier;
		PlatformNotifier fProductListNotifier;
};

// ----------------------------------------------------------------------------

class PlatformStoreProvider
{
	public:
		PlatformStoreProvider( const ResourceHandle<lua_State> &handle );
		virtual ~PlatformStoreProvider();
		
		virtual void SetActiveStore( const char *storeName );
		virtual PlatformStore* GetActiveStore();
		bool HasActiveStore();
		bool IsStoreAvailable( const char *storeName );
		PtrArray<String>& GetAvailableStores() { return fAvailableStoreNames; };
		virtual const char* GetTargetedStoreName();
		
	protected:
		const ResourceHandle<lua_State>& GetLuaHandle() const { return fLuaState; }
		
	private:
		ResourceHandle<lua_State> fLuaState;
		PtrArray<String> fAvailableStoreNames;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_PlatformStore_H__
