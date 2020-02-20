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
	screenOriginX = 58,
	screenOriginY = 190,
	screenWidth = 720,
	screenHeight = 1280,
	androidDisplayApproximateDpi = 320, -- xhdpi
	deviceImage = "GalaxyS3.png",
	displayManufacturer = "samsung",
	displayName = "GT-I9300",
	statusBarDefault = "", 
	statusBarTranslucent = "", 
	statusBarBlack = "", 
	statusBarLightTransparent = "",
	statusBarDarkTransparent = "",
	windowTitleBarName = "Samsung Galaxy S3",
}
simulator.defaultFontSize = 18.0 * (simulator.androidDisplayApproximateDpi / 160)
