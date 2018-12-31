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


/** Activity used to display a camera feed and for taking pictures. */
public class CameraActivity extends android.app.Activity {
	/** View object used to show a live camera preview. Also used to take a picture. */
    private CameraView fCameraView;
	
	/** Unique number to assign to the camera shot image file for the lifetime of this application. */
	private static int sNextImageFileNumber = 1;
	
	
	/**
	 * Called when the activity has been created.
	 * Creates the camera view and toolbar.
	 * @param savedInstanceState The bundle containing arguments used to initialize this activity.
	 *                           You are expected to assign a path\file to intent.setData() for this activity to save the picture to.
	 */
    @Override
	protected void onCreate(android.os.Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
		
		android.widget.FrameLayout.LayoutParams layoutParams;
		
		// Create a Corona resource services object used to obtain resource IDs by name.
		// This is needed because a Corona Enterprise built application may add its own resources
		// which would change and re-order the resource IDs that Corona was built with.
		com.ansca.corona.storage.ResourceServices resourceServices;
		resourceServices = new com.ansca.corona.storage.ResourceServices(this);

		// Create the ViewGroup that will contain all UI elements in this activity.
		android.widget.FrameLayout viewGroup = new android.widget.FrameLayout(this);
		setContentView(viewGroup);
		
		// Add the camera view and set it up to fill the screen.
		fCameraView = new CameraView(this);
		viewGroup.addView(fCameraView);
		
		// Set up a "Take Picture" listener.
		// This listener will grab the camera shot's JPEG and close this activity.
		fCameraView.setTakePictureListener(new android.hardware.Camera.PictureCallback() {
			public void onPictureTaken(byte[] data, android.hardware.Camera camera) {
				// Validate.
				if ((data == null) || (data.length <= 0)) {
					return;
				}
				
				// Set up a path\file name to save the image to.
				android.net.Uri uri = null;
				if (getIntent() != null) {
					// Attempt to fetch file name from intent.
					uri = getIntent().getData();
					if (uri == null) {
						// URI was not assigned to intent. Attempt to fetch URI from EXTRA_OUTPUT,normally used by camera apps.
						if (getIntent().getExtras() != null) {
							uri = (android.net.Uri)getIntent().getExtras().get(android.provider.MediaStore.EXTRA_OUTPUT);
						}
					}
				}
				boolean wasFileNameProvided = true;
				if (uri == null) {
					// File name not given to this activity.
					// Set up a default path\file name for the image.
					java.io.File imageFile = CameraActivity.createCameraShotFileObjectWith(CameraActivity.this, sNextImageFileNumber);
					uri = android.net.Uri.fromFile(imageFile);
					wasFileNameProvided = false;
				}
				
				// Save the camera shot to an image file.
				boolean wasSaved = false;
				try {
					java.io.FileOutputStream stream = new java.io.FileOutputStream(uri.getPath());
					stream.write(data);
					stream.flush();
					stream.close();
					wasSaved = true;
					if (wasFileNameProvided == false) {
						// File name was automatically generated. Increment number for the next generated file name.
						sNextImageFileNumber++;
					}
				}
				catch (Exception ex) {
					ex.printStackTrace();
				}
				if (wasSaved == false) {
					return;
				}
				
				// Set this activity's result to the path to the image file.
				// We can't pass the image object via an intent because it is usually too large,
				// which would cause an OutOfMemory exception.
				android.content.Intent intent = new android.content.Intent();
				intent.setData(uri);
				setResult(android.app.Activity.RESULT_OK, intent);
				
				// Close this activity.
				finish();
			}
		});
		
		// Add a toolbar to the bottom of the screen.
		android.widget.FrameLayout toolbarLayout = new android.widget.FrameLayout(this);
		toolbarLayout.setBackgroundColor(android.graphics.Color.argb(192, 0, 0, 0));
		layoutParams = new android.widget.FrameLayout.LayoutParams(
							android.widget.FrameLayout.LayoutParams.FILL_PARENT,
							android.widget.FrameLayout.LayoutParams.WRAP_CONTENT,
							android.view.Gravity.BOTTOM);
		viewGroup.addView(toolbarLayout, layoutParams);
		
		// Add a camera button to the toolbar.
		android.widget.ImageButton cameraButton = new android.widget.ImageButton(this);
		cameraButton.setPadding(2, 2, 2, 2);
		cameraButton.setBackgroundColor(android.graphics.Color.TRANSPARENT);
		cameraButton.setImageResource(resourceServices.getDrawableResourceId("ic_menu_camera"));
		layoutParams = new android.widget.FrameLayout.LayoutParams(
							android.widget.FrameLayout.LayoutParams.WRAP_CONTENT,
							android.widget.FrameLayout.LayoutParams.WRAP_CONTENT,
							android.view.Gravity.CENTER_HORIZONTAL | android.view.Gravity.CENTER_VERTICAL);
		toolbarLayout.addView(cameraButton, layoutParams);
		
		// Set up the camera button to take a picture.
		cameraButton.setOnClickListener(new android.view.View.OnClickListener() {
			public void onClick(android.view.View view) {
				fCameraView.takePicture();
			}
		});
		
		// If this device has more than one camera, then add a "Switch Cameras" button to the toolbar.
		if (CameraServices.getCameraCount() > 1) {
			android.widget.ImageButton switchButton = new android.widget.ImageButton(this);
			switchButton.setPadding(2, 2, 2, 2);
			switchButton.setBackgroundColor(android.graphics.Color.TRANSPARENT);
			switchButton.setImageResource(resourceServices.getDrawableResourceId("ic_menu_refresh"));
			layoutParams = new android.widget.FrameLayout.LayoutParams(
									android.widget.FrameLayout.LayoutParams.WRAP_CONTENT,
									android.widget.FrameLayout.LayoutParams.WRAP_CONTENT,
									android.view.Gravity.RIGHT | android.view.Gravity.CENTER_VERTICAL);
			toolbarLayout.addView(switchButton, layoutParams);
			switchButton.setOnClickListener(new android.view.View.OnClickListener() {
				public void onClick(android.view.View view) {
					// Select the next camera.
					int cameraIndex = (fCameraView.getSelectedCameraIndex() + 1) % CameraServices.getCameraCount();
					fCameraView.selectCameraByIndex(cameraIndex);
				}
			});
		}
    }
	
	/**
	 * Called when a physical button/key has been released by the user.
	 * @param keyCode Unique integer ID of the key that was released.
	 * @param event Provides information about the key that was released.
	 */
	@Override
	public boolean onKeyUp(int keyCode, android.view.KeyEvent event)  {
		// Take a picture if the physical OK button or Camera button was released.
		switch (keyCode) {
			case android.view.KeyEvent.KEYCODE_DPAD_CENTER:
			case android.view.KeyEvent.KEYCODE_CAMERA:
				fCameraView.takePicture();
				break;
		}
		
		// Given key was not handled. Let the base class handle it.
		return super.onKeyUp(keyCode, event);
	}
	
	/**
	 * Deletes all camera shot image files in the temporary cache directory.
	 * @param context Context object used to fetch a path to the application's cache directory. Cannot be null.
	 */
	public static void clearCachedPhotos(android.content.Context context) {
		java.io.File file;
		for (int index = 1; index < sNextImageFileNumber; index++) {
			try {
				file = createCameraShotFileObjectWith(context, index);
				if (file != null) {
					file.delete();
				}
			}
			catch (Exception ex) {
				ex.printStackTrace();
			}
		}
		sNextImageFileNumber = 1;
	}
	
	/**
	 * Creates a Java File object (not a file in the file system) having a cache path taken from the application's context
	 * and the number used in the image's file name.
	 * @param context Context object used to fetch a path to the application's cache directory. Cannot be null.
	 * @param number The number to be post-fixed to the image file's name.
	 * @return Returns a Java file object assigned a path and file name that a camera shot image should be saved to.
	 *         Returns null if given invalid arguments.
	 */
	private static java.io.File createCameraShotFileObjectWith(android.content.Context context, int number) {
		if (context == null) {
			return null;
		}
		return new java.io.File(context.getCacheDir(), "CameraShot" + Integer.toString(number) + ".jpg");
	}
}
