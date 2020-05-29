//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core/Rtt_Build.h"
#	include "Rtt_PlatformVideoProvider.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

/// Provides an video picker window. Selected video can be sent to a Lua listener function.
class WinRTVideoProvider : public PlatformVideoProvider
{
	public:
		WinRTVideoProvider(const ResourceHandle<lua_State> & handle);

		virtual bool Supports(int source) const;
		virtual bool Show(int source, lua_State* L, int maxTime, int quality);
};

} // namespace Rtt
