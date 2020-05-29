//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.listeners;

import com.ansca.corona.MailSettings;
import com.ansca.corona.permissions.PermissionsSettings;
import com.ansca.corona.SmsSettings;

/** The interface has the funcations that will show a new window/overlay */
public interface CoronaShowApiListener{
	/**
	 * Called from media.selectPhoto().  The lua script wants to select a photo from the gallery
	 * How the information gets back to the lua side is up to the CoronaKit developer.
	 * @param destinationFilePath The location of the requested save location.
	 */
	public void showSelectImageWindowUsing(String destinationFilePath);
	
	/**
	 * Called from media.selectPhoto().  The lua script wants to receive a photo from the camera.  
	 * How the information gets back to the lua side is up to the CoronaKit developer.
	 * @param destinationFilePath The location of the requested save location.
	 */
	public void showCameraWindowForImage(String destinationFilePath);
	
	/**
	 * Called from media.selectVideo().  The lua script wants to select a video from the gallery
	 * How the information gets back to the lua side is up to the CoronaKit developer.
	 */
	public void showSelectVideoWindow();
	
	/**
	 * Called from media.selectVideo().  The lua script wants to retrieve a video from the camera.
	 * How the information gets back to the lua side is up to the CoronaKit developer.
	 * @param maxVideoTime the preferred maximum length of the video.
	 * @param videoQuality the quality of the video.  0 for medium/low quality and 1 for high quality.
	 */
	public void showCameraWindowForVideo(int maxVideoTime, int videoQuality);

	/**
	 * Called from native.showPopup().  The lua script wants to send an email with the settings.
	 * @param mailSettings the settings the lua script passed made into an object.
	 */
	public void showSendMailWindowUsing(MailSettings mailSettings);

	/**
	 * Called from native.showPopup().  The lua script wants to send a sms with the settings.
	 * @param smsSettings the settings the lua script passed made into an object.
	 */
	public void showSendSmsWindowUsing(SmsSettings smsSettings);

	/**
	 * Called from native.showPopup().  The lua script wants to show an app store popup.
	 * @param settings A hash map of the app IDs and supported stores.
	 * @return Returns true if the window is about to be displayed
	 *         <p>
	 *         Returns false if the App Store could not be found and is unable to display a window.
	 */
	public boolean showAppStoreWindow(java.util.HashMap<String, Object> settings);

	/**
	 * Called from native.showPopup().  The lua script wants to request permissions with the settings.
	 * @param permissionsSettings the settings the lua script passed made into an object.
	 */
	public void showRequestPermissionsWindowUsing(PermissionsSettings permissionsSettings);
}
