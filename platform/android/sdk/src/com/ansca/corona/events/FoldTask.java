//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.events;


/** Delivers a foldable device posture change to the Corona runtime thread. */
public class FoldTask implements com.ansca.corona.CoronaRuntimeTask {
	private final int fState;
	private final int fOrientation;
	private final android.graphics.Rect fBounds;

	/**
	 * @param state A com.ansca.corona.FoldStateMonitor.STATE_* value.
	 * @param orientation A com.ansca.corona.FoldStateMonitor.ORIENTATION_* value.
	 * @param bounds Bounds of the fold within the window in pixels, or null when unknown.
	 */
	public FoldTask(int state, int orientation, android.graphics.Rect bounds) {
		fState = state;
		fOrientation = orientation;
		fBounds = bounds;
	}

	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		boolean hasBounds = (fBounds != null);
		com.ansca.corona.JavaToNativeShim.foldEvent(runtime, fState, fOrientation, hasBounds,
				hasBounds ? fBounds.left : 0, hasBounds ? fBounds.top : 0,
				hasBounds ? fBounds.width() : 0, hasBounds ? fBounds.height() : 0);
	}
}
