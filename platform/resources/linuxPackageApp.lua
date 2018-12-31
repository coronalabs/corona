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

linuxPluginsMetadata = {}
processExecute = processExecute or os.execute

local lfs = require "lfs"
local json = require "json"
local http = require( "socket.http" )
local debugBuildProcess = 0

local serverBackend = 'https://backendapi.coronalabs.com'
local dirSeparator = package.config:sub(1,1)
local windows = (dirSeparator == '\\')
local buildSettings = nil

function file_exists(name)
   local f = io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
end

-- check if /usr/bin/tar exists, it is in Mac but not in Linux
local tar = "/usr/bin/tar"
if file_exists(tar) == false then
	tar = "tar"   -- for linux
end

local function log(...)
	myprint(...)
end

local function log3(...)
	if debugBuildProcess >= 3 then
		myprint(...)
	end
end

local function quoteString( str )
	str = str:gsub('\\', '\\\\')
	str = str:gsub('"', '\\"')
	return "\"" .. str .. "\""
end

function dir_exists(path)
    local cd = lfs.currentdir()
    local is = lfs.chdir(path) and true or false
    lfs.chdir(cd)
    return is
end

function globToPattern(g)
  local p = "^"  -- pattern being built
  local i = 0    -- index in g
  local c        -- char at index i in g.

  -- unescape glob char
  local function unescape()
    if c == '\\' then
      i = i + 1; c = g:sub(i,i)
      if c == '' then
        p = '[^]'
        return false
      end
    end
    return true
  end

  -- escape pattern char
  local function escape(c)
    return c:match("^%w$") and c or '%' .. c
  end

  -- Convert tokens at end of charset.
  local function charset_end()
    while 1 do
      if c == '' then
        p = '[^]'
        return false
      elseif c == ']' then
        p = p .. ']'
        break
      else
        if not unescape() then break end
        local c1 = c
        i = i + 1; c = g:sub(i,i)
        if c == '' then
          p = '[^]'
          return false
        elseif c == '-' then
          i = i + 1; c = g:sub(i,i)
          if c == '' then
            p = '[^]'
            return false
          elseif c == ']' then
            p = p .. escape(c1) .. '%-]'
            break
          else
            if not unescape() then break end
            p = p .. escape(c1) .. '-' .. escape(c)
          end
        elseif c == ']' then
          p = p .. escape(c1) .. ']'
          break
        else
          p = p .. escape(c1)
          i = i - 1 -- put back
        end
      end
      i = i + 1; c = g:sub(i,i)
    end
    return true
  end

  -- Convert tokens in charset.
  local function charset()
    i = i + 1; c = g:sub(i,i)
    if c == '' or c == ']' then
      p = '[^]'
      return false
    elseif c == '^' or c == '!' then
      i = i + 1; c = g:sub(i,i)
      if c == ']' then
        -- ignored
      else
        p = p .. '[^'
        if not charset_end() then return false end
      end
    else
      p = p .. '['
      if not charset_end() then return false end
    end
    return true
  end

  -- Convert tokens.
  while 1 do
    i = i + 1; c = g:sub(i,i)
    if c == '' then
      p = p .. '$'
      break
    elseif c == '?' then
      p = p .. '.'
    elseif c == '*' then
      p = p .. '.*'
    elseif c == '[' then
      if not charset() then break end
    elseif c == '\\' then
      i = i + 1; c = g:sub(i,i)
      if c == '' then
        p = p .. '\\$'
        break
      end
      p = p .. escape(c)
    else
      p = p .. escape(c)
    end
  end
  return p
end

local function pathJoin(p1, p2, ... )
	local res
	local p1s = p1:sub(-1) == dirSeparator
	local p2s = p2:sub(1, 1) == dirSeparator
	if p1s and p2s then
		res = p1:sub(1,-2) .. p2
	elseif p1s or p2s then
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

local function unpackPlugin( archive, dst, tmpDir, plugin)
	if windows then
--		pipe not working	
--		local cmd = '""%CORONA_PATH%\\7za.exe" x "' .. archive .. '" -so  2> nul | "%CORONA_PATH%\\7za.exe" x -aoa -si -ttar -o"' .. dst .. '" 2> nul"'
		local cmd = '""%CORONA_PATH%\\7za.exe" x "' .. archive .. '" -o"' .. tmpDir .. '"'
		log3('unpackPlugin:', cmd)
		processExecute(cmd)
		local cmd = '""%CORONA_PATH%\\7za.exe" x "' .. pathJoin(tmpDir, plugin .. ".tar") .. '" -o"' .. dst .. '"'
		log3('unpackPlugin:', cmd)
		return processExecute(cmd)
	else
		lfs.mkdir(dst)
		local cmd = tar .. ' -xzf ' .. quoteString(archive) .. ' -C ' ..  quoteString(dst)
		log3('unpackPlugin:', cmd)
		return os.execute(cmd)
	end
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
		log3('gzip', cmd)
		processExecute(cmd);
	else
		local src = ''
		for i = 1, #ext do	
			src = src .. appname .. ext[i]
			src = src .. ' '
		end
		local cmd = 'cd '.. quoteString(path) .. ' && /usr/bin/zip "' .. dst .. '" ' .. src
		log3('gzip', cmd)
		os.execute(cmd)
	end

	for i = 1, #ext do	
		os.remove(pathJoin(path, appname .. ext[i]))
	end
end

local function zip( folder, zipfile )
	if windows then
		local cmd = '""%CORONA_PATH%\\7za.exe" a ' .. zipfile .. ' ' ..  folder .. '/*"'
		log3('zip:', cmd)
		return processExecute(cmd)
	else
		local cmd = 'cd '.. folder .. ' && /usr/bin/zip -r -X ' .. zipfile .. ' ' .. '*'
		log3('zip:', cmd)
		return os.execute(cmd)
	end
end

local function unzip( archive, dst )
	if windows then
		local cmd = '""%CORONA_PATH%\\7za.exe" x -aoa "' .. archive .. '" -o"' ..  dst .. '"'
		log3('zip:', cmd)
		return processExecute(cmd)
	else
		local cmd = '/usr/bin/unzip -o -q ' .. quoteString(archive) .. ' -d ' ..  quoteString(dst)
		log3('inzip:', cmd)
		return os.execute(cmd)
	end
end

local function createTarGZ(srcDir, tarFile, tarGZFile)
  log('crerating', tarGZFile)
	if windows then
		local cmd = '""%CORONA_PATH%\\7za.exe" a -ttar "' .. pathJoin(srcDir, tarFile) .. '" "' .. srcDir .. '\\*""'
		processExecute(cmd)
		local cmd = '""%CORONA_PATH%\\7za.exe" a "' .. pathJoin(srcDir, tarGZFile) .. '" "' .. pathJoin(srcDir, tarFile) .. '""'
		log3('createTarGZ:', cmd)
		return processExecute(cmd)
	else
		local cmd = 'cd '.. quoteString(srcDir) .. ' && ' .. tar .. ' --exclude=' .. tarGZFile .. ' -czf ' .. tarGZFile .. ' .'
		log3('createTarGZ:', cmd)
		return os.execute(cmd)
	end
end

local function setControlParams(args)
	local path = pathJoin(args.tmpDir, 'DEBIAN', 'control')
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
			local s = string.format('%d', filesize)
			while s:len() < 10 do
				s = s .. ' '
			end
			fi:write('control.tar.gz  1410122664  0     0     100644  ' .. s .. '`\n')

			f:seek("set", 0)
			local buf = f:read("*a")
			fi:write(buf)
			if (filesize % 2) ~= 0 then
				fi:write('\n')
			end
			f:close()
		end

		-- add data.tar.gz
		local path = pathJoin(srcDir, 'CONTENTS', 'data.tar.gz')
		local f = io.open(path, "rb")
		if (f) then
			local filesize = f:seek("end")
			local s = string.format('%d', filesize)
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
			return true;
		end
		log3('copyFile failed to write: ', src, dst)
		return false;
	end
	log3('copyFile failed to read: ', src, dst)
	return false;
end

local function copyDir( src, dst )
	if windows then
		local cmd = 'robocopy "' .. src .. '" ' .. '"' .. dst.. '" /e 2> nul'
		-- robocopy failed when exit code is > 7... Windows  ¯\_(ツ)_/¯ 
		log3('copydir:', cmd)
		return processExecute(cmd)>7 and 1 or 0
	else
		local cmd = 'cp -R ' .. quoteString(src) .. '/. ' ..  quoteString(dst)
		log3('copydir:', cmd)
		return os.execute(cmd)
	end
end

local function removeDir( dir )
	if windows then
		local cmd = 'rmdir /s/q "' .. dir .. '"'
		log3('removeDir:', cmd)
		return processExecute(cmd)
	else
		local cmd = "rm -f -r " .. quoteString(dir)
		log3('removeDir:', cmd)
		os.execute(cmd)
	end
end


local function getPluginDirectories(build, pluginsToDownload, tmpDir, appFolder)
	local platform = 'linux'
	local pluginDirectories = {}
	for _, pd in pairs(pluginsToDownload) do
		local plugin, developer, supportedPlatforms = unpack( pd )

		local skip = false
		if supportedPlatforms then
			skip = not supportedPlatforms[platform]
		end

		--log3('skip=',skip, plugin, developer, supportedPlatforms, pd)
		if not skip then
			local downloadInfoURL = serverBackend .. '/v1/plugins/download/' .. developer .. '/' .. plugin .. '/' .. build .. '/' .. platform
			log3('loading plugin:', plugin, 'platform:', platform, 'from: ', downloadInfoURL)

			local downloadInfoText, msg = http.request(downloadInfoURL)
			if not downloadInfoText then
				return "ERROR: unable to fetch plugin download location for " .. plugin .. ' ('.. developer.. '). Error message: ' .. msg 
			end

			local downloadInfoJSON = json.decode(downloadInfoText)
			local downloadURL = downloadInfoJSON.url
			if not downloadURL then
				return "ERROR: unable to parse plugin download location for " .. plugin .. ' ('.. developer.. ').'
			end

			log3('Downloading plugin from:', downloadURL)
			local file, err = http.request(downloadURL)
			if err ~= 200 then
				-- log error, ignore it and try to make linux app as is
				log("ERROR: unable to download " .. plugin .. ' ('.. developer.. '). Error message: \n' .. file)
			else
				-- put downloaded data in 
				local pluginArchivePath = pathJoin(tmpDir, plugin .. '_' .. developer .. ".tgz")

				fi = io.open(pluginArchivePath, "wb")
				if (fi == nil) then
					return 'ERROR: unable to create tgz' --unpack plugin " .. plugin .. ' (' .. developer .. ').'
				end
				fi:write(file)
				fi:close()

				local ret = unpackPlugin(pluginArchivePath,  pathJoin(tmpDir, plugin), tmpDir, plugin .. '_' .. developer)
				if ret ~= 0 then
					return "ERROR: unable to unpack plugin " .. plugin .. ' (' .. developer .. ').'
				end

				-- read metadata
				local pluginsDir = pathJoin(tmpDir, plugin)
				local metadataPath = pathJoin(pluginsDir, 'metadata.lua')
				local metadataChunk = loadfile( metadataPath )
				if metadataChunk then
					local metadata = metadataChunk()

					-- Store path to plugin folder
					metadata.plugin.path = pluginsDir

					-- Add plugin metadata to manifest
					log("Found native plugin: "..tostring(metadata.plugin.path))
					linuxPluginsMetadata[plugin] = metadata.plugin
				else
					local metadata = {}
					metadata.path = pluginsDir

					-- Add plugin metadata to manifest
					log("Found native plugin: "..tostring(metadata.path), ', no metadata found')
					linuxPluginsMetadata[plugin] = metadata
				end

			end
		else
			log3('skip loading plugin:', plugin, 'platform:', platform)
		end
	end
	return nil
end

local function linuxDownloadPlugins(user, buildYear, buildRevision, tmpDir, appFolder)
	log3('Plugins loader started')
	if type(buildSettings) ~= 'table' then
		log3('No build.settings file, so no plugins to download')
		return nil
	end

	if type(buildSettings.plugins) ~= 'table' then
		 buildSettings.plugins = {}
		 -- No plugins to download
		log3('No plugin section in build.settings')
		return nil 
	end

	local pluginsToDownload = {}

	for pluginName, pluginTable in pairs(buildSettings.plugins) do
		local publisherId = pluginTable['publisherId']
		table.insert( pluginsToDownload, {pluginName, publisherId, pluginTable.supportedPlatforms} )
	end

	if #pluginsToDownload > 0 then
		local authURL = serverBackend .. '/v1/plugins/show/' .. user

		log3('authURL to my plugins: ', authURL)

		--local authorisedPluginsText, msg = builder.fetch(authURL)
		local authorisedPluginsText, msg = http.request(authURL)

		if not authorisedPluginsText then
			return "ERROR: Unable to retrieve authorised plugins list (" .. msg .. ")."
		end

		local authPluginsJson = json.decode( authorisedPluginsText )
		if not authPluginsJson then
			return "ERROR: Unable to parse authorised plugins list."
		end
		log3('decoded plugins list')

		if authPluginsJson.status ~= 'success' then
			return "ERROR: Retrieving authorised plugins was unsuccessful. Info: " .. authorisedPluginsText
		end

		if not authPluginsJson.data then
			return "ERROR: received empty data for authorised plugins."
		end

		local authorisedPlugins = {}
		for _, ap in pairs(authPluginsJson.data) do 
			if ap['plugin_name'] ~= nil and ap['plugin_developer'] ~= nil then
				local p = ap['plugin_name'] .. ' ' .. ap['plugin_developer']
				authorisedPlugins[p] = ap['status']
			end
		end

		local authErrors = false
		for _, pd in pairs(pluginsToDownload) do
			local plugin, developer = unpack( pd )
			local status = authorisedPlugins[plugin .. ' ' .. developer]
			log3('plugins to load:', plugin, ', status=', status)
			if status ~= 2 and status ~= 1 then
				log("ERROR: plugin could not be validated: " .. plugin .. " (" .. developer .. ")")
				log("ERROR: Activate plugin at: https://marketplace.coronalabs.com/plugin/" .. developer .. "/" .. plugin)
				authErrors = true
			end
		end
		if authErrors then
			return "ERROR: exiting due to plugins above not being activated."
		end
	else
		log3("No plugins to download for Linux platform")
		return nil 
	end

	local build = buildYear .. '.' .. buildRevision

	local msg = getPluginDirectories(build, pluginsToDownload, tmpDir, appFolder)
	if type(msg) == 'string' then
		return msg
	end

	return nil
end

local function getExcludePredecate()
	local excludes = {
			"*.config",
			"*.lu",
			"*.lua",
			"*.bak",
			"*.orig",
			"*.swp",
			"*.DS_Store",
			"*.apk",
			"*.obb",
			"*.obj",
			"*.o",
			"*.lnk",
			"*.class",
			"*.log",
			".*",
			"build.properties",
		}

		-- append 'all:' and 'linux:'
		if buildSettings and buildSettings.excludeFiles then
			if buildSettings.excludeFiles.all then
				-- append excludes from 'all:'
				local excl = buildSettings.excludeFiles.all		
				for i = 1, #excl do	
					excludes[#excludes + 1] = excl[i]
				end
			end
			if buildSettings.excludeFiles.linux then
				-- append excludes from 'linux:'
				local excl = buildSettings.excludeFiles.linux
				for i = 1, #excl do	
					excludes[#excludes + 1] = excl[i]
				end
			end
		end

		-- glob ==> regexp
		for i = 1, #excludes do
			excludes[i] = globToPattern(excludes[i])
		end

--		for i = 1, #excludes do
--			log3('Exclude rule: ', excludes[i])
--		end

		return

		function(fileName)
			for i = 1, #excludes do
				local rc = fileName:find(excludes[i])
				if rc ~= nil then
					return false
				end
			end
			return true
		end
end

local function deleteUnusedFiles(srcDir, excludePredicate)
	log3('Deleting unused assets from ' .. srcDir)
	for file in lfs.dir(srcDir) do
		local f = pathJoin(srcDir, file)
		if excludePredicate(file) then
			local attr = lfs.attributes (f)
			if attr.mode == "directory" then
				deleteUnusedFiles( pathJoin(srcDir, file), excludePredicate)
			else
			end
		else
			if file ~= '..' and file ~= '.' then
				local result, reason = os.remove(f);
				if result then
--					log3('Excluded ' .. f)
				else
					log3("Failed to exclude" .. f) 
				end
			end
    end
	end
end

local function makeApp(arch, linuxappFolder, template, args, templateName)
	-- sanity check

	local archivesize = lfs.attributes (template, "size")
	if archivesize == nil or archivesize == 0 then
		return 'Failed to open template: ' .. template
	end

	-- create tmp folder
	removeDir(args.tmpDir)
	local success = lfs.mkdir(args.tmpDir)
	if not success then
		log('Failed to create tmpDir: ' .. args.tmpDir)
		return 'Failed to create tmpDir: ' .. args.tmpDir
	end
	log3('Created tmp folder: ' .. args.tmpDir)

	--local ret = unzip(template, args.tmpDir)
	local ret = unpackPlugin(template, args.tmpDir, args.tmpDir, templateName)
	if ret ~= 0 then
		return 'Failed to unpack template ' .. template .. ' to ' .. args.tmpDir ..  ', err=' .. ret
	end
	log3('Unzipped ' .. template, ' to ', args.tmpDir) 

	local binFile = args.applicationName
	local oldname = pathJoin(args.tmpDir, 'CONTENTS', 'usr', 'bin', 'linux_rtt')
	local newname = pathJoin(args.tmpDir, 'CONTENTS', 'usr', 'bin', binFile)
	os.rename(oldname, newname)

	-- exclude from build
	os.remove(pathJoin(args.tmpDir, 'CONTENTS', 'usr', 'bin', '.info'))
	os.remove(pathJoin(args.tmpDir, 'CONTENTS', 'usr', 'share', 'corona', '.info'))

	local appFolder = pathJoin(args.tmpDir, 'CONTENTS', 'usr', 'share', 'corona', args.applicationName)
	local appBinaryFolder = pathJoin(args.tmpDir, 'CONTENTS', 'usr', 'bin')

	local success = lfs.mkdir(appFolder)
	if not success then
		return 'Failed to create app folder: ' .. appFolder
	end
	log3('Created app folder: ' .. appFolder)

	-- dowmload plugins
	local msg = linuxDownloadPlugins(args.user, args.buildYear, args.buildRevision, args.tmpDir, appFolder)
	if type(msg) == 'string' then
		return msg
	end

	-- copy plugins into app folder
	for k, metadata in pairs(linuxPluginsMetadata) do
		-- just copy all files into app folder
		local pluginDir = metadata.path

		--	check if it is Lua plugin, Lua plugin is in '/lua/lua_51'
		if dir_exists(pluginDir .. '/lua/lua_51') then
			pluginDir = pluginDir .. '/lua/lua_51'
			copyDir(pluginDir, appFolder)
		else
			-- shared lib ?
			if dir_exists(pluginDir .. '/' .. arch) then
				pluginDir = pluginDir .. '/' .. arch
				copyDir(pluginDir, appFolder)
				copyDir(pluginDir, appBinaryFolder)
			end
		end

		-- exclude medata.lua from app
		os.remove(pathJoin(appFolder, 'metadata.lua'))

		-- exclude 
		os.remove(metadata.path)
	end

	-- gather files into appFolder (tmp folder)
	local ret = copyDir( args.srcDir, appFolder )
	if ret ~= 0 then
		return "Failed to copy " .. args.srcDir .. ' to ' .. appFolder
	end
	log3("Copied app files from ", args.srcDir, ' to ', appFolder)

	-- copy standard resources
	local widgetsDir = pathJoin(args.tmpDir, 'CONTENTS', 'usr', 'share', 'corona', 'res_widget')
	if args.useStandartResources then
		local ret = copyDir(widgetsDir, appFolder)
		if ret ~= 0 then
			return "Failed to copy standard resources"
		end
		log3("Copied startard resources")
	end

	-- remove res_widget folder from .deb because else it will common for all apps in the host
	removeDir(widgetsDir)

	-- compile .lua
	local rc = compileScriptsAndMakeCAR(args.linuxParams, appFolder, appFolder, args.tmpDir)
	if not rc then
		return "Failed to create .car file"
	end
	log3("Created .car file")

	-- delete .lua, .lu, etc
	deleteUnusedFiles(appFolder, getExcludePredecate())

	-- add debian package control file
	setControlParams(args)

	-- commpress and create deb file

	createTarGZ(pathJoin(args.tmpDir, 'CONTENTS'), 'data.tar', 'data.tar.gz')
	createTarGZ(pathJoin(args.tmpDir, 'DEBIAN'), 'control.tar', 'control.tar.gz')

	local debFile = args.applicationName .. '-' .. arch .. '-' .. args.versionName .. '.deb'
	createDebArchive(debFile, args.tmpDir)

	-- copy .deb to build folder
	local ret = copyFile(pathJoin(args.tmpDir, debFile), pathJoin(linuxappFolder, debFile))
	if not ret then
			return "Failed to create Linux deb package "
	end
	log3("Created " .. pathJoin(linuxappFolder, debFile))

	--
	-- create portable .zip 
	--

	local binSrc = pathJoin(args.tmpDir, 'CONTENTS', 'usr', 'bin', binFile)
	local binFolder = pathJoin(args.tmpDir, 'CONTENTS', 'usr', 'share', 'corona', args.applicationName)
	local binDst = pathJoin(binFolder, binFile)
	local ret = copyFile(binSrc, binDst)
	if not ret then
			return "Failed to copy bin file: " .. binSrc
	end

	-- set 'executable' attribute
	if not windows then
		os.execute('chmod +x ' .. quoteString(binDst))
	end

	local ret = createTarGZ(binFolder, 'app.tar', 'app.tar.gz')
	if ret ~= 0 then
		-- log3('Failed to create app.tar.gz')
	end

	local appFile = args.applicationName .. '-' .. arch .. '-' .. args.versionName .. '.tar.gz'
	local dstFile = pathJoin(linuxappFolder, appFile)
	os.remove(dstFile)

	-- copy .tar.gz to build folder
	local src = pathJoin(binFolder, 'app.tar.gz')
	local ret = copyFile(src, dstFile)
	if not ret then
			return "Failed to create Linux app"
	end
	log3("Created " .. dstFile)
end

--
-- global script to call from C++
---
function linuxPackageApp( args )
	debugBuildProcess = args.debugBuildProcess
	log('Linux builder started')
	log3(json.prettify(args))

	local template = args.templateLocation

-- for debugging
	-- local template = 'C:\\Program Files (x86)\\Corona Labs\\Corona\\Resources\\linuxtemplate.tar.gz'
	-- local template = '/Users/mymac/linuxtemplate.tar.gz'

	if not template then
		local coronaRoot
		if windows then
			coronaRoot = os.getenv("CORONA_PATH")
		else
			local coronaDir = lfs.symlinkattributes(os.getenv('HOME') .. '/Library/Application Support/Corona/Native', "target")
			if coronaDir then
				coronaRoot = coronaDir .. "../Corona Simulator.app/Contents"
			end
		end
		template = pathJoin(coronaRoot , 'Resources', 'linuxtemplate.tar.gz')
	end

	local templateArm = template
	templateArm = templateArm:gsub('linuxtemplate.tar.gz', 'raspbiantemplate.tar.gz')

	-- read settings
	local buildSettingsFile = pathJoin(args.srcDir, 'build.settings')
	local oldSettings = _G['settings']
	_G['settings'] = nil
	pcall( function() dofile(buildSettingsFile) end	)
	buildSettings = _G['settings']
	_G['settings'] = oldSettings

	local success = false;

	-- create app folder if it does not exists
	local linuxappFolder = pathJoin(args.dstDir, args.applicationName)
	if not dir_exists(linuxappFolder) then
		success = lfs.mkdir(linuxappFolder)
		if not success then
			return 'Failed to create app folder: ' .. linuxappFolder
		end
		log3('Created app folder: ' .. linuxappFolder)
	end

	local rc = makeApp('ubuntu-18.04-x86-64', linuxappFolder, template, args, 'linuxtemplate')
	if rc ~= nil then
		return rc
	end

	local rc = makeApp('raspberry-pi3', linuxappFolder, templateArm, args, 'raspbiantemplate')
	if rc ~= nil then
		return rc
	end

	log('LINUX builder ended')
	return nil 
end
