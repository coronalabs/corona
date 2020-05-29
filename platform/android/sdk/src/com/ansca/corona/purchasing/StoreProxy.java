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
 * Proxy class that wraps another store interface and passes all store operations to that wrapped store interface.
 * Can be assigned a store explicity such as through the useGoogleStore() method or can be set automatically
 * via the useDefaultStore() method. If a store is not assigned to this proxy then all store operations log warnings instead.
 * This proxy class supports changing stores at runtime, such as via the Lua store.init() function.
 * This class is intended to be used by the CoronaActivity to provide generic store access to Lua.
 */
public class StoreProxy extends Store {
	/**
	 * Message to be logged and provided to Lua listener when attempting to use this proxy before it was assigned a store.
	 * Assumes that this application does not support the store on this device if this is the case.
	 */
	private static final String STORE_NOT_SUPPORTED_WARNING_MESSAGE = "WARNING: This application does not support in-app purchases on this device.";
	
	/** The store to be accessed by this proxy. Set to null to not provide access to a store. */
	private Store fStore;
	
	/** Creates a new proxy for accessing a store. */
	public StoreProxy(com.ansca.corona.CoronaRuntime runtime, com.ansca.corona.Controller controller) {
		super(runtime, controller);
		
		fStore = null;
	}
	
	/**
	 * Assigns an activity to be used to display in-app purchasing windows.
	 * @param activity The activity to be used to display windows in.
	 */
	public void setActivity(android.app.Activity activity) {
		super.setActivity(activity);
		if (fStore != null) {
			fStore.setActivity(activity);
		}
	}
	
	/**
	 * Sets up this proxy to use the given store.
	 * @param storeName The unique name of the store to use matching a string constant in the StoreName class.
	 *                  Unknown or invalid store names will cause this proxy to use no store.
	 */
	public void useStore(String storeName) {
		
		// Since Google IAB v2 was deprecated and removed from Android, we don't use this at all.
		useNoStore();
	}
	
	/** Sets up this proxy to not support in-app purchases. All store methods will log warnings in this case. */
	public void useNoStore() {
		setStoreTo(null);
	}
	
	/**
	 * Sets up this proxy to use the given store.
	 * @param store The store that this proxy will use, such as an instance to GoogleStore.
	 *              Set to null to not allow access to a store in which case all store methods will do nothing.
	 */
	private void setStoreTo(Store store) {
		// Do not continue if this proxy has already been assigned the given reference.
		if (store == fStore) {
			return;
		}
		
		// Disable the previous store object.
		if (fStore != null) {
			fStore.disable();
		}
		
		// Set up the given store for this proxy.
		fStore = store;
		if (fStore != null) {
			fStore.setActivity(getActivity());
			if (isEnabled()) {
				fStore.enable();
			}
			else {
				fStore.disable();
			}
		}
	}
	
	/** Enables access to the store. Must be called before sending any requests. */
	public void enable() {
		super.enable();
		if (fStore != null) {
			fStore.enable();
		}
	}
	
	/**
	 * Called when store access has been enabled via the enable() method.
	 */
	protected void onEnabled() {
		// Do nothing. Instead, this proxy overrides the enable() method and explicitly call
		// its owned store's enable() method.
	}
	
	/** Disables access to the store. Should be called when exiting the main activity. */
	public void disable() {
		super.disable();
		if (fStore != null) {
			fStore.disable();
		}
	}
	
	/**
	 * Determines if this application is currently capable of making in-app purchases.
	 * @return Returns true if able to make purchases. Returns false if not.
	 */
	public boolean canMakePurchases() {
		if (fStore == null) {
			return false;
		}
		return fStore.canMakePurchases();
	}
	
	/**
	 * Sends a purchase request for the given product to the app store asynchronously.
	 * @param productName The unique name of the product to purchase.
	 */
	public void purchase(String productName) {
		// Do not continue if a store was not assigned.
		if (fStore == null) {
			StoreTransactionResultSettings result = new StoreTransactionResultSettings();
			result.setState(StoreTransactionState.FAILED);
			result.setErrorType(StoreTransactionErrorType.CLIENT_INVALID);
			result.setErrorMessage(STORE_NOT_SUPPORTED_WARNING_MESSAGE);
			result.setProductName(productName);
			raiseTransactionEventFor(result);
			logStoreNotSupportedWarning();
			return;
		}
		
		// Send the purchase request.
		fStore.purchase(productName);
	}
	
	/** Sends a restore request asynchronously for retrieving all past purchases from the store. */
	public void restorePurchases() {
		// Do not continue if a store was not assigned.
		if (fStore == null) {
			StoreTransactionResultSettings result = new StoreTransactionResultSettings();
			result.setState(StoreTransactionState.FAILED);
			result.setErrorType(StoreTransactionErrorType.CLIENT_INVALID);
			result.setErrorMessage(STORE_NOT_SUPPORTED_WARNING_MESSAGE);
			raiseTransactionEventFor(result);
			logStoreNotSupportedWarning();
			return;
		}
		
		// Send the restore request.
		fStore.restorePurchases();
	}
	
	/**
	 * Sends a confirmation message to the app store asynchronously to finalize a transaction.
	 * @param transactionStringId Unique string ID of the transaction to finish.
	 */
	public void confirmTransaction(String transactionStringId) {
		// Do not continue if a store was not assigned.
		if (fStore == null) {
			StoreTransactionResultSettings result = new StoreTransactionResultSettings();
			result.setState(StoreTransactionState.FAILED);
			result.setErrorType(StoreTransactionErrorType.CLIENT_INVALID);
			result.setErrorMessage(STORE_NOT_SUPPORTED_WARNING_MESSAGE);
			result.setTransactionStringId(transactionStringId);
			raiseTransactionEventFor(result);
			logStoreNotSupportedWarning();
			return;
		}
		
		// Send the confirmation.
		fStore.confirmTransaction(transactionStringId);
	}
	
	/**
	 * Logs a warning that this application does not support in-app purchases on the current device.
	 * To be called if this proxy object was not assigned a store.
	 */
	public void logStoreNotSupportedWarning() {
		android.util.Log.i("Corona", STORE_NOT_SUPPORTED_WARNING_MESSAGE);
	}
}
