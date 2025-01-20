//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
//
//////////////////////////////////////////////////////////////////////////////

package com.ansca.corona;

import com.naef.jnlua.LuaState;
import com.naef.jnlua.LuaType;

// ----------------------------------------------------------------------------

/**
 * <font face="Courier New" color="black">CoronaLuaEvent.java</font> contains generic property keys used in Corona 
 * <a href="http://docs.coronalabs.com/daily/api/type/Event.html">events</a> dispatched to Lua.
 */
public class CoronaLuaEvent {
	/**
	 * Generic name key.
	 * <p>
	 * &emsp; Constant Value: "name"
	 */
	public static final String NAME_KEY = "name";

	/**
	 * Generic error key.
	 * <p>
	 * &emsp; Constant Value: "isError"
	 */
	public static final String ISERROR_KEY = "isError";

	/**
	 * Generic provider key.
	 * <p>
	 * &emsp; Constant Value: "provider"
	 */
	public static final String PROVIDER_KEY = "provider";

	/**
	 * Generic verification key.
	 * <p>
	 * &emsp; Constant Value: "isVerified"
	 */
	public static final String ISVERIFIED_KEY = "isVerified";

	/**
	 * Generic response key.
	 * <p>
	 * &emsp; Constant Value: "response"
	 */
	public static final String RESPONSE_KEY = "response";

	/**
	 * Generic error type key.
	 * <p>
	 * &emsp; Constant Value: "errorType"
	 */
	public static final String ERRORTYPE_KEY = "errorType";

	/**
	 * Generic configuration key.
	 * <p>
	 * &emsp; Constant Value: "configuration"
	 */
	public static final String CONFIGURATION_ERROR = "configuration";

	/**
	 * Generic network key.
	 * <p>
	 * &emsp; Constant Value: "network"
	 */
	public static final String NETWORK_ERROR = "network";

	/**
	 * Generic ads request name key.
	 * <p>
	 * &emsp; Constant Value: "adsRequest"
	 */
	public static final String ADSREQUEST_TYPE = "adsRequest";

	/**
	 * Generic license type key.
	 * <p>
	 * &emsp; Constant Value: "licensing"
	 */
	public static final String LICENSE_REQUEST_TYPE = "licensing";
}


// ----------------------------------------------------------------------------
