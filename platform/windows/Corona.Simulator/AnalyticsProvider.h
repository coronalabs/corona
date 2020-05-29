//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef AnalyticsProvider_h
#define AnalyticsProvider_h

void AnalyticsProviderInit(const char *buildID);
void AnalyticsProviderFinish();
void AnalyticsProviderSendEvent(const char *eventName);
void AnalyticsProviderSendEvent(const char *eventName, const char *eventDataKey, const char *eventDataValue);
void AnalyticsProviderSendEvent(const char *eventName, const size_t numDataItems, char **eventDataKeys, char **eventDataValues);
#ifdef __cplusplus
#include <map>
void AnalyticsProviderSendEvent( const char *eventName, std::map<std::string, std::string> keyValues );
#endif

#endif /* AnalyticsProvider_h */
