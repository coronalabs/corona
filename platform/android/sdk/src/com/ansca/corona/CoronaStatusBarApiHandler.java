//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;

/** The interface has all the funcations that are activity specific and aren't implemented by default. */
class CoronaStatusBarApiHandler implements com.ansca.corona.listeners.CoronaStatusBarApiListener{	
	private CoronaActivity fActivity;

	public CoronaStatusBarApiHandler(CoronaActivity activity) {
		fActivity = activity;
	}

	@Override
	public CoronaStatusBarSettings getStatusBarMode() {
		if (fActivity == null) {
			return CoronaStatusBarSettings.HIDDEN;
		}
		return fActivity.getStatusBarMode();
	}

	@Override
	public void setStatusBarMode(CoronaStatusBarSettings mode) {
		if (fActivity == null) {
			return;
		}

		final CoronaStatusBarSettings finalMode = mode;
		fActivity.runOnUiThread(new Runnable() {
			@Override
			public void run() {
				if (fActivity != null) {
					fActivity.setStatusBarMode(finalMode);
				}
			}
		});
		
	}

	@Override
	public int getStatusBarHeight() {
		if (fActivity == null) {
			return 0;
		}
		return fActivity.getStatusBarHeight();
	}

	@Override
	public int getNavigationBarHeight(){
		if (fActivity == null) {
			return 0;
		}
		return fActivity.resolveNavBarHeight();
	}

	@Override
	public boolean IsAndroidTV(){
		if (fActivity == null) {
			return false;
		}
		return fActivity.IsAndroidTV();
	}

	@Override
	public boolean HasSoftwareKeys()
	{
		if (fActivity == null)
			return false;
		return fActivity.HasSoftwareKeys();
	}
}
