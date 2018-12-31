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

#ifndef __ratatouille__Rtt_AppleInputDeviceManager__
#define __ratatouille__Rtt_AppleInputDeviceManager__

#include "Input/Rtt_PlatformInputDeviceManager.h"
#include "Rtt_AppleInputDevice.h"

@class GCController;

namespace Rtt
{
	/// Stores a collection of all input devices that Corona supports and assigns them
	/// unique identifiers and descriptors for easy lookup within native code and in Lua.
	class AppleInputDeviceManager : public PlatformInputDeviceManager
	{
	public:
		AppleInputDevice::DriverType fCurrentDriverType;
		
		typedef AppleInputDeviceManager Self;

	public:
		AppleInputDeviceManager(Rtt_Allocator *allocatorPointer);
		virtual ~AppleInputDeviceManager();
		
		AppleInputDevice* GetBySerialNumber(NSString* sn);
		AppleInputDevice* GetByMFiDevice(GCController* controller);
		AppleInputDevice* GetByVendorName(NSString *vendor, AppleInputDevice::DriverType driver);

		
		int fLastSeenPlayerIndex;

	protected:
		virtual PlatformInputDevice* CreateUsing(const InputDeviceDescriptor &descriptor) override;
		virtual void Destroy(PlatformInputDevice* devicePointer) override;
		
	};
	
} // namespace Rtt


#endif /* defined(__ratatouille__Rtt_AppleInputDeviceManager__) */
