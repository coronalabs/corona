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

#ifndef _Rtt_IPhoneAudioSessionManagerImplementation_H__
#define _Rtt_IPhoneAudioSessionManagerImplementation_H__

#import <Foundation/Foundation.h>
#include <AudioToolbox/AudioToolbox.h>

@interface Rtt_IPhoneAudioSessionManagerImplementation : NSObject
{
@private
	BOOL audioSessionActive;
    BOOL usingManualSessionManagment;
	UInt32 savedPreviousSessionCategory; // only used if not using manual session management 
	UInt32 savedBackgroundPreviousSessionCategory; // only used if not using manual session management 

	BOOL inInterruption; // This is used to track if we are in a real interruption event. The Cocoa API is combined with too many additional cases.
	BOOL needToRestoreFromBackgroundOrScreenLock;
}

/** 
 * For calling AudioSessionSetActive().
 * Apple doesn't have a GetSessionActive call so we must track this state ourselves.
 * Corona must always go through this API for our values to remain consistent.
 * Note that setAudioSessionActive returns a bool which returns false if setting the audio session triggers an error.
 * This is to workaround:
 	radar://12022630 Core Audio breaks when leaving an app via Notification Center and then returning
	radar://12022774 Redundant WillResignActive/BecomeActive when leaving app via Notification Center
	radar://12022812 AudioSessionSetActive should not return errors if setting to false when already false
	radar://12022855 Need API: AudioSessionGetActive() to balance AudioSessionSetActive
	radar://12022872 Want Audio Session Interruptions to trigger for all events that require it
	radar://12022894 Want automatic handling for OpenAL/Core Audio interruptions like AVFoundation
 */
- (bool) setAudioSessionActive:(BOOL)is_active;
- (BOOL) audioSessionActive;
@property(assign, nonatomic, readonly) BOOL inInterruption;

+ (id) sharedManager;


/**
 * Will set the recording mode if not active.
 * Current behavior will reactivate automatic mode if not set.
 * This may change in the future.
 */
- (void) setImplicitRecordingModeIfNecessary; 
- (void) restoreAudioSessionCategoryIfNecessary; ///< will restore the previous audio session if in automatic mode

- (UInt32) audioSessionCategory;
- (void) setAudioSessionCategory:(UInt32)category; ///< tries to preserve automatic session management
- (void) setManualAudioSessionCategory:(UInt32)category; ///< Disables automatic session management

- (void) prepareAudioSystemForMoviePlayback;
- (void) restoreAudioSystemFromMoviePlayback;

/**
 * Reads the Info.plist for the UIBackgroundModes to find if audio is set.
 */
- (BOOL) supportsBackgroundAudio;

/**
 * True on kAudioSessionCategory_MediaPlayback, kAudioSessionCategory_RecordAudio, kAudioSessionCategory_PlayAndRecord
 */
- (BOOL) allowsAudioDuringScreenLock;

#ifdef Rtt_AUDIO_SESSION_PROPERTY

- (char) propertyType:(AudioSessionPropertyID)the_property;
- (UInt32) propertyUInt32:(AudioSessionPropertyID)the_property;
- (Float32) propertyFloat32:(AudioSessionPropertyID)the_property;
- (Float64) propertyFloat64:(AudioSessionPropertyID)the_property;
- (BOOL) propertyBool:(AudioSessionPropertyID)the_property;

- (void) setProperty:(AudioSessionPropertyID)the_property valueUInt32:(UInt32)value;
- (void) setProperty:(AudioSessionPropertyID)the_property valueFloat32:(Float32)value;
- (void) setProperty:(AudioSessionPropertyID)the_property valueFloat64:(Float64)value;
- (void) setProperty:(AudioSessionPropertyID)the_property valueBool:(BOOL)value;

#endif // Rtt_AUDIO_SESSION_PROPERTY

@end


#endif // _Rtt_IPhoneAudioSessionManagerImplementation_H__

