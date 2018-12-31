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
