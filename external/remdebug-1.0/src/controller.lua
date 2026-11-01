-- Original:
-- RemDebug 1.0 Beta
-- Copyright Kepler Project 2005 (http://www.keplerproject.org/remdebug)
-- 
-- Modifications:
-- Copyright Corona Labs Inc. 2012


local socket = require "socket"

-- Redefine "print" so we can prefix debugger output to distinguish it from app output
local dprefix = "DEBUGGER: "
local saved_print = print
local dprint = function(...)
  if ... == nil then
    saved_print(dprefix .. "nil")
  else
    saved_print(dprefix .. ...)
  end
end
print = dprint

-- Extracts and returns file name and line number from a "setb" or "delb" command line.
local function get_breakpoint_parameters_from(commandLine)
  local expression
  
  -- Select a pattern matching string used to extract the command line arguments.
  -- If it contains quotes, then assume a quoted path containing spaces was given.
  if not string.find(commandLine, '"') then
    expression = "^([%a]+)%s+([%w%p]+)%s+(%d+)$"
  else
    expression = '^([%a]+)%s+(["].-["])%s+(%d+)$'
  end
  
  -- Extract arguments and return them.
  local _, _, _, fileName, lineNumber = string.find(commandLine, expression)
  return fileName, lineNumber
end

-- Combines "path2" to the end of "path1" and returns it as a single path.
-- Correctly handles double quotes and trailing slashes in the paths when combining them.
-- Argument "path2" should not be set to an absolute path. It is expected to be sub-directories
-- and maybe a file name under "path1".
-- If "path1" is empty, then "path2" is returned. If "path2" is empty, then "path1" is returned.
local function combine_paths(path1, path2)
  local expression
  local substring
  local pathSeparator
  local combinedPath

  -- If one of the given paths is empty/nil, then return the other path.
  if (path1 == nil) or (path1 == "") then
    return path2
  end
  if (path2 == nil) or (path2 == "") then
    return path1
  end
  
  -- Guess which path separator to use ('/' or '\') by analyzing the first path.
  pathSeparator = "/"
  if string.find(path1, "\\") then
    pathSeparator = "\\"
  end
  
  -- Remove double quotes from paths.
  expression = '["](.-)["]'
  _, _, substring = string.find(path1, expression)
  if substring then
    path1 = substring
  end
  _, _, substring = string.find(path2, expression)
  if substring then
    path2 = substring
  end
  
  -- Remove periods, slashes, and spaces from the end of path1.
  -- Note: The "(.-)" part of the expression extracts all chars to the left of the chars to be removed.
  _, _, substring = string.find(path1, "(.-)\./\\%s")
  if substring then
    path1 = substring
  end
  
  -- Remove periods, slashes, and spaces from the beginning of path2.
  -- Note: The "(.-)" part of the expression extracts all chars to the right of the chars to be removed.
  _, _, substring = string.find(path2, "\./\\%s(.-)")
  if substring then
    path2 = substring
  end
  
  -- Combine the two paths. Double quote the result if it contains spaces.
  combinedPath = path1 .. pathSeparator .. path2
  if string.find(combinedPath, " ") then
    combinedPath = "\"" .. combinedPath .. "\""
  end
  return combinedPath
end

function get_basedir(path)
  return string.match(path, "(.-)([^\\/]-%.?([^%.\\/]*))$")
end

print("Corona Remote Debugger")
-- Change this to refer to Corona IDE when that's released
-- print("+---------------------------------------------------------+")
-- print("|  Corona Editor provides a better debugging experience   |")
-- print("|  Find out more: https://coronalabs.com/products/editor  |")
-- print("+---------------------------------------------------------+")

-- Start program-under-test

local simulator = ""
local cmd = ""

-- Calculate argc the deterministic way
local argc = 0
for index, a in pairs(arg) do
	-- print(index .. ": " .. a)
	argc = argc + 1
    -- print("arg["..tostring(index).."]: "..tostring(a))
end
-- print("argc: "..tostring(argc))
if argc < 2 or arg[1] ~= "--dont-start-simulator" then
    if arg[0]:find("Corona.Debugger.exe") ~= nil then
      -- The Windows Corona Debugger automatically launches the simulator.
      -- We don't need to do anything else here.
    else -- Mac
      simulator = arg[0]:gsub('debugger$', 'Corona Simulator.app/Contents/MacOS/Corona Simulator')
      cmd = "'".. simulator .."' -debug 1 -singleton 1"

      if argc > 1 then
          cmd = cmd .. " -project '"..arg[1].."'"
      end

      cmd = cmd .. " &"

      -- print("Executing: " .. cmd)
      os.execute(cmd)
    end
else
    -- Remove processed argument and recalculate argc
    arg[1] = nil
    argc = 0
    for index, a in pairs(arg) do
        -- print(index .. ": " .. a)
        argc = argc + 1
        print("arg["..tostring(index).."]: "..tostring(a))
    end
    print("argc: "..tostring(argc))
end

if argc == 1 then
	print("Go to the Simulator and run the program you wish to debug")
end

local server, msg = socket.bind("*", 8171)

if server == nil then
	print("problem communicating with Simulator: "..tostring(msg))
	return
end

local client = server:accept()

local breakpoints = {}
local watches = {}

client:send("STEP\n")
client:receive()

local breakpoint = client:receive()
local _, _, file, line = string.find(breakpoint, "^202 Paused%s+([%w%p]+)%s+(%d+)$")
if file and line then
  print("Paused at file " .. file )
  print("Type 'help' for commands")
else
  local _, _, size = string.find(breakpoint, "^401 Error in Execution (%d+)$")
  if size then
    print("Error in remote application: ")
    print(client:receive(size))
  end
end

local basedir = ""

if argc > 1 then
  basedir, _, _ = get_basedir(arg[1])
end

local aliases = {
  c = "run",
  continue = "run",
  b = "setb",
  bt = "backtrace",
  h = "help",
  f = "frame",
  l = "locals",
  p = "eval",
  d = "dump",
  print = "eval",
  si = "step",
  so = "over",
}

local line = ""
local last_line = ""

while true do
  io.write("> ")
  last_line = line
  line = io.read("*line")
  if line == nil then -- EOF
    line = "exit"
  elseif line == "" then -- blank input
    line = last_line
  end
  local _, _, command = string.find(line, "^([a-z]+)")

  -- if the command is an alias, then translate it
  if command then
    local lookup = aliases[command]
	if lookup then
	  command = lookup
    end
  end

  if command == "run" or command == "step" or command == "over" then
    client:send(string.upper(command) .. "\n")
    client:receive()
    local breakpoint = client:receive()
    if not breakpoint then
      print("Program finished")
      return
    end
    local _, _, status = string.find(breakpoint, "^(%d+)")
    if status == "202" then
      local _, _, label, file, line = string.find(breakpoint, "^202 (%a+)%s+([%w%p]+)%s+(%d+)$")
      if label == "Error" then
        label = "Runtime script error"
      end
      if file and line then 
		if file == "=?" then
          print(label .. " in system library")
		else
          print(label .. " at file " .. file .. " line " .. line)
		end
      end
    elseif status == "203" then
      local _, _, file, line, watch_idx = string.find(breakpoint, "^203 Paused%s+([%w%p]+)%s+(%d+)%s+(%d+)$")
      if file and line and watch_idx then
        print("Paused at file " .. file .. " line " .. line .. " (watch expression " .. watch_idx .. ": [" .. watches[watch_idx] .. "])")
      end
    elseif status == "401" then 
      local _, _, size = string.find(breakpoint, "^401 Error in Execution (%d+)$")
      if size then
        print("Error in remote application: ")
        print(client:receive(tonumber(size)))
        return
      end
    else
      print("Unknown error")
      return
    end
  elseif command == "exit" then
    client:send(string.upper(command) .. "\n")
    client:receive()
    client:close()
    return
  elseif command == "backtrace" or command == "locals" then
    client:send(string.upper(command) .. "\n")
	local line = client:receive()
    local _, _, status, len = string.find(line, "^(%d+)[%s%a]+(%d+)$")
    if status == "200" then
	  if len then
        len = tonumber(len)
        if len > 0 then
          local res = client:receive(len)
		  -- make sure 'DEBUGGER: ' appears at the beginning of each line
		  res = res:gsub('\n', '\nDEBUGGER: ')
          print( res)
		end
      end
    end
  elseif command == "frame" then
    local _, _, exp = string.find(line, "^[a-z]+%s+(%d+)$")
    if exp then
      client:send("FRAME " .. exp .. "\n")
      local line = client:receive()
      local _, _, status = string.find(line, "^(%d+)")
	  if status == "402" then
        print( "Invalid frame. Please specify a frame number for a Lua function" )
      end
    end
  elseif command == "setb" then
    local filename, line = get_breakpoint_parameters_from(line)
    if filename and line then
      filename = combine_paths(basedir, filename)
      if not breakpoints[filename] then breakpoints[filename] = {} end
      client:send("SETB " .. filename .. " " .. line .. "\n")
      if client:receive() == "200 OK" then 
        breakpoints[filename][line] = true
      else
        print("Error: breakpoint not inserted")
      end
    else
      print("Invalid command")
    end
  elseif command == "setw" then
    local _, _, exp = string.find(line, "^[a-z]+%s+(.+)$")
    if exp then
      client:send("SETW " .. exp .. "\n")
      local answer = client:receive()
      local _, _, watch_idx = string.find(answer, "^200 OK (%d+)$")
      if watch_idx then
        watches[watch_idx] = exp
        print("Inserted watch exp no. " .. watch_idx)
      else
        print("Error: Watch expression not inserted")
      end
    else
      print("Invalid command")
    end
  elseif command == "delb" then
    local filename, line = get_breakpoint_parameters_from(line)
    if filename and line then
      filename = combine_paths(basedir, filename)
      if not breakpoints[filename] then breakpoints[filename] = {} end
      client:send("DELB " .. filename .. " " .. line .. "\n")
      if client:receive() == "200 OK" then 
        breakpoints[filename][line] = nil
      else
        print("Error: breakpoint not removed")
      end
    else
      print("Invalid command")
    end
  elseif command == "delallb" then
    for filename, breaks in pairs(breakpoints) do
      for line, _ in pairs(breaks) do
        client:send("DELB " .. filename .. " " .. line .. "\n")
        if client:receive() == "200 OK" then 
          breakpoints[filename][line] = nil
        else
          print("Error: breakpoint at file " .. filename .. " line " .. line .. " not removed")
        end
      end
    end
  elseif command == "delw" then
    local _, _, index = string.find(line, "^[a-z]+%s+(%d+)$")
    if index then
      client:send("DELW " .. index .. "\n")
      if client:receive() == "200 OK" then 
      watches[index] = nil
      else
        print("Error: watch expression not removed")
      end
    else
      print("Invalid command")
    end
  elseif command == "delallw" then
    for index, exp in pairs(watches) do
      client:send("DELW " .. index .. "\n")
      if client:receive() == "200 OK" then 
      watches[index] = nil
      else
        print("Error: watch expression at index " .. index .. " [" .. exp .. "] not removed")
      end
    end
  elseif command == "eval" then
    local _, _, exp = string.find(line, "^[a-z]+%s+(.+)$")
    if exp then 
      client:send("EXEC return (" .. exp .. ")\n")
      local line = client:receive()
      local _, _, status, len = string.find(line, "^(%d+)[a-zA-Z ]+(%d+)$")
      if status == "200" then
        len = tonumber(len)
		if len > 0 then
          local res = client:receive(len)
          print(res)
        end
      elseif status == "401" then
        len = tonumber(len)
        local res = client:receive(len)
        print("Error in expression:")
        print(res)
      else
        print("Unknown error")
      end
    else
      print("Invalid command")
    end
  elseif command == "exec" then
    local _, _, exp = string.find(line, "^[a-z]+%s+(.+)$")
    if exp then 
      client:send("EXEC " .. exp .. "\n")
      local line = client:receive()
      local _, _, status, len = string.find(line, "^(%d+)[%s%w]+(%d+)$")
      if status == "200" then
        len = tonumber(len)
        local res = client:receive(len)
        print(res)
      elseif status == "401" then
        len = tonumber(len)
        local res = client:receive(len)
        print("Error in expression:")
        print(res)
      else
        print("Unknown error")
      end
    else
      print("Invalid command")
    end
  elseif command == "dump" then
    local _, _, exp = string.find(line, "^[a-z]+%s+(.+)$")
    if exp then 
      client:send("DUMP return (" .. exp .. ")\n")
      local line = client:receive()
      local _, _, status, len = string.find(line, "^(%d+)[a-zA-Z ]+(%d+)$")
      if status == "200" then
        len = tonumber(len)
		if len > 0 then
          local res = client:receive(len)
          print(res)
        end
      elseif status == "401" then
        len = tonumber(len)
        local res = client:receive(len)
        print("Error in expression:")
        print(res)
      else
        print("Unknown error")
      end
    else
      print("Invalid command")
    end
  elseif command == "listb" then
    for k, v in pairs(breakpoints) do
      io.write(k .. ": ")
      for k, v in pairs(v) do
        io.write(k .. " ")
      end
      io.write("\n")
    end
  elseif command == "listw" then
    for i, v in pairs(watches) do
      print("Watch exp. " .. i .. ": " .. v)
    end    
  elseif command == "basedir" then
    local _, _, dir = string.find(line, "^[a-z]+%s+(.+)$")
    if dir then
      basedir = dir
      print("New base directory is " .. basedir)
    elseif basedir == "" then
      print("<basedir not set>")
	else
      print(basedir)
    end
  elseif command == "help" then
    print("backtrace(bt)         -- show backtrace")
    print("frame(f) <num>        -- switch to frame <num>")
    print("locals(l)             -- show local variables")
    print("dump(d)               -- prints value of variable ")
    print("setb(b) <file> <line> -- sets a breakpoint")
    print("delb <file> <line>    -- removes a breakpoint")
    print("delallb               -- removes all breakpoints")
    print("setw <exp>            -- adds a new watch expression")
    print("delw <index>          -- removes the watch expression at index")
    print("delallw               -- removes all watch expressions")
    print("run                   -- run until next breakpoint")
    print("continue(c)           -- same as 'run'")
    print("step(si)              -- run until next line, stepping into function calls")
    print("over(so)              -- run until next line, stepping over function calls")
    print("listb                 -- lists breakpoints")
    print("listw                 -- lists watch expressions")
    print("eval <exp>            -- evaluates expression on the current context and returns its value")
    print("print(p) <exp>        -- same as 'eval'")
    print("exec <stmt>           -- executes statement on the current context")
    print("basedir [<path>]      -- sets the base path of the remote application, or shows the current one")
    print("credits               -- shows credit information")
    print("exit                  -- exits debugger")
	print("[blank line]          -- repeats last command")
  elseif command == "credits" then
	print("")
    print("Corona Debugger")
    print("version 1.1")
	print("Copyright © 2008-2023 Solar2D. All rights reserved.")
	print("")
    print("Portions contain:")
	print("    Lua, Copyright © 1994-2008 Lua.org, PUC-Rio.")
	print("    LuaSocket, Copyright © 2004-2007 Diego Nehab.")
	print("    RemDebug, Copyright © 2005 Kepler Project.")
	print("")
  else
    local _, _, spaces = string.find(line, "^(%s*)$")
    if not spaces then
      print("Invalid command")
    end
  end
end
