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
	screenOriginX = 35,
	screenOriginY = 80,
	screenWidth = 1440,
	screenHeight = 3088,
	safeScreenInsetTop = 100,
	safeScreenInsetLeft = 0 * 3,
	safeScreenInsetBottom = 30,
	safeScreenInsetRight = 0 * 3,
	safeLandscapeScreenInsetTop = 0 * 3,
	safeLandscapeScreenInsetLeft = 100,
	safeLandscapeScreenInsetBottom = 0 * 3,
	safeLandscapeScreenInsetRight = 80,
	androidDisplayApproximateDpi = 420, -- xxhdpi
	deviceImage = "Galaxy-Note20-Ultra.png",
	displayManufacturer = "samsung",
	displayName = "SM-N981x",
	statusBarDefault = "Galaxy-Note20-UltraStatusBarGray.png",
	statusBarTranslucent = "Galaxy-Note20-UltraStatusBarTransparent.png",
	statusBarBlack = "Galaxy-Note20-UltraStatusBarBlack.png",
	statusBarLightTransparent = "Galaxy-Note20-UltraStatusBarWhite.png",
	statusBarDarkTransparent = "Galaxy-Note20-UltraStatusBarBlack.png",
	screenDressing = "Galaxy-Note20-UltraScreenDressing.png",
	windowTitleBarName = "Galaxy Note20 Ultra",
}
simulator.defaultFontSize = 18.0 * (simulator.androidDisplayApproximateDpi / 160)
