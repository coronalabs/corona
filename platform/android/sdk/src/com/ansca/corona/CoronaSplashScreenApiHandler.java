//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;

/** The interface has all the functions that are activity specific and aren't implemented by default. */
class CoronaSplashScreenApiHandler implements com.ansca.corona.listeners.CoronaSplashScreenApiListener{
	private CoronaActivity fActivity;

	public CoronaSplashScreenApiHandler(CoronaActivity activity) {
		fActivity = activity;
	}

	@Override
	public void showSplashScreen() {
		if (fActivity == null) {
			return;
		}
		fActivity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				if (fActivity != null) {
					fActivity.showSplashScreen();
				}
			}
		});
	}

	@Override
	public void hideSplashScreen() {
		if (fActivity == null) {
			return;
		}
		fActivity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				if (fActivity != null) {
					fActivity.hideSplashScreen();
				}
			}
		});
	}
}
