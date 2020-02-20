//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "Core/Rtt_Build.h"

#import "BuildSessionState.h"

#include "Rtt_MacConsolePlatform.h"
#include "Rtt_MacPlatform.h"
#include "Rtt_WebServicesSession.h"

@implementation BuildSessionState

@synthesize webServicesSession;

- (id) init
{
	self = [super init];
	if(nil != self)
	{
		macConsolePlatform = new Rtt::MacConsolePlatform;
		macPlatformServices = new Rtt::MacPlatformServices( *macConsolePlatform );
		webServicesSession = new Rtt::WebServicesSession( *macPlatformServices );
	}
	return self;
}

- (void) dealloc
{
	delete webServicesSession;
	delete macPlatformServices;
	delete macConsolePlatform;
	
	[super dealloc];
}

@end
