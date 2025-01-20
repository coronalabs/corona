------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

local params = ...

-- params:
-- * title
-- * message
-- * buttonOK
-- * buttonAction
-- * url
-- * (optional) shouldQuit

local function CallBack( params )

	local function listener( event )
		if "clicked" == event.action then
			local i = event.index
			if 1 == i then
				-- Do nothing
			else
				system.openURL( params.url )
			end
		end

		-- Force quit
		if ( params.shouldQuit ) then
			os.exit( -1 )
		end
	end

	native.showAlert( params.title, params.message, { params.buttonOK, params.buttonAction }, listener )

end

CallBack( params )