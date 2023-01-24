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
	screenOriginY = 110,
	screenWidth = 1080,
	screenHeight = 2340,
	safeScreenInsetTop = 130,
	safeScreenInsetLeft = 0,
	safeScreenInsetBottom = 80,
	safeScreenInsetRight = 0,
	safeLandscapeScreenInsetTop = 0 * 3,
	safeLandscapeScreenInsetLeft = 130,
	safeLandscapeScreenInsetBottom = 0 * 3,
	safeLandscapeScreenInsetRight = 80,
	androidDisplayApproximateDpi = 420, -- xxhdpi
	deviceImage = "Pixel-4a.png",
	displayManufacturer = "google",
	displayName = "GA02099",
	statusBarDefault = "Pixel-4aStatusBarTransparent.png",
	statusBarTranslucent = "Pixel-4aStatusBarTransparent.png",
	statusBarBlack = "Pixel-4aStatusBarBlack.png",
	statusBarLightTransparent = "Pixel-4aStatusBarWhite.png",
	statusBarDarkTransparent = "Pixel-4aStatusBarBlack.png",
	screenDressing = "Pixel-4aScreenDressing.png",
	windowTitleBarName = "Pixel 4a",
}
simulator.defaultFontSize = 18.0 * (simulator.androidDisplayApproximateDpi / 160)
