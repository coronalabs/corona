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
	screenOriginX = 128,
	screenOriginY = 128,
	screenWidth = 1600,
	screenHeight = 2560,
	safeScreenInsetStatusBar = 72,
	safeScreenInsetBottom = 48,
	safeLandscapeScreenInsetStatusBar = 72,
	safeLandscapeScreenInsetBottom = 48,
	androidDisplayApproximateDpi = 320, -- xhdpi
	deviceImage = "Pixel-Tablet.png",
	displayManufacturer = "Google",
	displayName = "Pixel Tablet",
	-- Android 15+ draws apps edge-to-edge under a transparent status bar; only
	-- display.DarkTransparentStatusBar switches the icons to dark.
	statusBarDefault = "Pixel-TabletStatusBarWhite.png",
	statusBarTranslucent = "Pixel-TabletStatusBarWhite.png",
	statusBarBlack = "Pixel-TabletStatusBarWhite.png",
	statusBarLightTransparent = "Pixel-TabletStatusBarWhite.png",
	statusBarDarkTransparent = "Pixel-TabletStatusBarBlack.png",
	screenDressing = "Pixel-TabletScreenDressing.png",
	windowTitleBarName = "Pixel Tablet",
}
simulator.defaultFontSize = 18.0 * (simulator.androidDisplayApproximateDpi / 160)
