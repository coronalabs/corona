---------------------------------------------------------------------------------------
-- A simple physics example in 8 lines!
---------------------------------------------------------------------------------------

local physics = require( "physics" )
physics.start()

local sky = display.newImage( "sky.png", 160, 195 )
local ground = display.newImage( "ground.png", 160, 445 )

local crate = display.newImage( "crate.png", 180, -50 )
crate.rotation = 5

physics.addBody( ground, "static", { friction=0.5, bounce=0.3 } )
physics.addBody( crate, { density=3.0, friction=0.5, bounce=0.3 } )