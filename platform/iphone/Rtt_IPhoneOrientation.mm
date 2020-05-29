//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
