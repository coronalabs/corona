// ----------------------------------------------------------------------------
// 
// Rtt_WinRTVideoProvider.h
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

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
