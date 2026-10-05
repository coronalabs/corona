//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Rtt_WinRTDevice.h"
#include "CoronaLabs\Corona\WinRT\Interop\Input\IInputDeviceServices.h"
#include "CoronaLabs\Corona\WinRT\Interop\Sensors\AccelerometerMonitor.h"
#include "CoronaLabs\Corona\WinRT\Interop\Sensors\CoronaAccelerometerHandler.h"
#include "CoronaLabs\Corona\WinRT\Interop\Sensors\CoronaGyroscopeHandler.h"
#include "CoronaLabs\Corona\WinRT\Interop\Sensors\GyroscopeMonitor.h"
#include "CoronaLabs\Corona\WinRT\Interop\UI\ICoronaControlAdapter.h"
#include "CoronaLabs\Corona\WinRT\Interop\UI\IDispatcher.h"
#include "CoronaLabs\Corona\WinRT\Interop\UI\IPage.h"
#include "CoronaLabs\Corona\WinRT\Interop\UI\PageOrientation.h"
#include "CoronaLabs\Corona\WinRT\Interop\InteropServices.h"
#include "CoronaLabs\Corona\WinRT\CoronaRuntimeEnvironment.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core\Rtt_Build.h"
#	include "Rtt_Event.h"
#	include "Rtt_Runtime.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace Rtt
{

#pragma region Constructors/Destructors
WinRTDevice::WinRTDevice(Rtt_Allocator& allocator, CoronaLabs::Corona::WinRT::CoronaRuntimeEnvironment^ environment)
:	fAllocator(allocator),
	fAccelerometerHandler(nullptr),
	fGyroscopeHandler(nullptr),
	fInputDeviceManager(&allocator),
	fEnvironment(environment)
{
	// Validate.
	if (!environment)
	{
		throw ref new Platform::NullReferenceException("environment");
	}

	// Create all sensor monitors and their event handlers.
	fAccelerometerHandler = ref new CoronaLabs::Corona::WinRT::Interop::Sensors::CoronaAccelerometerHandler(environment);
	fAccelerometerHandler->AccelerometerMonitor = ref new CoronaLabs::Corona::WinRT::Interop::Sensors::AccelerometerMonitor();
	fGyroscopeHandler = ref new CoronaLabs::Corona::WinRT::Interop::Sensors::CoronaGyroscopeHandler(environment);
	fGyroscopeHandler->GyroscopeMonitor = ref new CoronaLabs::Corona::WinRT::Interop::Sensors::GyroscopeMonitor();
}

WinRTDevice::~WinRTDevice()
{
	// Stop all sensor monitors.
	fAccelerometerHandler->AccelerometerMonitor->Stop();
	fAccelerometerHandler->AccelerometerMonitor = nullptr;
	fGyroscopeHandler->GyroscopeMonitor->Stop();
	fGyroscopeHandler->GyroscopeMonitor = nullptr;
}

#pragma endregion


#pragma region Public Member Functions
void WinRTDevice::Vibrate() const
{
#if (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
	auto vibrationDevice = Windows::Phone::Devices::Notification::VibrationDevice::GetDefault();
	if (vibrationDevice)
	{
		static const int durationInMilliseconds = 200;
		Windows::Foundation::TimeSpan timeSpan;
		timeSpan.Duration = durationInMilliseconds * 10000LL;
		vibrationDevice->Vibrate(timeSpan);
	}
#endif
}

void WinRTDevice::SetAccelerometerInterval( U32 frequency ) const
{
	// Do not allow a frequency of zero.
	if (frequency < 1)
	{
		frequency = 1;
	}

	// Convert frequency to milliseconds.
	long long intervalInMilliseconds = 1000 / frequency;

	// Convert millisecond interval to a TimeSpan object, which is measured in per 100 nano-seconds.
	Windows::Foundation::TimeSpan interval;
	interval.Duration = intervalInMilliseconds * 10000LL;

	// Update the sensor's sample interval.
	fAccelerometerHandler->AccelerometerMonitor->Interval = interval;
}

void WinRTDevice::SetGyroscopeInterval( U32 frequency ) const
{
	// Do not allow a frequency of zero.
	if (frequency < 1)
	{
		frequency = 1;
	}

	// Convert frequency to milliseconds.
	long long intervalInMilliseconds = 1000 / frequency;

	// Convert millisecond interval to a TimeSpan object, which is measured in per 100 nano-seconds.
	Windows::Foundation::TimeSpan interval;
	interval.Duration = intervalInMilliseconds * 10000LL;

	// Update the sensor's sample interval.
	fGyroscopeHandler->GyroscopeMonitor->Interval = interval;
}

bool WinRTDevice::HasEventSource( EventType type ) const
{
	bool hasEventSource = false;
	
	switch (type)
	{
		case MPlatformDevice::kAccelerometerEvent:
			hasEventSource = fAccelerometerHandler->AccelerometerMonitor->IsAvailable;
			break;
		case MPlatformDevice::kGyroscopeEvent:
			hasEventSource = fGyroscopeHandler->GyroscopeMonitor->IsAvailable;
			break;
		case MPlatformDevice::kLocationEvent:
		case MPlatformDevice::kHeadingEvent:
		case MPlatformDevice::kInputDeviceStatusEvent:
		case MPlatformDevice::kMouseEvent:
			break;
		case MPlatformDevice::kOrientationEvent:
		case MPlatformDevice::kMultitouchEvent:
		case MPlatformDevice::kKeyEvent:
			hasEventSource = true;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}
	return hasEventSource;
}

void WinRTDevice::BeginNotifications( EventType type ) const
{
	fTracker.BeginNotifications( type );

	switch( type )
	{
		case MPlatformDevice::kAccelerometerEvent:
			fAccelerometerHandler->AccelerometerMonitor->Start();
			break;
		case MPlatformDevice::kGyroscopeEvent:
			fGyroscopeHandler->GyroscopeMonitor->Start();
			break;
		case MPlatformDevice::kOrientationEvent:
		case MPlatformDevice::kLocationEvent:
		case MPlatformDevice::kHeadingEvent:
			break;
		case MPlatformDevice::kMultitouchEvent:
			fEnvironment->InteropServices->InputDeviceServices->MultitouchEnabled = true;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}
}

void WinRTDevice::EndNotifications( EventType type ) const
{
	fTracker.EndNotifications( type );

	switch( type )
	{
		case MPlatformDevice::kAccelerometerEvent:
			fAccelerometerHandler->AccelerometerMonitor->Stop();
			break;
		case MPlatformDevice::kGyroscopeEvent:
			fGyroscopeHandler->GyroscopeMonitor->Stop();
			break;
		case MPlatformDevice::kOrientationEvent:
		case MPlatformDevice::kLocationEvent:
		case MPlatformDevice::kHeadingEvent:
			break;
		case MPlatformDevice::kMultitouchEvent:
			fEnvironment->InteropServices->InputDeviceServices->MultitouchEnabled = false;
			break;
		default:
			Rtt_ASSERT_NOT_REACHED();
			break;
	}
}

bool WinRTDevice::DoesNotify( EventType type ) const
{
	return fTracker.DoesNotify( type );
}

const char* WinRTDevice::GetName() const
{
	return "Not Supported Yet";
}

const char* WinRTDevice::GetManufacturer() const
{
	return "Not Supported Yet";
}

const char* WinRTDevice::GetModel() const
{
	return "Not Supported Yet";
}

const char* WinRTDevice::GetUniqueIdentifier( IdentifierType t ) const
{
	const char *stringId = "Not Supported Yet";

	switch ( t )
	{
		case MPlatformDevice::kDeviceIdentifier:
			break;
		case MPlatformDevice::kHardwareIdentifier:
			break;
		case MPlatformDevice::kOSIdentifier:
			break;
		case MPlatformDevice::kUdidIdentifier:
			break;
		default:
			stringId = NULL;
			break;
	}
	return stringId;
}

MPlatformDevice::EnvironmentType WinRTDevice::GetEnvironment() const
{
	return kDeviceEnvironment;
}

const char* WinRTDevice::GetPlatformName() const
{
	return "WinPhone";
}
	
const char* WinRTDevice::GetPlatform() const
{
	return "winphone";
}

const char* WinRTDevice::GetPlatformVersion() const
{
	return "8.0";
}

const char* WinRTDevice::GetProductName() const
{
	return "Not Supported Yet";
}

const char* WinRTDevice::GetArchitectureInfo() const
{
	SYSTEM_INFO sysInfo;
	const char *name;

	GetNativeSystemInfo(&sysInfo);
	switch (sysInfo.wProcessorArchitecture)
	{
		case PROCESSOR_ARCHITECTURE_AMD64:
			name = "x64"; //AMD or Intel
			break;
		case PROCESSOR_ARCHITECTURE_ARM:
			name = "ARM";
			break;
		case PROCESSOR_ARCHITECTURE_IA64:
			name = "IA64";
			break;
		case PROCESSOR_ARCHITECTURE_INTEL:
			name = "x86";
			break;
		default:
			name = "unknown";
			break;
	}
	return name;
}

PlatformInputDeviceManager& WinRTDevice::GetInputDeviceManager()
{
	return fInputDeviceManager;
}

void WinRTDevice::SetLocationAccuracy( Real meters ) const
{
}

void WinRTDevice::SetLocationThreshold( Real meters ) const
{
}

DeviceOrientation::Type WinRTDevice::GetOrientation() const
{
	auto coronaControl = fEnvironment->CoronaControlAdapter;
	if (nullptr == coronaControl)
	{
		return DeviceOrientation::kUnknown;
	}
	return coronaControl->ParentPageProxy->Orientation->CoronaDeviceOrientation;
}

#pragma endregion

} // namespace Rtt
