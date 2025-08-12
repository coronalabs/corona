//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "Rtt_IPhoneConstants.h"

#include "Core/Rtt_Build.h"

#include "Rtt_IPhoneVideoPlayer.h"

#include "Rtt_IPhonePlatformBase.h"
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
	[fMoviePlayerViewController autorelease];
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
	
    UIViewController *vc = [((IPhonePlatformBase*)&GetSessionRuntime()->Platform())->GetView() delegate];
	
    [vc presentViewController:fMoviePlayerViewController animated:YES completion:^{
		[fMoviePlayerViewController addObserver:fMovieObserver forKeyPath:@"view.frame" options:(NSKeyValueObservingOptionNew | NSKeyValueObservingOptionInitial) context:nil];
	}];
    fMoviePlayerViewController.view.frame = vc.view.frame;
#if defined(Rtt_IPHONE_ENV)
    fMoviePlayerViewController.updatesNowPlayingInfoCenter = NO;
#else
	if (@available(tvOS 11.0, *)) {
		fMoviePlayerViewController.playbackControlsIncludeInfoViews = YES;
	}
#endif
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

