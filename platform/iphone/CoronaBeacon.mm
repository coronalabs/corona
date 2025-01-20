//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#import "CoronaBeacon.h"


// Constants
const char CoronaBeacon::REQUEST[]    = "request";
const char CoronaBeacon::IMPRESSION[] = "impression";
const char CoronaBeacon::DELIVERY[]   = "delivery";


// Send device data to the corona beacon
int
CoronaBeacon::sendDeviceDataToBeacon(lua_State *L, const char *pluginName, const char *pluginVersion, const char *eventType, const char *placementId, int (*networkListener)(lua_State *L))
{
	return 0;
}
