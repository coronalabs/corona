//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;


/** Provides access to a running Corona project and its {@link com.naef.jnlua.LuaState LuaState}. */
public class CoronaRuntime {
	/** Stores a collection of listeners that will receive runtime events such as "onLoaded". */
	private static java.util.ArrayList<CoronaRuntimeListener> sListeners = new java.util.ArrayList<CoronaRuntimeListener>();

	/** Stores a collection of listeners that will receive the willLoadMain event */
	private static java.util.ArrayList<CoronaRuntimeWillLoadMainListener> sWillLoadMainListeners = new java.util.ArrayList<CoronaRuntimeWillLoadMainListener>();

	/** Wraps the Lua state that was created by the C/C++ side of Corona, making it available on the Java side. */
	private com.naef.jnlua.LuaState fLuaState;
	
	/** Set true when this object has been disposed. */
	private boolean fWasDisposed;
	
	/** The pointer address to the JavaToNativeBridge object on the C side */
	private long fJavaToNativeBridgeAddress;

	private CoronaRuntimeTaskDispatcher fTaskDispatcher;

	private Controller fController;

	private ViewManager fViewManager;

	private String fBaseDir;

	private com.ansca.corona.graphics.opengl.CoronaGLSurfaceView fGLView;

	private boolean fIsCoronaKit;

	/**
	 * Creates a new Corona runtime object for running a Corona project.
	 * <p>
	 * This constructor was made internal so that only the Corona can make instances of this class.
	 */
	CoronaRuntime(android.content.Context context, boolean isCoronaKit, boolean wantsDepthBuffer, boolean wantsStencilBuffer) {
		fLuaState = null;
		fWasDisposed = false;
		fTaskDispatcher = new CoronaRuntimeTaskDispatcher(this);
		fBaseDir = "";
		fIsCoronaKit = isCoronaKit;
		fGLView = new com.ansca.corona.graphics.opengl.CoronaGLSurfaceView(context, this, isCoronaKit, wantsDepthBuffer, wantsStencilBuffer);

		// Initialize the native side of the Corona runtime.
		// Note that this does not load and start the Corona project.
		// That will be done by the CoronaRenderer via the OpenGL thread.
		JavaToNativeShim.init(this);
		fController = new Controller(context, this);
		fController.setGLView(fGLView);
		fController.init();
		fViewManager = new ViewManager(context, this);
		fViewManager.setGLView(fGLView);
	}
	
	void reset(android.content.Context context) {
		fGLView = new com.ansca.corona.graphics.opengl.CoronaGLSurfaceView(context, this, true, false, false);
		fController.setGLView(fGLView);
		fViewManager.setGLView(fGLView);
	}

	/**
	 * Terminates the Corona runtime and releases the Lua state.
	 * Once this dispose() method has been called, this object can never be used again.
	 * <p>
	 * This is an internal method that can only be called by Corona.
	 */
	void dispose() {
		// Do not continue if this object was already disposed.
		if (fWasDisposed) {
			return;
		}
		
		// Notify all listeners that the runtime is being disposed of.
		onExiting();
		
		CoronaRuntimeProvider.removeRuntime(this);

		// Dispose of the last LuaState wrapper, if it exists.
		if (fLuaState != null) {
			fLuaState = null;
		}
		
		// Flag that this object has been disposed of.
		fWasDisposed = true;

		fController.destroy();

		fController = null;

		fViewManager.destroy();

		fViewManager = null;
	}

	/**
	 * Returns the {@link com.ansca.corona.CoronaRuntimeTaskDispatcher CoronaRuntimeTaskDispatcher} used by this 
	 * {@link com.ansca.corona.CoronaRuntime CoronaRuntime}.
	 * <p>
	 * <b>Added in <a href="http://developer.coronalabs.com/release-ent/2014/2148/">daily build 2014.2148</a></b>.
	 */
	public CoronaRuntimeTaskDispatcher getTaskDispatcher() {
		return fTaskDispatcher;
	}

	void onPause() {
		fController.stop();
		fGLView.onSuspendCoronaRuntime();
		fViewManager.suspend();
	}

	void onResume() {
		fController.start();
		fGLView.onResumeCoronaRuntime();
		updateViews();
		fViewManager.resume();
	}

	/** Shows or hides the views according to the current state of the activity. */
	void updateViews() {
		// Fetch this activity's root content view.
		android.view.ViewGroup contentView = null;
		com.ansca.corona.ViewManager viewManager = fViewManager;
		if (viewManager != null) {
			contentView = viewManager.getContentView();
		}
		if (contentView == null) {
			return;
		}

		// Temporarily remove the OpenGL view from the activity if the screen has been powered off.
		// This works-around a bug on Galaxy SII where last rendered frame is displayed on screen lock window.
		if (fController.getSystemMonitor() != null && fController.getSystemMonitor().isScreenOff()) {
			if (fGLView.getParent() != null) {
				contentView.removeView(fGLView);
			}
		}
		else {
			if (fGLView.getParent() == null) {
				contentView.addView(fGLView, 0);
				fGLView.getHolder().setSizeFromLayout();
			}
		}

		// Hide this activity's views while the screen lock is shown.
		// Note: Works-around bug on Saumsung Galaxy SII where the screen lock partially displays this activity's
		//       view in portrait orientation, even if this activity does not support that orientation.
		if (fController.getSystemMonitor() != null && fController.getSystemMonitor().isScreenLocked()) {
			contentView.setVisibility(android.view.View.INVISIBLE);
		}
		else {
			contentView.setVisibility(android.view.View.VISIBLE);
		}
	}

	Controller getController() {
		return fController;
	}

	ViewManager getViewManager() {
		return fViewManager;
	}

	void setPath(String baseDir) {
		fBaseDir = baseDir;
	}

	String getPath() {
		return fBaseDir;
	}

	com.ansca.corona.graphics.opengl.CoronaGLSurfaceView getGLView() {
		return fGLView;
	}

	boolean isCoronaKit() {
		return fIsCoronaKit;
	}

	/**
	 * Determines if the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} has been disposed of, meaning it was destroyed and can no 
	 * longer be used. This happens when the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} exits, which occurs when the 
	 * {@link com.ansca.corona.CoronaActivity CoronaActivity} has been destroyed.
	 * @return Returns true if this object was disposed of, meaning it can no longer be used.
	 *         Returns false if this object is still usable.
	 */
	public boolean wasDisposed() {
		return fWasDisposed;
	}
	
	/**
	 * Determines if the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} is still available and has not been destroyed yet, which 
	 * occurs on exit.
	 * @return Returns true if this object is still usable. Returns false if the object was disposed of.
	 */
	public boolean wasNotDisposed() {
		return !wasDisposed();
	}
	
	/**
	 * Determines if the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} is currently running its project.
	 * @return Returns true if the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} is currently running its project.
	 *         Returns false if the {@link com.ansca.corona.CoronaRuntime CoronaRuntime} has been suspended or stopped.
	 */
	public boolean isRunning() {
		if (fWasDisposed) {
			return false;
		}
		return fController.isRunning();
	}
	
	/**
	 * Gets the {@link com.naef.jnlua.LuaState LuaState} object that the Corona project's code runs in.
	 * Provides access to all modules, functions, and variables declared in the Corona project's code.
	 * This {@link com.naef.jnlua.LuaState LuaState} can be used to add new APIs to the project at run time.
	 * @return Returns a reference to the {@link com.naef.jnlua.LuaState LuaState} object.
	 *         Returns null if it has not been created yet, which can happen if Corona is currently loading a project.
	 */
	public com.naef.jnlua.LuaState getLuaState() {
		return fLuaState;
	}
	
	/**
	 * Creates and returns a shallow copy of this class' CoronaRuntimeListener collection "sListeners".
	 * <p>
	 * This is needed by this class' event raising methods such as onLoaded() to access the collection in
	 * a thread safe manner and to allow a called listener to add/remove listeners from this collection
	 * without invalidating the iterator which would cause an exception.
	 * @return Returns a shallow copy of this class' CoronaRuntimeListener collection.
	 */
	private static java.util.ArrayList<CoronaRuntimeListener> cloneListenerCollection() {
		synchronized (sListeners) {
			return (java.util.ArrayList<CoronaRuntimeListener>)sListeners.clone();
		}
	}

	/**
	 * Creates and returns a shallow copy of this class' CoronaRuntimeListener collection "sListeners".
	 * <p>
	 * This is needed by this class' event raising methods such as onLoaded() to access the collection in
	 * a thread safe manner and to allow a called listener to add/remove listeners from this collection
	 * without invalidating the iterator which would cause an exception.
	 * @return Returns a shallow copy of this class' CoronaRuntimeListener collection.
	 */
	private static java.util.ArrayList<CoronaRuntimeWillLoadMainListener> cloneWillLoadMainListenerCollection() {
		synchronized (sWillLoadMainListeners) {
			return (java.util.ArrayList<CoronaRuntimeWillLoadMainListener>)sWillLoadMainListeners.clone();
		}
	}

	/**
	 * Adds this application's native C/C++ library path to the Lua "package.cpath".
	 * This allows Lua to load "*.so" plugin libraries.
	 * @param L Reference to the Lua state to push the path into.
	 */
	void initializePackagePath(com.naef.jnlua.LuaState L) {
		// Validate argument.
		if (L == null) {
			return;
		}

		// Fetch the path to where this application's native C/C++ libraries are located.
		String nativeLibraryPath = ApiLevel9.getNativeLibraryDirectoryFrom(CoronaEnvironment.getApplicationContext());

		// Push the Lua "package" table to the top of the stack.
		fLuaState.getGlobal( "package" );

		// Fetch the "package.cpath" field's value.
		final String cpathKey = "cpath";
		fLuaState.getField( -1, cpathKey );
		String cpathValue = fLuaState.toString( -1 );
		fLuaState.pop( 1 );

		// Add this application's native library path to the front of Lua's package search path.
		String cpathNew = nativeLibraryPath + "/lib?.so;" + cpathValue;
		fLuaState.pushString( cpathNew );
		fLuaState.setField( -2, cpathKey );

		fLuaState.newTable();
		int i = 1;
		android.content.pm.ApplicationInfo ai = CoronaEnvironment.getApplicationContext().getApplicationInfo();
		String main = ai.sourceDir;
		if (main != null) {
			fLuaState.pushString(main);
			fLuaState.rawSet(-2, i++);
		}
		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.LOLLIPOP) {
			String[] splits = ai.splitSourceDirs;
			if (splits != null) {
				for (String split : splits) {
					fLuaState.pushString(split);
					fLuaState.rawSet(-2, i++);
				}
			}
		}
		fLuaState.pushString(android.os.Build.CPU_ABI); // despite promise, SUPPORTED_ABIS[0] is not what we want
		fLuaState.setField(-2, "abi");

		fLuaState.setField(-2, "APKs");
		// Pop the Lua "package" table off of the stack.
		fLuaState.pop( 1 );

        insertFakeNativeLoader();
	}
	
	/**
	 * This iserts fake loader into package.loaders which just calls System.loadLibrary
	 * This does not expose symbols to the C, but does something so dlopen starts to work
	 * on old x64 Androids. Weird workaround, but works.
	 */
	private void insertFakeNativeLoader() {
		fLuaState.getGlobal("table");
		fLuaState.getField(-1, "insert"); // push function to be called

		fLuaState.getGlobal("package");
		fLuaState.getField(-1, "loaders"); // push 1st arg: "package.loaders" table
		fLuaState.remove(-2); // pop "package"
		fLuaState.pushInteger(2); // position of the fake loader
		fLuaState.pushJavaFunction(new com.naef.jnlua.JavaFunction() {
			@Override
			public int invoke(com.naef.jnlua.LuaState luaState) {
				try {
                    // Insert "fake" library loader. This helps to load ARM64 libraries
                    // on adnroid 5.1 first gen x64 devices for some reason, helps them
                    // load dependencies
                    System.loadLibrary(luaState.toString(1));
				} catch (Throwable ignore) {
				}
				return 0;
			}
		});

		fLuaState.call(3, 0); // call table.insert( package.loaders, [index,] loader )
		fLuaState.pop(1); // pop "table"
	}

	/**
	 * To be called when the C++ side of the Corona runtime has been created and its Lua state has been initialized.
	 * This object will generate a LuaState wrapper around the C Lua state instance and then notify all listeners
	 * by calling their onLoaded() method.
	 * <p>
	 * This is an internal method that can only be called by Corona.
	 * @param luaStateMemoryAddress The memory address of the Lua state created on the C/C++ side.
	 */
	void onLoaded(long luaStateMemoryAddress) {
		// Validate.
		if (fWasDisposed || (luaStateMemoryAddress == 0)) {
			return;
		}
		
		// Dispose of the last LuaState wrapper, if it exists.
		// This should never happen, but prevent the possible memory leak in case it does.
		if (fLuaState != null) {
			// Note: This won't actually call lua_close() on the C side since the wrapp does not own the Lua state.
			fLuaState.close();
			fLuaState = null;
		}

		// Wrap the given Lua state that was created by the C++ side of Corona.
		fLuaState = new com.naef.jnlua.LuaState(luaStateMemoryAddress);

		CoronaRuntimeProvider.addRuntime(this);

		initializePackagePath( fLuaState );

		// Notify the rest of the application about this event.
		for (CoronaRuntimeListener listener : cloneListenerCollection()) {
			if (listener != null) {
				listener.onLoaded(this);
			}
		}
	}

	/**
	 * To be called right before "main.lua" is executed and before the splash screen shows.
	 * Notifies all assigned listeners by calling their onWillLoadMain method.
	 * <p>
	 * This is an internal method that can only be called by Corona.
	 */
	void onWillLoadMain() {
		// Do not continue if the runtime has been disposed of.
		if (fWasDisposed) {
			return;
		}
		
		// Notify the rest of the application about this event.
		for (CoronaRuntimeWillLoadMainListener listener : cloneWillLoadMainListenerCollection()) {
			if (listener != null) {
				listener.onWillLoadMain(this);
			}
		}
	}
	
	/**
	 * To be called when the Corona runtime has been started and just after the "main.lua" file has been executed.
	 * Notifies all assigned listeners by calling their onStarted() method.
	 * <p>
	 * This is an internal method that can only be called by Corona.
	 */
	void onStarted() {
		// Do not continue if the runtime has been disposed of.
		if (fWasDisposed) {
			return;
		}
		
		// Notify the rest of the application about this event.
		for (CoronaRuntimeListener listener : cloneListenerCollection()) {
			if (listener != null) {
				listener.onStarted(this);
			}
		}
	}
	
	/**
	 * To be called when the Corona runtime has been suspended.
	 * Notifies all assigned listeners by calling their onSuspended() method.
	 * <p>
	 * This is an internal method that can only be called by Corona.
	 */
	void onSuspended() {
		// Do not continue if the runtime has been disposed of.
		if (fWasDisposed) {
			return;
		}
		
		// Notify the rest of the application about this event.
		for (CoronaRuntimeListener listener : cloneListenerCollection()) {
			if (listener != null) {
				listener.onSuspended(this);
			}
		}
	}
	
	/**
	 * To be called when the Corona runtime has been resumed after a suspend.
	 * Notifies all assigned listeners by calling their onResumed() method.
	 * <p>
	 * This is an internal method that can only be called by Corona.
	 */
	void onResumed() {
		// Do not continue if the runtime has been disposed of.
		if (fWasDisposed) {
			return;
		}
		
		// Notify the rest of the application about this event.
		for (CoronaRuntimeListener listener : cloneListenerCollection()) {
			if (listener != null) {
				listener.onResumed(this);
			}
		}
	}
	
	/**
	 * To be called just before the Corona runtime terminates.
	 * Notifies all assigned listeners by calling their onExiting() method.
	 * <p>
	 * This is an internal method that can only be called by Corona.
	 */
	private void onExiting() {
		// Do not continue if the runtime has been disposed of.
		if (fWasDisposed) {
			return;
		}
		
		// Notify the rest of the application about this event.
		for (CoronaRuntimeListener listener : cloneListenerCollection()) {
			if (listener != null) {
				listener.onExiting(this);
			}
		}
	}

	void setJavaToNativeBridgeAddress(long pointerAddress) {
		fJavaToNativeBridgeAddress = pointerAddress;
	}

	long getJavaToNativeBridgeAddress() {
		return fJavaToNativeBridgeAddress;
	}
	
	/**
	 * Thread safe internal method that adds a listener for receiving events from the Corona runtime.
	 * <p>
	 * External code is expected to add listeners via the CoronaRuntime.addRuntimeListener() method instead.
	 * @param listener The listener that will receive events. Cannot be null.
	 */
	static void addListener(CoronaRuntimeListener listener) {
		synchronized (sListeners) {
			// Validate.
			if (listener == null) {
				return;
			}
			
			// Do not continue if the given listener already exists in the collection.
			if (sListeners.indexOf(listener) >= 0) {
				return;
			}
			
			// Add the given listener to the collection.
			sListeners.add(listener);
		}
	}
	
	/**
	 * Thread safe internal method for removing the given listener from the runtime, preventing it from receiving anymore events.
	 * <p>
	 * External code is expected to remove listeners via the CoronaRuntime.removeRuntimeListener() method instead.
	 * @param listener The listener object to be removed by reference.
	 */
	static void removeListener(CoronaRuntimeListener listener) {
		synchronized (sListeners) {
			// Validate.
			if (listener == null) {
				return;
			}
			
			// Remove the given listener from the collection.
			sListeners.remove(listener);
		}
	}
	
	/**
	 * Thread safe internal method that adds a listener for receiving events from the Corona runtime.
	 * @param listener The listener that will receive events. Cannot be null.
	 */
	static void addWillLoadMainListener(CoronaRuntimeWillLoadMainListener listener) {
		synchronized (sWillLoadMainListeners) {
			if (listener == null) {
				return;
			}

			// Do not continue if the given listener already exists in the collection.
			if (sWillLoadMainListeners.indexOf(listener) >= 0) {
				return;
			}

			// Add the given listener to the collection.
			sWillLoadMainListeners.add(listener);
		}
	}

	/**
	 * Thread safe internal method for removing the given listener from the runtime, preventing it from receiving anymore events.
	 * @param listener The listener object to be removed by reference.
	 */
	static void removeWillLoadMainListener(CoronaRuntimeWillLoadMainListener listener) {
		synchronized(sWillLoadMainListeners) {
			if (listener == null) {
				return;
			}

			// Remove the given lsitener from the collection.
			sWillLoadMainListeners.remove(listener);
		}
	}

	/**
	 * Provides access to API Level 9 (Android 2.3) features.
	 * Should only be accessed if running on an operating system matching this API Level.
	 * <p>
	 * You cannot create instances of this class.
	 * You are expected to call its static methods instead.
	 */
	private static class ApiLevel9 {
		/** Constructor made private to prevent instances from being made. */
		private ApiLevel9() { }

		/**
		 * Gets the path to the directory where this application's C/C++ "*.so" files are located.
		 * @param context Context needed to fetch this application's native library directory.
		 *                <p>
		 *                Cannot be null or else an exception will be thrown.
		 * @return Returns the path to this application's native C/C++ library files.
		 */
		public static String getNativeLibraryDirectoryFrom(android.content.Context context) {
			if (context == null) {
				throw new NullPointerException();
			}
			return context.getApplicationInfo().nativeLibraryDir;
		}
	}
}
