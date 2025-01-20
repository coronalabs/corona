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
#	include "Core/Rtt_String.h"
#	include "Rtt_PlatformVideoPlayer.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

class WinRTVideoPlayer : public PlatformVideoPlayer
{
	public:
		WinRTVideoPlayer(const Rtt::ResourceHandle<lua_State> & handle, Rtt_Allocator & allocator);

		virtual bool Load(const char* path, bool remote);
		virtual void NotificationCallback();
		virtual void Play();
		virtual void SetProperty(U32 mask, bool newValue);
};

} // namespace Rtt
