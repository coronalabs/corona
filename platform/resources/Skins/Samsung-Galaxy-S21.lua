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
	screenOriginX = 50,
<<<<<<< HEAD
	screenOriginY = 70,
=======
	screenOriginY = 65,
>>>>>>> ef49c70f0220a19716f524e23f1415119728a1f0
	screenWidth = 1080,
	screenHeight = 2400,
	safeScreenInsetTop = 30*3,
	safeScreenInsetLeft = 0 * 3,
	safeScreenInsetBottom = 30 * 3,
	safeScreenInsetRight = 0 * 3,
	safeLandscapeScreenInsetTop = 0 * 3,
	safeLandscapeScreenInsetLeft = 30 * 3,
	safeLandscapeScreenInsetBottom = 0 * 3,
	safeLandscapeScreenInsetRight = 30 * 3,
	androidDisplayApproximateDpi = 420, -- xxhdpi
	deviceImage = "SamsungGalaxyS21.png",
	displayManufacturer = "samsung",
	displayName = "SM-G991x",
	statusBarDefault = "SamsungGalaxyStatusBarGray.png",
	statusBarTranslucent = "SamsungGalaxyStatusBarGray.png",
	statusBarBlack = "SamsungGalaxyStatusBarBlack.png",
	statusBarLightTransparent = "SamsungGalaxyStatusBarWhite.png",
	statusBarDarkTransparent = "SamsungGalaxyStatusBarBlack.png",
	screenDressing = "SamsungGalaxy21ScreenDressing.png",
	windowTitleBarName = "Samsung Galaxy S21",
}
simulator.defaultFontSize = 18.0 * (simulator.androidDisplayApproximateDpi / 160)
