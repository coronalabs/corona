------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
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
	isTransparent = false,
	backend = "gl",

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
