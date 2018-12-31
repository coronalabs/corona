-- 
-- Abstract: Hello World sample app.
--
-- Version: 1.2
-- 
-- Sample code is MIT licensed
-- Copyright (C) 2014 Corona Labs Inc. All Rights Reserved.
--
-- Supports Graphics 2.0
------------------------------------------------------------

local background = display.newImage( "world.jpg", display.contentCenterX, display.contentCenterY )

local myText = display.newText( "Hello, World!", display.contentCenterX, display.contentWidth / 4, native.systemFont, 40 )
myText:setFillColor( 1.0, 0.4, 0.4 )
