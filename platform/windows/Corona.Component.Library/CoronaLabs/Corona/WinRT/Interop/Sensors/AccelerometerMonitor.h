//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif

#include "CoronaLabs\WinRT\EmptyEventArgs.h"


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Sensors {

/// <summary>Monitors an accelerometer sensor and provides regular measurements based on a given interval.</summary>
public ref class AccelerometerMonitor sealed
{
	public:
		#pragma region Events
		/// <summary>Raised just before this monitor starts taking sensor measurements.</summary>
		event Windows::Foundation::TypedEventHandler<AccelerometerMonitor^, CoronaLabs::WinRT::EmptyEventArgs^>^ Starting;

		/// <summary>Raised when this monitor stops taking sensor measurements.</summary>
		event Windows::Foundation::TypedEventHandler<AccelerometerMonitor^, CoronaLabs::WinRT::EmptyEventArgs^>^ Stopped;

		/// <summary>
		///  <para>Raised when a measurement has been received from the accelerometer sensor.</para>
		///  <para>This event is raised on the sensor's thread, not on the main UI thread.</para>
		/// </summary>
		event Windows::Foundation::TypedEventHandler<AccelerometerMonitor^, Windows::Devices::Sensors::AccelerometerReadingChangedEventArgs^>^ ReceivedAsyncMeasurement;

		#pragma endregion


		#pragma region Constructors
		/// <summary>Creates a new accelerometer sensor monitor.</summary>
		AccelerometerMonitor();

		#pragma endregion


		#pragma region Public Methods/Properties
		/// <summary>Starts monitoring the accelerometer using the assigned <see cref="Interval"/>.</summary>
		void Start();

		/// <summary>Stops monitor the accelerometer sensor.</summary>
		void Stop();

		/// <summary>Determines if an accelerometer sensor is available on the device/machine.</summary>
		/// <value>
		///  <para>Returns true if the device has an accelerometer to take measurements from.</para>
		///  <para>Returns false if an accelerometer is not available.</para>
		/// </value>
		property bool IsAvailable { bool get(); }

		/// <summary>Determines if this monitor has been started and is actively taking measurements from the sensor.</summary>
		/// <value>
		///  <para>
		///   Returns true if this monitor's Start() method has been called and there is a sensor available
		///   to take measurments from.
		///  </para>
		///  <para>Returns false if this monitor is not currently taking any measurements.</para>
		/// </value>
		property bool IsRunning { bool get(); }

		/// <summary>Gets or sets how often this monitor should take measurements from the sensor.</summary>
		/// <value>The amount of time the monitor should wait before taking its next reading from the sensor.</value>
		property Windows::Foundation::TimeSpan Interval
		{
			Windows::Foundation::TimeSpan get();
			void set(Windows::Foundation::TimeSpan value);
		}

		#pragma endregion

	private:
		#pragma region Private Methods
		/// <summary>Called when the sensor has received a new measurement.</summary>
		/// <param name="sender">The sensor that raised this event.</param>
		/// <param name="args">Provides the measurement data received from the sensor.</param>
		void OnReadingChanged(Windows::Devices::Sensors::Accelerometer^ sender, Windows::Devices::Sensors::AccelerometerReadingChangedEventArgs^ args);

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>The accelerometer sensor to monitor.</summary>
		Windows::Devices::Sensors::Accelerometer^ fSensor;

		/// <summary>The interval determining how often to take measurments from the sensor.</summary>
		Windows::Foundation::TimeSpan fInterval;

		/// <summary>Token received when adding a handler to the sensor's "ReadingChanged" event.</summary>
		Windows::Foundation::EventRegistrationToken fReadingChangedEventToken;

		#pragma endregion
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Sensors
