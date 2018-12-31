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
