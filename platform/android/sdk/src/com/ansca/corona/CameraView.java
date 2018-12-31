/*
 * Camera sample code written by Google and included in the Android SDK.
 * Heavily modified by Ansca Inc. to be integrated into Corona.
 *
 * -- Google's License Agreement --
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.ansca.corona;


/** Displays the Android device's camera feed on a view object. */
class CameraView extends android.view.SurfaceView implements android.view.SurfaceHolder.Callback {
	private android.view.SurfaceHolder fHolder = null;
	private android.hardware.Camera fCamera = null;
	private android.hardware.Camera.PictureCallback fTakePictureListener = null;
	private android.view.OrientationEventListener fOrientationListener;
	private int fDeviceOrientationInDegrees = 0;
	private int fUsingCameraId = 0;
	private boolean fIsUsingFrontFacingCamera = false;
	private int fCameraOrientationInDegrees = 0;
	private boolean fIsCameraOrientationKnown = false;
	
	/**
	 * Creates a new camera view to be displayed in an activity.
	 * @param context The parent that owns this view.
	 */
    CameraView(android.content.Context context) {
        super(context);

        // Install a SurfaceHolder.Callback so we get notified when the
        // underlying surface is created and destroyed.
        android.view.SurfaceHolder holder = getHolder();
        holder.addCallback(this);
        holder.setType(android.view.SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
		
		// Set up a listener for detecting the physical orientation of the device.
		// This is needed because the view might have a fixed orientation, so this is the only method for detecting orientation.
		// The result is used to set up the camera to output an image matching the device's actual orientation.
		fOrientationListener = new android.view.OrientationEventListener(context) {
			@Override
			public void onOrientationChanged(int orientationInDegrees) {
				// Do not continue if the device is incapable of determining its current orientation.
				if (orientationInDegrees == android.view.OrientationEventListener.ORIENTATION_UNKNOWN) {
					return;
				}
				
				// Given orientation angle is counter-clockwise.
				// Convert to clockwise to match getDefaultDisplay().getRotation() direction.
				orientationInDegrees = (360 - orientationInDegrees) % 360;
				
				// Store the orientation.
				if ((orientationInDegrees >= 45) && (orientationInDegrees < 135)) {
					fDeviceOrientationInDegrees = 90;
				}
				else if ((orientationInDegrees >= 135) && (orientationInDegrees < 225)) {
					fDeviceOrientationInDegrees = 180;
				}
				else if ((orientationInDegrees >= 225) && (orientationInDegrees < 315)) {
					fDeviceOrientationInDegrees = 270;
				}
				else {
					fDeviceOrientationInDegrees = 0;
				}
			}
		};
    }
	
	/**
	 * Sets the listener to be called when a camera shot has been taken.
	 * @param listener The listener that will receive the camera shot JPEG.
	 *                 Set to null to clear the listener.
	 */
	public void setTakePictureListener(android.hardware.Camera.PictureCallback listener) {
		fTakePictureListener = listener;
	}
	
	/**
	 * Switches this camera view to use the given camera.
	 * On most Android phones, 0 would be the rear camera and 1 would be the front-facing camera.
	 * You should call CameraServices.getCameraCount() to find out how many cameras exist on this device before calling this method.
	 * @param cameraIndex Zero based index of the camera to use.
	 */
	public void selectCameraByIndex(int cameraIndex) {
		// Do not continue if camera selection has not changed.
		if (cameraIndex == fUsingCameraId) {
			return;
		}
		
		// Store the camera selection.
		fUsingCameraId = cameraIndex;
		
		// If the last camera is currently enabled, then switch to the new camera selection.
		if (isCameraEnabled()) {
			disableCamera();
			enableCamera();
		}
	}
	
	/**
	 * Gets the index of the currently selected camera that this view is using.
	 * @return Zero based index of the camera used by this view.
	 */
	public int getSelectedCameraIndex() {
		return fUsingCameraId;
	}
	
	/**
	 * Determines if the camera is currently enabled and displaying video on this view.
	 * @return Returns true if an active camera feed is being displayed. Returns false if disabled.
	 */
	public boolean isCameraEnabled() {
		return (fCamera != null);
	}
	
	/**
	 * Determines if the camera feed has been disabled and nothing is being displayed.
	 * @return Returns true if the camera feed has been disabled. Returns false if enabled.
	 */
	public boolean isCameraDisabled() {
		return !isCameraEnabled();
	}
	
	/**
	 * Establishes a connection to the device's camera feed for taking camera shots.
	 */
	protected void enableCamera() {
		// Do not continue if camera is already enabled.
		if (isCameraEnabled() || (fHolder == null) || (fHolder.getSurface() == null)) {
			return;
		}
		
		// Open a connection to the camera.
        try {
			java.lang.reflect.Method openMethod = null;
			try {
				openMethod = android.hardware.Camera.class.getMethod("open", new Class[] {Integer.TYPE});
			}
			catch (Exception ex) { }
			if (openMethod != null) {
				fCamera = (android.hardware.Camera)(openMethod.invoke(null, new Object[] {fUsingCameraId}));
			}
			else {
				fCamera = android.hardware.Camera.open();
			}
			updateCameraInformation();
			updateCameraOrientation();
			fCamera.setPreviewDisplay(fHolder);
			fCamera.startPreview();
        }
		catch (Exception ex) {
			if (fCamera != null) {
				fCamera.release();
				fCamera = null;
			}
			android.util.Log.v("Corona", "Failed to enable camera.");
			return;
        }
		
		// Set up a listener for determining device orientation.
		// This is used to save camera shots at the orientation the user is holding the device.
		if (fOrientationListener.canDetectOrientation()) {
			fOrientationListener.enable();
		}
	}
	
	/**
	 * Disconnects from the camera stops displaying a camera feed on the view.
	 */
	protected void disableCamera() {
		// Do not continue if the camera is already disabled.
		if (isCameraDisabled()) {
			return;
		}

		// Close the camera connection.
		try {
			fCamera.stopPreview();
			fCamera.setPreviewDisplay(null);
		}
		catch (Exception ex) { }
        fCamera.release();
        fCamera = null;
		
		// Disable the orientation listener.
		if (fOrientationListener.canDetectOrientation()) {
			fOrientationListener.disable();
		}
	}
	
	/**
	 * Has the camera take a picture. The picture will be saved as a JPEG and sent to the
	 * listener assigned via the setTakePictureListener() function.
	 */
	public void takePicture() {
		// Do not continue if the camera is currently disabled.
		if (isCameraDisabled()) {
			return;
		}
		
		// Set the orientation of the camera to match the device
		// Set up the camera to output an image matching the device's actual orientation.
		fCamera.stopPreview();
		updateCameraOrientation();
		fCamera.startPreview();

		// Take a picture asynchronously. The result will be given to the provided listener.
		fCamera.takePicture(null, null, fTakePictureListener);
	}
	
	/**
	 * Called when this view's surface has been created. Automatically enables the camera feed.
	 * @param holder The object that holds the surface view that the camera renders to.
	 */
    public void surfaceCreated(android.view.SurfaceHolder holder) {
		// Enable the camera now that we have a surface to render the camera output to.
		fHolder = holder;
		enableCamera();
    }
	
	/**
	 * Called when the surface has been changed, such as startup and screen rotation.
	 * @param holder The object that holds the surface view that the camera renders to.
	 * @param w The width of the surface.
	 * @param h The height of the surface.
	 */
    public void surfaceChanged(android.view.SurfaceHolder holder, int format, int w, int h) {
		// Start showing the camera feed onscreen.
		if (isCameraEnabled()) {
			fCamera.stopPreview();
			updateCameraOrientation();
			fCamera.startPreview();
		}
    }
	
	/**
	 * Called when this view's drawin surface has been destroyed. Automatically disables the camera feed.
	 * @param holder The object that holds the surface view that the camera renders to.
	 */
    public void surfaceDestroyed(android.view.SurfaceHolder holder) {
		// Stop rendering the camera output to the surface.
		disableCamera();
    }
	
	/**
	 * Determines what the optimal viewing size for the camera to avoid screen stretching.
	 * @param sizes Must be set to a list of screen preview sizes supported by the camera.
	 * @param w The width of the view that the camera must render to.
	 * @param h The height of the view that the camera must render to.
	 * @return Returns the optimal width and height that this view's surface should be set to to avoid video stretching.
	 */
    private android.hardware.Camera.Size getOptimalPreviewSize(java.util.List<android.hardware.Camera.Size> sizes, int w, int h) {
        final double ASPECT_TOLERANCE = 0.05;
        double targetRatio = (double) w / h;
        if (sizes == null) return null;

        android.hardware.Camera.Size optimalSize = null;
        double minDiff = Double.MAX_VALUE;
        int targetHeight = h;

        // Try to find an size match aspect ratio and size
        for (android.hardware.Camera.Size size : sizes) {
            double ratio = (double) size.width / size.height;
            if (Math.abs(ratio - targetRatio) > ASPECT_TOLERANCE) continue;
            if (Math.abs(size.height - targetHeight) < minDiff) {
                optimalSize = size;
                minDiff = Math.abs(size.height - targetHeight);
            }
        }

        // Cannot find the one match the aspect ratio, ignore the requirement
        if (optimalSize == null) {
            minDiff = Double.MAX_VALUE;
            for (android.hardware.Camera.Size size : sizes) {
                if (Math.abs(size.height - targetHeight) < minDiff) {
                    optimalSize = size;
                    minDiff = Math.abs(size.height - targetHeight);
                }
            }
        }
        return optimalSize;
    }
	
	/**
	 * Updates the orientation of the camera feed and the image it outputs when taking a picture.
	 */
	private void updateCameraOrientation() {
		android.view.Display display;
		int displayAngle;
		boolean isNaturalOrientationPortrait;
		
		// Validate.
		if (fCamera == null) {
			return;
		}
		
		// Fetch screen's current orientation in degrees, clockwise.
		display = ((android.view.WindowManager)getContext().getSystemService(android.content.Context.WINDOW_SERVICE)).getDefaultDisplay();
		switch (display.getRotation()) {
			case android.view.Surface.ROTATION_180:
				displayAngle = 180;
				break;
			case android.view.Surface.ROTATION_270:
				displayAngle = 270;
				break;
			case android.view.Surface.ROTATION_90:
				displayAngle = 90;
				break;
			default:
				displayAngle = 0;
				break;
		}
		isNaturalOrientationPortrait = (display.getWidth() < display.getHeight()) && ((displayAngle == 0) || (displayAngle == 180));
		
		// Fetch the orientation of the camera, counter-clockwise.
		int mountedCameraOrientation;
		if (fIsCameraOrientationKnown) {
			// Camera orientation is known on Android 2.3 or higher devices.
			mountedCameraOrientation = fCameraOrientationInDegrees;
		}
		else {
			// Mounted camera orientation is unknown on Android 2.2 or older devices.
			// Assume that portrait Android devices such as phones have cameras mounted in portrait too,
			// which is 90 degrees (counter-clockwise) from its typical landscape position.
			mountedCameraOrientation = (isNaturalOrientationPortrait ? 90 : 0);
		}
		
		// Set camera preview orientation to be relative to the device's orientation.
		int cameraAngle;
		if (fIsUsingFrontFacingCamera) {
			// This is the front-facing camera, which is an Android 2.3 feature.
			cameraAngle = (mountedCameraOrientation + displayAngle) % 360;
			cameraAngle = (360 - cameraAngle) % 360;
		}
		else {
			// This is the rear camera.
			cameraAngle = (360 + mountedCameraOrientation - displayAngle) % 360;
		}
		fCamera.setDisplayOrientation(cameraAngle);
		
		// Set the orientation of the camera shot's image file to be relative to how the device is currently being held.
		// For example, if the device is held landscape then the camera shot image should be landscape too.
		android.hardware.Camera.Parameters parameters = fCamera.getParameters();
		int imageRotation = mountedCameraOrientation - fDeviceOrientationInDegrees;
		if (imageRotation < 0) {
			imageRotation += 360;
		}
		if (fIsUsingFrontFacingCamera) {
			// The resulting image file from a front-facing camera will be flipped/mirroed horizontally at the device's default display orientation.
			// When held horizontally, the image will appear upside down. Rotate the image by 180 degrees in this condition.
			if ((isNaturalOrientationPortrait && ((imageRotation % 180) == 0)) ||
			    (!isNaturalOrientationPortrait && ((imageRotation % 180) == 90))) {
				imageRotation = (imageRotation + 180) % 360;
			}
		}
		parameters.setRotation(imageRotation);
		fCamera.setParameters(parameters);
//TODO: The GalaxyTab does not correctly handle the setRotation() method up above. Instead of rotating the image file,
//      it will wrongly rotate the preview screen. To work-around this issue, we should not rotate the image file here.
//      Instead, the image file should be rotated by the CameraActivity where it can set the EXIF rotate meta-data itself.
	}
	
	/**
	 * Fetches information for the given camera ID and stores them in member variables.
	 * Information such as whether or not it is a front-facing camera and its mounted orientation.
	 */
	private void updateCameraInformation() {
		// Set member variables to default for an Android 2.2 or older OS.
		fIsUsingFrontFacingCamera = false;
		fIsCameraOrientationKnown = false;
		fCameraOrientationInDegrees = 0;
		
		// Attempt to fetch Android 2.3 camera information via reflection.
		try {
			Class cameraInfoClassType = Class.forName("android.hardware.Camera$CameraInfo");
			Object cameraInfo = cameraInfoClassType.newInstance();
			java.lang.reflect.Method getCameraInfoMethod =
							android.hardware.Camera.class.getMethod("getCameraInfo", new Class[] {Integer.TYPE, cameraInfoClassType});
			getCameraInfoMethod.invoke(null, new Object[] {fUsingCameraId, cameraInfo});
			java.lang.reflect.Field facingField = cameraInfoClassType.getField("facing");
			java.lang.reflect.Field orientationField = cameraInfoClassType.getField("orientation");
			fIsUsingFrontFacingCamera = (facingField.getInt(cameraInfo) == 1);
			fCameraOrientationInDegrees = orientationField.getInt(cameraInfo);
			fIsCameraOrientationKnown = true;
		}
		catch (Exception ex) { }
	}
}
