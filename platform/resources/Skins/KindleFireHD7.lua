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
	device = "android-tablet",
	screenOriginX = 72,
	screenOriginY = 70,
	screenWidth = 800,
	screenHeight = 1280,
	androidDisplayApproximateDpi = 240, -- hdpi
	deviceImage = "KindleFireHD7.png",
	displayManufacturer = "Amazon",
	displayName = "KFTT",
	statusBarDefault = "", 
	statusBarTranslucent = "", 
	statusBarBlack = "", 
	statusBarLightTransparent = "",
	statusBarDarkTransparent = "",
	windowTitleBarName = 'Kindle Fire HD 7" (2012)',
}
simulator.defaultFontSize = 18.0 * (simulator.androidDisplayApproximateDpi / 160)
