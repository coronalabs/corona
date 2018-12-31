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

#include "Rtt_IPhoneInneractiveNetwork.h"

#import "AppDelegate.h"
#import <Foundation/NSString.h>

#include "Core/Rtt_String.h"
#include "Rtt_LuaAux.h"
#include "Rtt_LaunchPad.h"
#include "Rtt_Event.h"

#import "InneractiveAd.h"

// ----------------------------------------------------------------------------

@interface InneractiveDelegate : NSObject
{
	Rtt::IPhoneInneractiveNetwork *fOwner;
}

@end


@implementation InneractiveDelegate

- (id)initWithOwner:(Rtt::IPhoneInneractiveNetwork*)owner
{
	self = [super init];

	if ( self )
	{
		fOwner = owner;
	}

	return self;
}

- (void)onReceiveAd:(id)sender
{
	using namespace Rtt;

	AdsRequestEvent event(
		PlatformAdNetwork::StringForProvider( PlatformAdNetwork::kInneractiveProvider ), false );
	fOwner->DispatchEvent( event );
}
- (void)onReceiveDefaultAd:(id)sender
{
	using namespace Rtt;

	AdsRequestEvent event(
		PlatformAdNetwork::StringForProvider( PlatformAdNetwork::kInneractiveProvider ), false );
	fOwner->DispatchEvent( event );
}
- (void)onFailedToReceiveAd:(id)sender
{
	using namespace Rtt;

	AdsRequestEvent event(
		PlatformAdNetwork::StringForProvider( PlatformAdNetwork::kInneractiveProvider ), true );
	fOwner->DispatchEvent( event );
}
- (void)onClickAd:(id)sender
{
// The ad has been clicked
}

@end

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

static NSString *
DistributionIdForType( IaAdType t )
{
	NSString *bannerOrFullscreenIPhone = @"642";
	NSString *textIPhone = @"632";
	NSString *bannerOrFullscreenIPad = @"947";
	NSString *textIPad = @"946";

	NSString *result = bannerOrFullscreenIPhone;

	bool isIPad = ( UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad );

	switch ( t )
	{
		case IaAdType_Banner:
		case IaAdType_FullScreen:
			result = ( isIPad ? bannerOrFullscreenIPad : bannerOrFullscreenIPhone );
			break;
		case IaAdType_Text:
			result = ( isIPad ? textIPad : textIPhone );
			break;
		default:
			break;
	}
	
	return result;
}

static IaAdType
AdUnitForString( const char *str )
{
	IaAdType result = IaAdType_Banner;

	if ( str )
	{
		if ( 0 == Rtt_StringCompareNoCase( "banner", str ) )
		{
			result = IaAdType_Banner;
		}
		else if ( 0 == Rtt_StringCompareNoCase( "text", str ) )
		{
			result = IaAdType_Text;
		}
		else if ( 0 == Rtt_StringCompareNoCase( "fullscreen", str ) )
		{
			result = IaAdType_FullScreen;
		}
	}

	return result;
}

IPhoneInneractiveNetwork::IPhoneInneractiveNetwork()
:	Super( kInneractiveProvider ),
	fDelegate( nil ),
	fAd( nil ),
	fAppId( nil )
{
}

IPhoneInneractiveNetwork::~IPhoneInneractiveNetwork()
{
	Hide();
	[fAppId release];
	[fDelegate release];
}

bool
IPhoneInneractiveNetwork::Init( const char *appId, LuaResource *listener )
{
	bool result = false;

	if ( appId )
	{
		Super::SetResource( listener );

		fDelegate = [[InneractiveDelegate alloc] initWithOwner:this];
		fAppId = [[NSString alloc] initWithUTF8String:appId];

		if ( ! appId )
		{
			NSLog( @"WARNING: No app id was supplied. A test app id will be used for ads served by inneractive." );
		}

		[[NSNotificationCenter defaultCenter] addObserver:fDelegate selector:@selector(onReceiveAd:) name:@"IaAdReceived" object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:fDelegate selector:@selector(onFailedToReceiveAd:) name:@"IaAdFailed" object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:fDelegate selector:@selector(onReceiveDefaultAd:) name:@"IaDefaultAdReceived" object:nil];
		[[NSNotificationCenter defaultCenter] addObserver:fDelegate selector:@selector(onClickAd:) name:@"IaAdClicked" object:nil];

		result = true;
	}

	return result;
}

bool
IPhoneInneractiveNetwork::Show( const char *adUnitType, lua_State *L, int index )
{
	// If banner is already showing, hide the current ad
	if ( fAd )
	{
		Hide();
	}

	CGRect fullScreenBounds = [UIScreen mainScreen].bounds;
	fAd = [[UIView alloc] initWithFrame:fullScreenBounds];

	IaAdType adUnit = AdUnitForString( adUnitType );
	NSString *distributionId = DistributionIdForType( adUnit );

	// Defaults
	Rtt_Real x = Rtt_REAL_0;
	Rtt_Real y = Rtt_REAL_0;
	bool isTestMode = true;
	int interval = 60; // default interval

	// TODO: Consolidate this with the InMobi implementation
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
					interval = 0;
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

	NSMutableDictionary *params = [[NSMutableDictionary alloc] init];
	[params setObject:distributionId forKey:[NSNumber numberWithInt:Key_Distribution_Id]];

	bool result = [InneractiveAd DisplayAd:fAppId withType:adUnit withRoot:fAd withReload:interval withParams:params];
	if ( result )
	{
		CGRect frame = fAd.frame;
		frame.origin.x = Rtt_RealToFloat( x );
		frame.origin.y = Rtt_RealToFloat( y );
		CGPoint center = CGPointMake( CGRectGetMidX( frame ), CGRectGetMidY( frame ) );
		fAd.center = center;

		AppDelegate *delegate = (AppDelegate*)[UIApplication sharedApplication].delegate;
		[delegate.viewController.view addSubview:fAd];
	}
	else
	{
		[[NSNotificationCenter defaultCenter] postNotificationName:@"onFailedToReceiveAd" object:nil];
	}

	return result;
}

void
IPhoneInneractiveNetwork::Hide()
{
	if ( fAd )
	{
		[fAd release];
		[fAd removeFromSuperview];
		fAd = nil;
	}
}

const char*
IPhoneInneractiveNetwork::GetTestAppId() const
{
	static const char kTestAppId[] = "IA_GameTest";
	return kTestAppId;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
