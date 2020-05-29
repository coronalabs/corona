//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.permissions;

/**
 * Enum depicting all possible states a permission can be in.
 * The Android framework does not provide a distinction between a permission being 
 * denied, and it being missing from the 
 * <a href="http://developer.android.com/guide/topics/manifest/manifest-intro.html">AndroidManifest.xml</a>, 
 * so we provide the missing portion.
 * <p>
 * <b>Added in <a href="http://developer.coronalabs.com/release-ent/2016/2828/">daily build 2016.2828</a></b>.
 */
public enum PermissionState {
	/**
	 * Permission is missing from 
	 * <a href="http://developer.android.com/guide/topics/manifest/manifest-intro.html">AndroidManifest.xml</a>.
	 * <p>
	 * &emsp; Constant value: 1
	 */
	MISSING(1),

	/**
	 * Corresponds to <a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#PERMISSION_DENIED">android.content.pm.PackageManager.PERMISSION_DENIED</a>
	 * <p>
	 * &emsp; Constant value: -1
	 */
	DENIED(android.content.pm.PackageManager.PERMISSION_DENIED),

	/**
	 * Corresponds to <a href="http://developer.android.com/reference/android/content/pm/PackageManager.html#PERMISSION_GRANTED">android.content.pm.PackageManager.PERMISSION_GRANTED</a>
	 * <p>
	 * &emsp; Constant value: 0
	 */
	GRANTED(android.content.pm.PackageManager.PERMISSION_GRANTED);

	private int mIntValue;

	PermissionState(int value) {
		mIntValue = value;
	}

	/**
	 * Returns the constant value of the desired PermissionState.
	 */
	public int getValue() {
		return mIntValue;
	}
}