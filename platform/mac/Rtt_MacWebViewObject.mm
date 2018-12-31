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

#include "Rtt_MacWebViewObject.h"

#import "AppDelegate.h"

#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Rtt_LuaLibDisplay.h"
#include "Rtt_LuaLibNative.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_LuaResource.h"
#include "Rtt_PlatformWebPopup.h"
#include "Rtt_Rendering.h"

#import <WebKit/WebKit.h>
#import <WebKit/WebPolicyDelegate.h>

// ----------------------------------------------------------------------------

@interface Rtt_WebView ()

- (id)initWithFrame:(NSRect)frameRect;

- (void)setDelegate:(id)delegate;

- (void)loadRequest:(NSURLRequest*)request;

- (void)loadHtmlString:(NSString*)htmlString baseURL:(NSURL*)baseUrl;

@end


@implementation Rtt_WebView

@synthesize owner;

- (id)initWithFrame:(NSRect)frameRect;
{
	self = [super initWithFrame:frameRect frameName:nil groupName:nil];
	if ( self )
	{
		owner = NULL;
		[self setWantsLayer:YES];
		[self setUIDelegate:self];
		[self setPolicyDelegate:self];
		[self setFrameLoadDelegate:self];
	}

	return self;
}

// Overriding this delegate method and doing nothing prevents JavaScript from
// modifying the size or position of the window containing the webview.  Such
// behavior is usually disallowed by current web browsers so we ignore it too.
// (note that "- (BOOL)webViewIsResizable:(WebView *)sender;" is not effective)
- (void)webView:(WebView *)sender setFrame:(NSRect)frame
{
	// NSLog(@"WebView: setFrame: %@", NSStringFromRect(frame));

	// do nothing
}

// Ignore window.close() from JavaScript because it exits the entire app but
// notify the app if it's listening
- (void)webViewClose:(WebView *)sender
{
	Rtt::UrlRequestEvent e( "window.close", Rtt::UrlRequestEvent::kOther );
	owner->DispatchEventWithTarget( e );
}

- (void)setDelegate:(id)delegate
{
	[self setPolicyDelegate:delegate];
	[self setFrameLoadDelegate:delegate];
}
- (void)loadHtmlString:(NSString*)htmlString baseURL:(NSURL*)baseUrl
{
	[[self mainFrame] loadHTMLString:htmlString baseURL:baseUrl];
}
- (void)loadRequest:(NSURLRequest*)request
{
	[[self mainFrame] loadRequest:request];
}

// WebView doesn't have UIWebview:
//- (BOOL)webView:(WebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(WebViewNavigationType)navigationType
// http://forums.macrumors.com/showthread.php?t=1077692
// Maybe this will work?
- (void)webView:(WebView *)sender decidePolicyForNavigationAction:(NSDictionary *)actionInformation request:(NSURLRequest *)request frame:(WebFrame *)frame decisionListener:(id<WebPolicyDecisionListener>)listener
{
	using namespace Rtt;

	NSURL *url = [request URL];
	
	UrlRequestEvent::Type urlRequestType = UrlRequestEvent::kUnknown;
	if (WebNavigationTypeLinkClicked == [[actionInformation objectForKey:WebActionNavigationTypeKey] intValue])
	{
		urlRequestType = UrlRequestEvent::kLink;
	}
	else if (WebNavigationTypeFormSubmitted == [[actionInformation objectForKey:WebActionNavigationTypeKey] intValue])
	{
		urlRequestType = UrlRequestEvent::kForm;
	}
	else if (WebNavigationTypeBackForward == [[actionInformation objectForKey:WebActionNavigationTypeKey] intValue])
	{
		urlRequestType = UrlRequestEvent::kHistory;
	}
	else if (WebNavigationTypeReload == [[actionInformation objectForKey:WebActionNavigationTypeKey] intValue])
	{
		urlRequestType = UrlRequestEvent::kReload;
	}
	else if (WebNavigationTypeFormResubmitted == [[actionInformation objectForKey:WebActionNavigationTypeKey] intValue])
	{
		urlRequestType = UrlRequestEvent::kFormResubmitted;
	}
	else if (WebNavigationTypeOther == [[actionInformation objectForKey:WebActionNavigationTypeKey] intValue])
	{
		urlRequestType = UrlRequestEvent::kOther;
	}

	const char *urlString = [[url absoluteString] UTF8String];
	UrlRequestEvent e( urlString, urlRequestType );
	owner->DispatchEventWithTarget( e ); // if listener succeeds, result will be modified as appropriate
	[listener use];
}

- (void)webView:(WebView *)sender didFinishLoadForFrame:(WebFrame *)frame
{
	using namespace Rtt;

	//This is a javascript hack, not sure how else to get the url request
	NSString *rawLocationString = [sender stringByEvaluatingJavaScriptFromString:@"location.href;"];
		
	const char *urlString = [rawLocationString UTF8String];
	UrlRequestEvent e( urlString, UrlRequestEvent::kLoaded );
	owner->DispatchEventWithTarget( e );
		
}

- (void)webView:(WebView *)sender didFailLoadWithError:(NSError *)error forFrame:(WebFrame *)frame
{
	using namespace Rtt;

	const char *urlString =
		[[[error userInfo] valueForKey:NSURLErrorFailingURLStringErrorKey] UTF8String];

	UrlRequestEvent e(
		urlString, [[error localizedDescription] UTF8String], (S32) [error code] );
	owner->DispatchEventWithTarget( e );

}

// Implementing this UI delegate method with nothing in it, makes links with "target=_blank" load
// in the same webview (because it is returned here) which is the behavior we want (it matches
// iOS and Android)
- (WebView*)webView:(WebView *)sender createWebViewWithRequest:(NSURLRequest *)request
{
	return sender;
}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

MacWebViewObject::MacWebViewObject( const Rect& bounds )
:	Super( bounds )
{
}

MacWebViewObject::~MacWebViewObject()
{
	// Nil out the delegate to prevent any notifications from being triggered on this dead object
	Rtt_WebView *view = (Rtt_WebView*)GetView();
	[view setDelegate:nil];
	[view stopLoading:nil];
	[view close];
	view = NULL;
}

bool
MacWebViewObject::Initialize()
{
	Rtt_ASSERT( ! GetView() );

	Rect screenBounds;
	GetScreenBounds( screenBounds );

	NSRect r = NSMakeRect( screenBounds.xMin, screenBounds.yMin, screenBounds.Width(), screenBounds.Height() );

	Rtt_WebView *v = [[Rtt_WebView alloc] initWithFrame:r];
//	t.borderStyle = UITextBorderStyleRoundedRect;
	v.owner = this;

	// We have a problem because  Build(matrix) is computed some time after the object was created.
	// If we insert the object when created, when this method is invoked shortly later, you can see
	// a visible jump in the object's position which looks bad.
	// The workaround is to defer adding the object to the view until after this method has been computed at least once.
	// TODO: Temprorarily disable until we fix problems with Mac native display objects
//The above comment doesn't seem to apply - transitions, movements, view changes all appear to function normally

	Super::InitializeView( v );

	[v release];
	
	return (v != nil);
}

const LuaProxyVTable&
MacWebViewObject::ProxyVTable() const
{
	return PlatformDisplayObject::GetWebViewObjectProxyVTable();
}

NSURL*
MacWebViewObject::GetBaseURLFromLuaState(lua_State *L, int index)
{
	NSURL *result = nil;
	
	if ( lua_isstring( L, index ) )
	{
		NSString *str = [[NSString alloc] initWithUTF8String:lua_tostring( L, index )];
		result = [NSURL URLWithString:str];
	}
	else if ( lua_islightuserdata( L, index ) )
	{
		MPlatform::Directory dir = (MPlatform::Directory)EnumForUserdata(
																		 LuaLibSystem::Directories(),
																		 lua_touserdata( L, index ),
																		 MPlatform::kNumDirs,
																		 MPlatform::kUnknownDir );
		if ( MPlatform::kUnknownDir != dir )
		{
			const MPlatform& platform = LuaContext::GetPlatform( L );
			String path( & platform.GetAllocator() );
			platform.PathForFile( NULL, dir, MPlatform::kDefaultPathFlags, path );
			NSString *basePath = [NSString stringWithExternalString:path.GetString()];
			result = [NSURL fileURLWithPath:basePath];
		}
	}
	return result;
}

int
MacWebViewObject::Load( lua_State *L )
{
const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	MacWebViewObject *o = (MacWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		const char *htmlBody = lua_tostring( L, 2 );
		NSString *htmlBodyString = [NSString stringWithExternalString:htmlBody];

		NSURL *baseUrl = GetBaseURLFromLuaState( L, 3);

		o->Load(htmlBodyString, baseUrl );
	}

	return 0;
}

// view:request( url )
int
MacWebViewObject::Request( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	MacWebViewObject *o = (MacWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		const char *url = lua_tostring( L, 2 );
		NSString *urlString = [NSString stringWithExternalString:url];

		NSURL *baseUrl = GetBaseURLFromLuaState( L, 3);

		o->Request( urlString, baseUrl );
	}

	return 0;
}

void
MacWebViewObject::Load( NSString *htmlBody, NSURL *baseUrl )
{
	Rtt_WebView *container = (Rtt_WebView*)GetView();
	[container loadHtmlString:htmlBody baseURL:baseUrl];
}

void
MacWebViewObject::Request( NSString *urlString, NSURL *baseUrl )
{
	NSURL *u = (nil != baseUrl)
		? [[NSURL alloc] initWithString:urlString relativeToURL:baseUrl]
		: [[NSURL alloc] initWithString:urlString];
	NSMutableURLRequest *request = [[NSMutableURLRequest alloc] initWithURL:u];
	[u release];
	
	[request setHTTPMethod:@"GET"];
	/*
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
	*/

	Rtt_WebView *view = (Rtt_WebView*)GetView();
	[view loadRequest:request];

	[request release];
}

int
MacWebViewObject::Stop( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	MacWebViewObject *o = (MacWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		Rtt_WebView *view = (Rtt_WebView*)o->GetView();
		[view stopLoading:nil];
	}

	return 0;
}

int
MacWebViewObject::Back( lua_State *L )
{

	bool result = false;
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	MacWebViewObject *o = (MacWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		Rtt_WebView *view = (Rtt_WebView*)o->GetView();
		result = [view goBack];
	}
	
	lua_pushboolean( L, result );

	return 1;

}

int
MacWebViewObject::Forward( lua_State *L )
{
	bool result = false;

	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	MacWebViewObject *o = (MacWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		Rtt_WebView *view = (Rtt_WebView*)o->GetView();
		result = [view goForward];
	}

	lua_pushboolean( L, result );

	return 1;
}

int
MacWebViewObject::Resize( lua_State *L )
{
	Rtt_ASSERT_NOT_IMPLEMENTED();

	return 0;
}

int
MacWebViewObject::Reload( lua_State *L )
{

	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	MacWebViewObject *o = (MacWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		Rtt_WebView *view = (Rtt_WebView*)o->GetView();
		[view reload:nil];
	}
	
	return 0;

}

int
MacWebViewObject::DeleteCookies( lua_State *L )
{
	
	// This isn't implemented because the cookies in the webview are tied to safari.
	// If we delete it from here then they're deleted from safari as well.
	
	return 0;
	
}
	
/*
int
MacWebViewObject::SetBackgroundColor( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	MacWebViewObject *o = (MacWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		bool hasBackground;
		if ( LUA_TNUMBER == lua_type( L, 2 ) )
		{
			Color c = LuaLibDisplay::toColor( L, 2 );
			RGBA rgba = ((ColorUnion*)(& c))->rgba;

			hasBackground = ( rgba.a > 0 );

			Rtt_WARN_SIM( hasBackground, ( "WARNING: The background color of native web views can only be white or transparent on MacOS.\n" ) );
		}
		else
		{
			hasBackground = false;
		}

		Rtt_WebView *view = (Rtt_WebView*)o->GetView();
		[view setDrawsBackground:hasBackground];
	}

	return 0;
}
*/

int
MacWebViewObject::ValueForKey( lua_State *L, const char key[] ) const
{
	Rtt_ASSERT( key );

	int result = 1;
	
	/*
	else if ( strcmp( "autoCancel", key ) == 0 )
	{
	}
	*/

	if ( strcmp( "request", key ) == 0 )
	{
		lua_pushcfunction( L, Request );
	}
	else if ( strcmp( "stop", key ) == 0 )
	{
		lua_pushcfunction( L, Stop );
	}
	else if ( strcmp( "back", key ) == 0 )
	{
		lua_pushcfunction( L, Back );
	}
	else if ( strcmp( "forward", key ) == 0 )
	{
		lua_pushcfunction( L, Forward );
	}
	else if ( strcmp( "reload", key ) == 0 )
	{
		lua_pushcfunction( L, Reload );
	}
	else if ( strcmp( "resize", key ) == 0 )
	{
		lua_pushcfunction( L, Resize );
	}
	else if ( strcmp( "deleteCookies", key ) == 0 )
	{
		lua_pushcfunction( L, DeleteCookies );
	}
	else if ( strcmp( "bounces", key ) == 0 )
	{
		Rtt_PRINT( ( "WARNING: Web views do not have bounce behavior on this platform.\n" ) );
	}
	else if ( strcmp( "canGoBack", key ) == 0 )
	{
		Rtt_WebView *view = (Rtt_WebView*)GetView();
		lua_pushboolean( L, view.canGoBack );
	}
	else if ( strcmp( "canGoForward", key ) == 0 )
	{
		Rtt_WebView *view = (Rtt_WebView*)GetView();
		lua_pushboolean( L, view.canGoForward );
	}
	else if ( strcmp( "hasBackground", key ) == 0 )
	{
		Rtt_WebView *view = (Rtt_WebView*)GetView();
		lua_pushboolean( L, view.drawsBackground );
	}
	else if ( strcmp( "load", key ) == 0 )
	{
		lua_pushcfunction( L, Load );
	}
	else
	{
		result = Super::ValueForKey( L, key );
	}

	return result;
}

bool
MacWebViewObject::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	Rtt_ASSERT( key );

	bool result = true;

	if ( strcmp( "hasBackground", key ) == 0 )
	{
		Rtt_WebView *view = (Rtt_WebView*)GetView();
		[view setDrawsBackground:lua_toboolean( L, valueIndex )];
	}
	else if ( strcmp( "bounces", key ) == 0 )
	{
		Rtt_PRINT( ( "WARNING: Web views do not have bounce behavior on this platform.\n" ) );
	}
	else if ( strcmp( "request", key ) == 0
		 || strcmp( "stop", key ) == 0
		 || strcmp( "back", key ) == 0
		 || strcmp( "forward", key ) == 0
		 || strcmp( "reload", key ) == 0
		 || strcmp( "resize", key ) == 0 )
	{
		// no-op
	}
	else
	{
	
		result = Super::SetValueForKey( L, key, valueIndex );
	}
		
	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

