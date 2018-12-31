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
#include "Rtt_IPhoneAudioSessionManager.h"
#import "Rtt_IPhoneAudioSessionManagerImplementation.h"

#include "Rtt_LuaContext.h"
#include "Rtt_Event.h"
#include "Rtt_Runtime.h"

namespace Rtt
{

IPhoneAudioSessionManager* s_iPhoneAudioSessionMananger = NULL;

IPhoneAudioSessionManager::IPhoneAudioSessionManager()
{
}
IPhoneAudioSessionManager::~IPhoneAudioSessionManager()
{
}

IPhoneAudioSessionManager*
IPhoneAudioSessionManager::CreateInstance()
{
	if(NULL == s_iPhoneAudioSessionMananger)
	{
		// I don't have easy access to an allocator since the need to access AudioSessions may happen at any point of an application life-cycle.
		// This is a singleton which uses an Obj-C singleton which calls all native APIs. Custom allocators aren't going to help us here anyway.
		s_iPhoneAudioSessionMananger = Rtt_NEW( NULL, IPhoneAudioSessionManager);
	}
	return s_iPhoneAudioSessionMananger;
}

IPhoneAudioSessionManager*
IPhoneAudioSessionManager::GetInstance()
{
	if(NULL == s_iPhoneAudioSessionMananger)
	{
		s_iPhoneAudioSessionMananger = CreateInstance();
	}
	return s_iPhoneAudioSessionMananger;
}

void 
IPhoneAudioSessionManager::DestroyInstance()
{
	if(NULL != s_iPhoneAudioSessionMananger)
	{
		Rtt_DELETE( s_iPhoneAudioSessionMananger );
		s_iPhoneAudioSessionMananger = NULL;
	}
}


bool
IPhoneAudioSessionManager::SetAudioSessionActive( bool is_active )
{
	return [[Rtt_IPhoneAudioSessionManagerImplementation sharedManager] setAudioSessionActive:is_active];
}

bool 
IPhoneAudioSessionManager::GetAudioSessionActive() const
{
	return (bool)[[Rtt_IPhoneAudioSessionManagerImplementation sharedManager] audioSessionActive];
}

void
IPhoneAudioSessionManager::SetImplicitRecordingModeIfNecessary()
{
	[[Rtt_IPhoneAudioSessionManagerImplementation sharedManager] setImplicitRecordingModeIfNecessary];
}

void
IPhoneAudioSessionManager::RestoreAudioSessionCategoryIfNecessary()
{
	[[Rtt_IPhoneAudioSessionManagerImplementation sharedManager] restoreAudioSessionCategoryIfNecessary];
}

void
IPhoneAudioSessionManager::PrepareAudioSystemForMoviePlayback()
{
	[[Rtt_IPhoneAudioSessionManagerImplementation sharedManager] prepareAudioSystemForMoviePlayback];
}

void
IPhoneAudioSessionManager::RestoreAudioSystemFromMoviePlayback()
{
	[[Rtt_IPhoneAudioSessionManagerImplementation sharedManager] restoreAudioSystemFromMoviePlayback];
}


bool
IPhoneAudioSessionManager::SupportsBackgroundAudio() const
{
	return (bool)[[Rtt_IPhoneAudioSessionManagerImplementation sharedManager] supportsBackgroundAudio];
}

bool
IPhoneAudioSessionManager::AllowsAudioDuringScreenLock() const
{
	return (bool)[[Rtt_IPhoneAudioSessionManagerImplementation sharedManager] allowsAudioDuringScreenLock];
}

bool
IPhoneAudioSessionManager::IsInInterruption() const
{
	return (bool)[[Rtt_IPhoneAudioSessionManagerImplementation sharedManager] inInterruption];
}

// -----------------------------------------------------------------------------
	
#ifdef Rtt_AUDIO_SESSION_PROPERTY

char
IPhoneAudioSessionManager::GetPropertyType(AudioSessionPropertyID the_property) const
{
	return [[Rtt_IPhoneAudioSessionManagerImplementation sharedManager] propertyType:the_property];
}

UInt32
IPhoneAudioSessionManager::GetPropertyUInt32(AudioSessionPropertyID the_property) const
{
	return [[Rtt_IPhoneAudioSessionManagerImplementation sharedManager] propertyUInt32:the_property];
}
	
Float32
IPhoneAudioSessionManager::GetPropertyFloat32(AudioSessionPropertyID the_property) const
{
	return [[Rtt_IPhoneAudioSessionManagerImplementation sharedManager] propertyFloat32:the_property];
}
	
Float64
IPhoneAudioSessionManager::GetPropertyFloat64(AudioSessionPropertyID the_property) const
{
	return [[Rtt_IPhoneAudioSessionManagerImplementation sharedManager] propertyFloat64:the_property];
}
	
bool
IPhoneAudioSessionManager::GetPropertyBool(AudioSessionPropertyID the_property) const
{
	return (bool)[[Rtt_IPhoneAudioSessionManagerImplementation sharedManager] propertyBool:the_property];
}
	
void
IPhoneAudioSessionManager::SetProperty( AudioSessionPropertyID the_property, Float32 value )
{
	[[Rtt_IPhoneAudioSessionManagerImplementation sharedManager] setProperty:the_property valueFloat32:value];
}

void
IPhoneAudioSessionManager::SetProperty( AudioSessionPropertyID the_property, Float64 value )
{
	[[Rtt_IPhoneAudioSessionManagerImplementation sharedManager] setProperty:the_property valueFloat64:value];
}
void
IPhoneAudioSessionManager::SetProperty( AudioSessionPropertyID the_property, UInt32 value )
{
	[[Rtt_IPhoneAudioSessionManagerImplementation sharedManager] setProperty:the_property valueUInt32:value];
}
	
void
IPhoneAudioSessionManager::SetProperty( AudioSessionPropertyID the_property, bool value )
{
	[[Rtt_IPhoneAudioSessionManagerImplementation sharedManager] setProperty:the_property valueBool:value];
}

#endif // Rtt_AUDIO_SESSION_PROPERTY

// -----------------------------------------------------------------------------

} // namespace Rtt

