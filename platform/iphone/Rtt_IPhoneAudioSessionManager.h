//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhoneAudioSessionManager_H__
#define _Rtt_IPhoneAudioSessionManager_H__


#include "Core/Rtt_Types.h"
#include "Rtt_PlatformAudioSessionManager.h"
#include "Core/Rtt_Types.h"
#include "Core/Rtt_ResourceHandle.h"

#include <AudioToolbox/AudioToolbox.h>
#include "Rtt_IPhoneAudioSessionConstants.h"
// ----------------------------------------------------------------------------

struct lua_State;

namespace Rtt
{
// Note: On iOS, AudioSessionServies MUST be treated as a singleton.
// There may not be multiple instances because there may only be ONE AudioSessionInitialize per-app and states like Active are fragile to begin with.
// And a AudioSession really can't be uninitialized.
// The API design is designed to make this absolutely clear.
class IPhoneAudioSessionManager : public PlatformAudioSessionManager
{
protected:
	IPhoneAudioSessionManager();
	virtual ~IPhoneAudioSessionManager();

public:
	
	static IPhoneAudioSessionManager* CreateInstance();
	static IPhoneAudioSessionManager* GetInstance();

	// Doesn't really do anything. Don't rely on this. We can't uninitialize an AudioSession. Don't try to recreate after you call this.
	// If you call this, it should only be part of clean up and the program is expected to exit.
	static void DestroyInstance();
	
	virtual bool SetAudioSessionActive( bool is_active );
	virtual bool GetAudioSessionActive() const;
	
	virtual void SetImplicitRecordingModeIfNecessary();
	virtual void RestoreAudioSessionCategoryIfNecessary();
	
	virtual void PrepareAudioSystemForMoviePlayback();
	virtual void RestoreAudioSystemFromMoviePlayback();

	
	virtual bool SupportsBackgroundAudio() const;
	virtual bool AllowsAudioDuringScreenLock() const;
	
	virtual bool IsInInterruption() const;
	
	
#ifdef Rtt_AUDIO_SESSION_PROPERTY
	// Assertion: AudioSessionPropertyID is defined as UInt32
	// Returns a type as defined in <objc/runtime.h>
	char GetPropertyType(AudioSessionPropertyID the_property) const;
	UInt32 GetPropertyUInt32(AudioSessionPropertyID the_property) const;
	Float32 GetPropertyFloat32(AudioSessionPropertyID the_property) const;
	Float64 GetPropertyFloat64(AudioSessionPropertyID the_property) const;
	bool GetPropertyBool(AudioSessionPropertyID the_property) const;

	void SetProperty( AudioSessionPropertyID the_property, Float32 value );
	void SetProperty( AudioSessionPropertyID the_property, Float64 value );
	void SetProperty( AudioSessionPropertyID the_property, UInt32 value );
	void SetProperty( AudioSessionPropertyID the_property, bool value );
#endif // Rtt_AUDIO_SESSION_PROPERTY
};
}
#endif // _Rtt_IPhoneAudioSessionManager_H__
