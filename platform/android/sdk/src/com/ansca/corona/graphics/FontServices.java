//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.graphics;


/** Provides easy access to the system's fonts and typefaces. */
public class FontServices extends com.ansca.corona.ApplicationContextProvider {
	/** Dictionary of re-usable typeface objects using a TypefaceInfo object as the key. */
	private static java.util.HashMap<TypefaceInfo, android.graphics.Typeface> sTypefaceCollection =
								new java.util.HashMap<TypefaceInfo, android.graphics.Typeface>(); //DO NOT RENAME; reflection modified by Live Builds


	/**
	 * Creates an object that provides access to the system's fonts for easy retrieval.
	 * @param context Reference to an Android created context used to access the system's fonts.
	 *                <p>
	 *                Setting this to null will cause an exception to be thrown.
	 */
	public FontServices(android.content.Context context) {
		super(context);
	}

	/**
	 * Fetches a typeface matching the given font settings.
	 * @param settings Settings indicating the font name and font style.
	 * @return Returns a typeface matching the given font settings.
	 *         <p>
	 *         Returns null if the requested font could not be found or if given a null argument.
	 */
	public android.graphics.Typeface fetchTypefaceFor(TypefaceSettings settings) {
		// Validate.
		if (settings == null) {
			return null;
		}

		// Fetch the requested typeface.
		return fetchTypefaceFor(new TypefaceInfo(settings));
	}

	/**
	 * Fetches a typeface matching the given font information.
	 * @param info Information indicating the font name and font style.
	 * @return Returns a typeface matching the given font information.
	 *         <p>
	 *         Returns null if the requested font could not be found or if given a null argument.
	 */
	public android.graphics.Typeface fetchTypefaceFor(TypefaceInfo info) {
		// Validate.
		if (info == null) {
			return null;
		}

		// Check if the requested typeface is already cached in the collection.
		android.graphics.Typeface typeface;
		synchronized (sTypefaceCollection) {
			typeface = sTypefaceCollection.get(info);
		}
		if (typeface != null) {
			return typeface;
		}
		
		// --- A new typeface has been requested. Attempt to load it below. ---

		// Load the font by name, if provided.
		if ((info.getName() != null) && (info.getName().length() > 0)) {
			// First, try to access the font from the APK's "assets" directory or Google expansion files.
			try {
				String fontFileName = info.getName();
				com.ansca.corona.storage.FileServices fileServices;
				fileServices = new com.ansca.corona.storage.FileServices(getApplicationContext());
				if (fileServices.doesAssetFileExist(fontFileName) == false) {
					fontFileName = info.getName() + ".ttf";
					if (fileServices.doesAssetFileExist(fontFileName) == false) {
						fontFileName = info.getName() + ".otf";
						if (fileServices.doesAssetFileExist(fontFileName) == false) {
							fontFileName = null;
						}
					}
				}
				if (fontFileName != null) {
					java.io.File fontFile = fileServices.extractAssetFile(fontFileName);
					if (fontFile != null) {
						typeface = android.graphics.Typeface.createFromFile(fontFile);
					}
				}
			}
			catch (Exception ex) { }

			// If the given font name was not found within this application, then load it from the system.
			if (typeface == null) {
				try {
					java.io.File fontFile = new java.io.File("/system/fonts/" + info.getName() + ".ttf");
					if (fontFile.exists()) {
						typeface = android.graphics.Typeface.createFromFile(fontFile);
					}
				}
				catch (Exception ex) { }
			}

			// If the given font still cannot be found, then log a warning.
			// We'll use the system's default font down below.
			if (typeface == null) {
				System.out.println("WARNING: Could not load font " + info.getName() + ". Using default." );
			}
		}
		
		// Use the system's default font.
		if (typeface == null) {
			typeface = info.isBold() ? android.graphics.Typeface.DEFAULT_BOLD : android.graphics.Typeface.DEFAULT;
			if (typeface == null) {
				typeface = android.graphics.Typeface.create((String)null, info.getAndroidTypefaceStyle());
			}
		}

		// Cache the requested typeface, if found.
		if (typeface != null) {
			synchronized (sTypefaceCollection) {
				sTypefaceCollection.put(info, typeface);
			}
		}

		// Return the requested typeface.
		return typeface;
	}

	/**
	 * Fetches all font family names that are available on the system.
	 * <p>
	 * This does not fetch the names of the fonts included with the application.
	 * @return Returns an array of font family names.
	 *         <p>
	 *         Returns an empty array if no fonts were found.
	 */
	public String[] fetchAllSystemFontNames() {
		// Set up a true type font file filter to be used below.
		final String fontSuffix = ".ttf";
		java.io.FilenameFilter fileNameFilter = new java.io.FilenameFilter() {
			@Override
			public boolean accept(java.io.File file, String name) {
				return name.endsWith(fontSuffix);
			}
		};

		// Traverse all fonts in the /system/fonts directory.
		java.util.ArrayList<String> fontNames = new java.util.ArrayList<String>();
		java.io.File fontDirectory = new java.io.File("/system/fonts/");
		for (java.io.File fontFile : fontDirectory.listFiles(fileNameFilter)) {
			String fontName = fontFile.getName();
			fontNames.add(fontName.subSequence(0, fontName.lastIndexOf(fontSuffix)).toString());
		}
		
		// Return an array of all the font family names found.
		String[] stringArray = new String[fontNames.size()];
		return fontNames.toArray(stringArray);
	}

	public java.util.HashMap<String, Float> getFontMetrics( String fontName, float fontSize, boolean isBold )
	{
		FontSettings settings = new FontSettings();
		settings.setName( fontName );
		settings.setPointSize( fontSize );
		settings.setIsBold( isBold );

		android.graphics.Typeface typeface = fetchTypefaceFor(settings);
		if (typeface == null) {
			return null;
		}

		android.text.TextPaint textPaint = new android.text.TextPaint();
		textPaint.setARGB(255, 255, 255, 255);
		textPaint.setAntiAlias(true);
		textPaint.setTextAlign(android.graphics.Paint.Align.LEFT);
		textPaint.setTypeface(typeface);
		textPaint.setTextSize(settings.getPointSize());
		
		java.util.HashMap<String, Float> metrics = new java.util.HashMap ( ) ;	  
		android.graphics.Paint.FontMetrics fontMetrics = textPaint.getFontMetrics();
		metrics.put( "ascent", -fontMetrics.ascent );    // consider replacing with fontMetrics.top
		metrics.put( "descent", -fontMetrics.descent );  // consider replacing with fontMetrics.bottom
		metrics.put( "leading", fontMetrics.leading );
		metrics.put( "height", fontMetrics.descent - fontMetrics.ascent + fontMetrics.leading );
		return metrics;
	}
}
