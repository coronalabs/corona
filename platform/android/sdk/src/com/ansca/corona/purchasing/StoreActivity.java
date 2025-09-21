//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.purchasing;


import android.content.res.Configuration;

/**
 * Activity used to display the Nook app store.
 * <p>
 * Currently, this activity is only used to display the Nook app store. This activity works-around a bug
 * where there is a 10-30 second delay displaying the Nook store's activity from a landscape only activity.
 * This bug is worked-around by displaying this activity as portrait before launch the Nook store dialog.
 * <p>
 * Example usage:
 * <code>
 *    android.content.Intent intent = new android.content.Intent(context, StoreActivity.class);
 *    intent.putExtra(StoreActivity.EXTRA_FULL_SCREEN, false);
 *    intent.putExtra(StoreActivity.EXTRA_NOOK_APP_EAN, "123456");
 *    context.startActivity(intent);
 * </code>
 * <p>
 * In the future, this activity may be used to display other app store information.
 */
public class StoreActivity extends android.app.Activity {
	/**
	 * The unique name of an Intent's extra data that stores a boolean value indicating if the StoreActivity
	 * should be displayed full screen (ie: without a status bar).
	 */
	public static final String EXTRA_FULL_SCREEN = "full_screen";

	/**
	 * The unique name of an Intent's extra data that stores an application's EAN value used to display
	 * the application's information in the Nook app store.
	 */
	public static final String EXTRA_NOOK_APP_EAN = "nook_app_ean";

	/** Set to true if the Nook app store dialog has been shown. */
	private boolean fHasShownStore;


	/** Initializes this activity's UI and member variables. */
	@Override
	protected void onCreate(android.os.Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		// Do not continue if a Nook EAN was not provided.
		if (getIntent().getStringExtra(EXTRA_NOOK_APP_EAN) == null) {
			finish();
			return;
		}

		// Only lock orientation on devices that don’t support multi-window / resizable
		if (android.os.Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.N) {
			int orientation = android.content.pm.ActivityInfo.SCREEN_ORIENTATION_PORTRAIT;
			if (android.os.Build.VERSION.SDK_INT >= 9) {
				orientation = android.content.pm.ActivityInfo.SCREEN_ORIENTATION_SENSOR_PORTRAIT;
			}
			setRequestedOrientation(orientation);
		}

		// Display this activity full screen if requested
		boolean isFullScreen = getIntent().getBooleanExtra(EXTRA_FULL_SCREEN, false);
		if (isFullScreen) {
			getWindow().addFlags(android.view.WindowManager.LayoutParams.FLAG_FULLSCREEN);
			getWindow().clearFlags(android.view.WindowManager.LayoutParams.FLAG_FORCE_NOT_FULLSCREEN);
		} else {
			getWindow().addFlags(android.view.WindowManager.LayoutParams.FLAG_FORCE_NOT_FULLSCREEN);
			getWindow().clearFlags(android.view.WindowManager.LayoutParams.FLAG_FULLSCREEN);
		}

		// Initialize this flag to false
		fHasShownStore = false;
	}
	/** This method is called when this activity can be interacted with. */
	@Override
	protected void onResume() {
		super.onResume();

		// Display the Nook app store dialog, if not done already.
		if (fHasShownStore == false) {
			fHasShownStore = true;
			android.content.Intent intent = new android.content.Intent();
			intent.setAction("com.bn.sdk.shop.details");
			intent.putExtra("product_details_ean", getIntent().getStringExtra(EXTRA_NOOK_APP_EAN));
			startActivityForResult(intent, 1);
		}
	}

	/**
	 * Called when an activity you launched exits, yielding the results of that activity.
	 * @param requestCode The integer request code originally supplied to startActivityForResult().
	 *                    Allows you to identify which activity the result is coming from.
	 * @param resultCode The integer result code returned by the child activity through its setResult().
	 * @param data An Intent object which can return result data to the caller. Can be null.
	 */
	@Override
	protected void onActivityResult(int requestCode, int resultCode, android.content.Intent data) {
		super.onActivityResult(requestCode, resultCode, data);

		// Exit out of this activity.
		finish();
	}

	/**
	 * Closes this activity.
	 * <p>
	 * Overriden to prevent this activity from doing a slide-out animation.
	 */
	@Override
	public void finish() {
		// Have the base class close this window.
		super.finish();
		
		// If this activity is flagged to not show transition animations, then disable the slide-out animation.
		if ((getIntent().getFlags() & android.content.Intent.FLAG_ACTIVITY_NO_ANIMATION) != 0) {
			overridePendingTransition(0, 0);
		}
	}
}
