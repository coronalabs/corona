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

package com.ansca.corona.purchasing;


/**
 * Provides thread safe static functions for fetching store information from the device such as the store
 * the application was installed by, the in-app purchase stores that are available on the device, etc.
 * <p>
 * You cannot create instances of this class. This class only provides constants and static methods.
 */
public class StoreServices {
	/** Constructor made private to prevent objects from being made. */
	private StoreServices() {
	}
	
	
	/**
	 * The package name of the Google Marketplace application.
	 * Note: This is the most common name of the marketplace application.
	 */
	public static final String GOOGLE_MARKETPLACE_APP_PACKAGE_NAME_1 = "com.android.vending";
	
	/**
	 * The package name of the Google Marketplace application.
	 * Note: We have not seen this package name during testing, but Internet searches suggest this does exist.
	 *       Possibly for newer or older versions of the marketplace?
	 */
	public static final String GOOGLE_MARKETPLACE_APP_PACKAGE_NAME_2 = "com.google.android.feedback";
	
	/** The package name of the Amazon App Store application. */
	public static final String AMAZON_MARKETPLACE_APP_PACKAGE_NAME = "com.amazon.venezia";
	
	/** The package name of the Samsung App Store application. */
	public static final String SAMSUNG_MARKETPLACE_APP_PACKAGE_NAME = "com.sec.android.app.samsungapps";
	

	/**
	 * Fetches the unique name of the store that this applications will use if store.init() was called in Lua without parameters.
	 * @return Returns a unique name matching a string constant in the StoreName class.
	 *         Returns StoreName.NONE if no store can be used.
	 */
	public static String getDefaultInAppStoreName() {
		String storeName;
		
		// Use the Google Marketplace if it is installed on this device.
		storeName = StoreName.GOOGLE;
		if (isInAppStoreAvailable(storeName)) {
			return storeName;
		}
		
		// Unable to determine which store this application should use. Default to no store.
		return StoreName.NONE;
	}
	
	/**
	 * Determines if the given in-app purchasing store is available/accessible on this device.
	 * @param storeName Unique name of the store matching a string key in the StoreName class.
	 * @return Returns true if the given store is available on this device.
	 *         Returns false if not or if given an unknown store name.
	 */
	public static boolean isInAppStoreAvailable(String storeName) {
		// Do not continue if given an invalid store name.
		if (StoreName.isNotValid(storeName)) {
			return false;
		}
		
		// Determine if the given store is available/accessible on this device.
		if (storeName.equals(StoreName.GOOGLE)) {
			// Check if the Google Marketplace app exists on this device.
			// Note: The marketplace app goes by two different package names.
			if (isPackageNameInstalled(GOOGLE_MARKETPLACE_APP_PACKAGE_NAME_1) ||
			    isPackageNameInstalled(GOOGLE_MARKETPLACE_APP_PACKAGE_NAME_2)) {
				return true;
			}
		}
		else if (storeName.equals(StoreName.AMAZON)) {
			// Corona does not support in-app purchases with Amazon yet.
		}
		
		// Given store is not available on this device.
		return false;
	}
	
	/**
	 * Determines if the given app store is available/accessible on this device.
	 * @param storeName Unique name of the store matching a string key in the StoreName class.
	 * @return Returns true if the given store is available on this device.
	 *         Returns false if not or if given an unknown store name.
	 */
	public static boolean isAppStoreAvailable(String storeName) {
		// Do not continue if given an invalid store name.
		if (StoreName.isNotValid(storeName)) {
			return false;
		}

		// Determine if the given store is available/accessible on this device.
		String[] availableStoreNames = getAvailableAppStoreNames();
		if (availableStoreNames != null) {
			if (java.util.Arrays.binarySearch(availableStoreNames, storeName) >= 0) {
				return true;
			}
		}
		
		// Given store is not available on this device.
		return false;
	}
	
	/**
	 * Gets an array of in-app purchasing stores that are currently available/accessible on this device.
	 * @return Returns an array of strings containing the unique name of each store available on this device.
	 *         Returns null or an empty array if no stores are available.
	 */
	public static String[] getAvailableInAppStoreNames() {
		String[] availableStoreNames = null;
		
		if (isInAppStoreAvailable(StoreName.GOOGLE)) {
			availableStoreNames = new String[1];
			availableStoreNames[0] = StoreName.GOOGLE;
		}
		return availableStoreNames;
	}

	/**
	 * Gets an array of app stores that are currently available/accessible on this device.
	 * These are stores that this application might have been purchased from.
	 * @return Returns an array of strings containing the unique name of each store available on this device.
	 *         Returns null or an empty array if no stores are available.
	 */
	public static String[] getAvailableAppStoreNames() {
		java.util.LinkedHashSet<String> availableStores = new java.util.LinkedHashSet<String>();
		String lowerCaseManufacturerName = android.os.Build.MANUFACTURER.toLowerCase();

		// Find all installed marketplace apps that Corona supports and add them to the collection.
		if (isPackageNameInstalled(GOOGLE_MARKETPLACE_APP_PACKAGE_NAME_1) ||
		    isPackageNameInstalled(GOOGLE_MARKETPLACE_APP_PACKAGE_NAME_2)) {
			availableStores.add(StoreName.GOOGLE);
		}
		if (isPackageNameInstalled(AMAZON_MARKETPLACE_APP_PACKAGE_NAME)) {
			availableStores.add(StoreName.AMAZON);
		}
		if (isPackageNameInstalled(SAMSUNG_MARKETPLACE_APP_PACKAGE_NAME)) {
			availableStores.add(StoreName.SAMSUNG);
		}

		// Add the Nook store to the collection if the device is detected to be a Nook.
		// Note that we must do this because there is no Nook app store for us to find on this device.
		String nookSystemPropertyValue = System.getProperty("ro.nook.manufacturer");
		if ((lowerCaseManufacturerName.contains("barnes") && lowerCaseManufacturerName.contains("noble")) ||
		    ((nookSystemPropertyValue != null) && (nookSystemPropertyValue.length() > 0))) {
			availableStores.add(StoreName.NOOK);
		}

		// Return the results as a string array.
		String[] availableStoreNames = null;
		if (availableStores.size() > 0) {
			availableStoreNames = availableStores.toArray(new String[0]);
		}
		return availableStoreNames;
	}

	/**
	 * Fetches the unique name of the app store/marketplace that this application is targeting.
	 * <p>
	 * This is the store that this application intends to be uploaded and purchased from.
	 * This comes in handy when you need to determine which store services you want to implement at runtime
	 * such as displaying an app rating dialog, selecting an in-app purchase system, validation purchases, etc.
	 * @return Returns a unique name of the store matching a constant in the StoreName class.
	 *         <p>
	 *         Returns StoreName.NONE if an app store is not being targeted by this application.
	 */
	public static String getTargetedAppStoreName() {
		String storeName = null;
		
		// Attempt to fetch the targeted app store from the "AndroidManifest.xml" file.
		// Corona Simulator builds normally add it as meta-data with the application tag.
		try {
			android.content.Context context = com.ansca.corona.CoronaEnvironment.getApplicationContext();
			android.content.pm.PackageManager packageManager = context.getPackageManager();
			android.content.pm.ApplicationInfo applicationInfo = packageManager.getApplicationInfo(
								context.getPackageName(), android.content.pm.PackageManager.GET_META_DATA);
			if ((applicationInfo != null) && (applicationInfo.metaData != null)) {
				storeName = applicationInfo.metaData.getString("targetedAppStore");
				if (storeName != null) {
					storeName = storeName.trim();
				}
			}
		}
		catch (Exception ex) { }

		// If a targeted app store was not provided, then set the store name to none.
		if ((storeName == null) || (storeName.length() <= 0)) {
			storeName = StoreName.NONE;
		}

		// Return the result.
		return storeName;
	}
	
	/**
	 * Fetches the name of the app store/marketplace that installed this application, if known.
	 * @return Returns a unique name of the store matching a constant in the StoreName class.
	 *         Returns StoreName.NONE if the application was not installed by a store/marketplace.
	 *         <p>
	 *         Returns StoreName.UNKNOWN if the application came from an unknown or unsupported store/marketplace.
	 */
	public static String getStoreApplicationWasPurchasedFrom() {
		String storeName = StoreName.NONE;
		
		try {
			android.content.Context context = com.ansca.corona.CoronaEnvironment.getApplicationContext();
			String installerPackageName = context.getPackageManager().getInstallerPackageName(context.getPackageName());
			storeName = StoreName.fromPackageName(installerPackageName);
		}
		catch (Exception ex) { }
		
		return storeName;
	}

	/**
	 * Determines if the given application exists on the device.
	 * @param packageName The full package name of the application to check for. Cannot be null or empty.
	 * @return Returns true if the application is installed on the device. Returns false if not or given an invalid argument.
	 */
	private static boolean isPackageNameInstalled(String packageName) {
		// Invoke PackageServices for this
		com.ansca.corona.storage.PackageServices packageServices = 
			new com.ansca.corona.storage.PackageServices(com.ansca.corona.CoronaEnvironment.getApplicationContext());
		return packageServices.isPackageNameInstalled(packageName);
	}
}
