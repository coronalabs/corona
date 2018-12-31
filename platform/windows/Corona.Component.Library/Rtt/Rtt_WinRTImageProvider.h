// ----------------------------------------------------------------------------
// 
// Rtt_WinRTImageProvider.h
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

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
