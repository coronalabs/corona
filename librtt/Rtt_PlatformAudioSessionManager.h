//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_PlatformAudioSessionManager_H__
#define _Rtt_PlatformAudioSessionManager_H__


#include "Core/Rtt_Types.h"
//#include "Rtt_IPhoneAudioSessionConstants.h"
// ----------------------------------------------------------------------------

namespace Rtt
{

// To get instances of PlatformAudioSessionManager, use Get().
// C++ isn't terribly good a factory patterns, so this is how we do it.
class PlatformAudioSessionManager
{
protected:
	PlatformAudioSessionManager();
	virtual ~PlatformAudioSessionManager();

public:
	static PlatformAudioSessionManager* SharedInstance();
	static void Destroy();
	
	
	// Workaround for iOS 4/5 bug: A user leaving an app via notification center triggers a unnecessary resume/suspend event in Cocoa.
	// This event forces us to re-initialize the audio system on resume which seems to lead to breaking audio completely.
	// My theory is that Core Audio is trying to shutdown and reactivating OpenAL messes things up.
	// It turns out setting the Audio Session to active seems to always fail here so we might be able to use that as a workaround clue.
	// The problem is that setting the audio session to the currently set state (e.g. on when already on, off when already off) also triggers an error.
	// And since there is no GetActive() API, we can't know the state since Apple may change it for us on audio interruptions or 3rd party API calls may change the state behind our back. So relying on this flag too much will also cause problems. But in a single isolated case of an EndInterruption, this might just work.
	// iOS 6 seems to work with or without this workaround. I'm indecisive on whether to use this on iOS 6 or not because Apple does not really define clearly why AudioSessionSetActive would fail. For simplicity, I'm thinking we will keep the code base the same, but this API is available so we can toggle it as needed (e.g. in AppDelegate after testing for iOS version).
	/* Returns true if the set was successful, false if an error occurred. Yes, this may seem a little weird but it is related to a set of iOS bugs where setting may fail for reasons out of our control and also stupid reasons (like setting it to off when already off). There is no getter in iOS to make things harder. And there is a bug we are trying to workaround which corrupts the entire audio system and our workaround is to use this failure as a clue to avoid things in that situation. The state of the audio session is unknown if this fails in iOS.
		 radar://12022630 Core Audio breaks when leaving an app via Notification Center and then returning
		 radar://12022774 Redundant WillResignActive/BecomeActive when leaving app via Notification Center
		 radar://12022812 AudioSessionSetActive should not return errors if setting to false when already false
		 radar://12022855 Need API: AudioSessionGetActive() to balance AudioSessionSetActive
		 radar://12022872 Want Audio Session Interruptions to trigger for all events that require it
		 radar://12022894 Want automatic handling for OpenAL/Core Audio interruptions like AVFoundation
	 */
	virtual bool SetAudioSessionActive( bool is_active );
	virtual bool GetAudioSessionActive() const;
	
	virtual void SetImplicitRecordingModeIfNecessary();
	virtual void RestoreAudioSessionCategoryIfNecessary();
	
	virtual void PrepareAudioSystemForMoviePlayback();
	virtual void RestoreAudioSystemFromMoviePlayback();

	
	virtual bool SupportsBackgroundAudio() const;
	virtual bool AllowsAudioDuringScreenLock() const;
	
	virtual bool IsInInterruption() const;
	
	
	// Assestion: AudioSessionPropertyID is defined as UInt32 

	// Returns a type as defined in <objc/runtime.h>
	/*
	char GetPropertyType(AudioSessionPropertyID the_property) const;
	UInt32 GetPropertyUInt32(AudioSessionPropertyID the_property) const;
	Float32 GetPropertyFloat32(AudioSessionPropertyID the_property) const;
	Float64 GetPropertyFloat64(AudioSessionPropertyID the_property) const;
	bool GetPropertyBool(AudioSessionPropertyID the_property) const;

	void SetProperty( AudioSessionPropertyID the_property, Float32 value );
	void SetProperty( AudioSessionPropertyID the_property, Float64 value );
	void SetProperty( AudioSessionPropertyID the_property, UInt32 value );
	void SetProperty( AudioSessionPropertyID the_property, bool value );
*/
};
}
#endif // _Rtt_PlatformAudioSessionManager_H__
