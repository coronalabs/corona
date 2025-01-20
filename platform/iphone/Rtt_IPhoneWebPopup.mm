//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_IPhoneWebPopup.h"

#include "Core/Rtt_String.h"
#include "Rtt_LuaAux.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_MKeyValueIterable.h"
#include "Rtt_Runtime.h"

#import "AppDelegate.h"

#import <Foundation/NSDictionary.h>
#import <Foundation/NSValue.h>
#import <UIKit/UIApplication.h>
#import <UIKit/UIGeometry.h>
#import <UIKit/UIScreen.h>
#import <UIKit/UIView.h>
#import <WebKit/WebKit.h>
#import <UIKit/UIWindow.h>

// ----------------------------------------------------------------------------

static CGFloat kAnimationDuration = 0.3;

static void
RectToCGRect( const Rtt::Rect& bounds, CGRect * outRect )
{
	if ( bounds.NotEmpty() )
	{
		Rtt::Real x = bounds.xMin;
		Rtt::Real y = bounds.yMin;
		outRect->origin.x = Rtt_RealToFloat( x );
		outRect->origin.y = Rtt_RealToFloat( y );

		Rtt::Real w = bounds.xMax - x;
		Rtt::Real h = bounds.yMax - y;
		outRect->size.width = Rtt_RealToFloat( w );
		outRect->size.height = Rtt_RealToFloat( h );
	}
	else
	{
		*outRect = CGRectNull;
	}
}

// Callback/Notification glue code
@interface IPhoneWebView : NSObject< WKNavigationDelegate >
{
	Rtt::IPhoneWebPopup *owner;
	WKWebView *fWebView;
	NSURL *fLoadingURL;
	UIView *fActivityView;
	BOOL isOpen;
	BOOL keyboardShown;
	BOOL isLoading;
	UIInterfaceOrientation initialOrientation;
	CGRect initialBounds;
}

@property(nonatomic,readonly,getter=webView) WKWebView *fWebView;
@property(nonatomic,readonly) BOOL isOpen;

- (id)initWithOwner:(Rtt::IPhoneWebPopup*)popup;
- (void)addObservers;
- (void)removeObservers;

- (void)openWithRequest:(NSURLRequest*)request  baseURL:(NSURL*)baseUrl;;

- (void)close;
- (void)finishClose;

@end


@implementation IPhoneWebView

@synthesize fWebView;
@synthesize isOpen;

- (id)initWithOwner:(Rtt::IPhoneWebPopup*)popup
{
	self = [super init];

	if ( self )
	{
		owner = popup;

		fWebView = nil;
		fLoadingURL = nil;

//		CGRect frame = [UIScreen mainScreen].applicationFrame;
		fActivityView = [[UIView alloc] initWithFrame:CGRectZero];
		fActivityView.backgroundColor = [UIColor grayColor];
//		fActivityView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
		
		UIActivityIndicatorView *indicator = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhiteLarge];
		[fActivityView addSubview:indicator];
//		CGPoint center = { frame.origin.x + 0.5*CGRectGetWidth( frame ), frame.origin.y + 0.5*CGRectGetHeight( frame ) };
//		indicator.center = center;
		
		[indicator release];
//		[fWebView addSubview:fActivityView];

		isOpen = false;
		keyboardShown = NO;
		isLoading = NO;

		// TODO: Is this really necessary?  We do this in openWithRequest:
		AppDelegate *delegate = (AppDelegate *)[UIApplication sharedApplication].delegate;
		UIViewController *viewController = delegate.viewController;
		initialOrientation = viewController.interfaceOrientation;
	}

	return self;
}

- (void)dealloc
{
	[fActivityView release];
	[fLoadingURL release];
	[fWebView release];

	[super dealloc];
}

- (void)initView
{
	if ( ( ! fWebView ) )
	{
		fWebView = [[WKWebView alloc] initWithFrame:CGRectZero];
		fWebView.navigationDelegate = self;
//		fWebView.scalesPageToFit = YES;
	}
}

- (void)addObservers
{
	NSNotificationCenter *notifier = [NSNotificationCenter defaultCenter];

	[notifier addObserver:self
		selector:@selector(orientationDidChange:)
		name:UIDeviceOrientationDidChangeNotification object:nil];

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
		name:UIDeviceOrientationDidChangeNotification object:nil];

	[notifier removeObserver:self
		name:UIKeyboardDidShowNotification object:nil];

	[notifier removeObserver:self
		name:UIKeyboardDidHideNotification object:nil];
}

- (void)didAppear:(NSString *)animationID finished:(NSNumber *)finished context:(void *)context
{
}

- (void)openWithRequest:(NSURLRequest*)request baseURL:(NSURL*)baseUrl
{
	[self initView];

	[fLoadingURL release];
	fLoadingURL = [request.URL retain];

	if([fLoadingURL isFileURL] && baseUrl && [fWebView respondsToSelector:@selector(loadFileURL:allowingReadAccessToURL:)]) {
		[fWebView loadFileURL:fLoadingURL allowingReadAccessToURL:baseUrl];
	} else {
		[fWebView loadRequest:request];
	}
	isLoading = YES;

	if ( ! isOpen )
	{
		isOpen = YES;

		AppDelegate *delegate = (AppDelegate *)[UIApplication sharedApplication].delegate;
		UIViewController *viewController = delegate.viewController;
		[viewController.view addSubview:fWebView];
		fWebView.hidden = YES;
		initialOrientation = viewController.interfaceOrientation;

		CGRect stageBounds;
		Rtt::Rect screenBounds;
		owner->GetScreenBounds( delegate.runtime->GetDisplay(), screenBounds );
		RectToCGRect( screenBounds, &stageBounds );

		CGRect frame = ( CGRectIsNull( stageBounds ) ? [UIScreen mainScreen].applicationFrame : stageBounds );
		if ( CGRectIsNull( stageBounds ) && UIInterfaceOrientationIsLandscape( initialOrientation ) )
		{
			// Swap w,h b/c UIScreen provides unrotated bounds
			Rtt::Swap( frame.size.width, frame.size.height );
		}
		initialBounds = frame;
		fWebView.frame = frame;

/*
		frame.origin = CGPointZero;
		fActivityView.frame = frame;
		CGPoint center = { 0.5*CGRectGetWidth( frame ), 0.5*CGRectGetHeight( frame ) };
		UIView *indicator = [[fActivityView subviews] objectAtIndex:0];
		[indicator setCenter:center];
*/

//		frame.origin = CGPointZero;
		[viewController.view addSubview:fActivityView];
		fActivityView.frame = frame;

		UIActivityIndicatorView *indicator = [[fActivityView subviews] objectAtIndex:0];
		CGPoint center = { (CGFloat)0.5f*CGRectGetWidth( frame ), (CGFloat)0.5f*CGRectGetHeight( frame ) };
		[indicator setCenter:center];

		fWebView.alpha = 0.0;
		fActivityView.alpha = 0.0;
		[UIView beginAnimations:nil context:nil];
		[UIView setAnimationDuration:kAnimationDuration];
		[UIView setAnimationDelegate:self];
		[UIView setAnimationDidStopSelector:@selector(didAppear:finished:context:)];
		fActivityView.alpha = 1.0;
		[UIView commitAnimations];
    
		[self addObservers];
	}
}

- (void)close
{
	isLoading = NO;

	if ( isOpen )
	{
		isOpen = NO;

		[fLoadingURL release];
		fLoadingURL = nil;

		[UIView beginAnimations:nil context:nil];
		[UIView setAnimationDuration:kAnimationDuration];
		[UIView setAnimationDelegate:self];
		[UIView setAnimationDidStopSelector:@selector(animationDidStop:finished:context:)];
		fWebView.alpha = 0;
		[UIView commitAnimations];
	}
}

- (void)finishClose
{
	[self removeObservers];
	[fWebView removeFromSuperview];
	[fActivityView removeFromSuperview];
	fWebView.alpha = 1;

	[fWebView release];
	fWebView = nil;
}

- (void)animationDidStop:(NSString *)animationID finished:(NSNumber *)finished context:(void *)context
{
	if ( ! isOpen )
	{
		[self finishClose];
	}
}
/*
- (void)drawRect:(CGRect)rect
{
	CGContextRef context = UIGraphicsGetCurrentContext();
	CGContextSaveGState( context );

	CGFloat kBorderBlack[4] = {0.3, 0.3, 0.3, 1};
	CGContextSetStrokeColor( context, kBorderBlack );
	CGContextSetLineWidth( context, 1.0 );
	UIRectFrame( rect );


	CGContextRestoreGState( context );
}
*/
- (void)keyboardWasShown:(NSNotification*)aNotification
{
	if ( ! keyboardShown && [fWebView isFirstResponder] )
	{
		NSDictionary* info = [aNotification userInfo];

		// Get the size of the keyboard.
		NSValue* aValue = [info objectForKey:UIKeyboardBoundsUserInfoKey];
		CGSize keyboardSize = [aValue CGRectValue].size;

		WKWebView *view = fWebView;

		// Resize the scroll view (which is the root view of the window)
		CGRect viewFrame = [view frame];
		viewFrame.size.height -= keyboardSize.height;
		view.frame = viewFrame;
/*
		// Scroll the active text field into view.
		CGRect textFieldRect = [activeField frame];
		[scrollView scrollRectToVisible:textFieldRect animated:YES];
*/
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
	 
		WKWebView *view = fWebView;

		// Reset the height of the scroll view to its original value
		CGRect viewFrame = [view frame];
		viewFrame.size.height += keyboardSize.height;
		view.frame = viewFrame;
	 
		keyboardShown = NO;
	}
}

#if 0
- (BOOL)shouldRotate:(UIDeviceOrientation)orientation
{
	BOOL result = NO;
	if ( ! keyboardShown && orientation != currentOrientation )
	{
		switch ( orientation )
		{
			case UIDeviceOrientationLandscapeLeft:
			case UIDeviceOrientationLandscapeRight:
			case UIDeviceOrientationPortrait:
			case UIDeviceOrientationPortraitUpsideDown:
				result = YES;
				break;
			default:
				break;
		}
	}

	return result;
}

- (void)sizeToFitOrientation:(BOOL)transform
{
	if (transform)
	{
		fWebView.transform = CGAffineTransformIdentity;
	}

	CGRect frame = [UIScreen mainScreen].applicationFrame;
	CGPoint center = CGPointMake( frame.origin.x + ceil(frame.size.width*0.5),
								  frame.origin.y + ceil(frame.size.height*0.5) );

	currentOrientation = [UIDevice currentDevice].orientation;

	if ( UIInterfaceOrientationIsLandscape( currentOrientation ) )
	{
		CGFloat tmp = frame.size.width;
		frame.size.width = frame.size.height;
		frame.size.height = tmp;
	}

	fWebView.frame = frame;
	fWebView.center = center;
	
	if ( transform )
	{
		switch( currentOrientation )
		{
			case UIInterfaceOrientationLandscapeLeft:
				fWebView.transform = CGAffineTransformMakeRotation(M_PI*1.5);
				break;
			case UIInterfaceOrientationLandscapeRight:
				fWebView.transform = CGAffineTransformMakeRotation(M_PI/2);
				break;
			case UIInterfaceOrientationPortraitUpsideDown:
				fWebView.transform = CGAffineTransformMakeRotation(-M_PI);
				break;
			default:
				break;
		}
	}
}

- (void)orientationDidChange:(void*)object
{
	UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
	if ( [self shouldRotate:orientation] )
	{
		CGFloat duration = [UIApplication sharedApplication].statusBarOrientationAnimationDuration;
		[UIView beginAnimations:nil context:nil];
		[UIView setAnimationDuration:duration];

		[self sizeToFitOrientation:YES];
		[UIView commitAnimations];
	}
}

#else

- (void)orientationDidChange:(id)object
{
	UIInterfaceOrientation startingOrientation = initialOrientation;

	AppDelegate *delegate = (AppDelegate *)[UIApplication sharedApplication].delegate;
	UIInterfaceOrientation currentOrientation = delegate.viewController.interfaceOrientation;

	bool isStartingLandscape = UIInterfaceOrientationIsLandscape( startingOrientation );
	bool isCurrentLandscape = UIInterfaceOrientationIsLandscape( currentOrientation );

	// Get starting w,h
	CGRect startingBounds;
	
	Rtt::Rect screenBounds;
	owner->GetScreenBounds( delegate.runtime->GetDisplay(), screenBounds );
	RectToCGRect( screenBounds, &startingBounds );
	CGSize newSize = ( CGRectIsNull( startingBounds ) ? initialBounds.size : startingBounds.size );
	
	if ( isStartingLandscape != isCurrentLandscape )
	{
		// Swap w,h b/c current orientation differs from starting,
		// which means the view has been auto-rotated
		// Rtt::Swap( newSize.width, newSize.height );
	}

	CGRect newFrame = fWebView.frame;
	newFrame.size = newSize;
	fWebView.frame = newFrame;
}

#endif

- (void)webView:(WKWebView *)webView decidePolicyForNavigationAction:(WKNavigationAction *)navigationAction decisionHandler:(void (^)(WKNavigationActionPolicy))decisionHandler {
	NSURL *url = navigationAction.request.URL;
	BOOL result = [fLoadingURL isEqual:url] || owner->ShouldLoadUrl( [[url absoluteString] UTF8String] );
	if ( ! result )
	{
		// Stop listening b/c listener requested popup to close
		owner->SetCallback( NULL );
		[self close];
	}
	decisionHandler(result?WKNavigationActionPolicyAllow:WKNavigationActionPolicyCancel);
}

- (void)webView:(WKWebView *)webView didStartProvisionalNavigation:(WKNavigation *)navigation {
	UIActivityIndicatorView *indicator = [[fActivityView subviews] objectAtIndex:0];
	[indicator startAnimating];
}

- (void)webView:(WKWebView *)webView didFinishNavigation:(WKNavigation *)navigation {
	if ( isLoading )
	{
		isLoading = NO;

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
}

- (void)webView:(WKWebView *)webView didFailNavigation:(WKNavigation *)navigation withError:(NSError *)error {
	isLoading = NO;

	if ( ! owner->DidFailLoadUrl( [[[error userInfo] valueForKey:NSURLErrorFailingURLStringErrorKey] UTF8String], [[error localizedDescription] UTF8String], (int)[error code] ) )
	{
		[self close];
	}
}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

IPhoneWebPopup::IPhoneWebPopup()
:	//fPopupView( [[UIView alloc] initWithFrame:CGRectZero] ),
	fWebView( [[IPhoneWebView alloc] initWithOwner:this] ),
	fMethod( @"GET" ),
	fBaseUrl( nil ),
	fBaseDirectory( MPlatform::kUnknownDir )
{
/*	UIView *popup = fPopupView;
	popup.backgroundColor = [UIColor clearColor];
	popup.autoresizesSubviews = YES;
	// popup.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
	// popup.contentMode = UIViewContentModeRedraw;

	[popup addSubview:fWebView.webView];
*/
}

IPhoneWebPopup::~IPhoneWebPopup()
{
	[fBaseUrl release];
	[fWebView finishClose];
	[fWebView release];
	//[fPopupView release];
}


/*
static NSMutableDictionary*
CreateDictionary( const MKeyValueIterable& iParams )
{
	NSMutableDictionary *result = [[NSMutableDictionary alloc] initWithCapacity:8];

	while( iParams.Next() )
	{
		NSString *k = [[NSString alloc] initWithUTF8String:iParams.Key()];
		NSString *v = [[NSString alloc] initWithUTF8String:iParams.Value()];

		[result setValue:v forKey:k];

		[v release];
		[k release];
	}

	return result;
}
*/

void
IPhoneWebPopup::Show( const MPlatform& platform, const char *url )
{
//	Load( [NSString stringWithUTF8String:url] );

	NSURL *baseUrl = nil;

	if ( MPlatform::kUnknownDir != fBaseDirectory )
	{
		Rtt_ASSERT( ! fBaseUrl );

		String path( & platform.GetAllocator() );
		platform.PathForFile( NULL, fBaseDirectory, MPlatform::kDefaultPathFlags, path );
		NSString *basePath = [NSString stringWithUTF8String:path.GetString()];
		baseUrl = [[NSURL alloc] initFileURLWithPath:basePath];
	}
	else if ( nil != fBaseUrl )
	{
		baseUrl = [[NSURL alloc] initWithString:fBaseUrl];
	}

	NSString *urlString = [NSString stringWithUTF8String:url];
	NSURL *u = (nil != baseUrl)
		? [[NSURL alloc] initWithString:urlString relativeToURL:baseUrl]
		: [[NSURL alloc] initWithString:urlString];
	NSMutableURLRequest *request = [[NSMutableURLRequest alloc] initWithURL:u];
	[u release];
	[baseUrl autorelease];

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

	[fWebView openWithRequest:request baseURL:baseUrl];

	[request release];
}

bool
IPhoneWebPopup::Close()
{
	bool didClose = fWebView.isOpen;

	// Cancel any pending loads
	WKWebView *view = fWebView.webView;
	[view stopLoading];

	[fWebView close];
	return didClose;
}

void
IPhoneWebPopup::Reset()
{
	Rtt_ASSERT_NOT_IMPLEMENTED();
}

void
IPhoneWebPopup::SetPostParams( const MKeyValueIterable& params )
{
	Rtt_ASSERT_NOT_IMPLEMENTED();
}

void
IPhoneWebPopup::Load( NSString *url )
{
	/** Cookies
	Comment
	CommentURL
	Discard
	Domain
	Expires
	Max-Age
	Name
	OriginURL
	Path
	Port
	Secure
	Value
	Version
	*/

}

int
IPhoneWebPopup::ValueForKey( lua_State *L, const char key[] ) const
{
	Rtt_ASSERT( key );

	int result = 1;

	if ( strcmp( "baseUrl", key ) == 0 )
	{
	}
	else if ( strcmp( "hasBackground", key ) == 0 )
	{
		// TODO: When we allow changing bkgd colors, we should cache this as
		// a separate property which would ignore the UIColor.
		UIColor *color = fWebView.webView.backgroundColor;
		CGFloat alpha = CGColorGetAlpha( [color CGColor] );
		lua_pushboolean( L, alpha > FLT_EPSILON );
	}
	else
	{
		result = 0;
	}

	return result;
}

bool
IPhoneWebPopup::SetValueForKey( lua_State *L, const char key[], int valueIndex )
{
	Rtt_ASSERT( key );

	bool result = true;

	[fWebView initView];

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
		UIColor *color = hasBackground ? [UIColor whiteColor] : [UIColor clearColor];
		fWebView.webView.backgroundColor = color;
		fWebView.webView.opaque = hasBackground;
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

