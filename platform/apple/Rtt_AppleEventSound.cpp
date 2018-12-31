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

#include "Rtt_AppleEventSound.h"
#include "Rtt_LuaContext.h"
#include "Rtt_Runtime.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

void
AppleEventSound::OnCompleteEventSound( SystemSoundID soundID, void* userData )
{
	AppleEventSound *eventSound = (AppleEventSound *) userData;

	eventSound->fPlaying = false;
	
	if ( eventSound->HasListener() )
	{
		lua_State *L = eventSound->GetLuaState();
		Rtt_ASSERT( L );
		if ( L )
		{
			CompletionEvent *e = PlatformEventSound::CreateCompletionEvent(
				LuaContext::GetRuntime( L )->GetAllocator(), eventSound );
			eventSound->ScheduleDispatch( e );
		}
	} 
	else if ( eventSound->WantsReleaseOnComplete() )
	{
		Rtt_DELETE( eventSound );
	}
}

AppleEventSound::AppleEventSound( const ResourceHandle<lua_State> & handle )
:	Super( handle ),
	fSoundID( 0 ),
	fPlaying( false )
{
}
		
AppleEventSound::~AppleEventSound()
{
	AudioServicesDisposeSystemSoundID( fSoundID );
}

bool
AppleEventSound::Load( const char * filePath )
{
	//	const char* filePath = PathForFile( filename, MPlatform::kResourceDir, false );
	CFURLRef fileURL = CFURLCreateFromFileSystemRepresentation( NULL, (const UInt8 *)filePath, strlen(filePath), false );
	if ( Rtt_VERIFY( NULL != fileURL ) )
	{
		OSStatus err = AudioServicesCreateSystemSoundID( fileURL, & fSoundID );
		Rtt_ASSERT( noErr == err ); Rtt_UNUSED(err);
		Rtt_WARN_SIM( noErr == err, ( "WARNING: Mac error(%d) while creating event sound for file(%s)\n", err, filePath ) );
		CFRelease( fileURL );
		
		if ( fSoundID )
		{
			err = AudioServicesAddSystemSoundCompletion( fSoundID, NULL, NULL, OnCompleteEventSound, this );
			
			return err == noErr;
		}
	}
	
	return false;
}


void 
AppleEventSound::Play()
{
	AudioServicesPlaySystemSound( fSoundID );
	fPlaying = true;
}

void
AppleEventSound::ReleaseOnComplete()
{
	if ( fPlaying == false )
	{
		Rtt_DELETE( this );
	}
	else
	{
		SetWantsReleaseOnComplete( true );
	}
}
	
void 
AppleEventSound::Stop()
{
	Rtt_ASSERT_NOT_REACHED();
}

void 
AppleEventSound::Pause()
{
	Rtt_ASSERT_NOT_REACHED();
}

void 
AppleEventSound::Resume()
{
	Rtt_ASSERT_NOT_REACHED();
}

void 
AppleEventSound::SetVolume( Rtt_Real volume )
{
	Rtt_ASSERT_NOT_REACHED();
}

Rtt_Real 
AppleEventSound::GetVolume() const
{
	Rtt_ASSERT_NOT_REACHED();
	
	return 1.0f;
}

// ----------------------------------------------------------------------------

} // namespace Rtt



