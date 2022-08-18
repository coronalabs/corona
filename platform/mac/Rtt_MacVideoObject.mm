//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import "AppDelegate.h"

#include "Rtt_MacVideoObject.h"

#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibMedia.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_LuaResource.h"
#include "Rtt_MPlatform.h"
#include "Core/Rtt_String.h"
#include "Rtt_Event.h"

// ----------------------------------------------------------------------------

static NSString *const kStatusKey = @"status";

static void *Rtt_VideoObjectViewStatusContext = & Rtt_VideoObjectViewStatusContext;

// ----------------------------------------------------------------------------

@implementation Rtt_VideoObjectView

@synthesize fOwner;
@synthesize touchTogglesPlay;

+ (Class)layerClass
{
	return [AVPlayerLayer class];
}

- (id)initWithFrame:(CGRect)rect
{
	self = [super initWithFrame:rect];
	if ( self )
	{
		fOwner = NULL;
		fUrl = nil;
		touchTogglesPlay = NO;
		fIsMuted = NO;
		fIsReadyToPlay = NO;
		fDidEnd = NO;
		fWasPlaying = NO;
	}

	return self;
}

- (void)dealloc
{
	[self unload];

	[fUrl release];

	[super dealloc];
}

- (void)unload
{
	//	[self.player.currentItem removeObserver:self forKeyPath:@"tracks"];
	[self.player.currentItem removeObserver:self forKeyPath:kStatusKey];
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)load:(NSURL *)url
{
	if ( fUrl != url )
	{
		if ( fUrl != nil )
		{
			[self unload];
		}

		[fUrl release];
		fUrl = [url copy];

		AVPlayer *p = [AVPlayer playerWithURL:fUrl];
		AVPlayerLayer *playerLayer = [AVPlayerLayer playerLayerWithPlayer:p];

		[self setLayer:playerLayer];

		AVPlayerItem *currentItem = self.player.currentItem;
		[currentItem addObserver:self
			forKeyPath:kStatusKey
			options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
			context:Rtt_VideoObjectViewStatusContext];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didVideoEnd:) name:AVPlayerItemDidPlayToEndTimeNotification object:currentItem];
	}
}

- (void)dispatchFailed
{
	using namespace Rtt;

	VideoEvent event( VideoEvent::kFailed );
	fOwner->DispatchEventWithTarget( event );
}

- (void)dispatchReadyToPlay
{
	using namespace Rtt;

	VideoEvent event( VideoEvent::kReady );
	fOwner->DispatchEventWithTarget( event );
}

- (void)didVideoEnd:(NSNotification *)notification
{
	using namespace Rtt;

	if ( [notification object] == self.player.currentItem )
	{
		if ( ! fDidEnd )
		{
			fDidEnd = YES;
			VideoEvent event( VideoEvent::kEnded );
			fOwner->DispatchEventWithTarget( event );
			fDidEnd = NO;
		}
	}
}

- (void)play
{
	if ( fDidEnd )
	{
		[self seek:kCMTimeZero];
		fDidEnd = NO;
	}

	[self.player play];
}

- (void)pause
{
	[self.player pause];
}

- (bool) isPaused
{
	float rate = [self.player rate];

	return (rate == 0.0);
}

- (void)seek:(CMTime)t
{
	// If the video had ended and the user sets the seek position, clear the flag because we don't play to rewind it in this case.
	if ( fDidEnd )
	{
		fDidEnd = NO;
	}

	[self.player seekToTime:t];
}

- (AVPlayer *)player
{
	// Under unknown circumstances we can get here when "self" is an instance of _NSViewBackingLayer
	// which does not have a member called "player" causing an exception (reported by DeskMetrics)
	if ([[self layer] isKindOfClass:[AVPlayerLayer class]])
	{
		return [(AVPlayerLayer *)[self layer] player];
	}
	else
	{
		return nil;
	}
}

- (NSString *)videoFillMode
{
	AVPlayerLayer *playerLayer = (AVPlayerLayer *)[self layer];
	return playerLayer.videoGravity;
}

/* Specifies how the video is displayed within a player layer’s bounds. 
	(AVLayerVideoGravityResizeAspect is default) */
- (void)setVideoFillMode:(NSString *)fillMode
{
	AVPlayerLayer *playerLayer = (AVPlayerLayer *)[self layer];
	playerLayer.videoGravity = fillMode;
}

- (BOOL)isMuted
{
	return fIsMuted;
}

- (void)updateAudioTracks
{
	NSArray *tracks = self.player.currentItem.tracks;
	for( NSInteger i = 0, iMax = [tracks count]; i < iMax; i++)
	{
		id item = [tracks objectAtIndex:i];
		if ( [[item assetTrack].mediaType isEqualToString:AVMediaTypeAudio] )
		{
			((AVPlayerItemTrack *)item).enabled = ! fIsMuted;
		}
	}
}

- (void)setMuted:(BOOL)newValue
{
	fIsMuted = newValue;
	[self updateAudioTracks];
}

- (void)observeValueForKeyPath:(NSString *)keyPath 
			ofObject:(id)object
			change:(NSDictionary *)change 
			context:(void *)context
{
	// AVPlayerItem "status" property value observer
	if ( context == Rtt_VideoObjectViewStatusContext )
	{
		AVPlayerStatus status = (AVPlayerStatus)[[change objectForKey:NSKeyValueChangeNewKey] integerValue];
		switch (status)
		{
			// Indicates that the status of the player is not yet known because 
			// it has not tried to load new media resources for playback
			case AVPlayerStatusUnknown:
			{
			}
			break;

			case AVPlayerStatusReadyToPlay:
			{
				if ( ! fIsReadyToPlay )
				{
					fIsReadyToPlay = YES;

					// Once the AVPlayerItem becomes ready to play, i.e. 
					// [playerItem status] == AVPlayerItemStatusReadyToPlay,
					// its duration can be fetched from the item.
					[self dispatchReadyToPlay];
					[self updateAudioTracks];
				}
			}
			break;

			case AVPlayerStatusFailed:
			{
				[self dispatchFailed];
			}
			break;
		}
	}
	else
	{
		[super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
	}
}

- (void)setSuspended:(BOOL)newValue
{
	if (newValue)
	{
		if ([self isPaused])
		{
			fWasPlaying = NO;
		}
		else
		{
			fWasPlaying = YES;
			[self pause];
		}
	}
	else
	{
		if (fWasPlaying)
		{
			[self play];
		}
	}
}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

MacVideoObject::MacVideoObject( const Rect& bounds )
:	Super( bounds )
{
}

MacVideoObject::~MacVideoObject()
{
	Rtt_VideoObjectView *v = (Rtt_VideoObjectView *)GetView();
	v.owner = NULL;
}

bool
MacVideoObject::Initialize()
{
	Rtt_ASSERT( ! GetView() );

	Rect screenBounds;
	GetScreenBounds( screenBounds );
	CGRect r = CGRectMake( screenBounds.xMin, screenBounds.yMin, screenBounds.Width(), screenBounds.Height() );
	Rtt_VideoObjectView *v = [[Rtt_VideoObjectView alloc] initWithFrame:r];
	v.owner = this;

	AppDelegate *delegate = (AppDelegate *)[NSApp delegate];
	GLView *view = [delegate layerHostView];
	[view addSubview:v];

	Super::InitializeView( v );
	[v release];

	return v;
}

const LuaProxyVTable&
MacVideoObject::ProxyVTable() const
{
	return PlatformDisplayObject::GetVideoObjectProxyVTable();
}

// view:load( path [, baseSource ] )
int
MacVideoObject::Load( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
	MacVideoObject *o = (MacVideoObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		const MPlatform& platform = LuaContext::GetPlatform( L );

		bool isRemote = false;
		String filePath( & platform.GetAllocator() );
		int nextArg = 2;
		const char *path = LuaLibMedia::GetLocalOrRemotePath( L, nextArg, filePath, isRemote );

		NSURL *url = isRemote
			? [[NSURL alloc] initWithString:[NSString stringWithUTF8String:path]]
			: (NSURL *)CFURLCreateFromFileSystemRepresentation( NULL, (const UInt8*)path, strlen(path), false );
		Rtt_VideoObjectView *view = (Rtt_VideoObjectView *)o->GetView();
		[view load:url];

		[url release];
	}

	return 0;
}

// view:play()
int
MacVideoObject::Play( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
	MacVideoObject *o = (MacVideoObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		Rtt_VideoObjectView *view = (Rtt_VideoObjectView *)o->GetView();
		[view play];
	}

	return 0;
}

// view:pause()
int
MacVideoObject::Pause( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
	MacVideoObject *o = (MacVideoObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		Rtt_VideoObjectView *view = (Rtt_VideoObjectView *)o->GetView();
		[view pause];
	}

	return 0;
}

// view:seek( t )
int
MacVideoObject::Seek( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
	MacVideoObject *o = (MacVideoObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		Rtt_VideoObjectView *view = (Rtt_VideoObjectView *)o->GetView();

		CMTime t = kCMTimeZero;
		if ( lua_isnumber( L, 2 ) )
		{
			const int32_t kPreferredTimeScale = view.player.currentItem.asset.duration.timescale;

			lua_Number value = lua_tonumber( L, 2 );
			t = CMTimeMakeWithSeconds( value, kPreferredTimeScale );
		}
		[view seek:t];
	}

	return 0;
}

// TODO: Move these into a cross-platform source file
static const char kZoomEven[] = "zoomEven";
static const char kZoomStretch[] = "zoomStretch";
static const char kLetterbox[] = "letterbox";

static NSString *
GravityForFill( const char *value )
{
	NSString *mode = AVLayerVideoGravityResizeAspect;
	if ( value )
	{
		if ( 0 == strcmp( value, kZoomEven ) )
		{
			mode = AVLayerVideoGravityResizeAspectFill;
		}
		else if ( 0 == strcmp( value, kZoomStretch ) )
		{
			mode = AVLayerVideoGravityResize;
		}
	}
	return mode;
}

static const char *
FillForGravity( NSString *value )
{
	const char *mode = kLetterbox;
	if ( value )
	{
		if ( [value isEqualToString:AVLayerVideoGravityResizeAspectFill] )
		{
			mode = kZoomEven;
		}
		else if ( [value isEqualToString:AVLayerVideoGravityResize] )
		{
			mode = kZoomStretch;
		}
	}
	return mode;
}

int
MacVideoObject::ValueForKey( lua_State *L, const char key[] ) const
{
	Rtt_ASSERT( key );

	int result = 1;

	Rtt_VideoObjectView *o = (Rtt_VideoObjectView *)GetView();
	
	/*
	else if ( strcmp( "autoCancel", key ) == 0 )
	{
	}
	*/
	if ( strcmp( "currentTime", key ) == 0 )
	{
		CMTime t = o.player.currentTime;
		lua_pushnumber( L, CMTimeGetSeconds( t ) );
	}
	else if ( strcmp( "totalTime", key ) == 0 )
	{
		AVPlayerItem *item = o.player.currentItem;
		CMTime t = [item duration];
		lua_pushnumber( L, CMTimeGetSeconds( t ) );
	}
	else if ( strcmp( "isMuted", key ) == 0 )
	{
		lua_pushboolean( L, [o isMuted] );
	}
	else if ( strcmp( "isPaused", key ) == 0 )
	{
		lua_pushboolean( L, [o isPaused] );
	}
	else if ( strcmp( "fillMode", key ) == 0 )
	{
		NSString *mode = [o videoFillMode];
		lua_pushstring( L, FillForGravity( mode ) );
	}
	else if ( strcmp( "load", key ) == 0 )
	{
		lua_pushcfunction( L, Load );
	}
	else if ( strcmp( "play", key ) == 0 )
	{
		lua_pushcfunction( L, Play );
	}
	else if ( strcmp( "pause", key ) == 0 )
	{
		lua_pushcfunction( L, Pause );
	}
	else if ( strcmp( "seek", key ) == 0 )
	{
		lua_pushcfunction( L, Seek );
	}
	else if ( strcmp( "isToggleEnabled", key ) == 0 )
	{
		lua_pushboolean( L, o.touchTogglesPlay );
	}
	else
	{
		result = Super::ValueForKey( L, key );
	}

	return result;
}

bool
MacVideoObject::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	Rtt_ASSERT( key );

	bool result = true;

	if ( strcmp( "isToggleEnabled", key ) == 0 )
	{
		// TODO: Should we just enable touch listener delivery instead of this?
		Rtt_ASSERT_NOT_IMPLEMENTED();
		Rtt_VideoObjectView *o = (Rtt_VideoObjectView *)GetView();
		o.touchTogglesPlay = lua_toboolean( L, valueIndex );
	}
	else if ( strcmp( "isMuted", key ) == 0 )
	{
		Rtt_VideoObjectView *o = (Rtt_VideoObjectView *)GetView();
		[o setMuted:lua_toboolean( L, valueIndex )];
	}
	else if ( strcmp( "fillMode", key ) == 0 )
	{
		Rtt_VideoObjectView *o = (Rtt_VideoObjectView *)GetView();

		const char *value = lua_tostring( L, valueIndex );
		NSString *mode = GravityForFill( value );
		[o setVideoFillMode:mode];
	}
	else if ( strcmp( "load", key ) == 0
		 || strcmp( "play", key ) == 0
		 || strcmp( "pause", key ) == 0
		 || strcmp( "isPaused", key ) == 0
		 || strcmp( "seek", key ) == 0 )
	{
		// no-op
	}
	else
	{
		result = Super::SetValueForKey( L, key, valueIndex );
	}
		
	return result;
}

id
MacVideoObject::GetNativeTarget() const
{
	Rtt_VideoObjectView *o = (Rtt_VideoObjectView *)GetView();

	return o.player;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

