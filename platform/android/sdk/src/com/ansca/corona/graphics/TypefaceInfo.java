//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.graphics;


/**
 * Stores information about one font typeface such as its font name and bold/italic font styles.
 * <p>
 * Instances of this class are immutable.
 */
public class TypefaceInfo {
	/** Stores a clone of the typeface's configuration. */
	private TypefaceSettings fSettings;


	/**
	 * Creates an object storing the information for one font typeface.
	 * @param settings The typeface settings to be copied.
	 *                 <p>
	 *                 Cannot be null or else an exception will be thrown.
	 */
	public TypefaceInfo(TypefaceSettings settings) {
		if (settings == null) {
			throw new NullPointerException();
		}
		fSettings = settings.clone();
	}

	/**
	 * Gets the font's family name or file name.
	 * @return Returns the font name.
	 *         <p>
	 *         Returns null if the font name was not set.
	 */
	public String getName() {
		return fSettings.getName();
	}

	/**
	 * Determines if the font is bold.
	 * @return Returns true if the font is bold. Returns false if not.
	 */
	public boolean isBold() {
		return fSettings.isBold();
	}

	/**
	 * Determines if the font is italicized.
	 * @return Returns true if the font is italic. Returns false if not.
	 */
	public boolean isItalic() {
		return fSettings.isItalic();
	}

	/**
	 * Combines the bold and italic font styles as a single bit field matching the constants
	 * in the Android "Typeface" class.
	 * @return Returns a bit field of font styles that can be used by an Android Typeface object.
	 */
	public int getAndroidTypefaceStyle() {
		return fSettings.getAndroidTypefaceStyle();
	}

	/**
	 * Determines if the given typeface information exactly matches this object's information.
	 * @param info Reference to the typeface information to be compared with this object.
	 * @return Returns true if the given information exactly matches this object's information.
	 *         <p>
	 *         Returns false if given a null argument or if the given information does not
	 *         match this object's information.
	 */
	public boolean equals(TypefaceInfo info) {
		if (info == null) {
			return false;
		}
		return equals(info.fSettings);
	}

	/**
	 * Determines if the given typeface settings exactly matches this object's typeface information.
	 * @param settings Reference to the typeface settings to be compared with this object.
	 * @return Returns true if the given settings exactly matches this object's information.
	 *         <p>
	 *         Returns false if given a null argument or if the given information does not
	 *         match this object's information.
	 */
	public boolean equals(TypefaceSettings settings) {
		if (settings == null) {
			return false;
		}
		return fSettings.equals(settings);
	}

	/**
	 * Determines if the given object matches this object.
	 * @param value Reference to the object to compare against this object.
	 * @return Returns true if the given object is of type TypefaceInfo or TypefaceSettings and
	 *         its typeface information exactly matches this object's information.
	 *         <p>
	 *         Returns false if given a null reference, an object that is not of type TypefaceInfo
	 *         or TypefaceSettings, or if the given information does not match this object's information.
	 */
	@Override
	public boolean equals(Object value) {
		if (value instanceof TypefaceInfo) {
			return equals((TypefaceInfo)value);
		}
		else if (value instanceof TypefaceSettings) {
			return equals((TypefaceSettings)value);
		}
		return false;
	}

	/**
	 * Gets an integer hash code for this object.
	 * @return Returns this object's hash code.
	 */
	@Override
	public int hashCode() {
		return fSettings.hashCode();
	}
}
