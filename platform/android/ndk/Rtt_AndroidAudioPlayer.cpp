//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "Core/Rtt_Build.h"

#include "Rtt_AndroidAudioPlayer.h"
#include "NativeToJavaBridge.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

AndroidAudioPlayer::~AndroidAudioPlayer()
{
}

bool
AndroidAudioPlayer::Load( const char * filePath )
{
	mySoundName.Set( filePath );

	fNativeToJavaBridge->LoadSound( (uintptr_t) this, filePath, false );

	return true;
}

void 
AndroidAudioPlayer::Play()
{
	fNativeToJavaBridge->PlaySound( (uintptr_t) this, mySoundName.GetString(), GetLooping() );
}

void 
AndroidAudioPlayer::Stop()
{
	fNativeToJavaBridge->StopSound((uintptr_t) this );
}

void 
AndroidAudioPlayer::Pause()
{
	fNativeToJavaBridge->PauseSound( (uintptr_t) this );
}

void 
AndroidAudioPlayer::Resume()
{
	fNativeToJavaBridge->ResumeSound( (uintptr_t) this );
}

void
AndroidAudioPlayer::SetVolume( Rtt_Real volume )
{
{
	fNativeToJavaBridge->SetVolume( (uintptr_t) this, volume);
}
}

Rtt_Real
AndroidAudioPlayer::GetVolume() const
{
{
	return fNativeToJavaBridge->GetVolume( (uintptr_t) this );
}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

