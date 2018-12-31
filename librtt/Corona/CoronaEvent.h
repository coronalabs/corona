//-----------------------------------------------------------------------------
//
// Corona Labs
//
// easing.lua
//
// Code is MIT licensed; see https://www.coronalabs.com/links/code/license
//
//-----------------------------------------------------------------------------

#ifndef _CoronaEvent_H__
#define _CoronaEvent_H__

#include "CoronaMacros.h"

// Generic property keys
// ----------------------------------------------------------------------------

// String: 'event.name'
CORONA_API const char *CoronaEventNameKey(void) CORONA_PUBLIC_SUFFIX;

// String: 'event.provider'
CORONA_API const char *CoronaEventProviderKey(void) CORONA_PUBLIC_SUFFIX;

// String: 'event.phase'
CORONA_API const char *CoronaEventPhaseKey(void) CORONA_PUBLIC_SUFFIX;

// String: 'event.type'
CORONA_API const char *CoronaEventTypeKey(void) CORONA_PUBLIC_SUFFIX;

// String: 'event.response'
CORONA_API const char *CoronaEventResponseKey(void) CORONA_PUBLIC_SUFFIX;

// Boolean: 'event.isError'
CORONA_API const char *CoronaEventIsErrorKey(void) CORONA_PUBLIC_SUFFIX;

// Number: 'event.errorCode'
CORONA_API const char *CoronaEventErrorCodeKey(void) CORONA_PUBLIC_SUFFIX;

// General purpose: 'event.data'
CORONA_API const char *CoronaEventDataKey(void) CORONA_PUBLIC_SUFFIX;

// Event types for library providers
// ----------------------------------------------------------------------------

// For "ads" providers
CORONA_API const char *CoronaEventAdsRequestName(void) CORONA_PUBLIC_SUFFIX;

// For "gameNetwork" providers
CORONA_API const char *CoronaEventGameNetworkName(void) CORONA_PUBLIC_SUFFIX;

// For "native.popup" providers
CORONA_API const char *CoronaEventPopupName(void) CORONA_PUBLIC_SUFFIX;

// ----------------------------------------------------------------------------

#endif // _CoronaEvent_H__
