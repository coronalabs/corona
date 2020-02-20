//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
	switch (android_getCpuFamily())
	{
	    case ANDROID_CPU_FAMILY_ARM:
            if ((android_getCpuFeatures() & ANDROID_CPU_ARM_FEATURE_NEON) != 0)
                return "ARM Neon";
            else
                return "ARM";
        case ANDROID_CPU_FAMILY_X86:
            return "x86";
        case ANDROID_CPU_FAMILY_MIPS:
            return "MIPS";
        case ANDROID_CPU_FAMILY_ARM64:
            return "ARM64";
        case ANDROID_CPU_FAMILY_X86_64:
            return "x86_64";
        case ANDROID_CPU_FAMILY_MIPS64:
            return "MIPS64";
        default:
            break;
	}
	return "Unknown";
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

