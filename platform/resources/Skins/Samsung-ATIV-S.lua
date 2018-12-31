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
	device = "winphone-phone",
	screenOriginX = 58,
	screenOriginY = 152,
	screenWidth = 720,
	screenHeight = 1280,
	deviceImage = "Samsung-ATIV-S.png",
	displayManufacturer = "Not Supported Yet", -- Unsupported on WP8, return the stub implementation
	displayName = "WindowsPhone",
	supportsScreenRotation = true,
	hasAccelerometer = true,
	windowTitleBarName = "Samsung ATIV S",
	defaultFontSize = 25.333 * 1.5,			-- Apply a 720p scale factor.
}
