//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.purchasing;


/**
 * Base class used to manage in-app purchases between Corona and the online store.
 * Derived classes are expected to implement in-app purchasing with their respective stores
 * such as "Android Billing", "Amazon", etc.
 */
public abstract class Store {
	/** Set true if access to the store has been enabled allowing requests to be sent and transactions automatically managed. */
	private boolean fIsEnabled;
	
	/** The activity that owns this object. */
	private android.app.Activity fActivity;
	
	protected com.ansca.corona.CoronaRuntime fCoronaRuntime;
	protected com.ansca.corona.Controller fController;

	/** Creates a new store accessing object. */
	public Store(com.ansca.corona.CoronaRuntime runtime, com.ansca.corona.Controller controller) {
		super();
		
		fController = controller;
		fCoronaRuntime = runtime;
		fIsEnabled = false;
		fActivity = null;
	}
	
	/**
	 * Assigns an activity to be used to display in-app purchasing windows.
	 * @param activity The activity to be used to display windows in.
	 */
	public void setActivity(android.app.Activity activity) {
		fActivity = activity;
	}
	
	/**
	 * Gets the activity used to handle and display in-app purchase windows.
	 * @return Returns the activity object used to display in-app purchase windows. Returns null if not set.
	 */
	public android.app.Activity getActivity() {
		return fActivity;
	}
	
	/** Enables access to the store. Must be called before sending any requests. */
	public void enable() {
		// Do not continue if already enabled.
		if (fIsEnabled) {
			return;
		}
		
		// It is an error to enable this object without assigning it an activity first.
		if (fActivity == null) {
			throw new java.lang.IllegalStateException();
		}
		
		// Notify the derived class that store access has been enabled.
		fIsEnabled = true;
		onEnabled();
	}
	
	/**
	 * Called when store access has been enabled via the enable() method.
	 * Must be overridden by a derived class to initialize and establish a connection to the store.
	 */
	protected abstract void onEnabled();
	
	/** Disables access to the store. Should be called when exiting the main activity. */
	public void disable() {
		// Do not continue if already disabled.
		if (fIsEnabled == false) {
			return;
		}
		
		// Notify the derived class that store access has been disabled.
		fIsEnabled = false;
		onDisabled();
	}
	
	/**
	 * Called when store access has been disabled via the disable() method.
	 * A derived class can optionally override this method to close and dispose its connection to the store.
	 */
	protected void onDisabled() {
	}
	
	/**
	 * Determines if store access has been enabled.
	 * @return Returns true if store access has been enabled and is ready to accept requests. Returns false if not.
	 */
	public boolean isEnabled() {
		return fIsEnabled;
	}
	
	/**
	 * Determines if store access has been disabled.
	 * @return Returns true if store access has been disabled, meaning all store requests will be ignored.
	 *         Returns false if store access is enabled.
	 */
	public boolean isDisabled() {
		return !fIsEnabled;
	}
	
	/**
	 * Determines if this application is currently capable of making in-app purchases.
	 * @return Returns true if able to make purchases. Returns false if not.
	 */
	public abstract boolean canMakePurchases();
	
	/**
	 * Sends a purchase request for the given product to the app store asynchronously.
	 * @param productName The unique name of the product to purchase.
	 */
	public abstract void purchase(String productName);
	
	/** Sends a restore request asynchronously for retrieving all past purchases from the store. */
	public abstract void restorePurchases();
	
	/**
	 * Sends a confirmation message to the app store asynchronously to finalize a transaction.
	 * @param transactionStringId Unique string ID of the transaction to finish.
	 */
	public abstract void confirmTransaction(String transactionStringId);
	
	/**
	 * Sends the transaction result to Corona's Lua listener.
	 * @param result Result of a store transaction. Cannot be null.
	 */
	protected void raiseTransactionEventFor(final StoreTransactionResultSettings result) {
		// Validate.
		if (result == null) {
			return;
		}
		
		// Fetch the event manager. Null indicates that the app is exiting.
		com.ansca.corona.events.EventManager eventManager = fController.getEventManager();
		if (eventManager == null) {
			return;
		}
		
		// Send the transaction result to the Lua listener.
		eventManager.addEvent(new com.ansca.corona.events.RunnableEvent(new java.lang.Runnable() {
			@Override
			public void run() {
				if (fController != null) {
					com.ansca.corona.JavaToNativeShim.storeTransactionEvent(fCoronaRuntime, result);
				}
			}
		}));
	}
}
