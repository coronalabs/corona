------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

-- Tell luacheck not to flag our builtin globals
-- luacheck: globals system
-- luacheck: globals display
-- luacheck: globals native
-- luacheck: globals network
-- luacheck: globals Runtime

-- TODO: Move this out of global namespace
local function getOrCreateTable( receiver, index )
	local t = receiver[index]
	if nil == t then
		t = {}
		receiver[index] = t
	end
	return t
end

local Object = {}
function Object:new( o )
	o = o or {}

	setmetatable( o, self )
	self.__index = self

	return o
end

function Object:newClass( o )
	o = self:new(o)
	o._super = self
	return o
end

--[[
local function printTable( t, label, level )
	if label then print( label ) end
	level = level or 1

	if t then
		for k,v in pairs( t ) do
			local prefix = ""
			for i=1,level do
				prefix = prefix .. "\t"
			end

			print( prefix .. "[" .. tostring(k) .. "] = " .. tostring(v) )
			if type( v ) == "table" then
				print( prefix .. "{" )
				printTable( v, nil, level + 1 )
				print( prefix .. "}" )
			end
		end
	end
end
--]]

-------------------------------------------------------------------------------
-- EventDispatcher
-------------------------------------------------------------------------------

local EventDispatcher = Object:newClass()

EventDispatcher._indexForType = { ["table"]="_tableListeners", ["function"]="_functionListeners" }

function EventDispatcher:getOrCreateTable( eventName, listenerType )
	local index = EventDispatcher._indexForType[ listenerType ]
	local t = nil
	if index then
		t = getOrCreateTable( self, index )
		t = getOrCreateTable( t, eventName )
	end
	if nil == t then
        error("addEventListener: listener cannot be nil: "..tostring(index))
    end
	return t
end

-- Every instance of EventDispatcher has two tables of listeners stored in then
-- _tableListeners and _functionListeners properties. Each of these tables is
-- itself a table that stores arrays keyed by the eventName. For example, a
-- table listener for the event "touch" is stored as an element of the array
-- self._tableListeners["touch"]. We have convenience methods to access the
-- either then _tableListeners or _functionListeners properties.
--
-- Here's an example of an EventDispatcher instance "o" and the contents of its
-- _tableListeners and _functionListeners private properties:
--
--		o._tableListeners =
--		{
--			"touch" = { obj1, obj2, obj3, ... },
--			"enterFrame" = { obj4, obj5, obj6, ... },
--		}
--
--		o._functionListeners =
--		{
--			"touch" = { func1, func2, func3, ... },
--			"enterFrame" = { func4, func5, func6, ... },
--		}
--
function EventDispatcher:addEventListener( eventName, listener )
    -- assume 'self' if listener not specified
    if not listener and self[eventName] then listener = self; end

	-- get table for either function or table listeners
	local listenerType = type( listener )
	local t = self:getOrCreateTable( eventName, listenerType )
	if t then
		table.insert( t, listener )
	end
	return (t ~= nil)
end

-- luacheck: push
-- luacheck: ignore 212 -- Unused argument.
function EventDispatcher:didRemoveListener( eventName )
end
-- luacheck: pop

function EventDispatcher:removeEventListener( eventName, listener )
    -- assume 'self' if listener not specified
    if not listener and self[eventName] then listener = self; end

	-- Find the given event listener in this dispatcher's collection.
	local wasRemoved = false
	local k = EventDispatcher._indexForType[ type(listener) ]
	local listenerTable = self[ k ] -- table either for function or table listeners
	if listenerTable then
		local listenerArray = listenerTable[ eventName ]
		if listenerArray then
			for index = 1, #listenerArray do
				if rawequal( listener, listenerArray[index] ) then
					-- Remove the listener from the event's array of listeners.
					table.remove( listenerArray, index )
					wasRemoved = true

					-- If the event's array of listeners is empty, then remove array from dispatcher's collection.
					if #listenerArray == 0 then
						listenerTable[ eventName ] = nil
					end

					-- Signal an EventDispatcher derived object that a listener has been removed.
					self:didRemoveListener( eventName )
					break
				end
			end
		end
	end

	-- Returns true if the listener was found and removed.
	return wasRemoved or nil
end

function EventDispatcher:hasEventListener( eventName, listener )
    -- assume 'self' if listener not specified
    if not listener and self[eventName] then listener = self; end

	-- Find the given event listener in this dispatcher's collection.
	local k = EventDispatcher._indexForType[ type(listener) ]
	local listenerTable = self[ k ] -- table either for function or table listeners
	if listenerTable then
		local listenerArray = listenerTable[eventName]
		if listenerArray then
			for index = 1, #listenerArray do
				if rawequal( listener, listenerArray[index] ) then
					return true
				end
			end
		end
	end
	return false
end

function cloneArray( array )
	local clone = {}
	for k,v in ipairs( array ) do
		clone[k] = v
	end
	return clone
end

display._initProfiling()

local profiles, augmentedNameStash = {}, {}
local _type = type

local function lookupProfile( name )
	local id = profiles[name]

	if not id and _type( name ) == "string" then
		local aug = name .. " (event)"

		id = display._allocateProfile( aug )
		profiles[name] = id
		augmentedNameStash[#augmentedNameStash + 1] = aug -- prevent collection
	end

	return display._beginProfile( id )
end

local WEBVIEW_JS_CUSTOM_EVENT_PREFIX = "JS_"
function EventDispatcher:dispatchEvent( event )
	local result = false;
	local eventName = event.name
	local isWebviewJSCustomEvent = (event.detail ~= nil and eventName:starts(WEBVIEW_JS_CUSTOM_EVENT_PREFIX))
	local profile

	-- array of functions is self._functionListeners[eventName]
	local functionDict = self._functionListeners
	local functionArray = ( functionDict and functionDict[eventName] ) or nil
	if ( functionArray ~= nil ) and ( #functionArray > 0 ) then
	--	local profile = display._beginProfile( functionArray._profileID )
		profile = lookupProfile( eventName )

		local functionArrayClone = cloneArray( functionArray )
		for index = 1, #functionArrayClone do
			local func = functionArrayClone[ index ]
			display._addProfileEntry( profile, func )
			if self:hasEventListener( eventName, func ) then
				-- Dispatch event to function listener.
				local handled
				if isWebviewJSCustomEvent then
					handled = func( event.detail )
				else
					handled = func( event )
				end
				result = handled or result
			end
		end
	--	display._endProfile( profile )
	end

	-- array of table listeners is self._tableListeners[eventName]
	local tableDict = self._tableListeners
	local tableArray = ( tableDict and tableDict[eventName] ) or nil
	if ( tableArray ~= nil ) and ( #tableArray > 0 ) then
	--	local profile = display._beginProfile( tableArray._profileID )
		profile = profile or lookupProfile( eventName )

		local tableArrayClone = cloneArray( tableArray )
		for index = 1, #tableArrayClone do
			local obj = tableArrayClone[ index ]
			display._addProfileEntry( profile, obj )
			if self:hasEventListener( eventName, obj ) then
				-- Fetch method stored as property of object.
				local method = obj[eventName]
				if ( type(method) == "function" ) then
					-- Dispatch event to table listener.
					local handled
					if isWebviewJSCustomEvent then
						handled = method( obj, event.detail )
					else
						handled = method( obj, event )
					end
					result = handled or result
				end
			end
		end
	--	display._endProfile( profile )
	end
	
	display._endProfile( profile )

	return result
end

function EventDispatcher:respondsToEvent( eventName )
	local t = self._functionListeners
	local result = t and t[eventName]

	if not result then
		t = self._tableListeners
		result = t and t[eventName]
	end

	return result;
end

-- Set up a public function allowing developers to create their own private event dispatchers.
-- Mostly intended for plugin developers. Avoids event name collision with Runtime's event dispatcher.
system.newEventDispatcher = function()
	return EventDispatcher:new()
end


-------------------------------------------------------------------------------
-- ExtendedEventDispatcher
-- * Corona's Runtime and display objects derive from this type.
-- * Globally enables physics event support when the 1st listener is added.
-------------------------------------------------------------------------------

local ExtendedEventDispatcher = EventDispatcher:newClass()

-- For Physics collision events, we need to count *all* event listeners
-- and notify the system whether it should be dispatching those events
local physicsListenerCount =
{
	collision = 0,
	preCollision = 0,
	postCollision = 0,
	particleCollision = 0,
}

function ExtendedEventDispatcher:addEventListener( eventName, listener )
	local wasAdded = EventDispatcher.addEventListener( self, eventName, listener )

	if wasAdded then
		local numListeners = physicsListenerCount[eventName]
		if ( nil ~= numListeners ) then
			if ( 0 == numListeners ) then
				system.beginListener( eventName )
			end
			physicsListenerCount[eventName] = numListeners + 1
		end
	end
	return wasAdded or nil
end

function ExtendedEventDispatcher:removeEventListener( eventName, listener )
	local wasRemoved = EventDispatcher.removeEventListener( self, eventName, listener )

	if wasRemoved then
		local numListeners = physicsListenerCount[eventName]
		if ( nil ~= numListeners ) then
			numListeners = numListeners - 1
			physicsListenerCount[eventName] = numListeners
			if ( 0 >= numListeners ) then
				system.endListener( eventName )
			end
		end
	end

	return wasRemoved or nil
end


-------------------------------------------------------------------------------
-- Runtime
-------------------------------------------------------------------------------

Runtime = ExtendedEventDispatcher:newClass()

Runtime._proxy =
{
	__index = system.__proxyindex,
	__newindex = system.__proxynewindex
}

local needsHardwareSupport = { orientation=true, accelerometer=true, gyroscope=true, location=true, heading=true }

function Runtime:addEventListener( eventName, listener )
	local super = self._super
	local noListeners = not self:respondsToEvent( eventName )
	local wasAdded = super.addEventListener( self, eventName, listener )

	-- If a "key" event listener is installed on a simulated iOS/tvOS/WinPhone device,
	-- warn it wont be effective on a real device
	if eventName and eventName == "key" and system.getInfo("environment") == "simulator" then
		local osName = system.getInfo("platform")
		if osName == "ios" or osName == "tvos" or osName == "winphone" then
			print("WARNING: Runtime:addEventListener: real "..osName.." devices don't generate 'key' events")
		end
	end

	if ( noListeners ) then
		if ( needsHardwareSupport[ eventName ] ) then
			system.beginListener( eventName )
		end
	end
	return wasAdded or nil
end

function Runtime:didRemoveListener( eventName )
	if ( not self:respondsToEvent( eventName ) ) then
		if ( needsHardwareSupport[ eventName ] ) then
			system.endListener( eventName )
		end
	end
end

function Runtime:removeEventListener( eventName, listener )
	local super = self._super
	return super.removeEventListener( self, eventName, listener )
end

-- luacheck: push
-- luacheck: ignore 212 -- Unused argument.
function Runtime:hasEventSource( eventName )
	return system.hasEventSource( eventName )
end

-- TODO: implement in a transparent way
function Runtime:setCheckGlobals( onOff )
	-- Runtime._coronaCheckGlobals = onOff
end
-- luacheck: pop

local function verifyListener( listener, name )
	local listenerType = type( listener )
	if "function" == listenerType then
		return listener
	elseif "table" == listenerType then
		local method = listener[name]
		if type( method ) == "function" then
			return listener
		end
	end
	return nil
end

local function callListener( listener, name, ... )
	local listenerType = type(listener)
	if listenerType == "function" then
		return listener( ... )
	elseif listenerType == "table" then
		return listener[name]( listener, ... )
	end
end

Runtime.verifyListener = verifyListener
Runtime.callListener = callListener

-- Create a publicly available prototype-based base object to create inheritance
-- Don't use Object as the prototype b/c that would expose Object to public manipulation
local PublicObject = {}
function PublicObject:new( o )
	o = o or {}

	setmetatable( o, self )
	self.__index = self

	return o
end

Runtime.Object = PublicObject

-------------------------------------------------------------------------------
-- Per-frame logic
-------------------------------------------------------------------------------

local _frame_id, _frame_start_time

local function onEnterFrameEvent( event )
	_frame_id, _frame_start_time = event.frame, event.time
end

Runtime:addEventListener( "enterFrame", onEnterFrameEvent )

function Runtime.getFrameID( )
	return _frame_id + 1
end

function Runtime.getFrameStartTime( )
	return _frame_start_time
end

-------------------------------------------------------------------------------
-- DisplayObject
-------------------------------------------------------------------------------

local DisplayObject = ExtendedEventDispatcher:new()

system.__proxyregister( "DisplayObject", DisplayObject )

function DisplayObject:addEventListener( eventName, listener )
	local noListeners = not self:respondsToEvent( eventName )
	local wasAdded = ExtendedEventDispatcher.addEventListener( self, eventName, listener )

	if ( noListeners ) then
		self:_setHasListener( eventName, true )
	end
	return wasAdded or nil
end

function DisplayObject:removeEventListener( eventName, listener )
	local wasRemoved = ExtendedEventDispatcher.removeEventListener( self, eventName, listener )

	if ( not self:respondsToEvent( eventName ) ) then
		self:_setHasListener( eventName, false )
	end
	return wasRemoved or nil
end


-------------------------------------------------------------------------------
-- DisplayObject derived "classes"
-------------------------------------------------------------------------------

-- luacheck: push
-- luacheck: ignore 211 -- Unused local variable.
local Group = DisplayObject:newClass()
local ImageGroup = DisplayObject:newClass()
local Sprite = DisplayObject:newClass()
local Container = DisplayObject:newClass()
local Stage = Group:newClass()
local Line = DisplayObject:newClass()
local Shape = DisplayObject:newClass()
local Text = DisplayObject:newClass()
--luacheck: pop

-- print( "DisplayObject = " .. DisplayObject )
-- print( "Group = " .. Group )
-- print( "Container = " .. Container )
-- print( "Stage = " .. Stage )
-- print( "Line = " .. Line )
-- print( "Shape = " .. Shape )
-- print( "Text = " ..  Text )


-------------------------------------------------------------------------------
-- display
-------------------------------------------------------------------------------

local function remoteImageListener( self, event )
	local listener = self.listener

	local target
--	if ( not event.isError and event.status == 200 ) then
--print( "remoteImageListener", event.phase )
	if ( not event.isError and event.phase == "ended" ) then
		target = display.newImage( self.filename, self.baseDir, self.x, self.y )
		event.target = target
	end

	callListener( listener, event.name, event )
end

-- display.loadRemoteImage( url, method, listener [, params], destFilename [, baseDir] [, x, y] )
display.loadRemoteImage = function( url, method, listener, ... )
	local arg = { ... }

	local params, destFilename, baseDir, x, y
	local nextArg = 1
	if "table" == type( arg[nextArg] ) then
		params = arg[nextArg]
		nextArg = nextArg + 1
	end

	if "string" == type( arg[nextArg] ) then
		destFilename = arg[nextArg]
		nextArg = nextArg + 1
	end

	if "userdata" == type( arg[nextArg] ) then
		baseDir = arg[nextArg]
		nextArg = nextArg + 1
	else
		baseDir = system.DocumentsDirectory
	end

	if "number" == type( arg[nextArg] ) and "number" == type( arg[nextArg + 1] ) then
		x = arg[nextArg]
		y = arg[nextArg + 1]
	end

	if ( destFilename ) then
		local o = {
			x=x, y=y,
			filename=destFilename, baseDir=baseDir,
			networkRequest=remoteImageListener, listener=listener }

		if ( params ) then
			network.download( url, method, o, params, destFilename, baseDir )
		else
			network.download( url, method, o, destFilename, baseDir )
		end
	else
		print( "ERROR: no destination filename supplied to display.loadRemoteImage()" )
	end
end

-- convenience wrapper for object:removeSelf() that eliminates check for a receiver that's nil
display.remove = function( object )
	if object then
		local method = object.removeSelf
		if "function" == type( method ) then
			method( object ) -- same as object:removeSelf()
		end
	end
end

-- display function to create retina-compatible text for double-pixel devices
function display.newRetinaText( ... )
	print( "WARNING: display.newRetinaText() has been deprecated. display.newText() is now retina-aware." )
	return display.newText( ... )
end

-------------------------------------------------------------------------------
-- luacheck: push
-- luacheck: ignore 111 -- Setting an undefined global variable.

-------------------------------------------------------------------------------
-- timer
-------------------------------------------------------------------------------

timer = require "timer"


-------------------------------------------------------------------------------
-- Easing
-------------------------------------------------------------------------------

easing = require "easing"

-------------------------------------------------------------------------------
-- transition
-------------------------------------------------------------------------------

transition = require "transition"

-- luacheck: pop
--------------------------------------------------------------------------------

local collectOrphans = display._collectOrphans
local collectgarbage_original = collectgarbage

-- luacheck: push
-- luacheck: ignore 121 -- Setting a read-only global variable.

-- wrap original collectgarbage with newer version
collectgarbage = function( opt, arg )
	-- map to original collectgarbage options
	local opt_lua = opt
	if "collectlua" == opt then
		opt_lua = "collect"
	end

	-- call original collectgarbage
	local result = collectgarbage_original( opt_lua, arg )

	-- new version also collects orphans
	if "collect" == opt or "collectorphans" == opt then
		collectOrphans()
	end

	return result
end

-- luacheck: pop

-- prevent public access to private function
display._collectOrphans = nil

--------------------------------------------------------------------------------

-- Load the "network" library on startup and provide it as a global.
-- This is needed since Corona's initialization code expects this functionality to be available on startup.
-- Note: Wrap it in a pcall() in case the network library has been removed or is unavailble.
network = nil
pcall( function() network = require "network" end )

--[[
network.setStatusListener = function( address, listener )
	local statusListeners = network._statusListeners

	if ( not listener ) then
		-- The Lua GC will implicitly finalize the userdata
		statusListeners[address] = nil
	elseif ( type(address) == "string" ) then
		listener = verifyListener( listener, "networkStatus" )
		if ( listener ) then
			local reachability = network._setStatusListener( address )
			if ( reachability ) then
				-- Store a reference to the reachability userdata so its lifetime matches that of the registered listener
				local data = { listener=listener, userdata=reachability }
				statusListeners[address] = data
			end
		end
	end
end

network._dispatchStatus = function( address, event )
	local data = network._statusListeners[address]
	if data then
		local listener = data.listener
		callListener( listener, event.name, event )
	end
end
--]]

--------------------------------------------------------------------------------
-- luacheck: push
-- luacheck: ignore 122 -- Mutating a read-only global variable.

-- table.indexOf( array, object ) returns the integer index of object in array.
-- Returns 'nil' if not in array. The search goes through the length of the array
-- as determined by #array, whose value is undefined if there are holes.
table.indexOf = function( t, object )
	local result

	if "table" == type( t ) then
		for i=1,#t do
			if object == t[i] then
				result = i
				break
			end
		end
	end

	return result
end

-- table.copy( array, ... ) returns a shallow copy of array, i.e. the portion
-- of the array (table) with integer keys. A variable number of additional
-- arrays can be passed in as optional arguments. If an array has a hole (a nil
-- entry), copying in a given source array stops at the last consecutive item
-- prior to the hole.
--
-- Note: In Lua, the function table.concat() is equivalent to JavaScript's
-- array.join(). Hence, the following function is called copy().
table.copy = function( t, ... )
	-- dst, dstStart are an optional pair
	local copyShallow = function( src, dst, dstStart )
		local result = dst or {}
		local resultStart = 0
		if dst and dstStart then
			resultStart = dstStart
		end
		local resultLen = 0 -- actually, the number of src items copied
		if "table" == type( src ) then
			resultLen = #src
			for i=1,resultLen do
				local value = src[i]
				if nil ~= value then
					result[i + resultStart] = value
				else
					resultLen = i - 1
					break;
				end
			end
		end
		return result,resultLen
	end

	local result, resultStart = copyShallow( t )

	local srcs = { ... }
	for i=1,#srcs do
		local _,len = copyShallow( srcs[i], result, resultStart )
		resultStart = resultStart + len
	end

	return result
end

--------------------------------------------------------------------------------

-- math.round( num ) rounds num to the nearest integer following the same behavior 
-- as the JavaScript version.
math.round = function(num) 
	return math.floor(num+.5)
end

--------------------------------------------------------------------------------

function string.starts( s, prefix )
	return string.sub(s,1,string.len(prefix)) == prefix
end

function string.ends( s, suffix )
	return suffix == '' or string.sub(s,-string.len(suffix)) == suffix
end

-- luacheck: pop

--------------------------------------------------------------------------------
-- Start: Special internal handlers for system events
--------------------------------------------------------------------------------

------------------------------------------------------------------------------------
	-- Start: native.VideoView section (Special internal handlers for system events)
------------------------------------------------------------------------------------
-- VideoViews need to be paused on suspend and resumed on application resume.
-- Note: These variables are also used by native.newVideo override section.
local _weakTableOfAllVideos = {}
local _weakTableOfAllSuspendedVideos = {}
do
	local s_weakmetatable = {}
	s_weakmetatable.__mode = "kv"
	setmetatable(_weakTableOfAllVideos, s_weakmetatable)
	setmetatable(_weakTableOfAllSuspendedVideos, s_weakmetatable)
end

local function ClearSuspendedVideoList()
	-- Do not set the table to {} because I want to reuse the existing table.
	-- Otherwise, I will lose the metatable that specifies the weak-property
	for k, _ in pairs(_weakTableOfAllSuspendedVideos) do
		_weakTableOfAllSuspendedVideos[k] = nil
	end
end

local function SuspendAllPlayingVideos()
	-- erase all entries from the suspended video list to clean it up from possible previous usage
	ClearSuspendedVideoList()

	for k, _ in pairs(_weakTableOfAllVideos) do
		if not k.isPaused then
			-- when object:removeSelf() is called, all we have is a plain Lua table
			-- so check "pause" property to make sure it's a function
			if type( k.pause ) == "function" then
				_weakTableOfAllSuspendedVideos[k] = k
				k:pause()
			else
				-- remove reference to native video object that no longer exists
				_weakTableOfAllVideos[k] = nil
			end
		end
	end
end

local function ResumeAllSuspendedVideos()
	for  k, _ in pairs(_weakTableOfAllSuspendedVideos) do
		if type( k.play ) == "function" then
			k:play()
		end
	end

	-- List is now stale. Clean up
	ClearSuspendedVideoList()
end
------------------------------------------------------------------------------------
	-- End: native.VideoView section (Special internal handlers for system events)
------------------------------------------------------------------------------------

-- luacheck: push
-- luacheck: ignore 542 -- An empty if branch.

local function onInternalSystemEvent( event )
	if  "applicationExit" == event.type then
		-- Create the unique file before exiting

	elseif "applicationOpen" == event.type then

	elseif "applicationSuspend" == event.type then
		SuspendAllPlayingVideos()

	elseif "applicationResume" == event.type then
		ResumeAllSuspendedVideos()
	else

	end
end

-- luacheck: pop

Runtime:addEventListener( "_internalSystem", onInternalSystemEvent )

--------------------------------------------------------------------------------
-- End: Special internal handlers for system events
--------------------------------------------------------------------------------


--------------------------------------------------------------------------------
-- Override/modify standard Lua functions
--------------------------------------------------------------------------------

-- Table to save old functions in case we need them
local _coronaPreservedLuaFunctions = {}
local _coronaBuildSettings
local _coronaBuildSettingsPath

-- Override 'print' because of Android and Apple suppressing stdout/stderr.
-- luacheck: push
-- luacheck: ignore

_coronaPreservedLuaFunctions.print = print
print = coronabaselib.print

-- luacheck: pop

-- luacheck: push
-- luacheck: ignore 121 -- Setting a read-only global variable.

_coronaPreservedLuaFunctions.require = require
require = function (modname)
	if string.find(modname, "/") then
		-- Bug:13760
		-- Override require so we trap users using '/' as separators instead of '.'
		error("Error calling 'require(\"" .. modname .. "\")'. Lua requires package names to use '.' as path separators, not '/'. Replace the '/' characters with '.' and try again.")
	elseif ( "simulator" == system.getInfo( "environment" ) ) or ( "win32" == system.getInfo( "platform" ) ) or ( "macos" == system.getInfo( "platform" ) ) then
		-- Replace '.' with '_' for the following cases:
		-- (1) For plugins
		-- (2) For subclasses CoronaProvider's, e.g. CoronaProvider.gameNetwork.corona.
		-- Note that this does _not_ affect core provider classes (CoronaProvider.gameNetwork)
		local prefix = "plugin."
		if ( string.sub( modname, 1, string.len( prefix ) ) == prefix )
			or ( nil ~= string.match( modname, 'CoronaProvider%.(.*)%.(.*)' ) ) then

			-- Only check for missing plugin config in the Simulators and
			-- not for CoronaProviders
			if ( "simulator" == system.getInfo( "environment" )
				and not string.starts(modname, "CoronaProvider.") ) then
				-- load build.settings so we can see if the plugin is configured
				-- (by caching the path we only load it once per app start)
				local path = system.pathForFile("build.settings", system.ResourceDirectory)
				if path and path ~= _coronaBuildSettingsPath then
					local fp = io.open(path, 'r')

					if fp then
						-- CoronaSDK doesn't expose loadfile() so we need to do it the hard way
						local lua = fp:read( '*a' )
						fp:close()

						local buildSettings = loadstring(lua)
						buildSettings() -- creates "settings" table
						_coronaBuildSettings = settings
						settings = nil
					end

					_coronaBuildSettingsPath = path
				end

				if _coronaBuildSettings == nil then
					_coronaBuildSettings = {}
				end

				if _coronaBuildSettings.plugins == nil then
					_coronaBuildSettings.plugins = {}
				end

				if _coronaBuildSettings.plugins[modname] == nil then
					-- check that this isn't a submodule of an already loaded plugin
					-- i.e. do not warn if this is "plugin.name.subname" and "plugin.name" is loaded
					local guardedRequredName = modname .. '.'
					local submodule = false
					for key, _ in pairs(_coronaBuildSettings.plugins) do
						local guardedSettingsName = key .. "." -- added to stop incorrect substring matches
						if guardedRequredName:starts(guardedSettingsName) or guardedSettingsName:starts(guardedRequredName) then
							submodule = true
							break
						end
					end

					if not submodule then
						local output = "WARNING: "..modname.." is not configured in build.settings"
						-- emit a simplified stack trace (it turns out that all the relevant
						-- info is embodied in the lines ending with "in main chunk")
						output = output .. "\nstack traceback:\n"
						local stackdesc = debug.traceback()
						for line in stackdesc:gmatch("[^\r\n]+") do 
							if string.ends(line, "in main chunk") then
								output = output .. line .. "\n"
							end
						end
						print(output)
					end
				end
			end

			-- Default to original require behavior first first, so `require "plugin.X"`
			-- does the standard thing (convert . to /) which allows subdirectory lookup
			-- to occur. This is useful for pure-Lua plugin developers.
			local result, mod = pcall( _coronaPreservedLuaFunctions.require, modname )
			if ( result ) then
				return mod -- traditional require works fine
			elseif ( "string" == type( mod ) ) then
				-- Determine if this was a Lua syntax error
				if string.starts( mod, "error loading module" ) then
					error( mod, 2 )
				end
			end

			-- To support Lua plugins for Simulator that are hosted on server,
			-- we modify the modname param (convert . to _):
			modname = string.gsub( modname, '%.', '_' )
		end
	end
	return _coronaPreservedLuaFunctions.require( modname )
end

-- luacheck: pop

--------------------------------------------------------------------------------
-- Override/modify Corona Lua functions
--------------------------------------------------------------------------------

_coronaPreservedLuaFunctions.native = _coronaPreservedLuaFunctions.native or {}
_coronaPreservedLuaFunctions.native.newVideo = native.newVideo
_coronaPreservedLuaFunctions.native.canShowPopup = native.canShowPopup
_coronaPreservedLuaFunctions.native.showPopup = native.showPopup
_coronaPreservedLuaFunctions.native.newTextField = native.newTextField
_coronaPreservedLuaFunctions.native.newTextBox = native.newTextBox
_coronaPreservedLuaFunctions.system = _coronaPreservedLuaFunctions.system or {}
_coronaPreservedLuaFunctions.system.getInfo = system.getInfo

-- We need to hijack the newVideo view function so we can keep a list of all living videos
-- so that we can pause them on app suspend and resume them on app resume.
native.newVideo = function (...)
	local video = _coronaPreservedLuaFunctions.native.newVideo(...)
	-- We don't really need both a key and value, but because these objects may be collected,
	-- we can't really use array semantics because there will be holes.
	_weakTableOfAllVideos[video] = video
	return video
end

native._getProvider = function( category, name )
	if not name then
		return nil
	end

	local modName = "CoronaProvider.native." .. category .. "." .. name
	local success, result = pcall( require, modName )

	if not success then
		result = nil
	end

	return result
end

native.canShowPopup = function( ... )
	local result = _coronaPreservedLuaFunctions.native.canShowPopup( ... )

	if not result then
		local name = ...
		local provider = native._getProvider( "popup", name )
		if provider then
			result = provider.canShowPopup( ... )
		end
	end

	return result
end

native.showPopup = function( ... )
	local result = false

	local name = ...
	if _coronaPreservedLuaFunctions.native.canShowPopup( name ) then
		result = _coronaPreservedLuaFunctions.native.showPopup( ... )
	else
		local provider = native._getProvider( "popup", name )
		if provider and "function" == type( provider.showPopup ) then
			result = provider.showPopup( ... )
		end
	end

	return result
end

-- Create a handler used to re-size a native TextField/TextBox font when Corona's content scale changes.
local function createNativeTextObjectHandler()
	-- Create the NativeTextObjectHandler to be returned by this function.
	local self = {}

	-- Initialize private variables for this new handler.
	local nativeObjectCollection = {}
	local lastContentScaleY = nil

	-- Called when the native TextField or TextBox has been removed from the display.
	-- Will remove this object's reference from this handler's collection, allowing it to be garbage collected.
	local function onNativeObjectFinalized(event)
		for index, nativeObject in ipairs(nativeObjectCollection) do
			if (nativeObject == event.target) then
				table.remove(nativeObjectCollection, index)
				return
			end
		end
	end

	-- Called when the the display has been resized.
	-- Re-scales the font size of all native TextFields and TextBoxes if Corona's content scale has changed.
	local function onDisplayResized( _ )
		-- Do not continue if Corona's content scale has not changed.
		if (lastContentScaleY == display.contentScaleY) then
			return
		end

		-- Re-scale the fonts for all native TextFields and TextBoxes.
		if (type(lastContentScaleY) == "number") then
			local scaleY = lastContentScaleY / display.contentScaleY
			local isFontSizeScaled
			for _, nativeObject in ipairs(nativeObjectCollection) do
				-- Set up the object to accept Corona scaled font sizes.
				isFontSizeScaled = nativeObject.isFontSizeScaled
				nativeObject.isFontSizeScaled = true

				-- Scale the font.
				nativeObject.size = nativeObject.size * scaleY

				-- Restore the object's original font sizing mode.
				nativeObject.isFontSizeScaled = isFontSizeScaled
			end
		end

		-- Store the current scale.
		lastContentScaleY = display.contentScaleY
	end
	Runtime:addEventListener("resize", onDisplayResized)

	-- Adds the given native TextField or TextBox to this handler.
	-- This handler will automatically scale its font when Corona's scale factor has changed.
	self.autoScaleFontFor = function(nativeObject)
		-- Store the current content scale, if not done already.
		-- Needed to detect if the scale has changed during a resize event.
		if (lastContentScaleY == nil) then
			lastContentScaleY = display.contentScaleY
		end

		-- Store the given native TextField/TextBox object.
		table.insert(nativeObjectCollection, nativeObject)
		nativeObject:addEventListener("finalize", onNativeObjectFinalized)
	end

	-- Return this native TextField/TextBox handler.
	return self
end
local nativeTextObjectHandler = createNativeTextObjectHandler()

native.newTextField = function( ... )
	-- Create a new text field.
	local textField = _coronaPreservedLuaFunctions.native.newTextField( ... )
	textField.isFontSizeScaled = display.getDefault("isNativeTextFieldFontSizeScaled")

	-- Add a public function which changes the font size to best-fit the field's current height.
	-- Note: Default to the native implementation of this function, if it exists.
	if (type(textField.resizeFontToFitHeight) ~= "function") then
		function textField:resizeFontToFitHeight()
			-- Temporarily enable Corona font scaling.
			local isFontSizeScaled = self.isFontSizeScaled
			self.isFontSizeScaled = true

			-- Fetch the field's margin in content coordinates.
			-- Note: This is the distance between the edge of the field and the text. (ie: Its border size.)
			local margin = self.margin
			if (type(margin) ~= "number") or (margin < 0) then
				margin = 0
			end

			-- Calculate a font size that will best fit the field's height.
			local newFontSize = nil
			local textToMeasure = display.newText("X", 0, 0, self.font)
			if textToMeasure then
				newFontSize = textToMeasure.size * ((textField.contentHeight - (margin * 2)) / textToMeasure.contentHeight)
				textToMeasure:removeSelf()
			end

			-- Apply the new font size that will best fit the height of the field.
			if newFontSize then
				self.size = newFontSize
			end

			-- Restore the field's font scale setting.
			self.isFontSizeScaled = isFontSizeScaled
		end
	end

	-- Add a public function which changes the height of the field to best-fit its current font size.
	-- Note: Default to the native implementation of this function, if it exists.
	if (type(textField.resizeHeightToFitFont) ~= "function") then
		function textField:resizeHeightToFitFont()
			-- Fetch the field's font in content coordinates.
			local isFontSizeScaled = self.isFontSizeScaled
			self.isFontSizeScaled = true
			local textFieldFont = self.font
			self.isFontSizeScaled = isFontSizeScaled

			-- Measure the height of the field's text in content coordinates.
			local textToMeasure = display.newText("X", 0, 0, textFieldFont)
			if (textToMeasure == nil) then
				return
			end
			local textHeight = textToMeasure.contentHeight
			textToMeasure:removeSelf()

			-- Fetch the field's margin in content coordinates.
			-- Note: This is the distance between the edge of the field and the text. (ie: Its border size.)
			local margin = self.margin
			if (type(margin) ~= "number") or (margin < 0) then
				margin = 0
			end

			-- Resize the field's height to fit the text.
			self.height = textHeight + (margin * 2)
		end
	end

	-- Automatically auto scale the field's font when Corona's content scale changes.
	-- This way the text will re-scale proportionally with the field.
	nativeTextObjectHandler.autoScaleFontFor(textField)

	-- By default, change the font size to best-fit the field's current height.
	textField:resizeFontToFitHeight()

	-- Return the new text field.
	return textField
end

native.newTextBox = function( ... )
	local textBox = _coronaPreservedLuaFunctions.native.newTextBox( ... )
	textBox.isFontSizeScaled = display.getDefault("isNativeTextBoxFontSizeScaled")
	nativeTextObjectHandler.autoScaleFontFor(textBox)
	return textBox
end

-- Provide a one line way to turn off runtime error alerts
Runtime.hideErrorAlerts = function( )

	local function _defaultUnhandledErrorListener( _ )
		-- Ignore all runtime errors
		return true
	end

	Runtime:addEventListener("unhandledError", _defaultUnhandledErrorListener)
end

system.getInfo = function( ... )
	-- Perform custom system.getInfo() handling here.
	local keyName = ...
	if (type(keyName) == "string") then
		if ("isoLanguageCode" == keyName) then
			-- *** Make sure that the returned ISO language code is handled consistently on all platforms. ***

			-- Fetch the system's currently selected language as an ISO 639 language code.
			local languageCode = _coronaPreservedLuaFunctions.system.getInfo(...)
			if (type(languageCode) ~= "string") then
				return ""
			end

			-- Make sure the language code is lowercase on all platforms.
			languageCode = string.lower(languageCode)

			-- If an ISO 15924 script code was postfixed to the end of string, such as "zh-hans" or "zh-hant",
			-- then make sure it uses a dash as the separator instead of an underscore for consistency.
			languageCode = string.gsub(languageCode, "%_", "-")

			-- Special handling for Chinese language.
			-- Make sure it always ends with an ISO 15924 script code such as "hans" or "hant".
			if (string.starts(languageCode, "zh")) then
				if (string.ends(languageCode, "hans") == false) and (string.ends(languageCode, "hant") == false) then
					-- Script code not found. Determine which to use based on country code.
					-- Note: Chinese Simplified is used in China and Singapore.
					--       All other countries/regions are assumed to use Chinese Traditional.
					local countryCode = _coronaPreservedLuaFunctions.system.getInfo("isoCountryCode")
					countryCode = string.lower(countryCode)
					if (countryCode == "cn") or (countryCode == "sg") then
						languageCode = languageCode .. "-hans"
					else
						languageCode = languageCode .. "-hant"
					end
				end
			end
			return languageCode
		elseif ("isoCountryCode" == keyName) then
			-- *** Make sure that the returned ISO country code is handled consistently on all platforms. ***

			-- Fetch the system's currently selected region as an ISO 3166-1 country code.
			local countryCode = _coronaPreservedLuaFunctions.system.getInfo(...)
			if (type(countryCode) ~= "string") then
				return ""
			end

			-- Make sure the country code is lowercase on all platforms.
			countryCode = string.lower(countryCode)
			return countryCode
		end
	end

	-- The given key name was not overriden by this module. Let the core function handle it.
	return _coronaPreservedLuaFunctions.system.getInfo( ... )
end

--------------------------------------------------------------------------------
-- Launch Analytics
--------------------------------------------------------------------------------

-- luacheck: push
-- luacheck: ignore 121 -- Setting a read-only global variable.

function sendLaunchAnalytics()
	local buildId
	-- If we are not running in the Simulator, send launch analytics
	if "simulator" ~= system.getInfo( "environment" ) then
		local coronaLaunchAnalytics = require("launchpad")

		if coronaLaunchAnalytics then
			pcall( function() buildId=coronaLaunchAnalytics.launchAnalytics() end )
		end
	end
	return buildId
end

-- luacheck: pop

--------------------------------------------------------------------------------
-- Startup Logging
--------------------------------------------------------------------------------

-- Output support info to debug log
-- "model" is "(null)" for Simulator windows and we don't need to report for those
if system.getInfo("model") ~= "(null)"
then
    print("Platform: " ..tostring(system.getInfo("model")) .." / "
        ..tostring(system.getInfo("architectureInfo")) .." / "
        ..tostring(system.getInfo("platformVersion")) .." / "
        ..tostring(system.getInfo("GL_RENDERER")) .." / "
        ..tostring(system.getInfo("GL_VERSION")) .." / "
        ..tostring(system.getInfo("build")) .." / "
        ..tostring(system.getPreference("ui", "language")) .." | "
        ..tostring(system.getPreference("locale", "country")) .." | "
        ..tostring(system.getPreference("locale", "identifier")) .." | "
        ..tostring(system.getPreference("locale", "language")))

end

-- Provide a way to access the real globals table (used to be needed when we overrode _G and remains
-- for backwards compatibility)
Runtime._G = _G
