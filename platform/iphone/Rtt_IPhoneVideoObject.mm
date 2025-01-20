//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_IPhoneVideoObject.h"

#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>

#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibMedia.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_LuaResource.h"
#include "Rtt_MPlatform.h"
#include "Core/Rtt_String.h"
#include "Rtt_Event.h"
//#include "Renderer/Rtt_RenderTypes.h"

#ifdef Rtt_DEBUG
	// Used in asserts in Initialize()
	#include "Display/Rtt_Display.h"
	#include "Rtt_Runtime.h"
#endif

// ----------------------------------------------------------------------------

static NSString * const kStatusKey = @"status";

static void *Rtt_AVPlayerViewStatusContext = & Rtt_AVPlayerViewStatusContext;

// ----------------------------------------------------------------------------

@interface Rtt_AVPlayerView : UIView
{
	Rtt::IPhoneVideoObject *fOwner;
	NSURL *fUrl;
	BOOL touchTogglesPlay;
	BOOL fIsMuted;
	BOOL fIsReadyToPlay;
	BOOL fDidEnd;
}

@property(nonatomic,assign,getter=owner,setter=setOwner:) Rtt::IPhoneVideoObject *fOwner;
@property(nonatomic, retain) AVPlayer *player;
@property(nonatomic, assign) BOOL touchTogglesPlay;

- (id)initWithFrame:(CGRect)rect;

- (void)load:(NSURL*)url;
- (void)play;
- (void)pause;
- (bool)isPaused;
- (void)seek:(CMTime)t;

- (void)setPlayer:(AVPlayer*)player;

- (NSString*)videoFillMode;
- (void)setVideoFillMode:(NSString *)fillMode;

- (BOOL)isMuted;
- (void)setMuted:(BOOL)newValue;

@end

@implementation Rtt_AVPlayerView

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
		self.clipsToBounds = YES;
	}

	return self;
}

- (void)unload
{
//	[self.player.currentItem removeObserver:self forKeyPath:@"tracks"];
	[self.player.currentItem removeObserver:self forKeyPath:kStatusKey];
	[[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)dealloc
{
	[self unload];

	[fUrl release];

	[super dealloc];
}



- (void)load:(NSURL*)url
{
	if ( fUrl != url )
	{
		[self unload];

		[fUrl release];
		fUrl = [url copy];

		AVPlayer *p = [AVPlayer playerWithURL:fUrl];
		[self setPlayer:p];

		AVPlayerItem *currentItem = self.player.currentItem;
		[currentItem addObserver:self
			forKeyPath:kStatusKey
			options:NSKeyValueObservingOptionInitial | NSKeyValueObservingOptionNew
			context:Rtt_AVPlayerViewStatusContext];
		[[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(didVideoEnd:) name:AVPlayerItemDidPlayToEndTimeNotification object:currentItem];
	}
}

- (void)dispatchReadyToPlay
{
	using namespace Rtt;

	VideoEvent event( VideoEvent::kReady );
	fOwner->DispatchEventWithTarget( event );
}

- (void)dispatchFailed
{
	using namespace Rtt;

	VideoEvent event( VideoEvent::kFailed );
	fOwner->DispatchEventWithTarget( event );
}

- (void)didVideoEnd:(NSNotification*)notification
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

- (AVPlayer*)player
{
	return [(AVPlayerLayer*)[self layer] player];
}

- (void)setPlayer:(AVPlayer*)player
{
	[(AVPlayerLayer*)[self layer] setPlayer:player];
}

- (NSString*)videoFillMode
{
	AVPlayerLayer *playerLayer = (AVPlayerLayer*)[self layer];
	return playerLayer.videoGravity;
}

/* Specifies how the video is displayed within a player layer’s bounds. 
	(AVLayerVideoGravityResizeAspect is default) */
- (void)setVideoFillMode:(NSString *)fillMode
{
	AVPlayerLayer *playerLayer = (AVPlayerLayer*)[self layer];
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

- (void)observeValueForKeyPath:(NSString*)keyPath 
			ofObject:(id)object
			change:(NSDictionary*)change 
			context:(void*)context
{
	/*
	if ( [keyPath isEqualToString:@"tracks"] )
	{
		[self updateAudioTracks];
	}
	*/

	// AVPlayerItem "status" property value observer
	if ( context == Rtt_AVPlayerViewStatusContext )
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

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

IPhoneVideoObject::IPhoneVideoObject( const Rect& bounds )
:	Super( bounds )
{
}

IPhoneVideoObject::~IPhoneVideoObject()
{
	Rtt_AVPlayerView *v = (Rtt_AVPlayerView*)GetView();
	v.owner = NULL;
}

bool
IPhoneVideoObject::Initialize()
{
	Rtt_ASSERT( ! GetView() );

	//AppDelegate *delegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
	//UIViewController *controller = delegate.viewController;

	//Rtt_ASSERT( delegate.runtime->GetDisplay().PointsWidth() == [UIScreen mainScreen].bounds.size.width );
	//Rtt_ASSERT( delegate.runtime->GetDisplay().PointsHeight() == [UIScreen mainScreen].bounds.size.height );

	Rect screenBounds;
	GetScreenBounds( screenBounds );
	CGRect r = CGRectMake( screenBounds.xMin, screenBounds.yMin, screenBounds.Width(), screenBounds.Height() );
	Rtt_AVPlayerView *v = [[Rtt_AVPlayerView alloc] initWithFrame:r];
	v.owner = this;

	[GetCoronaView() addSubview:v];

	Super::InitializeView( v );
	[v release];

	return v;
}

const LuaProxyVTable&
IPhoneVideoObject::ProxyVTable() const
{
	return PlatformDisplayObject::GetVideoObjectProxyVTable();
}

// view:load( path [, baseSource ] )
int
IPhoneVideoObject::Load( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
	IPhoneVideoObject *o = (IPhoneVideoObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		const MPlatform& platform = LuaContext::GetPlatform( L );

		bool isRemote = false;
		String filePath( & platform.GetAllocator() );
		int nextArg = 2;
		const char *path = LuaLibMedia::GetLocalOrRemotePath( L, nextArg, filePath, isRemote );

		NSURL *url = isRemote
			? [[NSURL alloc] initWithString:[NSString stringWithUTF8String:path]]
			: (NSURL*)CFURLCreateFromFileSystemRepresentation( NULL, (const UInt8*)path, strlen(path), false );
		Rtt_AVPlayerView *view = (Rtt_AVPlayerView*)o->GetView();
		[view load:url];

		[url release];
	}

	return 0;
}

// view:play()
int
IPhoneVideoObject::Play( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
	IPhoneVideoObject *o = (IPhoneVideoObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		Rtt_AVPlayerView *view = (Rtt_AVPlayerView*)o->GetView();
		[view play];
	}

	return 0;
}

// view:pause()
int
IPhoneVideoObject::Pause( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
	IPhoneVideoObject *o = (IPhoneVideoObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		Rtt_AVPlayerView *view = (Rtt_AVPlayerView*)o->GetView();
		[view pause];
	}

	return 0;
}

// view:seek( t )
int
IPhoneVideoObject::Seek( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetVideoObjectProxyVTable();
	IPhoneVideoObject *o = (IPhoneVideoObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		Rtt_AVPlayerView *view = (Rtt_AVPlayerView*)o->GetView();

		CMTime t = kCMTimeZero;
		if ( lua_isnumber( L, 2 ) )
		{
			// TODO: How to choose this properly?
			const int32_t kPreferredTimeScale = 600;

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

static NSString*
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

static const char*
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
IPhoneVideoObject::ValueForKey( lua_State *L, const char key[] ) const
{
	Rtt_ASSERT( key );

	int result = 1;

	Rtt_AVPlayerView *o = (Rtt_AVPlayerView*)GetView();
	
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
IPhoneVideoObject::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	Rtt_ASSERT( key );

	bool result = true;

	if ( strcmp( "isToggleEnabled", key ) == 0 )
	{
		// TODO: Should we just enable touch listener delivery instead of this?
		Rtt_ASSERT_NOT_IMPLEMENTED();
		Rtt_AVPlayerView *o = (Rtt_AVPlayerView*)GetView();
		o.touchTogglesPlay = lua_toboolean( L, valueIndex );
	}
	else if ( strcmp( "isMuted", key ) == 0 )
	{
		Rtt_AVPlayerView *o = (Rtt_AVPlayerView*)GetView();
		[o setMuted:lua_toboolean( L, valueIndex )];
	}
	else if ( strcmp( "fillMode", key ) == 0 )
	{
		Rtt_AVPlayerView *o = (Rtt_AVPlayerView*)GetView();

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
IPhoneVideoObject::GetNativeTarget() const
{
	Rtt_AVPlayerView *o = (Rtt_AVPlayerView*)GetView();

	return o.player;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

