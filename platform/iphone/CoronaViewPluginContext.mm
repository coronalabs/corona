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

#import "CoronaViewPluginContext.h"

#import "CoronaViewController.h"
#import "CoronaViewPrivate.h"
#include "Display/Rtt_Display.h"
#include "Rtt_LuaContext.h"
#include "Rtt_PlatformDisplayObject.h"
#include "Rtt_Runtime.h"

// ----------------------------------------------------------------------------

@interface CoronaViewPluginContext()
{
@private
	CoronaViewController *fOwner; // Weak ptr
}
@end

// ----------------------------------------------------------------------------

@implementation CoronaViewPluginContext

- (id)initWithOwner:(CoronaViewController *)owner
{
	self = [super init];
	if ( self )
	{
		fOwner = owner;
	}
	return self;
}

- (UIWindow *)appWindow
{
	return fOwner.view.window;
}

- (UIViewController *)appViewController
{
	return fOwner;
}

- (lua_State *)L
{
	CoronaView *view = (CoronaView *)fOwner.view;
	return view.runtime->VMContext().L();
}

- (CGPoint)coronaPointToUIKitPoint:(CGPoint)coronaPoint
{
	using namespace Rtt;

	Rtt_Real x = Rtt_FloatToReal( coronaPoint.x );
	Rtt_Real y = Rtt_FloatToReal( coronaPoint.y );

	Rtt::Rect bounds;
	bounds.xMin = x;
	bounds.yMin = y;
	bounds.xMax = x;
	bounds.yMax = y;

	Rtt_Real sx, sy;
	CoronaView *view = (CoronaView *)fOwner.view;
	const Display& display = view.runtime->GetDisplay();
	display.CalculateContentToScreenScale( sx, sy );
	bool shouldScale = ! Rtt_RealEqual( Rtt_REAL_1, sx ) || ! Rtt_RealEqual( Rtt_REAL_1, sy );
	if ( shouldScale )
	{
		sx = Rtt_RealDiv( Rtt_REAL_1, sx );
		sy = Rtt_RealDiv( Rtt_REAL_1, sy );
		PlatformDisplayObject::CalculateScreenBounds( display, sx, sy, bounds );
	}

	CGPoint result = { Rtt_RealToFloat( bounds.xMin ), Rtt_RealToFloat( bounds.yMin ) };

	return result;
}

- (void)suspend
{
	CoronaView *view = (CoronaView *)fOwner.view;
	[view suspend];
}

- (void)resume
{
	CoronaView *view = (CoronaView *)fOwner.view;
	[view resume];
}

@end

// ----------------------------------------------------------------------------

