------------------------------------------------------------------------------
--
-- Copyright (C) 2018 Corona Labs Inc.
-- Contact: support@coronalabs.com
--
-- This file is part of the Corona game engine.
--
-- Commercial License Usage
-- Licensees holding valid commercial Corona licenses may use this file in
-- accordance with the commercial license agreement between you and 
-- Corona Labs Inc. For licensing terms and conditions please contact
-- support@coronalabs.com or visit https://coronalabs.com/com-license
--
-- GNU General Public License Usage
-- Alternatively, this file may be used under the terms of the GNU General
-- Public license version 3. The license is as published by the Free Software
-- Foundation and appearing in the file LICENSE.GPL3 included in the packaging
-- of this file. Please review the following information to ensure the GNU 
-- General Public License requirements will
-- be met: https://www.gnu.org/licenses/gpl-3.0.html
--
-- For overview and more information on licensing please refer to README.md
--
------------------------------------------------------------------------------


local params = ...

local overlay = params.overlay
local onShellComplete = params.onShellComplete
local onShellCompleteCalled = false

-- Arrange to call onShellComplete() only once
local function callOnShellComplete(param)
	if not onShellCompleteCalled then
		onShellCompleteCalled = true
		onShellComplete(param)
	end
end

if params.showMessage then

	local halfW = display.contentWidth * 0.5
	local halfH = display.contentHeight * 0.5

	local fontSize = 96 * display.contentScaleX
	local lineSpace = fontSize * 0.6
	local options = 
	{
		parent = overlay,
		text = "CoronaCards",
		x = halfW,
		y = halfH-lineSpace,
		font = native.systemFontBold,   
		fontSize = fontSize,
	}

	local msg = display.newText( options )
	msg:setFillColor( 0, .1 )

	local msg = display.newText( options )
	msg:setFillColor( 1, .4 )

	local options =
	{
		parent = overlay,
		text = "TRIAL",
		x = halfW,
		y = halfH+lineSpace,
		font = native.systemFontBold,   
		fontSize = fontSize * 1.5,
	}

	local msg = display.newText( options )
	msg:setFillColor( 0, .3 )

	local msg = display.newText( options )
	msg:setFillColor( 1, .2 )

end

-------------------------------------------------------------------------------
-- Splash screen validation for Enterprise
-------------------------------------------------------------------------------
local fileExists, coronaSetup = pcall(require, '_CoronaSetup')
if fileExists then
	coronaSetup.validateSplashScreen()
end

-- Tell the runtime that shell.lua has completed
callOnShellComplete(nil)
