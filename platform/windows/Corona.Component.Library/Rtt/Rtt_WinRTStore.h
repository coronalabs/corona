// ----------------------------------------------------------------------------
// 
// Rtt_WinRTStore.h
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once

Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Rtt_PlatformInAppStore.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

class WinRTStore : public PlatformStore
{
	public:
		WinRTStore(const ResourceHandle<lua_State>& handle);
		virtual ~WinRTStore();

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

} // namespace Rtt
