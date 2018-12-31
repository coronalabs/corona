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

#include "Core/Rtt_Build.h"

#include "Rtt_MacWebPopup.h"
#include "Rtt_MKeyValueIterable.h"

#include "Rtt_Display.h"
#include "Rtt_LuaAux.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_MKeyValueIterable.h"
#include "Rtt_Runtime.h"

#include "Rtt_PlatformPlayer.h"
#include "Rtt_MacSimulator.h"

#import "AppDelegate.h"

#import <Foundation/NSDictionary.h>
#import <Foundation/NSValue.h>

#import <AppKit/NSApplication.h>

#import <WebKit/WebKit.h>
#import <WebKit/WebPolicyDelegate.h>

// ----------------------------------------------------------------------------

// static CGFloat kAnimationDuration = 0.3;

@interface MacWebView ()
@property(nonatomic,readwrite) Rtt::MacWebPopup* owner;
@end

@implementation MacWebView
@synthesize owner;

@end

// Callback/Notification glue code
@interface MacWebViewContainer ()

@property(nonatomic,readwrite,getter=webView, retain) MacWebView *fWebView;
@property(nonatomic,readwrite) Rtt::MacWebPopup* owner;

- (id)initWithOwner:(Rtt::MacWebPopup*)popup;
- (void) setFrame:(NSRect)frame;

- (void)openWithRequest:(NSURLRequest*)request;
- (void) removeFromSuperview;

- (void)close;
- (void) stopLoading:(id)sender;

- (void) setHidden:(BOOL)hidden;

@end


@implementation MacWebViewContainer

@synthesize fWebView;
@synthesize owner;

- (id)initWithOwner:(Rtt::MacWebPopup*)popup
{
	self = [super init];
	if ( self )
	{
		viewFrame = NSZeroRect;
		owner = popup;
	}

	return self;
}

- (void) dealloc
{
	[fWebView close];
	[fWebView release];
	[super dealloc];
}

- (void)initView
{
//	if ( Rtt_VERIFY( ! fWebView ) )
	if ( ( ! fWebView ) )
	{
		fWebView = [[MacWebView alloc] initWithFrame:viewFrame frameName:nil groupName:nil];
		[fWebView setWantsLayer:YES];
		[fWebView setShouldCloseWithWindow:YES];
		[fWebView setPolicyDelegate:self];
		[fWebView setFrameLoadDelegate:self];
		[fWebView setUIDelegate:self];
		fWebView.owner = self.owner;
	}
}

// Overriding this delegate method and doing nothing prevents JavaScript from
// modifying the size or position of the window containing the webpopup.  Such
// behavior is usually disallowed by current web browsers so we ignore it too.
// (note that "- (BOOL)webViewIsResizable:(WebView *)sender;" is not effective)
- (void)webView:(WebView *)sender setFrame:(NSRect)frame
{
	// NSLog(@"WebView: setFrame: %@", NSStringFromRect(frame));

	// do nothing
}

// Handle window.close() from JavaScript or it'll close the entire app
- (void)webViewClose:(WebView *)sender
{
	[self performSelector:@selector(close) withObject:nil afterDelay:0];
}

- (void) setFrame:(NSRect)frame
{
	CGFloat backingScaleFactor = [fWebView.window backingScaleFactor];
	if (backingScaleFactor > 1.0)
	{
		frame.origin.x /= backingScaleFactor;
		frame.origin.y /= backingScaleFactor;
		frame.size.width /= backingScaleFactor;
		frame.size.height /= backingScaleFactor;
	}

	viewFrame = frame;
	[self initView];
	[fWebView setFrame:frame];
}

- (void) removeFromSuperview
{
	[fWebView removeFromSuperview];
}

- (void) setHidden:(BOOL)hidden
{
	[self initView];
	[fWebView setHidden:hidden];
}

- (void)openWithRequest:(NSURLRequest*)request
{
	[self initView];

	AppDelegate* delegate = (AppDelegate*)[NSApp delegate];
	GLView* view = [delegate layerHostView];
	[view addSubview:fWebView];
	[self setFrame:[fWebView frame]];  // adjust for backingScaleFactor
	[[fWebView mainFrame] loadRequest:request];
}

- (void) stopLoading:(id)sender
{
	[fWebView stopLoading:sender];
}

// Note: Calling close on an Apple WebView essentially destroys (but does not release) the object.
// The object is dead and cannot be reused. Create another WebView if you need it.
- (void)close
{
	[fWebView stopLoading:nil];
	[fWebView close];
	[fWebView removeFromSuperview];
	
	// Releasing the web view to make sure we reclaim the memory like our iOS implementation 
	// because users have been complaining about leaks or resources continuing to play (e.g. network audio)
	[fWebView release];
	fWebView = nil;
}

-(void) setDrawsBackground:(BOOL)hasbackground
{
	[self initView];
	[fWebView setDrawsBackground:hasbackground];	
}

// WebView doesn't have UIWebview:
//- (BOOL)webView:(WebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(WebViewNavigationType)navigationType
// http://forums.macrumors.com/showthread.php?t=1077692
- (void)webView:(WebView *)sender decidePolicyForNavigationAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id<WebPolicyDecisionListener>)listener
{
	BOOL result = owner->ShouldLoadUrl( [[[request URL] absoluteString] UTF8String] );
    
	if ( ! result )
	{
		[listener ignore];
        [self performSelector:@selector(close) withObject:nil afterDelay:0];
	}
    else
    {
        [listener use];
    }
}

- (void)webView:(WebView *)sender didFailLoadWithError:(NSError *)error forFrame:(WebFrame *)frame
{
	if ( ! owner->DidFailLoadUrl( [[[error userInfo] valueForKey:NSURLErrorFailingURLStringErrorKey] UTF8String], [[error localizedDescription] UTF8String], (S32)[error code] ) )
	{
		[self close];
	}
}

- (void) recomputeTransformCallback
{
	owner->RecomputeFromRotationOrScale();
	[fWebView setHidden:NO];

}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

MacWebPopup::MacWebPopup()
:	fWebView( [[MacWebViewContainer alloc] initWithOwner:this] ), // [[MacWebView alloc] initWithOwner:this] ),
	fMethod( @"GET" ),
	fBaseUrl( nil ),
	fBaseDirectory( MPlatform::kUnknownDir ),
	fNeedsRecompute( false ),
	fRenderingStream( NULL ),
	fPlatformSurface( NULL )	
{
}

MacWebPopup::~MacWebPopup()
{
	[fBaseUrl release];
	[fWebView release];
}


void
MacWebPopup::Preinitialize( const Display& display )
{
	S32 contentW = display.ContentWidth();
	S32 contentH = display.ContentHeight();

	// We need to grab the scaled width and height
	S32 screenW = display.ScaledWidth();
	S32 screenH = display.ScaledHeight();

	Real sx, sy;
	display.CalculateContentToScreenScale( sx, sy );
	
	Display::ScaleMode scalemode = display.GetScaleMode();
	// Special hack for Native widgets: The problem is that when no content scaling is set in config.lua,
	// sx and sy get stuck at 1, but I really need the scale factor to be applied to get internal transformations to work correctly.
	// So in UpdateContentScale, the sx,sy get changed under content scaling modes. My workaround is to not touch
	// anything if those modes are set. But if no mode is set and sx and sy have not been altered, I will compute the screen scale
	// based on the differences between content and screen.
	if ( Rtt_RealIsOne(sx) && Rtt_RealIsOne(sy) && Display::kNone == scalemode )
	{
		sx = Rtt_RealDiv( contentW, screenW );
		sy = Rtt_RealDiv( contentH, screenH );
	}

	SetContentToScreenSx( Rtt_RealDiv( Rtt_REAL_1, sx ) );
	SetContentToScreenSy( Rtt_RealDiv( Rtt_REAL_1, sy ) );
}
	
void
MacWebPopup::SetNeedsRecomputeFromRotationOrScale()
{
	if ( fNeedsRecompute )
	{
		return;
	}
	fNeedsRecompute = true;
	// Ugh. Because WebPopup is not a DisplayObject, there is no Build() phase callback.
	// So I'm going to create my own callback to happen so I can force a recompute of the transform.
	// This needs to happen sometime after the runtime has been altered for the rotation and scale factors.
	[fWebView performSelector:@selector(recomputeTransformCallback) withObject:nil afterDelay:0.10];

	// hide the view while transitioning
	[fWebView setHidden:YES];

}

void
MacWebPopup::RecomputeFromRotationOrScale()
{
#ifdef Rtt_AUTHORING_SIMULATOR
	AppDelegate* delegate = (AppDelegate*)[NSApp delegate];

	const Display& display = delegate.runtime->GetDisplay();
	Preinitialize( display );
	Rtt::Rect screenBounds;
	GetScreenBounds( display, screenBounds );

	NSRect webbounds = NSMakeRect( screenBounds.xMin, screenBounds.yMin, screenBounds.Width(), screenBounds.Height() );
		
	[fWebView setFrame:webbounds];
	fNeedsRecompute = false;
#endif // Rtt_AUTHORING_SIMULATOR
}
	
void
MacWebPopup::SetStageBounds( const Rect& bounds, const Runtime *runtime )
{	
	if ( nil == fWebView )
	{
		fWebView =[[MacWebViewContainer alloc] initWithOwner:this];
	}
	PlatformWebPopup::SetStageBounds(bounds, runtime);

#if 1 // Rtt_AUTHORING_SIMULATOR
	// FIXME: What is the coordinate system mapping?
	
	// AppDelegate* delegate = (AppDelegate*)[NSApp delegate];

	Rtt::Rect screenBounds;
	GetScreenBounds( runtime->GetDisplay(), screenBounds );
	//	Rtt_ASSERT( delegate.runtime->Surface().ScaledWidth() == [UIScreen mainScreen].bounds.size.width );
	//	Rtt_ASSERT( delegate.runtime->Surface().ScaledHeight() == [UIScreen mainScreen].bounds.size.height );
	//	Rect screenBounds;
	//	GetScreenBounds( screenBounds );
	
	//	const Rect& stageBounds = StageBounds();
	//	NSRect r = NSMakeRect( stageBounds.xMin, stageBounds.yMin, stageBounds.Width(), stageBounds.Height() );
	NSRect webbounds = NSMakeRect( screenBounds.xMin, screenBounds.yMin, screenBounds.Width(), screenBounds.Height() );

#else
	
	NSRect webbounds = NSMakeRect(bounds.xMin, bounds.yMin, bounds.xMax-bounds.xMin, bounds.yMax-bounds.yMin);
#endif // Rtt_AUTHORING_SIMULATOR
	[fWebView setFrame:webbounds];
}

void
MacWebPopup::Show( const MPlatform& platform, const char *url )
{
	if ( nil == fWebView )
	{
		fWebView =[[MacWebViewContainer alloc] initWithOwner:this];
	}
	
#if 1
//	Load( [NSString stringWithExternalString:url] );

	NSURL *baseUrl = nil;

	if ( MPlatform::kUnknownDir != fBaseDirectory )
	{
		Rtt_ASSERT( ! fBaseUrl );


		
		String path( & platform.GetAllocator() );
		platform.PathForFile( NULL, fBaseDirectory, MPlatform::kDefaultPathFlags, path );
		NSString *basePath = [NSString stringWithExternalString:path.GetString()];
		baseUrl = [[NSURL alloc] initFileURLWithPath:basePath];
	}
	else if ( nil != fBaseUrl )
	{
		baseUrl = [[NSURL alloc] initWithString:fBaseUrl];
	}

	NSString *urlString = [NSString stringWithExternalString:url];
	NSURL *u = (nil != baseUrl)
		? [[NSURL alloc] initWithString:urlString relativeToURL:baseUrl]
		: [[NSURL alloc] initWithString:urlString];
	NSMutableURLRequest *request = [[NSMutableURLRequest alloc] initWithURL:u];
	[u release];
	[baseUrl release];

	[request setHTTPMethod:fMethod];
	if ( NSOrderedSame == [fMethod caseInsensitiveCompare:@"POST"] )
	{
		Rtt_ASSERT_NOT_IMPLEMENTED();
		NSString *boundary = @"";
		NSString *contentType = [NSString stringWithFormat:@"multipart/form-data; boundary=%@", boundary];
		[request setValue:contentType forHTTPHeaderField:@"Content-Type"];

		NSData* body = nil; Rtt_ASSERT_NOT_IMPLEMENTED();
		if (body)
		{
			[request setHTTPBody:body];
		}
	}
	else
	{
	}

//	[fWebView loadRequest:request];

	[fWebView openWithRequest:request];

	[request release];
#endif
}

bool
MacWebPopup::Close()
{
	[fWebView close];
	[fWebView release];
	fWebView = nil;
	SetCallback(NULL);

	return true;
}

void
MacWebPopup::Reset()
{
}

void
MacWebPopup::SetPostParams( const MKeyValueIterable& params )
{
}

int
MacWebPopup::ValueForKey( lua_State *L, const char key[] ) const
{

	if ( nil == fWebView )
	{
		// ???
		return 0;
	}
	
	Rtt_ASSERT( key );

	int result = 1;

	if ( strcmp( "baseUrl", key ) == 0 )
	{
	}
	else if ( strcmp( "hasBackground", key ) == 0 )
	{
		// TODO: When we allow changing bkgd colors, we should cache this as
		// a separate property which would ignore the UIColor.
//		UIColor *color = fWebView.webView.backgroundColor;
//		CGFloat alpha = CGColorGetAlpha( [color CGColor] );
//		lua_pushboolean( L, alpha > FLT_EPSILON );
	}
	else
	{
		result = 0;
	}

	return result;
}

bool
MacWebPopup::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	if ( nil == fWebView )
	{
		fWebView =[[MacWebViewContainer alloc] initWithOwner:this];
	}
	
	Rtt_ASSERT( key );

	bool result = true;

	if ( strcmp( "baseUrl", key ) == 0 )
	{
		// TODO: Currently assumes this property is only set once

		if ( lua_isstring( L, valueIndex ) )
		{
			Rtt_ASSERT( MPlatform::kUnknownDir == fBaseDirectory );

			fBaseUrl = [[NSString alloc] initWithUTF8String:lua_tostring( L, valueIndex )];
		}
		else if ( lua_islightuserdata( L, valueIndex ) )
		{
			Rtt_ASSERT( ! fBaseUrl );

			fBaseDirectory = (MPlatform::Directory)EnumForUserdata(
				 LuaLibSystem::Directories(),
				 lua_touserdata( L, valueIndex ),
				 MPlatform::kNumDirs,
				 MPlatform::kUnknownDir );
		}
	}
	else if ( strcmp( "hasBackground", key ) == 0 )
	{
		bool hasBackground = lua_toboolean( L, valueIndex );
		[fWebView setDrawsBackground:hasBackground];
		//		UIColor *color = hasBackground ? [UIColor whiteColor] : [UIColor clearColor];
//		fWebView.webView.backgroundColor = color;
//		fWebView.webView.opaque = hasBackground;
	}
	else
	{
		result = false;
	}

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

