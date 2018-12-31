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
	device = "borderless-android-device",
	screenOriginX = 0,
	screenOriginY = 0,
	screenWidth = 400,
	screenHeight = 854,
	androidDisplayApproximateDpi = 240, -- hdpi
	displayManufacturer = "Corona Labs Inc.",
	displayName = "GenericAndroidDevice",
	windowTitleBarName = "Android (hdpi)",
}
simulator.defaultFontSize = 18.0 * (simulator.androidDisplayApproximateDpi / 160)
