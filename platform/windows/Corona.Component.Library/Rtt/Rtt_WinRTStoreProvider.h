// ----------------------------------------------------------------------------
// 
// Rtt_WinRTStoreProvider.h
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once

#include "Rtt_WinRTStore.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Rtt_PlatformInAppStore.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

class WinRTStoreProvider : public PlatformStoreProvider
{
	public:
		WinRTStoreProvider(const ResourceHandle<lua_State>& handle);
		virtual ~WinRTStoreProvider();

		void SetActiveStore(const char *storeName);
		PlatformStore* GetActiveStore();
		const char* GetTargetedStoreName();

	private:
		WinRTStore *fActiveStore;
};

} // namespace Rtt
