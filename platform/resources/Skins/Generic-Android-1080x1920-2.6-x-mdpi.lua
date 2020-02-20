------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

-- Based on the Google Pixel and Nexus 5X
-- https://material.io/devices/
simulator =
{
	device = "borderless-android-device",
	screenOriginX = 0,
	screenOriginY = 0,
	screenWidth = 1080,
	screenHeight = 1920,
	androidDisplayApproximateDpi = 416, -- 2.6 x mdpi
	displayManufacturer = "Corona Labs Inc.",
	displayName = "GenericAndroidDevice",
	windowTitleBarName = "Android (2.6 * mdpi)",
}
simulator.defaultFontSize = 18.0 * (simulator.androidDisplayApproximateDpi / 160)
