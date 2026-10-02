------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
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
-- luacheck: globals graphics
-- luacheck: globals loadstring

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
				-- local w, h = display.pixelWidth, display.pixelHeight
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
	queue = {},
	now = os.time(),
	-- This is the catalog of plugin manifest.
	clientCatalogFilename = 'catalog.json',
	clientCatalog = { Version = 3 },

	-- It's not mandatory to declare this here. We're doing it for the sake
	-- of making it clear that we're going to populate these values later.
	CatalogFilenamePath = "",

}

local json = require("json")

-- luacheck: push
-- luacheck: ignore 212 -- Unused argument.
function PluginSync:debugPrint(...)
	-- Uncomment to get verbose reporting on PluginSync activities
	-- print("PluginSync: ", ...)
end
-- luacheck: pop

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

	local catalogBuildNumber = catalog.CoronaBuild
	if catalogBuildNumber ~= system.getInfo("build") then
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
	else
		local message = "Error updating Corona's plugin catalog."
		if ( type( ioErrorMessage ) == "string" ) and ( string.len( ioErrorMessage ) > 0 ) then
			message = message .. "\nReason:   " .. ioErrorMessage
		end
		print( message )
	end
end


function PluginSync:addPluginToQueueIfRequired( required_plugin )

	local pluginName = required_plugin.pluginName
	local publisherId = required_plugin.publisherId
	local key = tostring(publisherId) .. '/' .. pluginName
	required_plugin.clientCatalogKey = key

	-- Find reasons to queue the plugin for download.
	local should_queue = false
	local maxAge = (system.getPreference("simulator", "SimPluginCacheMaxAge","number") or 24) * 3600

	local manifest = self.clientCatalog[ key ]
	should_queue = should_queue or ( not manifest )
	if type(manifest) == 'table' and type(manifest.lastUpdate) == 'number'  then
		local age = os.difftime(self.now, manifest.lastUpdate)
		-- update plugins every 24 hours or so
		should_queue = should_queue or ( age > maxAge and maxAge > 0)
	else
		should_queue = true
	end

	if should_queue then
		-- Queue for download.
		table.insert( self.queue, required_plugin )
	end

end

local function collectPlugins(localQueue, extractLocation, platform, continueOnError, asyncOnComplete)
	local plugins = {}
	for i=1,#localQueue do
		local pluginInfo = localQueue[i]
		plugins[pluginInfo.pluginName] = {}
		if pluginInfo.json then
			plugins[pluginInfo.pluginName] = json.decode(pluginInfo.json)
		end
		plugins[pluginInfo.pluginName].publisherId = pluginInfo.publisherId
		if continueOnError and asyncOnComplete then
			if type(plugins[pluginInfo.pluginName].supportedPlatforms) == 'table' then
				if not plugins[pluginInfo.pluginName].supportedPlatforms[platform] then
					if plugins[pluginInfo.pluginName].supportedPlatforms[platform] ~= false then
						plugins[pluginInfo.pluginName].supportedPlatforms[platform] = true
					end
				end
			end
		end
	end
	local _, sim_build_number = string.match( system.getInfo( "build" ), '(%d+)%.(%d+)' )

	local collectorParams = {
		pluginPlatform = platform,
		plugins = plugins,
		destinationDirectory = system.pathForFile("", system.PluginsDirectory),
		build = sim_build_number,
		extractLocation = extractLocation,
		continueOnError = continueOnError,
	}
	return params.shellPluginCollector(json.encode(collectorParams), asyncOnComplete)
end

function PluginSync:downloadQueuedPlugins( onComplete )

	-- Only download if there have been keys added
	if #self.queue == 0 then
		-- Nothing to do.
		return
	end

	native.setActivityIndicator( true )

	self.onComplete = onComplete

	collectPlugins(self.queue, system.pathForFile("", system.PluginsDirectory), self.platform, true, function(result)
		local updateTime = self.now
		if type(result.result) == 'string' then
			updateTime = nil
			local res = result.result:gsub('%[(.-)%]%((https?://.-)%)', '%1 (%2)')
			print("WARNING: there was an issue while downloading simulator plugin placeholders:\n" .. res)
		end
		for i=1,#self.queue do
			local key = self.queue[i].clientCatalogKey
			self.clientCatalog[ key ] = { lastUpdate = updateTime }
		end
		self:UpdateClientCatalog()

		native.setActivityIndicator( false )
		self.onComplete()
	end)
end

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
	local result = collectPlugins(params.plugins, destinationPath, event.platform or params.platform, false, nil)
	if result == nil then
		return true
	else
		return result
	end
end
Runtime:addEventListener( "_internalRequestUnzipPlugins", onInternalRequestUnzipPlugins )

local function onInternalQueryAreAllPluginsAvailable( _ )
	return true
end
Runtime:addEventListener( "_internalQueryAreAllPluginsAvailable", onInternalQueryAreAllPluginsAvailable )

local function loadMain( onComplete )

	PluginSync:initialize( params.platform )
	if not params.shellPluginCollector then
		-- No way to download.
		onComplete( )
		return
	end

	local required_plugins = params.plugins
	if ( not required_plugins ) or
		( #required_plugins == 0 ) then
		-- Nothing to download.
		onComplete( )
		return
	end

	-- Find what needs to be downloaded.
	for i=1,#required_plugins do
		PluginSync:addPluginToQueueIfRequired( required_plugins[i] )
	end

	if #PluginSync.queue == 0 then
		-- Nothing to download.
		onComplete( )
	else
		-- Download.
		PluginSync:downloadQueuedPlugins( onComplete )
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
