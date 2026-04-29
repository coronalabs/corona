--------------------------------------------------------------------------------
-- Sample code is MIT licensed, see http://www.coronalabs.com/links/code/license
-- Copyright (C) 2012 Corona Labs Inc. All Rights Reserved.
--------------------------------------------------------------------------------

-- Hide the status bar.
display.setStatusBar(display.HiddenStatusBar)

-- Display a white background.
display.setDefault("background", 255, 255, 255)

-- Create stars with random colors and positions.
local stars = {}
for i = 1, 20 do
	local myStar = display.newLine( 0,-110, 27,-35 ) 
	myStar:append( 105,-35, 43,16, 65,90, 0,45, -65,90, -43,15, -105,-35, -27,-35, 0,-110 )

	myStar:setColor( math.random(255), math.random(255), math.random(255), math.random(200) + 55 )
	myStar.width = math.random(10)
	
	myStar.x = math.random( display.contentWidth )
	myStar.y = math.random( display.contentHeight )
	myStar.rotation = math.random(360)
	
	myStar.xScale = math.random(150)/100 + 0.5
	myStar.yScale = myStar.xScale
	
	myStar:setReferencePoint( display.CenterReferencePoint )

	local dr = math.random( 1, 4 )
	myStar.dr = dr
	if ( math.random() < 0.5 ) then
		myStar.dr = -dr
	end

	table.insert( stars, myStar )
end

-- Animate the stars using "enterFrame" events.
function onEnterFrame( event )
	for i,v in ipairs( stars ) do
		v.rotation = v.rotation + v.dr
	end
end
Runtime:addEventListener( "enterFrame", onEnterFrame )
