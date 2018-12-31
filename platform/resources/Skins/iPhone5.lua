------------------------------------------------------------------------------
--
-- Copyright (C) 2018 Corona Labs Inc.
-- Contact: support@coronalabs.com
--
-- This file is part of the Corona game engine.
--
-- Commercial License Usage
-- Licensees holding valid commercial Corona licenses may use this file in
-- accordance with the commercial license agreement between you and 
-- Corona Labs Inc. For licensing terms and conditions please contact
-- support@coronalabs.com or visit https://coronalabs.com/com-license
--
-- GNU General Public License Usage
-- Alternatively, this file may be used under the terms of the GNU General
-- Public license version 3. The license is as published by the Free Software
-- Foundation and appearing in the file LICENSE.GPL3 included in the packaging
-- of this file. Please review the following information to ensure the GNU 
-- General Public License requirements will
-- be met: https://www.gnu.org/licenses/gpl-3.0.html
--
-- For overview and more information on licensing please refer to README.md
--
------------------------------------------------------------------------------

simulator =
{
	device = "ios-phone",
	screenOriginX = 64,
	screenOriginY = 228,
	screenWidth = 640,
	screenHeight = 1136,
	iosPointWidth = 320,
	iosPointHeight = 568,
	safeLandscapeScreenInsetStatusBar = 20 * 2,
	safeScreenInsetStatusBar = 20 * 2,
	deviceImage = "iPhone5.png",
	displayManufacturer = "Apple",
	displayName = "iPhone",
	statusBarDefault = "iPhone5StatusBarBlack.png",
	statusBarTranslucent = "iPhone5StatusBarWhite.png",
	statusBarBlack = "iPhone5StatusBarBlack.png",
	statusBarLightTransparent = "iPhone5StatusBarWhite.png",
	statusBarDarkTransparent = "iPhone5StatusBarBlack.png",
	windowTitleBarName = "iPhone 5",
	defaultFontSize = 17 * 2,		-- Converts default font point size to pixels.
}
