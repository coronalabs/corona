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

package com.ansca.corona.maps;

public class MapMarker {
	int fMarkerId;
	int fListener;
	
	double fLatitude;
	double fLongitude;

	String fTitle;
	String fSubtitle;
	String fImageFile;

	public MapMarker(double latitude, double longitude) {
		fMarkerId = -1;
		fListener = com.ansca.corona.CoronaLua.REFNIL;
		fLatitude = latitude;
		fLongitude = longitude;
		fSubtitle = null;
		fTitle = null;
		fImageFile = null;
	}

	public MapMarker(int markerId, double latitude, double longitude, String title, String subtitle, int listener, String imageFile) {
		fMarkerId = markerId;
		fListener = listener;
		fLatitude = latitude;
		fLongitude = longitude;
		fTitle = title;
		fSubtitle = subtitle;
		fImageFile = imageFile;
	}
	
	public int getMarkerId() {
		return fMarkerId;
	}

	public int getListener() {
		return fListener;
	}

	public double getLongitude() {
		return fLongitude;
	}

	public double getLatitude() {
		return fLatitude;
	}

	public String getTitle() {
		return fTitle;
	}

	public String getSubtitle() {
		return fSubtitle;
	}

	public String getImageFile() {
		return fImageFile;
	}

	public void setLongitude(double longitude) {
		fLongitude = longitude;
	}

	public void setLatitude(double latitude) {
		fLatitude = latitude;
	}

	public void setMarkerId(int markerId) {
		fMarkerId = markerId;
	}

	public void setListener(int listener) {
		fListener = listener;
	}

	public void setTitle(String title) {
		fTitle = title;
	}

	public void setSubtitle(String subtitle) {
		fSubtitle = subtitle;
	}

	public void setImageFile(String imageFile) {
		fImageFile = imageFile;
	}

	public void deleteRef(com.ansca.corona.CoronaRuntime runtime) {
		if (runtime == null || fListener == com.ansca.corona.CoronaLua.REFNIL) {
			return;
		}
		com.naef.jnlua.LuaState L = runtime.getLuaState();

		if (L == null) {
			return;
		}

		com.ansca.corona.CoronaLua.deleteRef(L, fListener);
	}
}
