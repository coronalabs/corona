------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

simulator =
{
	device = "android-phone",
	screenOriginX = 75,
	screenOriginY = 223,
	screenWidth = 1080,
	screenHeight = 1920,
	androidDisplayApproximateDpi = 480, -- xxhdpi
	deviceImage = "Samsung-Galaxy-S5.png",
	displayManufacturer = "samsung",
	displayName = "SM-G900S",
	supportsScreenRotation = true,
	hasAccelerometer = true,
	windowTitleBarName = "Samsung Galaxy S5",
}
simulator.defaultFontSize = 18.0 * (simulator.androidDisplayApproximateDpi / 160)
