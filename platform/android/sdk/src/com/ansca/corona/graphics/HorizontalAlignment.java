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
 * Indicates if text/graphics should be horizontally aligned to the left, center, or right.
 * Also, provides easy conversion to different Android alignment types when used with
 * layout, paint, or gravity objects.
 * <p>
 * Instances of this class are immutable.
 * <p>
 * You cannot create instances of this class.
 * Instead, you use the pre-allocated objects from this class' static methods and fields.
 */
public class HorizontalAlignment {
	/** Corona's unique string ID for this alignment type. */
	private String fCoronaStringId;

	/** Bit flag for this alignment as defined by Android's Gravity class constants. */
	private int fAndroidGravityBitField;

	/** Android's unique "Layout.Alignment" enum value for this alignment. */
	private android.text.Layout.Alignment fAndroidTextLayoutAlignment;

	/** Android's unique "Paint.Align" enum value for this alignment. */
	private android.graphics.Paint.Align fAndroidPaintAlignment;

	
	/**
	 * Creates a new alignment object.
	 * @param coronaStringId Corona's unique string ID for this alignment.
	 * @param androidGravityBitField Bit flag for this alignment as defined by Android's Gravity class.
	 * @param androidTextLayoutAlignment Android's unique text layout alignment ID.
	 * @param androidPaintAlignment Android's unique paint alignment ID.
	 */
	private HorizontalAlignment(
		String coronaStringId, int androidGravityBitField,
		android.text.Layout.Alignment androidTextLayoutAlignment,
		android.graphics.Paint.Align androidPaintAlignment)
	{
		fCoronaStringId = coronaStringId;
		fAndroidGravityBitField = androidGravityBitField;
		fAndroidTextLayoutAlignment = androidTextLayoutAlignment;
		fAndroidPaintAlignment = androidPaintAlignment;
	}
	

	/** Indicates that the text/graphics should be left aligned. */
	public static final HorizontalAlignment LEFT = new HorizontalAlignment(
					"left", android.view.Gravity.LEFT,
					android.text.Layout.Alignment.ALIGN_NORMAL,
					android.graphics.Paint.Align.LEFT);

	/** Indicates that the text/graphics should be centered. */
	public static final HorizontalAlignment CENTER = new HorizontalAlignment(
					"center", android.view.Gravity.CENTER_HORIZONTAL,
					android.text.Layout.Alignment.ALIGN_CENTER,
					android.graphics.Paint.Align.CENTER);
	
	/** Indicates that the text/graphics should be right aligned. */
	public static final HorizontalAlignment RIGHT = new HorizontalAlignment(
					"right", android.view.Gravity.RIGHT,
					android.text.Layout.Alignment.ALIGN_OPPOSITE,
					android.graphics.Paint.Align.RIGHT);

//TODO: We should add NEAR/NORMAL and FAR/OPPOSITE types to support right-to-left languages.

	/**
	 * Gets a bit flag for this alignment matching a constant in Android's "Gravity" class.
	 * @return Returns Gravity.LEFT, Gravity.RIGHT, or Gravity.CENTER_HORIZONTAL.
	 */
	public int toAndroidGravityBitField() {
		return fAndroidGravityBitField;
	}
	
	/**
	 * Gets Android's unique text layout ID for this alignment.
	 * @return Returns Alignment.ALIGN_NORMAL, Alignment.ALIGN_CENTER, or Alignment.ALIGN_OPPOSITE.
	 */
	public android.text.Layout.Alignment toAndroidTextLayoutAlignment() {
		return fAndroidTextLayoutAlignment;
	}

	/**
	 * Gets Android's unique paint alignment ID for this object.
	 * @return Returns Align.LEFT, Align.CENTER, or Align.RIGHT.
	 */
	public android.graphics.Paint.Align toAndroidPaintAlignment() {
		return fAndroidPaintAlignment;
	}

	/**
	 * Gets Corona's unique string ID for this alignment.
	 * <p>
	 * This string ID is used in Lua for settings the alignment of text bitmaps and text fields.
	 * @return Returns a unique string ID such as "left", "center", or "right".
	 */
	public String toCoronaStringId() {
		return fCoronaStringId;
	}

	/**
	 * Gets Corona's unique string ID for this alignment.
	 * <p>
	 * This string ID is used in Lua for settings the alignment of text bitmaps and text fields.
	 * @return Returns a unique string ID such as "left", "center", or "right".
	 */
	@Override
	public String toString() {
		return fCoronaStringId;
	}

	/**
	 * Gets an alignment object matching the given Corona string ID.
	 * @param value Unique string ID for the alignment such as "left", "center", or "right".
	 * @return Returns an alignment object matching the given string ID.
	 *         <p>
	 *         Returns null if the given string ID is unknown.
	 */
	public static HorizontalAlignment fromCoronaStringId(String value) {
		// Return a pre-allocated object matching the given string ID via reflection.
		try {
			for (java.lang.reflect.Field field : HorizontalAlignment.class.getDeclaredFields()) {
				if (field.getType().equals(HorizontalAlignment.class)) {
					HorizontalAlignment alignment = (HorizontalAlignment)field.get(null);
					if (alignment.fCoronaStringId.equals(value)) {
						return alignment;
					}
				}
			}
		}
		catch (Exception ex) { }
		
		// The given string ID is unknown.
		return null;
	}
}
