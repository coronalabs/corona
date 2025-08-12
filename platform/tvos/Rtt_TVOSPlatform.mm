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

#include "Rtt_TVOSPlatform.h"
#include "Rtt_IPhoneTimer.h"

#include "Rtt_IPhoneAudioSessionManager.h"
#include "Rtt_IPhoneFont.h"

//#include "Rtt_IPhoneImageProvider.h"
//#include "Rtt_IPhoneVideoProvider.h"
//#include "Rtt_IPhoneMapViewObject.h"
//#include "Rtt_IPhoneWebPopup.h"
//#include "Rtt_IPhoneWebViewObject.h"

#include "Rtt_AppleInAppStore.h"
#include "Rtt_IPhoneScreenSurface.h"
#include "Rtt_IPhoneTextBoxObject.h"
#include "Rtt_IPhoneTextFieldObject.h"
#include "Rtt_IPhoneVideoObject.h"
#include "Rtt_IPhoneVideoPlayer.h"

#include "Rtt_LuaLibNative.h"
#include "Rtt_LuaLibSystem.h"
#include "Rtt_LuaResource.h"

//#import "AppDelegate.h"
#import "CoronaViewPrivate.h"

#import <UIKit/UIApplication.h>
#import <UIKit/UIDevice.h>
#import <UIKit/UIGestureRecognizerSubclass.h>
//#import <MessageUI/MFMailComposeViewController.h>
//#import <MessageUI/MFMessageComposeViewController.h>

#include "CoronaLua.h"
// TODO: Remove when we remove support for iOS 3.x
#include "Rtt_AppleBitmap.h"

#include "Rtt_TouchInhibitor.h"

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

TVOSPlatform::TVOSPlatform( CoronaView *view )
:	Super( view ),
	fDevice( GetAllocator(), view ),
	fVideoPlayer( NULL ),
	fInAppStoreProvider( NULL )
{
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

	// Always force activity view to be the top-most child of CoronaView
	fActivityView.layer.zPosition = MAXFLOAT;
	[view addSubview:fActivityView];

	// Prevent touches from bleeding through the activity view
	[fActivityView addGestureRecognizer:[[CoronaNullGestureRecognizer alloc] init]];

	fActivityView.hidden = YES;
}

TVOSPlatform::~TVOSPlatform()
{
	[fActivityView release];
	Rtt_DELETE( fInAppStoreProvider );
	Rtt_DELETE( fVideoPlayer );
}

// =====================================================================


MPlatformDevice&
TVOSPlatform::GetDevice() const
{
	return const_cast< TVOSDevice& >( fDevice );
}

// =====================================================================
PlatformVideoPlayer*
TVOSPlatform::GetVideoPlayer( const ResourceHandle<lua_State> & handle ) const
{
	if ( ! fVideoPlayer )
	{
		fVideoPlayer = Rtt_NEW( fAllocator, IPhoneVideoPlayer( handle ) );
	}

	return fVideoPlayer;
}

PlatformStoreProvider*
TVOSPlatform::GetStoreProvider( const ResourceHandle<lua_State>& handle ) const
{
	if (!fInAppStoreProvider)
	{
		fInAppStoreProvider = Rtt_NEW( fAllocator, AppleStoreProvider( handle ) );
	}
	return fInAppStoreProvider;
}

void
TVOSPlatform::SetActivityIndicator( bool visible ) const
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

bool
TVOSPlatform::CanShowPopup( const char *name ) const
{
	bool result =
		( Rtt_StringCompareNoCase( name, "rateApp" ) == 0 )
		|| ( Rtt_StringCompareNoCase( name, "appStore" ) == 0 );

	return result;
}

bool
TVOSPlatform::ShowPopup( lua_State *L, const char *name, int optionsIndex ) const
{
	bool result = false;

	id<CoronaRuntime> runtime = (id<CoronaRuntime>)CoronaLuaGetContext( L );
	UIViewController* viewController = runtime.appViewController ;
	if ( viewController.presentedViewController )
	{
		Rtt_ERROR( ( "ERROR: There is already a native modal interface being displayed. The '%s' popup will not be shown.\n", name ? name : "" ) );
	}
	else if ( !Rtt_StringCompareNoCase( name, "rateApp" ) || !Rtt_StringCompareNoCase( name, "appStore" ) )
	{
		const char *appStringId = NULL;
		if ( lua_istable( L, optionsIndex ) )
		{
			lua_getfield( L, optionsIndex, "tvOSAppId" );
			if ( lua_type( L, -1 ) == LUA_TSTRING )
			{
				appStringId = lua_tostring( L, -1 );
			}
			lua_pop( L, 1 );

			if( NULL == appStringId )
			{
				lua_getfield( L, optionsIndex, "iOSAppId" );
				if ( lua_type( L, -1 ) == LUA_TSTRING )
				{
					appStringId = lua_tostring( L, -1 );
				}
				lua_pop( L, 1 );
			}
		}
		if ( appStringId )
		{
			char url[256];
			// undocumented woodoo. If doesn't work use "https://geo.itunes.apple.com/app/id%s" instead.
			snprintf( url, sizeof(url), "com.apple.TVAppStore://itunes.apple.com/app/id%s", appStringId );
			result = OpenURL( url );
		}
		else
		{
			Rtt_ERROR( ( "ERROR: native.showPopup('%s') requires the iOS or tvOS app Id.\n", name ) );
		}
	}

	return result;
}

bool
TVOSPlatform::HidePopup( const char *name ) const
{
	bool result = false;

	Rtt_ASSERT_NOT_IMPLEMENTED();

	return result;
}

void
TVOSPlatform::SetTapDelay( Rtt_Real delay ) const
{
	GetView().tapDelay = delay;
}

Rtt_Real
TVOSPlatform::GetTapDelay() const
{
	return GetView().tapDelay;
}

void
TVOSPlatform::RuntimeErrorNotification( const char *errorType, const char *message, const char *stacktrace ) const
{
    NSLog(@"Runtime Error: %s: %s\n%s", errorType, message, stacktrace);
}

static Rtt_INLINE
double DegreesToRadians( double degrees )
{
	return degrees * M_PI/180;
}

bool
TVOSPlatform::SaveBitmap( PlatformBitmap* bitmap, const char* cFilePath, float jpegQuality ) const
{
	NSString* filePath = [NSString stringWithUTF8String:cFilePath];
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
    if ( [lowercase hasSuffix:@"png"] ||  filePath == NULL)
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

	if ( filePath )
	{
		NSData *imageData = nil;
        if (enablePngAlphaSave)
		{
            imageData = UIImagePNGRepresentation( image );
		}
        else
		{
            imageData = UIImageJPEGRepresentation( image, jpegQuality );
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



// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

