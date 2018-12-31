//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
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

