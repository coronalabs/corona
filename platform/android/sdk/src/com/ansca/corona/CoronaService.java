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


/** Service used to perform Corona's backgrounded operations. */
public class CoronaService extends android.app.Service {
	/** Binder used to provide external access to this Corona service. */
	private Binder fBinder = null;


	/** Called when this service has been created by the Android operating system. */
	@Override
	public void onCreate() {
	}

	/** Called when this service is about to be destroyed by the Android operating system. */
	@Override
	public void onDestroy() {
	}

	/**
	 * Called when this service has been started.
	 * @param intent The intent used to start this service. Can be null.
	 * @param flags Provides additional information such as START_FLAG_RETRY, START_FLAG_REDELIVERY, etc.
	 * @param startId Unique integer ID representing the specific feature to start.
	 *                This is not currently supported by Corona.
	 */
    @Override
    public int onStartCommand(android.content.Intent intent, int flags, int startId) {
        return android.app.Service.START_STICKY;
    }

    /**
     * Called when external code which is to connect this service via the Context.bindService() method.
     * @param intent The intent passed by the caller to this service.
     * @return Returns a binder used to access this service.
     */
	@Override
	public android.os.IBinder onBind(android.content.Intent intent) {
		if (fBinder == null) {
			fBinder = new CoronaService.Binder(this);
		}
		return fBinder;
	}


	/** Internal class providing external access to a CoronaService. */
    private static class Binder extends android.os.Binder {
    	/** Reference to the Corona service object this binder provides access to. */
    	private CoronaService fService;

    	/**
    	 * Creates a new binder for the given CoronaService object.
    	 * @param service Reference to the CoronaService object that this binder will provide access to.
    	 */
    	public Binder(CoronaService service) {
    		fService = service;
    	}

    	/**
    	 * Gets the Corona service object this binder provides access to.
    	 * @return Returns a reference to the Corona service.
    	 */
    	CoronaService getService() {
    		return fService;
    	}
    }
}
