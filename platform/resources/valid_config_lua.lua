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

-- fake out some things commonly found in config.lua
system = {}
system.getInfo = function(...) return ""; end
display = {}
display.pixelHeight = 480
display.pixelWidth = 320

-- These are items we recognize but cannot validate the contents of (generally because they're arbitrary)
stoplist = {"imageSuffix", "shaderPrecision"}

application = {
	launchPad = true,
	showRuntimeErrors = true,

	content = {
		graphicsCompatibility = 1,
		width = 1,
		height = 1,
		xAlign = "",
		yAlign = "",
		scale = "letterBox",
		fps = 60,
		antialias = false,
		audioPlayFrequency = 1,
		imageSuffix = {
		},
		shaderPrecision = "",
	},

	license = {
		google = {
			key = "",
			policy = "serverManaged",
			mapsKey = "",
		}
	},

	notification = {
		iphone = {
			types = {
				"badge", "sound", "alert"
			}
		},

        google =
        {
            projectNumber = "",
        },
	},

	steamworks =
	{
		appId = "",
	},
}
