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
#include "Rtt_EmscriptenStore.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class EmscriptenStoreProvider : public PlatformStoreProvider
{
	public:
		EmscriptenStoreProvider(const ResourceHandle<lua_State>& handle);
		virtual ~EmscriptenStoreProvider();

		void SetActiveStore(const char *storeName);
		PlatformStore* GetActiveStore();
		const char* GetTargetedStoreName();

	private:
		EmscriptenStore *fActiveStore;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
