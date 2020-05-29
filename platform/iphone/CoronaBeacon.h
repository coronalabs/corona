//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _CoronaBeacon_H_
#define _CoronaBeacon_H_

#import "CoronaLua.h"

class CoronaBeacon
{
public:
    static const char REQUEST[];
    static const char IMPRESSION[];
    static const char DELIVERY[];

public:
    static int sendDeviceDataToBeacon(lua_State *L, const char *pluginName, const char *pluginVersion, const char *eventType, const char *placementId, int (*networkListener)(lua_State *L));
};

#endif // _CoronaBeacon_H_
