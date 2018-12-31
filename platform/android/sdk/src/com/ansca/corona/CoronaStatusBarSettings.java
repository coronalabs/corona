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

package com.ansca.corona;

public class CoronaStatusBarSettings {
	public static final CoronaStatusBarSettings HIDDEN = new CoronaStatusBarSettings(0);
	public static final CoronaStatusBarSettings DEFAULT = new CoronaStatusBarSettings(1);
	public static final CoronaStatusBarSettings TRANSLUCENT = new CoronaStatusBarSettings(2);
	public static final CoronaStatusBarSettings DARK = new CoronaStatusBarSettings(3);
	public static final CoronaStatusBarSettings LIGHT_TRANSPARENT = new CoronaStatusBarSettings(4);
	public static final CoronaStatusBarSettings DARK_TRANSPARENT = new CoronaStatusBarSettings(5);

	private int fValue;

	private CoronaStatusBarSettings(int value) {
		fValue = value;
	}

	private int toIntegerValue() {
		return fValue;
	}

	public static CoronaStatusBarSettings fromCoronaIntId(int value) {
		CoronaStatusBarSettings setting = null;
		switch (value) {
			case 0:
				return HIDDEN;
			case 1:
				return DEFAULT;
			case 2:
				return TRANSLUCENT;
			case 3:
				return DARK;
			case 4:
				return LIGHT_TRANSPARENT;
			case 5:
				return DARK_TRANSPARENT;
		}
		
		return null;
	}

	public int toCoronaIntId() {
		return fValue;
	}
}


// ----------------------------------------------------------------------------
