//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.graphics;


/** Class used to easily render text to a bitmap. */
public class TextRenderer {
	/** Context needed to access the font and its typeface. */
	private android.content.Context fContext;

	/** Stores font settings such as font name, font size, bold style, etc. */
	private FontSettings fFontSettings;

	/** The text alignment such as LEFT, CENTER, or RIGHT. */
	private HorizontalAlignment fHorizontalAlignment;

	/** Number of pixels wide where text wrapping will occur. Set to zero for no wrapping. */
	private int fWrapWidth;

	/** Number of pixels wide until text is clipped. Set to zero for no clipping. */
	private int fClipWidth;

	/** Number of pixels high until text is clipped. Set to zero for no clipping. */
	private int fClipHeight;

	/** The text to be rendered. */
	private String fText;

	private float fBaselineOffset;

	/** Static collection of reusable TextPaint objects to be used by all instances of this text renderer. */
	private static java.util.HashMap<android.graphics.Typeface, android.text.TextPaint> sTextPaintCollection =
						new java.util.HashMap<android.graphics.Typeface, android.text.TextPaint>();


	/**
	 * Creates a new text renderer.
	 * @param context Context needed to access the system's fonts. Cannot be null.
	 */
	public TextRenderer(android.content.Context context) {
		// Validate.
		if (context == null) {
			throw new NullPointerException();
		}

		// Initialize member variables.
		fContext = context;
		fFontSettings = new FontSettings();
		fHorizontalAlignment = HorizontalAlignment.LEFT;
		fWrapWidth = 0;
		fClipWidth = 0;
		fClipHeight = 0;
		fText = "";
	}

	/**
	 * Gets the context used to access the system's fonts.
	 * @return Returns a reference to the Android context.
	 */
	public android.content.Context getContext() {
		return fContext;
	}

	/**
	 * Gets the modifiable font settings that this text renderer will use.
	 * <p>
	 * The caller is expected to set the font name, font size, and font style via the returned settings object.
	 * @return Returns this text renderer's font settings.
	 */
	public FontSettings getFontSettings() {
		return fFontSettings;
	}

	/**
	 * Gets the horizontal alignment that this renderer will use.
	 * @return Returns the alignment such as LEFT, CENTER, or RIGHT.
	 */
	public HorizontalAlignment getHorizontalAlignment() {
		return fHorizontalAlignment;
	}

	/**
	 * Sets the horizontal alignment that this renderer will use.
	 * @param value The alignment such as LEFT, CENTER, or RIGHT.
	 *              <p>
	 *              Cannot be null or else an exception will be thrown.
	 */
	public void setHorizontalAlignment(HorizontalAlignment value) {
		if (value == null) {
			throw new NullPointerException();
		}
		fHorizontalAlignment = value;
	}

	/**
	 * Gets the width in pixels that text wrapping will occur on.
	 * @return Returns the number of pixels wide that text wrapping will occur on.
	 *         <p>
	 *         Returns 0 if text wrapping is not enabled.
	 */
	public int getWrapWidth() {
		return fWrapWidth;
	}

	/**
	 * Sets the number of pixels wide until the text wraps to the next line.
	 * @param value The number of pixels until text should wrap to the next line.
	 *              <p>
	 *              Set to 0 to disable text wrapping.
	 */
	public void setWrapWidth(int value) {
		if (value < 0) {
			value = 0;
		}
		fWrapWidth = value;
	}

	/**
	 * Gets the number of pixels until the render clips text.
	 * <p>
	 * Note that the bitmap returned by createBitmap() will never be larger than this width.
	 * @return Returns the number of pixels wide until this renderer clips text.
	 *         <p>
	 *         Returns 0 if clipping is not enabled, meaning that this renderer will draw
	 *         text as wide as the given text requires.
	 */
	public int getClipWidth() {
		return fClipWidth;
	}

	/**
	 * Sets the number of pixels until this renderer clips text.
	 * <p>
	 * Note that the bitmap returned by createBitmap() will never be larger than this width.
	 * @param value The number of pixels wide until this renderer clips text.
	 *              <p>
	 *              Set to 0 disable clipping. This means that this renderer will draw text
	 *              as wide as the text requires.
	 *              <p>
	 *              If you are rendering text for OpenGL, then this width should be set to
	 *              the maximum texture size.
	 */
	public void setClipWidth(int value) {
		if (value < 0) {
			value = 0;
		}
		fClipWidth = value;
	}

	/**
	 * Gets the number of pixels until the render clips text.
	 * <p>
	 * Note that the bitmap returned by createBitmap() will never be larger than this height.
	 * @return Returns the number of pixels high until this renderer clips text.
	 *         <p>
	 *         Returns 0 if clipping is not enabled, meaning that this renderer will draw
	 *         text as wide as the given text requires.
	 */
	public int getClipHeight() {
		return fClipHeight;
	}

	/**
	 * Sets the number of pixels until this renderer clips text.
	 * <p>
	 * Note that the bitmap returned by createBitmap() will never be larger than this height.
	 * @param value The number of pixels high until this renderer clips text.
	 *              <p>
	 *              Set to 0 disable clipping. This means that this renderer will draw text
	 *              as high as the text requires.
	 *              <p>
	 *              If you are rendering text for OpenGL, then this height should be set to
	 *              the maximum texture size.
	 */
	public void setClipHeight(int value) {
		if (value < 0) {
			value = 0;
		}
		fClipHeight = value;
	}

	/**
	 * Gets the text to be rendered.
	 * @return Returns the text to be rendered.
	 */
	public String getText() {
		return fText;
	}

	/**
	 * Sets the text to be rendered.
	 * @param value Reference to the text to be rendered. Can be null or empty string.
	 */
	public void setText(String value) {
		if (value == null) {
			value = "";
		}
		fText = value;
	}

	public float getBaselineOffset(){
		return fBaselineOffset;
	}
	/**
	 * Creates a new bitmap with text drawn to it according to the settings applied to this renderer.
	 * @return Returns a bitmap with text drawn to it.
	 *         <p>
	 *         Returns null if setText() was given a null or empty string.
	 */
	public android.graphics.Bitmap createBitmap() {
		// Do not continue if this renderer has no text to render or invalid configuration.
		if ((fText == null) || (fText.length() <= 0) || (fFontSettings.getPointSize() <= 0)) {
			return null;
		}

		// Fetch the typeface for the given font settings.
		FontServices fontServices = new FontServices(fContext);
		if (fontServices == null) {
			return null;
		}
		android.graphics.Typeface typeface = fontServices.fetchTypefaceFor(fFontSettings);
		if (typeface == null) {
			return null;
		}

		// Fetch a text painter for the current font settings.
		android.text.TextPaint textPaint;
		synchronized (sTextPaintCollection) {
			// Attempt to fetch a cached text painter.
			textPaint = sTextPaintCollection.get(typeface);
		}
		if (textPaint == null) {
			// Cached text painter not found. Create a new one and cache it to the collection.
			textPaint = new android.text.TextPaint();
			textPaint.setARGB(255, 255, 255, 255);
			textPaint.setAntiAlias(true);
			textPaint.setTextAlign(android.graphics.Paint.Align.LEFT);
			textPaint.setTypeface(typeface);
			synchronized (sTextPaintCollection) {
				sTextPaintCollection.put(typeface, textPaint);
			}
		}

		// Apply this renderer's font size to the text painter.
		textPaint.setTextSize(fFontSettings.getPointSize());

		// Determine how wide the bitmap needs to be to fit all of the rendered text.
		int width = fWrapWidth;
		if (width <= 0) {
			// A text wrapping width has not been provided. Calculate a width that will fit all of the text.
			// Note: The below method can return a fractional width, so we need to round up to the next pixel.
			width = (int)(android.text.StaticLayout.getDesiredWidth(fText, textPaint) + 1.0f);
			if (width < 1) {
				width = 1;
			}
		}
		
		// Create a text layout. This does text wrapping for us and determines the pixel height required.
		android.text.StaticLayout layout = new android.text.StaticLayout(
					fText, textPaint, width, fHorizontalAlignment.toAndroidTextLayoutAlignment(), 1, 1, true);
		
		// Calculate a pixel width and height for the bitmap that the above text layout will be drawn to.
		// This will constrain the width and height to the given clipping width and height.
		int height = fClipHeight;
		if (height <= 0) {
			height = layout.getHeight();
			if (height < 1) {
				height = 1;
			}
		}
		if ((fClipWidth > 0) && (width > fClipWidth)) {
			width = fClipWidth;
		}
		if ((fClipHeight > 0) && (height > fClipHeight)) {
			height = fClipHeight;
		}

		// Round the pixel width up/down if it does not fit within the byte packing alignment.
		// This must be done for 8-bit grayscale images or else rendering issues will occur.
		int bytePackingAlignmentDelta = width % 4;
		if (bytePackingAlignmentDelta > 0) {
			width += 4 - bytePackingAlignmentDelta;
			if (width > fClipWidth) {
				width -= 4;
			}
		}
		
		// Draw the text to a bitmap.
		android.graphics.Bitmap bitmap = null;
		try {
			bitmap = android.graphics.Bitmap.createBitmap(width, height, android.graphics.Bitmap.Config.ALPHA_8);
			android.graphics.Canvas canvas = new android.graphics.Canvas(bitmap);
			layout.draw(canvas);
			fBaselineOffset = height*0.5f + textPaint.getFontMetrics().top;
		}
		catch (Exception ex) {
			ex.printStackTrace();
		}

		// Return a bitmap with the rendered text.
		return bitmap;
	}
}
