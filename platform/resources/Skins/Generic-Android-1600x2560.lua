------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

-- Based on the Dell Venue 8, Nexus 10, Sony Xperia Z4 Tablet, Kindle Fire HDX 8.9" (2013), and Kindle Fire HDX 8.9 (2014)
-- https://material.io/devices/
-- https://developer.amazon.com/public/solutions/devices/fire-tablets/specifications/01-device-and-feature-specifications
simulator =
{
	device = "borderless-android-device",
	screenOriginX = 0,
	screenOriginY = 0,
	screenWidth = 1600,
	screenHeight = 2560,
	androidDisplayApproximateDpi = 320, -- xhdpi
	displayManufacturer = "Corona Labs Inc.",
	displayName = "GenericAndroidDevice",
	windowTitleBarName = "Android (xhdpi)",
}
simulator.defaultFontSize = 18.0 * (simulator.androidDisplayApproximateDpi / 160)
