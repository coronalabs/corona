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


/**
 * Receives events from the Corona runtime to detect when it is about to run main.lua
 * <p>
 * Instances of this interface are to be given to the CoronaEnvironment.addRuntimeListener() method.
 * <p>
 * Warning! All methods in this interface are not called on the main UI thread.
 * The Corona runtime and its Lua scripts run on a separate thread.
 * This means that you should not manipulate an activity's UI elements from these methods.
 * @see CoronaEnvironment
 * @see CoronaRuntime
 */
interface CoronaRuntimeWillLoadMainListener {
	/**
	 * Called after the Corona runtime has been created and just before executing the "main.lua" file.
	 * <p>
	 * Warning! This method is not called on the main thread.
	 * @param runtime Reference to the CoronaRuntime object that has just been loaded/initialized.
	 *                Provides a LuaState object that allows the application to extend the Lua API.
	 */
	public void onWillLoadMain(CoronaRuntime runtime);
}
