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
	screenOriginX = 100,
	screenOriginY = 100,
	screenWidth = 2064,
	screenHeight = 2752,
	safeScreenInsetStatusBar = 24 * 2,
	safeScreenInsetBottom = 20 * 2,
	safeLandscapeScreenInsetStatusBar = 24 * 2,
	safeLandscapeScreenInsetBottom = 20 * 2,
	iosPointWidth = 1032,
	iosPointHeight = 1376,
	deviceImage = "iPadPro13M5.png",
	displayManufacturer = "Apple",
	displayName = "iPad",
	screenDressing = "iPadPro13M5ScreenDressing.png",
	-- Matches iOS: default and light modes draw white status bar text, dark modes draw black text.
	statusBarDefault = "iPadPro13M5StatusBarWhite.png",
	statusBarTranslucent = "iPadPro13M5StatusBarWhite.png",
	statusBarBlack = "iPadPro13M5StatusBarBlack.png",
	statusBarLightTransparent = "iPadPro13M5StatusBarWhite.png",
	statusBarDarkTransparent = "iPadPro13M5StatusBarBlack.png",
	windowTitleBarName = "iPad Pro 13-inch (M5)",
	defaultFontSize = 17 * 2,		-- Converts default font point size to pixels.
}
