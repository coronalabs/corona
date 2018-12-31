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

package com.ansca.corona.graphics;


/** Stores modifiable settings for one font such as its font name, font size, and its bold/italic style. */
public class FontSettings extends TypefaceSettings implements Cloneable {
	/** The font's point size. */
	private float fPointSize;


	/** Creates a new font settings object. */
	public FontSettings() {
		super();
		fPointSize = 8.0f;
	}

	/**
	 * Creates a new copy of this object.
	 * @return Returns a copy of this object.
	 */
	@Override
	public FontSettings clone() {
		FontSettings clone = null;
		try {
			clone = (FontSettings)super.clone();
		}
		catch (Exception ex) { }
		return clone;
	}

	/**
	 * Gets the font's point size.
	 * @return Returns the point size of the font.
	 */
	public float getPointSize() {
		return fPointSize;
	}

	/**
	 * Sets the font's point size.
	 * @param value The font's point size. Must be greater than or equal to 1.
	 */
	public void setPointSize(float value) {
		if (value < 1.0f) {
			value = 1.0f;
		}
		fPointSize = value;
	}

	/**
	 * Determines if the given font settings exactly matches this object's settings.
	 * @param settings Reference to the font settings to be compared with this object.
	 * @return Returns true if the given settings exactly matches this object's settings.
	 *         <p>
	 *         Returns false if given a null argument or if the given settings does not
	 *         match this object's settings.
	 */
	public boolean equals(FontSettings settings) {
		// First check if this object was given a reference to itself. Always equal in this case.
		if (settings == this) {
			return true;
		}

		// Not equal if given a null reference.
		if (settings == null) {
			return false;
		}

		// Compare font sizes.
		// Note: Differentiating font sizes in increments of 0.1 is good enough.
		final float pointSizeEpsilon = 0.1f;
		float deltaPointSize = fPointSize - settings.fPointSize;
		if ((deltaPointSize >= pointSizeEpsilon) || (deltaPointSize <= pointSizeEpsilon)) {
			return false;
		}

		// Compare the typeface settings.
		return super.equals(settings);
	}

	/**
	 * Determines if the given object matches this object.
	 * @param value Reference to the object to compare against this object.
	 * @return Returns true if the given object is of type FontSettings and its information
	 *         exactly matches this object's settings.
	 *         <p>
	 *         Returns false if given a null reference, an object that is not of type FontSettings,
	 *         or if the given settings does not match this object's settings.
	 */
	@Override
	public boolean equals(Object value) {
		if ((value instanceof FontSettings) == false) {
			return false;
		}
		return equals((FontSettings)value);
	}

	/**
	 * Gets an integer hash code for this object.
	 * @return Returns this object's hash code.
	 */
	@Override
	public int hashCode() {
		return super.hashCode() ^ Float.valueOf(fPointSize).hashCode();
	}
}
