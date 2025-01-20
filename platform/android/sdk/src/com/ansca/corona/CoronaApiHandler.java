//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;

import android.app.Activity;
import android.view.Window;
import android.view.WindowManager;

/** The interface has all the funcations that are activity specific and aren't implemented by default. */
public class CoronaApiHandler implements CoronaApiListener {
	private CoronaActivity fActivity;
	private CoronaRuntime fCoronaRuntime;

	public CoronaApiHandler(CoronaActivity activity, CoronaRuntime runtime) {
		fActivity = activity;
		fCoronaRuntime = runtime;
	}


/************************************************************************************************************************/


	@Override
	public void onScreenLockStateChanged(boolean isScreenLocked) {
		if (fActivity == null) {
			return;
		}
		fActivity.onScreenLockStateChanged(isScreenLocked);
	}


/************************************************************************************************************************/

	@Override
	public void removeKeepScreenOnFlag() {
		fActivity.runOnUiThread( new Runnable() {
			public void run()
			{
				Window window = fActivity.getWindow();
				if( null != window )
				{
					window.clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);						
				}
			}
		} );
	}

	@Override
	public void addKeepScreenOnFlag() {
		fActivity.runOnUiThread( new Runnable() {
			public void run()
			{
				Window window = fActivity.getWindow();
				if( null != window )
				{
					window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
				}
			}
		} );
	}
}
