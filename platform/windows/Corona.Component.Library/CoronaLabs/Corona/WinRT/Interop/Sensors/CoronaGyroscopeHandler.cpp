// ----------------------------------------------------------------------------
// 
// CoronaGyroscopeHandler.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "CoronaGyroscopeHandler.h"
#include "GyroscopeMonitor.h"
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
CoronaGyroscopeHandler::CoronaGyroscopeHandler(CoronaRuntimeEnvironment^ environment)
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
CoronaLabs::Corona::WinRT::Interop::Sensors::GyroscopeMonitor^ CoronaGyroscopeHandler::GyroscopeMonitor::get()
{
	return fSensorMonitor;
}

void CoronaGyroscopeHandler::GyroscopeMonitor::set(CoronaLabs::Corona::WinRT::Interop::Sensors::GyroscopeMonitor^ value)
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
		fMeasurementStartingEventToken = fSensorMonitor->Starting += ref new Windows::Foundation::TypedEventHandler<CoronaLabs::Corona::WinRT::Interop::Sensors::GyroscopeMonitor^, CoronaLabs::WinRT::EmptyEventArgs^>(this, &CoronaGyroscopeHandler::OnMeasurementStarting);
		fMeasurementStoppedEventToken = fSensorMonitor->Stopped += ref new Windows::Foundation::TypedEventHandler<CoronaLabs::Corona::WinRT::Interop::Sensors::GyroscopeMonitor^, CoronaLabs::WinRT::EmptyEventArgs^>(this, &CoronaGyroscopeHandler::OnMeasurementStopped);
		fReceivedAsyncMeasurementEventToken = fSensorMonitor->ReceivedAsyncMeasurement += ref new Windows::Foundation::TypedEventHandler<CoronaLabs::Corona::WinRT::Interop::Sensors::GyroscopeMonitor^, Windows::Devices::Sensors::GyrometerReadingChangedEventArgs^>(this, &CoronaGyroscopeHandler::OnReceivedAsyncMeasurment);
	}
}

#pragma endregion


#pragma region Private Methods
void CoronaGyroscopeHandler::OnMeasurementStarting(
	CoronaLabs::Corona::WinRT::Interop::Sensors::GyroscopeMonitor^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args)
{
	ClearLastRecordedData();
}

void CoronaGyroscopeHandler::OnMeasurementStopped(
	CoronaLabs::Corona::WinRT::Interop::Sensors::GyroscopeMonitor^ sender, CoronaLabs::WinRT::EmptyEventArgs^ args)
{
}

void CoronaGyroscopeHandler::OnReceivedAsyncMeasurment(
	CoronaLabs::Corona::WinRT::Interop::Sensors::GyroscopeMonitor^ sender,
	Windows::Devices::Sensors::GyrometerReadingChangedEventArgs^ args)
{
	// Dispatch the sensor measurement to Corona on the main UI thread.
	auto handler = ref new Windows::UI::Core::DispatchedHandler([this, args]()
	{
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

		// Convert gyroscope data from degrees to radians.
		static const double kDegreesToRadiansScale = (3.1415926535897932384626 / 180.0);
		double xRotationInRadians = args->Reading->AngularVelocityX * kDegreesToRadiansScale;
		double yRotationInRadians = args->Reading->AngularVelocityY * kDegreesToRadiansScale;
		double zRotationInRadians = args->Reading->AngularVelocityZ * kDegreesToRadiansScale;

		// Raise an event in Corona with the above data.
		Rtt::GyroscopeEvent event(xRotationInRadians, yRotationInRadians, zRotationInRadians, deltaTimeInSeconds);
		fEnvironment->NativeRuntimePointer->DispatchEvent(event);
	});
	fDispatcher->InvokeAsync(handler);
}

void CoronaGyroscopeHandler::ClearLastRecordedData()
{
	fLastMeasurementTimestamp.UniversalTime = 0;
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Sensors
