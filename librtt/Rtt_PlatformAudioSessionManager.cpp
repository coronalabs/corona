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

#include "Rtt_PlatformAudioSessionManager.h"


#include "Core/Rtt_Build.h"
#include "Rtt_PlatformAudioSessionManager.h"

#ifdef Rtt_IPHONE_ENV
	#include "Rtt_IPhoneAudioSessionManager.h"
#endif


namespace Rtt
{

PlatformAudioSessionManager* s_PlatformAudioSessionMananger = NULL;

PlatformAudioSessionManager::PlatformAudioSessionManager()
{		
}

PlatformAudioSessionManager::~PlatformAudioSessionManager()
{		
}
	
PlatformAudioSessionManager* 
PlatformAudioSessionManager::SharedInstance()
{
	if(NULL == s_PlatformAudioSessionMananger)
	{
#ifdef Rtt_IPHONE_ENV
//		s_PlatformAudioSessionMananger = Rtt_NEW( NULL, PlatformAudioSessionManager);
		s_PlatformAudioSessionMananger = IPhoneAudioSessionManager::GetInstance();
#else
		s_PlatformAudioSessionMananger = Rtt_NEW( NULL, PlatformAudioSessionManager);
#endif
	}
	return s_PlatformAudioSessionMananger;
}

void 
PlatformAudioSessionManager::Destroy()
{
	if(NULL != s_PlatformAudioSessionMananger)
	{
#ifdef Rtt_IPHONE_ENV
		IPhoneAudioSessionManager::DestroyInstance();
#else
		Rtt_DELETE( s_PlatformAudioSessionMananger );
#endif
		s_PlatformAudioSessionMananger = NULL;
	}
}


bool
PlatformAudioSessionManager::SetAudioSessionActive( bool is_active )
{
	return true;
}

bool 
PlatformAudioSessionManager::GetAudioSessionActive() const
{
	return true;
}

void
PlatformAudioSessionManager::SetImplicitRecordingModeIfNecessary()
{
}

void
PlatformAudioSessionManager::RestoreAudioSessionCategoryIfNecessary()
{
}

void
PlatformAudioSessionManager::PrepareAudioSystemForMoviePlayback()
{
}

void
PlatformAudioSessionManager::RestoreAudioSystemFromMoviePlayback()
{
}


bool
PlatformAudioSessionManager::SupportsBackgroundAudio() const
{
	return false;
}

bool
PlatformAudioSessionManager::AllowsAudioDuringScreenLock() const
{
	return false;
}

bool
PlatformAudioSessionManager::IsInInterruption() const
{
	return false;
}
#if 0
char
PlatformAudioSessionManager::GetPropertyType(AudioSessionPropertyID the_property) const
{
	return [[Rtt_PlatformAudioSessionManagerImplementation sharedManager] propertyType:the_property];
}

UInt32
PlatformAudioSessionManager::GetPropertyUInt32(AudioSessionPropertyID the_property) const
{
	return [[Rtt_PlatformAudioSessionManagerImplementation sharedManager] propertyUInt32:the_property];
}
	
Float32
PlatformAudioSessionManager::GetPropertyFloat32(AudioSessionPropertyID the_property) const
{
	return [[Rtt_PlatformAudioSessionManagerImplementation sharedManager] propertyFloat32:the_property];
}
	
Float64
PlatformAudioSessionManager::GetPropertyFloat64(AudioSessionPropertyID the_property) const
{
	return [[Rtt_PlatformAudioSessionManagerImplementation sharedManager] propertyFloat64:the_property];
}
	
bool
PlatformAudioSessionManager::GetPropertyBool(AudioSessionPropertyID the_property) const
{
	return (bool)[[Rtt_PlatformAudioSessionManagerImplementation sharedManager] propertyBool:the_property];
}
	
void
PlatformAudioSessionManager::SetProperty( AudioSessionPropertyID the_property, Float32 value )
{
	[[Rtt_PlatformAudioSessionManagerImplementation sharedManager] setProperty:the_property valueFloat32:value];
}

void
PlatformAudioSessionManager::SetProperty( AudioSessionPropertyID the_property, Float64 value )
{
	[[Rtt_PlatformAudioSessionManagerImplementation sharedManager] setProperty:the_property valueFloat64:value];
}
void
PlatformAudioSessionManager::SetProperty( AudioSessionPropertyID the_property, UInt32 value )
{
	[[Rtt_PlatformAudioSessionManagerImplementation sharedManager] setProperty:the_property valueUInt32:value];
}
	
void
PlatformAudioSessionManager::SetProperty( AudioSessionPropertyID the_property, bool value )
{
	[[Rtt_PlatformAudioSessionManagerImplementation sharedManager] setProperty:the_property valueBool:value];
}
#endif
} // namespace Rtt
