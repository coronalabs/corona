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
	device = "ios-tablet",
	screenOriginX = 128,
	screenOriginY = 128,
	screenWidth = 1488,
	screenHeight = 2266,
	safeScreenInsetStatusBar = 24 * 2,
	safeScreenInsetBottom = 20 * 2,
	safeLandscapeScreenInsetStatusBar = 24 * 2,
	safeLandscapeScreenInsetBottom = 20 * 2,
	iosPointWidth = 744,
	iosPointHeight = 1133,
	deviceImage = "iPadMiniA17Pro.png",
	displayManufacturer = "Apple",
	displayName = "iPad",
	screenDressing = "iPadMiniA17ProScreenDressing.png",
	-- Matches iOS: default and light modes draw white status bar text, dark modes draw black text.
	statusBarDefault = "iPadMiniA17ProStatusBarWhite.png",
	statusBarTranslucent = "iPadMiniA17ProStatusBarWhite.png",
	statusBarBlack = "iPadMiniA17ProStatusBarBlack.png",
	statusBarLightTransparent = "iPadMiniA17ProStatusBarWhite.png",
	statusBarDarkTransparent = "iPadMiniA17ProStatusBarBlack.png",
	windowTitleBarName = "iPad mini (A17 Pro)",
	defaultFontSize = 17 * 2,		-- Converts default font point size to pixels.
}
