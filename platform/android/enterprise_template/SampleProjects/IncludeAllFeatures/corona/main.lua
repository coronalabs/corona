--------------------------------------------------------------------------------
-- Sample code is MIT licensed, see http://www.coronalabs.com/links/code/license
-- Copyright (C) 2012 Corona Labs Inc. All Rights Reserved.
--------------------------------------------------------------------------------


-- Import optional Lua libraries.
local ui = require("ui")
local ads = require("ads")
local analytics = require("analytics")
local facebook = require("facebook")


-- Hide the status bar.
display.setStatusBar(display.HiddenStatusBar)


-- Display an orange background.
display.setDefault("background", 255, 165, 0)


-- Show an inneractive ad banner at the top of the screen.
ads.init("inneractive", "MyAppId")
ads.show("banner", { interval=60 } )


-- Log an event to Flurry analytics.
-- *** You must provide a valid app ID in order to init() the Flurry analytics feature. ***
analytics.init("MyAppId")
analytics.logEvent("My Event ID")


-- Set up a button for logging into facebook.
-- *** You must provide a valid app ID in order for the login to work. ***
function onFacebookResponse(event)
	print("A facebook response has been received.")
end
function onLogOnFacebookButtonTapped()
	facebook.login("MyAppId", onFacebookResponse,  {"publish_stream"}  )
end
logOnFacebookButton = ui.newButton
{
	default = "buttonBlue.png",
	over = "buttonBlueOver.png",
	onPress = onLogOnFacebookButtonTapped,
	text = "Log On Facebook",
	emboss = true
}
logOnFacebookButton.x = display.contentWidth / 2
logOnFacebookButton.y = (display.contentHeight / 2) + (logOnFacebookButton.contentHeight / 2)
