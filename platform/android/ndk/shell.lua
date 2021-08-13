------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

local params = ...


-- Variables that cross the boundaries between different components handled by shell.lua
local onShellComplete = params.onShellComplete
local handleCheckingForExpansionFiles
local onShellCompleteCalled = false

-- Arrange to call onShellComplete() only once
local function callOnShellComplete(param)
	if not onShellCompleteCalled then
		onShellCompleteCalled = true
		onShellComplete(param)
	end
end

---------------------------------------------------------------------------------------------------
-- This part is used for Corona Cards to show the trial overlay
---------------------------------------------------------------------------------------------------
local overlay = params.overlay

if params.showMessage then

	local halfW = display.contentWidth * 0.5
	local halfH = display.contentHeight * 0.5

	local fontSize = 96 * display.contentScaleX
	local lineSpace = fontSize * 0.6
	local options = 
	{
		parent = overlay,
		text = "CoronaCards",
		x = halfW,
		y = halfH-lineSpace,
		font = native.systemFontBold,   
		fontSize = fontSize,
	}

	local msg = display.newText( options )
	msg:setFillColor( 0, .1 )

	local msg = display.newText( options )
	msg:setFillColor( 1, .4 )

	local options =
	{
		parent = overlay,
		text = "TRIAL",
		x = halfW,
		y = halfH+lineSpace,
		font = native.systemFontBold,   
		fontSize = fontSize * 1.5,
	}

	local msg = display.newText( options )
	msg:setFillColor( 0, .3 )

	local msg = display.newText( options )
	msg:setFillColor( 1, .2 )

end

---------------------------------------------------------------------------------------------------
-- This part is used to check if Google Play Services is available
---------------------------------------------------------------------------------------------------

-- Determines if a module is available for the given module name.
-- This is done by partially "require"-ing the module in, returning false if it couldn't be required in.
-- From: http://stackoverflow.com/questions/15429236/how-to-check-if-a-module-exists-in-lua/15434737#15434737
local function isModuleAvailable(name)
	-- If this module is in the loaded table, it's already available.
	if package.loaded[name] then
		return true
	else
		-- "package.searchers" is the Lua 5.2 rename of "package.loaders"
		-- Per the documentation: https://www.lua.org/manual/5.1/manual.html#pdf-package.loaders
		-- Each entry in the package.searchers or package.loaders table is a searcher function
		for _, searcher in ipairs(package.searchers or package.loaders) do
			-- Run this searcher function, hoping to find a loader function for this module.
			local loader = searcher(name)
			if type(loader) == 'function' then
				-- We've found a loader function, which can now be added to our preload table for easier access.
				package.preload[name] = loader
				-- A Lua loader exists for this module, so it must be available to use.
				return true
			end
		end

		-- No Lua loader function was found. The module does not exist.
		return false
	end
end

local usingGooglePlayServices = isModuleAvailable("shared.google.play.services.base")
if usingGooglePlayServices then

	local gps = require("shared.google.play.services.base")

	local function googlePlayServicesAvailabileListener( event )
		gps.clearAvailabilityListener()

		-- Now handle checking for expansion files
		handleCheckingForExpansionFiles()
	end

	-- Set a listener to fire when it does become available.
	gps.setAvailabilityListener(googlePlayServicesAvailabileListener)

	-- Checks if Google Play Services is available and attempts to make it available if it's not.
	gps.handleGooglePlayServicesAvailability()

end

---------------------------------------------------------------------------------------------------
-- This part is used to check for expansion files
---------------------------------------------------------------------------------------------------

handleCheckingForExpansionFiles = function()
	callOnShellComplete(nil)
end


if not usingGooglePlayServices then
	-- Move on to Expansion files if needed.
	handleCheckingForExpansionFiles()
end

-- following line is commented out because callOnShellComplete should be
-- invoked when expansion file is done expanding
-- callOnShellComplete(nil)

