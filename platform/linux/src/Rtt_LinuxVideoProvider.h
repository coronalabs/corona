//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Rtt_PlatformVideoProvider.h"

namespace Rtt
{
	/// Provides an video picker window. Selected video can be sent to a Lua listener function.
	class LinuxVideoProvider : public PlatformVideoProvider
	{
	public:
		LinuxVideoProvider(const ResourceHandle<lua_State> &handle, int w, int h);

		virtual bool Supports(int source) const;
		virtual bool Show(int source, lua_State* L, int maxTime, int quality);
	};
}; // namespace Rtt
