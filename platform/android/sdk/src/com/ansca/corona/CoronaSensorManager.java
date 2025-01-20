//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;

import android.app.Activity;
import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.location.Location;
import android.location.LocationListener;
import android.location.LocationManager;
import android.os.Bundle;
import android.os.SystemClock;
import android.util.Log;

import com.ansca.corona.permissions.PermissionsServices;
import com.ansca.corona.permissions.PermissionsSettings;
import com.ansca.corona.permissions.PermissionState;


class CoronaSensorManager {
    // http://code.google.com/android/reference/android/hardware/SensorManager.html#SENSOR_ACCELEROMETER
    // for an explanation on the values reported by SENSOR_ACCELEROMETER.

	private boolean					myActiveSensors[] = new boolean[JavaToNativeShim.EventTypeNumTypes];
	private AccelerometerMonitor	myAccelerometerMonitor;
	private GyroscopeMonitor		myGyroscopeMonitor;
	private Sensor					myOrientationSensor;
	private SensorEventListener		myOrientationSensorListener;

	private CoronaLocationListener  myLocationListener;
	private PermissionsSettings     myLocationPermissionsSettings;
	
    private SensorManager 			mySensorManager;
    private LocationManager			myLocationManager;

    private double					myLocationThreshold = 0;
    private float					myLastHeading = -1.0f;

    private CoronaRuntime 			myCoronaRuntime;

	/**
	 * Constructor
	 */
	CoronaSensorManager(CoronaRuntime runtime) {
		myCoronaRuntime = runtime;
	}
	
	/**
	 * Initialize the manager
	 */
	public void init() {

		// Validate environment.
		if (myCoronaRuntime == null) {
			return;
		}
		Controller controller = myCoronaRuntime.getController();
		if (controller == null) {
			return;
		}
		Context context = controller.getContext();
		if (context == null) {
			return;
		}
		
        mySensorManager = (SensorManager)context.getSystemService(Context.SENSOR_SERVICE);
        myLocationManager = (LocationManager)context.getSystemService(Context.LOCATION_SERVICE);    
		myAccelerometerMonitor = new AccelerometerMonitor();
		myGyroscopeMonitor = new GyroscopeMonitor();
		
        for ( int i = 0; i < myActiveSensors.length; i++ )
        	myActiveSensors[i] = false;
	}
	
	/**
	 * Enable an event notification type
	 * 
	 * @param eventType		Type of event
	 * @param enable		True to enable, false to disable
	 */
	public void setEventNotification( int eventType, boolean enable ) {
		if ( enable )
			start( eventType );
		else
			stop( eventType );
	}

	private android.os.Handler getHandlerSafely() {
		// Validate.
		if (myCoronaRuntime == null) {
			return null;
		}
		Controller controller = myCoronaRuntime.getController();
		if (controller == null) {
			return null;
		}

		// Safely get the handler
		return controller.getHandler();
	}

	/**
	 * Start event notifications for type
	 * 
	 * @param eventType		Type of event
	 */
	private void startType( final int eventType ) {
		// Safely grab handler.
		android.os.Handler handler = getHandlerSafely();
		
		if (handler == null) {
			return;
		}

		// Run this on the main thread.
		handler.post(new Runnable() {
			@Override
			public void run() {
				switch ( eventType ) {
				case JavaToNativeShim.EventTypeMultitouch:
					// Multitouch actually isn't handled in the sensor manager, but in the activity onTouch.
					// But we'll use the sensor manager to determine if it's on or off.
					break;
					
				case JavaToNativeShim.EventTypeOrientation:
					// TODO: unimplemented
					break;
					
				case JavaToNativeShim.EventTypeAccelerometer:
					myAccelerometerMonitor.start();
					break;
					
				case JavaToNativeShim.EventTypeGyroscope:
					myGyroscopeMonitor.start();
					break;
						
				case JavaToNativeShim.EventTypeHeading:
					// Confusingly, Android refers to Compass as Orientation, because it also includes pitch and roll. 
					// But for Corona Heading, all we want is azimuth.
					myOrientationSensor = mySensorManager.getDefaultSensor( Sensor.TYPE_ORIENTATION );
					myOrientationSensorListener = new SensorEventListener() {
						@Override
						public void onSensorChanged(SensorEvent event) {
							synchronized (this)
							{
								// Do not continue if application is exiting.
								Controller controller = myCoronaRuntime.getController();
								if (controller == null) {
									return;
								}
								
								// All values are angles in degrees.
								// values[0]: Azimuth, angle between the magnetic north direction and the Y axis, around the Z axis (0 to 359).
								//            0 = North, 90 = East, 180 = South, 270 = West
								// values[1]: Pitch, rotation around X axis (-180 to 180), with positive values when the z-axis moves toward the y-axis.
								// values[2]: Roll, rotation around Y axis (-90 to 90), with positive values when the x-axis moves away from the z-axis.
								float currentHeading = event.values[0];
								
								// Provide azimuth data relative to portrait orientation.
								// That means devices that are naturally landscape such as tablets need to have this value rotated.
								if (controller.isNaturalOrientationPortrait() == false) {
									currentHeading -= 90.0f;
									if (currentHeading < 0) {
										currentHeading += 360.0f;
									}
								}
								
								// Only raise an event if the heading has changed.
								if (myLastHeading != currentHeading) {
									myLastHeading = currentHeading;
									if (myCoronaRuntime != null && myCoronaRuntime.isRunning()) {
										myCoronaRuntime.getTaskDispatcher().send(new com.ansca.corona.events.LocationTask(myLastHeading));
									}
								}
							}
						}
						
						@Override
						public void onAccuracyChanged(Sensor arg0, int arg1) {
						}
					};
					
					mySensorManager.registerListener(myOrientationSensorListener, myOrientationSensor, SensorManager.SENSOR_DELAY_UI);
					break;
		
				case JavaToNativeShim.EventTypeLocation:
					// TODO: Redo all of how permissions and checking for location hardware is handled here!
					// Do not continue if application is exiting.
					Controller controller = myCoronaRuntime.getController();
					Context context = null;
					if (controller != null) {
						context = controller.getContext();
					}
					if (context == null || controller == null) {
						return;
					}

					// Do not continue if this device is unable to provide location data.
					android.content.pm.PackageManager packageManager = context.getPackageManager();
					boolean hasGps = packageManager.hasSystemFeature(android.content.pm.PackageManager.FEATURE_LOCATION_GPS);
					boolean hasNetworkLocater = packageManager.hasSystemFeature(android.content.pm.PackageManager.FEATURE_LOCATION_NETWORK);
					if (!hasGps && !hasNetworkLocater) {
						Log.i("Corona", "WARNING: Unable to set up location listener. This device is incapable of providing location data.");
						return;
					}
					
					// Enable location tracking.
					Location lastLocation = null;
					final long minTime = 1000;
					boolean isLocationListenerEnabled = false;
					myLocationListener = new CoronaLocationListener();
					if (hasGps && (context.checkCallingOrSelfPermission(android.Manifest.permission.ACCESS_FINE_LOCATION) ==
								   android.content.pm.PackageManager.PERMISSION_GRANTED)) {
						myLocationListener.setSupportsGps();

						myLocationManager.requestLocationUpdates(
									LocationManager.GPS_PROVIDER, minTime, (float)myLocationThreshold, myLocationListener);
						isLocationListenerEnabled = true;

						lastLocation = myLocationManager.getLastKnownLocation(LocationManager.GPS_PROVIDER);
					}
					if (hasNetworkLocater && (context.checkCallingOrSelfPermission(android.Manifest.permission.ACCESS_COARSE_LOCATION) ==
												   android.content.pm.PackageManager.PERMISSION_GRANTED)) {
						myLocationListener.setSupportsNetwork();

						myLocationManager.requestLocationUpdates(
									LocationManager.NETWORK_PROVIDER, minTime, (float)myLocationThreshold, myLocationListener);
						isLocationListenerEnabled = true;

						if (lastLocation == null) {
							lastLocation = myLocationManager.getLastKnownLocation(LocationManager.NETWORK_PROVIDER);
						}
					}
					if (context.checkCallingOrSelfPermission(android.Manifest.permission.ACCESS_FINE_LOCATION) == android.content.pm.PackageManager.PERMISSION_GRANTED) {
						myLocationManager.requestLocationUpdates(
									LocationManager.PASSIVE_PROVIDER, minTime, (float)myLocationThreshold, myLocationListener);
						isLocationListenerEnabled = true;

						if (lastLocation == null) {
							lastLocation = myLocationManager.getLastKnownLocation(LocationManager.PASSIVE_PROVIDER);
						}
					}

					// Dispatches the last known location.  If the user stays at the same place, the location listener's onLocationChanged function doesn't get called
					// which makes it look like location services aren't working.
					if (myCoronaRuntime != null && myCoronaRuntime.isRunning() && lastLocation != null) {
						myCoronaRuntime.getTaskDispatcher().send(new com.ansca.corona.events.LocationTask(
								lastLocation.getLatitude(),
								lastLocation.getLongitude(),
								lastLocation.getAltitude(),
								(double)lastLocation.getAccuracy(),
								(double)lastLocation.getSpeed(),
								(double)lastLocation.getBearing(),
								(double)lastLocation.getTime() / 1000.0));	// Convert milliseconds to seconds.);
					}

					if (isLocationListenerEnabled == false) {
						// Request access to location services.
						PermissionsServices permissionsServices = new PermissionsServices(CoronaEnvironment.getApplicationContext());
						PermissionState fineLocationState = permissionsServices.getPermissionStateFor(
							PermissionsServices.Permission.ACCESS_FINE_LOCATION);
						PermissionState coarseLocationState = permissionsServices.getPermissionStateFor(
							PermissionsServices.Permission.ACCESS_COARSE_LOCATION);
						if (fineLocationState == PermissionState.MISSING && coarseLocationState == PermissionState.MISSING) {
							showLocationPermissionsMissingFromManifestAlert();
						} else if (fineLocationState == PermissionState.GRANTED || coarseLocationState == PermissionState.GRANTED) {
							// We shouldn't hit this code since the granted case is handled when checking for location hardware.
							// Log.d("Corona", "A Corona Engineer busted the Location enabling code!");
						} else if (android.os.Build.VERSION.SDK_INT >= 23) { // There's at least one permission that's denied, so request all the denied ones!
							if (myLocationPermissionsSettings == null) {
								myLocationPermissionsSettings = new PermissionsSettings(permissionsServices.getRequestedPermissionsInStateForGroup(
									PermissionState.DENIED, PermissionsServices.PermissionGroup.LOCATION));
							}
							if (myLocationPermissionsSettings.needsService()) { // TODO: Handle "Never Ask Again" here!
								permissionsServices.requestPermissions(myLocationPermissionsSettings, new CoronaSensorManager.LocationRequestPermissionsResultHandler());
							}
						} else {
							// Log.d("Corona", "Should never hit this!");
						}
					}
					
					break;
				} // switch
			} // run
		} ); // runnable
	}
	
	private void showLocationPermissionsMissingFromManifestAlert() {

		Controller controller = myCoronaRuntime.getController();
		if (controller != null) {
			controller.showLocationPermissionsMissingFromManifestAlert();
		}
	}
	/** Stores a copy of SensorEvent data. */
	private class SensorMeasurement {
		public int accuracy;
		public android.hardware.Sensor sensor;
		public long timestamp;
		public float[] values;
		
		public SensorMeasurement() {
			accuracy = 0;
			sensor = null;
			timestamp = 0;
			values = null;
		}
		
		public void copyFrom(SensorEvent event) {
			if (event == null) {
				return;
			}
			
			accuracy = event.accuracy;
			sensor = event.sensor;
			timestamp = event.timestamp;
			
			if (values == null) {
				values = new float[event.values.length];
			}
			for (int index = 0; (index < values.length) && (index < event.values.length); index++) {
				values[index] = event.values[index];
			}
		}
	}
	
	/** Base class used to sample measurements from a sensor periodically. */
	private abstract class SensorMonitor {
		/** Assignd to a SensorManager object to receive measurements from a sensor. */
		private SensorEventListener fSensorListener;
		
		/**
		 * Timer used to control when sensor measurments are to be sent to the EventManager based on the given
		 * interval in hertz. This is needed because Android does not allow you to set exact sample times.
		 */
		private MessageBasedTimer fTimer;
		
		/** Set true if the sensor monitor has been started. Set false if stopped. */
		private boolean fIsRunning;
		
		
		/** Creates a new sensor monitor object. */
		public SensorMonitor() {
			// Initialize member variables.
			fSensorListener = null;
			fTimer = new MessageBasedTimer();
			fIsRunning = false;
			
			// Set the sample interval to the default.
			setIntervalInHertz(10);
		}
		
		/**
		 * Sets the listener used to receive measurements from the sensor.
		 * This listener must be assigned by the derived class before calling the start() method.
		 * @param listener The listener that will receive sensor measurements. Cannot be null.
		 */
		protected void setSensorListener(SensorEventListener listener) {
			// Validate argument.
			if (listener == null) {
				throw new NullPointerException();
			}
			
			// Do not continue if listener hasn't changed.
			if (listener == fSensorListener) {
				return;
			}
			
			// Stop sensor monitoring if it is currently running because the old listener is no longer valid.
			boolean wasRunning = isRunning();
			if (wasRunning) {
				stop();
			}
			
			// Store the given listener.
			fSensorListener = listener;
			
			// Restart sensor monitoring if it was running before to use the new listener.
			if (wasRunning) {
				start();
			}
		}
		
		/**
		 * Sets the listener that will receive "onTimerElapsed" events used to control sample timing.
		 * To be called by the derived class before the start() method is called.
		 * <p>
		 * This listener is optional. Not setting it will prevent the timer from being used, in which case
		 * the derived sensor monitor is completely dependent on the sensor listener to record measurements.
		 * @param listener The listener that will receive "onTimerElapsed" events. Set to null to stop receiving timer events.
		 */
		protected void setTimerListener(MessageBasedTimer.Listener listener) {
			fTimer.setListener(listener);
		}
		
		/**
		 * Sets the sensor sample interval in hertz.
		 * It is okay to set the interval while the sensor monitor is running.
		 * @param value The interval in hertz, which is the number of samples per second.
		 */
		public void setIntervalInHertz(int value) {
			// Convert hertz to time.
			TimeSpan interval = TimeSpan.fromMilliseconds(1000 / value);
			
			// Do not continue if the interval has not changed.
			if (fTimer.getInterval().equals(interval)) {
				return;
			}
			
			// Stop sensor monitoring if it is currently running.
			boolean wasRunning = isRunning();
			if (wasRunning) {
				stop();
			}
			
			// Update the interval.
			fTimer.setInterval(interval);
			
			// Restart the sensor monitor if it was running before to use the new interval.
			if (wasRunning) {
				start();
			}
		}
		
		/**
		 * Gets the sensor sample interval in hertz.
		 * @return Returns the sample interval in hertz, which is the number of samples per second.
		 */
		public int getIntervalInHertz() {
			return (int)(1000L / fTimer.getInterval().getTotalMilliseconds());
		}
		
		/**
		 * Gets the sensor sample interval.
		 * @return Returns the sample interval.
		 */
		public TimeSpan getInterval() {
			return fTimer.getInterval();
		}
		
		/**
		 * Gets the type of sensor that is being monitored such as "Sensor.TYPE_ACCELEROMETER".
		 * @return Returns a unique integer ID matching a sensor type constant provided by Android class "android.hardware.Sensor".
		 */
		public abstract int getSensorType();
		
		/**
		 * Determines if the sensor monitor is running and receiving measurements.
		 * @return Returns true if the sensor monitor is running. Returns false if not.
		 */
		public boolean isRunning() {
			return fIsRunning;
		}
		
		/** Starts monitoring the sensor and providing measurements to the system. */
		public void start() {
			// Do not continue if already started.
			if (isRunning()) {
				return;
			}
			
			// Start monitoring the sensor.
			try {
				// Notify the derived class that the monitor is starting.
				onStarting();
				
				// Register the sensor listener. This starts the monitor.
				android.hardware.SensorManager sensorManager;
				sensorManager = (android.hardware.SensorManager)CoronaEnvironment.getApplicationContext().getSystemService(
										android.content.Context.SENSOR_SERVICE);
				android.hardware.Sensor sensor = sensorManager.getDefaultSensor(getSensorType());
				sensorManager.registerListener(fSensorListener, sensor, getSensorDelay());
				fIsRunning = true;
				
				// Set up and start the timer, but only if a timer listener has been provided.
				// If a timer listener was not provided, then data will only be received via the sensor listener.
				if (fTimer.getListener() != null) {
					if (myCoronaRuntime.getController().getHandler() != null) {
						fTimer.setHandler(myCoronaRuntime.getController().getHandler());
					}
					fTimer.start();
				}
			}
			catch (Exception ex) {
				ex.printStackTrace();
			}
		}
		
		/** Stops monitoring the sensor. Should be called before closing its associated activity. */
		public void stop() {
			// Do not continue if already stopped.
			if (isRunning() == false) {
				return;
			}
			
			// Stop monitoring the sensor.
			try {
				android.hardware.SensorManager sensorManager;
				sensorManager = (android.hardware.SensorManager)CoronaEnvironment.getApplicationContext().getSystemService(
										android.content.Context.SENSOR_SERVICE);
				sensorManager.unregisterListener(fSensorListener);
				fTimer.stop();
				fIsRunning = false;
				onStopped();
			}
			catch (Exception ex) {
				ex.printStackTrace();
			}
		}
		
		/** Called just before this object starts monitoring the sensor. To be overriden by derived classes. */
		protected void onStarting() { }
		
		/** Called just after sensor monitoring has been stopped. To be overriden by derived classes. */
		protected void onStopped() { }
		
		/**
		 * Gets an Android sensor delay constant matching the currently configured sample interval.
		 * Needed by the Android SensorManager class when setting up a sensor listener.
		 * @return Returns a unique integer ID matching a "SENSOR_DELAY_*" constant in Android class "android.hardware.SensorManager".
		 */
		private int getSensorDelay() {
			long intervalInMilliseconds;
			int delayType;
			
			intervalInMilliseconds = fTimer.getInterval().getTotalMilliseconds();
			if (intervalInMilliseconds >= 200) {
				delayType = android.hardware.SensorManager.SENSOR_DELAY_NORMAL;
			}
			else if (intervalInMilliseconds >= 60) {
				delayType = android.hardware.SensorManager.SENSOR_DELAY_UI;
			}
			else if (intervalInMilliseconds >= 20) {
				delayType = android.hardware.SensorManager.SENSOR_DELAY_GAME;
			}
			else {
				delayType = android.hardware.SensorManager.SENSOR_DELAY_FASTEST;
			}
			return delayType;
		}
	}
	
	/** Monitors the accelerometer sensor, takes measurements periodically, and then posts that data to the Corona EventManager. */
	private class AccelerometerMonitor extends SensorMonitor {
		/** Stores a copy of the last received sensor measurement. */
		private SensorMeasurement fLastSensorMeasurement;
		
		/** Set true if the first sensor measurement has been skipped. */
		private boolean fHasSkippedFirstMeasurement;
		
		/** Set true if member variable "fLastSensorMeasurement" has received a measurement. */
		private boolean fHasReceivedMeasurement;
		
		/** Set true after the first sensor sample has been accepted by the timer listener. */
		private boolean fHasReceivedSample;
		
		/**
		 * Set to the "sensor timestamp" value belonging to the last sample sent to the Corona EventManager.
		 * Needed to calculate delta time between samples.
		 */
		private long fLastSampleTimestamp;
		
		
		/** Creates a new accelerometer sensor monitor. */
		public AccelerometerMonitor() {
			super();
			
			// Initialize member variables.
			fLastSensorMeasurement = new SensorMeasurement();
			fLastSensorMeasurement.values = new float[] { 0.0f, 0.0f, 0.0f };
			fHasSkippedFirstMeasurement = false;
			fHasReceivedMeasurement = false;
			fHasReceivedSample = false;
			fLastSampleTimestamp = 0;
			
			// Set up the listeners.
			setSensorListener(new AccelerometerMonitor.SensorHandler());
			setTimerListener(new AccelerometerMonitor.TimerHandler());
		}
		
		/**
		 * Gets the unique Android sensor type ID for an accelerometer, to be used by an Android SensorManager object.
		 * @return Returns the unique integer ID identifying an accelerometer.
		 */
		@Override
		public int getSensorType() {
			return android.hardware.Sensor.TYPE_ACCELEROMETER;
		}
		
		/** Called when the sensor monitor is about to be started. */
		@Override
		protected void onStarting() {
			fHasSkippedFirstMeasurement = false;
			fHasReceivedMeasurement = false;
			fHasReceivedSample = false;
		}
		
		/** Private class used to receive data from an Android sensor. */
		private class SensorHandler implements SensorEventListener {
			@Override
			public void onSensorChanged(SensorEvent event) {
				// Validate.
				if (event == null) {
					return;
				}
				
				// Throw away the first measurement since it is typically taken before the start() method was called.
				// This causes the final recorded sample to have a much larger delta time than expected.
				if (fHasSkippedFirstMeasurement == false) {
					fHasSkippedFirstMeasurement = true;
					return;
				}
				
				// Make a copy of the received sensor event data to be eventually analyzed by the TimerHandler.
				// Note: We do not want to hold a reference to the given sensor event object because Android will
				//       eventually re-use and overwrite it for efficiency.
				fLastSensorMeasurement.copyFrom(event);
				fHasReceivedMeasurement = true;
			}
			
			@Override
			public void onAccuracyChanged(Sensor sensor, int accuracy) { }
		}
		
		/** Private class that receives events from a timer and post sensor measurements to the Corona EventManager. */
		private class TimerHandler implements MessageBasedTimer.Listener {
			@Override
			public void onTimerElapsed() {
				// Fetch the controller object.
				Controller controller = myCoronaRuntime.getController();
				if (controller == null) {
					return;
				}
				
				// Do not continue if a measurement has not been received yet.
				if (fHasReceivedMeasurement == false) {
					return;
				}
				
				// Skip the first sample and record its timestamp.
				// This initial timestamp is needed to compute delta time.
				if (fHasReceivedSample == false) {
					fLastSampleTimestamp = fLastSensorMeasurement.timestamp;
					fHasReceivedSample = true;
					return;
				}
				
				// Check if the following has occurred:
				// 1) If we have not received a new measurement, then duplicate the last received measurement.
				//    This typically happens on devices that do not provide measurements at regular intervals, such as the Kindle Fire.
				// 2) If the timestamp is too close to the last sample's timestamp (within half an interval), then
				//    bump up the timestamp to prevent a really small delta time from being calculated down below.
				long minSensorTimestamp = fLastSampleTimestamp + ((getInterval().getTotalMilliseconds() * 1000000L) / 2L);
				if (compareSensorTimestamps(fLastSensorMeasurement.timestamp, minSensorTimestamp) <= 0) {
					fLastSensorMeasurement.timestamp = fLastSampleTimestamp + (getInterval().getTotalMilliseconds() * 1000000L);
				}
				
				// Calculate delta time. Convert from nanoseconds to seconds.
				long deltaTimeInNanoseconds = subtractSensorTimestamps(fLastSensorMeasurement.timestamp, fLastSampleTimestamp);
				double deltaTimeInFractionalSeconds = (double)deltaTimeInNanoseconds * 0.000000001;
				fLastSampleTimestamp = fLastSensorMeasurement.timestamp;
				
				// Accelerometer values are 10x what they are on iPhone; normalize them.
				// Also, always provide values relative to portrait orientation.
				// This means we have to flip the x and y values on devices that are naturally landscape, such as tablets.
				boolean isNaturalOrientationPortrait = controller.isNaturalOrientationPortrait();
				int xIndex = isNaturalOrientationPortrait ? 0 : 1;
				int yIndex = isNaturalOrientationPortrait ? 1 : 0;
				double xValue = - fLastSensorMeasurement.values[xIndex] / 10.0;
				double yValue = - fLastSensorMeasurement.values[yIndex] / 10.0;
				double zValue = - fLastSensorMeasurement.values[2] / 10.0;
				if (isNaturalOrientationPortrait == false) {
					yValue *= -1;
				}
				
				// Push the accelerometer event data to the system.
				if (myCoronaRuntime != null && myCoronaRuntime.isRunning()) {
					myCoronaRuntime.getTaskDispatcher().send(new com.ansca.corona.events.AccelerometerTask(xValue, yValue, zValue, deltaTimeInFractionalSeconds));
				}
			}
		}
	}
	
	/** Monitors the gyroscope sensor, takes measurements periodically, and then posts that data to the Corona EventManager. */
	private class GyroscopeMonitor extends SensorMonitor {
		/** Stores a copy of the last received sensor measurement. */
		private SensorMeasurement fLastSensorMeasurement;
		
		/** Set true if the first sensor measurement has been skipped. */
		private boolean fHasSkippedFirstMeasurement;
		
		/** Set true if member variable "fLastSensorMeasurement" has received a measurement. */
		private boolean fHasReceivedMeasurement;
		
		/** Set true after the first sensor sample has been accepted by the timer listener. */
		private boolean fHasReceivedSample;
		
		/**
		 * Set to the "sensor timestamp" value belonging to the last sample sent to the Corona EventManager.
		 * Needed to calculate delta time between samples.
		 */
		private long fLastSampleTimestamp;
		
		
		/** Creates a new gyroscope sensor monitor. */
		public GyroscopeMonitor() {
			super();
			
			// Initialize member variables.
			fLastSensorMeasurement = new SensorMeasurement();
			fLastSensorMeasurement.values = new float[] { 0.0f, 0.0f, 0.0f };
			fHasSkippedFirstMeasurement = false;
			fHasReceivedMeasurement = false;
			fHasReceivedSample = false;
			fLastSampleTimestamp = 0;
			
			// Set up the listeners.
			setSensorListener(new GyroscopeMonitor.SensorHandler());
			setTimerListener(new GyroscopeMonitor.TimerHandler());
		}
		
		/**
		 * Gets the unique Android sensor type ID for an gyroscope, to be used by an Android SensorManager object.
		 * @return Returns the unique integer ID identifying an accelerometer.
		 */
		@Override
		public int getSensorType() {
			return android.hardware.Sensor.TYPE_GYROSCOPE;
		}
		
		/** Called when the sensor monitor is about to be started. */
		@Override
		protected void onStarting() {
			fHasSkippedFirstMeasurement = false;
			fHasReceivedMeasurement = false;
			fHasReceivedSample = false;
		}
		
		/** Private class used to receive data from an Android sensor. */
		private class SensorHandler implements SensorEventListener {
			@Override
			public void onSensorChanged(SensorEvent event) {
				// Validate.
				if (event == null) {
					return;
				}
				
				// Throw away the first measurement since it is typically taken before the start() method was called.
				// This causes the final recorded sample to have a much larger delta time than expected.
				if (fHasSkippedFirstMeasurement == false) {
					fHasSkippedFirstMeasurement = true;
					return;
				}
				
				// Make a copy of the received sensor event data to be eventually analyzed by the TimerHandler.
				// Note: We do not want to hold a reference to the given sensor event object because Android will
				//       eventually re-use and overwrite it for efficiency.
				fLastSensorMeasurement.copyFrom(event);
				fHasReceivedMeasurement = true;
			}
			
			@Override
			public void onAccuracyChanged(Sensor sensor, int accuracy) { }
		}
		
		/** Private class that receives events from a timer and post sensor measurements to the Corona EventManager. */
		private class TimerHandler implements MessageBasedTimer.Listener {
			@Override
			public void onTimerElapsed() {
				// Fetch the controller object.
				Controller controller = myCoronaRuntime.getController();
				if (controller == null) {
					return;
				}
				
				// Do not continue if a measurement has not been received yet.
				if (fHasReceivedMeasurement == false) {
					return;
				}
				
				// Skip the first sample and record its timestamp.
				// This initial timestamp is needed to compute delta time.
				if (fHasReceivedSample == false) {
					fLastSampleTimestamp = fLastSensorMeasurement.timestamp;
					fHasReceivedSample = true;
					return;
				}
				
				// Check if the following has occurred:
				// 1) If we have not received a new measurement, then duplicate the last received measurement.
				//    This typically happens on devices that do not provide measurements at regular intervals, such as the Kindle Fire.
				// 2) If the timestamp is too close to the last sample's timestamp (within half an interval), then
				//    bump up the timestamp to prevent a really small delta time from being calculated down below.
				long minSensorTimestamp = fLastSampleTimestamp + ((getInterval().getTotalMilliseconds() * 1000000L) / 2L);
				if (compareSensorTimestamps(fLastSensorMeasurement.timestamp, minSensorTimestamp) <= 0) {
					fLastSensorMeasurement.timestamp = fLastSampleTimestamp + (getInterval().getTotalMilliseconds() * 1000000L);
				}
				
				// Calculate delta time. Convert from nanoseconds to seconds.
				long deltaTimeInNanoseconds = subtractSensorTimestamps(fLastSensorMeasurement.timestamp, fLastSampleTimestamp);
				double deltaTimeInFractionalSeconds = (double)deltaTimeInNanoseconds * 0.000000001;
				fLastSampleTimestamp = fLastSensorMeasurement.timestamp;
				
				// Fetch the gyroscope data. Always provide values relative to portrait orientation.
				// This means we have to flip the x and y values on devices that are naturally landscape, such as tablets.
				boolean isNaturalOrientationPortrait = controller.isNaturalOrientationPortrait();
				int xIndex = isNaturalOrientationPortrait ? 0 : 1;
				int yIndex = isNaturalOrientationPortrait ? 1 : 0;
				double xValue = fLastSensorMeasurement.values[xIndex];
				double yValue = fLastSensorMeasurement.values[yIndex];
				double zValue = fLastSensorMeasurement.values[2];
				if (isNaturalOrientationPortrait == false) {
					yValue *= -1;
				}
				
				// Push the gyroscope event data to the system.
				if (myCoronaRuntime != null && myCoronaRuntime.isRunning()) {
					myCoronaRuntime.getTaskDispatcher().send(new com.ansca.corona.events.GyroscopeTask(xValue, yValue, zValue, deltaTimeInFractionalSeconds));
				}
			}
		}
	}
	
	/** Listens for location data from a LocationManager and outputs data to Corona's Lua listeners. */
	private class CoronaLocationListener implements LocationListener {
		private boolean fHasReceivedData = false;
		private boolean fSupportsGps = false;
		private boolean fSupportsNetwork = false;
		
		public void setSupportsGps() {
			fSupportsGps = true;
		}
		
		public void setSupportsNetwork() {
			fSupportsNetwork = true;
		}
		
		@Override
		public void onStatusChanged(String provider, int status, Bundle extra)
		{ }
		
		@Override
		public void onProviderEnabled(String provider)
		{ }
		
		@Override
		public void onProviderDisabled(String provider)
		{ }
		
		@Override
		public void onLocationChanged(Location location) {
			// Favor GPS data over Network data because it is more accurate.
			// However, always accept the first data point because the network usually provides data before GPS.
			if (location.getProvider().equals(LocationManager.GPS_PROVIDER) == false) {
				if (fHasReceivedData && fSupportsGps && myLocationManager.isProviderEnabled(LocationManager.GPS_PROVIDER)) {
					return;
				}
			}
			fHasReceivedData = true;

			// Send location data to the Lua listener.
			if (myCoronaRuntime != null && myCoronaRuntime.isRunning()) {
				myCoronaRuntime.getTaskDispatcher().send(new com.ansca.corona.events.LocationTask(
						location.getLatitude(),
						location.getLongitude(),
						location.getAltitude(),
						(double)location.getAccuracy(),
						(double)location.getSpeed(),
						(double)location.getBearing(),
						(double)location.getTime() / 1000.0));	// Convert milliseconds to seconds.);
			}
		}
	}

	/** Default handling of the location permission on Android 6+. */
	private static class LocationRequestPermissionsResultHandler 
		implements CoronaActivity.OnRequestPermissionsResultHandler {

		@Override
		public void onHandleRequestPermissionsResult(
				CoronaActivity activity, int requestCode, String[] permissions, int[] grantResults) {

			PermissionsSettings permissionsSettings = activity.unregisterRequestPermissionsResultHandler(this);

			if (permissionsSettings != null) {
				permissionsSettings.markAsServiced();
			}
		}
	}
	
	/**
	 * Stop event notifications for type
	 * 
	 * @param eventType		Event type
	 */
	private void stopType( final int eventType ) {
		
		// Safely grab handler.
		android.os.Handler handler = getHandlerSafely();
		
		if (handler == null) {
			return;
		}
		
		// Run this on the main thread.
		handler.post(new Runnable() {
			@Override
			public void run() {
				switch ( eventType ) {
				case JavaToNativeShim.EventTypeMultitouch:
					// Multitouch actually isn't handled in the sensor manager, but in the activity onTouch.
					break;
					
				case JavaToNativeShim.EventTypeOrientation:
					// TODO:
					break;
				case JavaToNativeShim.EventTypeAccelerometer:
					myAccelerometerMonitor.stop();
					break;
				case JavaToNativeShim.EventTypeGyroscope:
					myGyroscopeMonitor.stop();
					break;
				case JavaToNativeShim.EventTypeHeading:
		            if ( myOrientationSensorListener != null ) {
		            	mySensorManager.unregisterListener( myOrientationSensorListener ); 
		            	myOrientationSensorListener = null;
		            	myLastHeading = -1;
		            }
					break;
				case JavaToNativeShim.EventTypeLocation:
					if ( myLocationListener != null ) {
						myLocationManager.removeUpdates( myLocationListener );
						myLocationListener = null;
					}
					break;
				}
			}
		} );
	}

	/**
	 * Is the specified sensor active
	 * Mostly for multitouch, which isn't actually a sensor
	 * 
	 * @param eventType
	 * @return true if sensor is active
	 */
	public boolean isActive( int eventType ) {
		return myActiveSensors[ eventType ];
	}
	
	/**
	 * Start event notifications for type
	 * 
	 * @param eventType		Type of event
	 */
	public void start( int eventType ) {
		if ( myActiveSensors[ eventType ] == true )
			return;
		
		startType( eventType );

		myActiveSensors[ eventType ] = true;
	}
	
	/**
	 * Stop event notifications for type
	 * 
	 * @param eventType		Event type
	 */
	public void stop( int eventType ) {
		if ( myActiveSensors[ eventType ] == false )
			return;

		stopType( eventType );
		
		myActiveSensors[eventType] = false;
	}
	
	/**
	 * Stops event notifications for all sensor types.
	 */
	public void stop() {
		for ( int i = 0; i < JavaToNativeShim.EventTypeNumTypes; i++ ) {
			stop( i );
		}
	}
	
	/**
	 * Set accelerometer interval
	 * 
	 * @param frequency
	 */
	public void setAccelerometerInterval( int frequency ) {
		myAccelerometerMonitor.setIntervalInHertz(frequency);
	}

	/**
	 * Set gyroscope interval
	 * 
	 * @param frequency
	 */
	public void setGyroscopeInterval( int frequency ) {
		myGyroscopeMonitor.setIntervalInHertz(frequency);
	}
	
	/**
	 * Determines if this device has an accelerometer sensor.
	 * @return Returns true if this device has a accelerometer. Returns false if not.
	 */
	public boolean hasAccelerometer()
	{
		boolean hasSensor = false;
		
		if (mySensorManager != null)
		{
			hasSensor = (mySensorManager.getSensorList(Sensor.TYPE_ACCELEROMETER).size() > 0);
		}
		return hasSensor;
	}
	
	/**
	 * Determines if this device has a gyroscope sensor.
	 * @return Returns true if this device has a gyroscope. Returns false if not.
	 */
	public boolean hasGyroscope()
	{
		boolean hasSensor = false;
		
		if (mySensorManager != null)
		{
			hasSensor = (mySensorManager.getSensorList(Sensor.TYPE_GYROSCOPE).size() > 0);
		}
		return hasSensor;
	}

	/**
	 * Determines if this device has the hardware to compute heading events.
	 * @return Returns true if this device has said hardware. Returns false if not.
	 */
	public boolean hasHeadingHardware()
	{
		boolean hasSensor = false;

		if (mySensorManager != null)
		{
			hasSensor = (mySensorManager.getSensorList(Sensor.TYPE_ORIENTATION).size() > 0);
		}
		return hasSensor;
	}
	
	/**
	 * Pause all notifications
	 */
	public void pause() {
		for ( int i = 0; i < JavaToNativeShim.EventTypeNumTypes; i++ ) {
			stopType( i );
		}
	}
	
	/**
	 * Resume all notifications
	 */
	public void resume() {
		for ( int i = 0; i < JavaToNativeShim.EventTypeNumTypes; i++ ) {
			if ( myActiveSensors[i] )
				startType( i );
		}
	}
	
	/**
	 * Set the accuracy of the GPS
	 * 
	 * @param meters
	 */
	public void setLocationAccuracy( double meters ) {
		// No notion of accuracy on Android
	}

	/**
	 * Set the threshold for the GPS
	 * 
	 * @param meters
	 */
	public void setLocationThreshold( double meters ) {
		myLocationThreshold = meters;
	}
	
	/**
	 * Compares "SensorEvent.timestamp" values.
	 * Correctly handles timestamp overflow where large negative numbers are considered greater than large positive numberes.
	 * @param x Timestamp value to be compared with "y".
	 * @param y Timestamp value to be compared with "x".
	 * @return Returns a positive value if "x" is greater than "y".
	 *         Returns zero if "x" is equal to "y".
	 *         Returns a negative number if "x" is less than "y".
	 */
	public static int compareSensorTimestamps(long x, long y) {
		// You cannot negate the min integer value. Increment it by one before doing so.
		if (y == Long.MIN_VALUE) {
			y++;
		}
		
		// Compare the given timestamp values via subtraction. Overlow for this subtraction operation is okay.
		long deltaTime = x - y;
		if (deltaTime < 0) {
			return -1;
		}
		else if (deltaTime == 0) {
			return 0;
		}
		return 1;
	}
	
	/**
	 * Subtracts the given "SensorEvent.timestamp" values.
	 * @param x Timestamp value to be subtracted.
	 * @param y Timestamp value to be subtracted from "x".
	 * @return Returns the difference between the two given values in nanosecond.
	 */
	public static long subtractSensorTimestamps(long x, long y) {
		// You cannot negate the min integer value. Increment it by one before doing so.
		if (y == Long.MIN_VALUE) {
			y++;
		}
		
		// Compare the given timestamp values via subtraction. Overlow for this subtraction operation is okay.
		return x - y;
	}
}
