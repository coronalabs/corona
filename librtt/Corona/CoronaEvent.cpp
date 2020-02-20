//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "CoronaEvent.h"

#include "Rtt_Event.h"

// Generic property keys
// ----------------------------------------------------------------------------

CORONA_API
const char *CoronaEventNameKey()
{
	return Rtt::MEvent::kNameKey;
}

CORONA_API
const char *CoronaEventProviderKey()
{
	return Rtt::MEvent::kProviderKey;
}

CORONA_API
const char *CoronaEventPhaseKey()
{
	return Rtt::MEvent::kPhaseKey;
}

CORONA_API
const char *CoronaEventTypeKey()
{
	return Rtt::MEvent::kTypeKey;
}

CORONA_API
const char *CoronaEventResponseKey()
{
	return Rtt::MEvent::kResponseKey;
}

CORONA_API
const char *CoronaEventIsErrorKey()
{
	return Rtt::MEvent::kIsErrorKey;
}

CORONA_API
const char *CoronaEventErrorCodeKey()
{
	return Rtt::MEvent::kErrorCodeKey;
}

CORONA_API
const char *CoronaEventDataKey()
{
	return Rtt::MEvent::kDataKey;
}

// Event types
// ----------------------------------------------------------------------------

CORONA_API
const char *CoronaEventAdsRequestName()
{
	return Rtt::AdsRequestEvent::kName;
}

CORONA_API
const char *CoronaEventGameNetworkName()
{
	return Rtt::GameNetworkEvent::kName;
}

CORONA_API
const char *CoronaEventPopupName()
{
	return "popup";
}

// ----------------------------------------------------------------------------

