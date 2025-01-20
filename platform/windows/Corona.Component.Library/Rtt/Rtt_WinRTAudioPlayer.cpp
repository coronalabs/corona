//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Rtt_WinRTAudioPlayer.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core/Rtt_Build.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

#pragma region Constructors/Destructors
WinRTAudioPlayer::WinRTAudioPlayer(const ResourceHandle<lua_State> & handle, Rtt_Allocator & allocator)
:	PlatformAudioPlayer(handle)
{
}

WinRTAudioPlayer::~WinRTAudioPlayer()
{
}

#pragma endregion


#pragma region Public Member Functions
bool WinRTAudioPlayer::Load(const char * filePath)
{
	return true;
}

void WinRTAudioPlayer::Play()
{
}

void WinRTAudioPlayer::Stop()
{
}

void WinRTAudioPlayer::Pause()
{
}

void WinRTAudioPlayer::Resume()
{
}

void WinRTAudioPlayer::SetVolume(Rtt_Real volume)
{
}

Rtt_Real WinRTAudioPlayer::GetVolume() const
{
	return 0;
}

#pragma endregion

} // namespace Rtt
