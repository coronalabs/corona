//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
