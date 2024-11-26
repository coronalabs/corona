//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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

#define JS(...)  [[NSString alloc] initWithCString:#__VA_ARGS__ encoding:NSUTF8StringEncoding]

// ----------------------------------------------------------------------------
NSString * const kCoronaEventPrefix = @"JS_";
NSString * const kCorona4JS = @"corona";
NSString * const kNativeBridgeCode = JS(
	window.NativeBridge = {
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

	window.originalConsole = window.console;
	window.console = {
		log: function() {
			var args = Array.prototype.slice.call(arguments);
			window.webkit.messageHandlers.console.postMessage({
				type: 'log',
				message: args.map(function(arg) { return JSON.stringify(arg); }).join(', ')
			});
		},
		warn: function() {
			var args = Array.prototype.slice.call(arguments);
			window.webkit.messageHandlers.console.postMessage({
				type: 'warn',
				message: args.map(function(arg) { return JSON.stringify(arg); }).join(', ')
			});
		},
		error: function() {
			var args = Array.prototype.slice.call(arguments);
			window.webkit.messageHandlers.console.postMessage({
				type: 'error',
				message: args.map(function(arg) { return JSON.stringify(arg); }).join(', ')
			});
		}
	};
);
NSString * const kOnLoadedJSCode = JS(
	if (window.onNativeBridgeLoaded != undefined) {
		window.onNativeBridgeLoaded();
		delete window.onNativeBridgeLoaded;
	}
);

@interface Rtt_WebView : WKWebView <WKNavigationDelegate, WKUIDelegate, WKScriptMessageHandler>

@property(nonatomic, assign) Rtt::MacWebViewObject *owner;

- (id)initWithFrame:(CGRect)rect;

@end

@implementation Rtt_WebView

@synthesize owner;

- (id)initWithFrame:(CGRect)frameRect
{
	WKWebViewConfiguration *configuration = [[WKWebViewConfiguration alloc] init];
	configuration.preferences.javaScriptEnabled = YES;
	configuration.preferences.javaScriptCanOpenWindowsAutomatically = YES;

	// Add userContentController
	WKUserContentController *userContentController = [[WKUserContentController alloc] init];
	WKUserScript *consoleScript = [[WKUserScript alloc] initWithSource:kNativeBridgeCode
														injectionTime:WKUserScriptInjectionTimeAtDocumentStart
														forMainFrameOnly:YES];
	[userContentController addUserScript:consoleScript];
	[userContentController addScriptMessageHandler:self name:kCorona4JS];
	[userContentController addScriptMessageHandler:self name:@"console"];

	configuration.userContentController = userContentController;

	self = [super initWithFrame:frameRect configuration:configuration];
	if (self)
	{
		owner = NULL;
		[self setWantsLayer:YES];
		[self setNavigationDelegate:self];
		[self setUIDelegate:self];
	}
	return self;
}

- (void)webView:(WKWebView *)webView decidePolicyForNavigationAction:(WKNavigationAction *)navigationAction decisionHandler:(void (^)(WKNavigationActionPolicy))decisionHandler
{
	using namespace Rtt;

	NSURL *url = navigationAction.request.URL;
	BOOL shouldLoad = YES;

	// Handle special protocols (like tel:, mailto:, etc.)
	if (![url.scheme isEqualToString:@"http"] &&
		![url.scheme isEqualToString:@"https"] &&
		![url.scheme isEqualToString:@"file"] &&
		![url.scheme isEqualToString:@"about"])
	{
		// For special protocols, use the system default handling method
		if ([[NSWorkspace sharedWorkspace] openURL:url]) {
			shouldLoad = NO;
		}
	}

	UrlRequestEvent::Type urlRequestType = UrlRequestEvent::kUnknown;
	switch (navigationAction.navigationType) {
		case WKNavigationTypeLinkActivated:
			urlRequestType = UrlRequestEvent::kLink;
			break;
		case WKNavigationTypeFormSubmitted:
			urlRequestType = UrlRequestEvent::kForm;
			break;
		case WKNavigationTypeBackForward:
			urlRequestType = UrlRequestEvent::kHistory;
			break;
		case WKNavigationTypeReload:
			urlRequestType = UrlRequestEvent::kReload;
			break;
		case WKNavigationTypeFormResubmitted:
			urlRequestType = UrlRequestEvent::kFormResubmitted;
			break;
		case WKNavigationTypeOther:
			urlRequestType = UrlRequestEvent::kOther;
			break;
	}

	const char *urlString = [[url absoluteString] UTF8String];
	UrlRequestEvent e(urlString, urlRequestType);
	owner->DispatchEventWithTarget(e);

	// Determine whether to allow navigation based on shouldLoad
	decisionHandler(shouldLoad ? WKNavigationActionPolicyAllow : WKNavigationActionPolicyCancel);
}

- (WKWebView *)webView:(WKWebView *)webView createWebViewWithConfiguration:(WKWebViewConfiguration *)configuration forNavigationAction:(WKNavigationAction *)navigationAction windowFeatures:(WKWindowFeatures *)windowFeatures
{
	// If a link opens in a new window, open it in the current window instead
	if (!navigationAction.targetFrame.isMainFrame) {
		[webView loadRequest:navigationAction.request];
	}
	return nil;
}

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation
{
	[self evaluateJavaScript:kOnLoadedJSCode completionHandler:nil];

	using namespace Rtt;

	const char *urlString = [webView.URL.absoluteString UTF8String];
	UrlRequestEvent e(urlString, UrlRequestEvent::kLoaded);
	owner->DispatchEventWithTarget(e);
}

- (void)webView:(WKWebView *)webView didFailNavigation:(WKNavigation *)navigation withError:(NSError *)error
{
	using namespace Rtt;

	const char *urlString = [error.userInfo[NSURLErrorFailingURLStringErrorKey] UTF8String];

	UrlRequestEvent e(urlString, [[error localizedDescription] UTF8String], (S32)[error code]);
	owner->DispatchEventWithTarget(e);
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

		lua_State *L = owner->GetL();
		int status = owner->DisplayObject::DispatchEventWithTarget( L, e, 1 );
		if ( status == 0 && (! noResult ) )
		{
			int retValueIndex = lua_gettop( L );
			const char* jsonContent = "{}";
			if ( 0 == LuaContext::JsonEncode( L, retValueIndex ) )
			{
				jsonContent = lua_tostring( L, -1 );
			}

			NSString *jsCode = [NSString stringWithFormat:@"window.dispatchEvent(new CustomEvent('%s', { detail: %s }));", type, jsonContent];
			[self evaluateJavaScript:jsCode completionHandler:^(id _Nullable response, NSError * _Nullable error){
				if ( error != nil ) {
					NSLog(@"WKUserContentController error: %s, %@", type, error);
				}
			}];
			lua_pop( L, 1 );
		}
		lua_pop( L, 1 );
	}
	else if ([message.name isEqualToString:@"console"])
	{
		NSDictionary *logData = message.body;
		NSString *type = logData[@"type"];
		NSString *logMessage = logData[@"message"];

		if ([type isEqualToString:@"error"]) {
			NSLog(@"[WebView_ERROR] %@", logMessage);
		} else if ([type isEqualToString:@"warn"]) {
			NSLog(@"[WebView_WARNING] %@", logMessage);
		} else if ([type isEqualToString:@"info"]) {
			NSLog(@"[WebView_INFO] %@", logMessage);
		} else if ([type isEqualToString:@"debug"]) {
			NSLog(@"[WebView_DEBUG] %@", logMessage);
		} else {
			NSLog(@"[WebView_LOG] %@", logMessage);
		}
	}
}

// Handle JavaScript alert()
- (void)webView:(WKWebView *)webView runJavaScriptAlertPanelWithMessage:(NSString *)message initiatedByFrame:(WKFrameInfo *)frame completionHandler:(void (^)(void))completionHandler
{
	NSAlert *alert = [[NSAlert alloc] init];
	[alert setMessageText:message];
	// Use system default "OK" button
	[[alert addButtonWithTitle:NSLocalizedString(@"OK", nil)] setKeyEquivalent:@"\r"];
	[alert runModal];
	completionHandler();
}

// Handle JavaScript confirm()
- (void)webView:(WKWebView *)webView runJavaScriptConfirmPanelWithMessage:(NSString *)message initiatedByFrame:(WKFrameInfo *)frame completionHandler:(void (^)(BOOL))completionHandler
{
	NSAlert *alert = [[NSAlert alloc] init];
	[alert setMessageText:message];
	// Use system default "OK" and "Cancel" buttons
	[[alert addButtonWithTitle:NSLocalizedString(@"OK", nil)] setKeyEquivalent:@"\r"];
	[[alert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)] setKeyEquivalent:@"\033"];

	NSModalResponse response = [alert runModal];
	completionHandler(response == NSAlertFirstButtonReturn);
}

// Handle JavaScript prompt()
- (void)webView:(WKWebView *)webView runJavaScriptTextInputPanelWithPrompt:(NSString *)prompt defaultText:(NSString *)defaultText initiatedByFrame:(WKFrameInfo *)frame completionHandler:(void (^)(NSString *))completionHandler
{
	NSAlert *alert = [[NSAlert alloc] init];
	[alert setMessageText:prompt];
	// Use system default "OK" and "Cancel" buttons
	[[alert addButtonWithTitle:NSLocalizedString(@"OK", nil)] setKeyEquivalent:@"\r"];
	[[alert addButtonWithTitle:NSLocalizedString(@"Cancel", nil)] setKeyEquivalent:@"\033"];

	NSTextField *input = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 200, 24)];
	[input setStringValue:defaultText];
	[alert setAccessoryView:input];

	NSModalResponse response = [alert runModal];
	if (response == NSAlertFirstButtonReturn) {
		completionHandler([input stringValue]);
	} else {
		completionHandler(nil);
	}
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
	[view.configuration.userContentController removeScriptMessageHandlerForName:kCorona4JS];
	[view.configuration.userContentController removeScriptMessageHandlerForName:@"console"];
//    [view setDelegate:nil];
	[view stopLoading:nil];
//    [view close];
	view.owner = NULL;
	view = NULL;
}

bool
MacWebViewObject::Initialize()
{
	Rtt_ASSERT( ! GetView() );

	Rect screenBounds;
	GetScreenBounds( screenBounds );

	CGRect r = CGRectMake( screenBounds.xMin, screenBounds.yMin, screenBounds.Width(), screenBounds.Height() );

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
	[container loadHTMLString:htmlBody baseURL:baseUrl];
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
MacWebViewObject::InjectJS( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	MacWebViewObject *o = (MacWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		Rtt_WebView *view = (Rtt_WebView*)o->GetView();
		const char *jsContent = lua_tostring( L, 2 );
		NSString *jsCode = [NSString stringWithExternalString:jsContent];

		[view evaluateJavaScript:jsCode completionHandler:^(id _Nullable response, NSError * _Nullable error){
			if ( error != nil ) {
				NSLog(@"InjectJS error: %@", error);
			}
		}];
	}

	return 0;
}

int
MacWebViewObject::RegisterCallback( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	MacWebViewObject *o = (MacWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		const char *eventName = lua_tostring( L, 2 );
		NSString *jsEventName = [NSString stringWithFormat:@"%@%s", kCoronaEventPrefix, eventName];
		o->AddEventListener( L, 3, [jsEventName UTF8String] );
	}

	return 0;
}

int
MacWebViewObject::On( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	MacWebViewObject *o = (MacWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		const char *eventName = lua_tostring( L, 2 );
		NSString *jsEventName = [NSString stringWithFormat:@"%@%s", kCoronaEventPrefix, eventName];
		o->AddEventListener( L, 3, [jsEventName UTF8String] );
	}

	return 0;
}

int
MacWebViewObject::Send( lua_State *L )
{
	const LuaProxyVTable& table = PlatformDisplayObject::GetWebViewObjectProxyVTable();
	MacWebViewObject *o = (MacWebViewObject *)luaL_todisplayobject( L, 1, table );
	if ( o )
	{
		const char* eventName = lua_tostring( L, 2 );

		Rtt_WebView *view = (Rtt_WebView*)o->GetView();
		const char* jsonContent = "{}";
		if ( 0 == LuaContext::JsonEncode( L, 3 ) )
		{
			jsonContent = lua_tostring( L, -1 );
		}

		NSString *jsCode = [NSString stringWithFormat:@"window.dispatchEvent(new CustomEvent('%@%s', { detail: %s }));", kCoronaEventPrefix, eventName, jsonContent];
		[view evaluateJavaScript:jsCode completionHandler:^(id _Nullable response, NSError * _Nullable error){
			if ( error != nil ) {
				NSLog(@"Send '%s' error: %@", eventName, error);
			}
		}];
		lua_pop( L, 1 );
	}

	return 0;
}

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
		lua_pushboolean( L, false );
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
		bool hasBackground = lua_toboolean( L, valueIndex );
		[view setValue:hasBackground ? @YES : @NO forKey:@"drawsBackground"];
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

