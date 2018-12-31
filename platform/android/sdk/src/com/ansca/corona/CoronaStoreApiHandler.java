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

package com.ansca.corona;

/** The interface has all the funcations that are activity specific and aren't implemented by default. */
class CoronaStoreApiHandler implements com.ansca.corona.listeners.CoronaStoreApiListener{	
	private CoronaActivity fActivity;

	public CoronaStoreApiHandler(CoronaActivity activity) {
		fActivity = activity;
	}

	@Override
	public void storeInit(String storeName) {
		if (fActivity == null) {
			return;
		}

		final String finalStoreName = storeName;
		fActivity.runOnUiThread(new Runnable() {
			public void run() {
				if (fActivity != null && fActivity.getRuntime() != null && fActivity.getStore() != null) {
					com.ansca.corona.purchasing.StoreProxy store = fActivity.getStore();
					// Alert the user that they need to upgrade to Google IAP v3 as of January 2015.
					fActivity.getRuntime().getController().showStoreDeprecatedAlert();
					store.enable();
				}
			}
		});
	}
	public void storePurchase(String productName) {
		if (fActivity == null) {
			return;
		}

		final String finalProductName = productName;
		fActivity.runOnUiThread(new Runnable() {
			public void run() {
				if (fActivity != null) {
					fActivity.getStore().purchase(finalProductName);
				}
			}
		});
	}

	@Override
	public void storeFinishTransaction(String transactionStringId) {
		if (fActivity == null) {
			return;
		}

		final String finalTransactionStringId = transactionStringId;
		fActivity.runOnUiThread(new Runnable() {
			public void run() {
				if (fActivity != null) {
					fActivity.getStore().confirmTransaction(finalTransactionStringId);
				}
			}
		});
	}

	@Override
	public void storeRestore() {
		if (fActivity == null) {
			return;
		}

		fActivity.runOnUiThread(new Runnable() {
			public void run() {
				if (fActivity != null) {
					fActivity.getStore().restorePurchases();
				}
			}
		});
	}
}
