//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.storage;

/**
 * Enum giving the state of a package on an Android device.
 * <p>
 * <b>Added in <a href="http://developer.coronalabs.com/release-ent/2016/2932/">daily build 2016.2932</a></b>.
 */
public enum PackageState {
	/**
	 * Package is installed on the device and enabled.
	 */
	ENABLED,

	/**
	 * Package is installed on the device, but disabled in Settings.
	 */
	DISABLED,

	/**
	 * The package is not installed on the device.
	 */
	MISSING
}