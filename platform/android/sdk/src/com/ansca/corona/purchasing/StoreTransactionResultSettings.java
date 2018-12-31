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

package com.ansca.corona.purchasing;


/** Stores the result of a transaction with the marketplace. */
public class StoreTransactionResultSettings {
	/** Indicates the current state of the transaction. */
	private StoreTransactionState fState;
	
	/** Indicates the error that occurred during the transaction. Set to NONE if no error occurred. */
	private StoreTransactionErrorType fErrorType;
	
	/** Message indicating why an error occurred. Set to an empty string if no error occurred. */
	private String fErrorMessage;
	
	/** The unique name of the product involved in the transaction. */
	private String fProductName;
	
	/** The signature string that the transaction was digitally signed with. */
	private String fSignature;
	
	/** The receipt of the transaction in string form. */
	private String fReceipt;
	
	/** The unique string ID assigned to the transaction. */
	private String fTransactionStringId;
	
	/** The date and time the transaction occurred. */
	private java.util.Date fTransactionTime;
	
	/** The receipt of the restored transaction in string form. Only applicable if this is a "restore" transaction. */
	private String fOriginalReceipt;
	
	/** The unique string ID assigned to the restored purchase transaction. Only applicable if this is a "restore" transaction. */
	private String fOriginalTransactionStringId;
	
	/** The date and time of the restored purchase transaction. Only applicable if this is a "restore" transaction. */
	private java.util.Date fOriginalTransactionTime;
	
	
	/** Creates a new object for storing the result of a transaction. */
	public StoreTransactionResultSettings() {
		fState = StoreTransactionState.UNDEFINED;
		fErrorType = StoreTransactionErrorType.UNKNOWN;
		fErrorMessage = "";
		fProductName = "";
		fSignature = "";
		fReceipt = "";
		fTransactionStringId = "";
		fTransactionTime = null;
		fOriginalReceipt = "";
		fOriginalTransactionStringId = "";
		fOriginalTransactionTime = null;
	}
	
	/**
	 * Gets current state of the transaction.
	 * @return Returns the state of the transaction.
	 */
	public StoreTransactionState getState() {
		return fState;
	}
	
	/**
	 * Sets the state of the transaction.
	 * @param value The state of the transaction such as PURCHASED, CANCELED, FAILED, etc.
	 *              Setting this to null will set the state to UNDEFINED.
	 */
	public void setState(StoreTransactionState value) {
		if (value == null) {
			value = StoreTransactionState.UNDEFINED;
		}
		fState = value;
	}
	
	/**
	 * Gets the error that occurred during the transaction, if any.
	 * @return Returns the type of error that occurred such as PAYMENT_CANCELED, PAYMENT_NOT_ALLOWED, etc.
	 *         Returns NONE if no error occurred.
	 */
	public StoreTransactionErrorType getErrorType() {
		return fErrorType;
	}
	
	/**
	 * Sets the type of error that occurred during the transaction, if any.
	 * @param value The type of error that occurred. Set to NONE if no error occurred.
	 *              Setting this to null will set the error type to UNKNOWN.
	 */
	public void setErrorType(StoreTransactionErrorType value) {
		if (value == null) {
			value = StoreTransactionErrorType.UNKNOWN;
		}
		fErrorType = value;
	}
	
	/**
	 * Gets a descriptive message indicating what went wrong during the transaction if an error occurred.
	 * @return Returns a message indicating what went wrong during a transaction.
	 *         Returns an empty string if no error occurred.
	 */
	public String getErrorMessage() {
		return fErrorMessage;
	}
	
	/**
	 * Sets an error message describing what went wrong during the transaction if applicable.
	 * @param value A string describing the error that occurred during the transaction.
	 *              Set to null or an empty string if no error occurred.
	 */
	public void setErrorMessage(String value) {
		if (value == null) {
			value = "";
		}
		fErrorMessage = value;
	}
	
	/**
	 * Gets the unique name of the product involved in the transaction.
	 * @return Returns the unique name of the product.
	 *         Returns an empty string if a product was not involved during the transaction.
	 */
	public String getProductName() {
		return fProductName;
	}
	
	/**
	 * Sets the unique name of the product involved in the transaction.
	 * @param value The unique string ID of the product involved in the transaction.
	 *              Set to null or empty string if a product was not involved.
	 */
	public void setProductName(String value) {
		if (value == null) {
			value = "";
		}
		fProductName = value;
	}
	
	/**
	 * Gets the signature that the transaction was digitally signed with.
	 * @return Returns the transaction's signature in string form.
	 *         Returns an empty string if the marketplace did not provide a signature.
	 */
	public String getSignature() {
		return fSignature;
	}
	
	/**
	 * Sets the signature that the transaction was digitally signed with.
	 * @param value The transaction's signature in string form.
	 *              Set to null or empty string if the marketplace did not provide a signature.
	 */
	public void setSignature(String value) {
		if (value == null) {
			value = "";
		}
		fSignature = value;
	}
	
	/**
	 * Gets the receipt of the transaction in string form.
	 * @return Returns the transaction receipt in string form.
	 *         Returns an empty string if the marketplace did not provide a receipt.
	 */
	public String getReceipt() {
		return fReceipt;
	}
	
	/**
	 * Sets the receipt of the transaction in string form.
	 * @param value The transaction's receipt in string form.
	 *              Set to null or empty string if the marketplace did not provide a receipt.
	 */
	public void setReceipt(String value) {
		if (value == null) {
			value = "";
		}
		fReceipt = value;
	}
	
	/**
	 * Gets the unique string ID assigned to the transaction.
	 * Needed to confirm transactions to finalize purchases and restore operations.
	 * @return Returns the unique string ID of the transaction.
	 *         Returns an empty string if not assigned which typically happens if unable to communiate with the marketplace.
	 */
	public String getTransactionStringId() {
		return fTransactionStringId;
	}
	
	/**
	 * Sets the unique string ID assigned to the transaction.
	 * @param value The unique string ID assigned to the transaction.
	 *              Set to null or empty string if unable to start a transaction,
	 *              such as a failure to communicate with the marketplace.
	 */
	public void setTransactionStringId(String value) {
		if (value == null) {
			value = "";
		}
		fTransactionStringId = value;
	}
	
	/**
	 * Indicates if the transaction has a timestamp of when it occurred.
	 * @return Returns true if the getTransactionTime() method provides a timestamp.
	 *         Returns false if not, which can happen if unable to communicate with the marketplace.
	 */
	public boolean hasTransactionTime() {
		return (fTransactionTime != null);
	}
	
	/**
	 * Gets the date and time when the transaction occurred, such as when the product was purchased.
	 * You should only call this method if the hasTransactionTime() method return true.
	 * @return Returns the date and time when the transaction occurred with the marketplace.
	 *         Returns null if the transaction did not occur, which can happen if unable to communicate with the marketplace.
	 */
	public java.util.Date getTransactionTime() {
		if (fTransactionTime == null) {
			return null;
		}
		return new java.util.Date(fTransactionTime.getTime());
	}
	
	/**
	 * Sets the date and time the transaction occurred with the marketplace.
	 * @param value The date and time the transaction occurred.
	 *              Set to null if the transaction did not occurr.
	 */
	public void setTransactionTime(java.util.Date value) {
		fTransactionTime = value;
	}
	
	/**
	 * Gets the receipt of the restored transaction in string form.
	 * Only applicable if this is a "restore" transaction.
	 * @return Returns the receipt as a string.
	 *         Returns an empty string if this is not a "restore" transaction.
	 */
	public String getOriginalReceipt() {
		return fOriginalReceipt;
	}
	
	/**
	 * Sets the receipt of the restored transaction.
	 * Only applicable if this is a "restore" transaction.
	 * @param value The receipt of the restored transaction in string form.
	 *              Set to null or empty string if this is not a "restore" transaction.
	 */
	public void setOriginalReceipt(String value) {
		if (value == null) {
			value = "";
		}
		fOriginalReceipt = value;
	}
	
	/**
	 * Gets the unique string ID assigned to the restored transaction.
	 * Only applicable if this is a "restore" transaction.
	 * @return Returns the unique string ID of the restored transaction.
	 *         Returns an empty string if this is not a "restore" transaction.
	 */
	public String getOriginalTransactionStringId() {
		return fOriginalTransactionStringId;
	}
	
	/**
	 * Sets the unique string ID assigned to the restored transaction.
	 * Only applicable if this is a "restore" transaction.
	 * @param value The unique string ID assigned to the transaction.
	 *              Set to null or empty string if this is not a "restore" transaction.
	 */
	public void setOriginalTransactionStringId(String value) {
		if (value == null) {
			value = "";
		}
		fOriginalTransactionStringId = value;
	}
	
	/**
	 * Indicates if the restored transaction has a timestamp of when it occurred.
	 * Only applicable if this is a "restore" transaction.
	 * @return Returns true if the getOriginalTransactionTime() method provides a timestamp.
	 *         Returns false if not, which can happen if this is not a restore transaction.
	 */
	public boolean hasOriginalTransactionTime() {
		return (fOriginalTransactionTime != null);
	}
	
	/**
	 * Gets the date and time when the restored transaction occurred.
	 * Only applicable if this is a "restore" transaction.
	 * @return Returns the date and time the restored transaction occurred.
	 *         Returns null if this is not a "restore" transaction.
	 */
	public java.util.Date getOriginalTransactionTime() {
		if (fOriginalTransactionTime == null) {
			return null;
		}
		return new java.util.Date(fOriginalTransactionTime.getTime());
	}
	
	/**
	 * Sets the date and time when the restored transaction occurred.
	 * Only applicable if this is a "restore" transaction.
	 * @param value The date and time when the restored transaction occurred.
	 *              Set to null if this is not a "restore" transaction.
	 */
	public void setOriginalTransactionTime(java.util.Date value) {
		fOriginalTransactionTime = value;
	}
}
