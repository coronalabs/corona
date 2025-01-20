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
#	include "Rtt_PlatformImageProvider.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

/// Provides an image picker window. Selected image can be sent to a Lua listener function.
class WinRTImageProvider : public PlatformImageProvider
	{
	public:
		WinRTImageProvider(const ResourceHandle<lua_State> & handle);
		virtual bool Supports(int source) const;
		virtual bool Show(int source, const char* filePath, lua_State* L);
};

} // namespace Rtt
