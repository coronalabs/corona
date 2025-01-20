//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Core/Rtt_String.h"

#include "Rtt_IPhonePlatform.h"
#include "Rtt_IPhoneTimer.h"

#include "Rtt_IPhoneAudioSessionManager.h"
#include "Rtt_IPhoneFont.h"
#include "Rtt_IPhoneImageProvider.h"
#include "Rtt_IPhoneVideoProvider.h"
#include "Rtt_AppleInAppStore.h"
#include "Rtt_IPhoneMapViewObject.h"
#include "Rtt_IPhoneScreenSurface.h"
#include "Rtt_IPhoneVideoObject.h"
#include "Rtt_IPhoneVideoPlayer.h"
#include "Rtt_IPhoneWebPopup.h"
#include "Rtt_IPhoneWebViewObject.h"

#include "Rtt_LuaLibNative.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_LuaResource.h"

#import "AppDelegate.h"
#import "CoronaViewPrivate.h"

#import <UIKit/UIApplication.h>
#import <UIKit/UIDevice.h>
#import <UIKit/UIGestureRecognizerSubclass.h>
#import <MessageUI/MFMailComposeViewController.h>
#import <MessageUI/MFMessageComposeViewController.h>
#ifdef USE_IOS_AD_SUPPORT
#import <AdSupport/ASIdentifierManager.h>
#endif // USE_IOS_AD_SUPPORT

#include "CoronaLua.h"
// TODO: Remove when we remove support for iOS 3.x
#include "Rtt_AppleBitmap.h"

#include "Rtt_TouchInhibitor.h"

// ----------------------------------------------------------------------------

@interface PopupControllerDelegate : NSObject< MFMailComposeViewControllerDelegate, MFMessageComposeViewControllerDelegate >
{
}

@end

// ----------------------------------------------------------------------------

@implementation PopupControllerDelegate

- (void)mailComposeController:(MFMailComposeViewController *)controller didFinishWithResult:(MFMailComposeResult)result error:(NSError *)error
{
	[controller dismissViewControllerAnimated:YES completion:nil];
}

- (void)messageComposeViewController:(MFMessageComposeViewController *)controller didFinishWithResult:(MessageComposeResult)result
{
	[controller dismissViewControllerAnimated:YES completion:nil];
}

@end

// ----------------------------------------------------------------------------

// Consume all touches, preventing their propagation
@interface CoronaNullGestureRecognizer : UIGestureRecognizer
@end

@implementation CoronaNullGestureRecognizer

- (instancetype)init
{
	self = [super initWithTarget:self action:@selector(handleGesture)];
	return self;
}

- (void)handleGesture
{
	// no-op
}

- (void)reset
{
	self.state = UIGestureRecognizerStatePossible;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	self.state = UIGestureRecognizerStateRecognized;
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	self.state = UIGestureRecognizerStateRecognized;
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	self.state = UIGestureRecognizerStateRecognized;
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	self.state = UIGestureRecognizerStateRecognized;
}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

IPhonePlatform::IPhonePlatform( CoronaView *view )
:	Super( view ),
	fVideoPlayer( NULL ),
	fInAppStoreProvider( NULL ),
	fImageProvider( NULL ),
	fVideoProvider( NULL ),
//	fDelegate( [[AlertViewDelegate alloc] init] ),
	fWebPopup( NULL ),
	fPopupControllerDelegate( [[PopupControllerDelegate alloc] init] )
{
//	[pView retain];

	UIScreen *screen = [UIScreen mainScreen];
	CGRect frame = [screen bounds]; // want fullscreen dimensions
	fActivityView = [[UIView alloc] initWithFrame:frame];
	fActivityView.backgroundColor = [UIColor blackColor];
	fActivityView.alpha = 0.7;
	[fActivityView setAutoresizingMask:UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight];

	UIActivityIndicatorView *indicator = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhiteLarge];
	[fActivityView addSubview:indicator];
	CGPoint center = { (CGFloat)(frame.origin.x + 0.5f*CGRectGetWidth( frame )), (CGFloat)(frame.origin.y + 0.5f*CGRectGetHeight( frame )) };
	indicator.center = center;
	[indicator setAutoresizingMask:
		UIViewAutoresizingFlexibleLeftMargin
		| UIViewAutoresizingFlexibleRightMargin
		| UIViewAutoresizingFlexibleTopMargin
		| UIViewAutoresizingFlexibleBottomMargin];
	[indicator release];

	if( NSClassFromString( @"UIAlertController" ) )
	{
		// IOS 8 and after

		// Always force activity view to be the top-most child of CoronaView
		fActivityView.layer.zPosition = MAXFLOAT;
		[view addSubview:fActivityView];

		// Prevent touches from bleeding through the activity view
		[fActivityView addGestureRecognizer:[[CoronaNullGestureRecognizer alloc] init]];
	}
	else
	{
		// Legacy: Pre-IOS8
		AppDelegate *delegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
		UIWindow *window = delegate.window;
		[window insertSubview:fActivityView aboveSubview:delegate.view];
	}

	fActivityView.hidden = YES;
}

IPhonePlatform::~IPhonePlatform()
{
	[fPopupControllerDelegate release];
	Rtt_DELETE( fWebPopup );
	[fActivityView release];
//	[fDelegate release];
//	[fPeoplePickerDelegate release];
	Rtt_DELETE( fInAppStoreProvider );
	Rtt_DELETE( fVideoPlayer );
//	[fView release];
}

// =====================================================================

#if 0

MPlatformDevice&
IPhonePlatform::GetDevice() const
{
	return const_cast< IPhoneDevice& >( fDevice );
}

PlatformSurface*
IPhonePlatform::CreateScreenSurface() const
{
	return Rtt_NEW( fAllocator, IPhoneScreenSurface( fView ) );
}
PlatformTimer*
IPhonePlatform::CreateTimerWithCallback( MCallback& callback ) const
{
	AppDelegate* delegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
	Rtt_GLKViewController* viewController = (Rtt_GLKViewController*)delegate.viewController;
	
	return Rtt_NEW( fAllocator, IPhoneTimer( callback, viewController ) );
}

// TODO: Remove when we remove support for iOS 3.x
PlatformBitmap*
IPhonePlatform::CreateBitmap( const char *path, bool convertToGrayscale ) const
{
	PlatformBitmap *result = NULL;

	// UILocalNotification only exists in iOS 4.x and later
	Class myClass = NSClassFromString( @"UILocalNotification" );
	if ( ! myClass )
	{
		// Fallback for iOS 3.x
		if ( path )
		{
			UIImage *image = [UIImage imageWithContentsOfFile:[NSString stringWithUTF8String:path]];
			if ( Rtt_VERIFY( image ) )
			{
				result = Rtt_NEW( & GetAllocator(), IPhoneFileBitmap( image, convertToGrayscale ) );
			}
		}
	}
	else
	{
		result = Super::CreateBitmap( path, convertToGrayscale );
	}

	return result;
}
int
IPhonePlatform::GetStatusBarHeight() const
{
    return Super::GetStatusBarHeight();
}

int
IPhonePlatform::GetTopStatusBarHeightPixels() const
{
    int result = Super::GetTopStatusBarHeightPixels();
    CGFloat scale_factor = [[UIScreen mainScreen] scale];
    UIInterfaceOrientation currentOrienation = [UIApplication sharedApplication].statusBarOrientation;
    if ( UIInterfaceOrientationPortrait == currentOrienation ||
        UIDeviceOrientationPortraitUpsideDown == currentOrienation)
    {
        result = [UIApplication sharedApplication].statusBarFrame.size.height * scale_factor;
    }
    else
    {
        result = [UIApplication sharedApplication].statusBarFrame.size.width * scale_factor;
    }
    
    return result;
}
    
int
IPhonePlatform::GetBottomStatusBarHeightPixels() const
{
    return Super::GetBottomStatusBarHeightPixels();
}
    
static Rtt_INLINE
double DegreesToRadians( double degrees )
{
	return degrees * M_PI/180;
}
    
bool
IPhonePlatform::SaveImageToPhotoLibrary(const char* filePath) const
{
    UIImage *image = [UIImage imageWithContentsOfFile:[NSString stringWithUTF8String:filePath]];
    if (image)
    {
        UIImageWriteToSavedPhotosAlbum( image, nil, nil, nil );
    }
    
    return true;
}
    
bool
IPhonePlatform::SaveBitmap( PlatformBitmap* bitmap, NSString* filePath ) const
{
	Rtt_ASSERT( bitmap );
	PlatformBitmap::Orientation orientation = bitmap->GetOrientation();
	bool isSideways = PlatformBitmap::kLeft == orientation || PlatformBitmap::kRight == orientation;

	const void* buffer = bitmap->GetBits( & GetAllocator() );
	size_t w = bitmap->Width();
	size_t h = bitmap->Height();
	size_t wDst = w;
	size_t hDst = h;
	if ( isSideways )
	{
		Swap( wDst, hDst );
	}

	size_t bytesPerPixel = PlatformBitmap::BytesPerPixel( bitmap->GetFormat() );
//	size_t bitsPerPixel = (bytesPerPixel << 3);
	size_t bytesPerRow = w*bytesPerPixel;
	NSInteger numBytes = h*bytesPerRow;
//	const size_t kBitsPerComponent = 8;

#if 0
	NSData* data = [NSData dataWithBytesNoCopy:& buffer length:numBytes freeWhenDone:NO];
	UIImage* image = [UIImage imageWithData:data];
	UIImageWriteToSavedPhotosAlbum( image, nil, nil, nil );
#else



	CGBitmapInfo srcBitmapInfo = CGBitmapInfo(kCGBitmapByteOrderDefault);
	CGBitmapInfo dstBitmapInfo = CGBitmapInfo(kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Big);
    bool enablePngAlphaSave = false;
    NSString *lowercase = [filePath lowercaseString];
    if ( [lowercase hasSuffix:@"png"] )
    {
        enablePngAlphaSave = true;
		srcBitmapInfo = CGBitmapInfo(kCGBitmapByteOrderDefault | kCGImageAlphaLast);
        dstBitmapInfo = kCGImageAlphaPremultipliedLast;

    }

	CGDataProviderRef dataProvider = CGDataProviderCreateWithData( NULL, buffer, numBytes, NULL );
	CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
	CGImageRef imageRef = CGImageCreate(w, h, 8, 32, w*bytesPerPixel,
                                        colorspace, srcBitmapInfo, dataProvider,
                                        NULL, true, kCGRenderingIntentDefault);
    

	Rtt_ASSERT( w == CGImageGetWidth( imageRef ) );
	Rtt_ASSERT( h == CGImageGetHeight( imageRef ) );
    
    
	//void* pixels = calloc( bytesPerRow, h );
	CGContextRef context = CGBitmapContextCreate(NULL, wDst, hDst, 8, wDst*bytesPerPixel, colorspace, dstBitmapInfo);

	// On iPhone, when the image is sideways, we have to rotate the bits b/c when 
	// we read them in using glReadPixels, the window buffer is physically oriented 
	// as upright, so glReadPixels returns them assuming the buffer is physically
	// oriented upright, rather than sideways.
	if ( isSideways )
	{
		S32 angle = - ( bitmap->DegreesToUprightBits() );
		CGFloat dx = (CGFloat)wDst;
		CGFloat dy = (CGFloat)hDst;
		if ( 90 == angle )
		{
			dy = 0.f;
		}
		if ( -90 == angle )
		{
			dx = 0.f;
		}

		CGContextTranslateCTM( context, dx, dy );
		CGContextRotateCTM( context, DegreesToRadians( angle ) );
	}
	else if ( PlatformBitmap::kDown == orientation )
	{
		CGContextTranslateCTM( context, wDst, hDst );
		CGContextRotateCTM( context, DegreesToRadians( 180 ) );
	}

	CGContextDrawImage( context, CGRectMake( 0.0, 0.0, w, h ), imageRef );
	CGImageRef flippedImageRef = CGBitmapContextCreateImage(context);
	UIImage* image = [[UIImage alloc] initWithCGImage:flippedImageRef];

	if ( ! filePath )
	{
		UIImageWriteToSavedPhotosAlbum( image, nil, nil, nil );
	}
	else
	{
		NSData *imageData = nil;
        if (enablePngAlphaSave)
		{
            imageData = UIImagePNGRepresentation( image );
		}
        else
		{
            imageData = UIImageJPEGRepresentation( image, 1.0 );
		}
        
        [imageData writeToFile:filePath atomically:YES];
	}

	[image release];
	CGImageRelease( flippedImageRef );
	CGColorSpaceRelease( colorspace );
	CGContextRelease( context );
	//free( pixels );


//	UIImage* image = [[UIImage alloc] initWithCGImage:imageRef];
//	UIImageWriteToSavedPhotosAlbum( image, nil, nil, nil );
//	[image release];

	CGImageRelease( imageRef );
	CGDataProviderRelease( dataProvider );
#endif

	bitmap->FreeBits();

	return true;
}

bool
IPhonePlatform::SaveBitmap( PlatformBitmap* bitmap, const char* filePath ) const
{
	return SaveBitmap( bitmap, [NSString stringWithUTF8String:filePath] );
}

bool
IPhonePlatform::AddBitmapToPhotoLibrary( PlatformBitmap* bitmap ) const
{
	return SaveBitmap( bitmap, (NSString*)NULL );
}

bool
IPhonePlatform::OpenURL( const char* url ) const
{
	bool result = false;

	if ( url )
	{
		UIApplication *sharedApplication = [UIApplication sharedApplication];
		NSURL* urlPlatform = [NSURL URLWithString:[NSString stringWithUTF8String:url]];
		if ( [sharedApplication canOpenURL:urlPlatform] )
		{
			result = true; // We set to true due the fact we don't know if web link opened
            [[UIApplication sharedApplication] openURL:urlPlatform options:@{} completionHandler:nil];
		}
	}

	return result;
}

PlatformAudioSessionManager * 
IPhonePlatform::GetAudioSessionManager( const ResourceHandle<lua_State> & handle ) const
{
	return IPhoneAudioSessionManager::GetInstance( handle );
}

#endif // 0

// =====================================================================
	
PlatformVideoPlayer*
IPhonePlatform::GetVideoPlayer( const ResourceHandle<lua_State> & handle ) const
{
	if ( ! fVideoPlayer )
	{
		fVideoPlayer = Rtt_NEW( fAllocator, IPhoneVideoPlayer( handle ) );
	}

	return fVideoPlayer;
}

PlatformImageProvider*
IPhonePlatform::GetImageProvider( const ResourceHandle<lua_State> & handle ) const
{
	if ( ! fImageProvider )
	{
		fImageProvider = Rtt_NEW( fAllocator, IPhoneImageProvider( handle ) );
	}

	return fImageProvider;	
}

PlatformVideoProvider*
IPhonePlatform::GetVideoProvider( const ResourceHandle<lua_State> & handle ) const
{
	if ( ! fVideoProvider )
	{
		fVideoProvider = Rtt_NEW( fAllocator, IPhoneVideoProvider( handle ) );
	}
	
	return fVideoProvider;
}

PlatformStoreProvider*
IPhonePlatform::GetStoreProvider( const ResourceHandle<lua_State>& handle ) const
{
	if (!fInAppStoreProvider)
	{
		fInAppStoreProvider = Rtt_NEW( fAllocator, AppleStoreProvider( handle ) );
	}
	return fInAppStoreProvider;
}

void
IPhonePlatform::SetStatusBarMode( MPlatform::StatusBarMode newValue ) const
{
	UIApplication* application = [UIApplication sharedApplication];

    AppDelegate* delegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
    AppViewController* viewController = (AppViewController*)delegate.viewController;
    
	bool isHidden = application.statusBarHidden;
	if ( MPlatform::kHiddenStatusBar == newValue )
	{
		if ( ! isHidden )
		{
			[application setStatusBarHidden:YES withAnimation:UIStatusBarAnimationSlide];
            [viewController setPrefersStatusBarhidden:true];
		}
	}
	else
	{
		if ( isHidden )
		{
			[application setStatusBarHidden:NO withAnimation:UIStatusBarAnimationSlide];
            [viewController setPrefersStatusBarhidden:false];
		}

		switch( newValue )
		{
			case MPlatform::kDefaultStatusBar:
			case MPlatform::kLightTransparentStatusBar:
				[application setStatusBarStyle:UIStatusBarStyleDefault animated:YES];
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 70000
				 // This is needed because iOS 7 defaults to the dark style while Corona only has default and dark.  Making light the default lets the user choose without having to change the api.
				[viewController setPreferredStatusBarStyle:UIStatusBarStyleLightContent];
#endif
				break;
			case MPlatform::kTranslucentStatusBar:
				[application setStatusBarStyle:UIStatusBarStyleBlackTranslucent animated:YES];
				[viewController setPreferredStatusBarStyle:UIStatusBarStyleBlackTranslucent];
				break;
			case MPlatform::kDarkStatusBar:
			case MPlatform::kDarkTransparentStatusBar:
				[application setStatusBarStyle:UIStatusBarStyleBlackOpaque animated:YES];
				[viewController setPreferredStatusBarStyle:UIStatusBarStyleDefault];
				break;
			default:
				Rtt_ASSERT_NOT_REACHED();
				break;
		}
	}
	if ([viewController respondsToSelector:@selector(setNeedsStatusBarAppearanceUpdate)]) {
		[viewController setNeedsStatusBarAppearanceUpdate];
	}
    
}

MPlatform::StatusBarMode
IPhonePlatform::GetStatusBarMode() const
{
	MPlatform::StatusBarMode result = MPlatform::kHiddenStatusBar;

	UIApplication* application = [UIApplication sharedApplication];

	bool isHidden = application.statusBarHidden;
	if ( ! isHidden )
	{
		UIStatusBarStyle style = application.statusBarStyle;
		switch( style )
		{
			case UIStatusBarStyleDefault:
				result = MPlatform::kDefaultStatusBar;
				break;
			case UIStatusBarStyleBlackTranslucent:
				result = MPlatform::kTranslucentStatusBar;
				break;
			case UIStatusBarStyleBlackOpaque:
				result = MPlatform::kDarkStatusBar;
				break;
			default:
				Rtt_ASSERT_NOT_REACHED();
				break;
		}
	}

	return result;
}

// =====================================================================

#if 0 

void
IPhonePlatform::SetIdleTimer( bool enabled ) const
{
	[UIApplication sharedApplication].idleTimerDisabled = ! enabled;
}
	
bool
IPhonePlatform::GetIdleTimer() const
{
	return (bool)( ! [UIApplication sharedApplication].idleTimerDisabled);
}

#endif // 0

// =====================================================================

//NativeAlertRef
//IPhonePlatform::ShowNativeAlert(
//	const char *title,
//	const char *msg,
//	const char **buttonLabels,
//	U32 numButtons,
//	LuaResource* resource ) const
//{
//	AppDelegate *appDelegate = (AppDelegate*)[UIApplication sharedApplication].delegate;
//	Rtt::TouchInhibitor inhibitor( (CoronaView*)appDelegate.view );
//
//	NSString *t = [[NSString alloc] initWithUTF8String:title];
//	NSString *m = [[NSString alloc] initWithUTF8String:msg];
//	CustomAlertView *alertView = [[CustomAlertView alloc]
//								initWithTitle:t
//								message:m
//								delegate:fDelegate
//								cancelButtonTitle:nil
//								otherButtonTitles:nil];
//	alertView.fResource = resource;
//	[fDelegate setObject:alertView forKey:alertView];
//
//	for ( U32 i = 0; i < numButtons; i++ )
//	{
//		const char *label = buttonLabels[i]; Rtt_ASSERT( label );
//		NSString *l = [[NSString alloc] initWithUTF8String:label];
//		(void)Rtt_VERIFY( (U32)[alertView addButtonWithTitle:l] == i );
//		[l release];
//	}
//
//	[alertView show];
//
//	[m release];
//	[t release];
//
//	return alertView;
//}
//
//void
//IPhonePlatform::CancelNativeAlert( NativeAlertRef alert, S32 index ) const
//{
//	CustomAlertView *alertView = [fDelegate objectForKey:alert];
//	[alertView dismissWithClickedButtonIndex:index animated:true];
//}

void
IPhonePlatform::SetActivityIndicator( bool visible ) const
{
	UIActivityIndicatorView *indicator = [[fActivityView subviews] objectAtIndex:0];

	Rtt_ASSERT( fActivityView.hidden == visible );

	if ( visible )
	{
		// Always bring activity indicator to the front since we want to give user feedback
		[[fActivityView superview] bringSubviewToFront:fActivityView];
		[indicator startAnimating];
	}
	else
	{
		[indicator stopAnimating];
	}

	fActivityView.hidden = ! visible;
}

PlatformWebPopup*
IPhonePlatform::GetWebPopup() const
{
	if ( ! fWebPopup )
	{
		fWebPopup = Rtt_NEW( fAllocator, IPhoneWebPopup );
	}

	return fWebPopup;
}

// Returns an array of NSStrings. The object at index t is either a string
// or a (table) array of strings.
static NSArray*
ArrayWithStrings( lua_State *L, int t )
{
	Rtt_ASSERT( t > 0 );

	NSArray *result = nil;

	if ( LUA_TTABLE == lua_type( L, t ) )
	{
		int iMax = (int)lua_objlen( L, t );
		if ( iMax > 0 )
		{
			NSMutableArray *array = [NSMutableArray arrayWithCapacity:iMax];

			for ( int i = 1; i <= iMax; i++ )
			{
				lua_rawgeti( L, t, i );
				if ( LUA_TSTRING == lua_type( L, -1 ) )
				{
					const char *str = lua_tostring( L, -1 );
					[array addObject:[NSString stringWithUTF8String:str]];
				}
				lua_pop( L, 1 );
			}

			result = array;
		}
	}
	else if ( LUA_TSTRING == lua_type( L, t ) )
	{
		const char *str = lua_tostring( L, t );
		result = [NSArray arrayWithObject:[NSString stringWithUTF8String:str]];
	}

	return result;
}

static void
AddAttachment( MFMailComposeViewController *controller, const char *path, LuaLibSystem::FileType fileType )
{
	if ( path )
	{
		NSString *p = [[NSString alloc] initWithUTF8String:path];
		NSString *filename = [p lastPathComponent];
		NSString *extension = [filename pathExtension];

		NSString *mimeType = ( LuaLibSystem::kImageFileType == fileType
			? [NSString stringWithFormat:@"image/%@", extension]
			: @"content/unknown");

		NSData *data = [NSData dataWithContentsOfFile:p];
		[controller addAttachmentData:data mimeType:mimeType fileName:filename];
	}
}

static bool
InitializeController( lua_State *L, int index, MFMailComposeViewController *controller )
{
	bool result = true;

	if ( index > 0 )
	{
		lua_getfield( L, index, "to" );
		NSArray *to = ArrayWithStrings( L, lua_gettop( L ) );
		if ( to )
		{
			[controller setToRecipients:to];
		}	
		lua_pop( L, 1 );

		lua_getfield( L, index, "cc" );
		NSArray *cc = ArrayWithStrings( L, lua_gettop( L ) );
		if ( cc )
		{
			[controller setCcRecipients:cc];
		}	
		lua_pop( L, 1 );

		lua_getfield( L, index, "bcc" );
		NSArray *bcc = ArrayWithStrings( L, lua_gettop( L ) );
		if ( bcc )
		{
			[controller setBccRecipients:bcc];
		}	
		lua_pop( L, 1 );

		lua_getfield( L, index, "body" );
		if ( lua_type( L, -1 ) == LUA_TSTRING )
		{
			const char *body = lua_tostring( L, -1 );

			lua_getfield( L, index, "isBodyHtml" );
			bool isBodyHtml = lua_toboolean( L, -1 );
			lua_pop( L, 1 );
			
			[controller setMessageBody:[NSString stringWithUTF8String:body] isHTML:isBodyHtml];
		}	
		lua_pop( L, 1 );

		lua_getfield( L, index, "subject" );
		if ( lua_type( L, -1 ) == LUA_TSTRING )
		{
			const char *body = lua_tostring( L, -1 );

			[controller setSubject:[NSString stringWithUTF8String:body]];
		}	
		lua_pop( L, 1 );

		// TODO: suppport multiple attachments (i.e. array of these 'path' tables)
		LuaLibSystem::FileType fileType;
		lua_getfield( L, index, "attachment" );
		int numAttachments = (int)lua_objlen( L, -1 );
		if ( numAttachments > 0 )
		{
			// table is an array of 'path' tables
			for ( int i = 1; i <= numAttachments; i++ )
			{
				lua_rawgeti( L, -1, i );
				int numResults = LuaLibSystem::PathForTable( L, -1, fileType );
				if ( numResults > 0 )
				{
					const char *path = lua_tostring( L, -1 );
					AddAttachment( controller, path, fileType );
					lua_pop( L, numResults );
				}
				lua_pop( L, 1 );
			}
		}
		else
		{
			// table itself is a 'path' table
			int numResults = LuaLibSystem::PathForTable( L, -1, fileType );
			if ( numResults > 0 )
			{
				const char *path = lua_tostring( L, -1 );
				AddAttachment( controller, path, fileType );
				lua_pop( L, numResults );
			}
		}
		lua_pop( L, 1 );
	}

	return result;
}

static bool
InitializeController( lua_State *L, int index, MFMessageComposeViewController *controller )
{
	bool result = true;

	if ( index > 0 )
	{
		lua_getfield( L, index, "to" );
		NSArray *to = ArrayWithStrings( L, lua_gettop( L ) );
		if ( to )
		{
			controller.recipients = to;
		}	
		lua_pop( L, 1 );

		lua_getfield( L, index, "body" );
		if ( lua_type( L, -1 ) == LUA_TSTRING )
		{
			const char *body = lua_tostring( L, -1 );

			controller.body = [NSString stringWithUTF8String:body];
		}
		lua_pop( L, 1 );
	}

	return result;
}

bool
IPhonePlatform::CanShowPopup( const char *name ) const
{
	bool result =
		( Rtt_StringCompareNoCase( name, "mail" ) == 0 && [MFMailComposeViewController canSendMail] )
		|| ( Rtt_StringCompareNoCase( name, "sms" ) == 0 && [MFMessageComposeViewController canSendText] )
		|| ( Rtt_StringCompareNoCase( name, "rateApp" ) == 0 )
		|| ( Rtt_StringCompareNoCase( name, "appStore" ) == 0 );

	return result;
}

bool
IPhonePlatform::ShowPopup( lua_State *L, const char *name, int optionsIndex ) const
{
	bool result = false;

	AppDelegate* delegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
	UIViewController* viewController = delegate.viewController;
	if ( viewController.presentedViewController )
	{
		Rtt_ERROR( ( "ERROR: There is already a native modal interface being displayed. The '%s' popup will not be shown.\n", name ? name : "" ) );
	}
	else if ( Rtt_StringCompareNoCase( name, "mail" ) == 0 )
	{
		if ( [MFMailComposeViewController canSendMail] )
		{
			MFMailComposeViewController *controller = [[MFMailComposeViewController alloc] init];
			controller.mailComposeDelegate = fPopupControllerDelegate;

			result = InitializeController( L, optionsIndex, controller );
			if ( result )
			{
				[viewController presentViewController:controller animated:YES completion:nil];
			}

			[controller release];
		}
		else
		{
			Rtt_ERROR( ( "ERROR: This device cannot send mail. The 'mail' popup will not be shown.\n" ) );
		}
	}
	else if ( Rtt_StringCompareNoCase( name, "sms" ) == 0 )
	{
		if ( [MFMessageComposeViewController canSendText] )
		{
			MFMessageComposeViewController *controller = [[MFMessageComposeViewController alloc] init];
			controller.messageComposeDelegate = fPopupControllerDelegate;

			result = InitializeController( L, optionsIndex, controller );
			if ( result )
			{
				// casenum:12085 The controller is messing up hidden status bars. This must be fetched before we present the controller.
				BOOL isstatusbarhidden = [[UIApplication sharedApplication] isStatusBarHidden];
				
				[viewController presentViewController:controller animated:YES completion:nil];
				
				// casenum:12085 The controller is messing up hidden status bars. Force the status bar to hidden to prevent it from reappearing.
				if ( YES == isstatusbarhidden )
				{
					[[UIApplication sharedApplication] setStatusBarHidden:YES withAnimation:UIStatusBarAnimationNone];
				}

			}

			[controller release];
		}
		else
		{
			Rtt_ERROR( ( "ERROR: This device cannot send SMS. The 'sms' popup will not be shown.\n" ) );
		}
	}
	else if ( !Rtt_StringCompareNoCase( name, "rateApp" ) || !Rtt_StringCompareNoCase( name, "appStore" ) )
	{
		bool showWriteReviewScreen = !Rtt_StringCompareNoCase( name, "rateApp" );
		const char *appStringId = NULL;
		if ( lua_istable( L, optionsIndex ) )
		{
			lua_getfield( L, optionsIndex, "iOSAppId" );
			if ( lua_type( L, -1 ) == LUA_TSTRING )
			{
				appStringId = lua_tostring( L, -1 );
			}
			lua_pop( L, 1 );
		}
		if ( appStringId )
		{
			char url[256];
			if (showWriteReviewScreen && ([[[UIDevice currentDevice] systemVersion] doubleValue] < 6.0))
			{
				snprintf( url, sizeof(url), "itms-apps://ax.itunes.apple.com/WebObjects/MZStore.woa/wa/viewContentsUserReviews?type=Purple+Software&id=%s", appStringId );
			}
			else
			{
				snprintf( url, sizeof(url), "itms-apps://itunes.apple.com/%s/app/id%s",
						[[[NSLocale currentLocale] objectForKey: NSLocaleCountryCode] UTF8String], appStringId );
			}
			result = OpenURL( url );
		}
		else
		{
			Rtt_ERROR( ( "ERROR: native.showPopup('rateApp') requires the iOS app ID.\n" ) );
		}
	}

	return result;
}

bool
IPhonePlatform::HidePopup( const char *name ) const
{
	bool result = false;

	Rtt_ASSERT_NOT_IMPLEMENTED();

	return result;
}

PlatformDisplayObject*
IPhonePlatform::CreateNativeMapView( const Rect& bounds ) const
{
	return Rtt_NEW( & GetAllocator(), IPhoneMapViewObject( bounds ) );
}

PlatformDisplayObject*
IPhonePlatform::CreateNativeWebView( const Rect& bounds ) const
{
	return Rtt_NEW( & GetAllocator(), IPhoneWebViewObject( bounds ) );
}

PlatformDisplayObject*
IPhonePlatform::CreateNativeVideo( const Rect& bounds ) const
{
	return Rtt_NEW( & GetAllocator(), IPhoneVideoObject( bounds ) );
}

void
IPhonePlatform::SetTapDelay( Rtt_Real delay ) const
{
	AppDelegate *delegate = (AppDelegate*)[UIApplication sharedApplication].delegate;
	((CoronaView*)delegate.view).tapDelay = delay;
}

Rtt_Real
IPhonePlatform::GetTapDelay() const
{
	AppDelegate *delegate = (AppDelegate*)[UIApplication sharedApplication].delegate;
	return ((CoronaView*)delegate.view).tapDelay;
}

void*
IPhonePlatform::CreateAndScheduleNotification( lua_State *L, int index ) const
{
	/*
	 local variable = require("plugin.notifications")
	 variable.scheduleNotification(...);
	 
	This function has been deprecated improvements should go into the plugin.
	 */
	Rtt_LUA_STACK_GUARD( L );
	
	void *notificationId = NULL;
	int absoluateIndex = Lua::Normalize( L, index );
	int numArgs = lua_gettop( L ) - (absoluateIndex - 1);

	// local variable = require("plugin.notifications")
	lua_getglobal( L, "require" );
	lua_pushstring( L, "plugin.notifications" );
	int result = lua_pcall( L, 1, 1, 0 );

	// requiring the plugin suceeded, first item on the stack should be
	if ( 0 == result )
	{
		lua_getfield( L, -1, "scheduleNotification" );
		
		// setting up arguments for pcall
		for ( int i = 0; i < numArgs; i++ )
		{
			lua_pushvalue( L, absoluateIndex + i );
		}
		
		result = lua_pcall( L, numArgs, 1, 0 );
		if ( 0 == result )
		{
			notificationId = Lua::CheckUserdata( L, -1, "notification" );
		}
		
		// pop the notificationId object off the stack
		// or the error code from the pcall
		lua_pop( L, 1 );
	}
	else
	{
		CoronaLuaError(L, "system.scheduleNotification: Notifications plugin not found, you need to include the plugin in your build.settings file.  This function has been deprecated.");
	}
	
	// pop the library object put on the stack by require("plugin.notificatoins")
	// or the error code from the pcall
	lua_pop( L, 1 );
	
	return notificationId;
}

void
IPhonePlatform::ReleaseNotification( void *notificationId ) const
{
	UILocalNotification *notification = (UILocalNotification*)notificationId;
	[notification release];
}

void
IPhonePlatform::CancelNotification( void *notificationId ) const
{
	/*
	 This function has been deprecated improvements should go into the plugin.
	 */
	// Duplicate from the notifications plugin
	UIApplication *application = [UIApplication sharedApplication];
	
	if ( notificationId )
	{
		UILocalNotification *notification = (UILocalNotification*)notificationId;
		
		[application cancelLocalNotification:notification];
	}
	else
	{
		[application cancelAllLocalNotifications];
		application.applicationIconBadgeNumber = 0;
	}
}

void
IPhonePlatform::RuntimeErrorNotification( const char *errorType, const char *message, const char *stacktrace ) const
{
	// In the delegate we catch the press of the button on the non-modal dialog and take the app down
	AppDelegate *delegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];

	// we need to suspend the app so Lua doesn't keep churning out errors
	[delegate suspend];
	
	// This alert is designed to be similar to the Android version (we remove empty file and linenumber info if it's there)
	UIAlertView *alert = [[UIAlertView alloc] initWithTitle:[NSString stringWithUTF8String:errorType]
												message:[[NSString stringWithUTF8String:message] stringByReplacingOccurrencesOfString:@"?:0: " withString:@""]
												   delegate:delegate
										  cancelButtonTitle:@"OK"
										  otherButtonTitles:nil];

	[alert show];
	[alert release];
}


void
IPhonePlatform::SetNativeProperty( lua_State *L, const char *key, int valueIndex ) const
{
	AppDelegate *delegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
	AppViewController* viewController = (AppViewController*)delegate.viewController;

	NSString *k = [NSString stringWithUTF8String:key];

	if ( [k isEqualToString:@"prefersHomeIndicatorAutoHidden"] )
	{
		viewController.prefersHomeIndicatorAutoHidden = lua_toboolean( L, valueIndex );
		if (@available(iOS 11.0, *))
		{
			[viewController setNeedsUpdateOfHomeIndicatorAutoHidden];
		}
	}
	else if ([k isEqualToString:@"preferredScreenEdgesDeferringSystemGestures"])
	{
		viewController.preferredScreenEdgesDeferringSystemGestures = lua_toboolean( L, valueIndex )?UIRectEdgeAll:UIRectEdgeNone;
		if (@available(iOS 11.0, *))
		{
			[viewController setNeedsUpdateOfScreenEdgesDeferringSystemGestures];
		}
	}
	else
	{
		Super::SetNativeProperty(L, key, valueIndex);
	}
}

int
IPhonePlatform::PushNativeProperty( lua_State *L, const char *key ) const
{
	int result = 1;

	AppDelegate *delegate = (AppDelegate*)[[UIApplication sharedApplication] delegate];
	AppViewController* viewController = (AppViewController*)delegate.viewController;

	NSString *k = [NSString stringWithUTF8String:key];

	if ( [k isEqualToString:@"prefersHomeIndicatorAutoHidden"] )
	{
		lua_pushboolean(L, viewController.prefersHomeIndicatorAutoHidden);
	}
	else if ([k isEqualToString:@"preferredScreenEdgesDeferringSystemGestures"])
	{
		lua_pushboolean(L, viewController.preferredScreenEdgesDeferringSystemGestures != UIRectEdgeNone);
	}
	else
	{
		result = Super::PushNativeProperty(L, key);
	}

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

