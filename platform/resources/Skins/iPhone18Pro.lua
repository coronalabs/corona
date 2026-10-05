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
	screenWidth = 1206,
	screenHeight = 2622,
	safeScreenInsetTop = 62 * 3,
	safeScreenInsetLeft = 0 * 3,
	safeScreenInsetBottom = 34 * 3,
	safeScreenInsetRight = 0 * 3,
	safeLandscapeScreenInsetTop = 0 * 3,
	safeLandscapeScreenInsetLeft = 62 * 3,
	safeLandscapeScreenInsetBottom = 20 * 3,
	safeLandscapeScreenInsetRight = 62 * 3,
	iosPointWidth = 402,
	iosPointHeight = 874,
	deviceImage = "iPhone18Pro.png",
	displayManufacturer = "Apple",
	displayName = "iPhone",
	screenDressing = "iPhone18ProScreenDressing.png",
	-- Matches iOS: default and light modes draw white status bar text, dark modes draw black text.
	statusBarDefault = "iPhone18ProStatusBarWhite.png",
	statusBarTranslucent = "iPhone18ProStatusBarWhite.png",
	statusBarBlack = "iPhone18ProStatusBarBlack.png",
	statusBarLightTransparent = "iPhone18ProStatusBarWhite.png",
	statusBarDarkTransparent = "iPhone18ProStatusBarBlack.png",
	windowTitleBarName = "iPhone 18 Pro",
	defaultFontSize = 17 * 3,		-- Converts default font point size to pixels.
}
