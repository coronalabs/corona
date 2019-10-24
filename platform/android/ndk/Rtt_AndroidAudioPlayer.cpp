//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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

