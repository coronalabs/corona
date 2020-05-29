//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#include "Rtt_IPhoneInMobiNetwork.h"

#import "AppDelegate.h"
#import <Foundation/NSString.h>
#import "IMAdView.h"
#import "IMAdDelegate.h"

#import "Rtt_IPhoneDisplayObject.h"

#include "Rtt_LuaAux.h"
#include "Rtt_LaunchPad.h"
#include "Rtt_Runtime.h"
//#include "Rtt_LuaResource.h"
#include "Rtt_Event.h"

// ----------------------------------------------------------------------------

@interface InMobiDelegate : NSObject < IMAdDelegate >
{
	Rtt::IPhoneInMobiNetwork *fOwner;
}

@end


@implementation InMobiDelegate

- (id)initWithOwner:(Rtt::IPhoneInMobiNetwork*)owner
{
	self = [super init];

	if ( self )
	{
		fOwner = owner;
	}

	return self;
}

- (void)adViewDidFinishRequest:(IMAdView *)adView
{
	using namespace Rtt;

	AdsRequestEvent event(
		PlatformAdNetwork::StringForProvider( PlatformAdNetwork::kInMobiProvider ), false );
	fOwner->DispatchEvent( event );
}

- (void)adView:(IMAdView *)view didFailRequestWithError:(IMAdError *)error
{
	using namespace Rtt;

	AdsRequestEvent event(
		PlatformAdNetwork::StringForProvider( PlatformAdNetwork::kInMobiProvider ), true );
	fOwner->DispatchEvent( event );
}

- (void)adViewWillPresentScreen:(IMAdView *)adView
{
	AppDelegate *delegate = (AppDelegate*)[UIApplication sharedApplication].delegate;
	delegate.runtime->Suspend();
}

- (void)adViewWillDismissScreen:(IMAdView *)adView
{
}

- (void)adViewDidDismissScreen:(IMAdView *)adView
{
	AppDelegate *delegate = (AppDelegate*)[UIApplication sharedApplication].delegate;
	delegate.runtime->Resume();
}

- (void)adViewWillLeaveApplication:(IMAdView *)adView
{
}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static int
AdUnitForString( const char *str, Rtt_Real& rWidth, Rtt_Real& rHeight )
{
	int result = IM_UNIT_320x48;
	rWidth = 320;
	rHeight = 48;

	if ( str )
	{
		if ( 0 == Rtt_StringCompareNoCase( "banner320x48", str ) )
		{
			result = IM_UNIT_320x48;
		}
		else if ( 0 == Rtt_StringCompareNoCase( "banner300x250", str ) )
		{
			result = IM_UNIT_300x250;
			rWidth = 300;
			rHeight = 250;
		}
		else if ( 0 == Rtt_StringCompareNoCase( "banner728x90", str ) )
		{
			// iPad only
			result = IM_UNIT_728x90;
			rWidth = 728;
			rHeight = 90;
		}
		else if ( 0 == Rtt_StringCompareNoCase( "banner468x60", str ) )
		{
			// iPad only
			result = IM_UNIT_468x60;
			rWidth = 468;
			rHeight = 60;
		}
		else if ( 0 == Rtt_StringCompareNoCase( "banner120x600", str ) )
		{
			// iPad only
			result = IM_UNIT_120x600;
			rWidth = 120;
			rHeight = 600;
		}
		else if ( 0 == Rtt_StringCompareNoCase( "banner320x50", str ) )
		{
			result = IM_UNIT_320x50;
			rWidth = 320;
			rHeight = 50;
		}
	}

	return result;
}

IPhoneInMobiNetwork::IPhoneInMobiNetwork()
:	Super( kInMobiProvider ),
	fDelegate( nil ),
	fAd( nil ),
	fAppId( nil )
{
}

IPhoneInMobiNetwork::~IPhoneInMobiNetwork()
{
	Hide();
	[fAppId release];
	[fDelegate release];
}

bool
IPhoneInMobiNetwork::Init( const char *appId, LuaResource *listener )
{
	bool result = false;

	if ( appId )
	{
		Super::SetResource( listener );

		fDelegate = [[InMobiDelegate alloc] initWithOwner:this];
		fAppId = [[NSString alloc] initWithUTF8String:appId];

		result = true;
	}

	return result;
}

bool
IPhoneInMobiNetwork::Show( const char *adUnitType, lua_State *L, int index )
{
	// If banner is already showing, hide the current ad
	if ( fAd )
	{
		Hide();
	}

	Rtt_Real w, h;
	int adUnit = AdUnitForString( adUnitType, w, h );

	AppDelegate *delegate = (AppDelegate*)[UIApplication sharedApplication].delegate;
	Runtime *runtime = delegate.runtime;

	// Defaults
	Rtt_Real x = Rtt_REAL_0;
	Rtt_Real y = Rtt_REAL_0;
	bool isTestMode = true;
	int interval = 60; // default interval

	if ( lua_istable( L, index ) )
	{
		lua_getfield( L, index, "x" );
		x = luaL_toreal( L, -1 );
		lua_pop( L, 1 );
		
		lua_getfield( L, index, "y" );
		y = luaL_toreal( L, -1 );
		lua_pop( L, 1 );

		lua_getfield( L, index, "testMode" );
		if ( lua_isboolean( L, -1 ) )
		{
			isTestMode = lua_toboolean( L, -1 );
		}
		lua_pop( L, 1 );

		lua_getfield( L, index, "interval" );
		if ( lua_isnumber( L, -1 ) )
		{
			interval = lua_tointeger( L, -1 );
			if ( ! lua_isnil( L, -1 ) )
			{
				if ( interval <= 0 )
				{
					interval = REFRESH_INTERVAL_OFF;
				}
				else
				{
					// 20 is minimum interval
					interval = Rtt::Max( 20, interval );
				}
			}
		}
		lua_pop( L, 1 );
	}

	Rtt_ASSERT( ! fAd );
	Rect bounds;
	bounds.xMin = x;
	bounds.yMin = y;
	bounds.xMax = x;
	bounds.yMax = y;

	// Mapping Corona rectangles to UIView rectangles. We only map the origin
	// b/c w,h are already in UIKit point sizes.
	// TODO: This isn't quite correct but captures the 80% usage cases.
	// Content scaling fortuitously works between iPhone and iPhone4,
	// but doesn't work on iPad (we punt on iPad and force the complexity on the developer)
	Real sx, sy;
	const RenderingStream& stream = runtime->Stream();
	PlatformDisplayObject::CalculateContentToScreenScale( stream, runtime->Surface(), sx, sy );
	bool shouldScale = ! Rtt_RealEqual( Rtt_REAL_1, sx ) || ! Rtt_RealEqual( Rtt_REAL_1, sy );
	if ( shouldScale )
	{
		sx = Rtt_RealDiv( Rtt_REAL_1, sx );
		sy = Rtt_RealDiv( Rtt_REAL_1, sy );
		PlatformDisplayObject::CalculateScreenBounds( stream, sx, sy, bounds );
	}

	// w, h are already in points, so we add that in after all the mapping
	bounds.xMax = bounds.xMin + w;
	bounds.yMax = bounds.yMin + h;

	CGRect r = CGRectMake( bounds.xMin, bounds.yMin, bounds.Width(), bounds.Height() );

	IMAdView *view = [[IMAdView alloc]
		initWithFrame:r
		imAppId:fAppId
		imAdUnit:adUnit
		rootViewController:delegate.viewController];
	fAd = view;

	view.delegate = fDelegate;
	view.refreshInterval = interval;
	[view setRefTag:@"CoronaSDK" forKey:@"ref-tag"];

	IMAdRequest *request = [IMAdRequest request];
	request.testMode = isTestMode;

	view.imAdRequest = request;

	[delegate.viewController.view addSubview:view];

	// If the refreshInterval property is "off", we need to explicitly request the initial ad
	if ( REFRESH_INTERVAL_OFF == interval )
	{
		[view loadIMAdRequest];
	}

	return true;
}

void
IPhoneInMobiNetwork::Hide()
{
	if ( fAd )
	{
		fAd.refreshInterval = REFRESH_INTERVAL_OFF;
		fAd.delegate = nil;
		[fAd release];
		[fAd removeFromSuperview];
		fAd = nil;
	}
}

const char*
IPhoneInMobiNetwork::GetTestAppId() const
{
	static const char kTestAppId[] = "4028cb962895efc50128fc99d4b7025b";
	return kTestAppId;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
