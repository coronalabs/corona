//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import <AVKit/AVKit.h>

#include "Rtt_MacVideoPlayer.h"
#import "AppDelegate.h"
#import "GLView.h"

@interface Rtt_VideoPlayerView ()

@property(nonatomic, readwrite, assign) Rtt::MacVideoPlayer *owner;

@end

// ----------------------------------------------------------------------------

@implementation Rtt_VideoPlayerView

@synthesize owner;

- (id) initWithFrame:(NSRect)rect url:(NSURL *)movieURL
{
	self = [super initWithFrame:rect];

	if ( self )
	{
		self.player = [AVPlayer playerWithURL:movieURL];

		// Subscribe to the AVPlayerItem's DidPlayToEndTime notification.
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(itemDidFinishPlaying:) name:AVPlayerItemDidPlayToEndTimeNotification object:self.player.currentItem];
	}

	return self;
}

 -(void) dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];

	[super dealloc];
}

- (void)itemDidFinishPlaying:(NSNotification *) notification
{
	owner->HandleEndOfVideo();
}

- (void) recomputeTransformCallback
{
	owner->RecomputeFromRotationOrScale();	
}

- (void)mouseDown:(NSEvent*)event
{
	// Ignore
}

- (void)mouseUp:(NSEvent*)event
{
	if (! owner->IsSuspended())
	{
		// Pause/resume the video if the mouse is clicked on it
		if ([self.player rate] == 0.0)
		{
			[self.player play];
		}
		else
		{
			[self.player pause];
		}
	}
}

- (void)setSuspended:(BOOL)newValue
{
	owner->SetSuspended(newValue);
}

@end


// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

MacVideoPlayer::MacVideoPlayer( const ResourceHandle<lua_State> & handle )
:	PlatformVideoPlayer( handle ),
	fMovieView( nil ),
	fNeedsRecompute( false ),
	fIsPlaying( false ),
	fIsSuspended( false )
{
}

MacVideoPlayer::~MacVideoPlayer()
{
	Close();
}

bool
MacVideoPlayer::Load( const char* filepath, bool isRemote )
{
	if (nil != fMovieView)
	{
		Close();
	}

	NSString *filepathStr = [NSString stringWithExternalString:filepath];
	NSURL *filepathURL = nil;

	if (isRemote)
	{
		filepathURL = [NSURL URLWithString:filepathStr];
	}
	else
	{
		filepathURL = [NSURL fileURLWithPath:filepathStr isDirectory:NO];
	}

	AppDelegate* delegate = (AppDelegate*)[NSApp delegate];
	NSView* hostview = [delegate layerHostView];
	
	fMovieView = [[Rtt_VideoPlayerView alloc] initWithFrame:[hostview bounds] url:filepathURL];
	[fMovieView setOwner:this];

	fTimeObserverToken = [[fMovieView player] addPeriodicTimeObserverForInterval:CMTimeMakeWithSeconds(0.5, NSEC_PER_SEC) queue:NULL usingBlock:^(CMTime time) {

		HandlePeriodicTimeObserver();

	}];

	return true;
}

void
MacVideoPlayer::Close()
{
	[[fMovieView player] removeTimeObserver:fTimeObserverToken];

	fIsPlaying = false;

	[fMovieView removeFromSuperview];

	// This prevents a possible crash due to different object lifetimes between Cocoa and Lua
	[[NSNotificationCenter defaultCenter] removeObserver:fMovieView];

	[fMovieView release];
	fMovieView = nil;
}

void
MacVideoPlayer::Play()
{
	if( IsSuspended() )
	{
		return;
	}

	AppDelegate* delegate = (AppDelegate*)[NSApp delegate];
	GLView* view = [delegate layerHostView];
	
	[view addSubview:fMovieView];
	
	fIsPlaying = true;
	[[fMovieView player] play];
}

void
MacVideoPlayer::HandlePeriodicTimeObserver()
{
	// This allows us to track user interactions with the player controls
	if ([[fMovieView player] rate] == 0.0)
	{
		fIsPlaying = false;
	}
	else
	{
		fIsPlaying = true;
	}
}

void MacVideoPlayer::HandleEndOfVideo()
{
	DidDismiss( NULL, NULL );
	Close();
}

void
MacVideoPlayer::SetProperty( U32 mask, bool newValue )
{
	Super::SetProperty( mask, newValue );

	if ( fMovieView )
	{
		if ( IsProperty( kShowControls ) )
		{
			fMovieView.controlsStyle = AVPlayerViewControlsStyleInline;
		}
		else
		{
			fMovieView.controlsStyle = AVPlayerViewControlsStyleNone;
		}
	}
}

void
MacVideoPlayer::SetNeedsRecomputeFromRotationOrScale()
{
	if ( fNeedsRecompute )
	{
		return;
	}

	fNeedsRecompute = true;

	[fMovieView recomputeTransformCallback];
}

void
MacVideoPlayer::RecomputeFromRotationOrScale()
{
	AppDelegate* delegate = (AppDelegate*)[NSApp delegate];
	NSView* hostview = [delegate layerHostView];

	NSRect frame = [hostview frame];

	[fMovieView setFrameSize:frame.size];
	
	fNeedsRecompute = false;
}

void
MacVideoPlayer::SetSuspended( bool shouldsuspend )
{
	fIsSuspended = shouldsuspend;

	if ( shouldsuspend )
	{
		fWasPlaying = fIsPlaying;
		[[fMovieView player] setRate:0.0];
	}
	else
	{
		if ( fWasPlaying )
		{
			fIsPlaying = true;
			[[fMovieView player] play];
		}
	}
}

bool
MacVideoPlayer::IsSuspended() const
{
	return fIsSuspended;
}


void
MacVideoPlayer::SetPlayingFlag( bool isplaying )
{
	fIsPlaying = isplaying;
}


bool
MacVideoPlayer::IsPlayingFlag() const
{
	return fIsPlaying;
}
// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

