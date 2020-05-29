//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

// This is the DeskMetrics implementation of AnalyticsProvider

#include "Core/Rtt_Build.h"

#import <AnalyticsProvider.h>

#ifdef __cplusplus
#include <string>
#include <map>
#endif

void AnalyticsProviderInit(const char *buildID)
{
}

void AnalyticsProviderFinish()
{
}

void AnalyticsProviderSendEvent(const char *eventName)
{
}

void AnalyticsProviderSendEvent(const char *eventName, const char *eventDataKey, const char *eventDataValue)
{
}

void AnalyticsProviderSendEvent(const char *eventName, const size_t numDataItems, char **eventDataKeys, char **eventDataValues)
{
}

#ifdef __cplusplus

void AnalyticsProviderSendEvent( const char *eventName, std::map<std::string, std::string> keyValues )
{
}

#endif // __cplusplus
