//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.input;


/**
 * Handles input received from a view such as key events, touch events, motion events, etc. and
 * dispatches them to Corona's Lua listeners.
 * <p>
 * This class is expected to be used by an activity to listen for input events from its root content view.
 */
public class ViewInputHandler extends InputHandler {
	/** The view that this handler listens for input events from. */
	private android.view.View fView;

	/** The handler that will receive event from the view and send them to this input handler. */
	private EventHandler fEventHandler;


	/** Creates a new input handler. */
	public ViewInputHandler(com.ansca.corona.Controller controller) {
		super(controller);

		if (android.os.Build.VERSION.SDK_INT >= 12) {
			fEventHandler = ApiLevel12.createEventHandlerWith(this);
		}
		else {
			fEventHandler = new EventHandler(this);
		}
		fView = null;
	}

	/**
	 * Sets the view that this handler will listen for input events from.
	 * <p>
	 * This handler will replace the given view's event listeners by calling its setOn*Listener() methods.
	 * @param view Reference to the view this handler will listen for input events from.
	 *             Expected to be the root "content" view of an activity.
	 *             <p>
	 *             Set to null to have this handler stop listening to the last assigned view's events.
	 */
	public void setView(android.view.View view) {
		// Do not continue if the view reference has not changed.
		if (view == fView) {
			return;
		}

		// Remove the previous view's event listeners.
		if (fView != null) {
			fEventHandler.unsubscribe();
		}

		// Store the given view reference.
		fView = view;

		// Add event listeners to the given view.
		if (fView != null) {
			fEventHandler.subscribe();
		}
	}

	/**
	 * Gets the view that this handler is listening for input events from.
	 * @return Returns the view that was assigned to this handler via setView().
	 *         <p>
	 *         Returns null if this handler is not currently assigned a view.
	 */
	public android.view.View getView() {
		return fView;
	}


	/**
	 * Receives API Level 8 input events from a view and sends those events to a ViewInputHandler object.
	 */
	private static class EventHandler implements
		android.view.View.OnKeyListener, android.view.View.OnTouchListener
	{
		/** Reference to the input handler that owns this object. */
		private ViewInputHandler fInputHandler;


		/**
		 * Creates a new event handler object.
		 * @param inputHandler Reference to the input handler that will provide the view to receive
		 *                     input events from and that this object will send the received events to.
		 *                     <p>
		 *                     Cannot be null or else an exception will occur.
		 */
		public EventHandler(ViewInputHandler inputHandler) {
			if (inputHandler == null) {
				throw new NullPointerException();
			}
			fInputHandler = inputHandler;
		}

		/**
		 * Gets the input handler this object will send the view's input events to.
		 * @return Reference to the input handler.
		 */
		public ViewInputHandler getInputHandler() {
			return fInputHandler;
		}

		/** Assigns input listeners to the view for handling events. */
		public void subscribe() {
			setViewListenersTo(this);
		}

		/** Removes this object's input listeners from the view. */
		public void unsubscribe() {
			setViewListenersTo(null);
		}

		/**
		 * Sets the view's input listeners to the given reference.
		 * @param handler The reference to be passed to the view's listener methods.
		 *                <p>
		 *                Set to this object to subscribe to the view's input events.
		 *                <p>
		 *                Set to null to unsubscribe from the view's input events.
		 */
		protected void setViewListenersTo(EventHandler handler) {
			android.view.View view = fInputHandler.getView();
			if (view != null) {
				view.setOnKeyListener(handler);
				view.setOnTouchListener(handler);
			}
		}

		/**
		 * Called when a key has been pressed or released.
		 * @param view Reference to the view that has received the key event.
		 * @param keyCode Unique integer ID of the key, matching a key code constant in the KeyEvent class.
		 * @param event Provides all information about the key down/up event such as the key pressed,
		 *              modifiers such as Shift/Ctrl, and the device it came from.
		 * @return Returns true if the key event was handled. Returns false if not.
		 */
		@Override
		public boolean onKey(android.view.View view, int keyCode, android.view.KeyEvent event) {
			return fInputHandler.handle(event);
		}

		/**
		 * Called when a touch screen event has been received.
		 * @param view Reference to the view that has received the touch event.
		 * @param event Provides information about the touch event such as coordinates, number of pointers,
		 *              the input device that provided the touch event, etc.
		 * @return Returns true if the touch event was handled. Returns false if not.
		 */
		@Override
		public boolean onTouch(android.view.View view, android.view.MotionEvent event) {
			return fInputHandler.handle(event);
		}
	}


	/**
	 * Container for API Level 12 (Honeycomb) classes.
	 * Should only be accessed if running on an operating system matching this API Level.
	 * <p>
	 * You cannot create instances of this class.
	 */
	private static class ApiLevel12 {
		/** Constructor made private to prevent instances from being made. */
		private ApiLevel12() { }

		/**
		 * Creates a new EventHandler object that takes advantage of API Level 12 or higher features.
		 * @param inputHandler Reference to the input handler that will provide the view to receive
		 *                     input events from and that this object will send the received events to.
		 *                     <p>
		 *                     Cannot be null or else an exception will occur.
		 */
		public static ViewInputHandler.EventHandler createEventHandlerWith(ViewInputHandler inputHandler) {
			return new ApiLevel12.EventHandler(inputHandler);
		}

		/**
		 * Receives API Level 12 input events from a view and sends those events to a ViewInputHandler object.
		 */
		private static class EventHandler extends ViewInputHandler.EventHandler
			implements android.view.View.OnGenericMotionListener
		{
			/**
			 * Creates a new event handler object.
			 * @param inputHandler Reference to the input handler that will provide the view to receive
			 *                     input events from and that this object will send the received events to.
			 *                     <p>
			 *                     Cannot be null or else an exception will occur.
			 */
			public EventHandler(ViewInputHandler inputHandler) {
				super(inputHandler);
			}

			/**
			 * Sets the view's input listeners to the given reference.
			 * @param handler The reference to be passed to the view's listener methods.
			 *                <p>
			 *                Set to this object to subscribe to the view's input events.
			 *                <p>
			 *                Set to null to unsubscribe from the view's input events.
			 */
			@Override
			protected void setViewListenersTo(ViewInputHandler.EventHandler handler) {
				super.setViewListenersTo(handler);

				android.view.View view = getInputHandler().getView();
				if (view != null) {
					view.setOnGenericMotionListener((EventHandler)handler);
				}
			}

			/**
			 * Called when an analog axis or mouse move event has been received.
			 * @param view Reference to the view that has received the motion event.
			 * @param event Provides information about the motion.
			 * @return Returns true if the motion event was handled. Returns false if not.
			 */
			@Override
			public boolean onGenericMotion(android.view.View view, android.view.MotionEvent event) {
				return getInputHandler().handle(event);
			}
		}
	}
}
