//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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