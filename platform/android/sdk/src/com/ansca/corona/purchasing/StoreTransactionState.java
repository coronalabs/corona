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
 * Indicates the state of the transaction such as PURCHASED, FAILED, CANCELED, etc.
 * Provides integer IDs matching states in Corona's C++ enum PlatformStoreTransaction::State.
 * <p>
 * Instances of this class are immutable.
 * <p>
 * You cannot create instances of this class.
 * Instead, you use the pre-allocated objects from the class' static methods and fields.
 */
public class StoreTransactionState {
	/** The unique integer ID of this state. */
	private int fStateId;
	
	/**
	 * Creates a new state object with the given integer ID.
	 * @param stateId Unique integer ID of the state corresponding to an item in Corona's C++ enum PlatformStoreTransaction::State.
	 */
	private StoreTransactionState(int stateId) {
		fStateId = stateId;
	}
	
	/** Indicates that the transaction is in an unknown state. */
	public static final StoreTransactionState UNDEFINED = new StoreTransactionState(0);
	
	/** State indicating that a product is currently being purchased. */
	public static final StoreTransactionState PURCHASING = new StoreTransactionState(1);
	
	/** State indicating that product(s) have been purchased successfully. */
	public static final StoreTransactionState PURCHASED = new StoreTransactionState(2);
	
	/** State indicating that a transaction has failed. */
	public static final StoreTransactionState FAILED = new StoreTransactionState(3);
	
	/** State indicating that past purchased products and transaction have been restored. */
	public static final StoreTransactionState RESTORED = new StoreTransactionState(4);
	
	/** State indicating that a transaction/purchase has been canceled by the user or the marketplace. */
	public static final StoreTransactionState CANCELED = new StoreTransactionState(5);
	
	/** State indicating that a transaction/purchase was refunded. */
	public static final StoreTransactionState REFUNDED = new StoreTransactionState(6);
	
	/**
	 * Gets the unique integer ID of this state matching an item in Corona's C++ enum PlatformStoreTransaction::State.
	 * @return Returns this state's unqiue integer ID.
	 */
	public int toValue() {
		return fStateId;
	}
	
	/**
	 * Gets the transaction state for the given integer ID.
	 * @param stateId Unique integer ID of the state corresponding to an item in Corona's C++ enum PlatformStoreTransaction::State.
	 * @return Returns the state object matching the given integer ID.
	 *         Returns an UNDEFINED state object if the ID was not recognized or the ID was set to zero.
	 */
	public static StoreTransactionState fromValue(int stateId) {
		// Return a pre-allocated state object matching the given integer ID via reflection.
		try {
			for (java.lang.reflect.Field field : StoreTransactionState.class.getDeclaredFields()) {
				if (field.getType().equals(StoreTransactionState.class)) {
					StoreTransactionState state = (StoreTransactionState)field.get(null);
					if (state.toValue() == stateId) {
						return state;
					}
				}
			}
		}
		catch (Exception ex) { }
		
		// The given integer ID is unknown. Return an undefined state.
		return StoreTransactionState.UNDEFINED;
	}
}
