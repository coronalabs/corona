-- ui.lua (currently includes Button class with labels, font selection and optional event model)

-- Version 1.5 (works with multitouch, adds setText() method to buttons)
--
-- Copyright (C) 2010 Corona Labs Inc. All Rights Reserved.
--
-- Permission is hereby granted, free of charge, to any person obtaining a copy of 
-- this software and associated documentation files (the "Software"), to deal in the 
-- Software without restriction, including without limitation the rights to use, copy, 
-- modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, 
-- and to permit persons to whom the Software is furnished to do so, subject to the 
-- following conditions:
-- 
-- The above copyright notice and this permission notice shall be included in all copies 
-- or substantial portions of the Software.
-- 
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
-- INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
-- PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE 
-- FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR 
-- OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
-- DEALINGS IN THE SOFTWARE.

module(..., package.seeall)

-----------------
-- Helper function for newButton utility function below
local function newButtonHandler( self, event )

	local result = true

	local default = self[1]
	local over = self[2]
	
	-- General "onEvent" function overrides onPress and onRelease, if present
	local onEvent = self._onEvent
	
	local onPress = self._onPress
	local onRelease = self._onRelease

	local buttonEvent = {}
	if (self._id) then
		buttonEvent.id = self._id
	end

	local phase = event.phase
	if "began" == phase then
		if over then 
			default.isVisible = false
			over.isVisible = true
		end

		if onEvent then
			buttonEvent.phase = "press"
			result = onEvent( buttonEvent )
		elseif onPress then
			result = onPress( event )
		end

		-- Subsequent touch events will target button even if they are outside the contentBounds of button
		display.getCurrentStage():setFocus( self, event.id )
		self.isFocus = true
		
	elseif self.isFocus then
		local bounds = self.contentBounds
		local x,y = event.x,event.y
		local isWithinBounds = 
			bounds.xMin <= x and bounds.xMax >= x and bounds.yMin <= y and bounds.yMax >= y

		if "moved" == phase then
			if over then
				-- The rollover image should only be visible while the finger is within button's contentBounds
				default.isVisible = not isWithinBounds
				over.isVisible = isWithinBounds
			end
			
		elseif "ended" == phase or "cancelled" == phase then 
			if over then 
				default.isVisible = true
				over.isVisible = false
			end
			
			if "ended" == phase then
				-- Only consider this a "click" if the user lifts their finger inside button's contentBounds
				if isWithinBounds then
					if onEvent then
						buttonEvent.phase = "release"
						result = onEvent( buttonEvent )
					elseif onRelease then
						result = onRelease( event )
					end
				end
			end
			
			-- Allow touch events to be sent normally to the objects they "hit"
			display.getCurrentStage():setFocus( self, nil )
			self.isFocus = false
		end
	end

	return result
end


---------------
-- Button class

function newButton( params )
	local button, default, over, size, font, textColor, offset
	
	if params.default then
		button = display.newGroup()
		default = display.newImage( params.default )
		button:insert( default, true )
	end
	
	if params.over then
		over = display.newImage( params.over )
		over.isVisible = false
		button:insert( over, true )
	end
	
	-- Public methods
	function button:setText( newText )
	
		local labelText = self.text
		if ( labelText ) then
			labelText:removeSelf()
			self.text = nil
		end

		local labelShadow = self.shadow
		if ( labelShadow ) then
			labelShadow:removeSelf()
			self.shadow = nil
		end

		local labelHighlight = self.highlight
		if ( labelHighlight ) then
			labelHighlight:removeSelf()
			self.highlight = nil
		end
		
		if ( params.size and type(params.size) == "number" ) then size=params.size else size=20 end
		if ( params.font ) then font=params.font else font=native.systemFontBold end
		if ( params.textColor ) then textColor=params.textColor else textColor={ 255, 255, 255, 255 } end
		
		-- Optional vertical correction for fonts with unusual baselines (I'm looking at you, Zapfino)
		if ( params.offset and type(params.offset) == "number" ) then offset=params.offset else offset = 0 end
		
		if ( params.emboss ) then
			-- Make the label text look "embossed" (also adjusts effect for textColor brightness)
			local textBrightness = ( textColor[1] + textColor[2] + textColor[3] ) / 3
			
			labelHighlight = display.newText( newText, 0, 0, font, size )
			if ( textBrightness > 127) then
				labelHighlight:setTextColor( 255, 255, 255, 20 )
			else
				labelHighlight:setTextColor( 255, 255, 255, 140 )
			end
			button:insert( labelHighlight, true )
			labelHighlight.x = labelHighlight.x + 1.5; labelHighlight.y = labelHighlight.y + 1.5 + offset
			self.highlight = labelHighlight

			labelShadow = display.newText( newText, 0, 0, font, size )
			if ( textBrightness > 127) then
				labelShadow:setTextColor( 0, 0, 0, 128 )
			else
				labelShadow:setTextColor( 0, 0, 0, 20 )
			end
			button:insert( labelShadow, true )
			labelShadow.x = labelShadow.x - 1; labelShadow.y = labelShadow.y - 1 + offset
			self.shadow = labelShadow
		end
		
		labelText = display.newText( newText, 0, 0, font, size )
		labelText:setTextColor( textColor[1], textColor[2], textColor[3], textColor[4] )
		button:insert( labelText, true )
		labelText.y = labelText.y + offset
		self.text = labelText
	end
	
	if params.text then
		button:setText( params.text )
	end
	
	if ( params.onPress and ( type(params.onPress) == "function" ) ) then
		button._onPress = params.onPress
	end
	if ( params.onRelease and ( type(params.onRelease) == "function" ) ) then
		button._onRelease = params.onRelease
	end
	
	if (params.onEvent and ( type(params.onEvent) == "function" ) ) then
		button._onEvent = params.onEvent
	end
		
	-- Set button as a table listener by setting a table method and adding the button as its own table listener for "touch" events
	button.touch = newButtonHandler
	button:addEventListener( "touch", button )

	if params.x then
		button.x = params.x
	end
	
	if params.y then
		button.y = params.y
	end
	
	if params.id then
		button._id = params.id
	end

	return button
end


--------------
-- Label class

function newLabel( params )
	local labelText
	local size, font, textColor, align
	local t = display.newGroup()
	
	if ( params.bounds ) then
		local bounds = params.bounds
		local left = bounds[1]
		local top = bounds[2]
		local width = bounds[3]
		local height = bounds[4]
	
		if ( params.size and type(params.size) == "number" ) then size=params.size else size=20 end
		if ( params.font ) then font=params.font else font=native.systemFontBold end
		if ( params.textColor ) then textColor=params.textColor else textColor={ 255, 255, 255, 255 } end
		if ( params.offset and type(params.offset) == "number" ) then offset=params.offset else offset = 0 end
		if ( params.align ) then align = params.align else align = "center" end
		
		if ( params.text ) then
			labelText = display.newText( params.text, 0, 0, font, size )
			labelText:setTextColor( textColor[1], textColor[2], textColor[3], textColor[4] )
			t:insert( labelText )
			-- TODO: handle no-initial-text case by creating a field with an empty string?
	
			if ( align == "left" ) then
				labelText.x = left + labelText.contentWidth * 0.5
			elseif ( align == "right" ) then
				labelText.x = (left + width) - labelText.contentWidth * 0.5
			else
				labelText.x = ((2 * left) + width) * 0.5
			end
		end
		
		labelText.y = top + labelText.contentHeight * 0.5

		-- Public methods
		function t:setText( newText )
			if ( newText ) then
				labelText.text = newText
				
				if ( "left" == align ) then
					labelText.x = left + labelText.contentWidth / 2
				elseif ( "right" == align ) then
					labelText.x = (left + width) - labelText.contentWidth / 2
				else
					labelText.x = ((2 * left) + width) / 2
				end
			end
		end
		
		function t:setTextColor( r, g, b, a )
			local newR = 255
			local newG = 255
			local newB = 255
			local newA = 255

			if ( r and type(r) == "number" ) then newR = r end
			if ( g and type(g) == "number" ) then newG = g end
			if ( b and type(b) == "number" ) then newB = b end
			if ( a and type(a) == "number" ) then newA = a end

			labelText:setTextColor( r, g, b, a )
		end
	end
	
	-- Return instance (as display group)
	return t
	
end