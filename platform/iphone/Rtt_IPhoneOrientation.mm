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

#include "Rtt_IPhoneOrientation.h"

#import <Foundation/NSString.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

NSString*
IPhoneOrientation::StringForOrientation( UIInterfaceOrientation interfaceOrientation )
{
	NSString *result = nil;
	switch ( interfaceOrientation )
	{
		case UIInterfaceOrientationPortraitUpsideDown:
			result = @"UIInterfaceOrientationPortraitUpsideDown";
			break;
		case UIInterfaceOrientationLandscapeLeft:
			result = @"UIInterfaceOrientationLandscapeLeft";
			break;
		case UIInterfaceOrientationLandscapeRight:
			result = @"UIInterfaceOrientationLandscapeRight";
			break;
		case UIInterfaceOrientationPortrait:
			result = @"UIInterfaceOrientationPortrait";
			break;
	}
	
	return result;
}

UIInterfaceOrientation
IPhoneOrientation::OrientationForString( NSString *value )
{
	UIInterfaceOrientation result = UIInterfaceOrientationPortrait;
	
	if ( [value isEqualToString:@"UIInterfaceOrientationPortraitUpsideDown"] )
	{
		result = UIInterfaceOrientationPortraitUpsideDown;
	}
	else if ( [value isEqualToString:@"UIInterfaceOrientationLandscapeLeft"] )
	{
		result = UIInterfaceOrientationLandscapeLeft;
	}
	else if ( [value isEqualToString:@"UIInterfaceOrientationLandscapeRight"] )
	{
		result = UIInterfaceOrientationLandscapeRight;
	}

	return result;
}

UIInterfaceOrientationMask
IPhoneOrientation::OrientationMaskForString( NSString *value )
{
	UIInterfaceOrientationMask result = UIInterfaceOrientationMaskPortrait;
	
	if ( [value isEqualToString:@"UIInterfaceOrientationPortraitUpsideDown"] )
	{
		result = UIInterfaceOrientationMaskPortraitUpsideDown;
	}
	else if ( [value isEqualToString:@"UIInterfaceOrientationLandscapeLeft"] )
	{
		result = UIInterfaceOrientationMaskLandscapeLeft;
	}
	else if ( [value isEqualToString:@"UIInterfaceOrientationLandscapeRight"] )
	{
		result = UIInterfaceOrientationMaskLandscapeRight;
	}

	return result;
}


DeviceOrientation::Type
IPhoneOrientation::ConvertOrientation( NSString *value )
{
	DeviceOrientation::Type result = DeviceOrientation::kUnknown;

	if ( [value isEqualToString:@"UIInterfaceOrientationPortrait"] )
	{
		result = DeviceOrientation::kUpright;
	}
	else if ( [value isEqualToString:@"UIInterfaceOrientationPortraitUpsideDown"] )
	{
		result = DeviceOrientation::kUpsideDown;
	}
	else if ( [value isEqualToString:@"UIInterfaceOrientationLandscapeLeft"] )
	{
		result = DeviceOrientation::kSidewaysLeft;
	}
	else if ( [value isEqualToString:@"UIInterfaceOrientationLandscapeRight"] )
	{
		result = DeviceOrientation::kSidewaysRight;
	}

	return result;
}

DeviceOrientation::Type
IPhoneOrientation::ConvertOrientation( UIInterfaceOrientation o )
{
	DeviceOrientation::Type result = DeviceOrientation::kUnknown;

	switch ( o )
	{
		case UIInterfaceOrientationPortrait:
			result = DeviceOrientation::kUpright;
			break;
		case UIInterfaceOrientationPortraitUpsideDown:
			result = DeviceOrientation::kUpsideDown;
			break;
		case UIInterfaceOrientationLandscapeLeft:
			result = DeviceOrientation::kSidewaysLeft;
			break;
		case UIInterfaceOrientationLandscapeRight:
			result = DeviceOrientation::kSidewaysRight;
			break;
	}

	return result;
}

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------
