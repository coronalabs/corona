//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

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
