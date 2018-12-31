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

import com.naef.jnlua.JavaFunction;
import com.ansca.corona.CoronaRuntimeTaskDispatcher;

/**
 * Implements the Lua interface for the plugin.
 * <p>
 * Only one instance of this class will be created by Corona for the lifetime of the application.
 * This instance will be re-used for every new Corona activity that gets created.
 */
public class CoronaBeacon {
  public static final String REQUEST    = "request";
  public static final String IMPRESSION = "impression";
  public static final String DELIVERY   = "delivery";

  // Send the device data to the Corona plugin beacon endpoint
  public static int sendDeviceDataToBeacon(final CoronaRuntimeTaskDispatcher runtimeTaskDispatcher, final String pluginName, final String pluginVersion, final String eventType, final String placementId, final JavaFunction networkListener)
  {
    return 0;
  }
}
