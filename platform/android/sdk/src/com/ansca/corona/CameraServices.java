//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;


/**
 * Provides methods for detecting if camera hardware exists, checking camera permissions, and other helpful methods.
 * You cannot make instances of this class. Access is provided via static methods.
 */
public class CameraServices {

	/** Default handling of the camera permission on Android 6+. */
	public static class CameraRequestPermissionsResultHandler 
		implements CoronaActivity.OnRequestPermissionsResultHandler {

		@Override
		public void onHandleRequestPermissionsResult(
				CoronaActivity activity, int requestCode, String[] permissions, int[] grantResults) {
			com.ansca.corona.permissions.PermissionsSettings permissionsSettings = activity.unregisterRequestPermissionsResultHandler(this);

			if (permissionsSettings != null) {
				permissionsSettings.markAsServiced();
			} else {
				// Don't have any permissions settings corresponding with this request! SAY SOMETHING!
			}
		}
	}

	/** The constructor is made private to prevent instances from being made. */
	private CameraServices() {
	}
	
	/**
	 * Checks if this applicaction has permission to use the camera.
	 * @return Returns true if this application has permission. Returns false if not.
	 */
	public static boolean hasPermission() {
		boolean hasPermission = false;
		try {
			String permissionName = android.Manifest.permission.CAMERA;
			android.content.Context context = CoronaEnvironment.getApplicationContext();
			if (context != null) {
				hasPermission = (context.checkCallingOrSelfPermission(permissionName) == android.content.pm.PackageManager.PERMISSION_GRANTED);
			}
		}
		catch (Exception ex) { }
		return hasPermission;
	}
	
	/**
	 * Brings up a dialog, asking the user for permission to access the camera.
	 */
	public static void requestCameraPermission() {

		// Validate.
		final CoronaActivity ourActivity = CoronaEnvironment.getCoronaActivity();
		if (ourActivity == null || !hasCamera() || android.os.Build.VERSION.SDK_INT < 23) {
			return;
		}

		// Create our Permissions Settings to compare against in the handler.
		com.ansca.corona.permissions.PermissionsSettings settings = new com.ansca.corona.permissions.PermissionsSettings(android.Manifest.permission.CAMERA);

		// Request Camera permission.
		ourActivity.requestPermissions(settings.getPermissions().toArray(new String[0]), 
			ourActivity.registerRequestPermissionsResultHandler(new CameraServices.CameraRequestPermissionsResultHandler(), settings));
	}

	/**
	 * Determines if the device has any cameras.
	 * @return Returns true if this device has at least one camera. Returns false if no cameras were found.
	 */
	public static boolean hasCamera() {
		return (hasRearCamera() || hasFrontFacingCamera());
	}
	
	/**
	 * Determines if the device has a rear-facing camera.
	 * @return Returns true if the device has a rear-facing camera. Returns false if not.
	 */
	public static boolean hasRearCamera() {
		boolean hasCamera = false;
		android.content.Context context = CoronaEnvironment.getApplicationContext();
		if (context != null) {
			hasCamera = context.getPackageManager().hasSystemFeature(android.content.pm.PackageManager.FEATURE_CAMERA);
		}
		return hasCamera;
	}
	
	/**
	 * Determines if the device has a front-facing camera.
	 * @return Returns true if the device has a front-facing camera. Returns false if not.
	 */
	public static boolean hasFrontFacingCamera() {
		boolean hasCamera = false;
		android.content.Context context = CoronaEnvironment.getApplicationContext();
		if (context != null) { 
			hasCamera = context.getPackageManager().hasSystemFeature(android.content.pm.PackageManager.FEATURE_CAMERA_FRONT);
		}
		return hasCamera;
	}
	
	/**
	 * Fetches the number of cameras that exist on this device.
	 * @return Returns the number of cameras that this device has.
	 *         Returns zero if this device does not have any camera.
	 */
	public static int getCameraCount() {
		int cameraCount = 0;
		
		// Fetch the number of cameras this device has via reflection.
		// We're doing this because multiple camera support was added to Android 2.3, but Corona needs to support 2.2.
		// Warning: The getCameraMethod() can return an invalid positive value on devices that do not have a camera.
		//          We must check the PackageManager's camera feature first before calling this method.
		try {
			if (hasCamera()) {
				java.lang.reflect.Method getNumberOfCamerasMethod =
						android.hardware.Camera.class.getMethod("getNumberOfCameras");
				Integer result = (Integer)getNumberOfCamerasMethod.invoke(null);
				cameraCount = result.intValue();
			}
		}
		catch (Exception ex) { }
		
		// Return the result.
		return cameraCount;
	}
}
