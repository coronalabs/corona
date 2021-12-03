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
local windows = (dirSeparator == '\\')

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

function fileExists(name)
   local f = io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
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
	local cmd = tar .. ' -xzf ' .. quoteString(archive) .. ' -C ' ..  quoteString(dst .. dirSeparator) 
	--printf("extract tar cmd: %s", cmd)
	
	return os.execute(cmd)
end

local function gzip( path, appname, ext, destFile )
	local dst = pathJoin(path, destFile)
	if windows then
		local src = ''
		for i = 1, #ext do	
			src = src .. '"' .. pathJoin(path, appname .. ext[i]) .. '"'
			src = src .. ' '
		end
		local cmd = '""%CORONA_PATH%\\7za.exe" a -tzip "' .. dst .. '" ' ..  src
		processExecute(cmd);
	else
		local src = ''
		for i = 1, #ext do	
			src = src .. appname .. ext[i]
			src = src .. ' '
		end
		local cmd = 'cd '.. quoteString(path) .. ' && /usr/bin/zip "' .. dst .. '" ' .. src
		os.execute(cmd)
	end

	-- delete source files
	for i = 1, #ext do	
		os.remove(pathJoin(path, appname .. ext[i]))
	end
end

local function zip( folder, zipfile )
	if windows then
		local cmd = '""%CORONA_PATH%\\7za.exe" a ' .. zipfile .. ' ' ..  folder .. '/*"'
		return processExecute(cmd)
	else
		local cmd = 'cd '.. folder .. ' && /usr/bin/zip -r -X ' .. zipfile .. ' ' .. '*'
		return os.execute(cmd)
	end
end

local function unzip( archive, dst )
	if windows then
		local cmd = '""%CORONA_PATH%\\7za.exe" x -aoa "' .. archive .. '" -o"' ..  dst .. '"'
		return processExecute(cmd)
	else
		return os.execute('/usr/bin/unzip -o -q ' .. quoteString(archive) .. ' -d ' ..  quoteString(dst))
	end
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

local function copyDir( src, dst )
	if windows then
		local cmd = 'robocopy "' .. src .. '" ' .. '"' .. dst.. '" /e 2> nul'
		return processExecute(cmd)>7 and 1 or 0
	else
		local cmd = 'cp -R ' .. quoteString(src) .. '/. ' ..  quoteString(dst)
		return os.execute(cmd)
	end
end

local function removeDir( dir )
	if windows then
		local cmd = 'rmdir /s/q "' .. dir .. '"'
		return processExecute(cmd)
	else
		os.execute("rm -f -r " .. quoteString(dir))
	end
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

local function linuxDownloadPlugins(buildRevision, tmpDir, pluginDstDir, platform)
	if type(buildSettings) ~= 'table' then
		-- no build.settings file, so no plugins to download
		return nil
	end

	local collectorParams = { 
		pluginPlatform = 'linux',
		plugins = buildSettings.plugins or {},
		destinationDirectory = tmpDir,
		build = buildRevision,
		extractLocation = pluginDstDir,
		pluginStorage = pathJoin(os.getenv("HOME"), ".Solar2D")
	}
	
	local pluginCollector = require "CoronaBuilderPluginCollector"
	return pluginCollector.collect(collectorParams)
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
					removeDir(dir)
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
		if (str:sub(i, i) == dirSeparator) then
			pathIndexes[#pathIndexes + 1] = i
		end
	end

	return string.sub(str, 1, pathIndexes[#pathIndexes])
end

local function getLastPathComponent(str)
	local pathIndexes = {}

	for i = 1, #str do
		if (str:sub(i, i) == dirSeparator) then
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

	printf('%s unpacked %s to %s', linuxBuilderPrefx, template, linuxAppFolder)

	-- copy binary
	local binaryPath = sFormat("%s/%s", linuxAppFolder, "Solar2D")
	printf("%s renaming binary to %s", linuxBuilderPrefx, args.applicationName)
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
		copyDir(binPlugnDir, resourcesFolder)
		copyDir(binPlugnDir, resourcesFolder)
	end
	
	-- gather files into appFolder/Resources
	local resourcesFolder = pathJoin(linuxAppFolder, "Resources")
	lfs.mkdir(resourcesFolder)

	ret = copyDir(args.srcDir, resourcesFolder)
	if (ret ~= 0) then
		return sFormat("%s failed to copy %s to %s", linuxBuilderPrefx, args.srcDir, resourcesFolder)
	end
	printf(sFormat("%s copied app files from %s to %s", linuxBuilderPrefx, args.srcDir, resourcesFolder))

	-- compile .lua
	local rc = compileScriptsAndMakeCAR(args.linuxParams, resourcesFolder, resourcesFolder, resourcesFolder)

	if (not rc) then
		return sFormat("%s failed to create resource.car file", linuxBuilderPrefx)
	end
	printf("%s created resource.car", linuxBuilderPrefx)

	-- copy default font
	local defaultFontPath = sFormat("%s/%s", templatePath, "FreeSans.ttf")
	copyFile(defaultFontPath, pathJoin(resourcesFolder, "FreeSans.ttf"))

	-- copy standard resources
	if (args.useWidgetResources) then
		for file in lfs.dir(templatePath) do
			if (file ~= "." and file ~= "..") then
				if (file:find("widget_")) then
					copyFile(pathJoin(templatePath, file), pathJoin(resourcesFolder, file))
				end
			end
		end
		printf("%s copied widget resources", linuxBuilderPrefx)
	end

	-- delete unused files
	deleteUnusedFiles(resourcesFolder, getExcludePredecate())

	-- remove plugin dirs
	removeDir(pluginDownloadDir)
	removeDir(pluginExtractDir)

	-- remove empty folders
--fixme	os.execute(sFormat('find "%s" -type d -empty -delete', linuxAppFolder))
end

-- global script to call from C++
function linuxPackageApp(args)
	debugBuildProcess = tonumber(args.debugBuildProcess) or 0

	log('Linux builder started')
	log3(json.prettify(args))

	local template = args.templateLocation
	if template == nil and args.onlyGetPlugins == false then
			return 'No templateLocation'
	end

	-- read settings
	local buildSettingsFile = pathJoin(args.srcDir, 'build.settings')
	local oldSettings = _G['settings']
	_G['settings'] = nil
	pcall( function() dofile(buildSettingsFile) end	)
	buildSettings = _G['settings']
	_G['settings'] = oldSettings

	local success = false;

		-- dowmload plugins
	local pluginsFolder = sFormat("%s/.Solar2D/Plugins", os.getenv("HOME"))
	local pluginDownloadDir = sFormat("%s/.Solar2D/pluginDownloadDir", os.getenv("HOME")) -- pathJoin(args.tmpDir, "pluginDownloadDir")
	lfs.mkdir(pluginDownloadDir)
	lfs.mkdir(pluginsFolder)
	local msg = linuxDownloadPlugins(args.buildRevision, pluginDownloadDir, pluginsFolder)
	if type(msg) == 'string' then
		return msg
	end
	
	if (args.onlyGetPlugins) then
		return msg
	else 
		local startTime = os.time()

		-- create app folder
		local linuxAppFolder = pathJoin(args.dstDir, args.applicationName) .. '.Linux'
		if dirExists(linuxAppFolder) then
			removeDir(linuxAppFolder)
			log("Deleting existing directory " .. linuxAppFolder)
		end

		local ok,err = lfs.mkdir(linuxAppFolder)
		if not ok then
			return 'Failed to create app folder: ' .. linuxAppFolder
		end
		printf("%s App folder %s", linuxBuilderPrefx, linuxAppFolder)

		local templateFilename = getLastPathComponent(template);
		log3('templateFilename: ' .. templateFilename)

		local rc = makeApp('x86-64', linuxAppFolder, template, args, templateFilename:sub(1, templateFilename:len() - 4))

		if (rc ~= nil) then
			return rc
		end
		printf("%s build finished in %s seconds", linuxBuilderPrefx, os.difftime(os.time(), startTime))
	end
	
	log('Linux builder ended')
	return nil 
end
