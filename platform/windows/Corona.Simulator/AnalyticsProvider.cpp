//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Core/Rtt_Build.h"
#include "Simulator.h"
#include "AnalyticsProvider.h"


void AnalyticsProviderInit(const char *buildID)
{
	
}

void AnalyticsProviderFinish()
{

}

void AnalyticsProviderSendEvent(const char *eventName)
{
	// Just send EventName
}

void AnalyticsProviderSendEvent(const char *eventName, const char *eventDataKey, const char *eventDataValue)
{
	// Send event with single key-value
}

void AnalyticsProviderSendEvent(const char *eventName, const size_t numDataItems, char **eventDataKeys, char **eventDataValues)
{
	// Send event with multiple key-value entries
}

#ifdef __cplusplus

void AnalyticsProviderSendEvent( const char *eventName, std::map<std::string, std::string> keyValues )
{
	// Send event with multiple key-value entries
}

#endif // __cplusplus

