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
	-- Folded: the outer display. Hardware > Unfold (Mac Simulator) switches to the inner display below.
	device = "ios-phone",
	screenOriginX = 96,
	screenOriginY = 84,
	screenWidth = 1398,
	screenHeight = 2034,
	-- Safe areas are what Corona reports on the device: UIKit's safe area, grown where it leaves a rounded
	-- corner uncovered (iOS 26 corner adaptation). Values in pixels: 7 is 2.33pt, 52 is 17.33pt.
	safeScreenInsetTop = 7,
	safeScreenInsetLeft = 7,
	safeScreenInsetBottom = 34 * 3,
	safeScreenInsetRight = 84 * 3,
	-- landscapeRight. Turned sideways, iOS hides the status bar and keeps the inset on the camera's side.
	safeLandscapeScreenInsetTop = 52,
	safeLandscapeScreenInsetLeft = 84 * 3,
	safeLandscapeScreenInsetBottom = 34 * 3,
	safeLandscapeScreenInsetRight = 52,
	-- landscapeLeft isn't landscapeRight mirrored: the big corners are at the bottom then.
	safeLandscapeLeftScreenInsetTop = 7,
	safeLandscapeLeftScreenInsetLeft = 7,
	safeLandscapeLeftScreenInsetBottom = 34 * 3,
	safeLandscapeLeftScreenInsetRight = 84 * 3,
	iosPointWidth = 466,
	iosPointHeight = 678,
	deviceImage = "iPhoneDuo.png",
	displayManufacturer = "Apple",
	displayName = "iPhone",
	screenDressing = "iPhoneDuoScreenDressing.png",
	-- The time, battery and signal sit in a column beside the camera; iOS reports a 2pt status bar.
	statusBarHeight = 2 * 3,
	-- Matches iOS: default and light modes draw white status bar text, dark modes draw black text.
	statusBarDefault = "iPhoneDuoStatusBarWhite.png",
	statusBarTranslucent = "iPhoneDuoStatusBarWhite.png",
	statusBarBlack = "iPhoneDuoStatusBarBlack.png",
	statusBarLightTransparent = "iPhoneDuoStatusBarWhite.png",
	statusBarDarkTransparent = "iPhoneDuoStatusBarBlack.png",
	windowTitleBarName = "iPhone Duo",
	defaultFontSize = 17 * 3,		-- Converts default font point size to pixels.
	unfolded =
	{
		screenOriginX = 84,
		screenOriginY = 84,
		screenWidth = 2853,
		screenHeight = 2007,
		-- The wide inner screen is a landscape device: the art above is its landscapeLeft, with the
		-- status column beside the camera on the right and the home indicator at the bottom. The
		-- Simulator turns it for the other orientations, so each table below is that art turned.
		-- (Unverified against hardware: iOS may not turn the inner screen at all.)
		-- portrait: turned a quarter, the camera column at the top
		safeScreenInsetTop = 84 * 3,
		safeScreenInsetLeft = 16 * 3,
		safeScreenInsetBottom = 16 * 3,
		safeScreenInsetRight = 34 * 3,
		-- landscapeRight: turned around, the camera column on the left
		safeLandscapeScreenInsetTop = 34 * 3,
		safeLandscapeScreenInsetLeft = 84 * 3,
		safeLandscapeScreenInsetBottom = 16 * 3,
		safeLandscapeScreenInsetRight = 16 * 3,
		-- landscapeLeft: the art as drawn
		safeLandscapeLeftScreenInsetTop = 16 * 3,
		safeLandscapeLeftScreenInsetLeft = 16 * 3,
		safeLandscapeLeftScreenInsetBottom = 34 * 3,
		safeLandscapeLeftScreenInsetRight = 84 * 3,
		iosPointWidth = 951,
		iosPointHeight = 669,
		deviceImage = "iPhoneDuoUnfolded.png",
		screenDressing = "iPhoneDuoUnfoldedScreenDressing.png",
		statusBarDefault = "iPhoneDuoUnfoldedStatusBarWhite.png",
		statusBarTranslucent = "iPhoneDuoUnfoldedStatusBarWhite.png",
		statusBarBlack = "iPhoneDuoUnfoldedStatusBarBlack.png",
		statusBarLightTransparent = "iPhoneDuoUnfoldedStatusBarWhite.png",
		statusBarDarkTransparent = "iPhoneDuoUnfoldedStatusBarBlack.png",
	},
}
