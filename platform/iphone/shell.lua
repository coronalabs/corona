------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------


local params = ...

local overlay = params.overlay
local onShellComplete = params.onShellComplete
local onShellCompleteCalled = false

-- Arrange to call onShellComplete() only once
local function callOnShellComplete(param)
	if not onShellCompleteCalled then
		onShellCompleteCalled = true
		onShellComplete(param)
	end
end

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

-- Tell the runtime that shell.lua has completed
callOnShellComplete(nil)
