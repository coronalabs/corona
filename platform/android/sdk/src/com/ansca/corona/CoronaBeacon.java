//////////////////////////////////////////////////////////////////////////////
//
// This file is part of the Corona game engine.
// For overview and more information on licensing please refer to README.md 
// Home page: https://github.com/coronalabs/corona
// Contact: support@coronalabs.com
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
