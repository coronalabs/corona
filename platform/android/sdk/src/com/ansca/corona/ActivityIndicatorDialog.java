//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;


/**
 * Dialog used to display a "spinning" ProgressBar control at the center of the dialog.
 * This class is needed because Android's ProgressDialog cannot display the ProgressBar at the center.
 */
public class ActivityIndicatorDialog extends android.app.Dialog {
	/** Set true if the dialog can be cleared with the BACK or SEARCH buttons. */
	private boolean fIsCancelable = true;
	
	/**
	 * Creates a new activity indicator dialog.
	 * @param context The parent that will host this dialog.
	 */
	public ActivityIndicatorDialog(android.content.Context context) {
		this(context, 0);
	}

	/**
	 * Creates a new activity indicator dialog with the given theme.
	 * @param context The parent that will host this dialog.
	 * @param theme A "style" resource ID used to theme this dialog.
	 */
	public ActivityIndicatorDialog(android.content.Context context, int theme) {
		super(context, theme);
		
		// Remove the title bar.
		requestWindowFeature(android.view.Window.FEATURE_NO_TITLE);
		
		// Add progress bar control.
		android.widget.ProgressBar progressBar = new android.widget.ProgressBar(context);
		int xPadding = progressBar.getIndeterminateDrawable().getMinimumWidth() / 2;
		int yPadding = progressBar.getIndeterminateDrawable().getMinimumHeight() / 2;
		progressBar.setPadding(xPadding, xPadding, yPadding, yPadding);
		addContentView(progressBar, new android.view.ViewGroup.LayoutParams(
				android.view.ViewGroup.LayoutParams.WRAP_CONTENT,
				android.view.ViewGroup.LayoutParams.WRAP_CONTENT));
	}
	
	/**
	 * Sets whether or not this dialog can be closed when the BACK button is pressed.
	 * @param flag Set true to allow this dialog to be closed by the BACK button.
	 *             Set false to not allow this dialog to be closed by the user, making it only cancelable via code.
	 */
	@Override
	public void setCancelable(boolean flag) {
		fIsCancelable = flag;
		super.setCancelable(flag);
	}
	
	/**
	 * Determines if this dialog can be closed via user input, such as the BACK button.
	 * @return Returns true if the dialog can be closed by the user.
	 *         Returns false if this dialog can only be closed via code.
	 */
	public boolean isCancelable() {
		return fIsCancelable;
	}
	
	/**
	 * Called when a key has been released.
	 * Prevents the SEARCH key from closing this dialog.
	 * @param keyCode The value in event.getKeyCode().
	 * @param event Provides information about the key that was pressed.
	 */
	@Override
	public boolean onKeyUp(int keyCode, android.view.KeyEvent event) {
		// Prevent "search" button from closing this dialog if made not cancelable.
		if (!fIsCancelable && (keyCode == android.view.KeyEvent.KEYCODE_SEARCH)) {
			return true;
		}
		
		// Let the dialog do the default handling for all other buttons.
		return super.onKeyUp(keyCode, event);
	}
}
