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


#include "Core/Rtt_Build.h"

#include "Rtt_AndroidDevice.h"
#include "NativeToJavaBridge.h"

#ifdef Rtt_DEBUG
#include <android/log.h>
#endif

#include <dlfcn.h>
#include <cpu-features.h>

// ----------------------------------------------------------------------------

namespace Rtt
{

// ----------------------------------------------------------------------------

AndroidDevice::AndroidDevice( Rtt_Allocator& allocator, NativeToJavaBridge *ntjb )
:	fAllocator( allocator ),
	fInputDeviceManager( &allocator, ntjb ),
	fName( NULL ),
	fManufacturer( NULL ),
	fModel( NULL ),
	fUniqueIdentifier( NULL ),
	fHardwareIdentifier( NULL ),
	fOSIdentifier( NULL ),
	fPlatformVersion( NULL ),
	fProduct( NULL ),
	fOrientation( DeviceOrientation::kUnknown ),
	fNativeToJavaBridge( ntjb )
{
}

AndroidDevice::~AndroidDevice()
{
	Rtt_DELETE( fProduct );
	Rtt_DELETE( fPlatformVersion );
	Rtt_DELETE( fOSIdentifier );
	Rtt_DELETE( fHardwareIdentifier );
	Rtt_DELETE( fUniqueIdentifier );
	Rtt_DELETE( fModel );
	Rtt_DELETE( fManufacturer );
	Rtt_DELETE( fName );
}

void
AndroidDevice::SetOrientation( DeviceOrientation::Type orientation )
{
	fOrientation = orientation;
}

void
AndroidDevice::Vibrate() const
{
	fNativeToJavaBridge->Vibrate();
}

void
AndroidDevice::SetAccelerometerInterval( U32 frequency ) const
{
	fNativeToJavaBridge->SetAccelerometerInterval( frequency );
}

void
AndroidDevice::SetGyroscopeInterval( U32 frequency ) const
{
	fNativeToJavaBridge->SetGyroscopeInterval( frequency );
}

bool
AndroidDevice::HasEventSource( EventType type ) const
{
	bool hasEventSource = false;
	
	switch (type)
	{
		case MPlatformDevice::kAccelerometerEvent:
			hasEventSource = fNativeToJavaBridge->HasAccelerometer();
			break;
		case MPlatformDevice::kGyroscopeEvent:
			hasEventSource = fNativeToJavaBridge->HasGyroscope();
			break;
		case MPlatformDevice::kHeadingEvent:
			hasEventSource = fNativeToJavaBridge->HasHeadingHardware();
			break;
		case MPlatformDevice::kInputDeviceStatusEvent:
		case MPlatformDevice::kKeyEvent:
		case MPlatformDevice::kLocationEvent:
		case MPlatformDevice::kMouseEvent:
		case MPlatformDevice::kMultitouchEvent:
		case MPlatformDevice::kOrientationEvent:
			hasEventSource = true;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}
	return hasEventSource;
}

void
AndroidDevice::BeginNotifications( EventType type ) const
{
#ifdef Rtt_DEBUG
    __android_log_print(ANDROID_LOG_INFO, "Corona", "> BeginNotifications %d", type );
#endif

	fTracker.BeginNotifications( type );

	switch( type )
	{
		case MPlatformDevice::kOrientationEvent:
		case MPlatformDevice::kLocationEvent:
		case MPlatformDevice::kAccelerometerEvent:
		case MPlatformDevice::kGyroscopeEvent:
		case MPlatformDevice::kHeadingEvent:
		case MPlatformDevice::kMultitouchEvent:
			fNativeToJavaBridge->SetEventNotification( type, true );
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}
}

void
AndroidDevice::EndNotifications( EventType type ) const
{
	fTracker.EndNotifications( type );

	switch( type )
	{
		case MPlatformDevice::kOrientationEvent:
		case MPlatformDevice::kLocationEvent:
		case MPlatformDevice::kAccelerometerEvent:
		case MPlatformDevice::kGyroscopeEvent:
		case MPlatformDevice::kHeadingEvent:
		case MPlatformDevice::kMultitouchEvent:
			fNativeToJavaBridge->SetEventNotification( type, false );
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}
}

bool
AndroidDevice::DoesNotify( EventType type ) const
{
	return fTracker.DoesNotify( type );
}

const char*
AndroidDevice::GetName() const
{
	if ( ! fName )
	{
		fName = Rtt_NEW( & fAllocator, String( & fAllocator ) );
		fNativeToJavaBridge->GetName( fName );
	}

	return fName->GetString();
}

const char*
AndroidDevice::GetManufacturer() const
{
	if ( ! fManufacturer )
	{
		fManufacturer = Rtt_NEW( & fAllocator, String( & fAllocator ) );
		fNativeToJavaBridge->GetManufacturerName( fManufacturer );
	}

	return fManufacturer->GetString();
}

const char*
AndroidDevice::GetModel() const
{
	if ( ! fModel )
	{
		fModel = Rtt_NEW( & fAllocator, String( & fAllocator ) );
		fNativeToJavaBridge->GetModel( fModel );
	}

	return fModel->GetString();
}

const char*
AndroidDevice::GetUniqueIdentifier( IdentifierType t ) const
{
	const char *result = NULL;

	switch ( t )
	{
		case MPlatformDevice::kDeviceIdentifier:
			// The unique identifier can change depending on the state of the "READ_PHONE_STATE" permission, 
			// so we can't cache the value retrieved the first time.
			fUniqueIdentifier = Rtt_NEW( & fAllocator, String( & fAllocator ) );
			fNativeToJavaBridge->GetUniqueIdentifier( t, fUniqueIdentifier );
			result = fUniqueIdentifier->GetString();
			break;
		case MPlatformDevice::kHardwareIdentifier:
			if ( ! fHardwareIdentifier )
			{
				fHardwareIdentifier = Rtt_NEW( & fAllocator, String( & fAllocator ) );
				fNativeToJavaBridge->GetUniqueIdentifier( t, fHardwareIdentifier );
			}
			result = fHardwareIdentifier->GetString();
			break;
		case MPlatformDevice::kOSIdentifier:
			if ( ! fOSIdentifier )
			{
				fOSIdentifier = Rtt_NEW( & fAllocator, String( & fAllocator ) );
				fNativeToJavaBridge->GetUniqueIdentifier( t, fOSIdentifier );
			}
			result = fOSIdentifier->GetString();
			break;
		case MPlatformDevice::kUdidIdentifier:
			// TODO
			break;
		default:
			break;
	}

	return result;
}

MPlatformDevice::EnvironmentType
AndroidDevice::GetEnvironment() const
{
	return kDeviceEnvironment;
}

const char*
AndroidDevice::GetPlatformName() const
{
	return "Android";
}
	
const char*
AndroidDevice::GetPlatform() const
{
	return "android";
}

const char*
AndroidDevice::GetPlatformVersion() const
{
	if ( ! fPlatformVersion )
	{
		fPlatformVersion = Rtt_NEW( & fAllocator, String( & fAllocator ) );
		fNativeToJavaBridge->GetPlatformVersion( fPlatformVersion );
	}
	
	return fPlatformVersion->GetString();
}

const char*
AndroidDevice::GetProductName() const
{
	if ( ! fProduct )
	{
		fProduct = Rtt_NEW( & fAllocator, String( & fAllocator ) );
		fNativeToJavaBridge->GetName( fProduct );
	}
	
	return fProduct->GetString();
}

const char*
AndroidDevice::GetArchitectureInfo() const
{
	// Test if we are running an ARM built app on an x86 CPU via Intel's "libhoudini.so" translation library.
	// Note: We do this because there is a known bug in "libhoudini.so" where calling the below CPU functions
	//       will cause a crash on some version of Intel's library. We work-around this issue by identifying
	//       if "libhoudini.so" is currently loaded, which means we can assume that we're running on an x86 CPU.
	static bool sIsUsingArmToX86Translator = false;
#ifdef __arm__
	static bool sHasTestedForArmToX86Translator = false;
	if (!sHasTestedForArmToX86Translator)
	{
		dlerror();
		void *libraryHandle = dlopen("libhoudini.so", RTLD_LAZY);
		if (libraryHandle)
		{
			sIsUsingArmToX86Translator = true;
		}
		sHasTestedForArmToX86Translator = true;
	}
#endif

	// Fetch the name of the CPU type we're running on.
	// Note: Do not call the android_getCpu*() functions while "libhoudini.so" is loaded to avoid a crash.
	const char *name = "Unknown";
	if (sIsUsingArmToX86Translator || (android_getCpuFamily() == ANDROID_CPU_FAMILY_X86))
	{
		name = "x86";
	}
	else if (android_getCpuFamily() == ANDROID_CPU_FAMILY_ARM)
	{
		if ((android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0)
		{
			name = "ARM Neon";
		}
		else
		{
			name = "ARM";
		}
	}
	return name;
}

PlatformInputDeviceManager&
AndroidDevice::GetInputDeviceManager()
{
	return fInputDeviceManager;
}

void 
AndroidDevice::SetLocationAccuracy( Real meters ) const
{
	fNativeToJavaBridge->SetLocationAccuracy( meters );
}

void 
AndroidDevice::SetLocationThreshold( Real meters ) const
{
	fNativeToJavaBridge->SetLocationThreshold( meters );
}

DeviceOrientation::Type 
AndroidDevice::GetOrientation() const
{
	return fOrientation;
}


// ----------------------------------------------------------------------------

} // namespace Rtt

// ----------------------------------------------------------------------------

