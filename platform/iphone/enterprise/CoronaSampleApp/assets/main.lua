-- 
-- Insert your Lua code here
--

local mylibrary = require "mylibrary"

if mylibrary.canSendTweet() then
	mylibrary.sendTweet( "My custom library rocks!" )
else
	native.showAlert(
		"Cannot send tweet.",
		"Please setup your Twitter account or check your network connection",
		{ "OK" } )
end

-- CUSTOM EVENTS
-- Register listener for custom 'delegate' event.
-- This event is dispatched by `didLoadMain:` in MyCoronaDelegate.mm
local function delegateListener( event )
	native.showAlert(
		"Event dispatched from `didLoadMain:`",
		"of type: " .. tostring( event.name ),
		{ "OK" } )
end
Runtime:addEventListener( "delegate", delegateListener )
