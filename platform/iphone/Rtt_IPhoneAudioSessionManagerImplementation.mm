//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import "Rtt_IPhoneAudioSessionManagerImplementation.h"
#include <AudioToolbox/AudioToolbox.h>
#import "Rtt_PlatformOpenALPlayer.h"
#include "Rtt_IPhoneAudioSessionConstants.h"

#include <TargetConditionals.h>
#import <objc/runtime.h>

#if (TARGET_OS_IOS == 1)
	#include <AudioToolbox/AudioToolbox.h>
	#include <arpa/inet.h> /* htonl */
	#include "Rtt_IPhoneAudioSessionManager.h"

#import <UIKit/UIDevice.h>

@interface Rtt_IPhoneAudioSessionManagerImplementation ()
@property(assign, nonatomic, readwrite) BOOL inInterruption;
// This is needed because in an interruption, Core Audio automatically disables the active state,
// but we need to make sure our shadow variable is in sync. If we use the regular setter, setting the audio session again to off
// seems to trigger a Core Audio error and messes up things even more.
- (void) setInternalAudioSessionActive:(BOOL)is_active;
@end

static const char* CoreAudio_FourCCToString(int32_t error_code)
{
	static char return_string[16];
	uint32_t big_endian_code = htonl(error_code);
	char* big_endian_str = (char*)&big_endian_code;
	// see if it appears to be a 4-char-code
	if(isprint(big_endian_str[0])
	   && isprint(big_endian_str[1])
	   && isprint(big_endian_str[2])
	   && isprint (big_endian_str[3]))
	{
		return_string[0] = '\'';
		return_string[1] = big_endian_str[0];
		return_string[2] = big_endian_str[1];
		return_string[3] = big_endian_str[2];
		return_string[4] = big_endian_str[3];
		return_string[5] = '\'';
		return_string[6] = '\0';
	}
	else if(error_code > -200000 && error_code < 200000)
	{
		// no, format it as an integer
		snprintf(return_string, 16, "%d", error_code);
	}
	else
	{
		// no, format it as an integer but in hex
		snprintf(return_string, 16, "0x%x", error_code);
	}
	return return_string;
}

// TODO: Replace this InterruptionCallback with NSNotificationCenter events
// (AVAudioSessionDidBeginInterruptionNotification and AVAudioSessionDidEndInterruptionNotification)
// as AudioSessionInitialize is deprecated.
//
// TODO: The audio recorder code for iOS needs to be audited. 
// I suspect it doesn't handle interruptions because I couldn't find any audio session stuff in the code.
static void Internal_InterruptionCallback(void* user_data, UInt32 interruption_state)
{
	// The interruption callback should no-op if there are no CoronaViews in existence
	Rtt::PlatformOpenALPlayer *player = Rtt::PlatformOpenALPlayer::SharedInstance();
	if ( ! player )
	{
		return;
	}

	Rtt_IPhoneAudioSessionManagerImplementation* audio_session_manager = (Rtt_IPhoneAudioSessionManagerImplementation*)user_data;
	if(kAudioSessionBeginInterruption == interruption_state)
	{
		audio_session_manager.inInterruption = YES;
		// Need to make sure our shadow variable is consistent with the real state.
		// Don't call the normal setter because Core Audio disabled the active state behind our back already and setting it off again triggers a Core Audio error.
		// BeginInterruption() calls SetAudioSessionActive so these are redundant. We tested these commented out with 6575 so I don't want to re-enable them.
//		[audio_session_manager setAudioSessionActive:NO];
//		[audio_session_manager setInternalAudioSessionActive:NO];
		player->BeginInterruption();
	}
	else if(kAudioSessionEndInterruption == interruption_state)
	{
		// Apple bug: There seems to be another race condition bug: 6575 where setting the audio session active too soon causes the operation to fail.
		// This causes the whole audio chain to not resume correctly. The workaround is to sleep for a period of time.
		// We tested 1000000 and 2000000 and it worked for us.
		usleep(1000000);
		player->EndInterruption();
		// Need to make sure our shadow variable is consistent with the real state.
		[audio_session_manager setAudioSessionActive:YES];
		audio_session_manager.inInterruption = NO;
	}
}

#endif // (TARGET_OS_IOS == 1)

@implementation Rtt_IPhoneAudioSessionManagerImplementation

@synthesize inInterruption;

static Rtt_IPhoneAudioSessionManagerImplementation* s_AudioSessionManagerInstance = nil;

+ (id) sharedManager
{
	@synchronized(self)
	{
		if(nil == s_AudioSessionManagerInstance)
		{
			s_AudioSessionManagerInstance = [[Rtt_IPhoneAudioSessionManagerImplementation alloc] init];
		}
	}
	return s_AudioSessionManagerInstance;
}

- (id) init
{
    self = [super init];
    if(nil != self)
	{
#if (TARGET_OS_IOS == 1)

        // Initialization code here.
		audioSessionActive = NO;
		usingManualSessionManagment = NO;
		// Expected iOS default
		savedPreviousSessionCategory = kAudioSessionCategory_SoloAmbientSound;
		savedBackgroundPreviousSessionCategory = kAudioSessionCategory_SoloAmbientSound;
		needToRestoreFromBackgroundOrScreenLock = NO;
		
		OSStatus the_error = AudioSessionInitialize(NULL, NULL, Internal_InterruptionCallback, self);
		if(noErr != the_error)
		{
			NSLog(@"Error initializing audio session! %s\n", CoreAudio_FourCCToString(the_error));
		}
		
		// If the user has declared they want background audio in the Info.plist, we will infer they really want MediaPlayback mode instead.
		if(YES == [self supportsBackgroundAudio])
		{
			savedPreviousSessionCategory = kAudioSessionCategory_MediaPlayback;
			savedBackgroundPreviousSessionCategory = kAudioSessionCategory_MediaPlayback;

			//	UInt32 category = kAudioSessionCategory_RecordAudio;
			OSStatus error = AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(savedPreviousSessionCategory), &savedPreviousSessionCategory);
			if(noErr != error)
			{
				NSLog(@"Rtt_IPhoneAudioSessionManagerImplementation::init() Error setting audio session category! %s\n", CoreAudio_FourCCToString(error));
			}
		}

#endif		
    }
    return self;
}

- (void) dealloc
{
    [super dealloc];
}

// This is needed because in an interruption, Core Audio automatically disables the active state,
// but we need to make sure our shadow variable is in sync. If we use the regular setter, setting the audio session again to off
// seems to trigger a Core Audio error and messes up things even more.
- (void) setInternalAudioSessionActive:(BOOL)is_active
{
	audioSessionActive = is_active;
}

// Apple doesn't have a GetSessionActive call so we must track this state ourselves.
// Corona must always go through this API for our values to remain consistent.
- (bool) setAudioSessionActive:(BOOL)is_active
{
	bool ret_flag = true;
	// This check will be a problem if we don't consistently go through this API.
	// But bypassing this check may have negative performance implications or worse, particularly if audio is currently playing.
	if(is_active == audioSessionActive)
	{
		return true;
	}
	
#if (TARGET_OS_IOS == 1)
	OSStatus the_error;
	if(is_active)
	{
		the_error = AudioSessionSetActive(is_active);
	}
	else
	{
		// New to iOS 4.0
		// This might allow things like the iPod player to resume if they were cut off when we took control of the audio session.
		the_error = AudioSessionSetActiveWithFlags(is_active, kAudioSessionSetActiveFlag_NotifyOthersOnDeactivation);
	}
	if(noErr != the_error)
	{
		NSLog(@"Error setting audio session active to %d! %s\n", is_active, CoreAudio_FourCCToString(the_error));
		// Not sure how to deal with failure. Sometimes failure means we are setting to an already existing state in which case
		// we don't want to return and set our shadow variable.
		// iOS 4/5 bug: A user leaving an app via notification center triggers a unnecessary resume/suspend event in Cocoa.
		// This event forces us to re-initialize the audio system on resume which seems to lead to breaking audio completely.
		// My theory is that Core Audio is trying to shutdown and reactivating OpenAL messes things up.
		// It turns out setting the Audio Session to active seems to always fail here so we might be able to use that as a workaround clue.
		// The problem is that setting the audio session to the currently set state (e.g. on when already on, off when already off) also triggers an error.
		// And since there is no GetActive() API, we can't know the state since Apple may change it for us on audio interruptions or 3rd party API calls may change the state behind our back. So relying on this flag too much will also cause problems. But in a single isolated case of an EndInterruption, this might just work.
		ret_flag = false;
	}
#endif
	// Setting after call in case operation failed
	audioSessionActive = is_active;
	return ret_flag;
}

- (BOOL) audioSessionActive
{
	return audioSessionActive;
}

- (void) setAudioSessionCategory:(UInt32)category
{
#if (TARGET_OS_IOS == 1)
	OSStatus error;

	if(NO == usingManualSessionManagment)
	{
		UInt32 size = sizeof(savedPreviousSessionCategory);
		AudioSessionGetProperty(kAudioSessionProperty_AudioCategory, &size, &savedPreviousSessionCategory);		
	}
	
//	UInt32 category = kAudioSessionCategory_RecordAudio;
	error = AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(category), &category);
	if(noErr != error)
	{
		NSLog(@"Rtt_IPhoneAudioSessionManagerImplementation::Start() Error setting audio session category! %s\n", CoreAudio_FourCCToString(error));
	}
#endif
}


- (void) setManualAudioSessionCategory:(UInt32)category
{
#if (TARGET_OS_IOS == 1)
	OSStatus error;
	
	usingManualSessionManagment = YES;
	
	//	UInt32 category = kAudioSessionCategory_RecordAudio;
	error = AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(category), &category);
	if(noErr != error)
	{
		NSLog(@"Rtt_IPhoneAudioSessionManagerImplementation::Start() Error setting audio session category! %s\n", CoreAudio_FourCCToString(error));
	}
#endif
}


- (UInt32) audioSessionCategory
{
	UInt32 category = 0;
#if (TARGET_OS_IOS == 1)
//	OSStatus error;
	UInt32 size = sizeof(category);
	AudioSessionGetProperty(kAudioSessionProperty_AudioCategory, &size, &category);		
#endif
	return category;
}

- (void) restoreAudioSessionCategoryIfNecessary
{
#if (TARGET_OS_IOS == 1)
	if(NO == usingManualSessionManagment)
	{
		OSStatus error;
		error = AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(savedPreviousSessionCategory), &savedPreviousSessionCategory);
		if(noErr != error)
		{
			NSLog(@"Rtt_IPhoneAudioSessionManagerImplementation::restoreAudioSessionCategoryIfNecessary: Error setting audio session category! %s\n", CoreAudio_FourCCToString(error));
		}
	}
#endif
}

- (void) setImplicitRecordingModeIfNecessary
{
#if (TARGET_OS_IOS == 1)
	OSStatus error;
	UInt32 category;
	UInt32 size = sizeof(category);

	// No matter what, the audio session must be set active.
	// If the user is manually controlling it, it should be on by now so we can freely set it again.
	// They are responsible for turning it off again.
	[self setAudioSessionActive:true];

	
	error = AudioSessionGetProperty(kAudioSessionProperty_AudioCategory, &size, &category);
	if(noErr != error)
	{
		NSLog(@"Rtt_IPhoneAudioSessionManagerImplementation::setImplicitRecordingModeIfNecessary: Error getting audio session category! %s\n", CoreAudio_FourCCToString(error));
	}
	
	if( (kAudioSessionCategory_RecordAudio == category) || (kAudioSessionCategory_PlayAndRecord == category) )
	{
		// don't need to do anything
		return;
	}


	if(NO == usingManualSessionManagment)
	{
		savedPreviousSessionCategory = category;
		[self setAudioSessionActive:true];
	}
	else
	{
		// Assumption: The user set an audio session at some point but forgot to set the recording mode.
		// Not sure what we should do here.
		NSLog(@"AudioSession categories are being managed manually, but while trying to record, the audio session is set incorrectly.");
		savedPreviousSessionCategory = category;		
		usingManualSessionManagment = NO; // ???
		[self setAudioSessionActive:true];
	}
	
	// TODO: Remove the iOS version check.  Setting the category to kAudioSessionCategory_PlayAndRecord seemed to work on iOS 6/7 devices but
	// we just don't want to risk breaking anything which is why we have the version check.
	if ([[[UIDevice currentDevice] systemVersion] doubleValue] >= 8.0)
	{
		category = kAudioSessionCategory_PlayAndRecord;
	}
	else
	{
		category = kAudioSessionCategory_RecordAudio;
	}
	
	
	//	UInt32 category = kAudioSessionCategory_RecordAudio;
	error = AudioSessionSetProperty(kAudioSessionProperty_AudioCategory, sizeof(category), &category);
	if(noErr != error)
	{
		NSLog(@"Rtt_IPhoneAudioSessionManagerImplementation::setImplicitRecordingModeIfNecessary Error setting audio session category! %s\n", CoreAudio_FourCCToString(error));
	}
#endif
	
}

- (void) prepareAudioSystemForMoviePlayback
{
#if (TARGET_OS_IOS == 1)

	UInt32 mixing_allowed = false;
	
	UInt32 size = sizeof(mixing_allowed);
	AudioSessionGetProperty(kAudioSessionProperty_OverrideCategoryMixWithOthers, &size, &mixing_allowed);		
//	printf("kAudioSessionProperty_OverrideCategoryMixWithOthers: %d\n", mixing_allowed);


	if(false == mixing_allowed)
	{
		// This is a hack to get around an Apple bug.
		// MPMoviePlayer seems to be screwing up the OpenAL context if mixing modes are not allowed.
		// The workaround is to disable the OpenAL context while the movie is playing.
		// But this will prevent any mixing between the two systems.
		// Apple needs to fix this bug for mixing to work.
		Rtt::PlatformOpenALPlayer::SharedInstance()->BeginInterruption();
	}
#endif
}

- (void) restoreAudioSystemFromMoviePlayback
{
#if (TARGET_OS_IOS == 1)

	// There is a possible corner case where we are in a real interruption instead of a movie interruption
	// and we don't actually want to end the interruption.
	// But it might not be worth trying to handle that case because distinguishing between the two might be tricky.
	if(Rtt::PlatformOpenALPlayer::SharedInstance()->IsInInterruption())
	{
		Rtt::PlatformOpenALPlayer::SharedInstance()->EndInterruption();
	}
#endif
}

- (BOOL) supportsBackgroundAudio
{
#if (TARGET_OS_IOS == 1)
	// UIBackgroundModes is an array containing keys. We want to find the key 'audio'.
	NSArray* modes = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"UIBackgroundModes"];
	return [modes containsObject:@"audio"];
#else
	// Actually, maybe we should return YES since we can always do background on a Mac
	return NO;
#endif

}

- (BOOL) allowsAudioDuringScreenLock
{
#if (TARGET_OS_IOS == 1)
	OSStatus error;
	UInt32 category;
	UInt32 size = sizeof(category);
	
	error = AudioSessionGetProperty(kAudioSessionProperty_AudioCategory, &size, &category);
	if(noErr != error)
	{
		NSLog(@"Rtt_IPhoneAudioSessionManagerImplementation::allowsAudioDuringScreenLock: Error getting audio session category! %s\n", CoreAudio_FourCCToString(error));
	}
	
	if( (kAudioSessionCategory_MediaPlayback == category)
		|| (kAudioSessionCategory_RecordAudio == category)
		|| (kAudioSessionCategory_PlayAndRecord == category) )
	{
		// don't need to do anything
		return YES;
	}
	else
	{
		return NO;
	}
#else
	// Maybe we should return YES since we can always do background on a Mac?
	return YES;
#endif
}

#ifdef Rtt_AUDIO_SESSION_PROPERTY

/* Implementation Note: Care was actually put into this implementation to deal with different size type issues depending on the platform you are compiling on.
 * The types used are deliberate.
 */
- (char) propertyType:(AudioSessionPropertyID)the_property
{
#if (TARGET_OS_IOS == 1)
	switch ( the_property )
	{
		// I made up 'wxyz' to represent AudioSessionSetActive
		case kRtt_Fake_AudioSessionProperty_SetActiveMode:
		{
			return @encode(BOOL)[0];
		}
		case kAudioSessionProperty_PreferredHardwareSampleRate:
		case kAudioSessionProperty_CurrentHardwareSampleRate:
		{
			return @encode(Float64)[0];
		}
		case kAudioSessionProperty_PreferredHardwareIOBufferDuration:
		case kAudioSessionProperty_CurrentHardwareOutputVolume:
		case kAudioSessionProperty_CurrentHardwareInputLatency:
		case kAudioSessionProperty_CurrentHardwareOutputLatency:
		case kAudioSessionProperty_CurrentHardwareIOBufferDuration:
		{
			return @encode(Float32)[0];
		}
		default:
		{
			return @encode(UInt32)[0];
		}
	}
#else
	return @encode(UInt32)[0];
#endif
}

- (UInt32) propertyUInt32:(AudioSessionPropertyID)the_property
{
#if (TARGET_OS_IOS == 1)
	OSStatus error;
	UInt32 value = 0;
	UInt32 size = sizeof(value);
	error = AudioSessionGetProperty(the_property, &size, &value);
	if(noErr != error)
	{
		NSLog(@"Error with AudioSessionGetProperty UInt32: %s\n", CoreAudio_FourCCToString(error));
	}
	return value;
#else
	return 0;
#endif
}

- (Float32) propertyFloat32:(AudioSessionPropertyID)the_property
{
#if (TARGET_OS_IOS == 1)
	OSStatus error;
	Float32 value = 0;
	UInt32 size = sizeof(value);
	error = AudioSessionGetProperty(the_property, &size, &value);
	if(noErr != error)
	{
		NSLog(@"Error with AudioSessionGetProperty Float32: %s\n", CoreAudio_FourCCToString(error));
	}
	return value;
#else
	return 0.0;
#endif
}


- (Float64) propertyFloat64:(AudioSessionPropertyID)the_property
{
#if (TARGET_OS_IOS == 1)
	OSStatus error;
	Float64 value = 0;
	UInt32 size = sizeof(value);
	error = AudioSessionGetProperty(the_property, &size, &value);
	if(noErr != error)
	{
		NSLog(@"Error with AudioSessionGetProperty Float64: %s\n", CoreAudio_FourCCToString(error));
	}
	return value;
#else
	return 0.0;
#endif
}

- (BOOL) propertyBool:(AudioSessionPropertyID)the_property
{
#if (TARGET_OS_IOS == 1)
	OSStatus error;
	switch ( the_property )
	{
			// I made up 'wxyz' to represent AudioSessionSetActive
		case kRtt_Fake_AudioSessionProperty_SetActiveMode:
		{
			return (BOOL)audioSessionActive;
		}
		default:
		{
			BOOL value = 0;
			UInt32 size = sizeof(value);
			error = AudioSessionGetProperty(the_property, &size, &value);
			if(noErr != error)
			{
				NSLog(@"Error with AudioSessionGetProperty Bool: %s\n", CoreAudio_FourCCToString(error));
			}
			return value;
			
		}
	}
#else
	return false;
#endif
}


- (void) setProperty:(AudioSessionPropertyID)the_property valueUInt32:(UInt32)value
{
#if (TARGET_OS_IOS == 1)
	OSStatus error;
	switch ( the_property )
	{
		// Call our special wrapper for AudioCategory which disables automatic mode
		case kAudioSessionProperty_AudioCategory:
		{
			[self setManualAudioSessionCategory:value];
			return;
		}
		default:
		{
			error = AudioSessionSetProperty(the_property, sizeof(value), &value);
			if(noErr != error)
			{
				NSLog(@"Error with AudioSessionSetProperty UInt32: %s\n", CoreAudio_FourCCToString(error));
			}
			return;
		}
	}
#endif
}

- (void) setProperty:(AudioSessionPropertyID)the_property valueFloat32:(Float32)value
{
#if (TARGET_OS_IOS == 1)
	OSStatus error;
	error = AudioSessionSetProperty(the_property, sizeof(value), &value);
	if(noErr != error)
	{
		NSLog(@"Error with AudioSessionSetProperty Float32: %s\n", CoreAudio_FourCCToString(error));
	}
#endif
}


- (void) setProperty:(AudioSessionPropertyID)the_property valueFloat64:(Float64)value
{
#if (TARGET_OS_IOS == 1)
	OSStatus error;
	error = AudioSessionSetProperty(the_property, sizeof(value), &value);
	if(noErr != error)
	{
		NSLog(@"Error with AudioSessionSetProperty Float64: %s\n", CoreAudio_FourCCToString(error));
	}
#endif
}

- (void) setProperty:(AudioSessionPropertyID)the_property valueBool:(BOOL)value
{
#if (TARGET_OS_IOS == 1)
	OSStatus error;
	switch ( the_property )
	{
		// I made up 'wxyz' to represent AudioSessionSetActive
		// Hopefully Apple won't use this one.
		case kRtt_Fake_AudioSessionProperty_SetActiveMode:
		{
			[self setAudioSessionActive:value];
			return;
		}
		default:
		{
			error = AudioSessionSetProperty(the_property, sizeof(value), &value);
			if(noErr != error)
			{
				NSLog(@"Error with AudioSessionSetProperty Bool: %s\n", CoreAudio_FourCCToString(error));
			}
			return;
		}
	}
	
#endif
}

#endif // Rtt_AUDIO_SESSION_PROPERTY

@end
