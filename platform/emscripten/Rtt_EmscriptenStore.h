//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_PlatformInAppStore.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class EmscriptenStore : public PlatformStore
{
	public:
		EmscriptenStore(const ResourceHandle<lua_State>& handle);
		virtual ~EmscriptenStore();

	public:
		void UseStore(const char *storeName);
		void StartObservingTransactions();
		void StopObservingTransactions();
		void LoadProducts(lua_State *L, int index);
		void Purchase(lua_State *L, int index);
		bool CanMakePurchases() const;
		bool CanLoadProducts() const;
		void FinishTransaction(PlatformStoreTransaction *transaction);
		void RestoreCompletedTransactions();
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
