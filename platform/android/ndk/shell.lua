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


-- Variables that cross the boundaries between different components handled by shell.lua
local onShellComplete = params.onShellComplete
local handleCheckingForExpansionFiles
local onShellCompleteCalled = false

-- Arrange to call onShellComplete() only once
local function callOnShellComplete(param)
	if not onShellCompleteCalled then
		onShellCompleteCalled = true
		onShellComplete(param)
	end
end

---------------------------------------------------------------------------------------------------
-- This part is used for Corona Cards to show the trial overlay
---------------------------------------------------------------------------------------------------
local overlay = params.overlay

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

---------------------------------------------------------------------------------------------------
-- This part is used to check if Google Play Services is available
---------------------------------------------------------------------------------------------------

-- Determines if a module is available for the given module name.
-- This is done by partially "require"-ing the module in, returning false if it couldn't be required in.
-- From: http://stackoverflow.com/questions/15429236/how-to-check-if-a-module-exists-in-lua/15434737#15434737
local function isModuleAvailable(name)
	-- If this module is in the loaded table, it's already available.
	if package.loaded[name] then
		return true
	else
		-- "package.searchers" is the Lua 5.2 rename of "package.loaders"
		-- Per the documentation: https://www.lua.org/manual/5.1/manual.html#pdf-package.loaders
		-- Each entry in the package.searchers or package.loaders table is a searcher function
		for _, searcher in ipairs(package.searchers or package.loaders) do
			-- Run this searcher function, hoping to find a loader function for this module.
			local loader = searcher(name)
			if type(loader) == 'function' then
				-- We've found a loader function, which can now be added to our preload table for easier access.
				package.preload[name] = loader
				-- A Lua loader exists for this module, so it must be available to use.
				return true
			end
		end

		-- No Lua loader function was found. The module does not exist.
		return false
	end
end

local usingGooglePlayServices = isModuleAvailable("shared.google.play.services.base")
if usingGooglePlayServices then

	local gps = require("shared.google.play.services.base")

	local function googlePlayServicesAvailabileListener( event )
		gps.clearAvailabilityListener()

		-- Now handle checking for expansion files
		handleCheckingForExpansionFiles()
	end

	-- Set a listener to fire when it does become available.
	gps.setAvailabilityListener(googlePlayServicesAvailabileListener)

	-- Checks if Google Play Services is available and attempts to make it available if it's not.
	gps.handleGooglePlayServicesAvailability()

end

---------------------------------------------------------------------------------------------------
-- This part is used to check for expansion files
---------------------------------------------------------------------------------------------------

handleCheckingForExpansionFiles = function()
	local shouldExit = false

	-- we aren't targeting the google play store then assume we don't need expansion files 
	if system.getInfo("targetAppStore") ~= "google" then
		callOnShellComplete(nil)
		shouldExit = true
		return
	end

	local licensing = require("licensing")
	local initSuccess = false
	local pcallSuccess = pcall( function() initSuccess = licensing.init("google") end )

	-- if we can't init then theres something wrong too also if expansion files aren't required then we can leave
	if not ( pcallSuccess and initSuccess and licensing.isGoogleExpansionFileRequired() ) then
		callOnShellComplete(nil)
		shouldExit = true
		return
	end
	----------------------------------
	-- Checks to make sure external storage is mounted, if it is then it starts the verification process
	-- As of 3/21/2013 there is a bug in android where only 2 alerts will show up and then they won't show anymore.
	local sdCardChecker
	local function storageStateListener(event)
		if "clicked" == event.action then
			-- click "Ok" then we want to try again
			if 1 == event.index then
				sdCardChecker()
			else -- click exit we want to stop the whole thing and exit
				native.requestExit()
			end
		end
	end

	local verifyListener

	-- Checks to see if the app has been upgraded and if not then it looks for the expected expansion files to see if they are there
	-- If the app has been updated then we want to check no matter what to make sure there are no updates to the expansion files
	local function needsToDownloadFiles()
		local savedFileNames = licensing.getFileNamesFromPreferences()
		local isNewAppVersion = licensing.isNewAppVersion()
		
		if not isNewAppVersion then
			-- We only get to this point if expansion files are required.  On the first run through, we get no saved expansion file names from preferences
			-- so to guard against that we have to make sure there is at least one saved expansion file name.
			local hasNothing = true
			local fileHandler
			for key, fileName in pairs(savedFileNames) do
				hasNothing = false
				absPath = system.pathForFile("[[ExpansionFileDirectory]]"..fileName, system.TemporaryDirectory)
				fileHandler = io.open(absPath)
				if not fileHandler then
					return true
				end
				io.close(fileHandler)
			end	
			return hasNothing
		end
		return true
	end


	sdCardChecker = function()
		if "mounted" ~= licensing.getExternalStorageState() then
			-- SD Card isn't mounted then we check again and again until it is
			native.showAlert("Problem downloading expansion file", "Please mount your external storage.", {"OK", "Exit"}, storageStateListener)
		elseif needsToDownloadFiles() then
			licensing.verify(verifyListener, true)
		else
			licensing.loadExpansionFiles()
			callOnShellComplete(nil)
		end
	end


	local function noStoragePermissionListener( event )
		native.requestExit( )
	end


	local function storagePermissionListener( event )
		-- See if the Storage permission was granted.
		for i = 1, #event.grantedAppPermissions do
			if ("Storage" == event.grantedAppPermissions[i]) then
				-- Storage permission was granted! Try to load expansion files!
				sdCardChecker()
				return
			end
		end
		
		-- Let the user know the app is exiting because Storage permission wasn't granted!
		local appName = system.getInfo("appName")
		native.showAlert( appName .. " will now exit!", 
			"To use " .. appName .. " please grant the Storage permission so Expansion Files can be downloaded!",
			{"Exit"}, noStoragePermissionListener)
	end


	local function storagePermissionChecker()
		-- Check for the WRITE_EXTERNAL_STORAGE permission, requesting it if needed!
	    local grantedPermissions = system.getInfo( "androidGrantedAppPermissions" )
	    local storagePermissionGranted = false

	    for i = 1, #grantedPermissions do
	        if ( "android.permission.WRITE_EXTERNAL_STORAGE" == grantedPermissions[i] ) then
	            storagePermissionGranted = true
	            break
	        end
	    end

		if not storagePermissionGranted then
			local appName = system.getInfo("appName")
			if native.canShowPopup( "requestAppPermission" ) then
				-- Request the WRITE_EXTERNAL_STORAGE permission
				native.showPopup( "requestAppPermission", {
					appPermission = "android.permission.WRITE_EXTERNAL_STORAGE",
					urgency = "Critical",
					listener = storagePermissionListener,
					rationaleTitle = appName .. " needs Storage permission!",
					rationaleDescription = appName .. " uses Expansion Files and needs external storage permission to access them. Re-request access?",
					settingsRedirectTitle = appName .. " cannot continue!",
					settingsRedirectDescription = "Please grant " .. appName .. " Storage access in Settings so that it can use Expansion Files.",
				} )
			else
				-- The user is on an Android device that can't request permissions.
				-- If we get here, then the developer forgot to add the WRITE_EXTERNAL_STORAGE permission to the build.settings!
				native.showAlert( "Corona: Developer Error", 
					appName .. " cannot download Expansion Files because \"android.permission.WRITE_EXTERNAL_STORAGE\" is missing " ..
					"from build.settings! \n\nPlease add this permission to the project's build.settings and deploy again to continue!", 
					{"OK"}, noStoragePermissionListener )
			end
		else
			-- Permission is granted, check the state of the SD card before loading expansion files.
			sdCardChecker()
		end
	end


	--display objects
	local titleBackground
	local title
	local downloadProgressView
	local screenText
	local statusText
	local startButton
	local gradientBackground
	local appIcon

	----------------------------------

	verifyListener = function( event )
		local widget = require("widget")
		local lfs = require "lfs"
		local expansionFiles = event.expansionFiles
		local hasSomething = next(expansionFiles, nil)

		--counters
		local totalFiles = 0
		local totalFileSize = 0
		local totalFileSizeConverted = 0
		local totalBytesTransferred = 0
		local currentFile = 1

		--handlers
		local currentDownload
		local checkForSpace

		--functions
		local downloadNext
		local exitCleanly
		local startStopDownload
		local checkForDownloaded
		local deleteOldExpansionFiles

		--variables
		local fileNames = {}

		--params for network.download
		local params = {
			successNotification =
			{
				alert = system.getInfo("appName") .. " has finished downloading resources.",
				custom =
				{
					isExpansionFileNotification = true
				},
		   },
		   progress = true
		}

		local function removeAllUi ()
			display.remove(downloadProgressView)
			display.remove(screenText)
			display.remove(statusText)
			display.remove(startButton)
			display.remove(gradientBackground)
			display.remove(titleBackground)
			display.remove(title)
			display.remove(appIcon)
		end

		-- Rotate the text fields and progress bar on rotation
		local function onBackgroundOrientation (event)
			local textSize = display.viewableContentWidth/20

			local progress = 0
			local progressText = "0.00/"..totalFileSizeConverted.."MB"

			--Save the current download progress text
			if statusText and statusText.text ~= "" then
				progressText = statusText.text
			end

			--Save the current download progress
			if downloadProgressView then
				progress = downloadProgressView:getProgress()
			end

			--Its bad to remove the widget everytime but I ran into a bug where setting the widget's width to display.contentWidth kept shrinking it
			removeAllUi()

			-- This is to account for letterbox scaling.  When its letterbox the viewableContentHeight/viewableContentWidth is only the size of the letterbox
			local letterBoxHeight = 0
			local letterBoxWidth = display.viewableContentWidth
			if application and application.content and application.content.scale == "letterbox" then
				letterBoxWidth = letterBoxWidth - (2 * display.screenOriginX)
				letterBoxHeight = 2 * display.screenOriginY
			end

			gradientBackground = display.newRect(display.contentCenterX, display.contentCenterY, letterBoxWidth, display.viewableContentHeight - letterBoxHeight)
			gradientBackground:setFillColor( .1, .2, .3 )

			titleBackground = display.newRect(display.contentCenterX, display.screenOriginY + display.viewableContentHeight/20, letterBoxWidth, display.viewableContentHeight/10)
			titleBackground:setFillColor( .4, .5, .6 )

			appIcon = display.newImageRect("android.app.icon://", display.viewableContentHeight/10, display.viewableContentHeight/10)
			appIcon.x = display.screenOriginX + appIcon.width/2
			appIcon.y = display.screenOriginY + appIcon.height/2

			title = display.newText(system.getInfo("appName"), display.contentCenterX, display.screenOriginY, native.systemFontBold, textSize)
			title.x = titleBackground.x
			title.y = titleBackground.y

			downloadProgressView = widget.newProgressView
			{
				left = display.screenOriginX,
				top = titleBackground.y + titleBackground.height/2,
				width = letterBoxWidth,
				isAnimated = false
			}
			downloadProgressView:setProgress(progress)

			local buttonText = "Restart Download"
			local text1 = "Press restart to download resource files"
			if currentDownload then
				buttonText = "Cancel Download"
				text1 = "Downloading resource files"
			end

			screenText = display.newText(text1, display.contentCenterX, downloadProgressView.y + downloadProgressView.height, native.systemFont, textSize)
			statusText= display.newText(progressText, display.contentCenterX, screenText.y + screenText.height, native.systemFont, textSize)
			
			--checks the width of the button text so that we can size the button appropriatly
			local textSizeCalculation = display.newText(buttonText, 0, 0, native.systemFont, textSize)
			local sizeOfButtonTextWidth = textSizeCalculation.width
			local sizeOfButtonTextHeight = textSizeCalculation.height
			display.remove(textSizeCalculation)

			startButton = widget.newButton{
				left = display.screenOriginX,
				top = statusText.y + statusText.height/2,
				width = sizeOfButtonTextWidth + display.viewableContentWidth/20,
				height = sizeOfButtonTextHeight + display.viewableContentHeight/40,
				label = buttonText,
				fontSize = textSize,
				onEvent = startStopDownload,
			}
		end

		-- Updates the text fields and progress bar with the amount downloaded
		local function downloadProgress (event)
			if event.phase == "progress" then
				downloadProgressView:setProgress((event.bytesTransferred + totalBytesTransferred)/totalFileSize)
				local bytesTrans = string.format("%4.2f", (event.bytesTransferred + totalBytesTransferred)/1048576)

				statusText.text = bytesTrans .. "/" .. totalFileSizeConverted .. "MB"
				-- This is to account for the case where bytesTrans gets bigger and it should push the text to the right
				statusText.x = display.screenOriginX + statusText.width/2
			elseif event.phase == "ended" then
				if event.isError then
					native.showAlert("Network Error", "This application requires Internet access to download its resources on first startup. Please connect your device to the Internet and try again.", {"Ok"})
					local someEvent = {}
					someEvent.phase = "ended"
					startStopDownload(someEvent)
				elseif currentFile >= totalFiles then --all the expansion files have been downloaded
					exitCleanly()
				else --downloads the next file
					totalBytesTransferred = totalBytesTransferred + event.bytesTransferred
					currentFile = currentFile + 1
					downloadNext()
					onBackgroundOrientation(nil)
				end
			end
		end

		-- Initiates the download for the next expansion file
		downloadNext = function ()
			checkForDownloaded()

			if checkForSpace() then
				currentDownload  = network.download(
				    expansionFiles[tostring(currentFile)].url,
				    "GET",
				    downloadProgress,
				    params,
				    "[[ExpansionFileDirectory]]"..expansionFiles[tostring(currentFile)].fileName,
				    system.TemporaryDirectory
				)
			end 
		end

		-- Goes to the external storage directory and checks to see if the file is there
		-- TODO: make sure the size is correct
		checkForDownloaded = function()
			local absPath = system.pathForFile("[[ExpansionFileDirectory]]"..expansionFiles[tostring(currentFile)].fileName, system.TemporaryDirectory)
			local fileHandler = io.open(absPath)
			while fileHandler and currentFile<=totalFiles do 
				io.close(fileHandler)
				totalFileSize = totalFileSize - lfs.attributes(absPath, "size")
				totalFileSizeConverted = string.format("%4.2f", totalFileSize/1048576)
				currentFile = currentFile + 1
				if expansionFiles[tostring(currentFile)] then
					absPath = system.pathForFile("[[ExpansionFileDirectory]]"..expansionFiles[tostring(currentFile)].fileName, system.TemporaryDirectory)
				end
				fileHandler = io.open(absPath)
			end

			if fileHandler then
				io.close(fileHandler)
			end

			if currentFile > totalFiles then
				exitCleanly()
			end
		end

		checkForSpace = function()
			if totalFileSize > licensing.getAvailableExternalSpace() then 
				native.showAlert("Problem downloading expansion file", "Not enough room in your external storage device.", {"Ok"})
				return false
			end
			return true
		end

		-- Called when the start stop button is pressed
		startStopDownload = function(event)
			if event.phase == "ended" then
				if nil == hasSomething then
					licensing.verify(verifyListener, true)
				elseif currentDownload then
					--Stop the current download if there is one otherwise start downloading since its the same button to start/stop
					network.cancel(currentDownload)
					currentDownload = nil
					totalBytesTransferred = 0
					downloadProgressView:setProgress(0)
					statusText.text = ""
					onBackgroundOrientation(nil)
				else
					downloadNext()
					onBackgroundOrientation(nil)
				end
			end		
		end

		-- deletes all files in the expansion file directory that isn't a needed expansion file
		deleteOldExpansionFiles = function()
			local expansionPath = system.pathForFile("[[ExpansionFileDirectory]]", system.TemporaryDirectory)
			if pcall(lfs.dir, expansionPath) then
				for file in lfs.dir(expansionPath) do
					if not (lfs.attributes(file, "mode") == "directory") then
						if fileNames[file] == nil then
							os.remove(system.pathForFile("[[ExpansionFileDirectory]]"..file, system.TemporaryDirectory))
						end
					end
				end
			end
		end

		local function onKeyDown( event )
			-- When the use presses the back key, instead of having to reconnect all the handlers we just "suspend the app", leaving the handlers connected
			if event.phase == "down" and event.keyName == "back" then
				system.request("suspendApplication")
			end
		end

		-- Removes all the UI elements and the listeners and then exits
		exitCleanly = function()
			licensing.loadExpansionFiles()
			removeAllUi()
			Runtime:removeEventListener( "key", onKeyDown)
			Runtime:removeEventListener( "orientation", onBackgroundOrientation )
			callOnShellComplete(nil)
			return
		end

		Runtime:addEventListener( "orientation", onBackgroundOrientation )
		Runtime:addEventListener( "key", onKeyDown)

		onBackgroundOrientation(nil)

		if event.isError or (nil == hasSomething) then 
			native.showAlert("Error", event.response, {"Ok"})
			return
		end

		for k, v in pairs(expansionFiles) do
			fileNames[v.fileName] = v.fileName
			totalFileSize = totalFileSize + v.fileSize
			totalFiles = totalFiles + 1
		end

		-- Total size of all the files that need to be downloaded in mbs
		totalFileSizeConverted = string.format("%4.2f", totalFileSize/1048576)

		--no expansion files so we can exit now
		if totalFiles<1 then
			callOnShellComplete(nil)
		end

		deleteOldExpansionFiles()
		checkForDownloaded()
		onBackgroundOrientation(nil)
		
		--automatically starts the expansion file download
		local autoStart = {}
	    autoStart.phase = "ended"
		startStopDownload(autoStart)
	end
	if not shouldExit then
		storagePermissionChecker()
	end
end


if not usingGooglePlayServices then
	-- Move on to Expansion files if needed.
	handleCheckingForExpansionFiles()
end

-------------------------------------------------------------------------------
-- Splash screen validation for Enterprise
-------------------------------------------------------------------------------
local fileExists, coronaSetup = pcall(require, '_CoronaSetup')
if fileExists then
	coronaSetup.validateSplashScreen()
end

-- following line is commented out because callOnShellComplete should be
-- invoked when expansion file is done expanding
-- callOnShellComplete(nil)

