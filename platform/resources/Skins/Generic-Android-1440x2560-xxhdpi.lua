------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

-- Based on the LG G3 and Samsung Galaxy Note 4
-- https://material.io/devices/
simulator =
{
	device = "borderless-android-device",
	screenOriginX = 0,
	screenOriginY = 0,
	screenWidth = 1440,
	screenHeight = 2560,
	androidDisplayApproximateDpi = 480, -- xxhdpi
	displayManufacturer = "Corona Labs Inc.",
	displayName = "GenericAndroidDevice",
	windowTitleBarName = "Android (xxhdpi)",
}
simulator.defaultFontSize = 18.0 * (simulator.androidDisplayApproximateDpi / 160)
