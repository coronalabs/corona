// ----------------------------------------------------------------------------
// 
// CoronaAccelerometerHandler.h
// Copyright (c) 2014 Corona Labs Inc. All rights reserved.
// 
// Reviewers:
// 		Joshua Quick
//
// ----------------------------------------------------------------------------

#pragma once

#ifndef CORONALABS_CORONA_API_EXPORT
#	error This header file cannot be included by an external library.
#endif


#pragma region Forward Declarations
namespace CoronaLabs { namespace Corona { namespace WinRT {
	namespace Interop { namespace UI {
		interface class IDispatcher;
	} }
	ref class CoronaRuntimeEnvironment;
} } }
namespace CoronaLabs { namespace WinRT {
	ref class EmptyEventArgs;
} }
#pragma endregion


namespace CoronaLabs { namespace Corona { namespace WinRT { namespace Interop { namespace Sensors {

ref class AccelerometerMonitor;

/// <summary>Receives measurements from an AccelerometerMonitor and dispatches them to Corona as Lua events.</summary>
[Windows::Foundation::Metadata::WebHostHidden]
public ref class CoronaAccelerometerHandler sealed
{
	public:
		#pragma region Constructors
		/// <summary>Creates a new accelerometer handler associated with the given Corona runtime.</summary>
		/// <param name="environment">
		///  <para>The Corona runtime this handler will dispatch sensor measurments to.</para>
		///  <para>
		///   The given runtime must provide a Corona control so that this handler can use its dispatcher
		///   to re-sync with the main UI thread.
		///  </para>
		///  <para>The given runtime reference cannot be null or else an exception will be thrown.</para>
		/// </param>
		CoronaAccelerometerHandler(CoronaRuntimeEnvironment^ environment);

		#pragma endregion


		#pragma region Public Methods/Properties
		/// <summary>Gets or sets the accelerometer monitor this handler will collect measurements from.</summary>
		/// <value>
		///  <para>Reference to the sensor monitor this handler will collect measurements from.</para>
		///  <para>Set to null to have this handler stop listening for measurements.</para>
		/// </value>
		property CoronaLabs::Corona::WinRT::Interop::Sensors::AccelerometerMonitor^ AccelerometerMonitor
		{
			CoronaLabs::Corona::WinRT::Interop::Sensors::AccelerometerMonitor^ get();
			void set(CoronaLabs::Corona::WinRT::Interop::Sensors::AccelerometerMonitor^ value);
		}

		#pragma endregion

	private:
		#pragma region Private Methods
		/// <summary>Called when the given sensor monitor is about to start taking measurements.</summary>
		/// <param name="sender">The sensor monitor that raised this event.</param>
		/// <param name="args">Empty event arguments.</param>
		void OnMeasurementStarting(
				CoronaLabs::Corona::WinRT::Interop::Sensors::AccelerometerMonitor^ sender,
				CoronaLabs::WinRT::EmptyEventArgs^ args);

		/// <summary>Called when the given sensor monitor has stopped taking measurements.</summary>
		/// <param name="sender">The sensor monitor that raised this event.</param>
		/// <param name="args">Empty event arguments.</param>
		void OnMeasurementStopped(
			CoronaLabs::Corona::WinRT::Interop::Sensors::AccelerometerMonitor^ sender,
			CoronaLabs::WinRT::EmptyEventArgs^ args);

		/// <summary>Called from the sensor's worker thread when a new measurement has been received.</summary>
		/// <param name="sender">The sensor monitor that raised this event.</param>
		/// <param name="args">The accelerometer data that was just received.</param>
		void OnReceivedAsyncMeasurment(
				CoronaLabs::Corona::WinRT::Interop::Sensors::AccelerometerMonitor^ sender,
				Windows::Devices::Sensors::AccelerometerReadingChangedEventArgs^ args);

		/// <summary>
		///  <para>Clears the last received measurement values and last recorded timestamp.</para>
		///  <para>
		///   Expected to be called when the sensor monitor has been restarted or a new sensor monitor has been received.
		///  </para>
		/// </summary>
		void ClearLastRecordedData();

		#pragma endregion


		#pragma region Private Member Variables
		/// <summary>The Corona runtime to dispatch measurements to.</summary>
		CoronaRuntimeEnvironment^ fEnvironment;

		/// <summary>
		///  <para>Reference to a UI dispatcher used to re-sync with the main UI thread.</para>
		///  <para>
		///   This is taken from the runtime's Corona control. We keep our own reference to ensure it is not
		///   garbage collected when the runtime gets terminated.
		///  </para>
		/// </summary>
		Interop::UI::IDispatcher^ fDispatcher;

		/// <summary>The sensor monitor to collect measurments from.</summary>
		CoronaLabs::Corona::WinRT::Interop::Sensors::AccelerometerMonitor^ fSensorMonitor;

		/// <summary>
		///  <para>Stores the last sensor measurement with a low pass filter applied.</para>
		///  <para>Elements (0,1,2) are accelerometer values (x,y,z) relative to the screen in portrait orientation.</para>
		/// </summary>
		double fLastLowPassFilteredData[3];

		/// <summary>Stores the timestamp of the last received measurement.</summary>
		Windows::Foundation::DateTime fLastMeasurementTimestamp;

		/// <summary>Timestamp of the last measurement used to determine if a shake event occurred.</summary>
		Windows::Foundation::DateTime fLastShakeTimestamp;

		/// <summary>Token received when adding a handler to the monitor's "Starting" event.</summary>
		Windows::Foundation::EventRegistrationToken fMeasurementStartingEventToken;

		/// <summary>Token received when adding a handler to the monitor's "Stopped" event.</summary>
		Windows::Foundation::EventRegistrationToken fMeasurementStoppedEventToken;

		/// <summary>Token received when adding a handler to the "ReceivedAsyncMeasurement" event.</summary>
		Windows::Foundation::EventRegistrationToken fReceivedAsyncMeasurementEventToken;

		#pragma endregion
};

} } } } }	// namespace CoronaLabs::Corona::WinRT::Interop::Sensors
