--------------------------------------------------------------------------------
-- Sample code is MIT licensed, see http://www.coronalabs.com/links/code/license
-- Copyright (C) 2012 Corona Labs Inc. All Rights Reserved.
--------------------------------------------------------------------------------

-- Import the UI Lua library.
local ui = require("ui")

-- Hide the status bar.
display.setStatusBar(display.HiddenStatusBar)

-- Display a background.
local background = display.newRect(0, 0, display.contentWidth, display.contentHeight)
background:setFillColor(255, 165, 0)


-- Set up a button to cause a Lua error to be handled by the Java LuaErrorHandlerFunction class.
function onLuaErrorButtonTapped()
	callNonExistingFunction()
end
luaErrorButton = ui.newButton
{
	default = "buttonBlue.png",
	over = "buttonBlueOver.png",
	onPress = onLuaErrorButtonTapped,
	text = "Trigger Lua Error",
	emboss = true
}
luaErrorButton.x = display.contentWidth / 2
luaErrorButton.y = (display.contentHeight / 2) + (luaErrorButton.contentHeight / 2)


-- Set up a button to cause an unhandled Java exception.
function onExceptionButtonTapped()
	myTests.throwException()
end
exceptionButton = ui.newButton
{
	default = "buttonBlue.png",
	over = "buttonBlueOver.png",
	onPress = onExceptionButtonTapped,
	text = "Trigger Java Exception",
	emboss = true
}
exceptionButton.x = display.contentWidth / 2
exceptionButton.y = luaErrorButton.y + luaErrorButton.contentHeight + (exceptionButton.contentHeight / 2)
