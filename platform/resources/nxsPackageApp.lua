--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--

processExecute = processExecute or os.execute

local lfs = require "lfs"
local json = require "json"
local http = require( "socket.http" )
local debugBuildProcess = 0

local dirSeparator = package.config:sub(1,1)
local windows = (dirSeparator == '\\')
local buildSettings = nil		-- build.settings

local function log(...)
	myprint(...)
end

local function logd(...)
	if debugBuildProcess >= 3 then
		myprint(...)
	end
end

local function quoteString( str )
	str = str:gsub('\\', '\\\\')
	str = str:gsub('"', '\\"')
	return "\"" .. str .. "\""
end

local function globToPattern(g)
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

local function unzip( archive, dst, p )
	if windows then
		local cmd = '""%CORONA_PATH%\\7za.exe" x -aoa "' .. archive .. '" -o"' ..  dst .. '" ' .. (p or "") .. '"'
		return processExecute(cmd)
	else
		return os.execute('/usr/bin/unzip -o -q ' .. quoteString(archive) .. ' -d ' ..  quoteString(dst))
	end
end

local function isDir(f)
    return lfs.attributes(f, 'mode') == 'directory'
end

local function isFile(f)
    return lfs.attributes(f, 'mode') == 'file'
end

local function mkdirs(path)
    local c = ""
    path:gsub('[^/\\]+', function(dir)
        if windows and c == '' then
            c = dir
        else
            c = c .. dirSeparator .. dir
        end
        if isDir(c) then
            -- do nothing, directory already exists
        else
            lfs.mkdir(c)
        end
    end)
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
		logd('copyFile: Failed to read ' .. src)
		return false;
	end
	logd('copyFile: Failed to write ' .. dst)
	return false;
end

local function copyDir( src, dst )
	if windows then
		local cmd = 'robocopy "' .. src .. '" ' .. '"' .. dst.. '" /e 2> nul'
		-- robocopy failed when exit code is > 7... Windows  ¯\_(ツ)_/¯ 
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
		return os.execute("rm -f -r " .. quoteString(dir))
	end
end

local function nxsDownloadPlugins(buildRevision, tmpDir, pluginDstDir)
	if type(buildSettings) ~= 'table' then
		-- no build.settings file, so no plugins to download
		return nil
	end

	local collectorParams = {
		pluginPlatform = 'nx64',
		plugins = buildSettings.plugins or {},
		destinationDirectory = tmpDir,
		build = buildRevision,
		extractLocation = pluginDstDir,
	}

	local pluginCollector = require "CoronaBuilderPluginCollector"
	return pluginCollector.collect(collectorParams)
end

local function getExcludePredecate()
	local excludes = {
--			"*.nro",
--			"*.nrr",
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

		-- append 'all:' and 'nx:'
		if buildSettings and buildSettings.excludeFiles then
			if buildSettings.excludeFiles.all then
				-- append excludes from 'all:'
				local excl = buildSettings.excludeFiles.all		
				for i = 1, #excl do	
					excludes[#excludes + 1] = excl[i]
				end
			end
			if buildSettings.excludeFiles.nx then
				-- append excludes from 'nx:'
				local excl = buildSettings.excludeFiles.nx
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
--			logd('Exclude rule: ', excludes[i])
--		end

		return function(fileName)
			for i = 1, #excludes do
				local rc = fileName:find(excludes[i])
				if rc ~= nil then
					return false
				end
			end
			return true
		end
end

-- copy all assets except .lua, .lu files into .data
local function pullDir(srcDir, dstDir, dataFiles, folders, out, excludePredicate)
	logd('Pulling assets from ' .. srcDir .. ' to ' .. dstDir)
	for file in lfs.dir(srcDir) do
		local xf = dstDir .. file
		xf = xf:sub(2)
		local f = pathJoin(srcDir, file)
		if excludePredicate(xf) and excludePredicate(file) then
			local attr = lfs.attributes (f)
			if attr.mode == "directory" then

				-- keep, it will be used later to generate .js
				folders[#folders+1] = { parent=dstDir , name=file };

				pullDir( pathJoin(srcDir, file), dstDir..file..'/', dataFiles, folders, out, excludePredicate)
			else
       	-- append the file to out
       	local fi = io.open(f, 'rb')
				if (fi) then
					 local block = 1024000			-- block size, 1mb
						while true do
							local bytes = fi:read(block)
							if not bytes then break end
							out:write(bytes)
						end
					fi:close()

					-- keep, it will be used later to generate .js
					dataFiles[#dataFiles+1] = { name=dstDir..file, size=attr.size };
					logd('Added ' .. f .. ' into .data')
				else
					log('Failed to add ' .. f .. ' into .data')
				end
			end
		else
			logd('Excluded ' .. f)
    end
	end
end

local function GetFileName(url)
  return url:match("^.+/(.+)$")
end

local function GetFileExtension(url)
  return url:match("^.+(%..+)$")
end

local function deleteUnusedFiles(srcDir, excludePredicate)
--	 logd('Deleting unused assets from ' .. srcDir)
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
				local result
				if isDir(f) then
					result = removeDir(f)
				else
					result, reason = os.remove(f);
				end
				if result then
--					logd('Excluded ' .. f)
				else
					logd("Failed to exclude" .. f) 
				end
			end
		end
	end
end

--
-- global script to call from C++
--
function nxsPackageApp( args )

	local nxsRoot = os.getenv("NINTENDO_SDK_ROOT")
	if not nxsRoot then
		return "NX SDK not found"
	end

	debugBuildProcess = args.debugBuildProcess
	log('NX Switch App builder started')
	local nxInfo = args.nxInfo
	args.nxInfo = nil
	logd(json.prettify(args))

	local template = args.templateLocation
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
		template = pathJoin(coronaRoot , 'Resources', 'nxtemplate')
	end
	logd("template location: " .. template);

	if not isFile(template) then
		return 'Missing template: ' .. template
	end

	-- read settings
	local buildSettingsFile = pathJoin(args.srcDir, 'build.settings')
	local oldSettings = _G['settings']
	_G['settings'] = nil
	pcall( function() dofile(buildSettingsFile) end	)
	buildSettings = _G['settings']
	_G['settings'] = oldSettings

	local success = false;

	-- create app folder if it does not exists
	local nxsappFolder = pathJoin(args.dstDir, args.applicationName .. '.NX64')
	removeDir(nxsappFolder)	-- clear
	success = lfs.mkdir(nxsappFolder)
	if not success then
		return 'Failed to create App folder: ' .. nxsappFolder
	end
	logd('AppFolder: ' .. nxsappFolder)

	local appFolder = pathJoin(args.tmpDir, 'nxsapp')
	success = removeDir(appFolder)	-- clear
--	if not success then
--		return 'Failed to clear tmp folder: ' .. appFolder
--	end

	success = lfs.mkdir(appFolder)
	if not success then
		return 'Failed to create tmp folder: ' .. appFolder
	end
	logd('appFolder: ' .. appFolder)


	-- Download plugins
	local pluginDownloadDir = pathJoin(args.tmpDir, "pluginDownloadDir")
	local pluginExtractDir = pathJoin(args.tmpDir, "pluginExtractDir")
	lfs.mkdir(pluginDownloadDir)
	lfs.mkdir(pluginExtractDir)
	local msg = nxsDownloadPlugins(args.buildRevision, pluginDownloadDir, pluginExtractDir)
	if type(msg) == 'string' then
		return msg
	end

	-- Copy lua plugins over to the app folder
	local luaPluginDir = pathJoin(pluginExtractDir, 'lua', 'lua_51')
	if isDir( luaPluginDir ) then
		copyDir(luaPluginDir, pluginExtractDir)
		removeDir(pathJoin(pluginExtractDir, 'lua'))
	end
	luaPluginDir = pathJoin(pluginExtractDir, 'lua_51')
	if isDir( luaPluginDir ) then
		copyDir(luaPluginDir, pluginExtractDir)
		removeDir(pathJoin(pluginExtractDir, 'lua_51'))
	end
	if isDir( pluginExtractDir ) then
		copyDir(pluginExtractDir, appFolder)
	end

	-- unzip standard template

	local templateFolder = pathJoin(args.tmpDir, 'nxtemplate')
	success = removeDir(templateFolder)	
--	if not success then
--		return 'Failed to clear template folder: ' .. templateFolder
--	end

	success = lfs.mkdir(templateFolder)
	if not success then
		return 'Failed to create template folder: ' .. templateFolder
	end
	logd('templateFolder: ' .. templateFolder)

	-- sanity check
	local archivesize = lfs.attributes (template, "size")
	if archivesize == nil or archivesize == 0 then
		return 'Failed to open template: ' .. template
	end

	local ret = unzip(template, templateFolder, nxInfo)
	if ret ~= 0 then
		return 'Failed to unpack template ' .. template .. ' to ' .. templateFolder ..  ', err=' .. ret
	end
	logd('Unzipped ' .. template .. ' to ' .. templateFolder)

	-- gather files into appFolder (tmp folder)
	local ret = copyDir( args.srcDir, appFolder )
	if ret ~= 0 then
		return "Failed to copy " .. args.srcDir .. ' to ' .. appFolder
	end
	logd("Copied " .. args.srcDir .. ' to ' .. appFolder)

	if args.useStandartResources then
		local ret = copyDir( pathJoin(templateFolder, 'res_widget'), appFolder )
		if ret ~= 0 then
			return "Failed to copy standard resources"
		end
		logd("Copied startard resources")
	end

	-- compile .lua
	local rc = compileScriptsAndMakeCAR(args.nxsParams, appFolder, appFolder, args.tmpDir)
	if not rc then
		return "Failed to create .car file"
	end
	logd("Created .car file")

		-- delete .lua, .lu, etc
	deleteUnusedFiles(appFolder, getExcludePredecate())

	-- build App 
	-- sample: AuthoringTool.exe creatensp -o Rtt.nsp --metartt.nmeta --type Application --desc Application.desc--program program0.ncd/code assets2

	local metafile = args.nmetaPath
	if not isFile(metafile) then
		return 'Missing ' .. metafile .. ' file'
	end
	log('Using ' .. metafile)

	local nspfile = pathJoin(nxsappFolder, args.applicationName ..'.nsp')
	local descfile = pathJoin(nxsRoot, "\\Resources\\SpecFiles\\Application.desc")
	local solar2Dfile = pathJoin(args.tmpDir, '\\nxtemplate\\code')
	local assets = pathJoin(args.tmpDir, '\\nxsapp')

	-- update .npdm file
	local cmd = '"' .. nxsRoot .. '\\Tools\\CommandLineTools\\MakeMeta\\MakeMeta.exe'
	cmd = cmd .. ' --desc ' .. nxsRoot .. '\\Resources\\SpecFiles\\Application.desc'
	cmd = cmd .. ' --meta "' ..  metafile .. '"'
	cmd = cmd .. ' -o "' ..  solar2Dfile .. '\\main.npdm"'
	logd('\Creating the NPDM File ... ', cmd)
	local rc, stdout = processExecute(cmd, true);
	log('\MakeMeta retcode ' .. rc)
	if type(stdout) == 'string' and string.len(stdout) > 0 then
		log('\MakeMeta output\n' .. stdout)
	end

	-- create .nsp file
	local cmd = '"' .. nxsRoot .. '\\Tools\\CommandLineTools\\AuthoringTool\\AuthoringTool.exe" creatensp --type Application'
	cmd = cmd .. ' -o "' ..  nspfile .. '"'
	cmd = cmd .. ' --meta "' ..  metafile .. '"'
	cmd = cmd .. ' --desc "' ..  descfile .. '"'
	cmd = cmd .. ' --program "' ..  solar2Dfile .. '"'
	cmd = cmd .. ' "' .. assets .. '"'
	cmd = 'cmd /c "'.. cmd .. '"'
	
	logd('\nBuilding App ... ', cmd)
	local rc, stdout = processExecute(cmd, true);
	log('\nAuthoringTool retcode ' .. rc)
	if type(stdout) == 'string' and string.len(stdout) > 0 then
		log('\nAuthoringTool output\n' .. stdout)
	end

	if not isFile(nspfile) then
		return 'Failed to build NX Switch App'
	else
		log('\nBuild succeeded: ' .. nspfile)
	end

	return nil 
end
