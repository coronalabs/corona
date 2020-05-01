------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

local json = require "json"
local lfs = require "lfs"

local SEP = package.config:sub(1,1)
local isWindows = SEP == '\\'

local debugBuildProcess
if isWindows then
    debugBuildProcess = 0
else
    debugBuildProcess = os.execute("exit $(defaults read com.coronalabs.Corona_Simulator debugBuildProcess 2>/dev/null || echo 0)")
end
function log(...)
    if debugBuildProcess > 1 then
        print(...)
    end
end


local function isDir(f)
    return lfs.attributes(f, 'mode') == 'directory'
end

local function isFile(f)
    return lfs.attributes(f, 'mode') == 'file'
end


local function quoteString( str )
    str = str:gsub('\\', '\\\\')
    str = str:gsub('"', '\\"')
    return "\"" .. str .. "\""
end

local function mkdirs(path)
    local c = ""
    path:gsub('[^/\\]+', function(dir)
        if isWindows and c == '' then
            c = dir
        else
            c = c .. SEP .. dir
        end        
        if isDir(c) then
            -- do nothing, directory already exists
        else
            lfs.mkdir(c)
        end
    end)
end

local function exec(cmd)
    log('Running command', cmd)
    if debugBuildProcess < 1 then
        if isWindows then
        else
            cmd = cmd .. ' &> /dev/null'
        end
    end
    return (0 == os.execute(cmd))
end

local function copyFile(src, dst)
    -- note that dst can be directory or file
    if isWindows then
        exec('copy ' .. quoteString(src) .. ' ' .. quoteString(dst))
    else
        exec('/bin/cp ' .. quoteString(src) .. ' ' .. quoteString(dst))
    end
end


local PluginCollectorSolar2DDirectory =  {}
function PluginCollectorSolar2DDirectory:init(params)
    local t = {}
	local directoryPluginsText, msg = params.fetch("https://plugins.solar2d.com/plugins.json")
    if not directoryPluginsText then
        log("Solar2D Directory: error initializing directory " .. tostring(msg))
		return
	end
	local directoryPlugins = json.decode( directoryPluginsText )
	if not directoryPlugins then
		return
    end

    self.pluginsCache = {}

    for repoOwner, repoObject in pairs(directoryPlugins) do
        for providerName,providerObject  in pairs(repoObject) do
            for pluginName, pluginObject in pairs(providerObject) do
                self.pluginsCache[providerName .. '/' .. pluginName] = {repo = repoOwner, plugin = pluginObject}
            end
        end
	end
	return true
end

function PluginCollectorSolar2DDirectory:collect(destination, plugin, pluginTable, pluginPlatform, params)
    if not self.pluginsCache then
        return "Solar2D Directory: directory was not fetched"
    end
    local pluginEntry = self.pluginsCache[tostring(pluginTable.publisherId) .. '/' .. plugin]
    if not pluginEntry then
        return "Solar2D Directory: plugin " .. plugin .. " was not found at Solar2D Directory"
    end
    local pluginObject = pluginEntry.plugin
    local repoOwner = pluginEntry.repo

    local build = tonumber(params.build)
    local vFoundBuid, vFoundObject, vFoundBuildName
    for entryBuild, entryObject in pairs(pluginObject.v) do
        local entryBuildNumber = tonumber(entryBuild:match('^%d+%.(%d+)$'))
        if entryBuildNumber <= build and entryBuildNumber > (vFoundBuid or 0) then
            vFoundBuid = entryBuildNumber
            vFoundObject = entryObject
            vFoundBuildName = entryBuild
        end
    end
    if not vFoundBuid then
        return "Solar2D Directory: unable to find compatible version for " .. plugin .. "."
    end
    local hasPlatform = false
    for i=1,#vFoundObject do
        hasPlatform = hasPlatform or vFoundObject[i] == pluginPlatform
    end
    if not hasPlatform then
        log("Solar2D Directory: skipped plugin " .. plugin .. " because platform is not supported")
        return true
    end
    local repoName = pluginObject.p or (pluginTable.publisherId .. '-' .. plugin)
    local downloadURL = "https://github.com/" .. repoOwner .. "/" .. repoName .. "/releases/download/" .. pluginObject.r .. "/" .. vFoundBuildName .. "-" .. pluginPlatform .. ".tgz"
    local pluginArchivePath = params.pluginStorage .. '/' .. pluginTable.publisherId .. '/' .. plugin .. '/' .. vFoundBuildName .. '/' .. pluginPlatform
    mkdirs(destination)
    local file, err = params.download(downloadURL, destination .. '/data.tgz')
    if not file then
        return "Solar2D Directory: unable to download " .. plugin .. '. Code: ' .. err
    end
    return true
end


local pluginLocatorCoronaStore =  {}
function pluginLocatorCoronaStore:init(params)
    self.user = params.user
    if not self.user  then
        return
    end
    if params.pluginPlatform == 'mac-sim' or params.pluginPlatform == 'win32-sim' then
        return
    end
    self.serverBackend = 'https://backendapi.coronalabs.com'

    local authURL = self.serverBackend .. '/v1/plugins/show/' .. self.user
	local authorisedPluginsText, msg = params.fetch(authURL)

	if not authorisedPluginsText then
		return false
	end

	local authPluginsJson = json.decode( authorisedPluginsText )
	if not authPluginsJson then
		return false
	end

	if authPluginsJson.status ~= 'success' then
		return false
	end

	if not authPluginsJson.data then
		return false
	end
    
    self.authorizedPlugins = {}

    for _, ap in pairs(authPluginsJson.data) do
        local pluginName = ap['plugin_name']
        local pluginDeveloper = ap["plugin_developer"]
        self.authorizedPlugins[pluginName] = self.authorizedPlugins[pluginName] or {}
        self.authorizedPlugins[pluginName][pluginDeveloper] = true
	end
	return true
end

function pluginLocatorCoronaStore:collect(destination, plugin, pluginTable, pluginPlatform, params)
    if not self.user then
        return "Corona Store: no user login"
    end
    if params.pluginPlatform == 'mac-sim' or params.pluginPlatform == 'win32-sim' then
        return "Corona Store: Simulator is not supported by store plugin collector."
    end
    if not self.authorizedPlugins then
        return "Corona Store: authorized plugins was not fetched"
    end
    local developer = pluginTable.publisherId
    if not (self.authorizedPlugins[plugin] or {})[developer] then
        return "Corona Store: plugin " .. plugin .. " was not authorized by the store. Activate at: https://marketplace.coronalabs.com/plugin/" .. developer .. "/" .. plugin
    end

    local build = params.build
    local downloadInfoURL = self.serverBackend .. '/v1/plugins/download/' .. developer .. '/' .. plugin .. '/' .. build .. '/' .. pluginPlatform

    local downloadInfoText, msg = params.fetch(downloadInfoURL)
    if not downloadInfoText then
        return "Corona Store: unable to fetch plugin download location for " .. plugin .. ' ('.. developer.. '). Error message: ' .. msg 
    end

    local downloadInfoJSON = json.decode(downloadInfoText)
    local downloadURL = downloadInfoJSON.url
    if not downloadURL then
        return "Corona Store: unable to parse plugin download location for " .. plugin .. ' ('.. developer.. ').'
    end

    local buildStr = downloadURL:match('/(%d%d%d%d%.%d%d%d%d)/')
    local pluginArchivePath = params.pluginStorage .. '/' .. pluginTable.publisherId .. '/' .. plugin .. '/' .. buildStr .. '/' .. pluginPlatform
    mkdirs(pluginArchivePath)
    pluginArchivePath = pluginArchivePath .. '/data.tgz'
    local file, err = params.download(downloadURL, pluginArchivePath)
    if err then
        return "Corona Store: unable to download " .. plugin .. ' ('.. developer.. '). Code: ' .. err .. 'Error message: \n' .. file
    end

    mkdirs(destination)
    copyFile(pluginArchivePath, destination)
    return true
end


local function pluginLocatorCustomURL(destination, plugin, pluginTable, pluginPlatform, params)
    if type(pluginTable.supportedPlatforms) ~= 'table' then
        return "Custom URL: skipped because no table supportedPlatforms provided for " .. plugin
    end
    if type(pluginTable.supportedPlatforms[pluginPlatform]) ~= 'table' then
        log("Custom URL: skipped because supportedPlatforms[" .. pluginPlatform .. "] is not a table. Plugin is not supported by the platform")
        return true
    end
    if type(pluginTable.supportedPlatforms[pluginPlatform].url) ~= 'string' then
        return "Custom URL: skipped because supportedPlatforms[" .. pluginPlatform .. "].url is not a string"
    end
    local downloadURL = pluginTable.supportedPlatforms[pluginPlatform].url
    mkdirs(destination)
    local file, err = params.download(downloadURL, destination .. '/data.tgz')
    if not file then
        return "Custom URL: unable to download " .. plugin .. ' ('.. developer.. '). Code: ' .. err .. 'Destination: ' .. file .. "; URL: " .. downloadURL
    end

    return true
end


local function pluginLocatorFileSystemVersionized(destination, plugin, pluginTable, pluginPlatform, params)
    if type(pluginTable.publisherId) ~= 'string' then
        return "Locally: plugin has no string publisherId"
    end
    local pluginStorage = params.pluginStorage
    local pluginDir = pluginStorage .. '/' .. pluginTable.publisherId .. '/' .. plugin
    if not isDir(pluginDir) then
        return "Locally: no directory " .. pluginDir
    end
    local targetBuild = tonumber(params.build)
    local lastFound = -1
    local foundDir
    for file in lfs.dir(pluginDir) do
        if file ~= "." and file ~= ".." then
            local f = pluginDir..'/'..file
            if isDir(f) then
                local dirBuild = file:match('^%d+%.(%d+)$')
                if dirBuild then
                    dirBuild = tonumber(dirBuild)
                    if dirBuild > lastFound and dirBuild <= targetBuild then
                        lastFound = dirBuild
                        foundDir = f
                    end 
                end
            end
        end
    end
    if not foundDir then
        return "Locally: didn't find suitable version in " .. pluginDir
    end
    local localPath = foundDir .. '/' .. pluginPlatform .. '/data.tgz'
    if isFile(localPath) then
        mkdirs(destination)
        copyFile(localPath, destination)
        return true
    else
        return "Locally: plugin file not found: " .. localPath
    end
    return false
end


local function pluginLocatorFileSystemAllPlatforms(destination, plugin, pluginTable, pluginPlatform, params)
    if type(pluginTable.publisherId) ~= 'string' then
        return "Locally: plugin has no string publisherId"
    end
    local pluginStorage = params.pluginStorage
    local localPath = pluginStorage .. '/' .. pluginTable.publisherId .. '/' .. plugin .. '/data.tgz'
    if isFile(localPath) then
        mkdirs(destination)
        copyFile(localPath, destination)
        return true
    else
        return "Locally: no file '".. localPath .. "'"
    end
end

local function pluginLocatorFileSystem(destination, plugin, pluginTable, pluginPlatform, params)
    if type(pluginTable.publisherId) ~= 'string' then
        return "Locally: plugin has no string publisherId"
    end
    local pluginStorage = params.pluginStorage
    local localPath = pluginStorage .. '/' .. pluginTable.publisherId .. '/' .. plugin .. '/' .. pluginPlatform .. '/data.tgz'
    if isFile(localPath) then
        mkdirs(destination)
        copyFile(localPath, destination)
        return true
    elseif isDir(pluginStorage .. '/' .. pluginTable.publisherId .. '/' .. plugin) then
        log('Local lookup determined that plugin ' .. plugin .. ' is not supported by the platform ' ..  pluginPlatform)
    else
        return "Locally: no file '".. localPath .. "'"
    end
end

local function pluginLocatorIgnoreMissing(destination, plugin, pluginTable, pluginPlatform, params)
    if type(pluginTable.publisherId) ~= 'string' then
        return "Locally: plugin has no string publisherId"
    end
    local function hasIgnoreMissingMarker(path)
        return isFile(path .. '/' .. params.ignoreMissingMarker)
    end
    local pluginStorage = params.pluginStorage
    local pluginDir = pluginStorage .. '/' .. pluginTable.publisherId .. '/' .. plugin
    if not isDir(pluginDir) then
        return "Ignore Missing checker: no plugin directory " .. pluginDir
    end
    if hasIgnoreMissingMarker(pluginDir) then
        return true
    end
    if hasIgnoreMissingMarker(pluginDir .. '/' .. pluginPlatform) then
        return true
    end

    local targetBuild = tonumber(params.build)
    local lastFound = -1
    local foundDir
    for file in lfs.dir(pluginDir) do
        if file ~= "." and file ~= ".." then
            local f = pluginDir..'/'..file
            if isDir(f) then
                local dirBuild = file:match('^%d+%.(%d+)$')
                if dirBuild then
                    dirBuild = tonumber(dirBuild)
                    if dirBuild > lastFound and dirBuild <= targetBuild then
                        lastFound = dirBuild
                        foundDir = f
                    end 
                end
            end
        end
    end
    if not foundDir then
        return "Ignore Missing checker: didn't find suitable version in " .. pluginDir
    end
    if hasIgnoreMissingMarker(foundDir) or hasIgnoreMissingMarker(foundDir .. '/' .. pluginPlatform) then
        return true
    end
    return "Ignore Missing checker: did not find marker in " .. foundDir
end

local function fetchSinglePlugin(dstDir, plugin, pluginTable, pluginPlatform, params, pluginLocators)
    if type(pluginTable.supportedPlatforms) == 'table' and not pluginTable.supportedPlatforms[pluginPlatform] then
        return
    end
    local pluginDestination = dstDir .. '/' .. plugin
    local err = "Unable to find plugin '" .. plugin .. "' for " .. (params.modernPlatform or params.pluginPlatform) .. " in:"
    local ok =  false
    for i = 1,#pluginLocators do
        local locator = pluginLocators[i]
        local result
        if type(locator) == 'table' then
            if not locator.initialized and type(locator.init) == 'function' then
                locator:init(params)
                locator.initialized = true
            end
            result = locator:collect(pluginDestination, plugin, pluginTable, pluginPlatform, params)
        else
            result = locator(pluginDestination, plugin, pluginTable, pluginPlatform, params)
        end
        if result == true then
            ok = true
            break
        elseif type(result) == 'string' then
            err = err .. '\n\t' .. result
        end
    end
    if not ok then
        return err
    end
end

local function CollectCoronaPlugins(params)
    params.fetch = params.fetch or pluginCollector_fetch
    params.download = params.download or pluginCollector_download
    log("Collecting plugins", json.encode(params))
    local ret = nil

    local pluginLocators = { pluginLocatorCustomURL, pluginLocatorFileSystemVersionized, pluginLocatorFileSystem, pluginLocatorFileSystemAllPlatforms, PluginCollectorSolar2DDirectory, pluginLocatorCoronaStore, pluginLocatorIgnoreMissing }

    local dstDir = params.destinationDirectory

    params.pluginLocators = pluginLocators
    if not params.pluginStorage then
        params.pluginStorage = (os.getenv("HOME") or os.getenv("APPDATA")).. '/CoronaPlugins'
    end
    if not params.ignoreMissingMarker then
        params.ignoreMissingMarker = 'IgnoreMissing'
    end

    local plugins = params.plugins or json.decode(params.buildData).plugins
    if type(plugins) ~= 'table' then return end

    local pluginPlatform = params.pluginPlatform
    local collectedPlugins = {}
    for plugin, pluginTable in pairs(plugins) do
        assert(type(plugin) == 'string', "Plugin is not a string")
        assert(type(pluginTable) == 'table', 'Invalid plugin table for ' .. plugin)
        local result = fetchSinglePlugin(dstDir, plugin, pluginTable, pluginPlatform, params, pluginLocators)
        if type(result) == 'string'  then
            if params.continueOnError then
                ret = (ret or "") .. result .. "\n"
            else
                return result
            end
        else  
            collectedPlugins[plugin] = true
        end
    end

    log("Collecting plugin dependencies")
    local unresolvedDeps = {}
    local allFetched
    repeat
        for plugin, pluginTable in pairs(unresolvedDeps) do
            log("Collecting dependency " .. plugin)
            local result = fetchSinglePlugin(dstDir, plugin, pluginTable, pluginPlatform, params, pluginLocators)
            if type(result) == 'string'  then
                if params.continueOnError then
                    ret = (ret or "") .. result .. "\n"
                else
                    return result
                end
            else  
                collectedPlugins[plugin] = true
            end    
        end
        unresolvedDeps = {}
        allFetched = true
        for plugin, _ in pairs(collectedPlugins)  do
            local pluginArc = dstDir .. '/' .. plugin .. '/data.tgz'
            local pluginDestination = params.extractLocation or (dstDir .. '/' .. plugin)
            local ret
            if params.extractLocation then
                if isWindows then
                    local cmd = '""%CORONA_PATH%\\7za.exe" x ' .. quoteString(pluginArc) .. ' -so  2> nul | "%CORONA_PATH%\\7za.exe" x -aoa -si -ttar -o' .. quoteString(params.extractLocation) .. ' 2> nul "'
                    ret = exec(cmd)
                else
                    ret = exec('/usr/bin/tar -xzf ' .. quoteString(pluginArc) .. ' -C ' .. quoteString(params.extractLocation) )
                end                
            else
                if isWindows then
                    local cmd = '""%CORONA_PATH%\\7za.exe" x ' .. quoteString(pluginArc) .. ' -so  2> nul | "%CORONA_PATH%\\7za.exe" x -aoa -si -ttar -o' .. quoteString(pluginDestination) .. ' metadata.lua  2> nul "'
                    ret = exec(cmd)
                else
                    ret = exec('/usr/bin/tar -xzf ' .. quoteString(pluginArc) .. ' -C ' .. quoteString(pluginDestination) .. ' metadata.lua')
                end  
            end
            if ret then
                local toDownload = {}
                local metadataFile = pluginDestination .. "/metadata.lua"
                pcall( function()
                    local metadata = dofile(metadataFile)
                    toDownload = metadata.coronaManifest.dependencies
                end	)
                os.remove(metadataFile)
                if type(toDownload) == 'table' then
                    for depPlugin, depDeveloper in pairs(toDownload) do
                        if not collectedPlugins[depPlugin] then
                            unresolvedDeps[depPlugin] = {publisherId=depDeveloper}
                            allFetched = false
                        end
                    end
                end
            end
        end
    until allFetched
    return ret
end


return {
    collect = CollectCoronaPlugins
}
