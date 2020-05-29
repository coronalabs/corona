------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

local modname = ...
local widget = {}
package.loaded[modname] = widget
widget.version = "0.7"

-- cached locals
local mAbs = math.abs
local mFloor = math.floor

-- defaults
local scrollFriction = 0.935
local pickerFriction = 0.88

-- modify factory function to ensure widgets are properly cleaned on group removal
local cached_displayNewGroup = display.newGroup
function display.newGroup()
	local g = cached_displayNewGroup()
	
	-- function to find/remove widgets within group
	local function removeWidgets( group )
		if group.numChildren then
			for i=group.numChildren,1,-1 do
				if group[i]._isWidget then
					group[i]:removeSelf()
				
				elseif not group[i]._isWidget and group[i].numChildren then
					-- nested group (that is not a widget)
					removeWidgets( group[i] )
				end
			end
		end
	end
	
	-- store reference to original removeSelf method
	local cached_removeSelf = g.removeSelf
	
	-- subclass removeSelf method
	function g:removeSelf()
		removeWidgets( self )	-- remove widgets first
		
		-- continue removing group as usual
		if self.parent and self.parent.remove then
			self.parent:remove( self )
		end
	end
	return g
end

-- set current theme from external .lua module
function widget.setTheme( themeModule )
	widget.theme = require( themeModule )	-- should return table w/ theme data
end

-- add 'setText()' method to display.newText (to be consistent with display.newEmbossed text)
local cached_newText = display.newText
function display.newText( ... )
	local text = cached_newText( ... )

	function text:setText( newString )
		self.text = newString
	end

	return text
end

-- creates very sharp text for high resolution/high density displays
function widget.retinaText( ... )
	text = display.newText( ... );
	return text
end; display.newRetinaText = display.newText --widget.retinaText

-- creates sharp (retina) text with an embossed/inset effect
function widget.embossedText( ... )
	local arg = { ... }
	
	-- parse arguments
	local parentG, w, h
	local argOffset = 0
	
	-- determine if a parentGroup was specified
	if arg[1] and type(arg[1]) == "table" then
		parentG = arg[1]; argOffset = 1
	end
	
	local string = arg[1+argOffset] or ""
	local x = arg[2+argOffset] or 0
	local y = arg[3+argOffset] or 0
	local w, h = 0, 0
	
	local newOffset = 3+argOffset
	if type(arg[4+argOffset]) == "number" then w = arg[4+argOffset]; newOffset=newOffset+1; end
	if w and #arg >= 7+argOffset then h = arg[5+argOffset]; newOffset=newOffset+1; end
	
	local font = arg[1+newOffset] or native.systemFont
	local size = arg[2+newOffset] or 12
	local color = { 0, 0, 0, 255 }
	
	---------------------------------------------
	
	local r, g, b, a = color[1], color[2], color[3], color[4]
	local textBrightness = ( r + g + b ) / 3
	
	local highlight = display.newText( string, 0.5, 1, w, h, font, size )
	if ( textBrightness > 127) then
		highlight:setTextColor( 255, 255, 255, 20 )
	else
		highlight:setTextColor( 255, 255, 255, 140 )
	end
	
	local shadow = display.newText( string, -0.5, -1, w, h, font, size )
	if ( textBrightness > 127) then
		shadow:setTextColor( 0, 0, 0, 128 )
	else
		shadow:setTextColor( 0, 0, 0, 20 )
	end
	
	local label = display.newText( string, 0, 0, w, h, font, size )
	label:setTextColor( r, g, b, a )
	
	-- create display group, insert all embossed text elements, and position it
	local text = display.newGroup()
	text:insert( highlight ); text.highlight = highlight
	text:insert( shadow ); text.shadow = shadow
	text:insert( label ); text.label = label
	text.x, text.y = x, y
	text:setReferencePoint( display.CenterReferencePoint )
	
	-- setTextColor method
	function text:setTextColor( ... )
		local r, g, b, a; local arg = { ... }
		
		if #arg == 4 then
			r = arg[1]; g = arg[2]; b = arg[3]; a = arg[4]
		elseif #arg == 3 then
			r = arg[1]; g = arg[2]; b = arg[3]; a = 255
		elseif #arg == 2 then
			r = arg[1]; g = r; b = r; a = arg[2]
		elseif #arg == 1 then
			if type(arg[1]) == "number" then
				r = arg[1]; g = r; b = r; a = 255
			end
		end
		
		local textBrightness = ( r + g + b ) / 3
		if ( textBrightness > 127) then
			self.highlight:setTextColor( 255, 255, 255, 20 )
			self.shadow:setTextColor( 0, 0, 0, 128 )
		else
			self.highlight:setTextColor( 255, 255, 255, 140 )
			self.shadow:setTextColor( 0, 0, 0, 20 )
		end
		self.label:setTextColor( r, g, b, a )
	end
	
	-- setText method
	function text:setText( newString )
		local newString = newString or self.text
		self.highlight.text = newString
		self.shadow.text = newString
		self.label.text = newString
		self.text = newString
	end
	
	-- setSize method
	function text:setSize ( newSize )
		local newSize = newSize or size
		self.highlight.size = newSize
		self.shadow.size = newSize
		self.label.size = newSize
		self.size = newSize
	end
	
	if parentG then parentG:insert( text ) end
	text.text = string
	return text
end; display.newEmbossedText = widget.embossedText

-----------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------
--
-- button widget
--
-----------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------

function widget.newButton( options )
	
	local function onButtonTouch( self, event )	-- self == button
		local result = true
		local phase = event.phase
		event.name = "buttonEvent"
		event.target = self

		if phase == "began" then
			display.getCurrentStage():setFocus( self, event.id )
			self.isFocus = true

			event.phase = "press"
			if self.onEvent then
				result = self.onEvent( event )
			elseif self.onPress then
				result = self.onPress( event )
			end

			self.default.isVisible = false
			self.over.isVisible = true
			local r, g, b, a = self.label.color.over[1] or 0, self.label.color.over[2] or self.label.color.over[1], self.label.color.over[3] or self.label.color.over[1], self.label.color.over[4] or 255
			self.label:setTextColor( r, g, b, a )

		elseif self.isFocus then
			local bounds = self.contentBounds
			local x, y = event.x, event.y
			local isWithinBounds = bounds.xMin <= x and bounds.xMax >= x and bounds.yMin <= y and bounds.yMax >= y
			
			if phase == "moved" then
				if not isWithinBounds then
					self.default.isVisible = true
					self.over.isVisible = false

					local r, g, b, a = self.label.color.default[1] or 0, self.label.color.default[2] or self.label.color.default[1], self.label.color.default[3] or self.label.color.default[1], self.label.color.default[4] or 255
					self.label:setTextColor( r, g, b, a )
				else
					self.default.isVisible = false
					self.over.isVisible = true

					local r, g, b, a = self.label.color.over[1] or 0, self.label.color.over[2] or self.label.color.over[1], self.label.color.over[3] or self.label.color.over[1], self.label.color.over[4] or 255
					self.label:setTextColor( r, g, b, a )
				end

				if self.onEvent then
					result = self.onEvent( event )
				elseif self.onDrag then
					result = self.onDrag( event )
				end

			elseif phase == "ended" or phase == "cancelled" then
				if self.default and self.over then
					self.default.isVisible = true
					self.over.isVisible = false
					local r, g, b, a = self.label.color.default[1] or 0, self.label.color.default[2] or self.label.color.default[1], self.label.color.default[3] or self.label.color.default[1], self.label.color.default[4] or 255
					self.label:setTextColor( r, g, b, a )
				end
				
				-- trigger appropriate event listener if released within bounds of button
				if isWithinBounds then
					event.phase = "release"
					if self.onEvent then
						result = self.onEvent( event )
					elseif self.onRelease then
						result = self.onRelease( event )
					end
				end

				-- remove focus from button
				display.getCurrentStage():setFocus( self, nil )
				self.isFocus = false
			end
		end

		return result
	end
	
	local function setLabel( self, newLabel )	-- self == button
		if not newLabel then return; end
		
		if self.label.setText then
			self.label:setText( newLabel )
		else
			self.label.text = newLabel
		end
		
		-- re-center label on button
		self.label:setReferencePoint( display.CenterReferencePoint )
		self.label.x = (self.contentWidth*0.5) + self.label.xOffset
		self.label.y = (self.contentHeight*0.5) + self.label.yOffset
	end
	
	local function getLabel( self )
		return self.label.text
	end
	
	local function removeSelf( self )	-- self == button
		-- check to see if there is a clean method; if so, call it
		if self.clean then self:clean(); end
		
		-- remove all children of default and over
		if self.default and self.default.numChildren then
			for i=self.default.numChildren,1,-1 do
				display.remove( self.default.numChildren[i] )
			end
			display.remove( self.default )
		end
		
		if self.over and self.over.numChildren then
			for i=self.over.numChildren,1,-1 do
				display.remove( self.over.numChildren[i] )
			end
			display.remove( self.over )
		end
		
		-- remove label
		display.remove( self.label )
		
		-- remove button group
		self:cached_removeSelf()
	end
	
	local function createButton( options, theme )
		local	defaultBtnWidth = 124
		local	defaultBtnHeight = 42
		
		local 	options = options or {}
		local 	theme = theme or {}
		local	id = options.id or "widget_button"
		local	left = options.left or 0
		local	top = options.top or 0
		local	xOffset = options.xOffset or theme.xOffset or 0		-- offsets x value of the label text
		local	yOffset = options.yOffset or options.offset or theme.yOffset or theme.offset or 0		-- offsets y value of the label text
		local	label = options.label or ""
		local 	font = options.font or theme.font or native.systemFont
		local 	fontSize = options.fontSize or theme.fontSize or 14
		local	emboss = options.emboss or theme.emboss
		local	textFunction = display.newText; if emboss then textFunction = widget.embossedText; end
		local 	labelColor = options.labelColor or theme.labelColor or { default={ 0 }, over={ 255 } }
		local   onPress = options.onPress
		local 	onRelease = options.onRelease
		local 	onDrag = options.onDrag
		local 	onEvent = options.onEvent
		local 	default = options.default or theme.default
		local 	defaultColor = options.defaultColor or theme.defaultColor
		local	over = options.over or theme.over
		local 	overColor = options.overColor or theme.overColor
		local 	strokeColor = options.strokeColor or theme.strokeColor
		local 	strokeWidth = options.strokeWidth or theme.strokeWidth
		local 	cornerRadius = options.cornerRadius or theme.cornerRadius
		local 	width = options.width or theme.width
		local 	height = options.height or theme.height
		local 	sheet = options.sheet or theme.sheet
		local 	defaultIndex = options.defaultIndex or theme.defaultIndex
		local 	overIndex = options.overIndex or theme.overIndex
		local 	baseDir = options.baseDir or theme.baseDir or system.ResourceDirectory
		
		local button = display.newGroup()
		
		if default or sheet then

			-- user-provided image for default and over state
			if sheet and defaultIndex and width and height then
				-- image sheet option
				button.default = display.newImageRect( button, sheet, defaultIndex, width, height )
				button.default:setReferencePoint( display.TopLeftReferencePoint )
				button.default.x, button.default.y = 0, 0

				local over = overIndex or defaultIndex
				button.over = display.newImageRect( button, sheet, over, width, height )
				button.over:setReferencePoint( display.TopLeftReferencePoint )
				button.over.x, button.over.y = 0, 0

			elseif width and height then
				-- display.newImageRect() option (non)
				button.default = display.newImageRect( button, default, baseDir, width, height )
				button.default:setReferencePoint( display.TopLeftReferencePoint )
				button.default.x, button.default.y = 0, 0

				local over = over or default
				button.over = display.newImageRect( button, over, baseDir, width, height )
				button.over:setReferencePoint( display.TopLeftReferencePoint )
				button.over.x, button.over.y = 0, 0
			else
				-- display.newImage() option
				button.default = display.newImage( button, default, baseDir, true )
				button.default:setReferencePoint( display.TopLeftReferencePoint )
				button.default.x, button.default.y = 0, 0

				local over = over or default
				button.over = display.newImage( button, over, baseDir, true )
				button.over:setReferencePoint( display.TopLeftReferencePoint )
				button.over.x, button.over.y = 0, 0
				
				width, height = button.default.contentWidth, button.default.contentHeight
			end

			if defaultColor then
				if defaultColor[1] then
					button.default:setFillColor( defaultColor[1], defaultColor[2] or defaultColor[1], defaultColor[3] or defaultColor[1], defaultColor[4] or 255 )
				end
			end

			if overColor then
				if overColor[1] then
					button.over:setFillColor( overColor[1], overColor[2] or overColor[1], overColor[3] or overColor[1], overColor[4] or 255 )
				end
			end
		else
			-- no images; construct button using newRoundedRect
			if not width then width = defaultBtnWidth; end
			if not height then height = defaultBtnHeight; end
			if not cornerRadius then cornerRadius = 8; end

			button.default = display.newRoundedRect( button, 0, 0, width, height, cornerRadius )
			button.over = display.newRoundedRect( button, 0, 0, width, height, cornerRadius )

			if defaultColor and defaultColor[1] then
				button.default:setFillColor( defaultColor[1], defaultColor[2] or defaultColor[1], defaultColor[3] or defaultColor[1], defaultColor[4] or 255 )
			else
				button.default:setFillColor( 255 )
			end

			if overColor and overColor[1] then
				button.over:setFillColor( overColor[1], overColor[2] or overColor[1], overColor[3] or overColor[1], overColor[4] or 255 )
			else
				button.over:setFillColor( 128 )
			end

			if strokeColor and strokeColor[1] then
				button.default:setStrokeColor( strokeColor[1], strokeColor[2] or strokeColor[1], strokeColor[3] or strokeColor[1], strokeColor[4] or 255 )
				button.over:setStrokeColor( strokeColor[1], strokeColor[2] or strokeColor[1], strokeColor[3] or strokeColor[1], strokeColor[4] or 255 )
			else
				button.default:setStrokeColor( 0 )
				button.over:setStrokeColor( 0 )
			end

			if not strokeWidth then
				button.default.strokeWidth = 1
				button.over.strokeWidth = 1
			else
				button.default.strokeWidth = strokeWidth
				button.over.strokeWidth = strokeWidth
			end
		end
		button.over.isVisible = false	-- hide "down/over" state of button
		
		-- create the label
		if not labelColor then labelColor = {}; end
		if not labelColor.default then labelColor.default = { 0 }; end
		if not labelColor.over then labelColor.over = { 255 }; end
		local r, g, b, a = labelColor.default[1] or 0, labelColor.default[2] or labelColor.default[1], labelColor.default[3] or labelColor.default[1], labelColor.default[4] or 255

		button.label = textFunction( button, label, 0, 0, font, fontSize )
		button.label:setTextColor( r, g, b, a )
		button.label:setReferencePoint( display.CenterReferencePoint )
		button.label.x = (button.contentWidth * 0.5) + xOffset
		button.label.y = (button.contentHeight * 0.5) + yOffset
		button.label.color = labelColor
		button.label.xOffset = xOffset
		button.label.yOffset = yOffset
		
		-- set properties and methods
		button._isWidget = true
		button.id = id
		button.onPress = onPress
		button.onDrag = onDrag
		button.onRelease = onRelease
		button.onEvent = onEvent
		button.touch = onButtonTouch; button:addEventListener( "touch", button )
		button.cached_removeSelf = button.removeSelf
		button.removeSelf = removeSelf
		button.setLabel = setLabel
		button.getLabel = getLabel
		
		-- position the button
		button:setReferencePoint( display.TopLeftReferencePoint )
		button.x, button.y = left, top
		button:setReferencePoint( display.CenterReferencePoint )
		
		return button
	end
	
	-- this widget supports visual customization via themes
	local themeOptions
	if widget.theme then
		local buttonTheme = widget.theme.button
		
		if buttonTheme then
			if options and options.style then	-- style parameter optionally set by user
				
				-- for themes that support various "styles" per widget
				local style = buttonTheme[options.style]
				
				if style then themeOptions = style; end
			else
				-- if no style parameter set, use default style specified by theme
				themeOptions = buttonTheme
			end
		end
	end
	
	return createButton( options, themeOptions )
end

-----------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------
--
-- slider widget
--
-----------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------

function widget.newSlider( options )
	
	-- set slider value from 0 to 100
	local function setSliderValue( self, value )	-- self == slider
		-- make sure value is not less than 0 or greater than 100
		if value < 0 then
			value = 0
		elseif value > 100 then
			value = 100
		else
			value = mFloor( value )	-- round to the nearest whole number
		end
		
		local width = self.max - self.min
		
		-- calculate percentage based on slidable width
		local percent = value / 100
		
		-- move handle to new position
		local x = (width * percent) + self.min
		self.handle.x = x
		
		-- stretch fill image from left side to handle
		local fillScaleX = (self.handle.x - self.min) / self.fillWidth
		if fillScaleX <= 0 then fillScaleX = 0.1; end
		self.fill.xScale = fillScaleX
		
		-- update reference to value
		self.value = value
	end
	
	-- dispatch slider event
	local function dispatchSliderEvent( self )	-- self == slider
		if self.listener then
			local e = {}
			e.name = "sliderEvent"
			e.type = "sliderMoved"
			e.target = self
			e.value = self.value
			
			self.listener( e )
		end
	end
	
	-- slider touch event
	local function onSliderTouch( self, event )	-- self == slider
		if event.phase == "began" then
			display.getCurrentStage():setFocus( self )
			self.isFocus = true
			self:setReferencePoint( display.CenterReferencePoint )
			
			local sliderX = (self.contentBounds.xMin + (self.contentWidth*0.5))
			local x = event.x - sliderX
			local width = self.max - self.min
			local percent = mFloor(((( (width*0.5) + x) * 100) / width))
			self:setValue( percent )
			
			dispatchSliderEvent( self )
			
		elseif self.isFocus then
			local isWithinBounds = self.min <= event.x and self.max >= event.x
			
			if event.phase == "moved" then
				
				local sliderX = (self.contentBounds.xMin + (self.contentWidth*0.5))
				local x = event.x - sliderX
				local width = self.max - self.min
				local percent = mFloor(((( (width*0.5) + x) * 100) / width))
				self:setValue( percent )
				
				dispatchSliderEvent( self )
			
			elseif event.phase == "ended" or event.phase == "cancelled" then
				
				display.getCurrentStage():setFocus( nil )
				self.isFocus = nil
			end
		end
		
		return true
	end
	
	-- removeSelf() method for slider widget
	local function removeSelf( self )
		if self.clean and type(self.clean) == "function" then self:clean(); end
		
		if self.fill then self.fill:removeSelf(); self.fill = nil; end
		if self.handle then self.handle:removeSelf(); self.handle = nil; end
		self.fillWidth = nil
		self.value = nil
		
		self:cached_removeSelf()
	end
	
	local function createSlider( options, theme )
		local options = options or {}
		local theme = theme or {}
		local id = options.id or "widget_slider"
		
		local left = options.left or 0
		local top = options.top or 0
		local width = options.width or theme.width or 200
		local height = options.height or theme.height or 10
		local background = options.background or theme.background
		local handleImage = options.handle or theme.handle
		local handleWidth = options.handleWidth or theme.handleWidth
		local handleHeight = options.handleHeight or theme.handleHeight
		local leftImage = options.leftImage or theme.leftImage
		local leftWidth = options.leftWidth or theme.leftWidth or 16
		local fillImage = options.fillImage or theme.fillImage
		local fillWidth = options.fillWidth or theme.fillWidth or 2
		local cornerRadius = options.cornerRadius or theme.cornerRadius or 5
		local value = options.value or 50
		local listener = options.listener or options.callback
		local baseDir = options.baseDir or theme.baseDir or system.ResourceDirectory
		
		local fillColor = options.fillColor or theme.fillColor or {}
				fillColor[1] = fillColor[1] or 0
				fillColor[2] = fillColor[2] or 100
				fillColor[3] = fillColor[3] or 230
				fillColor[4] = fillColor[4] or 255
		
		local handleColor = options.handleColor or theme.handleColor or {}
				handleColor[1] = handleColor[1] or 189
				handleColor[2] = handleColor[2] or 189
				handleColor[3] = handleColor[3] or 189
				handleColor[4] = handleColor[4] or 255
		
		local handleStroke = options.handleStroke or theme.handleStroke or {}
				handleStroke[1] = handleStroke[1] or 143
				handleStroke[2] = handleStroke[2] or 143
				handleStroke[3] = handleStroke[3] or 143
				handleStroke[4] = handleStroke[4] or 255
		
		local bgFill = options.bgFill or theme.bgFill or {}
				bgFill[1] = bgFill[1] or 225
				bgFill[2] = bgFill[2] or 225
				bgFill[3] = bgFill[3] or 225
				bgFill[4] = bgFill[4] or 255
		
		local bgStroke = options.bgStroke or theme.bgStroke or {}
				bgStroke[1] = bgStroke[1] or 102
				bgStroke[2] = bgStroke[2] or 102
				bgStroke[3] = bgStroke[3] or 102
				bgStroke[4] = bgStroke[4] or 255
		
		-- construct slider widget based on provided parameters (or defaults)
		local slider = display.newGroup()
		local bg, leftSide, fill, handle
		
		if not background and not fillImage then		
			bg = display.newRoundedRect( slider, 0, 0, width, height, cornerRadius )
			bg.strokeWidth = 1
			bg:setStrokeColor( bgStroke[1], bgStroke[2], bgStroke[3], bgStroke[4] )
			bg:setFillColor( bgFill[1], bgFill[2], bgFill[3], bgFill[4] )
			
			leftSide = display.newRoundedRect( slider, 0, 0, leftWidth, height, cornerRadius )
			leftSide:setReferencePoint( display.CenterReferencePoint )
			leftSide:setFillColor( fillColor[1], fillColor[2], fillColor[3], fillColor[4] )
			
			fill = display.newRect( slider, leftWidth*0.5, 0, fillWidth, height )
			fill:setReferencePoint( display.CenterLeftReferencePoint )
			fill:setFillColor( fillColor[1], fillColor[2], fillColor[3], fillColor[4] )
		
		elseif background and fillImage then
			bg = display.newImageRect( slider, background, baseDir, width, height )
			bg:setReferencePoint( display.TopLeftReferencePoint )
			bg.x, bg.y = 0, 0
			
			fill = display.newImageRect( slider, fillImage, baseDir, fillWidth, height )
			fill:setReferencePoint( display.CenterLeftReferencePoint )
			fill.x, fill.y = leftWidth, height * 0.5
		else
			if background and not fillImage then
				print( "WARNING: You must also specify a fillImage when using a custom background with the slider widget." )
				return
			elseif fillImage and not background then
				print( "WARNING: You must specify a custom background when using a custom fillImage with the slider widget." )
				return
			end
		end
		
		slider.fill = fill
		slider.fillWidth = fillWidth
		
		if not handleImage or not handleWidth or not handleHeight then
			handle = display.newCircle( slider, width*0.5, height*0.5, height )
			handle:setReferencePoint( display.CenterReferencePoint )
			handle:setFillColor( handleColor[1], handleColor[2], handleColor[3], handleColor[4] )
			handle.strokeWidth = 1
			handle:setStrokeColor( handleStroke[1], handleStroke[2], handleStroke[3], handleStroke[4] )
		else
			handle = display.newImageRect( slider, handleImage, handleWidth, handleHeight )
			handle:setReferencePoint( display.CenterReferencePoint )
			handle.x, handle.y = width*0.5, height*0.5
		end
		slider.handle = handle
		
		-- properties and methods
		slider._isWidget = true
		slider.id = id
		slider.min = leftWidth*0.5
		slider.max = width - (leftWidth * 0.5)
		slider.setValue = setSliderValue
		slider.touch = onSliderTouch
		slider:addEventListener( "touch", slider )
		slider.listener = listener
		
		slider.cached_removeSelf = slider.removeSelf
		slider.removeSelf = removeSelf
		
		local fillScaleX = (handle.x - slider.min) / fillWidth
		fill.xScale = fillScaleX
		
		-- position the widget and set reference point to center
		slider.x, slider.y = left, top
		slider:setReferencePoint( display.CenterReferencePoint )
		
		-- set initial value
		slider:setValue( value )
		
		return slider
	end
	
	-- this widget supports visual customization via themes
	local themeOptions
	if widget.theme then
		local sliderTheme = widget.theme.slider
		
		if sliderTheme then
			if options and options.style then	-- style parameter optionally set by user
				
				-- for themes that support various "styles" per widget
				local style = sliderTheme[options.style]
				
				if style then themeOptions = style; end
			else
				-- if no style parameter set, use default style specified by theme
				themeOptions = sliderTheme
			end
		end
	end
	
	return createSlider( options, themeOptions )
end

-----------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------
--
-- pickerWheel widget
--
-----------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------

--Function to handle the soft-landing of the picker wheel
local function pickerSoftLand( self )
	local target = self.parent
	
	--Variables that equal the ones used in picker.getValues
	local height = self.height
	local selectionHeight = self.selectionHeight
	local top = self.parent.parent.parent.y --Get the actual pickers groups y position to use as the top position
	local selectionTop = target.topPadding
	
	--Index to scroll to							
	local index = nil
	
	--Get row using same system at picker.getValues uses
	if target:getRowAtCoordinate( top + selectionTop + ( selectionHeight * 0.5 ) ) ~= nil then
		index = target:getRowAtCoordinate( top + selectionTop + ( selectionHeight * 0.5 ) ).index
	end
	
	--If there is an index, scroll to it to give the impression of soft landing
	if index ~= nil then
		target:scrollToIndex( index, 400 )
	end
end

function widget.newPickerWheel( options )
	-- get selection values of pickerWheel columns (returns table)
	local function getValues( self )	-- self == pickerWheel
		local columnValues = {}
		local columns = self.columns
		local top = self.y
		local selectionTop = self.selectionTop or 255
		local selectionHeight = self.selectionHeight or 46
		
		--print( selectionTop)
						
		for i=1,columns.numChildren do
			local col = columns[i]
			local realSelectionY = top + selectionTop + (selectionHeight*0.5)
			local row = col:getRowAtCoordinate( realSelectionY )
						
			if row and row.value and row.index then
				columnValues[i] = {}
				columnValues[i].value = row.value
				columnValues[i].index = row.index
			end
		end
		
		return columnValues
	end
	
	
	-- creates new pickerWheel column
	local function newPickerColumn( pickerWheel, parentGroup, columnData, params )
		local column = widget.newTableView( params )
		
		-- create individual 'rows' for the column
		for i=1,#columnData do
			local labelX = 14
			local ref = display.CenterLeftReferencePoint
			
			if columnData.alignment and columnData.alignment ~= "left" then
				if columnData.alignment == "center" then
					labelX = params.width * 0.5
					ref = display.CenterReferencePoint
				elseif columnData.alignment == "right" then
					labelX = params.width - 14
					ref = display.CenterRightReferencePoint
				end
			end
			
			local function renderRow( event )
				local row = event.row
				local view = event.view
				local fc = params.fontColor
				
				local label = display.newText( columnData[i], 0, 0, params.font, params.fontSize )
				label:setTextColor( fc[1], fc[2], fc[3], fc[4] )
				label:setReferencePoint( ref )
				label.x = labelX
				label.y = row.height * 0.5
				
				row.value = columnData[i]
				view:insert( label )
			end
			
			
			column:insertRow{
				onRender = renderRow,
				width = params.width,
				height = params.rowHeight or 32,
				rowColor = params.bgColor or { 255, 255, 255, 255 },
				lineColor = params.bgColor or { 255, 255, 255, 255 },
				skipRender = true,
			}
		end
		
		parentGroup:insert( column )
		
		return column
	end
	
	-- subclassed removeSelf method for pickerWheel
	local function removeSelf( self )	-- self == pickerWheel
		-- check to see if there is a clean method; if so, call it
		if self.clean then self:clean(); end
		
		-- remove mask if it exists
		if self.mask then
			self.columns:setMask( nil )
			self.mask = nil
		end
		
		-- remove each column one by one
		for i=self.columns.numChildren,1,-1 do
			self.columns[i]:removeSelf()
		end
		self.columns = nil
				
		-- remove pickerWheel widget
		self:cached_removeSelf()
	end
	
	local function createPickerWheel( options, themeOptions )
		local options = options or {}
		local theme = themeOptions or {}

		-- parse parameters (options) or set defaults (or theme defaults)
		local id = options.id or "widget_pickerWheel"
		local left = options.left or 0
		local top = options.top or 0
		local width = options.width or theme.width or 296
		local height = options.height or theme.height or 222
		local bgWidth = options.bgWidth or options.totalWidth or theme.bgWidth or theme.totalWidth or display.contentWidth
		local selectionTop = options.selectionTop or theme.selectionTop or 90
		local selectionHeight = options.selectionHeight or theme.selectionHeight or 46
		local font = options.font or theme.font or system.nativeFontBold
		local fontSize = options.fontSize or theme.fontSize or 22
		local fontColor = options.fontColor or theme.fontColor or {}
			fontColor[1] = fontColor[1] or 0
			fontColor[2] = fontColor[2] or fontColor[1]
			fontColor[3] = fontColor[3] or fontColor[1]
			fontColor[4] = fontColor[4] or 255
		local columnColor = options.columnColor or theme.columnColor or {}
			columnColor[1] = columnColor[1] or 255
			columnColor[2] = columnColor[2] or columnColor[1]
			columnColor[3] = columnColor[3] or columnColor[1]
			columnColor[4] = columnColor[4] or 255
		local columns = options.columns or { { "One", "Two", "Three", "Four", "Five" } }
		local maskFile = options.maskFile or theme.maskFile
		local bgImage = options.bgImage or options.background or theme.bgImage or theme.background
		local bgImageWidth = options.bgImageWidth or options.backgroundWidth or theme.bgImageWidth or theme.backgroundWidth
		local bgImageHeight = options.bgImageHeight or options.backgroundHeight or theme.bgImageHeight or theme.backgroundHeight or height
		local overlayImage = options.overlayImage or options.glassFile or theme.overlayImage or theme.glassFile
		local overlayWidth = options.overlayWidth or options.glassWidth or theme.overlayWidth or theme.glassWidth
		local overlayHeight = options.overlayHeight or options.glassHeight or theme.overlayHeight or theme.glassHeight
		local separator = options.separator or theme.separator
		local separatorWidth = options.separatorWidth or theme.separatorWidth
		local separatorHeight = options.separatorHeight or theme.separatorHeight
		local baseDir = options.baseDir or theme.baseDir or system.ResourceDirectory
		
		local pickerWheel = display.newGroup()
		local columnGroup = display.newGroup()	-- will hold all column groups (tableViews)
		
		-- create background image
		if bgImage then
			local bg = display.newImageRect( pickerWheel, bgImage, baseDir, bgImageWidth, bgImageHeight )
			bg:setReferencePoint( display.TopLeftReferencePoint )
			bg.x, bg.y = 0, 0
			bg.xScale = bgWidth / bg.contentWidth
			
			local function disableTouchLeak() return true; end
			bg.touch = disableTouchLeak
			bg:addEventListener( "touch", bg )
		end
		
		-- insert the columns group into the pickerWheel widget group
		pickerWheel:insert( columnGroup )
		columnGroup.x = (bgWidth * 0.5) - width * 0.5
		columnGroup.y = 0
		
		local currentX = 0	-- variable that used for x-location of each column
		
		-- create all columns
		for i=1,#columns do
			local col = columns[i]
			
			-- set up tableView options (each column is a tableView widget)
			local params = {}
			-- tableView specific parameters
			params.id = "pickerColumn_" .. i
			params.renderThresh = (height - selectionTop) + selectionHeight
			params.left = 0
			params.top = 0
			params.topPadding = selectionTop
			params.bottomPadding = height - (selectionTop+selectionHeight)
			params.width = col.width or width/#columns
			params.height = height
			params.bgColor = columnColor
			params.friction = pickerFriction
			params.keepRowsPastTopVisible = true
			params.hideScrollBar = true
			
			--Used for controlling the pickers softlanding
			params.selectionHeight = selectionHeight
			params.isPicker = true
			params.pickerTop = top
			
			-- if last column, ensure width fills remaining space
			if i == #columns then params.width = width - currentX; end
			
			-- picker-specific parameters
			params.rowHeight = selectionHeight
			params.font = font
			params.fontSize = fontSize
			params.fontColor = fontColor
			
			-- create line separator that goes between the rows
			local separatorLine
			if separator and i ~= #columns then
				separatorLine = display.newImageRect( pickerWheel, separator, baseDir, separatorWidth, separatorHeight )
				separatorLine:setReferencePoint( display.TopLeftReferencePoint )
				separatorLine.x = (currentX + params.width) + columnGroup.x
				separatorLine.y = 0
				separatorLine.yScale = height / separatorLine.height
			end
			
			-- create the column
			local pickerColumn = newPickerColumn( pickerWheel, columnGroup, col, params )
			pickerColumn.x = currentX
			if #col <= 2 then pickerColumn.content.shortList = true; end
			
			currentX = currentX + params.width
			
			-- scroll to startIndex if specified
			if col.startIndex and col.startIndex > 1 then
				pickerColumn:scrollToIndex( col.startIndex )
			else
				pickerColumn:scrollToIndex( 1 )
			end
		end
		
		-- apply mask to columnGroup
		if maskFile then
			pickerWheel.mask = graphics.newMask( maskFile )
			columnGroup:setMask( pickerWheel.mask )
			columnGroup.maskX = columnGroup.width * 0.5
			columnGroup.maskY = height * 0.5
			columnGroup.isHitTestMasked = false
		end
		
		-- create overlay to go above columns
		if overlayImage then
			local overlay
			if overlayWidth and overlayHeight then
				overlay = display.newImageRect( pickerWheel, overlayImage, overlayWidth, overlayHeight )
			else
				overlay = display.newImage( pickerWheel, overlayImage, true )
			end
			overlay:setReferencePoint( display.CenterReferencePoint )
			overlay.x = bgWidth * 0.5
			overlay.y = height * 0.5
		end
		
		-- properties and methods
		pickerWheel._isWidget = true
		pickerWheel._isPicker = true
		pickerWheel.id = id
		pickerWheel.columns = columnGroup
		pickerWheel.getValues = getValues
		pickerWheel.selectionTop = selectionTop
		pickerWheel.cached_removeSelf = pickerWheel.removeSelf
		pickerWheel.removeSelf = removeSelf
		
		-- position the widget
		pickerWheel.x, pickerWheel.y = left, top
		
		return pickerWheel
	end
	
	-- this widget requires visual customization via themes to work properly
	local themeOptions
	if widget.theme then
		local pickerTheme = widget.theme.pickerWheel
		
		if pickerTheme then
			if options and options.style then	-- style parameter optionally set by user
				
				-- for themes that support various "styles" per widget
				local style = pickerTheme[options.style]
				
				if style then themeOptions = style; end
			else
				-- if no style parameter set, use default style specified by theme
				themeOptions = pickerTheme
			end
			
			return createPickerWheel( options, themeOptions )
		else
			print( "WARNING: The widget theme you are using does not support the pickerWheel widget." )
			return
		end
	else
		print( "WARNING: The pickerWheel widget requires a visual theme. Use widget.setTheme()." )
		return
	end
end

-----------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------
--
-- scrollView widget
--
-----------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------

function widget.newScrollView( options )
	local function dispatchBeganScroll( self, parent_widget ) 	-- self == content
		local e = {}
		e.name = "scrollEvent"
		e.type = "beganScroll"
		e.target = parent_widget or self.parent
		self.hasScrolled = false --Used to set whether the scrollview has actually being scrolled or just pressed
		if self.listener then self.listener( e ); end
	end
	
	local function dispatchEndedScroll( self )	-- self == content
		local e = {}
		e.name = "scrollEvent"
		e.type = "endedScroll"
		e.target = self.parent
		if self.listener then self.listener( e ); end
		--If the scrollbar isn't hidden
		if self.hideScrollBar == false then
			self.parent:hide_scrollbar()
		end
		
		
	end
	
	local function dispatchPickerSoftland( self )
		local e = {}
		e.target = self.parent
		
		--Make picker wheel softland
		if e.target._isPicker then
			pickerSoftLand( self )
		end
	end
	
	local function limitScrollViewMovement( self, upperLimit, lowerLimit )	-- self == content
		local function endedScroll()
			self.tween = nil
			if self.listener then
				--Dispatch the picker soft land
				dispatchPickerSoftland( self )
				
				--If the scrollview has scrolled then dispatch the ended scroll event ( this will trigger when the content has stopped moving )
				if self.hasScrolled == true then
					dispatchEndedScroll( self )
					self.hasScrolled = false
				end
			else
				--If the scrollbar isn't hidden
				if self.hideScrollBar == false then
					self.parent:hide_scrollbar()
				end
			end
		end
		
		local tweenContent = function( limit )
			if self.tween then transition.cancel( self.tween ); end
			if not self.isFocus then  -- if content is not being touched by user
				self.tween = transition.to( self, { time=400, y=limit, transition=easing.outQuad, onComplete=endedScroll } )
			end
			
			--If the scrollbar isn't hidden			
			if self.hideScrollBar == false then
				Runtime:addEventListener( "enterFrame", self.scrollbar_listener )
			end
		end
		local moveProperty = "y"
		local e = { name="scrollEvent", target=self.parent }
		local eventMin = "movingToTopLimit"
		local eventMax = "movingToBottomLimit"
		
		if self.moveDirection == "horizontal" then
			tweenContent = function( limit )
				if self.tween then transition.cancel( self.tween ); end
				self.tween = transition.to( self, { time=400, x=limit, transition=easing.outQuad, onComplete=endedScroll } )
			end
			moveProperty = "x"
			eventMin = "movingToLeftLimit"
			eventMax = "movingToRightLimit"
		end
		
		if self[moveProperty] > upperLimit then
	
			-- Content has drifted above upper limit of scrollView
			-- Stop content movement and transition back down to upperLimit
	
			self.velocity = 0
			Runtime:removeEventListener( "enterFrame", self )
			tweenContent( upperLimit )
			
			-- dispatch scroll event
			if self.listener then
				e.type = eventMin
				self.listener( e )
			end
	
		elseif self[moveProperty] < lowerLimit and lowerLimit < 0 then
	
			-- Content has drifted below lower limit (in case lower limit is above screen bounds)
			-- Stop content movement and transition back up to lowerLimit
	
			self.velocity = 0
			Runtime:removeEventListener( "enterFrame", self )
			tweenContent( lowerLimit )
			
			-- dispatch scroll event
			if self.listener then
				e.type = eventMax
				self.listener( e )
			end
			
		elseif self[moveProperty] < lowerLimit then
			
			-- Top of content has went past lower limit (in positive-y direction)
			-- Stop content movement and transition content back to upperLimit
			
			self.velocity = 0
			Runtime:removeEventListener( "enterFrame", self )
			if not self.shortList then
				tweenContent( upperLimit )
			else
				tweenContent( lowerLimit )
			end
			
			-- dispatch scroll event
			if self.listener then
				e.type = eventMin
				self.listener( e )
			end
		end
	end
	
	local function onScrollViewUpdate( self, event )	-- self == content
		if not self.trackVelocity then
			local time = event.time
			local timePassed = time - self.lastTime
			self.lastTime = time
	
			-- stop scrolling when velocity gets close to zero
			if mAbs( self.velocity ) < .01 then
				self.velocity = 0
				
				if self.moveDirection ~= "horizontal" then
					self.y = mFloor( self.y )
				
					-- if pulled past upper/lower boundaries, tween content properly
					limitScrollViewMovement( self, self.upperLimit, self.lowerLimit )
				else
					self.x = mFloor( self.x )
					
					-- if pulled past left/right boundaries, tween content properly
					limitScrollViewMovement( self, self.leftLimit, self.rightLimit )
				end
				
				self.moveDirection = nil
				Runtime:removeEventListener( "enterFrame", self )
				
				if self.listener then
					--Dispatch the pickers soft land
					dispatchPickerSoftland( self )
					
					-- dispatch an "endedScroll" event.type to user-specified listener
					--If the scrollview has scrolled then dispatch the ended scroll event ( this will trigger when the content has stopped moving )
					if self.hasScrolled == true then
						dispatchEndedScroll( self )
						self.hasScrolled = false
					end
				end

				-- self.tween is a transition that occurs when content is above or below lower limits
				-- and calls hide_scrollbar(), so the method does not need to be called here if self.tween exists
				if not self.tween then 
					--If the scrollbar isn't hidden
					if self.hideScrollBar == false then
						self.parent:hide_scrollbar(); 
					end
				end
			else
				-- update velocity and content location on every framestep
				local moveProperty = "y"
				if self.moveDirection == "horizontal" then moveProperty = "x"; end
				self.velocity = self.velocity * self.friction
				self[moveProperty] = self[moveProperty] + (self.velocity * timePassed)
				
				if moveProperty ~= "x" then
					limitScrollViewMovement( self, self.upperLimit, self.lowerLimit )
				else
					limitScrollViewMovement( self, self.leftLimit, self.rightLimit )
				end
			end
		else
			-- for timing how long user has finger held down
			if self.moveDirection == "vertical" then		
				if self.prevY == self.y then
					if self.eventStep > 5 then
						-- if finger is held down for 5 frames, ensure velocity is reset to 0
						self.prevY = self.y
						self.velocity = 0
						self.eventStep = 0
					else
						self.eventStep = self.eventStep + 1
					end
				end
			elseif self.moveDirection == "horizontal" then
				if self.prevX == self.x then
					if self.eventStep > 5 then
						-- if finger is held down for 5 frames, ensure velocity is reset to 0
						self.prevX = self.x
						self.velocity = 0
						self.eventStep = 0
					else
						self.eventStep = self.eventStep + 1
					end
				end
			end
		end

		--If the scrollbar isn't hidden
		if self.hideScrollBar == false then
			self.parent:update_scrollbar()
		end
	end
	
	local function onContentTouch( self, event )	-- self == content
		local scrollView = self.parent
		local phase = event.phase
		local time = event.time
		local hasScrolled = nil
		
		if phase == "began" then
			
			-- set focus on scrollView content
			display.getCurrentStage():setFocus( self )
			self.isFocus = true
			
			-- remove listener for auto-movement based on velocity
			Runtime:removeEventListener( "enterFrame", self )

			-- TODO: Restructure code into "transactions" that represent the different states
			-- of scrolling to "bottleneck" things like the following 'removeEventListener()' call
			-- so they are not sprinkled all over the place.

			Runtime:removeEventListener( "enterFrame", scrollView.content.scrollbar_listener )
			scrollView:cancel_scrollbar_hide()
			
			-- set some variables necessary movement/scrolling
			self.velocity = 0
			self.prevX = self.x
			self.prevY = self.y
			self.prevPositionX = event.x
			self.prevPositionY = event.y
			self.trackVelocity = true
			self.markTime = time
			self.eventStep = 0
			self.upperLimit = scrollView.topPadding or 0
			self.lowerLimit = self.maskHeight - self.contentHeight
			self.leftLimit = 0
			self.rightLimit = self.maskWidth - self.contentWidth
			
			-- determine whether or not to disable horizontal/vertical scrolling
			if self.contentWidth <= self.maskWidth or scrollView.isVirtualized then
				self.horizontalScrollDisabled = true
			end
			
			-- reset move direction
			self.moveDirection = nil
			
			-- for tableviews:
			if scrollView.isVirtualized then
				self.moveDirection = "vertical"
			end
			
			-- begin enterFrame listener (for velocity calculations)
			Runtime:addEventListener( "enterFrame", self )
			
			-- dispatch scroll event
			
			if self.listener then
				local event = event
				event.name = "scrollEvent"
				event.type = "contentTouch"
				event.phase = "press"
				event.target = scrollView
				self.listener( event )
			end
			
			
			-- change lowerLimit if scrollView is "virtualized" (used for tableViews)
			if scrollView.isVirtualized and scrollView.virtualContentHeight then
				self.lowerLimit = self.maskHeight - scrollView.virtualContentHeight
				if scrollView.bottomPadding then
					self.lowerLimit = self.lowerLimit - scrollView.bottomPadding
				end
			end
		
		elseif self.isFocus then
			if phase == "moved" and not scrollView.isLocked then
			
				-- ensure content isn't trying to move while user is dragging content
				if self.tween then transition.cancel( self.tween ); self.tween = nil; end
				
				-- determine if user is attempting to move content left/right or up/down
				if not self.moveDirection then
					if not self.verticalScrollDisabled or not self.horizontalScrollDisabled then
						local dx = mAbs(event.x - event.xStart)
						local dy = mAbs(event.y - event.yStart)
						local moveThresh = 8
						
						if dx > moveThresh or dy > moveThresh then
							if dx > dy then
								self.moveDirection = "horizontal"
							else
								self.moveDirection = "vertical"
							end
							
							--The content has actually started to scroll so dispatch the beganScroll event
							dispatchBeganScroll( self, scrollView )
							self.hasScrolled = true
						end
					end
				else
					
					-- Finger movement and swiping; prevent content from sticking past boundaries
					
					if self.moveDirection == "vertical" and not self.verticalScrollDisabled then
						-- VERTICAL movement
						
						self.delta = event.y - self.prevPositionY
						self.prevPositionY = event.y
						
						-- do "elastic" effect when finger is dragging content past boundaries
						if self.y > self.upperLimit or self.y < self.lowerLimit then
							self.y = self.y + self.delta/2
						else
							self.y = self.y + self.delta
						end
						
						-- modify velocity based on previous move phase
						--self.eventStep = 0
						self.velocity = (self.y - self.prevY) / (time - self.markTime)
						self.markTime = time
						self.prevY = self.y
					
					elseif self.moveDirection == "horizontal" and not self.horizontalScrollDisabled then
						-- HORIZONTAL movement
						
						self.delta = event.x - self.prevPositionX
						self.prevPositionX = event.x
						
						-- do "elastic" effect when finger is dragging content past boundaries
						if self.x > self.leftLimit or self.x < self.rightLimit then
							self.x = self.x + self.delta/2
						else
							self.x = self.x + self.delta
						end
						
						-- modify velocity based on previous move phase
						--self.eventStep = 0
						self.velocity = (self.x - self.prevX) / (time - self.markTime)
						self.markTime = time
						self.prevX = self.x
					end
				end
				
				-- dispatch scroll event 
				
				-- # NOTE # - 
				
				--[[
					If we can throttle this a bit the tableviews wouldn't appear to "jerk" when coming to a halt
				--]]
				if self.listener then
					local event = event
					event.name = "scrollEvent"
					event.type = "contentTouch"
					event.target = scrollView
					self.listener( event )
				end
				
			
			elseif phase == "ended" or phase == "cancelled" then
				
				self.lastTime = time		-- necessary for calculating scroll movement
				self.trackVelocity = nil	-- stop tracking velocity
				self.markTime = nil
				
				-- dispatch scroll event
				if self.listener then
					local event = event
					event.name = "scrollEvent"
					event.type = "contentTouch"
					event.phase = "release"
					event.target = scrollView
					self.listener( event )
				end
								
				-- remove focus from tableView's content
				display.getCurrentStage():setFocus( nil )
				self.isFocus = nil
			end
		end
	
		return true
	end
	
	local function onBackgroundTouch( self, event )
	
		-- This function allows scrollView to be scrolled when only the background of the
		-- widget is being touched (rather than having to touch the content itself)
	
		if event.phase == "began" then
			local content = self.parent.content
			content:touch( event )	-- transfer touch event to content group's touch event
		end
	end
	
	local function getContentPosition( self )
		local content = self.content
		return content.x, content.y
	end
	
	local function takeFocus( self, event )
		local target = event.target.view or event.target
		
		-- if button, restore back to "default" state
		if target.default and target.over then
			target.default.isVisible = true
			target.over.isVisible = false
			local r, g, b, a = target.label.color.default[1] or 0, target.label.color.default[2] or target.label.color.default[1], target.label.color.default[3] or target.label.color.default[1], target.label.color.default[4] or 255
			target.label:setTextColor( r, g, b, a )
		end
	
		-- remove focus from target
		display.getCurrentStage():setFocus( nil )
		target.isFocus = false
		
		-- set event.target to scrollView and start back at "began" phase
		event.target = self
		event.phase = "began"
		self.content.touch( self.content, event )
	end
	
	local function scrollToX( self, x, timeInMs, onComplete )	-- PRIVATE; self == scrollView
		local content = self.content
		if not self then print( "WARNING: The correct way to call scrollToX is with a ':' not a '.'" ); return; end
		if not x then return; end
		local time = timeInMs or 500
		
		if content.tween then transition.cancel( content.tween ); end
		content.tween = transition.to( content, { x=x, time=time, transition=easing.inOutQuad, onComplete=onComplete } )
	end
	
	local function scrollToY( self, y, timeInMs, onComplete )	-- PRIVATE; self == scrollView
		local content = self.content
		if not self then print( "WARNING: The correct way to call scrollToY is with a ':' not a '.'" ); return; end
		if not y then return; end
		local time = timeInMs or 500
		
		if content.tween then transition.cancel( content.tween ); end
		content.tween = transition.to( content, { y=y, time=time, transition=easing.inOutQuad, onComplete=onComplete } )
	end
	
	local function scrollToPosition( ... )
		local self, x, y, timeInMs, onComplete	-- self == scrollView
		
		if arg[1] and type(arg[1]) == "table" then
			self = arg[1]
		end
		
		self = arg[1]
		x = arg[2]
		y = arg[3]
		
		if arg[4] and type(arg[4]) == "number" then
			timeInMs = arg[4]
		elseif arg[4] and type(arg[4]) == "function" then
			onComplete = arg[4]
		end
		
		if arg[5] and type(arg[5]) == "function" then
			onComplete = arg[5]
		end
	
		if not self then print( "WARNING: The correct way to call scrollToPosition is with a ':' not a '.'" ); return; end
		if not x and not y then return; end
		if x and not y then
			scrollToX( self, x, timeInMs, onComplete )
		end
		
		if y and not x then
			scrollToY( self, y, timeInMs, onComplete )
		end
		
		if x and y then
			local content = self.content
			timeInMs = timeInMs or 500
			if content.tween then transition.cancel( content.tween ); end
			content.tween = transition.to( content, { x=x, y=y, time=timeInMs, transition=easing.inOutQuad, onComplete=onComplete } )
		end
	end
	
	local function scrollToTop( ... )
		local self, timeInMs, onComplete
		
		self = arg[1]
		if arg[2] and type(arg[2]) == "number" then
			timeInMs = arg[2]
		elseif arg[2] and type(arg[2]) == "function" then
			onComplete = arg[2]
		end
		
		if arg[3] and type(arg[3]) == "function" then
			onComplete = arg[3]
		end
		
		local content = self.content
		timeInMs = timeInMs or 500
		
		
		if content.tween then transition.cancel( content.tween ); end
		content.tween = transition.to( content, { y=0, time=timeInMs, transition=easing.inOutQuad, onComplete=onComplete } )
	end
	
	local function scrollToBottom( ... )
		local self, timeInMs, onComplete
		
		self = arg[1]
		if arg[2] and type(arg[2]) == "number" then
			timeInMs = arg[2]
		elseif arg[2] and type(arg[2]) == "function" then
			onComplete = arg[2]
		end
		
		if arg[3] and type(arg[3]) == "function" then
			onComplete = arg[3]
		end
		
		local content = self.content
		timeInMs = timeInMs or 500
		local lowerLimit = content.maskHeight - content.contentHeight
		
		if content.tween then transition.cancel( content.tween ); end
		content.tween = transition.to( content, { y=lowerLimit, time=timeInMs, transition=easing.inOutQuad, onComplete=onComplete } )
	end
	
	local function scrollToLeft( ... )
		local self, timeInMs, onComplete
		
		self = arg[1]
		if arg[2] and type(arg[2]) == "number" then
			timeInMs = arg[2]
		elseif arg[2] and type(arg[2]) == "function" then
			onComplete = arg[2]
		end
		
		if arg[3] and type(arg[3]) == "function" then
			onComplete = arg[3]
		end
		
		local content = self.content
		timeInMs = timeInMs or 500
		
		if content.tween then transition.cancel( content.tween ); end
		content.tween = transition.to( content, { x=0, time=timeInMs, transition=easing.inOutQuad, onComplete=onComplete } )
	end
	
	local function scrollToRight( ... )
		local self, timeInMs, onComplete
		
		self = arg[1]
		if arg[2] and type(arg[2]) == "number" then
			timeInMs = arg[2]
		elseif arg[2] and type(arg[2]) == "function" then
			onComplete = arg[2]
		end
		
		if arg[3] and type(arg[3]) == "function" then
			onComplete = arg[3]
		end
		
		local content = self.content
		timeInMs = timeInMs or 500
		local rightLimit = content.maskWidth - content.contentWidth
		
		if content.tween then transition.cancel( content.tween ); end
		content.tween = transition.to( content, { x=rightLimit, time=timeInMs, transition=easing.inOutQuad, onComplete=onComplete } )
	end
	
	local function removeSelf( self )
		-- check to see if there is a clean method; if so, call it
		if self.clean then self:clean(); end
		
		-- remove scrollView content
		if self.content then
			-- cancel any active transitions
			if self.content.tween then transition.cancel( self.content.tween ); self.content.tween = nil; end
			if self.sb_tween then transition.cancel( self.sb_tween ); self.sb_tween = nil; end
			if self.sb_timer then timer.cancel( self.sb_timer ); self.sb_timer = nil; end
			
			-- remove runtime listener
			Runtime:removeEventListener( "enterFrame", self.content )
			Runtime:removeEventListener( "enterFrame", self.content.scrollbar_listener )
			
			-- remove all children from content group
			for i=self.content.numChildren,1,-1 do
				display.remove( self.content[i] )
			end
			
			display.remove( self.content )
			self.content = nil
		end
		
		-- removed fixed (non scrollable) content
		if self.fixed then
			-- remove all children from fixed group
			for i=self.fixed.numChildren,1,-1 do
				display.remove( self.fixed[i] )
			end
			
			display.remove( self.fixed )
			self.fixed = nil
		end
		
		-- remove all children from virtual group
		if self.virtual then
			for i=self.virtual.numChildren,1,-1 do
				display.remove( self.virtual[i] )
			end
			
			display.remove( self.virtual )
			self.virtual = nil
		end
		
		-- remove bitmap mask
		if self.mask then
			self:setMask( nil )
			self.mask = nil
		end
		
		-- call original removeSelf method
		self:cached_removeSelf()
	end

	local function createScrollBar( parent, manual_height, options )
		-- set initial variables
		local fixed_group = parent.fixed
		local scrollbar_width = 6
		local top = 6
		local min_height = 24
		local max_height = parent.widgetHeight-(top*2)

		-- calculate scrollbar height (based on total content height)
		local sb_height
		local content_height = manual_height or parent.content.contentHeight
		local content_bleed = content_height - parent.widgetHeight

		if content_bleed > parent.widgetHeight then
			sb_height = min_height

		elseif content_bleed > 0 then

			local bleed_percent = content_bleed/parent.widgetHeight
			sb_height = max_height-(max_height*bleed_percent)

		else
			display.remove( parent._scrollbar ); parent._scrollbar = nil
			return
		end

		-- calculate proper location of scrollbar (in case a start_percent wasn't provided)
		local amount_above_top = content_height-(content_height+parent.content.y)
		local calculated_percent = (amount_above_top/content_bleed)

		-- calculate scrollbar height and handle "squish" effect when content goes past boundaries
		local min_y = top
		local max_y = (top+max_height)-sb_height
		local scroll_range = max_y-min_y
		local scroll_percent = calculated_percent
		local x = parent.widgetWidth-3
		local y = top+(scroll_range*scroll_percent)
		if y < min_y then
			local difference = min_y - y
			sb_height = sb_height - difference
			if sb_height < min_height then sb_height = min_height; end

			-- don't allow scrollbar to go past minimum y position (even when content goes past boundary)
			y = min_y

		elseif y > max_y then
			
			local difference = y - max_y
			sb_height = sb_height - difference
			if sb_height < min_height then sb_height = min_height; end

			-- adjust y position since we adjusted scrollbar height
			y = (top+max_height)-sb_height
		end

		-- create the actual scrollbar from a rounded rectangle
		local sb = parent._scrollbar
		if not sb then
			sb = display.newRoundedRect( fixed_group, 0, 0, scrollbar_width, sb_height, 2 )			
		else
			sb.height = sb_height
		end
		sb:setReferencePoint( display.TopRightReferencePoint )
		
		if options and options.scrollBarColor and type( options.scrollBarColor ) == "table" then
			sb:setFillColor( unpack( options.scrollBarColor ) )
		else	
			sb:setFillColor( 0, 128 )
		end
		
		sb.x, sb.y = x, y
		sb.alpha = 1.0

		return sb
	end
	
	local function createScrollView( options )
		-- extract parameters or use defaults
		local	options = options or {}
		local	id = options.id or "widget_scrollView"
		local	left = options.left or 0
		local	top = options.top or 0
		local	width = options.width or (display.contentWidth-left)
		local	height = options.height or (display.contentHeight-top)
		local	scrollWidth = options.scrollWidth or width
		local	scrollHeight = options.scrollHeight or height
		local	friction = options.friction or scrollFriction
		local	listener = options.listener
		local	bgColor = options.bgColor or {}
				bgColor[1] = bgColor[1] or 255
				bgColor[2] = bgColor[2] or bgColor[1]
				bgColor[3] = bgColor[3] or bgColor[1]
				bgColor[4] = bgColor[4] or 255
		local	maskFile = options.maskFile
		local	hideBackground = options.hideBackground
		local	isVirtualized = options.isVirtualized
		local	topPadding = options.topPadding
		local	bottomPadding = options.bottomPadding
		local 	baseDir = options.baseDir or system.ResourceDirectory
		
		--Picker (used in the pickers soft landing function)
		local 	isPicker = options.isPicker or nil
		local 	pickerTop = options.pickerTop or nil
		
		-- create display groups
		local scrollView = display.newGroup()	-- display group for widget; will be masked
		local content = display.newGroup()		-- will contain scrolling content
		local virtual = display.newGroup()		-- will contain "virtual" content (such as tableView rows)
		local fixed = display.newGroup()		-- will contain 'fixed' content (that doesn't scroll)
		scrollView:insert( content )
		scrollView:insert( virtual )
		scrollView:insert( fixed )
		
		-- important references
		scrollView.content = content
		scrollView.virtual = virtual
		scrollView.fixed = fixed
		
		-- set some scrollView properties (private properties attached to content group)
		scrollView._isWidget = true
		--Exposed variables for use with picker softlanding function
		scrollView._isPicker = isPicker
		scrollView.pickerTop = pickerTop
		----------------------------------
		scrollView.id = id
		scrollView.widgetWidth = width
		scrollView.widgetHeight = height
		scrollView.isVirtualized = isVirtualized
		scrollView.topPadding = topPadding
		scrollView.bottomPadding = bottomPadding
		content.hideScrollBar = options.hideScrollBar or false
		--Exposed for use with picker softlanding function
		content.selectionHeight = options.selectionHeight or nil
		content.maskWidth = width
		content.maskHeight = height
		content.friction = friction
		content.enterFrame = onScrollViewUpdate	-- enterFrame listener function
		content.touch = onContentTouch; content:addEventListener( "touch", content )
		content.listener = listener
		
		-- scrollView methods
		scrollView.getContentPosition = getContentPosition
		scrollView.takeFocus = takeFocus
		scrollView.scrollToPosition = scrollToPosition
		scrollView.scrollToTop = scrollToTop
		scrollView.scrollToBottom = scrollToBottom
		scrollView.scrollToLeft = scrollToLeft
		scrollView.scrollToRight = scrollToRight
		
		-- create background rectangle for widget
		local bgRect = display.newRect( 0, 0, width, height )
		bgRect:setFillColor( bgColor[1], bgColor[2], bgColor[3], bgColor[4] )
		if hideBackground then bgRect.isVisible = false; end
		bgRect.isHitTestable = true
		bgRect.touch = onBackgroundTouch; bgRect:addEventListener( "touch", bgRect )
		scrollView:insert( 1, bgRect )
		
		-- create a background for actual content
		local contentBg = display.newRect( 0, 0, scrollWidth, scrollHeight )
		contentBg:setFillColor( 255, 100 )
		contentBg.isVisible = false
		content:insert( 1, contentBg )
		
		-- apply mask (if user set option)
		if maskFile then
			scrollView.mask = graphics.newMask( maskFile, baseDir )
			scrollView:setMask( scrollView.mask )
	
			scrollView.maskX = width * 0.5
			scrollView.maskY = height * 0.5
			scrollView.isHitTestMasked = false
		end
	
		-- position widget based on left/top options
		scrollView:setReferencePoint( display.TopLeftReferencePoint )
		scrollView.x, scrollView.y = left, top
	
		-- override removeSelf method for scrollView (to ensure widget is properly removed)
		scrollView.cached_removeSelf = scrollView.removeSelf
		scrollView.removeSelf = removeSelf

		function scrollView.content.scrollbar_listener( event )
			scrollView:update_scrollbar()
		end
		
		-- override insert method for scrollView to insert into content instead
		scrollView.cached_insert = scrollView.insert
		function scrollView:insert( arg1, arg2 )
			local index, obj
			
			if arg1 and type(arg1) == "number" then
				index = arg1
			elseif arg1 and type(arg1) == "table" then
				obj = arg1
			end
			
			if arg2 and type(arg2) == "table" then
				obj = arg2
			end
			
			if index then
				self.content:insert( index, obj )
			else
				self.content:insert( obj )
			end
		end

		-- cancels scrollbar fadeout effect
		function scrollView:cancel_scrollbar_hide()
			if self.sb_tween then transition.cancel( self.sb_tween ); self.sb_tween = nil; end
			if self.sb_timer then timer.cancel( self.sb_timer ); self.sb_timer = nil; end
		end

		-- function to update scrollbar height and position
		function scrollView:update_scrollbar()
			local content_height = self.virtualContentHeight
			self._scrollbar = createScrollBar( self, content_height, options )
		end

		function scrollView:hide_scrollbar()
			Runtime:removeEventListener( "enterFrame", self.content.scrollbar_listener )
			self:cancel_scrollbar_hide()

			local function fade_out()
				local function remove_scrollbar()
					display.remove( self._scrollbar )
					self._scrollbar = nil
					self.sb_tween = nil
				end
				if self.sb_tween then
					transition.cancel( self.sb_tween ); self.sb_tween = nil;
					self._scrollbar.alpha = 1.0
				end
				self.sb_tween = transition.to( self._scrollbar, { time=300, alpha=0, onComplete=remove_scrollbar } )
			end
			self.sb_timer = timer.performWithDelay( 300, fade_out, 1 )
		end
		
		return scrollView	-- returns a display group
	end
	
	return createScrollView( options )
end

-----------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------
--
-- tabBar widget
--
-----------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------

function widget.newTabBar( options )
	
	local function invokeTabButtonSelectionState( button )
		-- ensure overlay and down graphic are showing
		button.up.isVisible = false
		button.down.isVisible = true
		button.selected = true
		if button.label then button.label:setTextColor( 255 ); end
		
		-- if hideOverlay is not set to true, show overlay graphic (to represent selection)
		if not button.hideOverlay then
			button.overlay.isVisible = true
		end
	end
	
	local function onButtonSelection( self )	-- self == tab button
		local tabBar = self.parent.parent
		
		if tabBar and tabBar.deselectAll then
			tabBar:deselectAll()	-- deselect all tab buttons
		end

		invokeTabButtonSelectionState( self )

		-- call listener function
		if self.onPress and type(self.onPress) == "function" then
			local event = {
				name = "tabButtonPress",
				target = self,
			}
			self.onPress( event )
		end
	end
	
	local function onButtonTouch( self, event )		-- self == tab button
		if event.phase == "began" then
			self:onSelection()	-- see: onButtonSelection()
		end
		return true
	end
	
	local function createTabButton( params )
		local	params = params or {}
		local	id = params.id
		local	label = params.label
		local	labelFont = params.font or native.systemFontBold
		local	labelFontSize = params.size or 10
		local	labelColor = params.labelColor or { 124, 124, 124, 255 }
		local	overlayWidth = params.overlayWidth
		local	overlayHeight = params.overlayHeight
		local	disableOverlay = params.disableOverlay
		local	width = params.width or 32		-- corresponds to up/down image width
		local	height = params.height or 32	-- corresponds to up/down image height
		local	cornerRadius = params.cornerRadius or 4
		local	default = params.default or params.up or params.image	-- params.default is supported; others old/deprecated
		local	down = params.down or params.over -- params.down is supported; others old/deprecated
		local	parentObject = params.parent
		local	selected = params.selected
		local	onPress = params.onPress
		local	upGradient = params.upGradient
		local	downGradient = params.downGradient
		local	baseDir = params.baseDir or system.ResourceDirectory

		local button = display.newGroup()
		button.id = id
		button.hideOverlay = disableOverlay
		
		-- create overlay (which is the highlight when button is selected/down)
		button.overlay = display.newRoundedRect( button, 0, 0, overlayWidth, overlayHeight, cornerRadius )
		button.overlay:setFillColor( 255, 25 )
		button.overlay:setStrokeColor( 0, 75 )
		button.overlay.strokeWidth = 1
		button.overlay.isVisible = false
		button.overlay.isHitTestable = true

		button.up = display.newImageRect( button, default, baseDir, width, height )
		button.up:setReferencePoint( display.CenterReferencePoint )
		button.up.x = button.overlay.width * 0.5
		button.up.y = button.overlay.height * 0.5

		if default and not down then down = default; end
		button.down = display.newImageRect( button, down, baseDir, width, height )
		button.down:setReferencePoint( display.CenterReferencePoint )
		button.down.x = button.up.x
		button.down.y = button.up.y
		button.down.isVisible = false

		if label then	-- label is optional
			-- shift icon up
			button.up.y = button.up.y - (labelFontSize-3)
			button.down.y = button.down.y - (labelFontSize-3)

			-- create label
			button.label = display.newText( label, 0, 0, labelFont, labelFontSize )
			local color = { labelColor[1] or 124, labelColor[2] or 124, labelColor[3] or 124, labelColor[4] or 255 }
			button.label:setTextColor( color[1], color[2], color[3], color[4] )
			button.label.color = color
			button.label:setReferencePoint( display.TopCenterReferencePoint )
			button.label.x = button.up.x
			button.label.y = button.up.y + (button.up.contentHeight*0.5)	-- button.up's reference point is center
			button:insert( button.label )
		end

		-- if selected, show overlay and 'down' graphic
		if selected then
			invokeTabButtonSelectionState( button )
		end

		-- touch event
		button.touch = onButtonTouch
		button:addEventListener( "touch", button )

		-- assign onPress event (user-specified listener function)
		button.onPress = onPress
		
		-- selection method to represent button visually and call listener
		button.onSelection = onButtonSelection

		return button
	end
	
	local function deselectAllButtons( self )	-- self == tabBar
		for i=1,self.buttons.numChildren do
			local button = self.buttons[i]

			button.overlay.isVisible = false
			button.down.isVisible = false
			button.up.isVisible = true
			button.selected = false
			if button.label then button.label:setTextColor( button.label.color[1], button.label.color[2], button.label.color[3], button.label.color[4] ); end
		end
	end
	
	local function pressButton( self, buttonIndex, invokeListener )		-- self == tabBar
		self:deselectAll()
		if invokeListener == nil then invokeListener = true; end
		
		local button = self.buttons[buttonIndex]
		if button then
			invokeTabButtonSelectionState( button )
			
			-- call listener function
			if invokeListener then
				if button.onPress and type(button.onPress) == "function" then
					local event = {
						name = "tabButtonPress",
						target = button
					}
					button.onPress( event )
				end
			end
		else
			print( "WARNING: Specified tab button '" .. buttonIndex .. "' does not exist." )
		end
	end
	
	local function removeSelf( self )	-- self == tabBar
		-- check to see if there is a clean method; if so, call it
		if self.clean then self:clean(); end
		
		-- remove all buttons
		for i=self.buttons.numChildren,1,-1 do
			display.remove( self.buttons[i] )
		end
		display.remove( self.buttons )
		
		-- remove gradient (if in use)
		if self.gradientRect then
			self.gradientRect:setFillColor( 0 )	-- so it's no longer using gradient image
			if self.gradient then
				self.gradient = nil
			end
		end
		
		-- remove tab bar widget itself
		self:cached_removeSelf()
	end
	
	local function createTabBar( options, theme )
		local options = options or {}
		local theme = theme or {}
		local id = options.id or "widget_tabBar"
		local buttons = options.buttons
		local maxTabWidth = options.maxTabWidth or 120
		local width = options.width or theme.width or display.contentWidth
		local height = options.height or theme.height or 50
		local background = options.background or theme.background
		local gradient = options.gradient or options.topGradient or theme.gradient	-- gradient must be pre-created using graphics.newGradient
		local topFill = options.topFill or theme.topFill
		local bottomFill = options.bottomFill or theme.bottomFill
		local left = options.left or 0
		local top = options.top or 0
		local baseDir = options.baseDir or system.ResourceDirectory
		
		local tabBar = display.newGroup()
		local barBg = display.newGroup(); tabBar:insert( barBg )
		
		local halfW = width * 0.5
		local halfH = height * 0.5
		
		-- create tab bar background
		if topFill and bottomFill then
			-- background made from two equal halves (2 different fills)
			
			topFill[1] = topFill[1] or 0
			topFill[2] = topFill[2] or topFill[1]
			topFill[3] = topFill[3] or topFill[1]
			topFill[4] = topFill[4] or 255
			
			bottomFill[1] = bottomFill[1] or 0
			bottomFill[2] = bottomFill[2] or bottomFill[1]
			bottomFill[3] = bottomFill[3] or bottomFill[1]
			bottomFill[4] = bottomFill[4] or 255
			
			local topRect = display.newRect( barBg, 0, 0, width, halfH )
			topRect:setFillColor( topFill[1], topFill[2], topFill[3], topFill[4] )
			
			local bottomRect = display.newRect( barBg, 0, halfH, width, halfH )
			bottomRect:setFillColor( bottomFill[1], bottomFill[2], bottomFill[3], bottomFill[4] )
		
		elseif gradient and type(gradient) == "userdata" then
			-- background made from rectangle w/ gradient fill
			
			local bg = display.newRect( barBg, 0, 0, width, height )
			bg:setFillColor( gradient )
			
			if bottomFill then
				bottomFill[1] = bottomFill[1] or 0
				bottomFill[2] = bottomFill[2] or bottomFill[1]
				bottomFill[3] = bottomFill[3] or bottomFill[1]
				bottomFill[4] = bottomFill[4] or 255
				
				local bottomRect = display.newRect( barBg, 0, halfH, width, halfH )
				bottomRect:setFillColor( bottomFill[1], bottomFill[2], bottomFill[3], bottomFill[4] )
			end
			
		elseif background and type(background) == "string" then
			-- background made from user-provided image file
			
			local bg = display.newImageRect( barBg, background, baseDir, width, height )
			bg:setReferencePoint( display.TopLeftReferencePoint )
			bg.x, bg.y = 0, 0
		else
			-- no background or fills specified (default)
			
			-- create a gradient background
			tabBar.gradient = graphics.newGradient( { 39 }, { 0 }, "down" )
			tabBar.gradientRect = display.newRect( barBg, 0, 0, width, height )
			tabBar.gradientRect:setFillColor( tabBar.gradient )
			
			-- create solid black rect for bottom half of background
			local bottomRect = display.newRect( barBg, 0, halfH, width, halfH )
			bottomRect:setFillColor( 0, 0, 0, 255 )
		end
		
		-- background created; create tab buttons
		
		tabBar.buttons = display.newGroup()
		tabBar:insert( tabBar.buttons )
		
		if buttons and type(buttons) == "table" then
			local buttonWidth = mFloor((width/#buttons)-4)
			local buttonHeight = height-4
			local buttonPadding = mFloor(width/buttonWidth)*2
			
			-- ensure button width doesn't exceed maxButtonWidth
			if buttonWidth > maxTabWidth then buttonWidth = maxTabWidth; end
			
			-- construct each button and insert into tabBar.buttons group
			for i=1,#buttons do
				buttons[i].id = buttons[i].id or i
				buttons[i].overlayWidth = buttonWidth
				buttons[i].overlayHeight = buttonHeight
				buttons[i].parent = tabBar

				local tab = createTabButton( buttons[i] )
				tab:setReferencePoint( display.TopLeftReferencePoint )
				tab.x = (buttonWidth*i-buttonWidth) + buttonPadding
				tab.y = 0
				tabBar.buttons:insert( tab )
			end
			
			-- center the 'tabBar.buttons' group on the widget
			tabBar.buttons:setReferencePoint( display.CenterReferencePoint )
			tabBar.buttons.x, tabBar.buttons.y = halfW, halfH
		end
		
		-- position the widget
		tabBar:setReferencePoint( display.TopLeftReferencePoint )
		tabBar.x, tabBar.y = left, top
		tabBar:setReferencePoint( display.CenterReferencePoint )
		
		-- prevent touches from going through tabBar background
		local preventTouches = function( self, event ) return true; end
		barBg.touch = preventTouches
		barBg:addEventListener( "touch", barBg )
		
		-- override removeSelf method to ensure widget is properly freed from memory
		tabBar.cached_removeSelf = tabBar.removeSelf
		tabBar.removeSelf = removeSelf
		
		-- additional tabBar methods and properties
		tabBar._isWidget = true
		tabBar.id = id
		tabBar.deselectAll = deselectAllButtons
		tabBar.makeTabActive = pressButton
		tabBar.pressButton = pressButton	-- to remain compatible with previous version
		
		return tabBar
	end
	
	-- this widget supports visual customization via themes
	local themeOptions
	if widget.theme then
		local tabBarTheme = widget.theme.tabBar
		
		if tabBarTheme then
			if options and options.style then	-- style parameter optionally set by user
				
				-- for themes that support various "styles" per widget
				local style = tabBarTheme[options.style]
				
				if style then themeOptions = style; end
			else
				-- if no style parameter set, use default style specified by theme
				themeOptions = tabBarTheme
			end
		end
	end
	
	return createTabBar( options, themeOptions )
end

-----------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------
--
-- tableView widget (based on scrollView widget)
--
-----------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------

function widget.newTableView( options )
	-- creates group for row, as well as a background and bottom-line
	local function newRowGroup( rowData )
		local row = display.newGroup()
		
		-- create background
		local bg = display.newRect( row, 0, 0, rowData.width, rowData.height )
		bg:setFillColor( rowData.rowColor[1], rowData.rowColor[2], rowData.rowColor[3], rowData.rowColor[4] )
		
		-- create bottom-line
		local line
		
		--Only create the line if the user hasn't specified noLines == true in options table.
		if options and not options.noLines == true or options and not options.noLines or options and options.noLines == false then
			line = display.newLine( row, 0, rowData.height, rowData.width, rowData.height )
			line:setColor( rowData.lineColor[1], rowData.lineColor[2], rowData.lineColor[3], rowData.lineColor[4] )
			row.line = line
		end
		
		row.background = bg
		
		--If the user has specified noLines == true then set row.line to nil
		if type( row.line ) ~= "table" then
			row.line = nil
		end
		
		return row
	end
	
	-- render row based on index in tableView.content.rows table
	local function renderRow( self, row )	-- self == tableView
		local content = self.content
		if row.view then row.view:removeSelf(); end
		row.view = newRowGroup( row )
		self.virtual:insert( row.view )
		
		row.view.x = 0
		row.view.y = row.top
		
		if row.onRender then
			-- set up event table
			local e = {}
			e.name = "tableView_rowRender"
			e.type = "render"
			e.parent = self	-- tableView that this row belongs to
			e.target = row
			e.row = row
			e.id = row.id
			e.view = row.view
			e.background = row.view.background
			e.line = row.view.line
			e.data = row.data
			e.phase = "render"		-- phases: render, press, release, swipeLeft, swipeRight
			e.index = row.index
			
			row.onRender( e )
		end
	end
	
	local function renderCategory( self, row )	-- self == tableView; row should be a table, not an index
		local content = self.content
		
		local newCategoryRender = function()
			if content.category then display.remove( content.category ); end
			
			content.category = newRowGroup( row )
			content.category.index = row.index
			content.category.x, content.category.y = 0, 0 ---row.height
			
			self.fixed:insert( content.category )	-- insert into tableView's 'fixed' group
			
			if row.onRender then
				-- set up event table
				local e = {}
				e.name = "tableView_rowRender"
				e.type = "render"
				e.parent = self	-- tableView that this row belongs to
				e.target = row
				e.row = row
				e.id = row.id
				e.view = content.category
				e.background = content.category.background
				e.line = content.category.line
				e.data = row.data
				e.phase = "render"		-- phases: render, press, release, swipeLeft, swipeRight
				e.index = row.index
				
				row.onRender( e )
			end
		end
		
		if not content.category then
			-- new; no category currently rendered
			newCategoryRender()
			
		else
			-- there is currently a category; render only if it's different
			if content.category.index ~= row.index then
				newCategoryRender()
			end
		end
	end
	
	-- renders row if it does not have a 'view' property (display group)
	local function ensureRowIsRendered( self, row )	-- self == tableView
		if not row.view then
			renderRow( self, row )
		else
			row.view.y = row.top
		end
	end
	
	-- render rows within the render range ( -renderThresh <-- widget height --> renderThresh )
	local function renderVisibleRows( self ) -- self == tableView			
		local content = self.content
		
		-- if widget has been removed during scrolling, be sure to remove certain enterFrame listeners
		if not content or not content.y then
			if content then Runtime:removeEventListener( "enterFrame", content ); end
			Runtime:removeEventListener( "enterFrame", self.rowListener )
			return
		end
		
		local currentCategoryIndex
		local rows = content.rows
		
		-- ensure all rows that are marked .isRendered are rendered	
		for i=1,#rows do
			local row = rows[i]
			-- update top/bottom locations
			row.top = content.y + row.topOffset
			row.bottom = row.top + row.height
			
			-- category "pushing" effect
			if content.category and row.isCategory and row.index ~= content.category.index then
				if row.top < content.category.contentHeight-3 and row.top >= 0 then
					-- push the category upward
					content.category.y = row.top - (content.category.contentHeight-3)
				end
			end
			
			-- determine which category should be rendered (sticky category at top)	
			if row.isCategory and row.top <= 0 then
				currentCategoryIndex = i
			
			elseif row.isCategory and row.top >= 0 and content.category and row.index == content.category.index then
				-- category moved below top of tableView, render previous category
				
				-- render the previous category, if this is not the first current category
				if row.index ~= content.firstCategoryIndex then
					currentCategoryIndex = content.categories["cat-" .. row.index]	-- stores reference to previous category index
				else
					-- remove current category if the first category moved below top of tableView
					if content.category then
						content.category:removeSelf()
						content.category = nil
						currentCategoryIndex = nil
					end
				end
			end
			
			-- check to see if row is within viewable area
			local belowTopThresh = row.bottom > -self.renderThresh
			local aboveBottomThresh = row.top < self.widgetHeight+self.renderThresh
			if belowTopThresh and aboveBottomThresh then
				row.isRendered = true
				
				-- ensures rows that are marked for rerendering and within render area get rendered
				if row.reRender then
					if row.view then row.view:removeSelf(); row.view = nil; end
					row.reRender = nil;
					row.isRendered = true
				end
				
				ensureRowIsRendered( self, row )
				
				-- hide row's view if it happens to be outside of viewable area; show it if in viewable bounds
				if row.bottom < 0 or row.top > self.widgetHeight then
					if row.view.isVisible then row.view.isVisible = false; end
				else
					if not row.view.isVisible then row.view.isVisible = true; end
				end
			else
				row.isRendered = false
				if row.view then row.view:removeSelf(); row.view = nil; end
			end

			-- hide row if it is the current category (category item will be rendered at top of widget)
			if row.index == currentCategoryIndex then
				if row.view then row.view.isVisible = false; end
			end
		end
		
		-- render current category
		if currentCategoryIndex then
			renderCategory( self, rows[currentCategoryIndex] )
		end
	end
	
	-- find the next row insert location (y-coordinate)
	local function getNextRowTop( content )
		local rows = content.rows
		local top, nextIndex = content.y, #rows+1
		
		local final = rows[#rows]
		if final then
			-- return final row's top location + its height
			top = final.top + final.height
		end
		
		return top, nextIndex	-- returns next 'top' coordinate, & the next avail. index
	end
	
	-- iterate through all rows and update row data (such as content offset, total height, category info, etc)
	local function updateRowData( self )	-- self == tableView
		local content = self.content
		local rows = content.rows
		local top = content.y
		local currentY = top
		local firstCategory
		local previousCategory
		local totalHeight = 0
		content.categories = {}
		
		for i=1,#rows do
			rows[i].topOffset = currentY - top
			
			-- update index while we're at it
			rows[i].index = i
			
			-- popuplate content.categories table
			if rows[i].isCategory then
				if not previousCategory then
					content.categories["cat-" .. i] = "first"
					previousCategory = i
				else
					content.categories["cat-" .. i] = previousCategory
					previousCategory = i
				end
				
				if not firstCategory then
					-- store reference to very first category index
					firstCategory = i
				end
			end
			local height = rows[i].height
			currentY = currentY + height + 1
			totalHeight = totalHeight + height + 1
		end
		
		-- make reference to first category
		content.firstCategoryIndex = firstCategory
		
		-- force re-calculation of top/bottom most rendered rows
		--self.topMostRendered = nil
		--self.bottomMostRendered = nil
		
		-- update total height of all rows
		self.virtualContentHeight = totalHeight
	end
	
	-- used to insert new rows into tableView.content.rows table
	local function insertRow( self, params )	-- self == tableView
		local row = {}
		row.id = params.id		-- custom id
		row.data = params.data	-- custom data
		row.width = self.content.maskWidth
		row.height = params.height or 56	-- default row height is 56
		row.isCategory = params.isCategory
		row.onEvent = params.listener or params.onEvent
		row.onRender = params.onRender
		local rowColor = params.rowColor or {}
			rowColor[1] = rowColor[1] or 255
			rowColor[2] = rowColor[2] or rowColor[1]
			rowColor[3] = rowColor[3] or rowColor[1]
			rowColor[4] = rowColor[4] or 255
		local lineColor = params.lineColor or {}
			lineColor[1] = lineColor[1] or 128
			lineColor[2] = lineColor[2] or lineColor[1]
			lineColor[3] = lineColor[3] or lineColor[1]
			lineColor[4] = lineColor[4] or 255
		row.rowColor = rowColor
		row.lineColor = lineColor
		row.top, row.index = getNextRowTop( self.content )
		row.isRendered = false	-- will ensure row gets rendered on next update
		
		-- increase renderThresh property of tableView if row height is larger
		-- renderThresh is the limit above and below widget where rendering occurs
		if row.height >= self.renderThresh then
			self.renderThresh = row.height + 10
		end
		
		-- insert as final item in tableView.content.rows table
		table.insert( self.content.rows, row )
		updateRowData( self )
		if not params.skipRender then renderVisibleRows( self ); end	-- refresh locations/rendering
	end
	
	-- finds rendered row at on-screen y-coordinate and returns the row object
	local function getRowAtCoordinate( self, yPosition )	-- self == tableView
		local top = self.y	-- y-coordinate of tableView, from top-left reference point
		local content = self.content
		--local firstRenderedRow = content.firstRenderedRow or 1
		local rows = content.rows
		local result
		
		for i=1,#rows do
			if rows[i].view then
				local viewBounds = rows[i].view.contentBounds
				local isWithinBounds = yPosition > viewBounds.yMin and yPosition < viewBounds.yMax
				
				-- if yPosition is within the row's view group, return this row
				if isWithinBounds then result = rows[i]; break; end
			end
		end
		
		return result
	end
	
	-- calls onEvent listener for row
	local function dispatchRowTouch( row, phase, parentWidget )
		if row.onEvent then
			-- set up event table
			local e = {}
			e.name = "tableView_rowTouch"
			e.type = "touch"
			e.parent = parentWidget	-- tableView that this row belongs to
			e.target = row
			e.row = row
			e.id = row.id
			e.view = row.view
			e.background = row.view.background
			e.line = row.view.line
			e.data = row.data
			e.phase = phase		-- phases: render, press, tap, release, swipeLeft, swipeRight
			e.index = row.index
			
			row.onEvent( e )
		end
	end
	
	-- listens to scrollView events for tableView
	local function scrollListener( event )
		local tableView = event.target
		local content = tableView.content
		local eType = event.type
		
		local moveRowsWithTween = function( self )	-- self == tableView
			local updateRows = function() renderVisibleRows( self ) end
			if self.rowTimer then timer.cancel( self.rowTimer ); end;
			self.rowTimer = timer.performWithDelay( 1, updateRows, 400 )
		end
		
		if eType == "contentTouch" then
			local tapThresh = 3		-- used to determine if touch was a "drag" or a quick tap
			local moveThresh = 10	-- do not allow swipes once user drags up/down past this amount
			local swipeThresh = 12	-- if finger moves left/right this amount, trigger swipe event
			
			if event.phase == "press" then
				
				-- tableView content has been touched
				
				if tableView.rowTimer then timer.cancel( tableView.rowTimer ); tableView.rowTimer = nil; end;
				Runtime:removeEventListener( "enterFrame", tableView.rowListener )
				
				-- find out which row the touch began on and store a reference to it
				tableView.currentSelectedRow = getRowAtCoordinate( tableView, event.y )
				tableView.renderFrameCount = 0
				tableView.renderFramePace = 0
				content.yDistance = 0
				content.canSwipe = true
				
				Runtime:addEventListener( "enterFrame", tableView.rowListener )
				content.trackRowSelection = true
				
			elseif event.phase == "moved" then
				
				-- tableView content is being dragged
				local canDrag = content.canDrag
				local canSwipe = content.canSwipe
				local yDistance
				
				-- calculate distance traveled in y direction (only when needed)
				if not canDrag or canSwipe then
					yDistance = mAbs( event.y - event.yStart )
				end
				
				-- determine if this touch event could possibly be a "row tap"
				if not canDrag then
					if yDistance > tapThresh then
						content.yDistance = yDistance
						content.canDrag, canDrag = true, true
						content.trackRowSelection = nil
					end
				end
				
				-- determine whether y distance traveled is low enough to allow left/right swipes
				if canSwipe then
					if yDistance > moveThresh then
						canSwipe = nil
						content.canSwipe = nil
					end
				else
					local selectedRow = tableView.currentSelectedRow
					
					if selectedRow and selectedRow.isTouched then
						selectedRow.isTouched = false
						selectedRow.reRender = true
					end
					
					-- ensure rows move with drag
					renderVisibleRows( tableView )
				end
				
				-- left/right swipes
				local row = tableView.currentSelectedRow
				if row and canSwipe then
					local xDistance = event.x - event.xStart
					
					-- check to see if a "swipe" event should be dispatched
					if xDistance > swipeThresh then
						dispatchRowTouch( row, "swipeRight", tableView )
						row.isTouched = false
						row = nil
						tableView.currentSelectedRow = nil
						renderVisibleRows( tableView )
						content.velocity = 0
						content.trackRowSelection = false
					
						-- remove focus from tableView's content
						display.getCurrentStage():setFocus( nil )
						content.isFocus = nil
						
					elseif xDistance < -swipeThresh then
						dispatchRowTouch( row, "swipeLeft", tableView )
						row.isTouched = false
						row = nil
						tableView.currentSelectedRow = nil
						renderVisibleRows( tableView )
						content.velocity = 0
						content.trackRowSelection = false
					
						-- remove focus from tableView's content
						display.getCurrentStage():setFocus( nil )
						content.isFocus = nil
					end
				end
			
			elseif event.phase == "release" then
				
				local row = tableView.currentSelectedRow
				if row then
					if content.yDistance < tapThresh and content.trackRowSelection then
						-- user tapped tableView content (dispatch row release event)
						dispatchRowTouch( row, "tap", tableView )
						row.isTouched = nil
						row = nil
						tableView.currentSelectedRow = nil
					else
						if row and row.isTouched then
							dispatchRowTouch( row, "release", tableView )
							row.isTouched = nil
							row = nil
							tableView.currentSelectedRow = nil
							renderVisibleRows( tableView )
						end
					end
				end
				
				-- variables used during "moved" phase
				content.yDistance = nil
				content.canDrag = nil
				content.canSwipe = nil
				content.trackRowSelection = nil
				
				-- prevents categories from getting 'stuck' in the wrong position
				if content.category and content.category.y ~= 0 then content.category.y = 0; end
			end
		
		elseif eType == "movingToTopLimit" or eType == "movingToBottomLimit" then
			-- prevents categories from getting 'stuck' in the wrong position
			if content.category and content.category.y ~= 0 then content.category.y = 0; end
			moveRowsWithTween( tableView )
		end
	end
	
	-- times how long finger has been held down on a specific row
	local function checkSelectionStatus( self, time )	-- self == tableView
		local content = self.content
		if content.trackRowSelection then
			local timePassed = time - content.markTime
				
			if timePassed > 110 then		-- finger has been held down for more than 100 milliseconds
				content.trackRowSelection = false
				
				-- initiate "press" event if a row is being touched
				local row = self.currentSelectedRow
				if row then
					row.isTouched = true
					dispatchRowTouch( row, "press", tableView )
				end
			end
		end
	end
	
	-- enterFrame listener for tableView widget
	local function rowListener( self, event )	-- self == tableView
		-- limit velocity to maximum amount (self.maxVelocity)
		local velocity = self.content.velocity
		if velocity < -self.maxVelocity then self.content.velocity = -self.maxVelocity; end
		if velocity > self.maxVelocity then self.content.velocity = self.maxVelocity; end
		
		local isTrackingVelocity = self.content.trackVelocity
		if not isTrackingVelocity then
			if self.content.velocity == 0 then
				Runtime:removeEventListener( "enterFrame", self.rowListener )
			end
			-- prevents categories from getting 'stuck' in the wrong position
			local content = self.content
			if content.category and content.category.y ~= 0 then content.category.y = 0; end
			
			-- renderFramePace and renderFrameCount will skip rendering frames if velocity (list travel speed) is too high
			-- [[
			local velocity = mAbs( velocity )
			if velocity >= 4 then
				self.renderFramePace = 1
				
			elseif velocity >= 6 then
				self.renderFramePace = 2
			
			elseif velocity >= 8 then
				self.renderFramePace = 3
			
			elseif velocity >= 10 then
				self.renderFramePace = 4
				
			else
				self.renderFramePace = 0
			end
			
			if self.renderFrameCount >= self.renderFramePace then
				self.renderFrameCount = 0
				renderVisibleRows( self )
			else
				self.renderFrameCount = self.renderFrameCount + 1
			end
			--]]
			
			--renderVisibleRows( self )
		else
			-- check to see if current row should be selected
			checkSelectionStatus( self, event.time )
		end
	end
	
	-- force re-render of all rows
	local function forceReRender( self )	-- self == tableView
		local content = self.content
		local rows = content.rows
		
		--self.topMostRendered, self.bottomMostRendered = nil, nil
		for i=1,#rows do
			local r = rows[i]
			if r.view then r.view:removeSelf(); r.view = nil; end
			r.isRendered = nil
		end
	end
	
	-- find proper category in proportion to specific row being at very top of list
	local function renderProperCategory( self, rows, rowIndex )		-- self == tableView
		local categoryIndex
				
		if rows[rowIndex].isCategory then
			categoryIndex = rowIndex
		else
			-- loop backwards to find the current category
			for i=rowIndex,1,-1 do
				if rows[i].isCategory then
					categoryIndex = i
					break
				end
			end
		end
		
		-- category found; render it and return the index
		if categoryIndex then
			renderCategory( self, rows[categoryIndex] )
			return categoryIndex
		end
	end
	
	-- scroll content to specified y-position
	local function scrollToY( self, yPosition, timeInMs )		-- self == tableView
		yPosition = yPosition or 0
		timeInMs = timeInMs or 1500
				
		if yPosition > 0 and not self._isPicker then
			print( "WARNING: You must specify a y-value less than zero (negative) when using tableView:scrollToY()." )
			return
		end

		local content = self.content
		
		-- called once content is in desired location
		local function contentInPosition()
			local row = getRowAtCoordinate( self, self.y )
			if row then 
				renderProperCategory( self, content.rows, row.index )
			end
		end

		if timeInMs > 0 then
			local updateTimer
			local cancelUpdateTimer = function() timer.cancel( updateTimer ); updateTimer = nil; contentInPosition(); end
			if content.tween then transition.cancel( content.tween ); end
			content.tween = transition.to( content, { y=yPosition, time=timeInMs, transition=easing.outQuad, onComplete=cancelUpdateTimer } )
			local updateRows = function() renderVisibleRows( self ); end
			updateTimer = timer.performWithDelay( 1, updateRows, timeInMs )
		else
			content.y = yPosition
			forceReRender( self )
			renderVisibleRows( self )
			contentInPosition()
		end
	end
	
	-- scroll content to place specific row at top of tableView
	local function scrollToIndex( self, rowIndex, timeInMs )	-- self == tableView
		local content = self.content
		local rows = content.rows
		local padding = self.topPadding or 0
		local yPosition = -(rows[rowIndex].topOffset) + padding
		
		if yPosition then
			if timeInMs then
				scrollToY( self, yPosition, timeInMs )
			else
				content.y = yPosition
				forceReRender( self )
				renderVisibleRows( self )
				renderProperCategory( self, rows, rowIndex ) -- render the appropriate category
			end
		end
	end
	
	-- returns y-position of content
	local function getContentPosition( self )	-- self == tableView
		return self.content.y
	end

	-- clear all rows from tableview (method)
	local function deleteAllRows( self ) -- self == tableView
		local content = self.content
		local rows = content.rows

		for i=#rows,1,-1 do
			local r = rows[i]
			display.remove( r.view ); r.view = nil
			table.remove( rows, r.index )

			if r.isCategory and content.category then
				if content.category.index == r.index then
					display.remove( content.category )
					content.category = nil
				end
			end
		end
		self.content.rows = {}
	end
	
	-- permanently remove specified row from tableView
	local function deleteRow( self, rowOrIndex )	-- self == tableView
		local content = self.content
		local rows = content.rows
		local row
		
		-- function accepts row table or index integer
		if type(rowOrIndex) == "table" then
			row = rowOrIndex
		else
			row = rows[rowOrIndex]
		end
		
		if row then
			if row.isRendered then
				if row.view then display.remove( row.view ); row.view = nil; end
			end
			table.remove( content.rows, row.index )

			if row.isCategory and content.category then
				if content.category.index == row.index then
					display.remove( content.category )
					content.category = nil
				end
			end
		end
		
		updateRowData( self )
		renderVisibleRows( self )
	end
	
	-- cleanup function (automatically called prior to calling removeSelf)
	local function cleanTableView( self )	-- self == tableView
		-- remove tableView's enterframe listener
		Runtime:removeEventListener( "enterFrame", self.rowListener )
		self.rowListener = nil
		
		-- cancel any potentially active timers/transitions
		if self.rowTimer then timer.cancel( self.rowTimer ); self.rowTimer = nil; end
		if self.content and self.content.tween then
			transition.cancel( self.content.tween ); self.content.tween = nil
		end
		
		-- remove category object if it exists
		if self.content.category then
			self.content.category:removeSelf()
			self.content.category = nil
		end
	end
	
	local function createTableView( options )
		local options = options or {}
		
		-- tableView-specific parameters
		local	id = options.id or "widget_tableView"
		local	renderThresh = options.renderThresh or 100	-- px amount above and below to begin rendering rows
		
		-- shared scrollView parameters (if not specified will use scrollView defaults)
		local	left = options.left or 0
		local	top = options.top or 0
		local	width = options.width or (display.contentWidth-left)
		local	height = options.height or (display.contentHeight-top)
		local	scrollWidth = width
		local	scrollHeight = height
		local	friction = options.friction
		local	bgColor = options.bgColor
		local	maskFile = options.maskFile
		local	hideBackground = options.hideBackground
		local	keepRowsPastTopVisible = options.keepRowsPastTopVisible
		local	topPadding = options.topPadding
		local	bottomPadding = options.bottomPadding
		local	maxVelocity = options.maxVelocity or 10
		local 	baseDir = options.baseDir or system.ResourceDirectory
		local 	hideScrollBar = options.hideScrollBar or false
		local 	scrollBarColor = options.scrollBarColor
		
		--Picker - Variables used by the picker soft landing function
		local 	selectionHeight = options.selectionHeight or nil
		local 	isPicker = options.isPicker or false
		local 	pickerTop = options.pickerTop or nil
				
		-- tableView foundation is a scrollView widget
		local tableView = widget.newScrollView{
			id = id,
			left = left,
			top = top,
			width = width,
			height = height,
			scrollWidth = scrollWidth,
			scrollHeight = scrollHeight,
			friction = friction,
			bgColor = bgColor,
			maskFile = maskFile,
			hideBackground = hideBackground,
			listener = scrollListener,
			isVirtualized = true,
			topPadding = topPadding,
			bottomPadding = bottomPadding,
			baseDir = baseDir,
			hideScrollBar = hideScrollBar,
			scrollBarColor = scrollBarColor,
			selectionHeight = selectionHeight,
		}
		
		-- properties and methods
		tableView._isWidget = true
		--Variables exposed for picker soft landing function
		tableView._isPicker = isPicker
		tableView.pickerTop = pickerTop
		-------------------------------------
		function tableView.rowListener( event ) rowListener( tableView, event ); end
		tableView.content.rows = {}	-- holds row data
		tableView.insertRow = insertRow
		tableView.deleteRow = deleteRow
		tableView.deleteAllRows = deleteAllRows
		tableView.renderThresh = renderThresh
		tableView.scrollToY = scrollToY
		tableView.scrollToIndex = scrollToIndex
		tableView.getRowAtCoordinate = getRowAtCoordinate
		tableView.getContentPosition = getContentPosition
		tableView.keepRowsPastTopVisible = keepRowsPastTopVisible
		tableView.maxVelocity = maxVelocity
		tableView.renderFramePace = 0
		tableView.renderFrameCount = 0
		
		-- clean method will be called whenever 'removeSelf' is called
		tableView.clean = cleanTableView
		
		-- position the content based on 'topPadding' variable
		tableView.content.y = topPadding or 0
		
		return tableView
	end
	
	return createTableView( options )
end

-----------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------

return widget

