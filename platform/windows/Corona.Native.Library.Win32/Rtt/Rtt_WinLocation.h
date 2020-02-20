//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_WinLocation_H__
#define _Rtt_WinLocation_H__

#include "Rtt_PlatformLocation.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class WinLocation : public PlatformLocation
{
	public:
		virtual ~WinLocation();

};

// ----------------------------------------------------------------------------

class WinLocationManager : public PlatformLocationManager
{
	public:
		virtual ~WinLocationManager();

};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_WinLocation_H__
