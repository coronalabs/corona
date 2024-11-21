//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_IPhoneWebViewObject.h"

#import <UIKit/UIKit.h>
#include <math.h>
#import <WebKit/WebKit.h>
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

#import "CoronaLuaObjC+NSObject.h"

#define JS(...)  [[NSString alloc] initWithCString:#__VA_ARGS__ encoding:NSUTF8StringEncoding]

// ----------------------------------------------------------------------------

static CGFloat kAnimationDuration = 0.3;

NSString * const kCoronaEventPrefix = @"JS_";
NSString * const kCorona4JS = @"corona";
NSString * const kNativeBridgeCode = JS(
	const NativeBridge = {
		callNative: function(method, args) {
			return new Promise((resolve, reject) => {
				var eventName = "JS_" + method;
				window.addEventListener(eventName, function(e) {
					resolve(e.detail);
				}, { once: true });
				window.webkit.messageHandlers.corona.postMessage({
					type: eventName,
					data: JSON.stringify(args),
					noResult: false
				});
			});
		},
		sendToLua: function(event, data) {
			var eventName = "JS_" + event;
			window.webkit.messageHandlers.corona.postMessage({
				type: eventName,
				data: JSON.stringify(data),
				noResult: true
			});
		},
		on: function(event, callback, options) {
			var eventName = "JS_" + event;
			window.addEventListener(eventName, function(e) {
				callback(e.detail)
			}, options);
		}
	};
);

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

@interface Rtt_iOSWebViewContainer : UIView< WKNavigationDelegate, WKUIDelegate, WKScriptMessageHandler >
{
	Rtt::IPhoneWebViewObject *fOwner;
	WKWebViewConfiguration *fWebViewConfiguration;
	WKWebView *fWebView;
	NSURL *fLoadingURL;
	UIView *fActivityView;
	BOOL isOpen;
	BOOL keyboardShown;
	BOOL isLoading;
	UIInterfaceOrientation initialOrientation;
}

@property(nonatomic,assign,getter=owner,setter=setOwner:) Rtt::IPhoneWebViewObject *fOwner;
@property(nonatomic,readonly,getter=webView) WKWebView *fWebView;
@property(nonatomic,readonly,getter=webViewConfiguration) WKWebViewConfiguration *fWebViewConfiguration;
@property(nonatomic,readonly) BOOL isOpen;

- (id)initWithFrame:(CGRect)rect;

- (void)addObservers;
- (void)removeObservers;
- (void)loadHtmlString:(NSString*)htmlString baseURL:(NSURL*)baseUrl;
- (void)loadRequest:(NSURLRequest*)request baseURL:(NSURL*)baseUrl;
- (void)stopLoading;
- (BOOL)back;
- (BOOL)forward;
- (void)reload;

- (BOOL)bounces;
- (void)setBounces:(BOOL)newValue;

@end

@implementation Rtt_iOSWebViewContainer

@synthesize fOwner;
@synthesize fWebView;
@synthesize isOpen;
@synthesize fWebViewConfiguration;

-(WKWebView*)webView
{
	if(fWebView == nil) {
        // Propagate the w,h, but do not propagate the origin, as the parent already accounts for it.
        CGRect rect = [super frame];
		CGRect webViewRect = rect;
		webViewRect.origin = CGPointZero;
		fWebView = [[WKWebView alloc] initWithFrame:webViewRect configuration:fWebViewConfiguration];
		[fWebViewConfiguration release];
		fWebViewConfiguration = nil;
		fWebView.navigationDelegate = self;
		fWebView.UIDelegate = self;
//		fWebView.scalesPageToFit = YES;
        
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
	}
	return fWebView;
}

- (id)initWithFrame:(CGRect)rect
{
	self = [super initWithFrame:rect];
	if ( self )
	{
		fWebViewConfiguration = [[WKWebViewConfiguration alloc] init];
		WKUserContentController *userContentController = [[WKUserContentController alloc] init];
		[userContentController addScriptMessageHandler:self name:kCorona4JS];
		fWebViewConfiguration.userContentController = userContentController;

		fOwner = nil;
		fLoadingURL = nil;
		fWebView = nil;

		isOpen = false;
		keyboardShown = NO;
		isLoading = NO;
		        
        [self addObservers];
	}

	return self;
}

- (void)dealloc
{
	[self removeObservers];

	[fActivityView release];
	[fLoadingURL release];

	[fWebView setNavigationDelegate:nil];
	[fWebView stopLoading];
	[fWebView release];
	[fWebViewConfiguration release];

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
		[self.webView stopLoading];
	}
	
	fLoadingURL = nil;
	
	if(htmlString)
	{
		isLoading = true;
		[self.webView loadHTMLString:htmlString baseURL:baseUrl];
	}
	
}
- (void)loadRequest:(NSURLRequest*)request baseURL:(NSURL*)baseUrl
{
	if ( isLoading )
	{
		[self.webView stopLoading];
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
	if([fLoadingURL isFileURL] && baseUrl && [self.webView respondsToSelector:@selector(loadFileURL:allowingReadAccessToURL:)]) {
		[self.webView loadFileURL:request.URL allowingReadAccessToURL:baseUrl];
	} else {
		[self.webView loadRequest:request];
	}

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
	[self.webView stopLoading];

	UIActivityIndicatorView *indicator = [[fActivityView subviews] objectAtIndex:0];
	[indicator stopAnimating];
}

- (BOOL)back
{
	BOOL result = self.webView.canGoBack;

	if ( result )
	{
		[self.webView goBack];
	}

	return result;
}

- (BOOL)forward
{
	BOOL result = self.webView.canGoForward;

	if ( result )
	{
		[self.webView goForward];
	}

	return result;
}

- (void)reload
{
	[self.webView reload];
}

- (BOOL)bounces
{
	BOOL result = YES;

	for ( id subview in self.webView.subviews )
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
	for ( id subview in self.webView.subviews )
	{
		if ( [[subview class] isSubclassOfClass:[UIScrollView class]] )
		{
			((UIScrollView*)subview).bounces = newValue;
		}
	}
}

- (void)keyboardWasShown:(NSNotification*)aNotification
{
	if ( ! keyboardShown && [self.webView isFirstResponder] )
	{
		NSDictionary* info = [aNotification userInfo];

		// Get the size of the keyboard.
		// (Deprecated const takes into account rotation, otherwise use: UIKeyboardFrameBeginUserInfoKey)
		NSValue* aValue = [info objectForKey:UIKeyboardBoundsUserInfoKey];
		CGSize keyboardSize = [aValue CGRectValue].size;

		WKWebView *view = self.webView;

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
	if ( [self.webView isFirstResponder] )
	{
		NSDictionary* info = [aNotification userInfo];
	 
		// Get the size of the keyboard.
		NSValue* aValue = [info objectForKey:UIKeyboardBoundsUserInfoKey];
		CGSize keyboardSize = [aValue CGRectValue].size;
	 
		WKWebView *view = self.webView;

		// Reset the height of the scroll view to its original value
		CGRect viewFrame = [view frame];
		viewFrame.size.height += keyboardSize.height;
		view.frame = viewFrame;
	 
		keyboardShown = NO;
	}
}

Rtt::UrlRequestEvent::Type
static EventTypeForNavigationType( WKNavigationType t )
{
	using namespace Rtt;

	UrlRequestEvent::Type result = UrlRequestEvent::kUnknown;

	switch ( t )
	{
		case WKNavigationTypeLinkActivated:
			result = UrlRequestEvent::kLink;
			break;
		case WKNavigationTypeFormSubmitted:
			result = UrlRequestEvent::kForm;
			break;
		case WKNavigationTypeBackForward:
			result = UrlRequestEvent::kHistory;
			break;
		case WKNavigationTypeReload:
			result = UrlRequestEvent::kReload;
			break;
		case WKNavigationTypeFormResubmitted:
			result = UrlRequestEvent::kFormResubmitted;
			break;
		case WKNavigationTypeOther:
			result = UrlRequestEvent::kOther;
			break;
		default:
			break;
	}

	return result;
}
- (void)webView:(WKWebView *)webView decidePolicyForNavigationAction:(WKNavigationAction *)navigationAction decisionHandler:(void (^)(WKNavigationActionPolicy))decisionHandler {

	using namespace Rtt;

	NSURL *url = navigationAction.request.URL;

	const char *urlString = [[url absoluteString] UTF8String];
	
	Rtt::UrlRequestEvent::Type navType = EventTypeForNavigationType( navigationAction.navigationType );
	
	UrlRequestEvent e( urlString, navType );
	
	fOwner->DispatchEventWithTarget( e );
	
	if (navType == UrlRequestEvent::kLink ||
		navType == UrlRequestEvent::kHistory ||
		navType == UrlRequestEvent::kReload )
	{
		isLoading = true;
	}
	
	decisionHandler(WKNavigationActionPolicyAllow); // Always load
}

- (WKWebView *)webView:(WKWebView *)webView createWebViewWithConfiguration:(WKWebViewConfiguration *)configuration forNavigationAction:(WKNavigationAction *)navigationAction windowFeatures:(WKWindowFeatures *)windowFeatures
{
	if (!navigationAction.targetFrame.isMainFrame) {
		[webView loadRequest:navigationAction.request];
	}
	return nil;
}

- (void)webView:(WKWebView *)webView didStartProvisionalNavigation:(WKNavigation *)navigation
{
	UIActivityIndicatorView *indicator = [[fActivityView subviews] objectAtIndex:0];
	[indicator startAnimating];
}

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation
{
	using namespace Rtt;

	if ( isLoading )
	{
		isLoading = NO;

		[self hideActivityViewIndicator];

		[fWebView evaluateJavaScript:kNativeBridgeCode completionHandler:nil];

		NSURL *url = webView.URL;
		const char *urlString = [[url absoluteString] UTF8String];
		UrlRequestEvent e( urlString, UrlRequestEvent::kLoaded );
		fOwner->DispatchEventWithTarget( e );
	}
}

- (void)webView:(WKWebView *)webView didFailNavigation:(WKNavigation *)navigation withError:(NSError *)error
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
	
	self.webView.hidden = NO;
	self.webView.alpha = 1.0;
	fActivityView.alpha = 1.0;
	[UIView beginAnimations:nil context:nil];
	[UIView setAnimationDuration:kAnimationDuration];
	[UIView setAnimationDelegate:self];
	[UIView setAnimationDidStopSelector:@selector(didAppear:finished:context:)];
	fActivityView.alpha = 0.0;
	[UIView commitAnimations];
}

// Implement WKScriptMessageHandler protocol
- (void)userContentController:(WKUserContentController *)userContentController didReceiveScriptMessage:(WKScriptMessage *)message
{
	if ([message.name isEqualToString:kCorona4JS])
	{
		NSDictionary *messageBody = message.body;

		using namespace Rtt;

		const char* type = [messageBody[@"type"] UTF8String];
		const char* data = [messageBody[@"data"] UTF8String];
		bool noResult =  [messageBody[@"noResult"] boolValue];

		CommonEvent e(type, data);

		lua_State *L = fOwner->GetL();
		int status = fOwner->DisplayObject::DispatchEventWithTarget( L, e, 1 );
		if ( status == 0 && (! noResult ) )
		{
			int retValueIndex = lua_gettop( L );
			const char* jsonContent = "{}";
			if ( 0 == LuaContext::JsonEncode( L, retValueIndex ) )
			{
				jsonContent = lua_tostring( L, -1 );
			}

			NSString *jsCode = [NSString stringWithFormat:@"window.dispatchEvent(new CustomEvent('%s', { detail: %s }));", type, jsonContent];
			[fWebView evaluateJavaScript:jsCode completionHandler:^(id _Nullable response, NSError * _Nullable error){
				if ( error != nil ) {
					NSLog(@"WKUserContentController error: %s, %@", type, error);
				}
			}];
			lua_pop( L, 1 );
		}
		lua_pop( L, 1 );
	}
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
	Rtt_iOSWebViewContainer *v = (Rtt_iOSWebViewContainer*)GetView();
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
	Rtt_iOSWebViewContainer *v = [[Rtt_iOSWebViewContainer alloc] initWithFrame:r];
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
	Rtt_iOSWebViewContainer *container = (Rtt_iOSWebViewContainer*)GetView();
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

	Rtt_iOSWebViewContainer *container = (Rtt_iOSWebViewContainer*)GetView();
	[container loadRequest:request baseURL:baseUrl];

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
		Rtt_iOSWebViewContainer *container = (Rtt_iOSWebViewContainer*)view;
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
		Rtt_iOSWebViewContainer *container = (Rtt_iOSWebViewContainer*)view;
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
		Rtt_iOSWebViewContainer *container = (Rtt_iOSWebViewContainer*)view;
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
		Rtt_iOSWebViewContainer *container = (Rtt_iOSWebViewContainer*)view;
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
		Rtt_iOSWebViewContainer *container = (Rtt_iOSWebViewContainer*)view;
		container.webView.backgroundColor = color;
		container.webView.opaque = hasBackground;
	}

	return 0;
}
*/

int
IPhoneWebViewObject::InjectJS( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	IPhoneWebViewObject *o = (IPhoneWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		Rtt_iOSWebViewContainer *container = (Rtt_iOSWebViewContainer*)o->GetView();
		const char *jsContent = lua_tostring( L, 2 );
		NSString *jsCode = [NSString stringWithUTF8String:jsContent];
		// Rtt_Log( "InjectJS: %s ", [jsCode UTF8String] );

		[container.webView evaluateJavaScript:jsCode completionHandler:^(id _Nullable response, NSError * _Nullable error){
			if ( error != nil ) {
				NSLog(@"InjectJS error: %@", error);
			}
		}];
	}

	return 0;
}

int
IPhoneWebViewObject::RegisterCallback( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	IPhoneWebViewObject *o = (IPhoneWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		const char *eventName = lua_tostring( L, 2 );
		NSString *jsEventName = [NSString stringWithFormat:@"%@%s", kCoronaEventPrefix, eventName];
		o->AddEventListener( L, 3, [jsEventName UTF8String] );
	}

	return 0;
}

int
IPhoneWebViewObject::On( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	IPhoneWebViewObject *o = (IPhoneWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		const char *eventName = lua_tostring( L, 2 );
		NSString *jsEventName = [NSString stringWithFormat:@"%@%s", kCoronaEventPrefix, eventName];
		o->AddEventListener( L, 3, [jsEventName UTF8String] );
	}

	return 0;
}

int
IPhoneWebViewObject::Send( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	IPhoneWebViewObject *o = (IPhoneWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		const char* eventName = lua_tostring( L, 2 );

		Rtt_iOSWebViewContainer *container = (Rtt_iOSWebViewContainer*)o->GetView();
		const char* jsonContent = "{}";
		if ( 0 == LuaContext::JsonEncode( L, 3 ) )
		{
			jsonContent = lua_tostring( L, -1 );
		}

		NSString *jsCode = [NSString stringWithFormat:@"window.dispatchEvent(new CustomEvent('%@%s', { detail: %s }));", kCoronaEventPrefix, eventName, jsonContent];
		[container.webView evaluateJavaScript:jsCode completionHandler:^(id _Nullable response, NSError * _Nullable error){
			if ( error != nil ) {
				NSLog(@"Send '%s' error: %@", eventName, error);
			}
		}];
		lua_pop( L, 1 );
	}

	return 0;
}

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
	else if ( strcmp( "injectJS", key ) == 0 )
	{
		lua_pushcfunction( L, InjectJS );
	}
	else if ( strcmp( "registerCallback", key ) == 0 )
	{
		lua_pushcfunction( L, RegisterCallback );
	}
	else if ( strcmp( "on", key ) == 0 )
	{
		lua_pushcfunction( L, On );
	}
	else if ( strcmp( "send", key ) == 0 )
	{
		lua_pushcfunction( L, Send );
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
		Rtt_iOSWebViewContainer *container = (Rtt_iOSWebViewContainer*)GetView();
		lua_pushboolean( L, [container bounces] );
	}
	else if ( strcmp( "canGoBack", key ) == 0 )
	{
		Rtt_iOSWebViewContainer *container = (Rtt_iOSWebViewContainer*)GetView();
		lua_pushboolean( L, container.webView.canGoBack );
	}
	else if ( strcmp( "canGoForward", key ) == 0 )
	{
		Rtt_iOSWebViewContainer *container = (Rtt_iOSWebViewContainer*)GetView();
		lua_pushboolean( L, container.webView.canGoForward );
	}
	else if ( strcmp( "hasBackground", key ) == 0 )
	{
		Rtt_iOSWebViewContainer *container = (Rtt_iOSWebViewContainer*)GetView();
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
		Rtt_iOSWebViewContainer *container = (Rtt_iOSWebViewContainer*)GetView();

		bool hasBackground = lua_toboolean( L, valueIndex );
		UIColor *color = hasBackground ? [UIColor whiteColor] : [UIColor clearColor];
		container.webView.backgroundColor = color;
		container.webView.opaque = hasBackground;
	}
	else if ( strcmp( "bounces", key ) == 0 )
	{
		Rtt_iOSWebViewContainer *container = (Rtt_iOSWebViewContainer*)GetView();
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
	Rtt_iOSWebViewContainer *container = (Rtt_iOSWebViewContainer*)GetView();
	return container.webView;
}

int
IPhoneWebViewObject::GetNativeProperty( lua_State *L, const char key[] ) const
{
	Rtt_iOSWebViewContainer *container = (Rtt_iOSWebViewContainer*)GetView();
	id target = container.webViewConfiguration;
	int result = [target pushLuaValue:L forKey:@(key)];

	if ( 0 == result )
	{
		result = Super::GetNativeProperty( L, key );
	}

	return result;
}

bool
IPhoneWebViewObject::SetNativeProperty( lua_State *L, const char key[], int valueIndex )
{
	Rtt_iOSWebViewContainer *container = (Rtt_iOSWebViewContainer*)GetView();
	id target = container.webViewConfiguration;
	bool result = [target set:L luaValue:valueIndex forKey:@(key)];

	if ( ! result )
	{
		result = Super::SetNativeProperty( L, key, valueIndex );
	}

	return result;
}


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
