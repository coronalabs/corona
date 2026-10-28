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
 * Watches the posture of a foldable device through Jetpack WindowManager and reports it to Corona.
 * State and orientation values match Rtt::FoldEvent. The library ("androidx.window:window-java") is only
 * linked when build.settings sets android.supportsFoldables = true, and needs Android 6.0; without it this
 * class reports an unknown state. Only {@link Impl} touches the library's classes, so a missing library is caught here.
 */
public class FoldStateMonitor {
	public static final int STATE_UNKNOWN = 0;
	public static final int STATE_CLOSED = 1;
	public static final int STATE_HALF_OPEN = 2;
	public static final int STATE_OPEN = 3;

	public static final int ORIENTATION_UNKNOWN = 0;
	public static final int ORIENTATION_HORIZONTAL = 1;
	public static final int ORIENTATION_VERTICAL = 2;

	/** Set once the window library was found; null when it is unavailable. */
	private Impl fImpl = null;
	private final CoronaActivity fActivity;
	private boolean fIsStarted = false;

	private volatile int fState = STATE_UNKNOWN;
	private volatile int fOrientation = ORIENTATION_UNKNOWN;
	private volatile android.graphics.Rect fFoldBounds = null;
	/** True once a fold was ever reported, or the device advertises a hinge angle sensor. */
	private volatile boolean fIsFoldable = false;

	public FoldStateMonitor(CoronaActivity activity) {
		fActivity = activity;
		if (android.os.Build.VERSION.SDK_INT >= 30 && activity != null) {
			try {
				fIsFoldable = activity.getPackageManager().hasSystemFeature("android.hardware.sensor.hinge_angle");
			}
			catch (Exception ex) { }
		}
	}

	/** Returns true when fold information can be reported on this device and build. */
	public boolean isAvailable() {
		if (android.os.Build.VERSION.SDK_INT < 23) {
			return false;
		}
		try {
			Class.forName("androidx.window.java.layout.WindowInfoTrackerCallbackAdapter");
			return true;
		}
		catch (Throwable ex) {
			return false;
		}
	}

	/** Starts observing. Safe to call more than once. */
	public void start() {
		if (fIsStarted || fActivity == null || !isAvailable()) {
			return;
		}
		try {
			if (fImpl == null) {
				fImpl = new Impl();
			}
			fImpl.start();
			fIsStarted = true;
		}
		catch (Throwable ex) {
			// The window library is missing or incompatible; fold state stays unknown.
			android.util.Log.i("Corona", "Fold state is unavailable: " + ex.getMessage());
			fImpl = null;
		}
	}

	public void stop() {
		if (!fIsStarted) {
			return;
		}
		try {
			fImpl.stop();
		}
		catch (Throwable ex) { }
		fIsStarted = false;
	}

	public int getState() {
		return fState;
	}

	public int getOrientation() {
		return fOrientation;
	}

	/** The Lua string for the current state, or null when unknown. */
	public String getStateName() {
		switch (fState) {
			case STATE_CLOSED: return "closed";
			case STATE_HALF_OPEN: return "halfOpen";
			case STATE_OPEN: return "open";
			default: return null;
		}
	}

	private void update(final int state, final int orientation, final android.graphics.Rect bounds) {
		boolean changed = (state != fState) || (orientation != fOrientation) ||
		                  (bounds == null ? fFoldBounds != null : !bounds.equals(fFoldBounds));
		fState = state;
		fOrientation = orientation;
		fFoldBounds = bounds;
		if (!changed || state == STATE_UNKNOWN) {
			return;
		}

		// Hand the change to the Corona runtime thread; native code forwards it to Lua only while a "fold" listener exists.
		CoronaRuntimeTaskDispatcher dispatcher = fActivity.getRuntimeTaskDispatcher();
		if (dispatcher != null) {
			dispatcher.send((CoronaRuntime runtime) -> {
				boolean hasBounds = (bounds != null);
				JavaToNativeShim.foldEvent(runtime, state, orientation, hasBounds,
						hasBounds ? bounds.left : 0, hasBounds ? bounds.top : 0,
						hasBounds ? bounds.width() : 0, hasBounds ? bounds.height() : 0);
			});
		}
	}

	/** Everything that references Jetpack WindowManager lives here. */
	private class Impl {
		private final androidx.window.java.layout.WindowInfoTrackerCallbackAdapter fTracker;
		private final java.util.concurrent.Executor fExecutor;
		private final androidx.core.util.Consumer<androidx.window.layout.WindowLayoutInfo> fListener;

		Impl() {
			// getOrCreate() lives on the Kotlin companion object; the interface itself has no static method.
			fTracker = new androidx.window.java.layout.WindowInfoTrackerCallbackAdapter(
					androidx.window.layout.WindowInfoTracker.Companion.getOrCreate(fActivity));
			final android.os.Handler handler = new android.os.Handler(android.os.Looper.getMainLooper());
			fExecutor = (Runnable command) -> handler.post(command);
			fListener = (androidx.window.layout.WindowLayoutInfo layoutInfo) -> onLayoutInfo(layoutInfo);
		}

		void start() {
			fTracker.addWindowLayoutInfoListener(fActivity, fExecutor, fListener);
		}

		void stop() {
			fTracker.removeWindowLayoutInfoListener(fListener);
		}

		private void onLayoutInfo(androidx.window.layout.WindowLayoutInfo layoutInfo) {
			androidx.window.layout.FoldingFeature fold = null;
			for (androidx.window.layout.DisplayFeature feature : layoutInfo.getDisplayFeatures()) {
				if (feature instanceof androidx.window.layout.FoldingFeature) {
					fold = (androidx.window.layout.FoldingFeature)feature;
					break;
				}
			}

			if (fold == null) {
				// No fold crosses the window: closed (outer screen) when the device is known to fold
				update(fIsFoldable ? STATE_CLOSED : STATE_UNKNOWN, ORIENTATION_UNKNOWN, null);
				return;
			}

			fIsFoldable = true;
			int state = (fold.getState() == androidx.window.layout.FoldingFeature.State.HALF_OPENED) ? STATE_HALF_OPEN : STATE_OPEN;
			int orientation = (fold.getOrientation() == androidx.window.layout.FoldingFeature.Orientation.HORIZONTAL)
					? ORIENTATION_HORIZONTAL : ORIENTATION_VERTICAL;
			update(state, orientation, new android.graphics.Rect(fold.getBounds()));
		}
	}
}
