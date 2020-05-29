------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

-- Platform dependent initialization for Windows

-- Fetch the launch arguments passed to this script.
local launchArguments = ...

-- If CoronaCards has a watermark message to display to the end-user, then do so on the first frame.
if launchArguments.showMessage then
	local coronaCardsDarkText = nil
	local coronaCardsLightText = nil
	local trialDarkText = nil
	local trialLightText = nil

	-- Displays the CoronaCards trial watermark.
	local onShowTrialWatermark = function(event)
		local fontSize = 96 * display.contentScaleX
		local lineSpace = fontSize * 0.6
		local scale = 1.0

		-- Sets the fill color of a text object using normalized color value (0 - 1) for both graphics v1 and v2.
		local isGraphicsCompatibilityModeEnabled = (1 == display.getDefault("graphicsCompatibility"))
		local setNormalizedFillColorFor = function(displayObject, gray, alpha)
			if isGraphicsCompatibilityModeEnabled then
				gray = gray * 255
				alpha = alpha * 255
			end
			displayObject:setFillColor(gray, alpha)
		end

		-- Remove the previous watermark objects, if currently displayed.
		if coronaCardsDarkText then
			coronaCardsDarkText:removeSelf()
			coronaCardsDarkText = nil
		end
		if coronaCardsLightText then
			coronaCardsLightText:removeSelf()
			coronaCardsLightText = nil
		end
		if trialDarkText then
			trialDarkText:removeSelf()
			trialDarkText = nil
		end
		if trialLightText then
			trialLightText:removeSelf()
			trialLightText = nil
		end

		-- Display a "CoronaCards" watermark title.
		local options = 
		{
			parent = launchArguments.overlay,
			text = "CoronaCards",
			x = display.contentCenterX,
			y = display.contentCenterY - lineSpace,
			font = native.systemFontBold,   
			fontSize = fontSize,
		}
		coronaCardsDarkText = display.newText( options )
		setNormalizedFillColorFor(coronaCardsDarkText, 0, 0.1)
		if isGraphicsCompatibilityModeEnabled then
			coronaCardsDarkText:setReferencePoint(display.CenterReferencePoint)
			coronaCardsDarkText:translate(-(coronaCardsDarkText.contentWidth / 2), -(coronaCardsDarkText.contentHeight / 2))
		end
		if (coronaCardsDarkText.contentWidth > display.contentWidth) then
			scale = display.contentWidth / coronaCardsDarkText.contentWidth
			coronaCardsDarkText:scale(scale, scale)
		end
		coronaCardsLightText = display.newText( options )
		setNormalizedFillColorFor(coronaCardsLightText, 1, 0.4)
		if isGraphicsCompatibilityModeEnabled then
			coronaCardsLightText:setReferencePoint(display.CenterReferencePoint)
			coronaCardsLightText:translate(-(coronaCardsLightText.contentWidth / 2), -(coronaCardsLightText.contentHeight / 2))
		end
		if (scale < 1.0) then
			coronaCardsLightText:scale(scale, scale)
		end

		-- Display a "TRIAL" watermark.
		local options =
		{
			parent = launchArguments.overlay,
			text = "TRIAL",
			x = display.contentCenterX,
			y = display.contentCenterY + lineSpace,
			font = native.systemFontBold,   
			fontSize = fontSize * 1.5,
		}
		trialLightText = display.newText( options )
		setNormalizedFillColorFor(trialLightText, 0, 0.3)
		if isGraphicsCompatibilityModeEnabled then
			trialLightText:setReferencePoint(display.CenterReferencePoint)
			trialLightText:translate(-(trialLightText.contentWidth / 2), -(trialLightText.contentHeight / 2))
		end
		if (scale < 1.0) then
			trialLightText:scale(scale, scale)
		end
		trialDarkText = display.newText( options )
		setNormalizedFillColorFor(trialDarkText, 1, 0.2)
		if isGraphicsCompatibilityModeEnabled then
			trialDarkText:setReferencePoint(display.CenterReferencePoint)
			trialDarkText:translate(-(trialDarkText.contentWidth / 2), -(trialDarkText.contentHeight / 2))
		end
		if (scale < 1.0) then
			trialDarkText:scale(scale, scale)
		end
	end

	-- Display the watermark 1 millisecond from now.
	-- Note: We can't call display.newText() immediately here because "shell.lua" gets executed while Corona
	--       is synchronized with the rendering thread. So, we must wait until just before the first render pass.
	timer.performWithDelay(1, onShowTrialWatermark, 1)

	-- Set up an event handler to re-layout the watermark when the Corona control has been resized.
	Runtime:addEventListener("resize", onShowTrialWatermark)
end

-- Notify the Corona runtime that this "shell.lua" script is done doing its work and it's time to run the "main.lua".
launchArguments.onShellComplete(nil)
