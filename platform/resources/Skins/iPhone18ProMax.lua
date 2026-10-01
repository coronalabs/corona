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
	device = "ios-phone",
	screenOriginX = 84,
	screenOriginY = 84,
	screenWidth = 1320,
	screenHeight = 2868,
	safeScreenInsetTop = 62 * 3,
	safeScreenInsetLeft = 0 * 3,
	safeScreenInsetBottom = 34 * 3,
	safeScreenInsetRight = 0 * 3,
	safeLandscapeScreenInsetTop = 0 * 3,
	safeLandscapeScreenInsetLeft = 62 * 3,
	safeLandscapeScreenInsetBottom = 20 * 3,
	safeLandscapeScreenInsetRight = 62 * 3,
	iosPointWidth = 440,
	iosPointHeight = 956,
	deviceImage = "iPhone18ProMax.png",
	displayManufacturer = "Apple",
	displayName = "iPhone",
	screenDressing = "iPhone18ProMaxScreenDressing.png",
	-- Matches iOS: default and light modes draw white status bar text, dark modes draw black text.
	statusBarDefault = "iPhone18ProMaxStatusBarWhite.png",
	statusBarTranslucent = "iPhone18ProMaxStatusBarWhite.png",
	statusBarBlack = "iPhone18ProMaxStatusBarBlack.png",
	statusBarLightTransparent = "iPhone18ProMaxStatusBarWhite.png",
	statusBarDarkTransparent = "iPhone18ProMaxStatusBarBlack.png",
	windowTitleBarName = "iPhone 18 Pro Max",
	defaultFontSize = 17 * 3,		-- Converts default font point size to pixels.
}
