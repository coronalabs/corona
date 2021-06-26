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

function dir_exists(path)
    local cd = lfs.currentdir()
    local is = lfs.chdir(path) and true or false
    lfs.chdir(cd)
    return is
end

function file_exists(name)
   local f = io.open(name,"r")
   if f~=nil then io.close(f) return true else return false end
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

local function unzip( archive, dst )
	if windows then
		local cmd = '""%CORONA_PATH%\\7za.exe" x -aoa "' .. archive .. '" -o"' ..  dst .. '"'
		logd('processExecute: ' .. cmd)
		return processExecute(cmd)
	else
		return os.execute('/usr/bin/unzip -o -q ' .. quoteString(archive) .. ' -d ' ..  quoteString(dst))
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
		return false;
	end
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
		os.execute("rm -f -r " .. quoteString(dir))
	end
end

local function nintendoDownloadPlugins(buildRevision, tmpDir, pluginDstDir)
	if type(buildSettings) ~= 'table' then
		-- no build.settings file, so no plugins to download
		return nil
	end

	local collectorParams = { 
		pluginPlatform = 'nintendo',
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

		-- append 'all:' and 'nintendo:'
		if buildSettings and buildSettings.excludeFiles then
			if buildSettings.excludeFiles.all then
				-- append excludes from 'all:'
				local excl = buildSettings.excludeFiles.all		
				for i = 1, #excl do	
					excludes[#excludes + 1] = excl[i]
				end
			end
			if buildSettings.excludeFiles.nintendo then
				-- append excludes from 'nintendo:'
				local excl = buildSettings.excludeFiles.nintendo
				for i = 1, #excl do	
					excludes[#excludes + 1] = excl[i]
				end
			end
		end

		-- glob ==> regexp
		for i = 1, #excludes do
			excludes[i] = globToPattern(excludes[i])
		end

		for i = 1, #excludes do
			logd('Exclude rule: ', excludes[i])
		end

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

function GetFileName(url)
  return url:match("^.+/(.+)$")
end

function GetFileExtension(url)
  return url:match("^.+(%..+)$")
end

--
-- global script to call from C++
-- 
function nintendoPackageApp( args )
	debugBuildProcess = args.debugBuildProcess
	log('Nintendo Switch App builder started')
	logd(json.prettify(args))

	local template = args.nintendotemplateLocation

-- for debugging
	local template = 'e:/nintendotemplate.zip'

	-- check if user purchased splash screen
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
		template = pathJoin(coronaRoot , 'Resources', 'nintendotemplate.zip')
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
	local nintendoappFolder = pathJoin(args.dstDir, args.applicationName)
	if not dir_exists(nintendoappFolder) then
		success = lfs.mkdir(nintendoappFolder)
		if not success then
			return 'Failed to create app folder: ' .. nintendoappFolder
		end
		logd('Created app folder: ' .. nintendoappFolder)
	end
	logd('nintendoappFolder: ' .. nintendoappFolder)

	local appFolder = pathJoin(args.tmpDir, 'nintendoapp')
	success = removeDir(appFolder)	-- clear
--	if not success then
--		return 'Failed to clear tmp folder: ' .. appFolder
--	end

	success = lfs.mkdir(appFolder)
	if not success then
		return 'Failed to create tmp folder: ' .. appFolder
	end
	logd('appFolder: ' .. appFolder)

	-- unzip standard template

	local templateFolder = pathJoin(args.tmpDir, 'nintendotemplate')
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

	local ret = unzip(template, templateFolder)
	if ret ~= 0 then
		return 'Failed to unpack template ' .. template .. ' to ' .. templateFolder ..  ', err=' .. ret
	end
	logd('Unzipped ' .. template, ' to ', templateFolder)

	-- gather files into appFolder (tmp folder)
	local ret = copyDir( args.srcDir, appFolder )
	if ret ~= 0 then
		return "Failed to copy " .. args.srcDir .. ' to ' .. appFolder
	end
	logd("Copied ", args.srcDir, ' to ', appFolder)

	if args.useStandartResources then
		local ret = copyDir( pathJoin(templateFolder, 'res_widget'), appFolder )
		if ret ~= 0 then
			return "Failed to copy standard resources"
		end
		logd("Copied startard resources")
	end

	-- compile .lua
	local rc = compileScriptsAndMakeCAR(args.nintendoParams, appFolder, appFolder, args.tmpDir)
	if not rc then
		return "Failed to create .car file"
	end
	logd("Created .car file")

	-- sample: AuthoringTool.exe creatensp -o C:/corona/platform/switch/NX64/Release/Rtt.nsp --meta C:/corona/platform/switch/Solar2D/rtt.nmeta --type Application --desc C:/Nintendo\vitaly/NintendoSDK/Resources/SpecFiles/Application.desc--program C:/corona/platform/switch/NX64/Release/Rtt.nspd/program0.ncd/code C:\corona\platform\test\assets2
	-- build App 
	local nspfile = "C:\\corona\\platform\\switch\\NX64\\Release\\Rtt.nsp"
	local metafile = "C:\\corona\\platform\\switch\\Solar2D\\rtt.nmeta"
	local descfile = "C:\\Nintendo\\vitaly\\NintendoSDK\\Resources\\SpecFiles\\Application.desc"
	local solar2Dfile = "C:\\corona\\platform\\switch\\NX64\\Release\\Rtt.nspd\\program0.ncd\\code"
	local assets = "C:\\corona\\platform\\test\\assets2"

	local nintendoRoot = os.getenv("NINTENDO_SDK_ROOT")
	if not nintendoRoot then
		return "no NINTENDO_SDK_ROOT, please install Nintendo SDK"
	end

	local cmd = '"' .. nintendoRoot .. '\\Tools\\CommandLineTools\\AuthoringTool\\AuthoringTool.exe" creatensp --type Application'
	cmd = cmd .. ' -o "' ..  nspfile .. '"'
	cmd = cmd .. ' --meta "' ..  metafile .. '"'
	cmd = cmd .. ' --desc "' ..  descfile .. '"'
	cmd = cmd .. ' --program "' ..  solar2Dfile .. '"'
	cmd = cmd .. ' "' .. assets .. '"'
	cmd = 'cmd /c "'.. cmd .. '"'
	
	logd('Running ',cmd)
	local rc = processExecute(cmd);
	if rc ~=0 then
		log('Nintendo Switch App build failed, retcode ', rc)
	else
		log('Nintendo Switch App build success')
	end

	return nil 
end
