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
#import <MediaPlayer/MediaPlayer.h>
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


- (void)loadStateDidChange:(NSNotification*)notification;
- (void)contentPreloadDidFinish:(NSNotification*)notification; // still need for legacy pre-3_2

- (void)didExitFullscreen:(NSNotification*)notification;
- (void)playbackDidFinish:(NSNotification*)notification; // still need for legacy pre-3_2

// - (void)scalingModeDidChange;

@end


// iOS 6 started allowing touch events to get through to our main view controller which was breaking assumptions that
// users wouldn't get touch events while the movie was playing.
// This subclass is a quick solution to override the touch events so nothing gets passed through.
// Strangely, the built-in HUD still works to toggle when touched so Apple must be doing something weird under the hood.
// But this works to our advantage. But be warned that Apple could break this in the future.
@interface CoronaMPMoviePlayerViewController : MPMoviePlayerViewController
@end

@implementation CoronaMPMoviePlayerViewController

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	
}
- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	
}
- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	
}
- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	
}
-(BOOL)allowOrientationOverride
{
    return YES;
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    return YES;
}

-(BOOL) shouldAutorotate
{
    return YES;
}
- (NSUInteger)supportedInterfaceOrientations
{
    return UIInterfaceOrientationMaskAll;
}


@end

@implementation IPhoneMovieObserver

@synthesize callback;
- (void)loadStateDidChange:(NSNotification*)notification
{
	callback->LoadStateDidChange();
}

- (void)contentPreloadDidFinish:(NSNotification*)notification
{
	callback->LoadStateDidChange();
}

- (void)didExitFullscreen:(NSNotification*)notification
{
	callback->DidExitFullscreen();
}

- (void)playbackDidFinish:(NSNotification*)notification
{
	callback->PlaybackDidFinish();
}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

IPhoneVideoPlayer::IPhoneVideoPlayer( const ResourceHandle<lua_State> & handle )
:	PlatformVideoPlayer( handle ),
	fMovieController( nil ),
	fMovieObserver( [[IPhoneMovieObserver alloc] init] )
	,fMoviePlayerViewController( nil )
	,fMoviePlayerViewControllerIsPushed( false )
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
	NSNotificationCenter* notifier = [NSNotificationCenter defaultCenter];

	[notifier removeObserver:fMovieObserver name:MPMoviePlayerLoadStateDidChangeNotification object:fMovieController];
	[notifier removeObserver:fMovieObserver name:MPMoviePlayerDidExitFullscreenNotification object:fMovieController];			
	[notifier removeObserver:fMovieObserver name:MPMoviePlayerPlaybackDidFinishNotification object:fMovieController];
	fMovieController = nil;
	[fMoviePlayerViewController release];
	fMoviePlayerViewController = nil;
	
	// We set the kDoNotSuspend property in Rtt_LuaLibMedia before calling play.
	// This unsets the property.
	SetProperty( PlatformModalInteraction::kDoNotSuspend, false );

}

bool
IPhoneVideoPlayer::Load( const char* path, bool isRemote )
{
	if ( fMovieController )
	{
		Cleanup();
	}

	NSURL *url = isRemote
		? [[NSURL alloc] initWithString:[NSString stringWithUTF8String:path]]
		: (NSURL*)CFURLCreateFromFileSystemRepresentation( NULL, (const UInt8*)path, strlen(path), false );

	fMoviePlayerViewController = [[CoronaMPMoviePlayerViewController alloc] initWithContentURL:url];
	fMovieController = fMoviePlayerViewController.moviePlayer;
	// don't set controlStyle. Seems to interfere with view controller.
	// What happens is the moviecontroller bar appears instead of the movieviewcontroller bar.
	// quiting the moviecontroller causes the viewcontroller bar to reappear, and then vice-versa.
	// In both cases, the controls don't seem to correctly control their properties.
	
	[url release];


	NSNotificationCenter* notifier = [NSNotificationCenter defaultCenter];

	[notifier addObserver:fMovieObserver selector:@selector(loadStateDidChange:) name:MPMoviePlayerLoadStateDidChangeNotification object:fMovieController];
	[notifier addObserver:fMovieObserver selector:@selector(didExitFullscreen:) name:MPMoviePlayerDidExitFullscreenNotification object:fMovieController];
	[notifier addObserver:fMovieObserver selector:@selector(playbackDidFinish:) name:MPMoviePlayerPlaybackDidFinishNotification object:fMovieController];
	

	return fMovieController;
}

void
IPhoneVideoPlayer::Play()
{
	// This is a hack to get around an Apple bug.
	// MPMoviePlayer seems to be screwing up the OpenAL context when mixing modes are not set.
	// The workaround is to disable the OpenAL context while the movie is playing.
	//	PlatformAudioSessionManager::Get()->PrepareAudioSystemForMoviePlayback();
	PlatformAudioSessionManager::SharedInstance()->PrepareAudioSystemForMoviePlayback();
	
	AppDelegate *delegate = (AppDelegate*)[UIApplication sharedApplication].delegate;
//		[delegate.viewController.view addSubview:fMoviePlayerViewController.view];
//		[fMovieController setFullscreen:YES animated:YES];
	if(false == fMoviePlayerViewControllerIsPushed)
	{
		[delegate.viewController presentMoviePlayerViewControllerAnimated:fMoviePlayerViewController];
		fMoviePlayerViewControllerIsPushed = true;
	}
	else
	{
		[fMovieController play];
	}

}
/*
void
IPhoneVideoPlayer::Stop()
{
	[fMovieController stop];
}
*/
void
IPhoneVideoPlayer::SetProperty( U32 mask, bool newValue )
{
	Super::SetProperty( mask, newValue );

	if ( fMovieController )
	{
		// Warning: This is broken. Setting to MPMovieControlStyleNone seems to work, but setting 
		// to MPMovieControlStyleDefault will break things.
		// What happens is the moviecontroller bar appears instead of the movieviewcontroller bar.
		// quiting the moviecontroller causes the viewcontroller bar to reappear, and then vice-versa.
		// In both cases, the controls don't seem to correctly control their properties.
		// So by default, we create a new viewcontroller and don't set anything.
		// Don't explicitly set this to MPMovieControlStyleDefault
		// or you get the fighting.
		if ( (mask & kShowControls) && !newValue )
		{
			// Use controlStyle if absolutely necessarly.
			// Warning: This is broken. Setting to MPMovieControlStyleNone seems to work, but setting 
			// to MPMovieControlStyleDefault will break things.
			// What happens is the moviecontroller bar appears instead of the movieviewcontroller bar.
			// quiting the moviecontroller causes the viewcontroller bar to reappear, and then vice-versa.
			// In both cases, the controls don't seem to correctly control their properties.
			fMovieController.controlStyle = ( newValue ? MPMovieControlStyleDefault : MPMovieControlStyleNone );
		}
	}
}

void
IPhoneVideoPlayer::LoadStateDidChange()
{

	MPMovieLoadState loadState = fMovieController.loadState;
	if ( MPMovieLoadStatePlayable == loadState || MPMovieLoadStatePlaythroughOK == loadState )
	{		
		// For now, if we don't explicitly call Play(), then automatically start
		// playing when movie preload is complete
		Play();
		
	}

}

void
IPhoneVideoPlayer::DidExitFullscreen()
{
	// This undoes the hack where we may have suspended the OpenAL context to get around an Apple bug.
	// This must be called before the Lua callback in case the user tries to play audio in the callback.
	// FIXME: This code needs to be removed and only called from PlaybackDidFinish if/when we decide to support non-fullscreen video.
	// Since we quit here, we need this code.
//	PlatformAudioSessionManager::Get()->RestoreAudioSystemFromMoviePlayback();
	PlatformAudioSessionManager::SharedInstance()->RestoreAudioSystemFromMoviePlayback();

	DidDismiss( NULL, NULL );
	Cleanup();
}

void
IPhoneVideoPlayer::PlaybackDidFinish()
{
	AppDelegate *delegate = (AppDelegate*)[UIApplication sharedApplication].delegate;
	[delegate.viewController dismissMoviePlayerViewControllerAnimated];
	fMoviePlayerViewControllerIsPushed = false;

	// This undoes the hack where we may have suspended the OpenAL context to get around an Apple bug.
	// This must be called before the Lua callback in case the user tries to play audio in the callback.
//	PlatformAudioSessionManager::Get()->RestoreAudioSystemFromMoviePlayback();
	PlatformAudioSessionManager::SharedInstance()->RestoreAudioSystemFromMoviePlayback();
	
	DidDismiss( NULL, NULL );
	Cleanup();
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

