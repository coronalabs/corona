//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _Rtt_MacDeviceIdentifier_H__
#define _Rtt_MacDeviceIdentifier_H__

#include "Core/Rtt_Build.h"

#define Rtt_EthernetAddressSize 6

Rtt_EXPORT int Rtt_GetDeviceIdentifier( U8 MACAddress[Rtt_EthernetAddressSize] );

#endif // _Rtt_MacDeviceIdentifier_H__
