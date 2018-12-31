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
 * Provides unique store names used to identify the store that installed the application and the marketplace apps
 * that are installed/available on the device.
 * <p>
 * You cannot create instances of this class. This class only provides constants and static methods.
 */
public class StoreName {
	/** Constructor made private to prevent objects from being made. */
	private StoreName() {
	}
	
	/**
	 * Indicates that there is no store.
	 * Typically used to identify that an application did not come from a store an was installed by hand.
	 */
	public static final String NONE = "none";
	
	/** Indicates that the store type is unknown and not supported by Corona. */
	public static final String UNKNOWN = "unknown";
	
	/** Identifies a store as Google's Android Marketplace. */
	public static final String GOOGLE = "google";
	
	/** Identifies a store as Amazon's App Store. */
	public static final String AMAZON = "amazon";
	
	/** Identifies a store as Barnes and Noble's Nook App Store. */
	public static final String NOOK = "nook";
	
	/** Identifies a store as Samsung's App Store. */
	public static final String SAMSUNG = "samsung";
	
	
	/**
	 * Determines if the given store name is of a known type.
	 * @param storeName The unique name of the store to check.
	 * @return Returns true if the given store name matches a string constant in this class.
	 *         Returns false if the given store name is of type NONE, UNKOWN, an unknown string, empty, or null.
	 */
	public static boolean isValid(String storeName) {
		if (storeName != null) {
			if (storeName.equals(StoreName.GOOGLE) ||
				storeName.equals(StoreName.AMAZON) ||
				storeName.equals(StoreName.NOOK) ||
				storeName.equals(StoreName.SAMSUNG)) {
				return true;
			}
		}
		return false;
	}
	
	/**
	 * Determines if the given store name is of an unknown or invalid type.
	 * @param storeName The unique name of the store to check.
	 * @return Returns true if the given store name is of type NONE, UNKOWN, an unknown string, empty, or null.
	 *         Returns false if the given store name matches a string constant in this class.
	 */
	public static boolean isNotValid(String storeName) {
		return !isValid(storeName);
	}
	
	/**
	 * Fetches the unique store name for the given application package name.
	 * @param packageName Expected to be a package name of a marketplace application.
	 * @return Returns the name of the store for the given package name if known, such as GOOGLE or AMAZON.
	 *         Returns NONE if given a null or empty package name string.
	 *         Returns UNKNOWN if the given package name is unknown and not supported by Corona.
	 */
	public static String fromPackageName(String packageName) {
		String storeName;
		
		// Determine what kind of store the application package name represents.
		if ((packageName == null) || (packageName.length() <= 0)) {
			storeName = StoreName.NONE;
		}
		else if (packageName.equals(StoreServices.GOOGLE_MARKETPLACE_APP_PACKAGE_NAME_1) ||
		         packageName.equals(StoreServices.GOOGLE_MARKETPLACE_APP_PACKAGE_NAME_2)) {
			storeName = StoreName.GOOGLE;
		}
		else if (packageName.equals(StoreServices.AMAZON_MARKETPLACE_APP_PACKAGE_NAME)) {
			storeName = StoreName.AMAZON;
		}
		else if (packageName.equals(StoreServices.SAMSUNG_MARKETPLACE_APP_PACKAGE_NAME)) {
			storeName = StoreName.SAMSUNG;
		}
		else {
			storeName = StoreName.UNKNOWN;
		}
		return storeName;
	}
}
