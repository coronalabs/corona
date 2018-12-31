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
