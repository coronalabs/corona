//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;


/** Provides access to the application <a href="http://developer.android.com/reference/android/content/Context.html">context</a>. */
public class ApplicationContextProvider {
	/**
	 * Creates a new provider which will return the application 
	 * <a href="http://developer.android.com/reference/android/content/Context.html">context</a> from the given 
	 * <a href="http://developer.android.com/reference/android/content/Context.html">context</a>.
	 * @param context The <a href="http://developer.android.com/reference/android/content/Context.html">context</a> from which the 
	 *				  application <a href="http://developer.android.com/reference/android/content/Context.html">context</a> will be 
	 *				  extracted from. Setting this to null will cause an {@link java.lang.Exception exception} to be thrown.
	 */
	public ApplicationContextProvider(android.content.Context context) {
		// Have the CoronaEnvironment store the application context, if not done already.
		// This will make the application context available to all other Corona services.
		CoronaEnvironment.setApplicationContext(context);
	}

	/**
	 * Gets this application's <a href="http://developer.android.com/reference/android/content/Context.html">context</a>.
	 * @return Returns a reference to the application 
	 *		   <a href="http://developer.android.com/reference/android/content/Context.html">context</a>.
	 */
	public static android.content.Context getApplicationContext() {
		return CoronaEnvironment.getApplicationContext();
	}
}
