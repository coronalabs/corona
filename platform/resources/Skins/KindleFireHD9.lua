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
	screenOriginX = 102,
	screenOriginY = 100,
	screenWidth = 1200,
	screenHeight = 1920,
	androidDisplayApproximateDpi = 240, -- hdpi
	deviceImage = "KindleFireHD9.png",
	displayManufacturer = "Amazon",
	displayName = "KFJWI",
	statusBarDefault = "", 
	statusBarTranslucent = "", 
	statusBarBlack = "", 
	statusBarLightTransparent = "",
	statusBarDarkTransparent = "",
	windowTitleBarName = 'Kindle Fire HD 8.9" (2012)',
}
simulator.defaultFontSize = 18.0 * (simulator.androidDisplayApproximateDpi / 160)
