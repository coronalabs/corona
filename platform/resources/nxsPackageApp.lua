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
local buildSettings = nil

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
  local p = "^"
  local i = 0
  local c

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

  local function escape(c)
    return c:match("^%w$") and c or '%' .. c
  end

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
          i = i - 1
        end
      end
      i = i + 1; c = g:sub(i,i)
    end
    return true
  end

  local function charset()
    i = i + 1; c = g:sub(i,i)
    if c == '' or c == ']' then
      p = '[^]'
      return false
    elseif c == '^' or c == '!' then
      i = i + 1; c = g:sub(i,i)
      if c == ']' then
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
    if not f then return false end
    return lfs.attributes(f, 'mode') == 'directory'
end

local function isFile(f)
    if not f then return false end
    return lfs.attributes(f, 'mode') == 'file'
end

local function getFileSize(f)
    if not f then return 0 end
    return lfs.attributes(f, 'size') or 0
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
        logd('copyFile: Failed to write ' .. dst)
        return false
    end
    logd('copyFile: Failed to read ' .. src)
    return false
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
        return os.execute("rm -f -r " .. quoteString(dir))
    end
end

-- Convert short 8.3 path to long path on Windows
local function getLongPath(path)
    if not windows then
        return path
    end
    
    -- Skip if path doesn't contain ~ (not a short path)
    if not path:find("~") then
        return path
    end
    
    -- Use cmd /c with for loop to expand short path to long path
    local cmd = 'cmd /c "for %I in ("' .. path .. '") do @echo %~fI"'
    local rc, stdout = processExecute(cmd, true)
    
    if rc == 0 and type(stdout) == 'string' and #stdout > 0 then
        local result = stdout:gsub("[\r\n]+$", "") -- trim newlines
        if #result > 0 then
            return result
        end
    end
    
    return path
end

local function nxsDownloadPlugins(buildRevision, tmpDir, pluginDstDir)
    if type(buildSettings) ~= 'table' then
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

    if buildSettings and buildSettings.excludeFiles then
        if buildSettings.excludeFiles.all then
            for i = 1, #buildSettings.excludeFiles.all do
                excludes[#excludes + 1] = buildSettings.excludeFiles.all[i]
            end
        end
        if buildSettings.excludeFiles.nx then
            for i = 1, #buildSettings.excludeFiles.nx do
                excludes[#excludes + 1] = buildSettings.excludeFiles.nx[i]
            end
        end
    end

    for i = 1, #excludes do
        excludes[i] = globToPattern(excludes[i])
    end

    return function(fileName)
        for i = 1, #excludes do
            if fileName:find(excludes[i]) then
                return false
            end
        end
        return true
    end
end

local function deleteUnusedFiles(srcDir, excludePredicate)
    for file in lfs.dir(srcDir) do
        local f = pathJoin(srcDir, file)
        if excludePredicate(file) then
            local attr = lfs.attributes(f)
            if attr and attr.mode == "directory" then
                deleteUnusedFiles(pathJoin(srcDir, file), excludePredicate)
            end
        else
            if file ~= '..' and file ~= '.' then
                local result
                if isDir(f) then
                    result = removeDir(f)
                else
                    result = os.remove(f)
                end
                if not result then
                    logd("Failed to exclude " .. f)
                end
            end
        end
    end
end

local function findNssFile(codeFolder)
    if not isDir(codeFolder) then
        return nil
    end
    for file in lfs.dir(codeFolder) do
        if file:match("%.nss$") then
            return pathJoin(codeFolder, file)
        end
    end
    return nil
end

local function countFilesWithExtension(folder, ext)
    local count = 0
    local files = {}
    if isDir(folder) then
        for file in lfs.dir(folder) do
            if file ~= '.' and file ~= '..' and file:match("%." .. ext .. "$") then
                count = count + 1
                files[#files + 1] = file
            end
        end
    end
    return count, files
end

-- Check NSS file and log info for debugging
local function checkNssFile(nssPath)
    if not isFile(nssPath) then
        return "File does not exist", false
    end
    
    local size = getFileSize(nssPath)
    
    -- Read first bytes for debugging
    local f = io.open(nssPath, "rb")
    if not f then
        return "Cannot open file", false
    end
    
    local header = f:read(16)
    f:close()
    
    if not header then
        return "Cannot read file", false
    end
    
    -- Show first bytes as hex
    local hexStr = ""
    for i = 1, math.min(#header, 16) do
        hexStr = hexStr .. string.format("%02X ", header:byte(i))
    end
    
    -- Check for ELF magic: 0x7F 'E' 'L' 'F' (bytes: 127, 69, 76, 70)
    local isElf = false
    if #header >= 4 then
        local b1, b2, b3, b4 = header:byte(1, 4)
        isElf = (b1 == 0x7F and b2 == 0x45 and b3 == 0x4C and b4 == 0x46)
    end
    
    local info = string.format("Size: %d bytes, Magic: %s, IsELF: %s", size, hexStr, tostring(isElf))
    return info, isElf
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
    logd("template location: " .. template)

    if not isFile(template) then
        return 'Missing template: ' .. template
    end

    -- read settings
    local buildSettingsFile = pathJoin(args.srcDir, 'build.settings')
    local oldSettings = _G['settings']
    _G['settings'] = nil
    pcall( function() dofile(buildSettingsFile) end )
    buildSettings = _G['settings']
    _G['settings'] = oldSettings

    local success = false

    -- create app folder if it does not exists
    local nxsappFolder = pathJoin(args.dstDir, args.applicationName .. '.NX64')
    removeDir(nxsappFolder)
    success = lfs.mkdir(nxsappFolder)
    if not success then
        return 'Failed to create App folder: ' .. nxsappFolder
    end
    logd('AppFolder: ' .. nxsappFolder)

    local appFolder = pathJoin(args.tmpDir, 'nxsapp')
    removeDir(appFolder)
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
    if isDir(luaPluginDir) then
        copyDir(luaPluginDir, pluginExtractDir)
        removeDir(pathJoin(pluginExtractDir, 'lua'))
    end
    luaPluginDir = pathJoin(pluginExtractDir, 'lua_51')
    if isDir(luaPluginDir) then
        copyDir(luaPluginDir, pluginExtractDir)
        removeDir(pathJoin(pluginExtractDir, 'lua_51'))
    end
    if isDir(pluginExtractDir) then
        copyDir(pluginExtractDir, appFolder)
    end

    -- unzip standard template
    local templateFolder = pathJoin(args.tmpDir, 'nxtemplate')
    removeDir(templateFolder)
    success = lfs.mkdir(templateFolder)
    if not success then
        return 'Failed to create template folder: ' .. templateFolder
    end
    logd('templateFolder: ' .. templateFolder)

    -- sanity check
    local archivesize = lfs.attributes(template, "size")
    if archivesize == nil or archivesize == 0 then
        return 'Failed to open template: ' .. template
    end

    local ret = unzip(template, templateFolder, nxInfo)
    if ret ~= 0 then
        return 'Failed to unpack template ' .. template .. ' to ' .. templateFolder ..  ', err=' .. ret
    end
    logd('Unzipped ' .. template .. ' to ' .. templateFolder)

    -- Define paths
    local codeFolder = pathJoin(templateFolder, 'code')
    log('Code folder: ' .. codeFolder)

    -- gather files into appFolder (tmp folder)
    ret = copyDir(args.srcDir, appFolder)
    if ret ~= 0 then
        return "Failed to copy " .. args.srcDir .. ' to ' .. appFolder
    end
    logd("Copied " .. args.srcDir .. ' to ' .. appFolder)

    if args.useStandartResources then
        ret = copyDir(pathJoin(templateFolder, 'res_widget'), appFolder)
        if ret ~= 0 then
            return "Failed to copy standard resources"
        end
        logd("Copied standard resources")
    end

    -- compile .lua
    local rc = compileScriptsAndMakeCAR(args.nxsParams, appFolder, appFolder, args.tmpDir)
    if not rc then
        return "Failed to create .car file"
    end
    logd("Created .car file")

    -- delete .lua, .lu, etc
    deleteUnusedFiles(appFolder, getExcludePredecate())

    -- Copy NRO files from template root to appFolder
    local nroCount = 0
    local nroFiles = {}
    for file in lfs.dir(templateFolder) do
        if file ~= '.' and file ~= '..' and file:match("%.nro$") then
            local src = pathJoin(templateFolder, file)
            local dst = pathJoin(appFolder, file)
            if copyFile(src, dst) then
                log('Copied NRO to appFolder: ' .. file)
                nroCount = nroCount + 1
                nroFiles[#nroFiles + 1] = file
            else
                log('FAILED to copy NRO: ' .. file)
            end
        end
    end
    log('Total NRO files copied: ' .. nroCount)

    -- Copy .nrr folder from template to appFolder
    -- Per docs: ".nrr directory must be under the data region directory"
    local nrrCount = 0
    local nrrFiles = {}
    local appNrrFolder = pathJoin(appFolder, '.nrr')
    
    local nrrSourceLocations = {
        pathJoin(templateFolder, '.nrr'),
        pathJoin(templateFolder, 'nrr'),
    }
    
    local foundNrrSource = nil
    for _, loc in ipairs(nrrSourceLocations) do
        if isDir(loc) then
            local count = countFilesWithExtension(loc, 'nrr')
            if count > 0 then
                foundNrrSource = loc
                log('Found NRR source folder: ' .. loc .. ' with ' .. count .. ' file(s)')
                break
            end
        end
    end
    
    if foundNrrSource then
        lfs.mkdir(appNrrFolder)
        log('Created .nrr folder: ' .. appNrrFolder)
        
        for file in lfs.dir(foundNrrSource) do
            if file ~= '.' and file ~= '..' and file:match("%.nrr$") then
                local src = pathJoin(foundNrrSource, file)
                local dst = pathJoin(appNrrFolder, file)
                if copyFile(src, dst) then
                    local size = lfs.attributes(dst, 'size') or 0
                    log('Copied NRR: ' .. file .. ' (' .. size .. ' bytes)')
                    nrrCount = nrrCount + 1
                    nrrFiles[#nrrFiles + 1] = file
                else
                    log('FAILED to copy NRR: ' .. file)
                end
            end
        end
    else
        log('WARNING: No .nrr folder found in template!')
    end
    
    log('Total NRR files copied: ' .. nrrCount)

    -- Build App
    local metafile = args.nmetaPath
    if not isFile(metafile) then
        return 'Missing ' .. metafile .. ' file'
    end
    log('Using metafile: ' .. metafile)

    local nspfile = pathJoin(nxsappFolder, args.applicationName .. '.nsp')
    local descfile = pathJoin(nxsRoot, 'Resources', 'SpecFiles', 'Application.desc')

    -- Update .npdm file
    local cmd = '"' .. nxsRoot .. '\\Tools\\CommandLineTools\\MakeMeta\\MakeMeta.exe'
    cmd = cmd .. ' --desc ' .. nxsRoot .. '\\Resources\\SpecFiles\\Application.desc'
    cmd = cmd .. ' --meta "' .. metafile .. '"'
    cmd = cmd .. ' -o "' .. pathJoin(codeFolder, 'main.npdm') .. '"'
    cmd = cmd .. '"'
    log('Creating NPDM file...')
    logd('MakeMeta command: ' .. cmd)
    rc, stdout = processExecute(cmd, true)
    log('MakeMeta retcode: ' .. rc)
    if type(stdout) == 'string' and #stdout > 0 then
        log('MakeMeta output: ' .. stdout)
    end

    -- Find .nss file in code folder (REQUIRED by AuthoringTool)
    local nssFile = findNssFile(codeFolder)
    local nssIsValid = false
    
    if nssFile then
        log('Found NSS file: ' .. nssFile)
        local nssInfo
        nssInfo, nssIsValid = checkNssFile(nssFile)
        log('NSS file info: ' .. nssInfo)
        
        if not nssIsValid then
            log('WARNING: NSS file does not have valid ELF header!')
        end
    else
        log('WARNING: No NSS file found in code folder!')
        log('AuthoringTool requires --nss option. Build may fail.')
    end

    local hasNroFiles = nroCount > 0
    local hasNrrFiles = nrrCount > 0

    log('Has NRO files: ' .. tostring(hasNroFiles) .. ' (' .. nroCount .. ')')
    log('Has NRR files: ' .. tostring(hasNrrFiles) .. ' (' .. nrrCount .. ')')

    -- Convert paths to long paths (avoid 8.3 short names that can cause issues)
    local codeFolderLong = getLongPath(codeFolder)
    local appFolderLong = getLongPath(appFolder)
    local nssFileLong = nssFile and getLongPath(nssFile) or nil
    local metafileLong = getLongPath(metafile)
    
    log('Using long paths:')
    log('  codeFolder: ' .. codeFolderLong)
    log('  appFolder: ' .. appFolderLong)
    if nssFileLong then
        log('  nssFile: ' .. nssFileLong)
    end

    -- Build AuthoringTool command
    -- From docs:
    -- AuthoringTool creatensp -o <OUTPUT_FILE>
    --   --desc <desc file>
    --   --meta <nmeta file>
    --   --type Application
    --   --program <code region directory> [<data region directory>]
    --   [--nro <NRO directory>]
    --   --nss <application's NSS file>  -- REQUIRED
    
    cmd = '"' .. nxsRoot .. '\\Tools\\CommandLineTools\\AuthoringTool\\AuthoringTool.exe creatensp'
    cmd = cmd .. ' -o "' .. nspfile .. '"'
    cmd = cmd .. ' --desc "' .. descfile .. '"'
    cmd = cmd .. ' --meta "' .. metafileLong .. '"'
    cmd = cmd .. ' --type Application'
    cmd = cmd .. ' --program "' .. codeFolderLong .. '" "' .. appFolderLong .. '"'
    
    -- --nro specifies directory with NRO files
    if hasNroFiles then
        cmd = cmd .. ' --nro "' .. appFolderLong .. '"'
        log('Added --nro: ' .. appFolderLong)
    end

    -- NSS is REQUIRED by AuthoringTool
    if nssFileLong then
        cmd = cmd .. ' --nss "' .. nssFileLong .. '"'
        log('Added --nss: ' .. nssFileLong)
    else
        log('ERROR: No NSS file available!')
    end

    cmd = cmd .. '"'

    -- Final verification
    log('FINAL CHECK before AuthoringTool:')
    log('  Code region (--program arg1): ' .. codeFolderLong)
    log('  Data region (--program arg2): ' .. appFolderLong)
    if hasNroFiles then
        log('  NRO directory (--nro): ' .. appFolderLong)
    end
    log('  NSS file (--nss): ' .. (nssFileLong or 'MISSING'))
    log('  NSS is valid ELF: ' .. tostring(nssIsValid))
    log('  NRO count: ' .. nroCount)
    log('  NRR count: ' .. nrrCount)
    log('  .nrr folder exists: ' .. tostring(isDir(appNrrFolder)))
    
    if isDir(appNrrFolder) then
        log('  .nrr contents:')
        for file in lfs.dir(appNrrFolder) do
            if file ~= '.' and file ~= '..' then
                local fpath = pathJoin(appNrrFolder, file)
                local size = lfs.attributes(fpath, 'size') or 0
                log('    ' .. file .. ' (' .. size .. ' bytes)')
            end
        end
    end

    log('Building App...')
    log('Command: ' .. cmd)

    rc, stdout = processExecute(cmd, true)
    log('AuthoringTool retcode: ' .. rc)
    if type(stdout) == 'string' and #stdout > 0 then
        log('AuthoringTool output: ' .. stdout)
    end

    if not isFile(nspfile) then
        return 'Failed to build NX Switch App'
    else
        log('Build succeeded: ' .. nspfile)
    end

    return nil
end