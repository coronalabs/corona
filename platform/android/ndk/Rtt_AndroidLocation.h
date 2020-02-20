//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////


#ifndef _Rtt_AndroidLocation_H__
#define _Rtt_AndroidLocation_H__

#include "librtt/Rtt_PlatformLocation.h"

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

class AndroidLocation : public PlatformLocation
{
	public:
		virtual ~AndroidLocation();

};

// ----------------------------------------------------------------------------

class AndroidLocationManager : public PlatformLocationManager
{
	public:
		virtual ~AndroidLocationManager();

};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_AndroidLocation_H__
