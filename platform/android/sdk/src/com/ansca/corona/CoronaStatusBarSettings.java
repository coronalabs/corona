//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
