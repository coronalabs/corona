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

#ifndef _Rtt_TVOSDevice_H__
#define _Rtt_TVOSDevice_H__

#include "Rtt_MPlatformDevice.h"
#include "Rtt_DeviceOrientation.h"
#include "Rtt_AppleInputDeviceManager.h"

#import <UIKit/UIApplication.h>
#import <UIKit/UIDevice.h>

// ----------------------------------------------------------------------------

@class AppleCallback;
@class CoronaView;

namespace Rtt
{

class IPhonePlatform;
class MCallback;

// ----------------------------------------------------------------------------

class TVOSDevice : public MPlatformDevice
{
	public:
		typedef MPlatformDevice Super;

	public:
		TVOSDevice( Rtt_Allocator &allocator, CoronaView *view );
		~TVOSDevice();

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
		virtual PlatformInputDeviceManager& GetInputDeviceManager();
	
	public:
		virtual void Vibrate() const;

	public:
		virtual void BeginNotifications( EventType type ) const;
		virtual void EndNotifications( EventType type ) const;
		virtual bool DoesNotify( EventType type ) const;
		virtual bool HasEventSource( EventType type ) const;
		virtual void SetAccelerometerInterval( U32 frequency ) const;
		virtual void SetGyroscopeInterval( U32 frequency ) const;

		virtual bool Activate( ActivationType key ) const;
		virtual bool Deactivate( ActivationType key ) const;

	public:
		virtual void SetLocationAccuracy( Real meters ) const;
		virtual void SetLocationThreshold( Real meters ) const;

	public:
		virtual DeviceOrientation::Type GetOrientation() const;

	public:
///		DeviceOrientation::Type GetPreviousOrientationAndUpdate( UIDeviceOrientation newValue );

	private:
		Rtt_Allocator &fAllocator;
		CoronaView *fView; // Weak ref
		DeviceNotificationTracker fTracker;
		mutable DeviceOrientation::Type fPreviousOrientation;
		AppleInputDeviceManager fInputDeviceManager;
};

// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

#endif // _Rtt_TVOSDevice_H__
