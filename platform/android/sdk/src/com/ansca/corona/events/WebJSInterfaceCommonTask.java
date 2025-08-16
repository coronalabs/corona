//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona.events;

public class WebJSInterfaceCommonTask implements com.ansca.corona.CoronaRuntimeTask {
	private int fId;
	private String fType;
	private String fData;
	private boolean fNoResult;

	public WebJSInterfaceCommonTask(int id, String type, String data, boolean noResult) {
		fId = id;
		fType = type;
		fData = data;
		fNoResult = noResult;
	}

	@Override
	public void executeUsing(com.ansca.corona.CoronaRuntime runtime) {
		com.ansca.corona.JavaToNativeShim.webViewJSInterfaceCommonEvent(runtime, fId, fType, fData, fNoResult);
	}
}
