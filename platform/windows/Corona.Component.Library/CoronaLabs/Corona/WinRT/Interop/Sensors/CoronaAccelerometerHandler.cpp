//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "CoronaAccelerometerHandler.h"
#include "AccelerometerMonitor.h"
#include "CoronaLabs\Corona\WinRT\CoronaRuntimeEnvironment.h"
#include "CoronaLabs\Corona\WinRT\Interop\UI\ICoronaControlAdapter.h"
#include "CoronaLabs\Corona\WinRT\Interop\UI\IDispatcher.h"
#include "CoronaLabs\WinRT\EmptyEventArgs.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_BEGIN
#	include "Core\Rtt_Build.h"
#	include "Rtt_Event.h"
#	include "Rtt_Runtime.h"
Rtt_DISABLE_WIN_XML_COMMENT_COMPILER_WARNINGS_END


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Sensors {

#pragma region Constructors/Destructors
CoronaAccelerometerHandler::CoronaAccelerometerHandler(CoronaRuntimeEnvironment^ environment)
:	fEnvironment(environment),
	fDispatcher(nullptr),
	fSensorMonitor(nullptr)
{
	// Validate.
	if (nullptr == environment)
	{
		throw ref new Platform::NullReferenceException(L"environment");
	}

	// Fetch the dispatcher needed to synchronize back onto the main UI thread.
	if (environment->CoronaControlAdapter)
	{
		fDispatcher = environment->CoronaControlAdapter->Dispatcher;
	}

	// Initialize measurement member variables.
	ClearLastRecordedData();
}

#pragma endregion


#pragma region Public Methods/Properties
CoronaLabs::Corona::WinRT::Interop::Sensors::AccelerometerMonitor^ CoronaAccelerometerHandler::AccelerometerMonitor::get()
{
	return fSensorMonitor;
}

void CoronaAccelerometerHandler::AccelerometerMonitor::set(
	CoronaLabs::Corona::WinRT::Interop::Sensors::AccelerometerMonitor^ value)
{
	// Do not continue if the reference isn't changing.
	if (value == fSensorMonitor)
	{
		return;
	}

	// Handle the last sensor monitor.
	if (fSensorMonitor)
	{
		// Remove event handlers.
		if (fMeasurementStartingEventToken.Value)
		{
			fSensorMonitor->Starting -= fMeasurementStartingEventToken;
			fMeasurementStartingEventToken.Value = 0;
		}
		if (fMeasurementStoppedEventToken.Value)
		{
			fSensorMonitor->Stopped -= fMeasurementStoppedEventToken;
			fMeasurementStoppedEventToken.Value = 0;
		}
		if (fReceivedAsyncMeasurementEventToken.Value)
		{
			fSensorMonitor->ReceivedAsyncMeasurement -= fReceivedAsyncMeasurementEventToken;
			fReceivedAsyncMeasurementEventToken.Value = 0;
		}
	}

	// Store the sensor monitor reference.
	fSensorMonitor = value;

	// Handle the new sensor monitor, but only if we have a dispatcher so that we re-sync with the main UI thread.
	if (fSensorMonitor && fDispatcher)
	{
		// Reset the last recorded data.
		ClearLastRecordedData();

		// Add event handlers.
		fMeasurementStartingEventToken = fSensorMonitor->Starting += ref new Windows::Foundation::TypedEventHandler<CoronaLabs::Corona::WinRT::Interop::Sensors::AccelerometerMonitor^, CoronaLabs::WinRT::EmptyEventArgs^>(this, &CoronaAccelerometerHandler::OnMeasurementStarting);
		fMeasurementStoppedEventToken = fSensorMonitor->Stopped += ref new Windows::Foundation::TypedEventHandler<CoronaLabs::Corona::WinRT::Interop::Sensors::AccelerometerMonitor^, CoronaLabs::WinRT::EmptyEventArgs^>(this, &CoronaAccelerometerHandler::OnMeasurementStopped);
		fReceivedAsyncMeasurementEventToken = fSensorMonitor->ReceivedAsyncMeasurement += ref new Windows::Foundation::TypedEventHandler<CoronaLabs::Corona::WinRT::Interop::Sensors::AccelerometerMonitor^, Windows::Devices::Sensors::AccelerometerReadingChangedEventArgs^>(this, &CoronaAccelerometerHandler::OnReceivedAsyncMeasurment);
	}
}

#pragma endregion


#pragma region Private Methods
void CoronaAccelerometerHandler::OnMeasurementStarting(
	CoronaLabs::Corona::WinRT::Interop::Sensors::AccelerometerMonitor^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args)
{
	ClearLastRecordedData();
}

void CoronaAccelerometerHandler::OnMeasurementStopped(
	CoronaLabs::Corona::WinRT::Interop::Sensors::AccelerometerMonitor^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args)
{
}

void CoronaAccelerometerHandler::OnReceivedAsyncMeasurment(
	CoronaLabs::Corona::WinRT::Interop::Sensors::AccelerometerMonitor^ sender,
	Windows::Devices::Sensors::AccelerometerReadingChangedEventArgs^ args)
{
	// Dispatch the sensor measurement to Corona on the main UI thread.
	auto handler = ref new Windows::UI::Core::DispatchedHandler([this, args]()
	{
		const double kFilteringFactor = 0.1;

		// Do not continue if the Corona runtime has been terminated.
		if (nullptr == fEnvironment->NativeRuntimePointer)
		{
			return;
		}

		// Ignore this measurement if the sensor was just shut off.
		if ((nullptr == fSensorMonitor) || (fSensorMonitor->IsRunning == false))
		{
			return;
		}

		// Fetch the raw sensor data.
		double rawData[3];
		rawData[0] = args->Reading->AccelerationX;
		rawData[1] = args->Reading->AccelerationY;
		rawData[2] = args->Reading->AccelerationZ;

		// Calculate a simple high-pass and low-pass filtered data.
		double highPassFilteredData[3];
		double highPassFilteredMagnitude = 0;
		for (int index = 0; index < 3; index++)
		{
			fLastLowPassFilteredData[index] =
					(rawData[index] * kFilteringFactor) + (fLastLowPassFilteredData[index] * (1.0 - kFilteringFactor));
			highPassFilteredData[index] = rawData[index] - fLastLowPassFilteredData[index];
			highPassFilteredMagnitude += highPassFilteredData[index] + highPassFilteredData[index];
		}

		// Do not continue if this is the first measurement.
		// We need the first measurement's timestamp in order to calculate delta time below.
		bool isFirstMeasurement = (0 == fLastMeasurementTimestamp.UniversalTime);
		if (isFirstMeasurement)
		{
			fLastMeasurementTimestamp = args->Reading->Timestamp;
			return;
		}

		// Calculate how long it has been since the last measurment.
		auto deltaUniversalTime = args->Reading->Timestamp.UniversalTime - fLastMeasurementTimestamp.UniversalTime;
		double deltaTimeInSeconds = deltaUniversalTime / 10000000.0;
		if (deltaTimeInSeconds <= 0)
		{
			deltaTimeInSeconds = 1.0;
		}
		fLastMeasurementTimestamp = args->Reading->Timestamp;

		// Determine if a shake event occurred.
		// This is based on the magnitude exceeding a set threshold within a 500 millisecond period.
		bool wasShakened = false;
		if (highPassFilteredMagnitude >= 4.0)
		{
			if (fLastMeasurementTimestamp.UniversalTime > (fLastShakeTimestamp.UniversalTime + 5000000LL))
			{
				wasShakened = true;
				fLastShakeTimestamp = fLastMeasurementTimestamp;
			}
		}

		// Raise an event in Corona with the above data.
		Rtt::AccelerometerEvent event(
				fLastLowPassFilteredData, highPassFilteredData, rawData, wasShakened, deltaTimeInSeconds);
		fEnvironment->NativeRuntimePointer->DispatchEvent(event);
	});
	fDispatcher->InvokeAsync(handler);
}

void CoronaAccelerometerHandler::ClearLastRecordedData()
{
	fLastLowPassFilteredData[0] = 0;
	fLastLowPassFilteredData[1] = 0;
	fLastLowPassFilteredData[2] = 0;
	fLastMeasurementTimestamp.UniversalTime = 0;
	fLastShakeTimestamp.UniversalTime = 0;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Sensors
