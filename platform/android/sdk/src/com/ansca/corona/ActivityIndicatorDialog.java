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
