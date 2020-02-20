//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BUILD_SESSION_STATE_H
#define BUILD_SESSION_STATE_H

#import <Foundation/Foundation.h>

namespace Rtt
{
	class MacConsolePlatform;
	class MacPlatformServices;
	class WebServicesSession;
}

@interface BuildSessionState : NSObject
{
	Rtt::MacConsolePlatform* macConsolePlatform;
	Rtt::MacPlatformServices* macPlatformServices;
	Rtt::WebServicesSession* webServicesSession;
}

@property(nonatomic, readonly, assign) Rtt::WebServicesSession* webServicesSession;

@end

#endif // BUILD_SESSION_STATE_H