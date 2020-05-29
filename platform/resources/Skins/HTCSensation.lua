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
	screenOriginX = 54,
	screenOriginY = 170,
	screenWidth = 540,
	screenHeight = 960,
	androidDisplayApproximateDpi = 240, -- hdpi
	deviceImage = "HTCSensation.png",
	displayManufacturer = "HTC",
	displayName = "Sensation",
	statusBarDefault = "", 
	statusBarTranslucent = "", 
	statusBarBlack = "", 
	statusBarLightTransparent = "",
	statusBarDarkTransparent = "",
	windowTitleBarName = "HTC Sensation",
}
simulator.defaultFontSize = 18.0 * (simulator.androidDisplayApproximateDpi / 160)
