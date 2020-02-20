//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.listeners;

/** The interface has all the funcations that are activity specific and aren't implemented by default. */
public interface CoronaStoreApiListener{	
	/**
	 * Called from store.init().  This should initialize the store library.
	 * @param storeName The name of the store to initialize.
	 */
	public void storeInit(String storeName);

	/**
	 * Called from store.purchase().  The product passed into this function should be purchased.
	 * @param productName The name of the product to purchase.
	 */
	public void storePurchase(String productName);

	/**
	 * Called from store.finishTransaction().  This is called when the use has funished purchasing their transcations.
	 * @param transactionStringId The ID of the transaction that the script is finished interacting with.
	 */
	public void storeFinishTransaction(String transactionStringId);

	/**
	 * Called from store.restore().  Restore all previously purchased items.
	 */
	public void storeRestore();
}
