------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

-- Google Pixel 10 Pro Fold. The top-level table is the folded phone (outer 6.4" screen); the
-- "unfolded" table replaces fields for the open 8" inner screen. Hardware > Unfold swaps them at
-- runtime and the app receives "fold" and "resize" events.
-- Outer screen 1080x2364 (408 ppi) with a centred punch-hole camera; inner screen 2076x2152 (373 ppi)
-- with the camera top-right; gesture navigation.
-- Art: Google's Pixel 9 Pro Fold device frames and display masks from Android Studio's
-- device-art-resources (Apache 2.0); the two generations share the inner screen and body, and the
-- closed frame is scaled to the 10 Pro Fold's shorter outer screen. Cutouts and the 66dp status bar
-- come from the AOSP pixel_9_pro_fold framework overlay: outer camera circle at (540,86) r41.5 with a
-- 112x152 reserved rect, inner camera at (1987.5,80) r39.5 with a 136x136 rect top-right.
simulator =
{
	device = "android-phone",
	screenOriginX = 94,
	screenOriginY = 66,
	screenWidth = 1080,
	screenHeight = 2364,
	-- Portrait: the camera's reserved rect (152px) at the top, 24dp gesture bar at the bottom
	safeScreenInsetTop = 152,
	safeScreenInsetLeft = 0,
	safeScreenInsetBottom = 63,
	safeScreenInsetRight = 0,
	-- Landscape: the cutout edge becomes a side, the gesture bar follows the Pixel 4a skin's convention
	safeLandscapeScreenInsetTop = 0,
	safeLandscapeScreenInsetLeft = 152,
	safeLandscapeScreenInsetBottom = 0,
	safeLandscapeScreenInsetRight = 63,
	androidDisplayApproximateDpi = 420, -- xxhdpi
	deviceImage = "Pixel-10-Pro-Fold.png",
	displayManufacturer = "google",
	displayName = "Pixel 10 Pro Fold",
	statusBarDefault = "Pixel-10-Pro-FoldStatusBarTransparent.png",
	statusBarTranslucent = "Pixel-10-Pro-FoldStatusBarTransparent.png",
	statusBarBlack = "Pixel-10-Pro-FoldStatusBarBlack.png",
	statusBarLightTransparent = "Pixel-10-Pro-FoldStatusBarWhite.png",
	statusBarDarkTransparent = "Pixel-10-Pro-FoldStatusBarBlack.png",
	screenDressing = "Pixel-10-Pro-FoldScreenDressing.png",
	windowTitleBarName = "Pixel 10 Pro Fold",
	unfolded =
	{
		screenOriginX = 62,
		screenOriginY = 62,
		screenWidth = 2076,
		screenHeight = 2152,
		-- The inner screen is slightly taller than wide, so it keeps the usual portrait rules.
		-- The camera's reserved rect is 136px tall at the top right; 24dp gesture bar at the bottom.
		safeScreenInsetTop = 136,
		safeScreenInsetLeft = 0,
		safeScreenInsetBottom = 58,
		safeScreenInsetRight = 0,
		safeLandscapeScreenInsetTop = 0,
		safeLandscapeScreenInsetLeft = 136,
		safeLandscapeScreenInsetBottom = 0,
		safeLandscapeScreenInsetRight = 58,
		androidDisplayApproximateDpi = 390, -- inner screen is 373 ppi; the emulator uses 390
		deviceImage = "Pixel-10-Pro-FoldUnfolded.png",
		statusBarDefault = "Pixel-10-Pro-FoldUnfoldedStatusBarTransparent.png",
		statusBarTranslucent = "Pixel-10-Pro-FoldUnfoldedStatusBarTransparent.png",
		statusBarBlack = "Pixel-10-Pro-FoldUnfoldedStatusBarBlack.png",
		statusBarLightTransparent = "Pixel-10-Pro-FoldUnfoldedStatusBarWhite.png",
		statusBarDarkTransparent = "Pixel-10-Pro-FoldUnfoldedStatusBarBlack.png",
		screenDressing = "Pixel-10-Pro-FoldUnfoldedScreenDressing.png",
	},
}
simulator.defaultFontSize = 18.0 * (simulator.androidDisplayApproximateDpi / 160)
