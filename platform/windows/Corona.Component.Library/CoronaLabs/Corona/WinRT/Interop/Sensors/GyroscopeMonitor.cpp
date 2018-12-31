// ----------------------------------------------------------------------------
// 
// GyroscopeMonitor.cpp
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#include "pch.h"
#include "GyroscopeMonitor.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Sensors {

#pragma region Constructors/Destructors
GyroscopeMonitor::GyroscopeMonitor()
:	fSensor(nullptr)
{
	// Default the interval to 100 milliseconds. (ie: A sample frequency of 100 Hz.)
	// Note: The TimeSpan.Duration field is set in 100 nanosecond increments.
	fInterval.Duration = 1000000;
}

#pragma endregion


#pragma region Public Methods/Properties
void GyroscopeMonitor::Start()
{
	// Do not continue if already started.
	if (fSensor)
	{
		return;
	}

	// Fetch the sensor, if available.
	try
	{
		fSensor = Windows::Devices::Sensors::Gyrometer::GetDefault();
	}
	catch (...) {}
	if (nullptr == fSensor)
	{
		return;
	}

	// Set up the sensor's sample interval.
	int intervalInMilliseconds = (int)(fInterval.Duration / 10000LL);
	int minIntervalInMilliseconds = (int)fSensor->MinimumReportInterval;
	if (intervalInMilliseconds < minIntervalInMilliseconds)
	{
		intervalInMilliseconds = minIntervalInMilliseconds;
	}
	fSensor->ReportInterval = (unsigned int)intervalInMilliseconds;

	// Notify the system that this sensor monitor is about to start taking measurments.
	this->Starting(this, CoronaLabs::WinRT::EmptyEventArgs::Instance);

	// Start listening for measurement events.
	fReadingChangedEventToken = fSensor->ReadingChanged += ref new Windows::Foundation::TypedEventHandler<Windows::Devices::Sensors::Gyrometer^, Windows::Devices::Sensors::GyrometerReadingChangedEventArgs^>(this, &GyroscopeMonitor::OnReadingChanged);
}

void GyroscopeMonitor::Stop()
{
	// Do not continue if not currently running.
	if (nullptr == fSensor)
	{
		return;
	}

	// Stop listening for measurements.
	if (fReadingChangedEventToken.Value)
	{
		fSensor->ReadingChanged -= fReadingChangedEventToken;
		fReadingChangedEventToken.Value = 0;
	}
	fSensor = nullptr;

	// Notify the system that this sensor monitor has been stopped.
	this->Stopped(this, CoronaLabs::WinRT::EmptyEventArgs::Instance);
}

bool GyroscopeMonitor::IsAvailable::get()
{
	// [Microsoft Bug Warning]
	// The GetDefault() function call below will throw an exception on Windows Phone 8.0 and 8.1 devices
	// that do not have a gyroscope sensor (10.x devices are okay). It'll also trigger a break point
	// if the native debugger is attached, even with the try/catch block handling below.
	// Note: Silverlight's .NET sensor APIs won't trigger an exception and would be a better solution.
	Windows::Devices::Sensors::Gyrometer^ sensorPointer = nullptr;
	try
	{
		sensorPointer = Windows::Devices::Sensors::Gyrometer::GetDefault();
	}
	catch (...) {}
	return (sensorPointer != nullptr);
}

bool GyroscopeMonitor::IsRunning::get()
{
	return (fSensor != nullptr);
}

Windows::Foundation::TimeSpan GyroscopeMonitor::Interval::get()
{
	return fInterval;
}

void GyroscopeMonitor::Interval::set(Windows::Foundation::TimeSpan value)
{
	// Do nothing if the value hasn't changed.
	if (value.Duration == fInterval.Duration)
	{
		return;
	}

	// Store the given interval.
	fInterval.Duration = value.Duration;

	// Update the sensor's sample interval if currently running.
	if (this->IsRunning)
	{
		Stop();
		Start();
	}
}

#pragma endregion


#pragma region Private Methods
void GyroscopeMonitor::OnReadingChanged(
	Windows::Devices::Sensors::Gyrometer^ sender,
	Windows::Devices::Sensors::GyrometerReadingChangedEventArgs^ args)
{
	// Do not continue if this sensor monitor is no longer running.
	if (this->IsRunning == false)
	{
		return;
	}

	// Raise an event notifying this sensor monitor's owner(s) that a measurement was received.
	this->ReceivedAsyncMeasurement(this, args);
}

#pragma endregion

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Sensors
