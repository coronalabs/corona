//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Rtt_WinRTVideoPlayer.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core/Rtt_Build.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

#pragma region Constructors/Destructors
WinRTVideoPlayer::WinRTVideoPlayer(const Rtt::ResourceHandle<lua_State> & handle, Rtt_Allocator & allocator)
:	PlatformVideoPlayer(handle)
{
}

#pragma endregion


#pragma region Public Member Functions
bool WinRTVideoPlayer::Load(const char * path, bool isRemote)
{
	return false;
}

void WinRTVideoPlayer::Play()
{
}

void WinRTVideoPlayer::SetProperty(U32 mask, bool newValue)
{
	PlatformVideoPlayer::SetProperty(mask, newValue);

	if (mask & kShowControls)
	{
		//		fMediaControlsEnabled = newValue;
	}
}

void WinRTVideoPlayer::NotificationCallback()
{
	DidDismiss(NULL, NULL);
}

#pragma endregion

} // namespace Rtt
