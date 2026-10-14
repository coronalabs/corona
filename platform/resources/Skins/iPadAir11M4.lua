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
	screenWidth = 1640,
	screenHeight = 2360,
	safeScreenInsetStatusBar = 24 * 2,
	safeScreenInsetBottom = 20 * 2,
	safeLandscapeScreenInsetStatusBar = 24 * 2,
	safeLandscapeScreenInsetBottom = 20 * 2,
	iosPointWidth = 820,
	iosPointHeight = 1180,
	deviceImage = "iPadAir11M4.png",
	displayManufacturer = "Apple",
	displayName = "iPad",
	screenDressing = "iPadAir11M4ScreenDressing.png",
	-- Matches iOS: default and light modes draw white status bar text, dark modes draw black text.
	statusBarDefault = "iPadAir11M4StatusBarWhite.png",
	statusBarTranslucent = "iPadAir11M4StatusBarWhite.png",
	statusBarBlack = "iPadAir11M4StatusBarBlack.png",
	statusBarLightTransparent = "iPadAir11M4StatusBarWhite.png",
	statusBarDarkTransparent = "iPadAir11M4StatusBarBlack.png",
	windowTitleBarName = "iPad Air 11-inch (M4)",
	defaultFontSize = 17 * 2,		-- Converts default font point size to pixels.
}
