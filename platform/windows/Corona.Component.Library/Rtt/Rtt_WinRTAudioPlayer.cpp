// ----------------------------------------------------------------------------
// 
// Rtt_WinRTAudioPlayer.cpp
// Copyright (c) 2013 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

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
