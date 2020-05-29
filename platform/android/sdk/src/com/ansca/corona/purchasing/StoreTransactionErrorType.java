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
 * Indicates the type of error that occurred during a transaction, if any.
 * Provides integer IDs matching error types in Corona's C++ enum PlatformStoreTransaction::ErrorType.
 * <p>
 * Instances of this class are immutable.
 * <p>
 * You cannot create instances of this class.
 * Instead, you use the pre-allocated objects from this class' static methods and fields.
 */
public class StoreTransactionErrorType {
	/** The unique integer ID of this error. */
	private int fNumericId;
	
	/**
	 * Creates a new error type object with the given integer ID.
	 * @param errorTypeId Unique integer ID of the error corresponding to an item in
	 *                    Corona's C++ enum PlatformStoreTransaction::ErrorType.
	 */
	private StoreTransactionErrorType(int errorTypeId) {
		fNumericId = errorTypeId;
	}
	
	/** Indicates that no error occurred during the transaction. */
	public static final StoreTransactionErrorType NONE = new StoreTransactionErrorType(0);
	
	/** Indicates that an unknown or unexpected error occurred during the transaction or marketplace request. */
	public static final StoreTransactionErrorType UNKNOWN = new StoreTransactionErrorType(1);
	
	/** Indicates that the client is not allowed to perform the attempted action. */
	public static final StoreTransactionErrorType CLIENT_INVALID = new StoreTransactionErrorType(2);
	
	/** Indicates that the user canceled the transaction. */
	public static final StoreTransactionErrorType PAYMENT_CANCELED = new StoreTransactionErrorType(3);
	
	/** Indicates that the marketplace did not recognize the payment settings provided by the client. */
	public static final StoreTransactionErrorType PAYMENT_INVALID = new StoreTransactionErrorType(4);
	
	/** Indicates that the user is not authorized to make purchases. */
	public static final StoreTransactionErrorType PAYMENT_NOT_ALLOWED = new StoreTransactionErrorType(5);
	
	
	/**
	 * Gets the unique integer ID of this error matching an item in Corona's C++ enum PlatformStoreTransaction::ErrorType.
	 * @return Returns this error's unqiue integer ID.
	 */
	public int toValue() {
		return fNumericId;
	}
	
	/**
	 * Gets the transaction error type for the given integer ID.
	 * @param errorTypeId Unique integer ID of the error corresponding to an item in Corona's C++ enum PlatformStoreTransaction::ErrorType.
	 * @return Returns the error object matching the given integer ID.
	 *         Returns an UNKNOWN error object if the ID was not recognized or the ID was set to 1.
	 */
	public static StoreTransactionErrorType fromValue(int errorTypeId) {
		// Return a pre-allocated object matching the given integer ID via reflection.
		try {
			for (java.lang.reflect.Field field : StoreTransactionErrorType.class.getDeclaredFields()) {
				if (field.getType().equals(StoreTransactionErrorType.class)) {
					StoreTransactionErrorType errorType = (StoreTransactionErrorType)field.get(null);
					if (errorType.toValue() == errorTypeId) {
						return errorType;
					}
				}
			}
		}
		catch (Exception ex) { }
		
		// The given integer ID is unknown. Return an unknown error type.
		return StoreTransactionErrorType.UNKNOWN;
	}
}
