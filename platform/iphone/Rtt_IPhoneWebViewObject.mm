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

#include "Rtt_IPhoneWebViewObject.h"

#import <UIKit/UIKit.h>
#include <math.h>
//#import <MapKit/MapKit.h>
//#import <MapKit/MKAnnotation.h>

#import "AppDelegate.h"



//#include "Rtt_IPhoneFont.h"

#include "Rtt_Event.h"
#include "Rtt_Lua.h"
#include "Rtt_LuaContext.h"
#include "Display/Rtt_LuaLibDisplay.h"
#include "Rtt_LuaLibNative.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_LuaProxy.h"
#include "Rtt_LuaProxyVTable.h"
#include "Rtt_MPlatform.h"
#include "Rtt_PlatformWebPopup.h"
#include "Renderer/Rtt_RenderTypes.h"
#include "Core/Rtt_String.h"

#ifdef Rtt_DEBUG
	// Used in asserts in Initialize()
	#include "Display/Rtt_Display.h"
	#include "Rtt_Runtime.h"
#endif

// ----------------------------------------------------------------------------

static CGFloat kAnimationDuration = 0.3;

//static void
//RectToCGRect( const Rtt::Rect& bounds, CGRect * outRect )
//{
//	if ( bounds.NotEmpty() )
//	{
//		Rtt::Real x = bounds.xMin;
//		Rtt::Real y = bounds.yMin;
//		outRect->origin.x = Rtt_RealToFloat( x );
//		outRect->origin.y = Rtt_RealToFloat( y );
//
//		Rtt::Real w = bounds.xMax - x;
//		Rtt::Real h = bounds.yMax - y;
//		outRect->size.width = Rtt_RealToFloat( w );
//		outRect->size.height = Rtt_RealToFloat( h );
//	}
//	else
//	{
//		*outRect = CGRectNull;
//	}
//}

// ----------------------------------------------------------------------------

@interface Rtt_UIWebViewContainer : UIView< UIWebViewDelegate >
{
	Rtt::IPhoneWebViewObject *fOwner;
	UIWebView *fWebView;
	NSURL *fLoadingURL;
	UIView *fActivityView;
	BOOL isOpen;
	BOOL keyboardShown;
	BOOL isLoading;
	UIInterfaceOrientation initialOrientation;
}

@property(nonatomic,assign,getter=owner,setter=setOwner:) Rtt::IPhoneWebViewObject *fOwner;
@property(nonatomic,readonly,getter=webView) UIWebView *fWebView;
@property(nonatomic,readonly) BOOL isOpen;

- (id)initWithFrame:(CGRect)rect;

- (void)addObservers;
- (void)removeObservers;
- (void)loadHtmlString:(NSString*)htmlString baseURL:(NSURL*)baseUrl;
- (void)loadRequest:(NSURLRequest*)request;
- (void)stopLoading;
- (BOOL)back;
- (BOOL)forward;
- (void)reload;

- (BOOL)bounces;
- (void)setBounces:(BOOL)newValue;

@end

@implementation Rtt_UIWebViewContainer

@synthesize fOwner;
@synthesize fWebView;
@synthesize isOpen;

- (id)initWithFrame:(CGRect)rect
{
	self = [super initWithFrame:rect];
	if ( self )
	{
		fOwner = nil;
		fLoadingURL = nil;

        // Propagate the w,h, but do not propagate the origin, as the parent already accounts for it.
		CGRect webViewRect = rect;
		webViewRect.origin = CGPointZero;
		fWebView = [[UIWebView alloc] initWithFrame:webViewRect];
		fWebView.delegate = self;
		fWebView.scalesPageToFit = YES;
        
		fActivityView = [[UIView alloc] initWithFrame:webViewRect];
		fActivityView.backgroundColor = [UIColor grayColor];
		
		UIActivityIndicatorView *indicator = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhiteLarge];
		[fActivityView addSubview:indicator];		
		[indicator release];

		isOpen = false;
		keyboardShown = NO;
		isLoading = NO;

		// TODO: Remove dependency on AppDelegate. This is called from IPhoneWebViewObject::Initialize()
		// which should have access to the view controller, and thus the interfaceOrientation.
		AppDelegate *delegate = (AppDelegate *)[UIApplication sharedApplication].delegate;
		UIViewController *viewController = delegate.viewController;
		initialOrientation = viewController.interfaceOrientation;
		
		[self addSubview:fWebView];
		[self addSubview:fActivityView];

		CGPoint center = { (CGFloat)0.5f*CGRectGetWidth( rect ), (CGFloat)0.5f*CGRectGetHeight( rect ) };
		[indicator setCenter:center];
        
        [self addObservers];
	}

	return self;
}

- (void)dealloc
{
	[self removeObservers];

	[fActivityView release];
	[fLoadingURL release];

	[fWebView setDelegate:nil];
	[fWebView stopLoading];
	[fWebView release];

	[super dealloc];
}
-(void) setFrame:(CGRect)frame
{
	[super setFrame:frame];
	
	frame.origin = CGPointZero;
	[fWebView setFrame:frame];
	[fActivityView setFrame:frame];
}

- (void)addObservers
{
	NSNotificationCenter *notifier = [NSNotificationCenter defaultCenter];

	[notifier addObserver:self
		selector:@selector(keyboardWasShown:)
		name:UIKeyboardDidShowNotification object:nil];

	[notifier addObserver:self
		selector:@selector(keyboardWasHidden:)
		name:UIKeyboardDidHideNotification object:nil];
}

- (void)removeObservers
{
	NSNotificationCenter *notifier = [NSNotificationCenter defaultCenter];

	[notifier removeObserver:self
		name:UIKeyboardDidShowNotification object:nil];

	[notifier removeObserver:self
		name:UIKeyboardDidHideNotification object:nil];
}

- (void)didAppear:(NSString *)animationID finished:(NSNumber *)finished context:(void *)context
{
}
- (void)loadHtmlString:(NSString*)htmlString baseURL:(NSURL*)baseUrl
{
	if ( isLoading )
	{
		[fWebView stopLoading];
	}
	
	fLoadingURL = nil;
	
	if(htmlString)
	{
		isLoading = true;
		[fWebView loadHTMLString:htmlString baseURL:baseUrl];
	}
	
}
- (void)loadRequest:(NSURLRequest*)request
{
	if ( isLoading )
	{
		[fWebView stopLoading];
	}

    
    bool loadImmediately = false;
    
    if (fLoadingURL)
    {
        if (request)
        {
            if (request.URL)
            {
                NSString *urlString1 = [fLoadingURL absoluteString];
                NSString *urlString2 = [request.URL absoluteString];
                
                //If the url requests are the same, then load like normal since this
                //generates a callback and will create a dispatch event (i.e. page reload)
                if( [urlString1 caseInsensitiveCompare:urlString2] != NSOrderedSame )
                {
                    //If the requests are the same up to the hash tag then do an immediate load
                    //since we never get a callback for this load and therefore never removed
                    //the grey activity window
                    NSString *subString1 = [[urlString1 componentsSeparatedByString:@"#"] objectAtIndex:0];
                    NSString *subString2 = [[urlString2 componentsSeparatedByString:@"#"] objectAtIndex:0];
                    
                    //Only the hash fragments differ
                    if( [subString1 caseInsensitiveCompare:subString2] == NSOrderedSame )
                    {
                        loadImmediately = true;
                    }
                }
            }
        }
    }

    
	[fLoadingURL release];
	fLoadingURL = [request.URL retain];

	[fWebView loadRequest:request];

    if (false == loadImmediately)
    {
        if ( ! isLoading )
        {
            fActivityView.alpha = 0.0;
            [UIView beginAnimations:nil context:nil];
            [UIView setAnimationDuration:kAnimationDuration];
            [UIView setAnimationDelegate:self];
            [UIView setAnimationDidStopSelector:@selector(didAppear:finished:context:)];
            fActivityView.alpha = 1.0;
            [UIView commitAnimations];
            
            [self addObservers];
        }
        isLoading = YES;
    }
	
}

- (void)stopLoading
{
	[fWebView stopLoading];

	UIActivityIndicatorView *indicator = [[fActivityView subviews] objectAtIndex:0];
	[indicator stopAnimating];
}

- (BOOL)back
{
	BOOL result = fWebView.canGoBack;

	if ( result )
	{
		[fWebView goBack];
	}

	return result;
}

- (BOOL)forward
{
	BOOL result = fWebView.canGoForward;

	if ( result )
	{
		[fWebView goForward];
	}

	return result;
}

- (void)reload
{
	[fWebView reload];
}

- (BOOL)bounces
{
	BOOL result = YES;

	for ( id subview in fWebView.subviews )
	{
		if ( [[subview class] isSubclassOfClass:[UIScrollView class]] )
		{
			result = ((UIScrollView*)subview).bounces;
			break;
		}
	}

	return result;
}

- (void)setBounces:(BOOL)newValue
{
	for ( id subview in fWebView.subviews )
	{
		if ( [[subview class] isSubclassOfClass:[UIScrollView class]] )
		{
			((UIScrollView*)subview).bounces = newValue;
		}
	}
}

- (void)keyboardWasShown:(NSNotification*)aNotification
{
	if ( ! keyboardShown && [fWebView isFirstResponder] )
	{
		NSDictionary* info = [aNotification userInfo];

		// Get the size of the keyboard.
		// (Deprecated const takes into account rotation, otherwise use: UIKeyboardFrameBeginUserInfoKey)
		NSValue* aValue = [info objectForKey:UIKeyboardBoundsUserInfoKey];
		CGSize keyboardSize = [aValue CGRectValue].size;

		UIWebView *view = fWebView;

		// Resize the scroll view (which is the root view of the window)
		CGRect viewFrame = [view frame];
		viewFrame.size.height -= keyboardSize.height;
		view.frame = viewFrame;

		keyboardShown = YES;
	}
}

// Called when the UIKeyboardDidHideNotification is sent
- (void)keyboardWasHidden:(NSNotification*)aNotification
{
	if ( [fWebView isFirstResponder] )
	{
		NSDictionary* info = [aNotification userInfo];
	 
		// Get the size of the keyboard.
		NSValue* aValue = [info objectForKey:UIKeyboardBoundsUserInfoKey];
		CGSize keyboardSize = [aValue CGRectValue].size;
	 
		UIWebView *view = fWebView;

		// Reset the height of the scroll view to its original value
		CGRect viewFrame = [view frame];
		viewFrame.size.height += keyboardSize.height;
		view.frame = viewFrame;
	 
		keyboardShown = NO;
	}
}

Rtt::UrlRequestEvent::Type
static EventTypeForNavigationType( UIWebViewNavigationType t )
{
	using namespace Rtt;

	UrlRequestEvent::Type result = UrlRequestEvent::kUnknown;

	switch ( t )
	{
		case UIWebViewNavigationTypeLinkClicked:
			result = UrlRequestEvent::kLink;
			break;
		case UIWebViewNavigationTypeFormSubmitted:
			result = UrlRequestEvent::kForm;
			break;
		case UIWebViewNavigationTypeBackForward:
			result = UrlRequestEvent::kHistory;
			break;
		case UIWebViewNavigationTypeReload:
			result = UrlRequestEvent::kReload;
			break;
		case UIWebViewNavigationTypeFormResubmitted:
			result = UrlRequestEvent::kFormResubmitted;
			break;
		case UIWebViewNavigationTypeOther:
			result = UrlRequestEvent::kOther;
			break;
		default:
			break;
	}

	return result;
}

- (BOOL)webView:(UIWebView *)webView shouldStartLoadWithRequest:(NSURLRequest *)request navigationType:(UIWebViewNavigationType)navigationType
{
	using namespace Rtt;

	NSURL *url = request.URL;

	const char *urlString = [[url absoluteString] UTF8String];
	
	Rtt::UrlRequestEvent::Type navType = EventTypeForNavigationType( navigationType );
	
	UrlRequestEvent e( urlString, navType );
	
	fOwner->DispatchEventWithTarget( e );
	
	if (navType == UrlRequestEvent::kLink ||
		navType == UrlRequestEvent::kHistory ||
		navType == UrlRequestEvent::kReload )
	{
		isLoading = true;
	}
	
	return true; // Always load
}

- (void)webViewDidStartLoad:(UIWebView *)webView
{
	UIActivityIndicatorView *indicator = [[fActivityView subviews] objectAtIndex:0];
	[indicator startAnimating];
}

- (void)webViewDidFinishLoad:(UIWebView *)webView
{
	using namespace Rtt;

	if ( isLoading )
	{
		isLoading = NO;

		[self hideActivityViewIndicator];

		NSURL *url = webView.request.URL;
		const char *urlString = [[url absoluteString] UTF8String];
		UrlRequestEvent e( urlString, UrlRequestEvent::kLoaded );
		fOwner->DispatchEventWithTarget( e );
	}
}

- (void)webView:(UIWebView *)webView didFailLoadWithError:(NSError *)error
{
	using namespace Rtt;

	isLoading = NO;

	const char *urlString =
		[[[error userInfo] valueForKey:NSURLErrorFailingURLStringErrorKey] UTF8String];

	UrlRequestEvent e(
		urlString, [[error localizedDescription] UTF8String], (S32)[error code] );
	fOwner->DispatchEventWithTarget( e );

	[self hideActivityViewIndicator];
}

- (void)hideActivityViewIndicator
{
	UIActivityIndicatorView *indicator = [[fActivityView subviews] objectAtIndex:0];
	[indicator stopAnimating];
	
	fWebView.hidden = NO;
	fWebView.alpha = 1.0;
	fActivityView.alpha = 1.0;
	[UIView beginAnimations:nil context:nil];
	[UIView setAnimationDuration:kAnimationDuration];
	[UIView setAnimationDelegate:self];
	[UIView setAnimationDidStopSelector:@selector(didAppear:finished:context:)];
	fActivityView.alpha = 0.0;
	[UIView commitAnimations];
}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

IPhoneWebViewObject::IPhoneWebViewObject( const Rect& bounds )
:	Super( bounds ),
	fMethod( @"GET" )
{
}

IPhoneWebViewObject::~IPhoneWebViewObject()
{
	Rtt_UIWebViewContainer *v = (Rtt_UIWebViewContainer*)GetView();
	v.owner = NULL;
}

bool
IPhoneWebViewObject::Initialize()
{
	Rtt_ASSERT( ! GetView() );

	// TODO: Remove dependency on AppDelegate by fetching controller via: GetCoronaView().viewController
	AppDelegate *delegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
	UIViewController *controller = delegate.viewController;

	// TODO: Remove asserts. Already done in IPhoneDisplayObject::Preinitialize()
	Rtt_ASSERT( delegate.runtime->GetDisplay().PointsWidth() == [UIScreen mainScreen].bounds.size.width );
	Rtt_ASSERT( delegate.runtime->GetDisplay().PointsHeight() == [UIScreen mainScreen].bounds.size.height );

	Rect screenBounds;
	GetScreenBounds( screenBounds );
	CGRect r = CGRectMake( screenBounds.xMin, screenBounds.yMin, screenBounds.Width(), screenBounds.Height() );
	Rtt_UIWebViewContainer *v = [[Rtt_UIWebViewContainer alloc] initWithFrame:r];
	v.owner = this;

	UIView *parent = controller.view;
	[parent addSubview:v];

	Super::InitializeView( v );
	[v release];

	return v;
}

const LuaProxyVTable&
IPhoneWebViewObject::ProxyVTable() const
{
	return PlatformDisplayObject::GetWebViewObjectProxyVTable();
}
NSURL* IPhoneWebViewObject::GetBaseURLFromLuaState(lua_State *L, int index)
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
			NSString *basePath = [NSString stringWithUTF8String:path.GetString()];
			result = [NSURL fileURLWithPath:basePath];
		}
	}
	return result;
}
int
IPhoneWebViewObject::Load( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	IPhoneWebViewObject *o = (IPhoneWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		const char *htmlBody = lua_tostring( L, 2 );
		NSString *htmlBodyString = [NSString stringWithUTF8String:htmlBody];

		NSURL *baseUrl = GetBaseURLFromLuaState(L,3);

		o->Load(htmlBodyString,baseUrl);
	}

	return 0;
}

// view:request( url )
int
IPhoneWebViewObject::Request( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	IPhoneWebViewObject *o = (IPhoneWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		const char *url = lua_tostring( L, 2 );
		NSString *urlString = [NSString stringWithUTF8String:url];

		NSURL *baseUrl = GetBaseURLFromLuaState(L,3);

		o->Request( urlString, baseUrl );
	}

	return 0;
}
void
IPhoneWebViewObject::Load( NSString *htmlBody, NSURL *baseUrl )
{
	Rtt_UIWebViewContainer *container = (Rtt_UIWebViewContainer*)GetView();
	[container loadHtmlString:htmlBody baseURL:baseUrl];
}
void
IPhoneWebViewObject::Request( NSString *urlString, NSURL *baseUrl )
{
	NSURL *u = (nil != baseUrl)
		? [[NSURL alloc] initWithString:urlString relativeToURL:baseUrl]
		: [[NSURL alloc] initWithString:urlString];
	NSMutableURLRequest *request = [[NSMutableURLRequest alloc] initWithURL:u];
	[u release];
	
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

	Rtt_UIWebViewContainer *container = (Rtt_UIWebViewContainer*)GetView();
	[container loadRequest:request];

	[request release];
}

int
IPhoneWebViewObject::Stop( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	IPhoneWebViewObject *o = (IPhoneWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		UIView *view = o->GetView();
		Rtt_UIWebViewContainer *container = (Rtt_UIWebViewContainer*)view;
		[container stopLoading];
	}

	return 0;
}

int
IPhoneWebViewObject::Back( lua_State *L )
{
	bool result = false;

	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	IPhoneWebViewObject *o = (IPhoneWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		UIView *view = o->GetView();
		Rtt_UIWebViewContainer *container = (Rtt_UIWebViewContainer*)view;
		result = [container back];
	}

	lua_pushboolean( L, result );

	return 1;
}

int
IPhoneWebViewObject::Forward( lua_State *L )
{
	bool result = false;

	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	IPhoneWebViewObject *o = (IPhoneWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		UIView *view = o->GetView();
		Rtt_UIWebViewContainer *container = (Rtt_UIWebViewContainer*)view;
		result = [container forward];
	}

	lua_pushboolean( L, result );

	return 1;
}

int
IPhoneWebViewObject::Resize( lua_State *L )
{
	Rtt_ASSERT_NOT_IMPLEMENTED();

	return 0;
}

int
IPhoneWebViewObject::Reload( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	IPhoneWebViewObject *o = (IPhoneWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		UIView *view = o->GetView();
		Rtt_UIWebViewContainer *container = (Rtt_UIWebViewContainer*)view;
		[container reload];
	}

	return 0;
}
	
int
IPhoneWebViewObject::DeleteCookies( lua_State *L )
{
	NSHTTPCookie *cookie;
	NSHTTPCookieStorage *storage = [NSHTTPCookieStorage sharedHTTPCookieStorage];
	for ( cookie in [storage cookies] )
	{
		[storage deleteCookie:cookie];
	}
	[[NSUserDefaults standardUserDefaults] synchronize];
	
	return 0;
}

/*
int
IPhoneWebViewObject::SetBackgroundColor( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	IPhoneWebViewObject *o = (IPhoneWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		UIColor *color;
		bool hasBackground;
		if ( LUA_TNUMBER == lua_type( L, 2 ) )
		{
			Color c = LuaLibDisplay::toColor( L, 2 );
			RGBA rgba = ((ColorUnion*)(& c))->rgba;

			color = [UIColor colorWithRed:rgba.r green:rgba.g blue:rgba.b alpha:rgba.a];
			hasBackground = ( rgba.a > 0 );
		}
		else
		{
			color = [UIColor clearColor];
			hasBackground = false;
		}

		UIView *view = o->GetView();
		Rtt_UIWebViewContainer *container = (Rtt_UIWebViewContainer*)view;
		container.webView.backgroundColor = color;
		container.webView.opaque = hasBackground;
	}

	return 0;
}
*/

int
IPhoneWebViewObject::ValueForKey( lua_State *L, const char key[] ) const
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
		Rtt_UIWebViewContainer *container = (Rtt_UIWebViewContainer*)GetView();
		lua_pushboolean( L, [container bounces] );
	}
	else if ( strcmp( "canGoBack", key ) == 0 )
	{
		Rtt_UIWebViewContainer *container = (Rtt_UIWebViewContainer*)GetView();
		lua_pushboolean( L, container.webView.canGoBack );
	}
	else if ( strcmp( "canGoForward", key ) == 0 )
	{
		Rtt_UIWebViewContainer *container = (Rtt_UIWebViewContainer*)GetView();
		lua_pushboolean( L, container.webView.canGoForward );
	}
	else if ( strcmp( "hasBackground", key ) == 0 )
	{
		Rtt_UIWebViewContainer *container = (Rtt_UIWebViewContainer*)GetView();
		lua_pushboolean( L, container.webView.opaque );
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
IPhoneWebViewObject::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	Rtt_ASSERT( key );

	bool result = true;

	if ( strcmp( "hasBackground", key ) == 0 )
	{
		Rtt_UIWebViewContainer *container = (Rtt_UIWebViewContainer*)GetView();

		bool hasBackground = lua_toboolean( L, valueIndex );
		UIColor *color = hasBackground ? [UIColor whiteColor] : [UIColor clearColor];
		container.webView.backgroundColor = color;
		container.webView.opaque = hasBackground;
	}
	else if ( strcmp( "bounces", key ) == 0 )
	{
		Rtt_UIWebViewContainer *container = (Rtt_UIWebViewContainer*)GetView();
		[container setBounces:lua_toboolean( L, valueIndex )];
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

id
IPhoneWebViewObject::GetNativeTarget() const
{
	Rtt_UIWebViewContainer *container = (Rtt_UIWebViewContainer*)GetView();
	return container.webView;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
