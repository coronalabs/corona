------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------


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

local function webDownloadPlugins(buildRevision, tmpDir, pluginDstDir)
	if type(buildSettings) ~= 'table' then
		-- no build.settings file, so no plugins to download
		return nil
	end

	local collectorParams = { 
		pluginPlatform = 'web',
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

		-- append 'all:' and 'web:'
		if buildSettings and buildSettings.excludeFiles then
			if buildSettings.excludeFiles.all then
				-- append excludes from 'all:'
				local excl = buildSettings.excludeFiles.all		
				for i = 1, #excl do	
					excludes[#excludes + 1] = excl[i]
				end
			end
			if buildSettings.excludeFiles.web then
				-- append excludes from 'web:'
				local excl = buildSettings.excludeFiles.web
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
			log3('Exclude rule: ', excludes[i])
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
	log3('Pulling assets from ' .. srcDir .. ' to ' .. dstDir)
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
					log3('Added ' .. f .. ' into .data')
				else
					log('Failed to add ' .. f .. ' into .data')
				end
			end
		else
			log3('Excluded ' .. f)
    end
	end
end

function GetFileName(url)
  return url:match("^.+/(.+)$")
end

function GetFileExtension(url)
  return url:match("^.+(%..+)$")
end

local function buildTemplate(templateFolder)
--[[
	-- get plugin files

	local pluginFiles = {}
	local exportedFunctions = {}
	for k, metadata in pairs(webPluginsMetadata) do
		if metadata.format == 'a' then
			for i = 1, #metadata.exportedFunctions do	
				table.insert(exportedFunctions, metadata.exportedFunctions[i])
			end
			for file in lfs.dir(metadata.path) do
				local ext = GetFileExtension(file)
				if ext == '.a' or ext == '.o' then
					table.insert(pluginFiles, quoteString(pathJoin(metadata.path, file)))
				end
			end
		end
	end

	-- hack
	if windows then
		cmd = 'z:/corona/link_emscripten.bat'
	else
		cmd = '/Users/mymac/link_emscripten.sh'
	end
	cmd = cmd .. ' -O2'

	-- get bin files

	local binFolder = templateFolder .. '/bin'
	local binFiles = {}
	for file in lfs.dir(binFolder) do
		local ext = GetFileExtension(file)
		if ext == '.a' or ext == '.o' then
			table.insert(binFiles, quoteString(pathJoin(binFolder, file)))
		end
		if ext == '.js' then
			table.insert(binFiles, '--js-library ' .. quoteString(pathJoin(binFolder, file)))
		end
	end

	-- bin files
	for i = 1, #binFiles do	
		cmd = cmd .. ' ' .. binFiles[i]
	end
	-- plugins
	for i = 1, #pluginFiles do	
		cmd = cmd .. ' ' .. pluginFiles[i]
	end

	cmd = cmd .. " -s \"EXTRA_EXPORTED_RUNTIME_METHODS=['ccall', 'cwrap']\""

	local exportedFunctionsString = " -s \"EXPORTED_FUNCTIONS=['_main'"
	for i = 1, #exportedFunctions do	
		exportedFunctionsString = exportedFunctionsString .. ",'" .. exportedFunctions[i] .. "'"
	end
	exportedFunctionsString = exportedFunctionsString .. "]\""

	cmd = cmd .. exportedFunctionsString
	cmd = cmd .. " -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s ALLOW_MEMORY_GROWTH=1 --use-preload-plugins"

	-- generate empty data
	lfs.mkdir(pathJoin(templateFolder, 'data'))
	lfs.mkdir(pathJoin(templateFolder, 'data', 'subfolder'))
	copyFile(pathJoin(templateFolder, 'html/emscripten.html'), pathJoin(templateFolder, 'data/emscripten.html'))

	local preloadFile = quoteString(pathJoin(templateFolder, 'data'))
	cmd = cmd .. " --preload-file " .. preloadFile

	lfs.mkdir(pathJoin(templateFolder, 'newhtml'))
	local outputFile = quoteString(pathJoin(templateFolder, 'newhtml/emscripten.html'))
	cmd = cmd .. " -o " .. outputFile

	 log3(cmd)
	 local rc = 0
	if windows then
--		processExecute(cmd)
		rc = os.execute(cmd)
	else
		rc = os.execute(cmd)
	end

	if rc == 0 then
		-- replace .js and .mem in old template
		copyFile(pathJoin(templateFolder, 'newhtml/emscripten.js'), pathJoin(templateFolder, 'html/emscripten.js'))
		copyFile(pathJoin(templateFolder, 'newhtml/emscripten.data'), pathJoin(templateFolder, 'html/emscripten.data'))
		copyFile(pathJoin(templateFolder, 'newhtml/emscripten.html.mem'), pathJoin(templateFolder, 'html/emscripten.html.mem'))

--		local template = quoteString(pathJoin(templateFolder, 'newtemplate.zip'))
--		zip(quoteString(templateFolder), template)
	end
--]]
end

--
-- global script to call from C++
---
function webPackageApp( args )
	debugBuildProcess = args.debugBuildProcess
	log('HTML5 builder started')
	log3(json.prettify(args))

	local template = args.webtemplateLocation

-- for debugging
--	local template = 'z:/webtemplate.zip'
--	local template = '/Users/mymac/corona/main-vitaly/platform/emscripten/webtemplate.zip'

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
		template = pathJoin(coronaRoot , 'Resources', 'webtemplate.zip')
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
	local webappFolder = pathJoin(args.dstDir, args.applicationName)
	if not dir_exists(webappFolder) then
		success = lfs.mkdir(webappFolder)
		if not success then
			return 'Failed to create app folder: ' .. webappFolder
		end
		log3('Created app folder: ' .. webappFolder)
	end

	local appFolder = pathJoin(args.tmpDir, 'webapp')
	success = removeDir(appFolder)	-- clear
--	if not success then
--		return 'Failed to clear tmp folder: ' .. appFolder
--	end

	success = lfs.mkdir(appFolder)
	if not success then
		return 'Failed to create tmp folder: ' .. appFolder
	end
	log3('TMP folder: ' .. appFolder)

	-- unzip standard template
	local templateFolder = pathJoin(args.tmpDir, 'webtemplate')
	success = removeDir(templateFolder)	
--	if not success then
--		return 'Failed to clear template folder: ' .. templateFolder
--	end

	success = lfs.mkdir(templateFolder)
	if not success then
		return 'Failed to create template folder: ' .. templateFolder
	end
	log3('Template folder: ' .. templateFolder)

	-- sanity check
	local archivesize = lfs.attributes (template, "size")
	if archivesize == nil or archivesize == 0 then
		return 'Failed to open template: ' .. template
	end

	local ret = unzip(template, templateFolder)
	if ret ~= 0 then
		return 'Failed to unpack template ' .. template .. ' to ' .. templateFolder ..  ', err=' .. ret
	end
	log3('Unzipped ' .. template, ' to ', templateFolder)

	-- dowmload plugins
	local pluginDownloadDir = pathJoin(args.tmpDir, "pluginDownloadDir")
	local pluginExtractDir = pathJoin(args.tmpDir, "pluginExtractDir")
	lfs.mkdir(pluginDownloadDir)
	lfs.mkdir(pluginExtractDir)
	local msg = webDownloadPlugins(args.buildRevision, pluginDownloadDir, pluginExtractDir)
	if type(msg) == 'string' then
		return msg
	end

	local luaPluginDir = pathJoin(pluginExtractDir, 'lua', 'lua_51')
	if dir_exists( luaPluginDir ) then
		copyDir(luaPluginDir, pluginExtractDir)
		removeDir(pathJoin(pluginExtractDir, 'lua'))
	end
	local luaPluginDir = pathJoin(pluginExtractDir, 'lua_51')
	if dir_exists( luaPluginDir ) then
		copyDir(luaPluginDir, pluginExtractDir)
		removeDir(pathJoin(pluginExtractDir, 'lua_51'))
	end
	if dir_exists( pluginExtractDir ) then
		copyDir(pluginExtractDir, appFolder)
	end

	-- build app specific template
	if useNewTemplate then
		buildTemplate(templateFolder)
		log('Builder used custom template')
	end

	local function copyHtmlTemplateFile(srcFileName, dstFileName, skipIfExists)
		local destinationFile = pathJoin(webappFolder, dstFileName)
		if skipIfExists and file_exists(dstFileName) then
			log('Using existing ', dstFileName)
		else
			local templateFile = pathJoin(templateFolder, 'html', srcFileName)
			local ret = copyFile(templateFile, destinationFile)
			if not ret then
					return "Failed to copy " .. templateFile .. ' to ' .. destinationFile
			end
			log3("Copied " .. templateFile .. ' to ' .. destinationFile)
		end
	end

	-- copy html template files
	local err
	err = copyHtmlTemplateFile('index.html', 'index.html', true)
	if err then
		return err
	end

	err = copyHtmlTemplateFile('index-nosplash.html', 'index-nosplash.html', true)
	if err then
		return err
	end

	local err = copyHtmlTemplateFile('index-debug.html', 'index-debug.html', true)
	if err then
		return err
	end

	local err = copyHtmlTemplateFile('coronaHtml5App.js', 'coronaHtml5App.js')
	if err then
		return err
	end

	local err = copyHtmlTemplateFile('coronaHtml5App.html.mem', 'coronaHtml5App.html.mem')
	if err then
		return err
	end

	-- gather files into appFolder (tmp folder)
	local ret = copyDir( args.srcDir, appFolder )
	if ret ~= 0 then
		return "Failed to copy " .. args.srcDir .. ' to ' .. appFolder
	end
	log3("Copied ", args.srcDir, ' to ', appFolder)

	if args.useStandartResources then
		local ret = copyDir( pathJoin(templateFolder, 'res_widget'), appFolder )
		if ret ~= 0 then
			return "Failed to copy standard resources"
		end
		log3("Copied startard resources")
	end

	-- compile .lua
	local rc = compileScriptsAndMakeCAR(args.webParams, appFolder, appFolder, args.tmpDir)
	if not rc then
		return "Failed to create .car file"
	end
	log3("Created .car file")

	-- open emscripten .data for output

	local outPath = pathJoin(webappFolder, args.applicationName .. ".data")
	local out = io.open(outPath, "wb");
	if out == nil then
		return 'Failed to create .data file';
	end
	log3("Created .data file")
	
	local dataFiles = {};
	local folders = {};

	pullDir(appFolder, "/", dataFiles, folders, out, getExcludePredecate());
	out:close();

	if #dataFiles == 0 then
		return 'No files in ' .. appFolder;
	end

	--log3('DataFiles: size=', #dataFiles, json.prettify(dataFiles))
	--log3('folders: size=', #folders, json.prettify(folders))

	--generate new loadPackaghe for .js

	local loadPackage = 'loadPackage({"files":[';
	local pos = 0

	log('.data file map (size/name)');
	local totalDataSize = 0;
	for i = 1, #dataFiles do
		-- {"audio":0,"start":0,"crunched":0,"end":5,"filename":"/main.lua"}
		loadPackage = loadPackage .. '{"audio":0,';
		loadPackage = loadPackage .. '"start":' .. pos .. ','; 
		pos = pos + dataFiles[i].size
		loadPackage = loadPackage .. '"crunched":0,';
		loadPackage = loadPackage .. '"end":' .. pos .. ',';
		loadPackage = loadPackage .. '"filename":"' .. dataFiles[i].name ..'"}';
		if i == #dataFiles then loadPackage = loadPackage .. ']' else loadPackage = loadPackage .. "," end

		-- print log
		local s = dataFiles[i].size;
		while string.len(s) < 7 do
			s = ' ' .. s
		end
		log('Size = ' .. s .. ', ' .. dataFiles[i].name);
		totalDataSize = totalDataSize + dataFiles[i].size;
	end
	log('Total data file size = ' .. totalDataSize .. ' = ' .. math.floor(totalDataSize / 1024) .. 'KB = ' .. math.floor(totalDataSize / 1024 / 1024) .. 'MB');

	-- ,"remote_package_size":5,"package_uuid":"134361ad-01a4-42aa-aea6-5b48c05818f7"})
	loadPackage = loadPackage .. ',"remote_package_size":' .. pos;

	-- fixme UUID
	loadPackage = loadPackage .. ',"package_uuid":"134361ad-01a4-42aa-aea6-5b48c05818f7"})';
	--log3('loadPackage:', loadPackage);

	--generate new FS_createPath for .js
	
	local createPaths = ''
	for i = 1, #folders do
		createPaths = createPaths .. 'Module["FS_createPath"]("'
		createPaths = createPaths .. folders[i].parent
		createPaths = createPaths .. '","'
		createPaths = createPaths .. folders[i].name
		createPaths = createPaths .. '",true,true);'
	end
	--log3('FS_createPath:',createPaths);

	-- generate .js

	-- read template, source
	local fi = io.open(pathJoin(webappFolder, "coronaHtml5App.js"), 'rb')
	if (fi == nil) then
		return 'Failed not open ' .. 'coronaHtml5App.js'
	end
	local src = fi:read("*a")	-- .js file
	fi:close()

	-- seek loadPackage({"files":[{"audio":0,"start":0,"crunched":0,"end":359,"filename":"/main.lua"},{"audio":0,"start":359,"crunched":0,"end":718,"filename":"/zzz/main.lua"}],"remote_package_size":718,"package_uuid":"be67bd33-1e30-46ab-85c8-ab4d3f06cf1d"})
	local count
 	src, count = src:gsub('loadPackage%b()', loadPackage, 1)
 	if count < 1 then
		return 'Source .js file does not contain loadPackage(...)';
 	end

	-- seek Module["FS_createPath"]("/","CORONA_FOLDER_PLACEHOLDER",true,true);
 	src, count = src:gsub('Module%["FS_createPath"]%b();', createPaths, 1)
 	if count < 1 then
		return 'Source .js file does not contain FS_createPath()';
 	end

 	-- rename
 	src, count = src:gsub('coronaHtml5App.data', args.applicationName .. ".data")
 	if count < 1 then
		return 'Source .js file does not contain coronaHtml5App.data';
 	end

	-- write .js
	local outPath = pathJoin(webappFolder, "coronaHtml5App.js");
	local out = io.open(outPath, "wb");
	if (out == nil) then
		return 'Failed to create .js file';
	end

	out:write(src)
	out:close()
	log3('Created ', outPath)

	-- prepare index.html

	-- read file
	local fi = io.open(pathJoin(webappFolder, "index.html"), 'rb')
	if (fi == nil) then
		return 'Failed to open index.html';
	end
	local s = fi:read("*a")	-- read file
	fi:close()

	local count
 	s, count = s:gsub('coronaHtml5App.bin', args.applicationName .. ".bin", 1)
	if count > 0 then
		-- replace title
		s = s:gsub('Corona HTML5 App', 'Downloading: ' .. args.applicationName)

		-- rewrite file
		local outPath = pathJoin(webappFolder, "index.html");
		local out = io.open(outPath, "wb");
		if (out == nil) then
			return 'Failed to renew index.html';
		end
		out:write(s)
		out:close() 	
		log3('Created ', outPath)
	end

	-- prepare index-debug.html

	-- read file
	local fi = io.open(pathJoin(webappFolder, "index-debug.html"), 'rb')
	if (fi == nil) then
		return 'Failed to open index-debug.html';
	end
	local s = fi:read("*a")	-- read file
	fi:close()

	local count
 	s, count = s:gsub('coronaHtml5App.bin', args.applicationName .. ".bin", 1)
	if count > 0 then
		-- replace title
		s = s:gsub('Corona HTML5 App', 'Downloading: ' .. args.applicationName)

		-- rewrite file
		local outPath = pathJoin(webappFolder, "index-debug.html");
		local out = io.open(outPath, "wb");
		if (out == nil) then
			return 'Failed to renew index-debug.html';
		end
		out:write(s)
		out:close() 	
		log3('Created ', outPath)
	end

	-- compress .js & .mem into .bin
	gzip(webappFolder, "coronaHtml5App" , {'.js', '.html.mem'}, args.applicationName..'.bin')

	--
	-- build FB Instant app
	--
	if args.createFBInstantArchive then
		-- create tmp folder for fb app
		local appFolder = pathJoin(args.tmpDir, 'fbapp')
		success = removeDir(appFolder)	-- clear
		success = lfs.mkdir(appFolder)
		if not success then
			return 'Failed to create tmp folder: ' .. appFolder
		end
		log3('TMP folder: ' .. appFolder)

		-- copy template files into appFolder (tmp folder)
		local srcDir =  pathJoin(templateFolder, 'fbinstant')
		local ret = copyDir(srcDir, appFolder)
		if ret ~= 0 then
			return "Failed to copy " ..  srcDir .. ' to ' .. appFolder
		end
		log3("Copied ", srcDir, ' to ', appFolder)

		-- copy .data
		local src = pathJoin(webappFolder, args.applicationName .. '.data')
		local dst = pathJoin(appFolder, args.applicationName .. '.data')
		local ret = copyFile(src, dst)
		if not ret then
				return "Failed to copy " .. src .. ' to ' .. dst
		end
		log3("Copied " .. src .. ' to ' .. dst)

		-- copy .js and .mem files
		local src = pathJoin(webappFolder, args.applicationName .. '.bin')
		local ret = unzip(src, appFolder)
		if ret ~= 0 then
			return 'Failed to unpack ' .. src .. ' to ' .. appFolder ..  ', err=' .. ret
		end
		log3('Unzipped ' .. src, ' to ', appFolder)

		-- replace  coronaHtml5App.data
		-- read file
		local src = pathJoin(appFolder, "index.html")
		local fi = io.open(src, 'rb')
		if (fi == nil) then
			return 'Failed to open ' .. src
		end
		local s = fi:read("*a")	-- read file
		fi:close()

		local count
 		s, count = s:gsub('coronaHtml5App.data', args.applicationName .. ".data", 1)
		if count > 0 then
			-- rewrite file
			local out = io.open(src, "wb");
			if (out == nil) then
				return 'Failed to renew ' .. src
			end
			out:write(s)
			out:close() 	
			log3('Created ', src)
		end

		-- compress folder
		local src = quoteString(appFolder)
		local dst = quoteString(pathJoin(webappFolder, args.applicationName .. '_fbinstant_app.zip'))
		local ret = zip(src, dst)
		if ret ~= 0 then
			return 'Failed to create ' .. dst
		end
		log3('Created FB instant app: ', dst)

	end

	log('HTML5 builder ended')

	return nil 
end
