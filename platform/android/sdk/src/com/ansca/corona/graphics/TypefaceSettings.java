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


/**
 * Stores modifiable settings for one font typeface object such as its font name and whether or
 * not it should be rendered in bold and italic.
 */
public class TypefaceSettings implements Cloneable {
	/** The font's family name or file name. */
	private String fName;

	/** Set to true for bold text. */
	private boolean fIsBold;

	/** Set to true for italic text. */
	private boolean fIsItalic;


	/** Creates a new font typeface settings object. */
	public TypefaceSettings() {
		fName = null;
		fIsBold = false;
		fIsItalic = false;
	}

	/**
	 * Creates a new copy of this object.
	 * @return Returns a copy of this object.
	 */
	@Override
	public TypefaceSettings clone() {
		TypefaceSettings clone = null;
		try {
			clone = (TypefaceSettings)super.clone();
		}
		catch (Exception ex) { }
		return clone;
	}

	/**
	 * Gets the font's family name or file name.
	 * @return Returns the font name.
	 *         <p>
	 *         Returns null if the font name was not set.
	 */
	public String getName() {
		return fName;
	}

	/**
	 * Sets the name of the font to use.
	 * @param name The font family name or font file name, minus the file extension.
	 */
	public void setName(String name) {
		fName = name;
	}

	/**
	 * Determines if the font is bold.
	 * @return Returns true if the font is bold. Returns false if not.
	 */
	public boolean isBold() {
		return fIsBold;
	}

	/**
	 * Sets whether or not the font will be rendered with a bold style.
	 * @param value Set to true to use a bold font style. Set to false to not use a bold style.
	 */
	public void setIsBold(boolean value) {
		fIsBold = value;
	}

	/**
	 * Determines if the font is italicized.
	 * @return Returns true if the font is italic. Returns false if not.
	 */
	public boolean isItalic() {
		return fIsItalic;
	}

	/**
	 * Sets whether or not the font will be rendered with an italic style.
	 * @param value Set to true use an italic font style. Set to false to not use an italic style.
	 */
	public void setIsItalic(boolean value) {
		fIsItalic = value;
	}

	/**
	 * Combines the bold and italic font styles as a single bit field matching the constants
	 * in the Android "Typeface" class.
	 * @return Returns a bit field of font styles that can be used by an Android Typeface object.
	 */
	public int getAndroidTypefaceStyle() {
		int style = 0;
		if (fIsBold) {
			style |= android.graphics.Typeface.BOLD;
		}
		if (fIsItalic) {
			style |= android.graphics.Typeface.ITALIC;
		}
		return style;
	}

	/**
	 * Determines if the given settings exactly matches this object's settings.
	 * @param settings Reference to the settings to be compared with this object.
	 * @return Returns true if the given settings exactly matches this object's settings.
	 *         <p>
	 *         Returns false if given a null argument or if the given settings does not
	 *         match this object's settings.
	 */
	public boolean equals(TypefaceSettings settings) {
		// First check if this object was given a reference to itself. Always equal in this case.
		if (settings == this) {
			return true;
		}

		// Not equal if given a null reference.
		if (settings == null) {
			return false;
		}

		// Compare font names.
		if (((fName == null) && (settings.fName != null)) ||
		    ((fName != null) && (settings.fName == null)))
		{
			return false;
		}
		else if ((fName != null) && (fName.equals(settings.fName) == false)) {
			return false;
		}

		// Compare font styles.
		if ((fIsBold != settings.fIsBold) || (fIsItalic != settings.fIsItalic)) {
			return false;
		}

		// The given settings matches this object's settings.
		return true;
	}

	/**
	 * Determines if the given object matches this object.
	 * @param value Reference to the object to compare against this object.
	 * @return Returns true if the given object is of type TypefaceSettings and its information
	 *         exactly matches this object's settings.
	 *         <p>
	 *         Returns false if given a null reference, an object that is not of type TypefaceSettings,
	 *         or if the given settings does not match this object's settings.
	 */
	@Override
	public boolean equals(Object value) {
		if ((value instanceof TypefaceSettings) == false) {
			return false;
		}
		return equals((TypefaceSettings)value);
	}

	/**
	 * Gets an integer hash code for this object.
	 * @return Returns this object's hash code.
	 */
	@Override
	public int hashCode() {
		int hashCode = getAndroidTypefaceStyle();
		if (fName != null) {
			hashCode ^= fName.hashCode();
		}
		return hashCode;
	}
}
