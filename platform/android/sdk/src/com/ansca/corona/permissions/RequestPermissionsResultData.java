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

import com.ansca.corona.CoronaActivity;
import java.util.ArrayList;

/**
 * Stores data provided by a Permissions Request.
 * TODO: IMPROVE DOCS FOR ENTIRE CLASS and make this public when the full feature set is available to Enterprise.
 */

public class RequestPermissionsResultData {

	// The permissions settings that go along with this request.
	private PermissionsSettings fPermissionsSettings = null;

	// The permissions that were granted by this request.
	private ArrayList<String> fGrantedPermissions = null;

	// The permissions that were denied by this request.
	private ArrayList<String> fDeniedPermissions = null;

	// Whether the user checked the "Never Ask Again" checkbox.
	private boolean fUserHitNeverAskAgain = false;

	// The request code for this request.
	private int fRequestCode = -1;

	// The CoronaActivity instance associated with this data.
	private CoronaActivity fCoronaActivity = null;

	// The RequestPermissionsResultHandler used to generate this data.
	private CoronaActivity.OnRequestPermissionsResultHandler fRequestPermissionsResultHandler = null;

	public RequestPermissionsResultData(
		PermissionsSettings permissionsSettings,
		ArrayList<String> grantedPermissions,
		ArrayList<String> deniedPermissions,
		boolean userHitNeverAskAgain, 
		int requestCode,
		CoronaActivity coronaActivity,
		CoronaActivity.OnRequestPermissionsResultHandler requestPermissionsResultHandler) {

		fPermissionsSettings = permissionsSettings;
		fGrantedPermissions = grantedPermissions;
		fDeniedPermissions = deniedPermissions;
		fUserHitNeverAskAgain = userHitNeverAskAgain;
		fRequestCode = requestCode;
		fCoronaActivity = coronaActivity;
		fRequestPermissionsResultHandler = requestPermissionsResultHandler;
	}

	public PermissionsSettings getPermissionsSettings() {
		return fPermissionsSettings;
	}

	public ArrayList<String> getGrantedPermissions() {
		return fGrantedPermissions;
	}

	public ArrayList<String> getDeniedPermissions() {
		return fDeniedPermissions;
	}

	public boolean getUserHitNeverAskAgain() {
		return fUserHitNeverAskAgain;
	}

	public int getReuestCode() {
		return fRequestCode;
	}

	public CoronaActivity getCoronaActivity() {
		return fCoronaActivity;
	}

	public CoronaActivity.OnRequestPermissionsResultHandler getRequestPermissionsResultHandler() {
		return fRequestPermissionsResultHandler;
	}
}