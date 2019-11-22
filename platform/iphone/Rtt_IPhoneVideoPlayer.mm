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

#import "Rtt_IPhoneConstants.h"

#include "Core/Rtt_Build.h"

#include "Rtt_IPhoneVideoPlayer.h"

#import "AppDelegate.h"
#import <AVKit/AVKit.h>
#include "Rtt_LuaContext.h"
#include "Rtt_PlatformAudioSessionManager.h"
#include "Rtt_Runtime.h"
// ----------------------------------------------------------------------------

// Callback/Notification glue code
@interface IPhoneMovieObserver : NSObject
{
	Rtt::IPhoneVideoPlayer* callback;
}

@property (nonatomic, readwrite) Rtt::IPhoneVideoPlayer* callback;

- (void)playbackDone:(NSNotification*)notification; // still need for legacy pre-3_2

// - (void)scalingModeDidChange;

@end
@implementation IPhoneMovieObserver
@synthesize callback;
- (void)playbackDone:(NSNotification*)notification
{
	callback->PlaybackDone();
}

- (void)observeValueForKeyPath:(NSString *)keyPath
					  ofObject:(id)object
						change:(NSDictionary<NSKeyValueChangeKey, id> *)change
					   context:(void *)context {
	if([keyPath isEqualToString:@"view.frame"]) {
		UIViewController *vc = (UIViewController*)object;
		if(vc.isBeingDismissed) {
			callback->PlaybackDone();
		}
	}
	else if([keyPath isEqualToString:@"currentItem.status"]) {
		if([change[NSKeyValueChangeNewKey] intValue] == AVPlayerItemStatusFailed) {
			callback->PlaybackDone();
		}
	}
	else if([keyPath isEqualToString:@"status"]) {
		if([change[NSKeyValueChangeNewKey] intValue] == AVPlayerStatusFailed) {
			callback->PlaybackDone();
		}
	}
}

@end

namespace Rtt
{

// ----------------------------------------------------------------------------

IPhoneVideoPlayer::IPhoneVideoPlayer( const ResourceHandle<lua_State> & handle )
:	PlatformVideoPlayer( handle )
    , fMovieObserver( [[IPhoneMovieObserver alloc] init] )
	, fMoviePlayerViewController( nil )
{
	fMovieObserver.callback = this;
}

IPhoneVideoPlayer::~IPhoneVideoPlayer()
{
	Cleanup();
	[fMovieObserver release];
}

void
IPhoneVideoPlayer::Cleanup()
{
	[[NSNotificationCenter defaultCenter] removeObserver:fMovieObserver];

	[fMoviePlayerViewController dismissViewControllerAnimated:YES completion:nil];
	[fMoviePlayerViewController release];
	fMoviePlayerViewController = nil;
	
	// We set the kDoNotSuspend property in Rtt_LuaLibMedia before calling play.
	// This unsets the property.
	SetProperty( PlatformModalInteraction::kDoNotSuspend, false );
}

bool
IPhoneVideoPlayer::Load( const char* path, bool isRemote )
{
	if ( fMoviePlayerViewController )
	{
		Cleanup();
	}

	NSURL *url = isRemote
		? [[NSURL alloc] initWithString:[NSString stringWithUTF8String:path]]
		: (NSURL*)CFURLCreateFromFileSystemRepresentation( NULL, (const UInt8*)path, strlen(path), false );


    fMoviePlayerViewController = [[AVPlayerViewController alloc]init];
    fMoviePlayerViewController.player = [AVPlayer playerWithURL:url];
    AVPlayerItem *item = fMoviePlayerViewController.player.currentItem;
	[[NSNotificationCenter defaultCenter] addObserver:fMovieObserver selector:@selector(playbackDone:) name:AVPlayerItemDidPlayToEndTimeNotification object:item];
	[[NSNotificationCenter defaultCenter] addObserver:fMovieObserver selector:@selector(playbackDone:) name:AVPlayerItemFailedToPlayToEndTimeNotification object:item];
	
	[fMoviePlayerViewController.player addObserver:fMovieObserver forKeyPath:@"currentItem.status" options:(NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld) context:nil];
	[fMoviePlayerViewController.player addObserver:fMovieObserver forKeyPath:@"status" options:(NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld) context:nil];

    if(!item) {
		[[NSOperationQueue mainQueue] addOperationWithBlock:^{
			PlaybackDone();
		}];
	}

	[url release];

	return item;
}

void
IPhoneVideoPlayer::Play()
{
	// This is a hack to get around an Apple bug.
	// MPMoviePlayer seems to be screwing up the OpenAL context when mixing modes are not set.
	// The workaround is to disable the OpenAL context while the movie is playing.
	//	PlatformAudioSessionManager::Get()->PrepareAudioSystemForMoviePlayback();
	PlatformAudioSessionManager::SharedInstance()->PrepareAudioSystemForMoviePlayback();
	
	UIViewController *vc = ((AppDelegate*)[UIApplication sharedApplication].delegate).viewController;
    [vc presentViewController:fMoviePlayerViewController animated:YES completion:^{
		[fMoviePlayerViewController addObserver:fMovieObserver forKeyPath:@"view.frame" options:(NSKeyValueObservingOptionNew | NSKeyValueObservingOptionInitial) context:nil];
	}];
    fMoviePlayerViewController.view.frame = vc.view.frame;
    fMoviePlayerViewController.updatesNowPlayingInfoCenter = NO;
//    if([fMoviePlayerViewController respondsToSelector:@selector(setPlaybackControlsIncludeInfoViews:)]) {
//		[fMoviePlayerViewController setPlaybackControlsIncludeInfoViews:NO];
//    }
    [fMoviePlayerViewController.player play];
}

void
IPhoneVideoPlayer::SetProperty( U32 mask, bool newValue )
{
	Super::SetProperty( mask, newValue );

	if ( fMoviePlayerViewController )
	{
		if(kShowControls == (mask & kShowControls))
		{
			fMoviePlayerViewController.showsPlaybackControls = newValue;
		}
	}
}

void
IPhoneVideoPlayer::PlaybackDone()
{
	if(fMoviePlayerViewController) {
		PlatformAudioSessionManager::SharedInstance()->RestoreAudioSystemFromMoviePlayback();
		DidDismiss( NULL, NULL );
		Cleanup();
	}
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

