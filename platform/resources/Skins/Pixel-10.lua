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
	screenOriginX = 72,
	screenOriginY = 72,
	screenWidth = 1080,
	screenHeight = 2424,
	safeScreenInsetTop = 173,
	safeScreenInsetLeft = 0,
	safeScreenInsetBottom = 63,
	safeScreenInsetRight = 0,
	safeLandscapeScreenInsetTop = 0,
	safeLandscapeScreenInsetLeft = 173,
	safeLandscapeScreenInsetBottom = 63,
	safeLandscapeScreenInsetRight = 0,
	androidDisplayApproximateDpi = 420, -- xxhdpi
	deviceImage = "Pixel-10.png",
	displayManufacturer = "Google",
	displayName = "Pixel 10",
	-- Android 15+ draws apps edge-to-edge under a transparent status bar; only
	-- display.DarkTransparentStatusBar switches the icons to dark.
	statusBarDefault = "Pixel-10StatusBarWhite.png",
	statusBarTranslucent = "Pixel-10StatusBarWhite.png",
	statusBarBlack = "Pixel-10StatusBarWhite.png",
	statusBarLightTransparent = "Pixel-10StatusBarWhite.png",
	statusBarDarkTransparent = "Pixel-10StatusBarBlack.png",
	screenDressing = "Pixel-10ScreenDressing.png",
	windowTitleBarName = "Pixel 10",
}
simulator.defaultFontSize = 18.0 * (simulator.androidDisplayApproximateDpi / 160)
