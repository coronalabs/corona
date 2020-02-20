------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

-- Based on the Samsung Galaxy Tab 10 and Kindle Fire HD 10 (2015)
-- https://material.io/devices/
-- https://developer.amazon.com/public/solutions/devices/fire-tablets/specifications/01-device-and-feature-specifications
simulator =
{
	device = "borderless-android-device",
	screenOriginX = 0,
	screenOriginY = 0,
	screenWidth = 800,
	screenHeight = 1280,
	androidDisplayApproximateDpi = 160, -- mdpi
	displayManufacturer = "Corona Labs Inc.",
	displayName = "GenericAndroidDevice",
	windowTitleBarName = "Android (mdpi)",
}
simulator.defaultFontSize = 18.0 * (simulator.androidDisplayApproximateDpi / 160)
