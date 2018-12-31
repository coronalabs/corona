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

#include "stdafx.h"
#include "Rtt_WinEventSound.h"
#include "Interop\RuntimeEnvironment.h"
#include "Interop\UI\MessageOnlyWindow.h"
#include "Rtt_WinAudioPlayer.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Runtime.h"


namespace Rtt
{

WinEventSound::WinEventSound(Interop::RuntimeEnvironment& environment, const ResourceHandle<lua_State>& handle)
:	Super( handle ),
	fCompletionCallback(this, &WinEventSound::OnSoundCompleted),
	fSound( nullptr ),
	fPlaying( false )
{
	fSound = new WinAudioPlayer(environment, handle);
	fSound->SetCompletionHandler( &fCompletionCallback );
}

WinEventSound::~WinEventSound()
{
	Rtt_DELETE( fSound );
}

bool WinEventSound::Load( const char * filePath )
{
	return fSound->Load( filePath );
}

void WinEventSound::Play()
{
	fSound->Play();
}

void WinEventSound::ReleaseOnComplete()
{
	if (fPlaying == false)
	{
		Rtt_DELETE(this);
	}
	else
	{
		SetWantsReleaseOnComplete( true );
	}
}
	
void WinEventSound::Stop()
{
	Rtt_ASSERT_NOT_REACHED();
}

void WinEventSound::Pause()
{
	Rtt_ASSERT_NOT_REACHED();
}

void WinEventSound::Resume()
{
	Rtt_ASSERT_NOT_REACHED();
}

void WinEventSound::SetVolume( Rtt_Real volume )
{
	Rtt_ASSERT_NOT_REACHED();
}

Rtt_Real WinEventSound::GetVolume() const
{
	Rtt_ASSERT_NOT_REACHED();
	
	return 1.0f;
}

void WinEventSound::OnSoundCompleted()
{
	fPlaying = false;

	if (HasListener())
	{
		Runtime *runtimePointer = LuaContext::GetRuntime(GetLuaState());
		if (runtimePointer)
		{
			CompletionEvent *event = PlatformEventSound::CreateCompletionEvent(runtimePointer->GetAllocator(), this);
			ScheduleDispatch(event);
		}
	}
	else if (WantsReleaseOnComplete())
	{
		Rtt_DELETE(this);
	}
}

}	// namespace Rtt
