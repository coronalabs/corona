------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

platformFallbacks = {
    { "appletvos", "tvos" },
    { "appletvsimulator", "tvos-sim" },
    { "android-kindle", "android" },
    { "android-nongoogle", "android" },
    { "macos", "mac-sim" },
    { "win32", "win32-sim" },
    { "linux", "linux-sim" },
    { "html5", "web" },
    { "*", "lua" },
}

if coronabaselib then print = coronabaselib.print end
json = require "json"
lfs = require "lfs"

SEP = package.config:sub(1,1)
isWindows = SEP == '\\'

function fetch() assert(false, "HTTP Fetch is not set!") end
function download() assert(false, "HTTP Download is not set!") end
locatorNames = {}

debugBuildProcess = tonumber(debugBuildProcess or os.getenv('DEBUG_BUILD_PROCESS') or "0")
function log(...)
    if debugBuildProcess > 0 then
        print(...)
    end
end

function pathJoin(p1, p2, ... )
	local res
	local p1s = p1:sub(-1) == SEP
	local p2s = p2:sub(1, 1) == SEP
	if p1s and p2s then
		res = p1:sub(1,-2) .. p2
	elseif p1s or p2s then
		res = p1 .. p2
	else
		res = p1 .. SEP .. p2
	end
	if ... then
		return pathJoin(res, ...)
	else
		return res
	end
end


function isDir(f)
    return lfs.attributes(f, 'mode') == 'directory'
end

function isFile(f)
    return lfs.attributes(f, 'mode') == 'file'
end

function quoteString( str )
    if not isWindows then
        str = str:gsub('\\', '\\\\')
        str = str:gsub('"', '\\"')
    end
    return "\"" .. str .. "\""
end

function mkdirs(path)
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

function exec(cmd)
    log('Running command', cmd)
    if debugBuildProcess < 1 then
        if isWindows then
        else
            cmd = cmd .. ' &> /dev/null'
        end
    end
    local ret = (0 == os.execute2(cmd))
    return ret
end

function copyFile(src, dst)
    -- note that dst can be directory or file
    if isWindows then
        exec('copy ' .. quoteString(src) .. ' ' .. quoteString(dst))
    else
        exec('/bin/cp ' .. quoteString(src) .. ' ' .. quoteString(dst))
    end
end

-- Default Plugin Collectors

local PluginCollectorSolar2DDirectory =  { name = "Solar2D Free Directory"}
function PluginCollectorSolar2DDirectory:init(params)

    local directoryPluginsText, msg = fetch("https://plugins.solar2d.com/plugins.json")
    if not directoryPluginsText then
        log("Solar2D Directory: error initializing directory " .. tostring(msg))
		return
	end
	local directoryPlugins = json.decode( directoryPluginsText )
	if type(directoryPlugins) ~= "table" then
		return
    end

    self.pluginsCache = {}

    for repoOwner, repoObject in pairs(directoryPlugins) do
        for providerName,providerObject  in pairs(repoObject) do
            for pluginName, pluginObject in pairs(providerObject) do
                self.pluginsCache[providerName .. SEP .. pluginName] = {repo = repoOwner, plugin = pluginObject}
            end
        end
	end
	return true
end

function PluginCollectorSolar2DDirectory:collect(destination, plugin, pluginTable, pluginPlatform, params)
    if not self.pluginsCache then
        return "Solar2D Directory: directory was not fetched"
    end
    local pluginEntry = self.pluginsCache[tostring(pluginTable.publisherId) .. SEP .. plugin]
    if not pluginEntry then
        return "Solar2D Directory: plugin " .. plugin .. " was not found at Solar2D Directory"
    end

    local pluginObject = pluginEntry.plugin
    local repoOwner = pluginEntry.repo
    if pluginObject.e then
        return "! " .. pluginObject.e
    end

    local build = tonumber(params.build)
    local vFoundBuid, vFoundObject, vFoundBuildName
    for entryBuild, entryObject in pairs(pluginObject.v or {}) do
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
        if params.canSkip then
            log("Solar2D Directory: skipped plugin " .. plugin .. " because platform " .. pluginPlatform .. " is not supported")
        end
        return params.canSkip or "Solar2D Directory: skipped plugin " .. plugin .. " because platform " .. pluginPlatform .. " is not supported"
    end
    local repoName = pluginObject.p or (pluginTable.publisherId .. '-' .. plugin)
    local downloadURL = "https://github.com/" .. repoOwner .. "/" .. repoName .. "/releases/download/" .. pluginObject.r .. "/" .. vFoundBuildName .. "-" .. pluginPlatform .. ".tgz"

    local cacheDir = pathJoin(params.pluginStorage, "Caches", "Solar2Directory", repoOwner, pluginTable.publisherId, plugin, pluginPlatform )
    local cacheUrlFile = pathJoin(cacheDir, "info.txt")
    local cacheDestFile = pathJoin(cacheDir, "data.tgz")
    local validCache = false
    if isFile(cacheUrlFile) and isFile(cacheDestFile) then
        local f = io.open(cacheUrlFile, "rb")
        if f then
            validCache = f:read("*all") == downloadURL
            f:close()
        end
    end
    if not validCache then
        mkdirs(cacheDir)
        local result, err = download(downloadURL, cacheDestFile)
        if not result then
            return "Solar2D Directory: unable to download " .. plugin .. '. Code: ' .. err
        end
        local f = io.open(cacheUrlFile, "wb")
        if f then
            f:write(downloadURL)
            f:close()
        end
    else
        log("Solar2D Directory: cache hit " .. plugin)
    end

    mkdirs(destination)
    copyFile(cacheDestFile, destination)
    return true
end


-- Solar2d Marketplace Collector
local PluginCollectorSolar2DMarketplaceDirectory =  { name = "Solar2d Marketplace Directory"}

function PluginCollectorSolar2DMarketplaceDirectory:lazyInit()
    if self.lazyInitDone then return end
    self.lazyInitDone = true

    local directoryPluginsText, msg = fetch("https://solar2dmarketplace.com/getAllPlugins")
    if not directoryPluginsText then
        log("Solar2D Marketplace: error initializing directory " .. tostring(msg))
		return
	end
	local directoryPlugins = json.decode( directoryPluginsText )
	if type(directoryPlugins) ~= "table" then
		return
    end

    self.pluginsCache = {}

    for repoOwner, repoObject in pairs(directoryPlugins) do
        for providerName,providerObject  in pairs(repoObject) do
            for pluginName, pluginObject in pairs(providerObject) do
                self.pluginsCache[providerName .. SEP .. pluginName] = {repo = repoOwner, plugin = pluginObject}
            end
        end
	end
end

function PluginCollectorSolar2DMarketplaceDirectory:init(params)
	return true
end

function PluginCollectorSolar2DMarketplaceDirectory:collect(destination, plugin, pluginTable, pluginPlatform, params)
    if not pluginTable.marketplaceId then
        return "Solar2D Marketplace Directory: skipped because marketplaceId is not set"
    end

    self:lazyInit()
    if not self.pluginsCache then
        return "Solar2D Marketplace Directory: directory was not fetched"
    end

    local pluginEntry = self.pluginsCache[tostring(pluginTable.publisherId) .. SEP .. plugin]
    if not pluginEntry then
        return "Solar2D Marketplace Directory: plugin " .. plugin .. " was not found at Solar2D Marketplace Directory"
    end

    local pluginObject = pluginEntry.plugin
    local repoOwner = pluginEntry.repo
    if pluginObject.e then
        return "! " .. pluginObject.e
    end

    local build = tonumber(params.build)
    local vFoundBuid, vFoundObject, vFoundBuildName
    local pluginVersion = pluginObject.r
    for entryBuild, entryObject in pairs(pluginObject.v or {}) do
        local entryBuildNumber = tonumber(entryBuild:match('^%d+%.(%d+)$'))
        if entryBuildNumber <= build and entryBuildNumber > (vFoundBuid or 0) then
            vFoundBuid = entryBuildNumber
            vFoundObject = entryObject
            vFoundBuildName = entryBuild
        end
    end
    if not vFoundBuid then
        return "Solar2D Marketplace Directory: unable to find compatible version for " .. plugin .. "."
    end
    local hasPlatform = false
    for i=1,#vFoundObject do
        hasPlatform = hasPlatform or vFoundObject[i] == pluginPlatform
    end
    if not hasPlatform then
        if params.canSkip then
            log("Solar2D Marketplace Directory: skipped plugin " .. plugin .. " because platform " .. pluginPlatform .. " is not supported")
        end
        return params.canSkip or "Solar2D Marketplace Directory: skipped plugin " .. plugin .. " because platform " .. pluginPlatform .. " is not supported"
    end
    local repoName = pluginObject.p or (pluginTable.publisherId .. '-' .. plugin)
    local downloadURL

    if pluginTable.marketplaceId then
      downloadURL = "https://solar2dmarketplace.com/marketplacePlugins?pluginType=collector&ID=" .. pluginTable.marketplaceId .. "&plugin=" .. plugin .. "_" .. pluginTable.publisherId .. "&type=" .. pluginPlatform.."&version="..pluginVersion
    else
      return "Solar2D Marketplace Directory: skipped plugin marketplaceId not found"
    end

    local cacheDir = pathJoin(params.pluginStorage, "Caches", "Solar2DMarketplaceDirectory", repoOwner, pluginTable.publisherId, plugin, pluginPlatform )
    local cacheUrlFile = pathJoin(cacheDir, "info.txt")
    local cacheDestFile = pathJoin(cacheDir, "data.tgz")
    local validCache = false
    if isFile(cacheUrlFile) and isFile(cacheDestFile) then
        local f = io.open(cacheUrlFile, "rb")
        if f then
            validCache = f:read("*all") == downloadURL
            f:close()
        end
    end


    if not validCache then
        mkdirs(cacheDir)
        local result, err = download(downloadURL, cacheDestFile)
        if not result then
            return "Solar2D Marketplace Directory: unable to download " .. plugin .. '. Code: ' .. err
        end
        local f = io.open(cacheUrlFile, "wb")
        if f then
            f:write(downloadURL)
            f:close()
        end
    else
        log("Solar2D Marketplace Directory: cache hit " .. plugin)
    end

    mkdirs(destination)
    copyFile(cacheDestFile, destination)
    return true
end


local function pluginLocatorCustomURL(destination, plugin, pluginTable, pluginPlatform, params)
    if type(pluginTable.supportedPlatforms) ~= 'table' then
        return "Custom URL: skipped because no table supportedPlatforms provided for " .. plugin
    end
    local canSkip = false
    if params.canSkip then
        canSkip = false
        for platform, ptable in pairs(pluginTable.supportedPlatforms) do
            if type(ptable) == "table" and type(ptable.url) == "string" then
                canSkip = true
            end
        end
    end

    if type(pluginTable.supportedPlatforms[pluginPlatform]) ~= 'table' then
        if canSkip then
            log("Custom URL: skipped because supportedPlatforms[" .. pluginPlatform .. "] is not a table. Plugin is not supported by the platform")
        end
        return canSkip or "Custom URL: skipped because supportedPlatforms[" .. pluginPlatform .. "] is not a table. Plugin is not supported by the platform"
    end
    if type(pluginTable.supportedPlatforms[pluginPlatform].url) ~= 'string' then
        if canSkip then
            log("Custom URL: skipped because supportedPlatforms[" .. pluginPlatform .. "].url is not a string")
        end
        return canSkip or "Custom URL: skipped because supportedPlatforms[" .. pluginPlatform .. "].url is not a string"
    end
    local downloadURL = pluginTable.supportedPlatforms[pluginPlatform].url
    mkdirs(destination)
    local file, err = download(downloadURL, pathJoin(destination, 'data.tgz'))
    if not file then
        return "Custom URL: unable to download " .. tostring(plugin) .. '. Code: ' .. tostring(err) .. 'Destination: ' .. tostring(destination) .. "; URL: " .. tostring(downloadURL)
    end

    return true
end
locatorNames[pluginLocatorCustomURL] = "Custom URL Locator"

local function pluginLocatorFileSystemVersionized(destination, plugin, pluginTable, pluginPlatform, params)
    if type(pluginTable.publisherId) ~= 'string' then
        return "Locally: plugin has no string publisherId"
    end
    local pluginStorage = params.pluginStorage
    local pluginDir = pathJoin(pluginStorage, pluginTable.publisherId, plugin)
    if not isDir(pluginDir) then
        return "Locally: no directory " .. pluginDir
    end
    local targetBuild = tonumber(params.build)
    local lastFound = -1
    local foundDir
    for file in lfs.dir(pluginDir) do
        if file ~= "." and file ~= ".." then
            local f = pathJoin(pluginDir, file)
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
    local localPath = pathJoin(foundDir, pluginPlatform, 'data.tgz')
    if isFile(localPath) then
        mkdirs(destination)
        copyFile(localPath, destination)
        return true
    else
        if params.canSkip then
            log("Locally: skipped plugin because platform is not supported")
            return true
        else
            return "Locally: plugin file not found: " .. localPath
        end
    end
    return false
end
locatorNames[pluginLocatorFileSystemVersionized] = "Local File System Versionized"

local function pluginLocatorFileSystemAllPlatforms(destination, plugin, pluginTable, pluginPlatform, params)
    if type(pluginTable.publisherId) ~= 'string' then
        return "Locally: plugin has no string publisherId"
    end
    local pluginStorage = params.pluginStorage
    local localPath = pathJoin(pluginStorage, pluginTable.publisherId, plugin, 'data.tgz')
    if isFile(localPath) then
        mkdirs(destination)
        copyFile(localPath, destination)
        return true
    else
        return "Locally: no file '".. localPath .. "'"
    end
end
locatorNames[pluginLocatorFileSystemAllPlatforms] = "Local File System Per Platform Versionized"

local function pluginLocatorFileSystem(destination, plugin, pluginTable, pluginPlatform, params)
    if type(pluginTable.publisherId) ~= 'string' then
        return "Locally: plugin has no string publisherId"
    end
    local pluginStorage = params.pluginStorage
    local localPath = pathJoin(pluginStorage, pluginTable.publisherId, plugin, pluginPlatform, 'data.tgz')
    if isFile(localPath) then
        mkdirs(destination)
        copyFile(localPath, destination)
        return true
    elseif isDir(pathJoin(pluginStorage, pluginTable.publisherId, plugin)) and params.canSkip then
        log('Local lookup determined that plugin ' .. plugin .. ' is not supported by the platform ' ..  pluginPlatform)
        return true
    else
        return "Locally: no file '".. localPath .. "'"
    end
end
locatorNames[pluginLocatorFileSystem] = "Local File System Per Platform"

local function pluginLocatorIgnoreMissing(destination, plugin, pluginTable, pluginPlatform, params)
    if type(pluginTable.publisherId) ~= 'string' then
        return "Locally: plugin has no string publisherId"
    end
    local function hasIgnoreMissingMarker(path)
        return isFile(pathJoin(path, params.ignoreMissingMarker))
    end
    local pluginStorage = params.pluginStorage
    local pluginDir = pathJoin(pluginStorage, pluginTable.publisherId, plugin)
    if not isDir(pluginDir) then
        return "Ignore Missing checker: no plugin directory " .. pluginDir
    end
    if hasIgnoreMissingMarker(pluginDir) then
        return true
    end
    if hasIgnoreMissingMarker(pathJoin(pluginDir, pluginPlatform)) then
        return true
    end

    local targetBuild = tonumber(params.build)
    local lastFound = -1
    local foundDir
    for file in lfs.dir(pluginDir) do
        if file ~= "." and file ~= ".." then
            local f = pathJoin(pluginDir, file)
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
    if hasIgnoreMissingMarker(foundDir) or hasIgnoreMissingMarker(pathJoin(foundDir, pluginPlatform)) then
        return true
    end
    return "Ignore Missing checker: did not find marker in " .. foundDir
end
locatorNames[pluginLocatorIgnoreMissing] = "Missing Marker"

-- Funcitonality
local function locatorName(l)
    if locatorNames[l] then return locatorNames[l] end
    if type(l) == 'table' and l.name then return l.name end
end

local function fetchSinglePluginNoFallbacks(dstDir, plugin, pluginTable, pluginPlatform, params, pluginLocators, canSkip)
    params.canSkip = canSkip
    local pluginDestination = pathJoin(dstDir, plugin)
    local err = "Unable to find plugin '" .. plugin .. "' for platform '" .. pluginPlatform .. "':"
    local results = {}
    local ok =  false
    for i = 1,#pluginLocators do
        local locator = pluginLocators[i]
        local success, result
        if type(locator) == 'table' then
            if not locator.initialized and type(locator.init) == 'function' then
                success, result = pcall(locator.init, locator, params)
                if not success then
                    print("WARNING: error initializing plugin locator " .. (locatorName(locator) or "<unknown>") .. ": " .. tostring(result))
                end
                locator.initialized = true
            end
            success, result = pcall(locator.collect, locator, pluginDestination, plugin, pluginTable, pluginPlatform, params)
        else
            success, result = pcall(locator, pluginDestination, plugin, pluginTable, pluginPlatform, params)
        end
        if not success then
            print("WARNING: runtime error while executing plugin locator " .. (locatorName(locator) or "<unknown>") .. ": " .. tostring(result))
        end
        if success and result == true then
            log("Located " .. plugin .. " with locator " .. (locatorName(locator) or "<unknown>"))
            ok = true
            break
        elseif type(result) == 'string' then
            results[#results+1] = result
        end
    end
    if not ok then
        for i = 1,#results do
            local result = results[i]
            if result:sub(1,2) == "! " then
                return result
            end
            err = err .. '\n\t' .. result
        end
        return err
    end
end

local function fetchSinglePlugin(dstDir, plugin, pluginTable, basePluginPlatform, params, pluginLocators)
    local fallbackChain = {basePluginPlatform, }
    for i = 1, #platformFallbacks do
        local base, fallback = unpack(platformFallbacks[i])
        if (base == "*" or base == basePluginPlatform) and fallback ~= basePluginPlatform then
            fallbackChain[#fallbackChain+1] = fallback
        end
    end
    local numFallbacks = #fallbackChain

    if type(pluginTable.supportedPlatforms) == 'table' then
        local skip = true
        local skipped = ""
        for i = 1, numFallbacks do
            if pluginTable.supportedPlatforms[fallbackChain[i]] then
                skip = false
            else
                skipped = skipped .. " " .. fallbackChain[i]
            end
        end
        if skip then
            log("Skipping plugin " .. plugin .. " because supportedPlatforms is not set for any of" .. skipped)
            return
        end
    end

    local results = {}
    for i = 1, numFallbacks do
        local fallbackRes = fetchSinglePluginNoFallbacks(dstDir, plugin, pluginTable, fallbackChain[i], params, pluginLocators, i == numFallbacks)
        if not fallbackRes then return end -- success
        results[#results+1] = fallbackRes
    end
    local res
    for i = 1,#results do
        local result = results[i]
        if result:sub(1,2) == "! " then
            return result
        end
        if res then
            res = res .. "\n" .. result
        else
            res = result
        end
    end
    return res
end

local function mergeMoveDirectory(src, dst)
    for file in lfs.dir(src) do
        if file ~= "." and file ~= ".." then
            local srcFile = pathJoin(src, file)
            local dstFile = pathJoin(dst, file)
            if isDir(srcFile) and isDir(dstFile) then
                mergeMoveDirectory(srcFile, dstFile)
            else
                os.rename(srcFile, dstFile)
            end
        end
    end
    exec("rmdir " .. quoteString(src))
end

local function CollectCoronaPlugins(params)
    if type(params) == "string" then params = json.decode(params) end
    fetch = params.fetch or pluginCollector_fetch
    download = params.download or pluginCollector_download
    params.fetch = nil
    pluginCollector_fetch = nil
    params.download = nil
    pluginCollector_download = nil

    local ret = nil

    local pluginLocators = { pluginLocatorCustomURL, pluginLocatorFileSystemVersionized, pluginLocatorFileSystem, pluginLocatorFileSystemAllPlatforms, PluginCollectorSolar2DMarketplaceDirectory, PluginCollectorSolar2DDirectory, pluginLocatorIgnoreMissing }

    local dstDir = params.destinationDirectory

    params.pluginLocators = pluginLocators
    if not params.pluginStorage then
        params.pluginStorage = pathJoin(os.getenv("HOME") or os.getenv("APPDATA"), 'Solar2DPlugins')
    end
    if not params.ignoreMissingMarker then
        params.ignoreMissingMarker = 'IgnoreMissing'
    end
    params.customCollectorsDir = pathJoin(params.pluginStorage, "Collectors")
    if isDir(params.customCollectorsDir) then
        local pathBackup = package.path
        package.path = pathJoin(params.customCollectorsDir, "?.lua") .. ';' .. package.path
        for file in lfs.dir(params.customCollectorsDir) do
            local module = file:match("(.*)%.lua")
            if module then
                local err, module = pcall(require, module)
                if type(module) == 'function' then
                    table.insert(params.pluginLocators, #params.pluginLocators, module)
                elseif type(module) == 'table' and type(module.collect) == 'function' then
                    table.insert(params.pluginLocators, #params.pluginLocators, module)
                else
                    print("WARNING: error importing plugin locator " .. file .. ": " .. tostring(module))
                end
            end
        end
        package.path = path
    end

    debugBuildProcess = tonumber(debugBuildProcess) or 0
    if debugBuildProcess > 0 then
        local copyLocators = {}
        for i=1,#params.pluginLocators do
            copyLocators[params.pluginLocators[i]] = true
        end
        log("Collecting plugins", json.encode(params, {
            tables = copyLocators,
            exception = function(reason, value, state, defaultmessage)
                local cn = locatorName(value)
                if cn then return "<Collector: " .. cn .. ">" end
                return quoteString("<" .. defaultmessage .. ">")
            end
        }))
    end

    local plugins = params.plugins or json.decode(params.buildData).plugins
    if type(plugins) ~= 'table' then return end

    local pluginPlatform = params.pluginPlatform
    local collectedPlugins = {}
    for plugin, pluginTable in pairs(plugins) do
        if type(plugin) ~= 'string' then  return "Plugin is not a string" end
        if type(pluginTable) ~= 'table' then return 'Invalid plugin table for ' .. plugin end
        local result = fetchSinglePlugin(dstDir, plugin, pluginTable, pluginPlatform, params, pluginLocators)
        if type(result) == 'string'  then
            if result:sub(1,2) == "! " then
                result = result:sub(3)
            end
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
                if result:sub(1,2) == "! " then
                    result = result:sub(3)
                end
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
            local pluginArc = pathJoin(dstDir, plugin, 'data.tgz')
            local pluginDestination = params.extractLocation or pathJoin(dstDir, plugin)
            local ret = false
            if isFile(pluginArc) then
                if params.extractLocation then
                    if isWindows then
                        local cmd = '""%CORONA_PATH%\\7za.exe" x ' .. quoteString(pluginArc) .. ' -so  2> nul | "%CORONA_PATH%\\7za.exe" x -aoa -si -ttar -o' .. quoteString(params.extractLocation) .. ' 2> nul "'
                        ret = exec(cmd)
                    else
                        ret = exec('/usr/bin/tar -xzf ' .. quoteString(pluginArc) .. ' -C ' .. quoteString(params.extractLocation) )
                    end
                    local lua51Dir = pathJoin(params.extractLocation, "lua_51")
                    if ret and isDir(lua51Dir) then
                        mergeMoveDirectory(lua51Dir, params.extractLocation)
                    end
                else
                    if isWindows then
                        local cmd = '""%CORONA_PATH%\\7za.exe" x ' .. quoteString(pluginArc) .. ' -so  2> nul | "%CORONA_PATH%\\7za.exe" x -aoa -si -ttar -o' .. quoteString(pluginDestination) .. ' metadata.lua  2> nul "'
                        ret = exec(cmd)
                    else
                        ret = exec('/usr/bin/tar -xzf ' .. quoteString(pluginArc) .. ' -C ' .. quoteString(pluginDestination) .. ' metadata.lua')
                    end
                end
            end
            if ret then
                local toDownload = {}
                local metadataFile = pathJoin(pluginDestination, "metadata.lua")
                if isFile(metadataFile) then
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
        end
    until allFetched
    return ret
end


return {
    collect = CollectCoronaPlugins
}
