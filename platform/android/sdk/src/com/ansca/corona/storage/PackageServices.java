//////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2018 Corona Labs Inc.
// Contact: support@coronalabs.com
//
// This file is part of the Corona game engine.
//
// Commercial License Usage
// Licensees holding valid commercial Corona licenses may use this file in
// accordance with the commercial license agreement between you and 
// Corona Labs Inc. For licensing terms and conditions please contact
// support@coronalabs.com or visit https://coronalabs.com/com-license
//
// GNU General Public License Usage
// Alternatively, this file may be used under the terms of the GNU General
// Public license version 3. The license is as published by the Free Software
// Foundation and appearing in the file LICENSE.GPL3 included in the packaging
// of this file. Please review the following information to ensure the GNU 
// General Public License requirements will
// be met: https://www.gnu.org/licenses/gpl-3.0.html
//
// For overview and more information on licensing please refer to README.md
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.storage;

/**
 * Provides easy access to information about various packages installed on an Android device.
 * A "package" in this case can refer to an app, a background service, or any other APK installed on a device.
 * <p>
 * <b>Added in <a href="http://developer.coronalabs.com/release-ent/2016/2932/">daily build 2016.2932</a></b>.
 * @see ApplicationContextProvider
 */
public class PackageServices extends com.ansca.corona.ApplicationContextProvider {
	/**
	 * Creates an object that provides easy access to the packages on an Android device.
	 * @param context Reference to an Android created 
	 *				  <a href="http://developer.android.com/reference/android/content/Context.html">Context</a> used to access the 
	 *				  input device system.
	 *                <p>
	 *                Setting this to null will cause an exception to be thrown.
	 */
	public PackageServices(android.content.Context context) {
		super(context);
	}

	/**
	 * Gets the state of the provided package on the device.
	 * @param packageName The full package name of the application to check for. Cannot be null or empty.
	 * @param flags Additional option flags. Use any combination of the following to modify the results.
	 *              <p><a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#GET_ACTIVITIES">GET_ACTIVITIES</a>
	 *              <p><a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#GET_CONFIGURATIONS">GET_CONFIGURATIONS</a>
	 *              <p><a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#GET_GIDS">GET_GIDS</a>
	 *              <p><a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#GET_INSTRUMENTATION">GET_INSTRUMENTATION</a>
	 *              <p><a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#GET_INTENT_FILTERS">GET_INTENT_FILTERS</a>
	 *              <p><a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#GET_META_DATA">GET_META_DATA</a>
	 *              <p><a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#GET_PERMISSIONS">GET_PERMISSIONS</a>
	 *              <p><a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#GET_PROVIDERS">GET_PROVIDERS</a>
	 *              <p><a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#GET_RECEIVERS">GET_RECEIVERS</a>
	 *              <p><a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#GET_SERVICES">GET_SERVICES</a>
	 *              <p><a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#GET_SHARED_LIBRARY_FILES">GET_SHARED_LIBRARY_FILES</a>
	 *              <p><a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#GET_SIGNATURES">GET_SIGNATURES</a>
	 *              <p><a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#GET_URI_PERMISSION_PATTERNS">GET_URI_PERMISSION_PATTERNS</a>
	 *              <p><a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#GET_UNINSTALLED_PACKAGES">GET_UNINSTALLED_PACKAGES</a>
     * @return Returns a {@link com.ansca.corona.storage.PackageState PackageState} indicating what state the desired package is in.
     *         <p>
     *         Returns {@link com.ansca.corona.storage.PackageState#MISSING PackageState.MISSING} if the provided packageName is invalid.
     */
	public PackageState getPackageState(String packageName, int flags) {
		// Validate.
		if ((packageName == null) || (packageName.length() <= 0)) {
			return PackageState.MISSING;
		}

		// Fetch the PackageManager
		android.content.pm.PackageManager pm = getApplicationContext().getPackageManager();

		android.content.pm.PackageInfo packageInfo = null;
		try {
            packageInfo = pm.getPackageInfo(packageName, flags);
        } catch (Exception e) {
            // The package is missing!
            return PackageState.MISSING;
        }

		if (packageInfo != null) {
            android.content.pm.ApplicationInfo appInfo =
                    packageInfo.applicationInfo;
            if (appInfo == null) {
                try {
                    appInfo = pm.getApplicationInfo(packageName, 0);
                } catch (Exception e) {
                    // Can't get the ApplicationInfo for the package.
                    // Perhaps this user isn't allowed to access it?
                    // Either way, it's missing as far as we're concerned.
					return PackageState.MISSING;
                }
            }

            // Now that we have some sanity around the ApplicationInfo for the package,
            // we can check if it's enabled or not.
            if (appInfo != null) {
                // Check if the app is disabled
                if (!appInfo.enabled) {
                    return PackageState.DISABLED;
                } else {
					return PackageState.ENABLED;
				}
            }
        }
		// Package is missing! Should never reach this.
		return PackageState.MISSING;
	}

	/**
	 * <!-- Ported from StoreServices.java -->
	 * Determines if the given application exists on the device.
	 * @param packageName The full package name of the application to check for. Cannot be null or empty.
	 * @return Returns true if the application is installed on the device. Returns false if not or given an invalid argument.
	 */
	public boolean isPackageNameInstalled(String packageName) {
		// Validate.
		if ((packageName == null) || (packageName.length() <= 0)) {
			return false;
		}
		
		// Check if the given package is installed on the device.
		boolean isInstalled = false;
		try {
			android.content.Context context = getApplicationContext();
			context.getPackageManager().getPackageInfo(packageName, android.content.pm.PackageManager.GET_ACTIVITIES);
			isInstalled = true;
		}
		catch (Exception ex) { }
		
		// Return the result.
		return isInstalled;
	}
}