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
//#include "Rtt_IPhoneVideoPlayer.h"

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
//	fVideoPlayer( NULL ),
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
//	Rtt_DELETE( fVideoPlayer );
}

// =====================================================================


MPlatformDevice&
TVOSPlatform::GetDevice() const
{
	return const_cast< TVOSDevice& >( fDevice );
}

// =====================================================================

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

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

