--
-- RemDebug 1.0 Beta
-- Copyright Kepler Project 2005 (http://www.keplerproject.org/remdebug)
--

local socket = require"socket"
--local lfs = require"lfs"
local debug = require"debug"
local json = require("json")

module("remdebug.engine", package.seeall)

_COPYRIGHT = "2006 - Kepler Project"
_DESCRIPTION = "Remote Debugger for the Lua programming language"
_VERSION = "1.0"


-- Extracts and returns file name and line number from a "setb" or "delb" command line.
local function get_breakpoint_parameters_from(commandLine)
  local expression
  local substring
  
  -- Select a pattern matching string used to extract the command line arguments.
  -- If it contains quotes, then assume a quoted path containing spaces was given.
  if not string.find(commandLine, '"') then
    expression = "^([%a]+)%s+([%w%p]+)%s+(%d+)$"
  else
    expression = '^([%a]+)%s+(["].-["])%s+(%d+)$'
  end
  
  -- Extract arguments.
  local _, _, _, fileName, lineNumber = string.find(commandLine, expression)
  
  -- Remove double quotes from retrieved file name string.
  -- This way the debug_hook() function below can lookup breakpoints by file name only.
  _, _, substring = string.find(fileName, '["](.-)["]')
  if substring then
    fileName = substring
  end
  --[[
  -- Remove path from retrieved file name string.
  _, _, substring = string.find(fileName, ".-([^/\\]+)$")
  if substring then
    fileName = substring
  end
  --]]

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


local coro_debugger
local events = { BREAK = 1, WATCH = 2 }
local breakpoints = {}
local watches = {}
local step_into = false
local step_over = false
local step_level = 0
local stack_level = 0
local start_frame_index = 3
local error_event = false

local controller_host = "localhost"
local controller_port = 8171

local function set_breakpoint(file, line)
print( "set_breakpoint("..file..","..line..")" )
  if not breakpoints[file] then
    breakpoints[file] = {}
  end
  breakpoints[file][line] = true  
end

local function remove_breakpoint(file, line)
  if breakpoints[file] then
    breakpoints[file][line] = nil
  end
end

local function has_breakpoint(file, line)
  return breakpoints[file] and breakpoints[file][line]
end

local function restore_vars(frame, vars)
  if type(vars) ~= 'table' then return end
  local func = debug.getinfo(frame, "f").func
  local i = 1
  local written_vars = {}
  while true do
    local name = debug.getlocal(frame, i)
    if not name then break end
    debug.setlocal(frame, i, vars[name])
    written_vars[name] = true
    i = i + 1
  end
  i = 1
  while true do
    local name = debug.getupvalue(func, i)
    if not name then break end
    if not written_vars[name] then
      debug.setupvalue(func, i, vars[name])
      written_vars[name] = true
    end
    i = i + 1
  end
end

local function restore_stack(stack)
  local stack = {}
  local frameNum = (not error_event) and start_frame_index or start_frame_index + 1
  for i,frame in ipairs(stack) do

    -- Calling debug.getlocal in a different function frame from this
    -- current one screws up the frame number index passed to debug.getlocal
    -- restore_vars inlined to avoid screwing up the frame number
    local vars = frame.vars
    if type(vars) ~= 'table' then return end
    local func = debug.getinfo(frameNum, "f").func
    local i = 1
    local written_vars = {}
    while true do
      local name = debug.getlocal(frameNum, i)
      if not name then break end
      debug.setlocal(frameNum, i, vars[name])
      written_vars[name] = true
      i = i + 1
    end
    i = 1
    while true do
      local name = debug.getupvalue(func, i)
      if not name then break end
      if not written_vars[name] then
        debug.setupvalue(func, i, vars[name])
        written_vars[name] = true
      end
      i = i + 1
    end

    -- restore_vars( frameNum, frame.vars )
    frameNum = frameNum + 1
  end
end

local function capture_stack()
  local stack = {}
  local frameNum = (not error_event) and start_frame_index or start_frame_index + 1
  while true do
--    local frame = capture_frame(frameNum)
--	if not frame then break end

    local info = debug.getinfo(frameNum, "Snlf")
    if not info then
      break
    end

    -- Calling debug.getlocal in a different function frame from this
    -- current one screws up the frame number index passed to debug.getlocal

--print( "Frame("..frameNum..") of type: "..tostring(info.what) )

    -- capture_vars inlined to avoid screwing up the frame number
    local vars = {}
    local func = info.func
    local i = 1

    if func then
      while true do
        local name, value = debug.getupvalue(func, i)
        if not name then break end
        vars[name] = value
        i = i + 1
      end
      i = 1
    end

    while true do
      local name, value = debug.getlocal(frameNum, i)
      if not name then break end
--print( "[" .. frameNum .. "] " .. tostring(name) .. " = " .. tostring(value) )
      vars[name] = value
      i = i + 1
    end
    setmetatable(vars, { __index = getfenv(func), __newindex = getfenv(func) })

    -- capture_frame
    local frame = {}  
    frame.vars = vars
    frame.what = info.what
    frame.name = info.name or "(nil)"
    frame.file = info.source -- info.short_src
    frame.line = info.currentline
--[[
    print( "Frame("..frameNum..")" )
    for k,v in pairs(frame) do
      print( k .. ":" .. tostring(v) )
    end
    print( "" )
--]]
    table.insert(stack, frame)
	frameNum = frameNum + 1
  end
  return stack
end

local function break_dir(path) 
  local paths = {}
  path = string.gsub(path, "\\", "/")
  for w in string.gfind(path, "[^\/]+") do
    table.insert(paths, w)
  end
  return paths
end

local function merge_paths(path1, path2)
  local paths1 = break_dir(path1)
  local paths2 = break_dir(path2)
  for i, path in ipairs(paths2) do
    if path == ".." then
      table.remove(paths1, table.getn(paths1))
    elseif path ~= "." then
      table.insert(paths1, path)
    end
  end
  return table.concat(paths1, "/")
end

local function debug_hook(event, line, msg)
--print( "event("..tostring(event)..") line("..tostring(line)..")" )
  if event == "call" then
    stack_level = stack_level + 1
  elseif event == "return" or event == "tail return" then
    stack_level = stack_level - 1
  else
    local level = 2
	if event == "error" then
      if msg then print( msg ) end
      level = level + 1
      error_event = true
	end
    local file = debug.getinfo(level, "S").source
    if string.find(file, "@") == 1 then
      file = string.sub(file, 2)
    end

    -- TODO: Find a better solution to the basedir problem
	-- For now, strip path info off file.
    local _, _, filename = string.find(file, ".-([^/\\]+)$")
	-- if filename then file = filename end
--    file = merge_paths(lfs.currentdir(), file)
--print( file .."("..tostring(line)..")")
--    local vars = capture_vars()

--print( "event("..tostring(Event)..") at "..tostring(file) .. ":" ..tostring(line) )
    local stack = capture_stack()
	local frame = stack[1]
	local vars = frame.vars
    table.foreach(watches, function (index, value)
      setfenv(value, vars)
      local status, res = pcall(value)
      if status and res then
        coroutine.resume(coro_debugger, events.WATCH, stack, file, line, index)
      end
    end)

    if step_into or (step_over and stack_level <= step_level) or has_breakpoint(file, line) or event == "error" then
      step_into = false
      step_over = false
      coroutine.resume(coro_debugger, events.BREAK, stack, file, line)
      restore_stack(stack)
    end
  end
end

local function format_data_for_display(data, pretty_print_json)

	local pretty_print_json = pretty_print_json or false
	local value = data
	local typename = type(data)

	if typename == "string" then
		-- Enclose strings in double quotes
		value = '"' .. value .. '"'
	elseif typename == "function" then
		local functionInfo = debug.getinfo(data, "S")
		if functionInfo.source == "=[C]" then
			value = "internal"
		else
			value = string.format("%s:%d", functionInfo.source, functionInfo.linedefined)
		end
	elseif typename == "table" then
		-- If this is a Corona internal type it'll have these properties
		if data._type ~= nil then
			typename = data._type
		end
		if data._properties ~= nil then
			value = data._properties
		else
			-- Format the table as JSON
			local pcall_status, pretty_val = pcall(json.encode, data)

			if not pcall_status then
				-- Something went wrong, emit the unformatted data
				value = "[raw] "..tostring(data)
			else
				-- We'll use "pretty_val", tidy it up
				value = pretty_val:gsub("<type '(.-)' is not supported by JSON.>", "<%1>")
			end
		end

		if pretty_print_json then
			value = json.prettify(value)
		end
	end

	return tostring(typename), tostring(value)
end

local function debugger_loop(server)
  local command
  local eval_env = {}
  local frameIndex = 1

  while true do
    local line, status = server:receive()
    command = string.sub(line, string.find(line, "^[A-Z]+"))
	-- print("remdebug: line: "..tostring(line))
	-- print("remdebug: command: "..tostring(command))

    if command == "SETB" then
      local filename, line = get_breakpoint_parameters_from(line)
      if filename and line then
        set_breakpoint(filename, tonumber(line))
        server:send("200 OK\n")
      else
        server:send("400 Bad Request\n")
      end

    elseif command == "DELB" then
      local filename, line = get_breakpoint_parameters_from(line)
      if filename and line then
        remove_breakpoint(filename, tonumber(line))
        server:send("200 OK\n")
      else
        server:send("400 Bad Request\n")
      end

    elseif command == "EXEC" then
      local _, _, chunk = string.find(line, "^[A-Z]+%s+(.+)$")
      if chunk then 
        local func = loadstring(chunk)
        local status, res
        if func then
          setfenv(func, eval_env[frameIndex].vars)
          status, res = xpcall(func, debug.traceback)
        end
        res = tostring(res)
        if status then
          server:send("200 OK " .. string.len(res) .. "\n") 
          server:send(res)
        else
          server:send("401 Error in Expression " .. string.len(res) .. "\n")
          server:send(res)
        end
      else
        server:send("400 Bad Request\n")
      end

    elseif command == "DUMP" then
      local _, _, chunk = string.find(line, "^[A-Z]+%s+(.+)$")
      if chunk then 
        local func = loadstring(chunk)
        local status, res
        if func then
          setfenv(func, eval_env[frameIndex].vars)
          status, res = xpcall(func, debug.traceback)
        end

        if status then
          local _, _, expr = string.find(chunk, "return %((.*)%)")

		  typename, value = format_data_for_display(res, true)

		  if expr and typename and value then
			response = string.format("%s = (%s) %s", expr, typename, value)
		  else
			response = "Could not evaluate '"..tostring(expr).."' (perhaps it's not defined yet)"
		  end

          server:send("200 OK " .. string.len(response) .. "\n") 
          server:send(response)
        else
          res = tostring(res)
          server:send("401 Error in Expression " .. string.len(res) .. "\n")
          server:send(res)
        end
      else
        server:send("400 Bad Request\n")
      end

    elseif command == "SETW" then
      local _, _, exp = string.find(line, "^[A-Z]+%s+(.+)$")
      if exp then 
        local func = loadstring("return(" .. exp .. ")")
        local newidx = table.getn(watches) + 1
        watches[newidx] = func
        -- table.setn(watches, newidx)
        server:send("200 OK " .. newidx .. "\n") 
      else
        server:send("400 Bad Request\n")
      end

    elseif command == "DELW" then
      local _, _, index = string.find(line, "^[A-Z]+%s+(%d+)$")
      index = tonumber(index)
      if index then
        watches[index] = nil
        server:send("200 OK\n") 
      else
        server:send("400 Bad Request\n")
      end


    elseif command == "RUN" then
      server:send("200 OK\n")
	  print("RUN: breakpoints: "..json.encode(breakpoints))
      local ev, stack, file, line, idx_watch = coroutine.yield()
      eval_env = stack
      if ev == events.BREAK then
        local label = (not error_event and "Paused " or "Error " )
        server:send("202 " .. label .. file .. " " .. line .. "\n")
      elseif ev == events.WATCH then
        server:send("203 Paused " .. file .. " " .. line .. " " .. idx_watch .. "\n")
      else
        server:send("401 Error in Execution " .. string.len(file) .. "\n")
        server:send(file)
      end

    elseif command == "STEP" then
      server:send("200 OK\n")
      step_into = true
      local ev, stack, file, line, idx_watch = coroutine.yield()
      eval_env = stack
      if ev == events.BREAK then
        local label = (not error_event and "Paused " or "Error " )
        server:send("202 " .. label .. file .. " " .. line .. "\n")
      elseif ev == events.WATCH then
        server:send("203 Paused " .. file .. " " .. line .. " " .. idx_watch .. "\n")
      else
        server:send("401 Error in Execution " .. string.len(file) .. "\n")
        server:send(file)
      end

    elseif command == "OVER" then
      server:send("200 OK\n")
      step_over = true
      step_level = stack_level
      local ev, stack, file, line, idx_watch = coroutine.yield()
      eval_env = stack
      if ev == events.BREAK then
        local label = (not error_event and "Paused " or "Error " )
        server:send("202 " .. label .. file .. " " .. line .. "\n")
      elseif ev == events.WATCH then
        server:send("203 Paused " .. file .. " " .. line .. " " .. idx_watch .. "\n")
      else
        server:send("401 Error in Execution " .. string.len(file) .. "\n")
        server:send(file)
      end

    elseif command == "BACKTRACE" then
      local stacktrace = {}
	  for i,frame in ipairs(eval_env) do
	    local prefix = ( i == frameIndex and "*" ) or " "
		local s
		-- print("json.version: "..tostring(json.version))
		-- Can't use json for frames because they use userdata as array indices
	    -- print("remdebug: frame.name: "..tostring(frame.name))
	    -- print("          frame.file: "..tostring(frame.file))
	    -- print("          frame.line: "..tostring(frame.line))
		if frame.what ~= "C" then
          s = string.format("%s[%3d] %s at %s:%d", prefix, i, frame.name, frame.file, frame.line )
        else
		  s = string.format("%s[%3d] (Runtime)", prefix, i )
		end
		table.insert(stacktrace, s)
	  end
      stacktrace = table.concat(stacktrace, "\n")
      if not stacktrace then
        server:send("200 OK " .. 0 .. "\n")
      else
        server:send("200 OK " .. string.len(stacktrace) .. "\n")
        server:send(stacktrace)
      end

    elseif command == "FRAME" then
      local _, _, _, index = string.find(line, "^([A-Z]+)%s+([%d]+)$")
      index = tonumber(index)
      if index > #eval_env then
        index = #eval_env
      end
      if eval_env[index].what ~= "C" then
        frameIndex = index
        server:send("200 OK\n")
      else
        server:send("402 Invalid frame index number\n")
      end

    elseif command == "LOCALS" then
      local ignore = { __FILE__ = true, __LINE__ = true }
      local vars = {}
      local frame = eval_env[frameIndex]
      if frame then
        for k,v in pairs(frame.vars) do
          if k == "(*temporary)" or ignore[k] then
			  -- do nothing
          else
			typename, value = format_data_for_display(v)

			table.insert(vars, string.format("%s = (%s) %s",
					tostring(k), typename, value))
          end
        end
      end
      local msg = table.concat(vars,"\n")
      if msg == nil then
        server:send("200 OK " .. 0 .. "\n")
      else
        server:send("200 OK " .. string.len(msg) .. "\n")
        server:send(msg)
      end
    else
      server:send("400 Bad Request\n")
    end
  end
end

coro_debugger = coroutine.create(debugger_loop)

--
-- remdebug.engine.config(tab)
-- Configures the engine
--
function config(tab)
  if tab.host then
    controller_host = tab.host
  end
  if tab.port then
    controller_port = tab.port
  end
end

--
-- remdebug.engine.start()
-- Tries to start the debug session by connecting with a controller
--
function start()
  -- pcall(require, "remdebug.config")
  local server = socket.connect(controller_host, controller_port)
  if server then
    _TRACEBACK = function (message) 
      local err = debug.traceback(message)
      server:send("401 Error in Execution " .. string.len(err) .. "\n")
      server:send(err)
      server:close()
      return err
    end
    debug.sethook(debug_hook, "lcr")
    return coroutine.resume(coro_debugger, server)
  end
end

