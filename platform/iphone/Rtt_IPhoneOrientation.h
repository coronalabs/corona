//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhoneOrientation_H__
#define _Rtt_IPhoneOrientation_H__

#import <UIKit/UIApplication.h>

#include "Rtt_DeviceOrientation.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class IPhoneOrientation
{
	public:
		static NSString *StringForOrientation( UIInterfaceOrientation interfaceOrientation );
		static UIInterfaceOrientation OrientationForString( NSString *value );
		static UIInterfaceOrientationMask OrientationMaskForString( NSString *value );
		static DeviceOrientation::Type ConvertOrientation( NSString *value );
		static DeviceOrientation::Type ConvertOrientation( UIInterfaceOrientation o );
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_IPhoneOrientation_H__
