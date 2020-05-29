//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Core\Rtt_Build.h"
#include "Interop\RuntimeEnvironment.h"
#include "Interop\UI\MessageOnlyWindow.h"
#include "Rtt_Callback.h"
#include "Rtt_WinAudioPlayer.h"


namespace Rtt
{

WinAudioPlayer::WinAudioPlayer(Interop::RuntimeEnvironment& environment, const ResourceHandle<lua_State> & handle)
:	PlatformAudioPlayer( handle ),
	fSound(environment.GetMessageOnlyWindow())
{
	fCompletionCallbackPointer = nullptr;
	fSound.SetPlayer(this);
}

WinAudioPlayer::~WinAudioPlayer()
{
}

bool WinAudioPlayer::Load( const char* filePath )
{
	fSound.Load(filePath);

	return true;
}

void WinAudioPlayer::Play()
{
	fSound.Play();
}

void WinAudioPlayer::Stop()
{
	fSound.Stop();
}

void WinAudioPlayer::Pause()
{
	fSound.Pause();
}

void WinAudioPlayer::Resume()
{
	fSound.Resume();
}

void WinAudioPlayer::SetVolume( Rtt_Real volume )
{
	fSound.SetVolume(volume);
}

Rtt_Real WinAudioPlayer::GetVolume() const
{
	return (Rtt_Real)fSound.GetVolume();
}

void WinAudioPlayer::NotificationCallback()
{
	if (fCompletionCallbackPointer)
	{
		fCompletionCallbackPointer->Invoke();
	}
	else
	{
		PlatformAudioPlayer::NotificationCallback();
	}
}

}	// namespace Rtt
