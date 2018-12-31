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

--
-- Platform dependent initialization for Simulators
--


-- Tell luacheck not to flag our builtin globals
-- luacheck: globals system
-- luacheck: globals display
-- luacheck: globals native
-- luacheck: globals network
-- luacheck: globals transition
-- luacheck: globals easing
-- luacheck: globals Runtime

local params = ...

local overlay = params.overlay
local onShellComplete = params.onShellComplete
local statusBarFiles = params.statusBarFiles or { }
local exitCallback = params.exitCallback
local isSimulatorExtension = params.isSimulatorExtension
local appOrientation = nil
local screenDressingMask = nil

local statusBarNames = {
	[display.DefaultStatusBar]=statusBarFiles.default,
	[display.DarkStatusBar]=statusBarFiles.dark,
	[display.TranslucentStatusBar]=statusBarFiles.translucent,
	[display.LightTransparentStatusBar]=statusBarFiles.lightTransparent,
	[display.DarkTransparentStatusBar]=statusBarFiles.darkTransparent,
}

if statusBarFiles and statusBarFiles.default and not isSimulatorExtension then
	-- Status bar

	local isGraphicsV1 = ( 1 == display.getDefault( "graphicsCompatibility" ) )

	appOrientation = system.orientation
	local isLandscape = ("landscapeLeft" == appOrientation) or ("landscapeRight" == appOrientation)

	local x = display.screenOriginX
	local y = display.screenOriginY
    local sx = display.contentScaleX
	local sy = display.contentScaleY
	if isLandscape then
		sx, sy = sy, sx
	end

	for _,v in pairs( statusBarNames ) do
		local portraitFilename = v
		local landscapeFilename = string.gsub( portraitFilename, "(.*)%.png", "%1.landscape.png" )
		local imgPortrait = display.newImage( overlay, portraitFilename, system.SkinResourceDirectory, x, y, true )
		local imgLandscape = display.newImage( overlay, landscapeFilename, system.SkinResourceDirectory, x, y, true )

		overlay[portraitFilename] = imgPortrait
		overlay[landscapeFilename] = imgLandscape

        if imgPortrait ~= nil then
            imgPortrait.xScale = sx
            imgPortrait.yScale = sy

			-- Move offscreen
			imgPortrait.y = imgPortrait.y - (imgPortrait.height * 2)
            imgPortrait.isVisible = false
        else
            print("Warning: Could not load status bar image '"..portraitFilename.."'")
        end

        if imgLandscape ~= nil then
            imgLandscape.xScale = sy
            imgLandscape.yScale = sx

            -- Move offscreen
            imgLandscape.y = imgLandscape.y - (imgLandscape.height * 2)
            imgLandscape.isVisible = false
        else
            print("Warning: Could not load status bar image '"..landscapeFilename.."'")
        end
	end

	-- add unique string value after creating status bar image objects
	statusBarNames[display.HiddenStatusBar] = "none"

	function overlay:showScreenDressing( )
		local dressing = self["_screenDressing"]

		if not dressing then
			-- Initial load of screen dressing
			local screenDressingFilename = statusBarFiles.screenDressing

			if screenDressingFilename then
				local w, h = display.pixelWidth, display.pixelHeight
				local cx, cy = display.contentScaleX, display.contentScaleY
				if not appOrientation then
					appOrientation = system.orientation
				end
				if appOrientation == "landscapeLeft" or appOrientation == "landscapeRight" then
					cx, cy = cy, cx
				end

				dressing = display.newImage( overlay, screenDressingFilename, system.SkinResourceDirectory, true )
				dressing.xScale = cx -- * w / dressing.width
				dressing.yScale = cy -- * h / dressing.height
				dressing.isVisible = false

				-- This mask allows us to ignore clicks/taps on the screenDressing overlay
				if not screenDressingMask then
					local screenDressingMaskFilename = string.gsub(screenDressingFilename, "(.*)%.png", "%1.mask.png")
					screenDressingMask = graphics.newMask(screenDressingMaskFilename, system.SkinResourceDirectory)
					dressing:setMask(screenDressingMask)

					local function onTapDressing()
						-- Ignore taps on the screen dressing
						return true
					end

					dressing:addEventListener("tap", onTapDressing)
				end

				self["_screenDressing"] = dressing
			end
		end

		if dressing then
			if not appOrientation then
				appOrientation = system.orientation
			end

			local rotation = 0

			if appOrientation == "portrait" then
				rotation = 0
			elseif appOrientation == "landscapeLeft" then
				rotation = 90
			elseif appOrientation == "portraitUpsideDown" then
				rotation = 180
			elseif appOrientation == "landscapeRight" then
				rotation = 270
			end

			dressing.x = display.screenOriginX
			dressing.y = display.screenOriginY
			dressing.rotation = rotation;
			dressing:translate(dressing.contentWidth*0.5, dressing.contentHeight*0.5)

			dressing.isVisible = true;
		end
	end

	function overlay:setStatusBarMode( mode )
		if not mode then
			print( "WARNING: nil parameter passed to display.setStatusBarMode()" )

			return
		end

		local name = statusBarNames[mode]

		if not appOrientation then
			appOrientation = system.orientation
		end
		isLandscape = ("landscapeLeft" == appOrientation) or ("landscapeRight" == appOrientation)
		if isLandscape then
			name = string.gsub( name, "(.*)%.png", "%1.landscape.png" )
		end

		if not name then
			print( "WARNING: invalid parameter passed to display.setStatusBarMode() (expected userdata, got "..type(mode)..")" )
		else
			local current = self.current
			if current ~= name then
				local hiddenKey = statusBarNames[display.HiddenStatusBar]
				if current and current ~= hiddenKey then
					local src = self[current]
					src.isVisible = false
				end

				if name and name ~= hiddenKey then
					local dst = self[name]
					dst.x = display.screenOriginX + dst.contentWidth*0.5
					dst.y = display.screenOriginY + dst.contentHeight*0.5

					dst.isVisible = true;

					--Here we overwrite the top status bar height in the Lua table so that the simulator
					--knows the actual status bar height based on the loaded image size and contentScaleY
					--Note: statusBarHeight is deprecated and reverts to defaults

					rawset(display,"topStatusBarContentHeight",dst.height*display.contentScaleY)

				end

				self.current = name
			end
		end
	end

	overlay:setStatusBarMode(display.TranslucentStatusBar)

	-- Show screen dressing, if any
	overlay:showScreenDressing( )

else
	-- luacheck: push
	-- luacheck: ignore 212 -- Unused argument.
	function overlay:setStatusBarMode( mode )
		if not isSimulatorExtension then
			if type(mode) ~= "userdata" then
				print( "WARNING: invalid parameter passed to display.setStatusBarMode() (expected userdata, got "..type(mode)..")" )

				return
			end
		end
	end

	function overlay:showScreenDressing( )
	end

	-- luacheck: pop
end


local function _on_resize( _ )
	appOrientation =  system.orientation
	local fileName = overlay.current
	local hiddenKey = statusBarNames[display.HiddenStatusBar]

	if fileName and fileName ~= hiddenKey then

		local portraitFileName = string.gsub(fileName, ".landscape", "")
		local landscapeName    = string.gsub( portraitFileName, "(.*)%.png", "%1.landscape.png" )
		local newFileName = portraitFileName

		local isLandscape = ("landscapeLeft" == appOrientation) or ("landscapeRight" == appOrientation)

		if isLandscape then
			newFileName = landscapeName
		end

		local current = overlay.current
		if current ~= newFileName then
			if current and current ~= hiddenKey then
				local src = overlay[current]
				src.isVisible = false
			end

			if newFileName  then
				local dst = overlay[newFileName]
				dst.x = display.screenOriginX + ((dst.width / 2) * dst.xScale)
				dst.y = display.screenOriginY + ((dst.height / 2) * dst.yScale)

				--dst.x = display.screenOriginX
				--dst.y = display.screenOriginY
				dst.isVisible = true;
			end
			overlay.current = newFileName
		end
	end

	overlay:showScreenDressing( )
end
Runtime:addEventListener( "resize", _on_resize )

--------------------------------------------------------------------------------

local PluginSync =
{
	contextForUrl = {},
	queue = {},

	-- This is the catalog of plugin manifest.
	clientCatalogFilename = 'catalog.json',
	clientCatalog = { Version = 2 },
	serverCatalog = { Version = 2 },

	-- It's not mandatory to declare this here. We're doing it for the sake
	-- of making it clear that we're going to populate these values later.
	CatalogFilenamePath = "",

}

local lfs = require("lfs")
local json = require("json")

-- luacheck: push
-- luacheck: ignore 212 -- Unused argument.
function PluginSync:debugPrint(...)
	-- Uncomment to get verbose reporting on PluginSync activities
	-- print("PluginSync: ", ...)
end
-- luacheck: pop

function PluginSync:catalog_should_be_refreshed()
	-- The catalog should be refreshed if the "catalog.json" file is over 10 minutes old.
	local age_in_seconds =
				os.difftime(os.time(), lfs.attributes( self.CatalogFilenamePath ).modification )
	local maximum_age_in_seconds = ( 10 * 60 )
	if ( age_in_seconds >= maximum_age_in_seconds ) then
		return true
	end

	-- The catalog should be refreshed if the file was download by a previous Corona version.
	-- This handles the case where the user has just installed a new version of the Corona Simulator.
	local catalogBuildNumber = self.clientCatalog.CoronaBuild
	if ( catalogBuildNumber ~= system.getInfo("build") ) then
		return true
	end

	-- We do not need to update the catalog file.
	return false
end

function PluginSync:LoadCatalog()
	local f = io.open( self.CatalogFilenamePath )
	if not f then
		return
	end

	local content = f:read( "*a" )
	f:close()
	if not content then
		return
	end

	local catalog = json.decode( content )
	if not catalog then
		return
	end

	if not catalog.Version then
		-- This file isn't versioned.
		return
	end

	if catalog.Version ~= self.clientCatalog.Version then
		-- We want to use the catalog ONLY when the
		-- version number is an exact match.
		return
	end

	self.clientCatalog = catalog
end

function PluginSync:initialize( platform )
	self.platform = platform

	self.CatalogFilenamePath = system.pathForFile( self.clientCatalogFilename,
														system.PluginsDirectory )

	self:LoadCatalog()
end

function PluginSync:UpdateClientCatalog()
	self.clientCatalog.CoronaBuild = system.getInfo("build")
	local content = json.encode( self.clientCatalog )

	local f, ioErrorMessage = io.open( self.CatalogFilenamePath, 'w' ) -- erase previous contents
	if f then
		f:write( content )
		f:close()
		self:debugPrint( self.clientCatalogFilename .. " has been updated." )
	else
		local message = "Error updating Corona's plugin catalog."
		if ( type( ioErrorMessage ) == "string" ) and ( string.len( ioErrorMessage ) > 0 ) then
			message = message .. "\nReason:   " .. ioErrorMessage
		end
		print( message )
	end
end

local function file_exists(name)
	local f=io.open(name,"r")
	if f~=nil then
		io.close(f)
		return true
	else
		return false
	end
end

function PluginSync:buildListOfManifestsToDownload( required_plugin )

	local pluginName = required_plugin.pluginName
	local publisherId = required_plugin.publisherId

	-- This is key serves as the subpath into plugins.coronasphere.com
	-- For example: http://plugins.coronasphere.com/com.coronalabs/plugin.openudid/manifest.json
	local key = publisherId .. '/' .. pluginName
	required_plugin.clientCatalogKey = key

	-- Find reasons to queue the plugin for download.
	local should_queue = false

	local manifest = self.clientCatalog[ key ]

	-- Queue because the manifest does not have the key.
	should_queue = should_queue or ( not manifest )

	-- Queue because the catalog needs to be refreshed
	should_queue = should_queue or PluginSync:catalog_should_be_refreshed()

	if manifest and
		( manifest ~= "" ) then

		-- An empty string ("") manifest means that the plugin has been
		-- marked to avoid redownloading needlessly. This happens when
		-- the manifest of the plugin has been downloaded previously and
		-- it didn't contain support for the current simulator platform.

		local full_file_path = system.pathForFile( manifest.filename,
													system.PluginsDirectory )

		-- Queue because the key/value is present in the manifest,
		-- but the downloaded plugin does not exist.
		should_queue = should_queue or not file_exists( full_file_path )

	end

	if should_queue then
		-- Queue for download.
		table.insert( self.queue, key )
	else
		self:debugPrint( "Skipped download of: " .. key )
	end

end

function PluginSync:downloadQueuedManifests( onComplete )

	-- Only download if there have been keys added
	if #self.queue == 0 then
		-- Nothing to do.
		return
	end

	local http = require("socket.http")
	if http.request( "http://plugins.coronasphere.com" ) == nil then

		-- No internet access.
		local warnNoInternetTime = system.getPreference("simulator", "pluginSyncWarnNoInternetTime", "number") or os.time()

		if os.time() >= warnNoInternetTime then

			local function listener( event )
				if "clicked" == event.action then
					if 2 == event.index then
						-- Suspend warnings for 3 hours
						local preference = { pluginSyncWarnNoInternetTime = (os.time() + (3 * 60 * 60)), }
						system.setPreferences("simulator", preference)
					end

					-- 'Continue anyway', meaning launch the project without
					-- downloading plugins
					onComplete()
				end
			end

			system.deletePreference("simulator", "pluginSyncWarnNoInternetTime")

			-- Handle error case (no internet, or coronasphere is down)
			native.showAlert(
				"Plugin Warning",
				"This project may not behave properly. This project requires certain plugins that need to be downloaded from the Internet, but a connection could not be established.",
				{ "Continue Anyway", "Continue and Don't Show Again" },
				listener )
		else
			-- If we've been asked not to warn about no internet then the default action is
			-- 'Continue anyway', meaning launch the project without downloading plugins
			print("WARNING: PluginSync: no internet to download/check plugins; continuing anyway")
			onComplete()
		end
	else

		-- We have internet access.
		--
		-- Download plugins and defer launching project.
		--
		-- The application won't function properly without plugins,
		-- and the Simulator is already phoning home multiple times
		-- by this point, so it's unnecessary to query the user to
		-- proceed.
		self:downloadManifest( onComplete )

	end

end

function PluginSync:downloadManifest( onComplete )

	native.setActivityIndicator( true )

	self.requests = {}
	self.onComplete = onComplete

	for i=1,#self.queue do
		-- Store a mapping between the key used in the clientCatalog
		-- and the url which we'll need in the networkRequest listener
		local key = self.queue[i]
		local url = 'http://plugins.coronasphere.com/' .. key .. '/manifest.json'
		-- Cache buster
		url = url .. '?ts='..tostring(os.time())

		self.contextForUrl[url] =
		{
			key=key,
		}

		-- make request to get the manifest for the plugin.
		-- later on, we'll have to download the actual plugin file itself.

		self:debugPrint("PluginSync: downloading manifest: " .. url)

		-- Download to memory.
		network.request( url,
							"GET",
							self ) -- "self" is who will receive the networkRequest() event.

		-- and record the url request
		table.insert( self.requests, url )
	end
end

function PluginSync:allDownloadsCompleted()
	native.setActivityIndicator( false )
	self.onComplete()
end

-- luacheck: push
-- luacheck: ignore 212 -- Unused argument.
function PluginSync:unzip( zipPath, dstDir )
	local wasSuccessful = false
	if system.getInfo( "platformName" ) == "Win" then
		-- Fetch the path to the unzip tool included with the Corona Simulator.
		local unzipToolPath = system.pathForFile( "..\\7za.exe", system.SystemResourceDirectory )
		if nil == unzipToolPath then
			print( "PluginSync: Failed to find unzip tool needed to extract plugins." )
			return false
		end

		-- Remove the trailing backslash.
		if string.ends( dstDir, '\\' ) then
			-- Remove last char (-2 means 2nd from end)
			dstDir = string.sub( dstDir, 1, -2 )
		end

		-- Use a hidden/secret Corona Win32 API to run the unzip command line tool invisibly in the background.
		-- We need to do this because Win32 console apps spawn console windows upon execution.
		local executionSettings =
		{
			commandLine = '"' .. unzipToolPath .. '" x -y "' .. zipPath .. '" -o"' ..dstDir .. '"',
			isVisible = false,
		}
		wasSuccessful = system.request( "executeUntilExit", executionSettings )
	else
		local status = os.execute( 'unzip -o "' .. zipPath .. '" -d "' ..dstDir .. '"' )
		if ( 0 == status ) then
			wasSuccessful = true
		end
	end

	return wasSuccessful
end
-- luacheck: pop

local function onInternalRequestUnzipPlugins( event )
	-- Verify that a destination path was provided.
	local destinationPath = event.destinationPath
	if type( destinationPath ) ~= "string" then
		return "onInternalRequestUnzipPlugins() Destination path argument was not provided."
	end

	-- Do not continue if this app does not use any plugins.
	if #params.plugins <= 0 then
		return true
	end

	-- Do not continue if we haven't finished download plugins yet.
	if PluginSync.requests and ( #PluginSync.requests > 0 ) then
		return "Cannot continue until the system finishes downloading plugins."
	end

	-- Unzip all plugins belonging to this app to the given destination directory.
	-- Note: Skip over plugins not in the "build.settings" supported list.
	for pluginIndex = 1, #params.plugins do
		if params.plugins[ pluginIndex ].isSupportedOnThisPlatform then
			local pluginKey = params.plugins[ pluginIndex ].clientCatalogKey
			local pluginEntry = PluginSync.clientCatalog[ pluginKey ]
			if type( pluginEntry ) == "table" then
				local zipFileName = pluginEntry.filename
				if type( zipFileName ) == "string" then
					local zipFilePath = system.pathForFile( zipFileName, system.PluginsDirectory )
					local unzipResult = PluginSync:unzip( zipFilePath, destinationPath )
					if unzipResult == false then
						return "Failed to unzip plugin: " .. zipFileName
					end
				end
			else
				-- detailed info for the console
				print("RequestUnzipPlugins: failed to download plugin: ", json.prettify(params.plugins[ pluginIndex ]))

				return "Unable to download plugin. Make sure that the plugin is available for this platform and that your machine has Internet access.\n\n"..
						"Plugin: "..tostring(params.plugins[ pluginIndex ].pluginName)
			end
		end
	end
	return true
end
Runtime:addEventListener( "_internalRequestUnzipPlugins", onInternalRequestUnzipPlugins )

local function onInternalQueryAreAllPluginsAvailable( _ )
	-- Return true now if this app does not require any plugins.
	if #params.plugins <= 0 then
		return true
	end

	-- Check if all plugins in the queue have been downloaded.
	-- Note: Skip over plugins not in the "build.settings" supported list.
	local hasDownloadedAllPlugins = true
	local missingPluginNames = ""
	for pluginIndex = 1, #params.plugins do
		if params.plugins[ pluginIndex ].isSupportedOnThisPlatform then
			-- Fetch the next plugin in the entry table that this app depends on and make sure its zip file exists.
			local pluginKey = params.plugins[ pluginIndex ].clientCatalogKey
			local wasPluginDownloaded = false
			local pluginEntry = PluginSync.clientCatalog[ pluginKey ]
			if type( pluginEntry ) == "table" then
				local zipFileName = pluginEntry.filename
				if type( zipFileName ) == "string" then
					local testFileExistence = true
					local zipFilePath = system.pathForFile( zipFileName, system.PluginsDirectory, testFileExistence )
					if (type( zipFilePath ) == "string") and (string.len( zipFilePath ) > 0) then
						wasPluginDownloaded = true
					end
				end
			end

			-- If the plugin is missing, then flag it as an error and add it to the missing plugins string.
			if ( wasPluginDownloaded == false ) then
				hasDownloadedAllPlugins = false
				if ( string.len( missingPluginNames ) > 0 ) then
					missingPluginNames = missingPluginNames .. "\n"
				end
				missingPluginNames = missingPluginNames .. pluginKey
			end
		end
	end

	-- If we've failed to find at least 1 plugin this app depends on, then return a string
	-- listing all of the plugins not found as a newline separated list.
	if ( hasDownloadedAllPlugins == false ) then
		return missingPluginNames
	end

	-- All plugin zip files this app depends on have been found.
	return true
end
Runtime:addEventListener( "_internalQueryAreAllPluginsAvailable", onInternalQueryAreAllPluginsAvailable )

function PluginSync:handleManifestDownload( event, context )

	self:debugPrint( "Manifest download response: " .. event.response )
	local serverCatalog = json.decode( event.response )

	if ( event.status == 404 ) or
		( not serverCatalog ) then

		-- No manifest available. Remove the plugin from the clientCatalog.
		self.clientCatalog[ context.key ] = nil
		self:UpdateClientCatalog()

		print( "PluginSync: failed to download plugin: " .. event.url )
		native.showAlert(
				"Plugin Download Error",
				"The following plugin could not be downloaded:\n    " .. context.key .. "\n\nIf you are sure you are requiring the correct plugin name, then please contact support.",
				{ "OK" } )

	else

		-- Find the plugin file.
		local manifest_found = self:findPluginToDownloadFromServerCatalog( context.key, serverCatalog )

		-- luacheck: push
		-- luacheck: ignore 542 -- An empty if branch.

		if manifest_found then
			self:downloadPlugin( context.key, manifest_found )
		else
			-- Nothing to do. Errors are reported in findPluginToDownloadFromServerCatalog() above.
		end

		-- luacheck: pop

	end
end

function PluginSync:handlePluginDownload( event, context )

	-- If it is a .zip file, uncompress it
	local fileTable = event.response

	if type( fileTable ) == "table" then
		local filename = fileTable.filename
		local baseDir = fileTable.baseDirectory
		if "string" == type( filename ) and baseDir then
			local zipPath = system.pathForFile( filename, baseDir )
			local dstPath = system.pathForFile( "", system.PluginsDirectory )

			if not self:unzip( zipPath, dstPath ) then

				-- Unzipping failed.
				self.clientCatalog[ context.key ] = nil
				self:UpdateClientCatalog()

				native.showAlert(
					"Plugin Error",
					"The following plugin appears to be corrupted:\n    " .. tostring( context.key ),
					{ "OK" } )
			end
		end
	end
end

-- NOTE: This is used as the listener for both the manifest.json and the plugin files.
function PluginSync:networkRequest( event )

	if event.isError then
		-- NOTE: we do not bother handling errors
		-- in requests to keep things simple
		if (type(event.response) == "string") and (string.len(event.response) > 0) then
			native.showAlert(
				"Plugin Download Error",
				"The following network error occurred while downloading a plugin.\n\n" .. tostring(event.response),
				{ "OK" } )
		else
			native.showAlert(
				"Plugin Download Error",
				"A network error occurred while downloading a plugin. Please check that you have access to the Internet and try again.",
				{ "OK" } )
		end
	else
		local context = self.contextForUrl[event.url]
		if context then
			-- If there's a context, then this was a download of the
			-- manifest.json file (as opposed to the download of the plugin file)
			PluginSync:handleManifestDownload( event, context )
		else
			-- This is a download of the actual plugin file
			PluginSync:handlePluginDownload( event, context )
		end
	end

	local requests = self.requests
	for i=1,#requests do
		if event.url == requests[i] then
			table.remove( requests, i )

			-- A flaw in this system is that we add to the clientCatalog
			-- before we know the result of the download. We generally assume
			-- that the existence of an entry in the clientCatalog means
			-- that a specific version of a plugin is available. This isn't
			-- the case if we don't get a chance to remove the entry in the
			-- clientCatalog uppon failure.
			--
			-- A solution is to track the downloads (see self.requests), then
			-- add entires into the clientCatalog uppon successful
			-- completion of a transfer (here).
		end
	end

	if #requests <= 0 then
		self:allDownloadsCompleted()
	end

end

function PluginSync:findPluginToDownloadFromServerCatalog( key, serverCatalog )

	if not serverCatalog then
		-- Nothing to do.
		return
	end

	if not serverCatalog.platforms then
		-- Nothing to do.
		-- This ISN'T an error. It's normal for some plugins
		-- NOT to be available for certain platforms.
		--
		self:debugPrint( "Marking this plugin so we don't try to download it again: " .. key )
		self.clientCatalog[ key ] = ""
		self:UpdateClientCatalog()
		return
	end

	local versions = serverCatalog.platforms[ self.platform ]
	if not versions then
		-- Nothing to do.
		-- This ISN'T an error. It's normal for some plugins
		-- NOT to be available for certain platforms.
		--
		self:debugPrint( "Marking this plugin so we don't try to download it again: " .. key )
		self.clientCatalog[ key ] = ""
		self:UpdateClientCatalog()
		return
	end

	-- We're assuming the build number will only grow year-to-year.
	-- ie: The build number ISN'T reset at the begining of a new year.
	-- In which case we can ignore the year, and only take the build
	-- number into account.

	-- Metadata for build of this simulator
	local _, sim_build_number = string.match( system.getInfo( "build" ),
																	'(%d+)%.(%d+)' )
	sim_build_number = tonumber( sim_build_number )
	if not sim_build_number then
		-- Nothing to do.
		return
	end

	-- Actual build we will use to index into the manifest for the download url
	local chosen_build_number = 0

	-- find appropriate version of plugin for this build of the simulator
	local manifest_found

	-- For all the plugin versions available, find the closest
	-- current_build_number to sim_build_number, without going over.
	for current_daily_build_string,
		current_daily_build_manifest in pairs( versions ) do

		local _, current_build_number = string.match( current_daily_build_string,
														'(%d+)%.(%d+)' )
		current_build_number = tonumber( current_build_number )

		if current_build_number then
			if ( current_build_number > chosen_build_number ) and
				( current_build_number <= sim_build_number ) then
				chosen_build_number = current_build_number
				manifest_found = current_daily_build_manifest
			end
		end
	end

	local manifestMessage

	-- On older builds, manifest_found.removed is nil which is false.
	-- On newer builds (2015.2541 and newer), this property can be explicitly
	-- set to designate that a plugin was removed, in which case we force
	-- manifest_found to be nil.
	--
	if "table" == type(manifest_found) and manifest_found.removed then
		manifestMessage = manifest_found.message
		manifest_found = nil
	end

	-- Compare md5 values in case the plugin we already have has been updated.
	if manifest_found then
		if self.clientCatalog[ key ] then
			local full_file_path = system.pathForFile( manifest_found.filename, system.PluginsDirectory )

			if ( file_exists( full_file_path ) and ( manifest_found.md5 == self.clientCatalog[ key ].md5 ) ) then
				-- The file still exists and the md5 values HAVEN'T changed. Nothing to do.
				self:debugPrint( "plugin " .. key .. " DOESN'T need to be updated for platform " .. self.platform .. " Already build number: " .. chosen_build_number )
				return
			else
				print( "PluginSync: plugin " .. key .. " needs to be updated for platform " .. self.platform .. " to build number: " .. chosen_build_number )
			end
		end
	else
		local errorTitle = "Plugin Version Error"
		local errorMsg = "The following plugin is not available for this version of the Simulator (" .. system.getInfo( "build" ) .. ")\n    " .. key

		native.showAlert( errorTitle, errorMsg, { "OK" } )

		-- Print error message to console
		print( errorTitle .. ": " .. errorMsg )
		if ( manifestMessage ) then
			print( "   ", manifestMessage )
		end
	end

	return manifest_found
end

function PluginSync:downloadPlugin( key, manifest )

	print("PluginSync: downloading plugin: " .. manifest.url)

	-- Cache buster
	local url = manifest.url .. '?ts='..tostring(os.time())

	-- Download to a file.
	network.download( url,
						"GET",
						self, -- "self" is who will receive the networkRequest() event.
						manifest.filename,
						system.PluginsDirectory )

	-- and record the url request
	table.insert( self.requests, url )

	-- Add the manifest to the clientCatalog.
	self.clientCatalog[ key ] = manifest
	self:UpdateClientCatalog()
end

--------------------------------------------------------------------------------

local function loadMain( onComplete )

	PluginSync:initialize( params.platform )

	local required_plugins = params.plugins
	if ( not required_plugins ) or
		( #required_plugins == 0 ) then
		-- Nothing to download.
		onComplete( )
		return
	end

	-- Find what needs to be downloaded.
	for i=1,#required_plugins do
		PluginSync:buildListOfManifestsToDownload( required_plugins[i] )
	end

	-- Download.
	PluginSync:downloadQueuedManifests( onComplete )
	if #PluginSync.queue == 0 then
		-- Nothing to download.
		onComplete( )
	end

end

--
-- Show a custom splash screen if one is configured (we don't show the default splash every time as
-- that would be annoying)
--
local _coronaSplashControl = false
local _splashShown = false

if params.plugins then
	for i=1, #params.plugins do
		if params.plugins[i].pluginName == "plugin.CoronaSplashControl" and
           params.plugins[i].publisherId == "com.coronalabs" then
			_coronaSplashControl = true
		end
	end
end

if _coronaSplashControl then
	-- load build.settings so we can see if a custom splash screen is configured
	local _coronaBuildSettings
	local path = system.pathForFile("build.settings", system.ResourceDirectory)
	if path then
		local fp = io.open(path, 'r')

		if fp then
			-- CoronaSDK doesn't expose loadfile() so we need to do it the hard way
			local lua = fp:read( '*a' )
			fp:close()

			local buildSettings = loadstring(lua)
			buildSettings() -- creates "settings" table
			-- luacheck: push
			-- luacheck: ignore 111
			-- luacheck: ignore 113
			_coronaBuildSettings = settings
			settings = nil
			-- luacheck: pop
		end
	end

	if _coronaBuildSettings == nil then
		_coronaBuildSettings = {}
	end

	if _coronaBuildSettings.splashScreen ~= nil then

		local enableSplash = _coronaBuildSettings.splashScreen.enable
		local imageFile = _coronaBuildSettings.splashScreen.image
		local platform = system.getInfo("platform")

		-- platform specific settings override general settings
		if _coronaBuildSettings.splashScreen[platform] ~= nil then
			enableSplash = _coronaBuildSettings.splashScreen[platform].enable
			imageFile = _coronaBuildSettings.splashScreen[platform].image
		end

		if enableSplash and imageFile ~= nil then

		    if system.pathForFile( imageFile, system.ResourceDirectory, true ) then

				-- Size the splash to fit in the current screen, resizing it proportionally

				_splashShown = true
				local inset = 0
				local screenW = display.actualContentWidth - (inset * 2)
				local screenH = display.actualContentHeight - (inset * 2)
				local pointsPerPixel = display.actualContentWidth / display.contentWidth
				local image = display.newImage( imageFile )

				if image.width > screenW then
					image.height = image.height * (screenW / image.width)
					image.width = screenW
				end
				if image.height > screenH then
					image.width = image.width * (screenH / image.height)
					image.height = screenH
				end

				image.x = ((screenW / 2) + inset) / pointsPerPixel
				image.y = ((screenH / 2) + inset)
				image.alpha = 1.0

				local function onSplashComplete( target )
					local onComplete = function()
						onShellComplete( target )
					end

					loadMain( onComplete )
				end

				transition.to( image, { alpha=0.0, delay=1000, time=500, transition=easing.outCubic, onComplete=onSplashComplete } )

			else
				print( "WARNING: splashScreen.image '"..imageFile.."' not found" )
			end
		end
	end
end

if not _splashShown then
	loadMain( onShellComplete )
end

-- Only override os.exit if a function is provided
if exitCallback then
	_G.os.exit = function( code )
		print( code )
		exitCallback( code )
	end
end

--------------------------------------------------------------------------------
