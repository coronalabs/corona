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

// ------------------------------------------------
// This is a stub to enable building CoronaBuilder
// without importing most of the Simulator code
// ------------------------------------------------

#ifndef _Rtt_MacDevice_H__
#define _Rtt_MacDevice_H__

#include "Rtt_MPlatformDevice.h"
#include "Rtt_DeviceOrientation.h"

#include "Input/Rtt_PlatformInputDeviceManager.h"

#include <list>
#include <string>

// ----------------------------------------------------------------------------

namespace Rtt
{

class MCallback;
class PlatformSimulator;

// ----------------------------------------------------------------------------

class WinConsoleDevice : public MPlatformDevice
{
	public:
		typedef MPlatformDevice Super;

	public:
		WinConsoleDevice(Rtt_Allocator &allocator);
		virtual ~WinConsoleDevice();

	public:
		virtual const char* GetModel() const;
		virtual const char* GetName() const;
		virtual const char* GetUniqueIdentifier( IdentifierType t ) const;
		virtual EnvironmentType GetEnvironment() const;
		virtual const char* GetPlatformName() const;
		virtual const char* GetPlatform() const override { Rtt_ASSERT_MSG( 0, "Code should NOT be reached" ); return NULL; }
		virtual const char* GetPlatformVersion() const;
		virtual const char* GetArchitectureInfo() const;
		virtual PlatformInputDeviceManager& GetInputDeviceManager() override;
		virtual const char* GetManufacturer() const override { Rtt_ASSERT_MSG( 0, "Code should NOT be reached" ); return NULL; }
	
	public:
		// No-ops
		virtual void Vibrate() const;
		virtual void BeginNotifications( EventType type ) const;
		virtual void EndNotifications( EventType type ) const;
		virtual bool DoesNotify( EventType type ) const;
		virtual bool HasEventSource( EventType type ) const;
		virtual void SetAccelerometerInterval( U32 frequency ) const;
		virtual void SetGyroscopeInterval( U32 frequency ) const;

	public:
		virtual void SetLocationAccuracy( Real meters ) const;
		virtual void SetLocationThreshold( Real meters ) const;

	public:
		virtual DeviceOrientation::Type GetOrientation() const;

	protected:

	private:
		bool CopyMachineSidStringTo(char *sidString, int sidStringSize) const;
		bool CopyFirstEthernetMacAddressStringTo(char *addressString, int addressStringSize) const;
		int AddEthernetMacAddressStringsTo(std::list<std::string> &stringCollection) const;

		PlatformInputDeviceManager fInputDeviceManager;
		mutable char* fDeviceIdentifier;
		mutable char* fComputerName;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MacDevice_H__
