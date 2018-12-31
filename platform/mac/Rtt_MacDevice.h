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

#ifndef _Rtt_MacDevice_H__
#define _Rtt_MacDevice_H__

#include "Rtt_MPlatformDevice.h"
#include "Rtt_DeviceOrientation.h"

#if defined( Rtt_NO_GUI )
	#include "Input/Rtt_PlatformInputDeviceManager.h"
#else
	#include "Rtt_AppleInputDeviceManager.h"
#endif

// ----------------------------------------------------------------------------

@class AppleCallback;
@class NSString;
@class CoronaView;

namespace Rtt
{

class MCallback;
class PlatformSimulator;

// ----------------------------------------------------------------------------

class MacConsoleDevice : public MPlatformDevice
{
	public:
		typedef MPlatformDevice Super;

	public:
		MacConsoleDevice( Rtt_Allocator &allocator, CoronaView *view );
		virtual ~MacConsoleDevice();

	public:
		void SetManufacturer( NSString *newValue );
		void SetModel( NSString *newValue );

	public:
		const char* GetDeviceIdentifier() const;

	public:
		virtual const char* GetName() const;
		virtual const char* GetManufacturer() const;
		virtual const char* GetModel() const;
		virtual const char* GetUniqueIdentifier( IdentifierType t ) const;
		virtual EnvironmentType GetEnvironment() const;
		virtual const char* GetPlatformName() const;
		virtual const char* GetPlatform() const;
		virtual const char* GetPlatformVersion() const;
		virtual const char* GetArchitectureInfo() const;
		virtual PlatformInputDeviceManager& GetInputDeviceManager() override;
	
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
		CoronaView *fView; // Weak ref

	private:
		Rtt_Allocator &fAllocator;
		mutable NSString *fDeviceManufacturer;
		mutable NSString *fDeviceModel;
		mutable NSString *fSystemVersion;
		mutable char* fDeviceIdentifier;
		DeviceNotificationTracker fTracker;
#if defined( Rtt_NO_GUI )
		PlatformInputDeviceManager fInputDeviceManager;
#else
		AppleInputDeviceManager fInputDeviceManager;
#endif

	
};

// TODO: Move this to a separate file
#if !defined( Rtt_WEB_PLUGIN )

class MacDevice : public MacConsoleDevice
{
	public:
		typedef MacConsoleDevice Super;

	public:
//		static DeviceOrientation::Type ToOrientationType( UIDeviceOrientation orientation );

	public:
		MacDevice( Rtt_Allocator &allocator, PlatformSimulator& simulator );

	public:
		virtual void Vibrate() const;

	public:
		virtual void BeginNotifications( EventType type ) const;
		virtual void EndNotifications( EventType type ) const;
		virtual bool HasEventSource( EventType type ) const;
		virtual void SetAccelerometerInterval( U32 frequency ) const;
		virtual void SetGyroscopeInterval( U32 frequency ) const;
		virtual const char *GetPlatform() const;

	public:
		virtual void SetLocationAccuracy( Real meters ) const;
		virtual void SetLocationThreshold( Real meters ) const;

	public:
		virtual DeviceOrientation::Type GetOrientation() const;

	public:
		PlatformSimulator& GetSimulator() const { return fSimulator; }

	public:
//		DeviceOrientation::Type GetPreviousOrientationAndUpdate( UIDeviceOrientation newValue );

	private:
		PlatformSimulator& fSimulator;
		mutable DeviceOrientation::Type fPreviousOrientation;
};

#endif // Rtt_WEB_PLUGIN
	
class MacAppDevice : public MacConsoleDevice
{
	public:
		typedef MacConsoleDevice Super;

	public:
		MacAppDevice( Rtt_Allocator &allocator, DeviceOrientation::Type orientation );
		virtual DeviceOrientation::Type GetOrientation() const;

	private:
		DeviceOrientation::Type fDeviceOrientation;
};


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_MacDevice_H__
