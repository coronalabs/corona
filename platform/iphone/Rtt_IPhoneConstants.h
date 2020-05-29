//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_IPhoneConstants_H__
#define _Rtt_IPhoneConstants_H__

#include <CoreFoundation/CoreFoundation.h>

// Generic Authorization status enum.
// All documented AuthorizationStatus enums have this same mapping.
typedef enum : NSInteger {
	AuthorizationStatusNotDetermined = 0,
	AuthorizationStatusRestricted,
	AuthorizationStatusDenied,
	AuthorizationStatusAuthorized
} AuthorizationStatus;

#endif /* _Rtt_IPhoneScreenSurface_H__ */

