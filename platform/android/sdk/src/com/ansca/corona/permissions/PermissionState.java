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