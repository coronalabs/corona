------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

processExecute = processExecute or os.execute

local lfs = require("lfs")
local json = require("json")
local http = require("socket.http")
local ltn12 = require("ltn12")
local debugBuildProcess = 0
local dirSeparator = package.config:sub(1,1)
local buildSettings = nil
local sFormat = string.format
local linuxBuilderPrefx = "Linux Builder:"

local function fileExists(name)
	local f = io.open(name, "r")

	if (f ~= nil) then
		io.close(f) return true
	else 
		return false 
	end
end

-- check if /usr/bin/tar exists, it is in Mac but not in Linux
local tar = "/usr/bin/tar"

if (fileExists(tar) == false) then
	tar = "tar"   -- for linux
end

local function printf(msg, ...)
	luaPrint(msg:format(...))
end

local function log(...)
	luaPrint(...)
end

local function log3(...)
	if (debugBuildProcess >= 3) then
		luaPrint(...)
	end
end

local function quoteString(str)
	str = str:gsub('\\', '\\\\')
	str = str:gsub('"', '\\"')

	return sFormat("\"%s\"", str)
end

local function dirExists(path)
    local cd = lfs.currentdir()
	local is = lfs.chdir(path) and true or false

	lfs.chdir(cd)
	
    return is
end

local function pathJoin(p1, p2, ... )
	local res
	local p1s = p1:sub(-1) == dirSeparator
	local p2s = p2:sub(1, 1) == dirSeparator
	
	if (p1s and p2s) then
		res = p1:sub(1,-2) .. p2
	elseif (p1s or p2s) then
		res = p1 .. p2
	else
		res = p1 .. dirSeparator .. p2
	end
	
	if ... then
		return pathJoin(res, ...)
	else
		return res
	end
end

local function extractTar(archive, dst)
	lfs.mkdir(dst)
	local cmd = tar .. ' -xzf ' .. quoteString(archive) .. ' -C ' ..  quoteString(dst .. "/") 
	--printf("extract tar cmd: %s", cmd)
	
	return os.execute(cmd)
end

local function gzip(path, appname, ext, destFile)
	local dst = pathJoin(path, destFile)
	local src = ''
	
	for i = 1, #ext do	
		src = src .. appname .. ext[i]
		src = src .. ' '
	end
	
	local cmd = 'cd '.. quoteString(path) .. ' && /usr/bin/zip "' .. dst .. '" ' .. src
	log3('gzip', cmd)
	os.execute(cmd)

	for i = 1, #ext do	
		os.remove(pathJoin(path, appname .. ext[i]))
	end
end

local function zip(folder, zipfile)
	local cmd = 'cd '.. folder .. ' && /usr/bin/zip -r -X ' .. zipfile .. ' ' .. '*'
	log3('zip:', cmd)
	
	return os.execute(cmd)
end

local function unzip(archive, dst)
	local cmd = '/usr/bin/unzip -o -q ' .. quoteString(archive) .. ' -d ' ..  quoteString(dst)
	log3('inzip:', cmd)
	
	return os.execute(cmd)
end

local function createTarGZ(srcDir, tarFile, tarGZFile)
	log('crerating', tarGZFile)

	local cmd = 'cd '.. quoteString(srcDir) .. ' && ' .. tar .. ' --exclude=' .. tarGZFile .. ' -czf ' .. tarGZFile .. ' .'
	log3('createTarGZ:', cmd)
	
	return os.execute(cmd)
end

local function setControlParams(args, localTmpDir)
	local path = pathJoin(localTmpDir, 'DEBIAN', 'control')
	local f = io.open(path, "rb")
	
	if (f) then
		local s = f:read("*a")
		io.close(f)

		local count
 		s, count = s:gsub('@package', args.applicationName, 1)
 		s, count = s:gsub('@version', args.versionName, 1)
 		s, count = s:gsub('@size', '10000', 1)		-- fixme
 		s, count = s:gsub('@maintainer', 'Corona Labs corp. <support@coronalabs.com>', 1)
 		s, count = s:gsub('@description', 'This is my app', 1)
 		s = s .. '  ' .. 'description1\n'
 		s = s .. '  ' .. 'description2\n'
 		s = s .. '  ' .. 'description3\n'

		f = io.open(path, "wb")

		if (f) then
			f:write(s)
			io.close(f)
		end
	end
end

-- create deb file
local function createDebArchive(debFile, srcDir)
	local fi = io.open(pathJoin(srcDir, debFile), "wb")
	
	if (fi) then
		fi:write('!<arch>\n')
		fi:write('debian-binary   1410122664  0     0     100644  4         `\n')
		fi:write('2.0\n')

		-- add control.tar.gz
		local path = pathJoin(srcDir, 'DEBIAN', 'control.tar.gz')
		local f = io.open(path, "rb")
		
		if (f) then
			local filesize = f:seek("end")
			local s = sFormat('%d', filesize)
			
			while s:len() < 10 do
				s = s .. ' '
			end
			
			fi:write('control.tar.gz  1410122664  0     0     100644  ' .. s .. '`\n')
			f:seek("set", 0)

			local buf = f:read("*a")
			fi:write(buf)

			if ((filesize % 2) ~= 0) then
				fi:write('\n')
			end

			f:close()
		end

		-- add data.tar.gz
		local path = pathJoin(srcDir, 'CONTENTS', 'data.tar.gz')
		local f = io.open(path, "rb")
		
		if (f) then
			local filesize = f:seek("end")
			local s = sFormat('%d', filesize)
			
			while s:len() < 10 do
				s = s .. ' '
			end
			
			fi:write('data.tar.gz     1410122664  0     0     100644  ' .. s .. '`\n')
			f:seek("set", 0)
	
			local buf = f:read("*a")
			fi:write(buf)

			if (filesize % 2) ~= 0 then
				fi:write('\n')
			end

			f:close()
		end

		fi:close()
	end
end

local function copyFile(src, dst)
	local fi = io.open(src, "rb")
	
	if (fi) then
		local buf = fi:read("*a")
		fi:close()
		fi = io.open(dst, "wb")

		if (fi) then
			fi:write(buf)
			fi:close()
			return true
		end

		log3('copyFile failed to write: ', src, dst)
		return false
	end

	log3('copyFile failed to read: ', src, dst)
	return false;
end

local function copyDir(src, dst)
	local cmd = 'cp -R ' .. quoteString(src) .. '/. ' ..  quoteString(dst)
	log3('copydir:', cmd)

	return os.execute(cmd)
end

local function removeDir(dir)
	local cmd = "rm -f -r " .. quoteString(dir)
	log3('removeDir:', cmd)

	os.execute(cmd)
end

local function loadTable(path)
	local loc = location

	if not location then
		loc = defaultLocation
	end

	local file, errorString = io.open(path, "r")

	if not file then
		return {}
	else
		local contents = file:read("*a")
		local t = json.decode(contents)
		io.close(file)

		return t
	end
end

local function saveTable(t, path)
	local loc = location

	if not location then
		loc = defaultLocation
	end

	local file, errorString = io.open(path, "w")

	if not file then
		print("File error: " .. errorString)
		return false
	else
		file:write(json.encode(t))
		io.close(file)

		return true
	end
end

local function linuxDownloadPlugins(pluginDestinationDir, forceDownload)
	if (type(buildSettings) ~= 'table') then
		-- no build.settings file, so no plugins to download
		return nil
	end

	local pluginMessagePrefix = "Linux Plugin Manager:"
	local publisherIDKey = "publisherId"
	local supportedPlatformsKey = "supportedPlatforms"
	local linuxKey = "linux"
	local linuxSimKey = "linux-sim"
	local androidKey = "android"
	local pluginBaseDir = sFormat("%s/.Solar2DPlugins", os.getenv("HOME"))
	local pluginCatalogPath = sFormat("%s/.Solar2D/Plugins/catalog.json", os.getenv("HOME"))
	local pluginCatalog = loadTable(pluginCatalogPath)
	local updateTime = 1800 -- 30 minutes

	local function downloadPlugin(options)
		local url = options.url
		local pluginName = options.pluginName
		local downloadDir = options.downloadDir
		local pluginPath = options.pluginPath
		local pluginCatalogKey = options.pluginCatalogKey
		local pluginArchiveDir = sFormat("%sdata.tgz", downloadDir)
		local isLuaPlugin = options.isLuaPlugin

		local function removeOldPlugin()
			if (fileExists(pluginPath)) then
				os.remove(pluginPath)
			end

			if (dirExists(downloadDir)) then
				os.execute(sFormat("rm -rf %s", downloadDir))
			end
		end

		removeOldPlugin()
		local success, reason = lfs.mkdir(downloadDir)

		if (not success) then
			printf("%s failed to create directory %s, reason: %s - Aborting", pluginMessagePrefix, downloadDir, reason)
			return
		end

		printf("%s trying to download %s from %s", pluginMessagePrefix, pluginName, url)
		
		local code, response = _download(url, pluginArchiveDir)

		if (code == 0 and fileExists(pluginArchiveDir)) then
			printf("%s downloaded %s successfully", pluginMessagePrefix, pluginName)

			local ret = extractTar(pluginArchiveDir, pluginDestinationDir)

			if (ret ~= 0) then
				return sFormat('%s failed to unpack plugin %s to %s - curl error code: %s', pluginMessagePrefix, pluginName, pluginDestinationDir, code)
			end

			if (fileExists(pluginPath)) then
				pluginCatalog[pluginCatalogKey] = {lastUpdate = os.time()}
				saveTable(pluginCatalog, pluginCatalogPath)
			end

			if (isLuaPlugin) then
				copyDir(sFormat("%s/.Solar2D/Plugins/lua_51/plugin", os.getenv("HOME")), sFormat("%s/.Solar2D/Plugins/plugin/", os.getenv("HOME")))
				removeDir(sFormat("%s/.Solar2D/Plugins/lua_51", os.getenv("HOME")))
			end
		else
			removeOldPlugin()
			printf("%s Failed to download %s. err: %s, %s", pluginMessagePrefix, pluginName, code, err)
		end
	end

	local function getPluginFromRepo(options)
		local pluginName = options.pluginName
		local publisherID = options.publisherID
		local downloadDir = options.downloadDir
		local pluginCatalogKey = options.pluginCatalogKey
		local url = "https://plugins.solar2d.com/plugins.json"
		local repoCatalogPath = sFormat("%s/.Solar2D/Plugins/repo.json", os.getenv("HOME"))

		printf("%s looking for %s in plugin repo", pluginMessagePrefix, pluginName)
		
		local code, response = _fetch(url)

		if (code == 0) then
			local file = io.open(repoCatalogPath, 'w')
			file:write(response)
			file:close()
		end

		if (fileExists(repoCatalogPath)) then
			printf("%s found catalog file", pluginMessagePrefix)
			local repoData = loadTable(repoCatalogPath)
			local publisherKey = "solar2d"
			local publisherTable = repoData["solar2d"][publisherID]

			if (publisherTable == nil) then
				publisherTable = repoData["coronalabs"][publisherID]
				publisherKey = "coronalabs"
			end

			local pluginTable = publisherTable[pluginName]

			if (pluginTable ~= nil) then
				printf("%s found plugin %s in plugin repo", pluginMessagePrefix, pluginName)

				print(json.prettify(pluginTable))
				print("------------------------")

				local pluginInfo = {
					name = pluginName,
					publisherID = publisherID,
				}

				for k, v in pairs(pluginTable) do
					if (k == "r") then
						pluginInfo.releaseVersion = v
					end

					if (type(v) == "table") then
						for buildKey, buildValue in pairs(v) do
							if (buildKey:gmatch("?(%d.%d)")) then
								pluginInfo.supportedBuildNumber = buildKey
							end

							if (type(buildValue) == "table") then
								for _, platform in pairs(buildValue) do

									if (platform == "lua") then
										pluginInfo.isLuaPlugin = true
									elseif (platform == androidKey) then
										pluginInfo.platformKey = platform
										pluginInfo.isAndroidPlugin = true
									elseif (platform == linuxKey or platform == linuxSimKey) then
										pluginInfo.platformKey = platform
										pluginInfo.isNativePlugin = true
									end
								end
							end
						end
					end
				end
				
				print(json.prettify(pluginInfo))
				local pluginPath = pluginDestinationDir
				local url = nil

				if (pluginInfo.isLuaPlugin) then
					url = sFormat("https://github.com/%s/%s-%s/releases/download/%s/%s-lua.tgz", publisherKey, publisherID, pluginName, pluginInfo.releaseVersion, pluginInfo.supportedBuildNumber)
					pluginPath = downloadDir .. pluginName:sub(8) .. ".lua"
				elseif (pluginInfo.isAndroidPlugin) then
					-- plugin stubs are in short supply for Linux, so we download the win32-sim stubs instead.
					url = sFormat("https://github.com/%s/%s-%s/releases/download/%s/%s-%s.tgz", publisherKey, publisherID, pluginName, pluginInfo.releaseVersion, pluginInfo.supportedBuildNumber, "win32-sim")
					pluginPath = downloadDir .. pluginName:sub(8) .. ".lua"
				elseif (pluginInfo.isNativePlugin) then
					url = sFormat("https://github.com/%s/%s-%s/releases/download/%s/%s-%s.tgz", publisherKey, publisherID, pluginName, pluginInfo.releaseVersion, pluginInfo.supportedBuildNumber, pluginInfo.platformKey)
					pluginPath = downloadDir .. pluginName:sub(8) .. ".so"
				end

				printf("%s plugin url from repo is: %s", pluginMessagePrefix, url)
				printf("%s plugin path: %s, plugin download dir: %s", pluginMessagePrefix, pluginPath, downloadDir)

				downloadPlugin(
				{
					url = url,
					pluginName = pluginName,
					pluginPath = pluginPath,
					downloadDir = downloadDir,
					pluginCatalogKey = pluginCatalogKey,
					isLuaPlugin = pluginInfo.isLuaPlugin
				})
			end
			--https://github.com/solar2d/provider-plugin.name/releases/download/version/SOLARVERSION-lua.tgz
			--https://github.com/solar2d/com.schroederapps-plugin.progressRing/releases/download/v1/2017.3032-lua.tgz
		end
	end

	-- gather the plugins
	if (type(buildSettings.plugins) == "table") then
		printf("%s gathering plugins", pluginMessagePrefix)

		for k, v in pairs(buildSettings.plugins) do
			if (type(v) == "table") then
				local pluginName = k
				local pluginPath = sFormat("%s/.Solar2D/Plugins/%s.so", os.getenv("HOME"), pluginName)

				for pluginKey, pluginValue in pairs(v) do
					if (pluginKey == publisherIDKey) then
						local publisherID = pluginValue
						local pluginCatalogKey = sFormat("%s/%s", publisherID, pluginName)
						local shouldDownloadPlugin = true

						-- only download the plugin again if it's time to refresh it
						if (not forceDownload and pluginCatalog[pluginCatalogKey] ~= nil) then
							if (pluginCatalog[pluginCatalogKey].lastUpdate ~= nil) then
								if (os.time() - pluginCatalog[pluginCatalogKey].lastUpdate < updateTime) then
									printf("%s not downloading %s again, it has only been %s seconds since it was last updated", pluginMessagePrefix, pluginName, os.time() - pluginCatalog[pluginCatalogKey].lastUpdate)
									shouldDownloadPlugin = false
								end
							end
						end

						-- only download the plugin again if it doesn't exist
						if (not forceDownload and not fileExists(pluginPath)) then
							shouldDownloadPlugin = true
						end
					
						-- look for the plugin
						local pluginDir = sFormat("%s/%s/%s/%s/data.tgz", pluginBaseDir, publisherID, pluginName, linuxKey)
						local otherPluginDir = sFormat("%s/%s/%s/%s/data.tgz", pluginBaseDir, publisherID, pluginName, linuxSimKey)
						local androidPluginDir = sFormat("%s/%s/%s/%s/data.tgz", pluginBaseDir, publisherID, pluginName, androidKey)
						local pluginDownloadDir = sFormat("%s/.Solar2D/Plugins/%s/", os.getenv("HOME"), pluginName)

						if (shouldDownloadPlugin) then
							-- check local plugin storage first
							if (fileExists(pluginDir)) then
								printf("%s found %s at path %s", pluginMessagePrefix, pluginName, pluginDir)
								local ret = extractTar(pluginDir, pluginDestinationDir)

								if (ret ~= 0) then
									return sFormat('%s failed to unpack plugin %s to %s - error: %s', pluginMessagePrefix, pluginName, pluginDestinationDir, ret)
								end

								pluginCatalog[pluginCatalogKey] = {lastUpdate = os.time()}
								saveTable(pluginCatalog, pluginCatalogPath)
							elseif (fileExists(otherPluginDir)) then
								printf("%s found %s at path %s", pluginMessagePrefix, pluginName, otherPluginDir)
								local ret = extractTar(otherPluginDir, pluginDestinationDir)

								if (ret ~= 0) then
									return sFormat('%s failed to unpack plugin %s to %s - error: %s', pluginMessagePrefix, pluginName, pluginDestinationDir, ret)
								end

								pluginCatalog[pluginCatalogKey] = {lastUpdate = os.time()}
								saveTable(pluginCatalog, pluginCatalogPath)
							-- plugin doesn't exist in local storage, look for it online
							else
								local supportedPlatforms = v[supportedPlatformsKey]
								printf("%s %s does not exist in local storage", pluginMessagePrefix, pluginName)

								if (supportedPlatforms ~= nil and type(supportedPlatforms) == "table") then
									local linuxTable = supportedPlatforms[linuxKey]

									if (linuxTable == nil) then
										linuxTable = supportedPlatforms[linuxSimKey]
									end

									if (linuxTable == nil) then
										linuxTable = supportedPlatforms[androidKey]
									end

									if (linuxTable ~= nil and type(linuxTable) == "table") then
										local url = linuxTable["url"]

										if (url ~= nil and type(url) == "string") then
											downloadPlugin(
											{
												url = url,
												pluginName = pluginName,
												pluginPath = pluginPath,
												downloadDir = pluginDownloadDir,
												pluginCatalogKey = pluginCatalogKey
											})
										end
									end
									-- search the repo for lua plugins
								else
									printf("%s time to check the plugin repo for %s plugin by %s", pluginMessagePrefix, pluginName, publisherID)
									getPluginFromRepo(
									{
										pluginName = pluginName,
										publisherID = publisherID,
										downloadDir = pluginDownloadDir,
										pluginCatalogKey = pluginCatalogKey
									})
								end
							end
						end
					end
				end
			end
		end
	end
end


local function getExcludePredecate()
	local excludes = 
	{
		".@(!(.|))", -- hidden files/folders
		"(.lu)$", -- lu files
		"(.lua)$", -- lua files
		"build.settings", -- build.settings
		"**.storyboardc", -- storyboard assets
		"**.xcassets", -- xcode assets
		"**AndroidResources", -- android resources
		"(.icns)$", -- ico files
		"(.ico)$", -- icns files
		"Icon.png", -- project icon
		"Icon-xxxhdpi.png", -- android icons
		"Icon-xxhdpi.png", -- android icons
		"Icon-xhdpi.png", -- android icons
		"Icon-hdpi.png", -- android icons
		"Icon-mdpi.png", -- android icons
		"Icon-ldpi.png", -- android icons
		"Banner-xhdpi.png", -- android tv banner
	}

	-- append 'all:' and 'linux:'
	if (buildSettings and buildSettings.excludeFiles) then
		if (buildSettings.excludeFiles.all) then
			-- append excludes from 'all:'
			local excl = buildSettings.excludeFiles.all

			for i = 1, #excl do	
				excludes[#excludes + 1] = excl[i]
			end
		end

		if (buildSettings.excludeFiles.linux) then
			-- append excludes from 'linux:'
			local excl = buildSettings.excludeFiles.linux

			for i = 1, #excl do	
				excludes[#excludes + 1] = excl[i]
			end
		end
	end

	return function(fileName)
		for i = 1, #excludes do
			local rc = fileName:match(excludes[i])
			
			if (rc ~= nil) then
				return true
			end
		end
	
		return false
	end
end

local function deleteUnusedFiles(srcDir, excludePredicate)
	local paths = {srcDir}
	local count = 0
	local dirCount = 0
	local fileList = {}
	local directoryList = {}

	local function scanFoldersRecursively(event)
		if (#paths == 0) then
			paths = nil
			
			for i = 1, #fileList do
				local file = fileList[i]

				if (excludePredicate(file)) then
					local result, reason = os.remove(file)

					if (result) then
						--printf("removed file at  %s", file)
					else
						printf("! couldn't remove file at %s", file) 
					end
				end
			end

			for i = 1, #directoryList do
				local dir = directoryList[i]
				
				if (excludePredicate(dir)) then
					--printf("removing directory: %s", dir)
					os.execute(sFormat('rm -rf "%s"', dir))
				end
			end

			fileList = nil
			directoryList = nil

			return
		end

		local fullPath = nil
		local attributes = nil

		for file in lfs.dir(paths[1]) do
			if (file ~= "." and file ~= "..") then
				fullPath = sFormat("%s/%s", paths[1], file)
				attributes = lfs.attributes(fullPath)

				if (attributes) then
					if (attributes.mode == "directory") then
						--print("file: " .. file .. " is directory")
						table.insert(paths, fullPath)
						dirCount = dirCount + 1
						directoryList[dirCount] = fullPath
					elseif (attributes.mode == "file") then
						count = count + 1
						fileList[count] = fullPath
					end
				end
			end
		end

		table.remove(paths, 1)
		scanFoldersRecursively()
	end

	scanFoldersRecursively()
end

local function getPathFromString(str)
	local pathIndexes = {}

	for i = 1, #str do
		if (str:sub(i, i) == "/") then
			pathIndexes[#pathIndexes + 1] = i
		end
	end

	return string.sub(str, 1, pathIndexes[#pathIndexes])
end

local function getLastPathComponent(str)
	local pathIndexes = {}

	for i = 1, #str do
		if (str:sub(i, i) == "/") then
			pathIndexes[#pathIndexes + 1] = i
		end
	end

	return string.sub(str, pathIndexes[#pathIndexes] + 1)
end

local function makeApp(arch, linuxAppFolder, template, args, templateName)
	-- sanity check
	local archivesize = lfs.attributes(template, "size")
	local templatePath = getPathFromString(template)

	if (archivesize == nil or archivesize == 0) then
		return sFormat('%s failed to open template: %s', linuxBuilderPrefx, template)
	end

	local ret = extractTar(template, linuxAppFolder)

	if (ret ~= 0) then
		return sFormat('%s failed to unpack template %s to %s - error: %s', linuxBuilderPrefx, template, linuxAppFolder, ret)
	end

	printf('%s unzipped %s to %s', linuxBuilderPrefx, template, linuxAppFolder)

	-- copy binary
	local binaryPath = sFormat("%s/%s", linuxAppFolder, "template_x64")
	printf("%s renaming binary from %s to %s", linuxBuilderPrefx, templateName, args.applicationName)
	os.rename(binaryPath, sFormat("%s/%s", linuxAppFolder , args.applicationName))

	-- dowmload plugins
	local pluginDownloadDir = pathJoin(args.tmpDir, "pluginDownloadDir")
	local pluginExtractDir = pathJoin(args.tmpDir, "pluginExtractDir")
	local binPlugnDir = pathJoin(pluginExtractDir, arch)
	local luaPluginDir = pathJoin(pluginExtractDir, 'lua', 'lua_51')

	if (dirExists(luaPluginDir)) then
		copyDir(luaPluginDir, pluginExtractDir)
	end
	
	if (dirExists(binPlugnDir)) then
		copyDir(binPlugnDir, linuxAppFolder)
		copyDir(binPlugnDir, linuxAppFolder)
	end
	
	-- gather files into appFolder
	ret = copyDir(args.srcDir, linuxAppFolder)

	if (ret ~= 0) then
		return sFormat("%s failed to copy %s to %s", linuxBuilderPrefx, args.srcDir, linuxAppFolder)
	end

	printf(sFormat("%s copied app files from %s to %s", linuxBuilderPrefx, args.srcDir, linuxAppFolder))

	-- compile .lua
	local rc = compileScriptsAndMakeCAR(args.linuxParams, linuxAppFolder, linuxAppFolder, linuxAppFolder)

	if (not rc) then
		return sFormat("%s failed to create resource.car file", linuxBuilderPrefx)
	end

	lfs.mkdir(pathJoin(linuxAppFolder, "Resources"))
	os.rename(pathJoin(linuxAppFolder, "resource.car"), pathJoin(linuxAppFolder, "Resources", "resource.car"))
	printf("%s created resource.car", linuxBuilderPrefx)

	-- copy default font
	local defaultFontPath = sFormat("%s/%s", templatePath, "FreeSans.ttf")
	copyFile(defaultFontPath, pathJoin(linuxAppFolder, "Resources", "FreeSans.ttf"))

	-- copy standard resources
	if (args.useWidgetResources) then
		for file in lfs.dir(templatePath) do
			if (file ~= "." and file ~= "..") then
				if (file:find("widget_")) then
					copyFile(pathJoin(templatePath, file), pathJoin(linuxAppFolder, "Resources", file))
				end
			end
		end

		printf("%s copied widget resources", linuxBuilderPrefx)
	end

	-- delete unused files
	deleteUnusedFiles(linuxAppFolder, getExcludePredecate())
	-- remove plugin dirs
	os.execute(sFormat('rm -rf "%s"', pluginDownloadDir))
	os.execute(sFormat('rm -rf "%s"', pluginExtractDir))
	-- remove empty folders
	os.execute(sFormat('find "%s" -type d -empty -delete', linuxAppFolder))
end

-- global script to call from C++
function linuxPackageApp(args)
	debugBuildProcess = args.debugBuildProcess

	local template = args.templateLocation
	local templateArm = template

	-- read settings
	local buildSettingsFile = pathJoin(args.srcDir, 'build.settings')
	local oldSettings = _G['settings']
	_G['settings'] = nil
	pcall( function() dofile(buildSettingsFile) end	)
	buildSettings = _G['settings']
	_G['settings'] = oldSettings

	local success = false;

	if (args.onlyGetPlugins) then
		local msg = linuxDownloadPlugins(sFormat("%s/.Solar2D/Plugins", os.getenv("HOME")))

		if (type(msg) == 'string') then
			return msg
		end
	else
		local startTime = os.time()
		templateArm = templateArm:gsub('template_x64.tgz', 'template_arm.tgz')
		printf("%s build started", linuxBuilderPrefx)
		--print(json.prettify(args))
		--printf("%s template: %s", linuxBuilderPrefx, getLastPathComponent(args.templateLocation))

		printf("%s template location: %s", linuxBuilderPrefx, getPathFromString(args.templateLocation))

		-- create app folder
		local linuxAppFolder = pathJoin(args.dstDir, args.applicationName)

		os.execute(sFormat('rm -rf "%s"', linuxAppFolder))

		local success = lfs.mkdir(getPathFromString(linuxAppFolder))
		success = lfs.mkdir(linuxAppFolder)
		if (not success) then
			return sFormat('%s failed to create app folder: %s', linuxBuilderPrefx, linuxAppFolder)
		end

		printf("%s created app folder: %s", linuxBuilderPrefx, linuxAppFolder)

		local pluginDownloadDir = pathJoin(args.tmpDir, "pluginDownloadDir")
		local pluginExtractDir = pathJoin(args.tmpDir, "pluginExtractDir")
		lfs.mkdir(pluginDownloadDir)
		lfs.mkdir(pluginExtractDir)
		local templateFilename = getLastPathComponent(template);

		local rc = makeApp('x86-64', linuxAppFolder, template, args, templateFilename:sub(1, templateFilename:len() - 4))

		if (rc ~= nil) then
			return rc
		end

		local msg = linuxDownloadPlugins(linuxAppFolder, true)

		if (type(msg) == 'string') then
			return msg
		end

		printf("%s build finished in %s seconds", linuxBuilderPrefx, os.difftime(os.time(), startTime))
	end

	return nil 
end
