//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;

import android.os.Build;
import android.util.Log;
import android.content.Intent;

/** The interface has all the functions that are activity specific and aren't implemented by default. */
public class CoronaSystemApiHandler implements com.ansca.corona.listeners.CoronaSystemApiListener {
	private CoronaActivity fActivity;
	private static final boolean DEBUG = true;

	public CoronaSystemApiHandler(CoronaActivity activity) {
		fActivity = activity;
	}

	@Override
	public boolean requestSystem(CoronaRuntime runtime, String actionName, long luaStateMemoryAddress, int luaStackIndex) {
		if (fActivity == null) {
			return false;
		}

		// Validate.
		if ((actionName == null) || (actionName.length() <= 0) || fActivity == null) {
			return false;
		}

		// Attempt to execute the requested operation.
		if (actionName.equals("exitApplication")) {
			// Exit the application by destroying the activity window which also destroys the Corona runtime.
			// Note that the application process will still be alive, which we need to keep notifications alive.
			fActivity.runOnUiThread(new Runnable() {
				@Override
				public void run() {
					if (fActivity != null) {
						if(android.os.Build.VERSION.SDK_INT>=16 && android.os.Build.VERSION.SDK_INT<21){
						    fActivity.finishAffinity();
						} else if(android.os.Build.VERSION.SDK_INT>=21){
						    fActivity.finishAndRemoveTask();
						}
					}
					java.lang.System.exit(0);
				}
			});
		}
		else if (actionName.equals("exitActivity")) {
			// Exit the application by destroying the activity window which also destroys the Corona runtime.
			// Note that the application process will still be alive, which we need to keep notifications alive.
			fActivity.runOnUiThread(new Runnable() {
				@Override
				public void run() {
					if (fActivity != null) {
						fActivity.finish();
					}
				}
			});
		}
		else if (actionName.equals("suspendApplication")) {
			// Suspend this application by hiding its activity window.
			fActivity.runOnUiThread(new Runnable() {
				@Override
				public void run() {
					if (fActivity != null) {
						fActivity.moveTaskToBack(true);
					}
				}
			});
		}
		else if (actionName.equals("validateResourceFile"))
		{
			// Fetch the Lua state object for the given memory address.
			if (runtime == null)
			{
				return false;
			}
			com.naef.jnlua.LuaState luaState = runtime.getLuaState();
			if (luaState == null || CoronaRuntimeProvider.getLuaStateMemoryAddress(luaState) != luaStateMemoryAddress)
			{
				return false;
			}

			String assetFilename = "";
			double assetFileSize = -1;

			if (luaState.type(luaStackIndex) == com.naef.jnlua.LuaType.TABLE)
			{
				luaState.getField( -1, "filename" );
				if ((assetFilename = luaState.toString( -1 )) == null)
				{
					return false;
				}
				luaState.pop( 1 );

				luaState.getField( -1, "size" );
				if (luaState.type( -1 ) == com.naef.jnlua.LuaType.NUMBER)
				{
					assetFileSize = luaState.toNumber( -1 );
				}
				luaState.pop( 1 );
			}

			if (assetFilename.length() > 0 && assetFileSize >= 0)
			{
				android.content.Context context = CoronaEnvironment.getApplicationContext();
				com.ansca.corona.storage.FileServices fileServices;
				fileServices = new com.ansca.corona.storage.FileServices(context);

				long fileSize = fileServices.getResourceFileSize(assetFilename);

				if ( DEBUG )
				{
					Log.v("Corona", "validateResourceFile: assetFilename: " + assetFilename);
					Log.v("Corona", "validateResourceFile: assetFileSize: " + assetFileSize);
					Log.v("Corona", "validateResourceFile: fileSize : " + fileSize);
				}

				return (fileSize == assetFileSize);
			}
			else
			{
				return false;
			}
		}
		else if (actionName.equals("reportFullyDrawn"))
		{
			// Report to the system that app is now fully drawn, for diagnostic and optimization purposes.
			// API introduced in KITKAT but has permission issue, so raise to LOLLIPOP, see https://github.com/flutter/flutter/issues/46172.
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
				fActivity.runOnUiThread(new Runnable() {
					@Override
					public void run() {
						if (fActivity != null) {
							fActivity.reportFullyDrawn();
						}
					}
				});
			}
		}
		else {
			// The requested action is unknown.
			return false;
		}

		// Return true to indicate that the requested action was accepted.
		return true;
	}

	@Override
	public Intent getIntent() {
		if (fActivity == null) {
			return null;
		}
		return fActivity.getIntent();
	}

	@Override
	public Intent getInitialIntent() {
		if (fActivity == null) {
			return null;
		}
		return fActivity.getInitialIntent();
	}
}
