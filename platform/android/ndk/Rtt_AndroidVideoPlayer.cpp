//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#include "Core/Rtt_Build.h"

#include "Rtt_AndroidVideoPlayer.h"
#include "NativeToJavaBridge.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

bool
AndroidVideoPlayer::Load( const char * path, bool isRemote )
{
	fPath.Set( path );
	return true;
}

void
AndroidVideoPlayer::Play()
{
	fNativeToJavaBridge->PlayVideo( (uintptr_t) this, fPath.GetString(), fMediaControlsEnabled );
}

void
AndroidVideoPlayer::SetProperty( U32 mask, bool newValue )
{
	PlatformVideoPlayer::SetProperty( mask, newValue );
	
	if (mask & kShowControls)
	{
		fMediaControlsEnabled = newValue;
	}
}

void
AndroidVideoPlayer::NotificationCallback()
{
	DidDismiss( NULL, NULL );
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

