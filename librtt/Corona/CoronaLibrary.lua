-------------------------------------------------------------------------------
--
-- Corona Labs
--
-- easing.lua
--
-- Code is MIT licensed; see https://www.coronalabs.com/links/code/license
--
-------------------------------------------------------------------------------

local Prototype = require "CoronaPrototype"

local classOptions = { shouldNewConvertFirstParameterToInstance = true }
local Library = Prototype:newClass( "Library", classOptions )

-- Defaults
Library.name = "anonymous"
Library.publisherId = "anonymous"
Library.version = 1
Library.revision = 1

local anonymousCount = 1

-- Modify constructor to create a new class
function Library:new( o )
	-- Each library is a singleton, so create a class for it,
	-- so we are free to modify its metatable without disturbing other classes
	local newSelf = Library.new
	local newSuper = Library.super.new

	-- Temporarily replace 'new' with the original and then restore
	-- This ensures newClass() calls the right constructor
	Library.new = newSuper -- original 'new'
	local class = Library:newClass( o.name )
	local library = class:new( o )
	Library.new = newSelf -- modified 'new'

	return library
end

function Library:initialize()
	assert(
		not rawget( self, "name" ) or type( rawget( self, "name" ) ) == "string",
		"Expected string for library property 'name'. Got " .. type( rawget( self, "name" ) ) .. " instead." )
	assert(
		not rawget( self, "publisherId" ) or type( rawget( self, "publisherId" ) ) == "string",
		"Expected string for library property 'publisherId'. Got " .. type( rawget( self, "publisherId" ) ) .. " instead." )
	assert(
		not rawget( self, "version" ) or type( rawget( self, "version" ) ) == "number",
		"Expected number for library property 'version'. Got " .. type( rawget( self, "version" ) ) .. " instead." )
	assert(
		not rawget( self, "revision" ) or type( rawget( self, "revision" ) ) == "number",
		"Expected number for library property 'revision'. Got " .. type( rawget( self, "revision" ) ) .. " instead." )

	-- Assign default name
	if ( not rawget( self, "name" ) ) then
		self.name = Library.name .. anonymousCount
		anonymousCount = anonymousCount + 1
	end

	-- If the library has providers, initialize the library to use the default provider
	if self.usesProviders then
		self:initializeProvider()
	end

	return self
end

function Library:initializeProvider()
	local provider = self:requireProvider()
	if type( provider ) == 'table' then
		self._provider = provider
		self._providerName = nil

		-- Instances of 'provider' provide the real implementation
		-- so we forward accesses to the provider. We wrap this inside the
		-- closure '__index' so that we can change provider dynamically.
		local function __index( t, k )
			return rawget( self, '_provider' )[k]
		end
		self:setExtension( __index )	-- forward property accesses to provider
	end
end

function Library:providerNameToModuleName( providerName )
	local modName = 'CoronaProvider.' .. self.name

	if providerName then
		modName = modName .. "." .. providerName
	end

	return modName
end

local isSimulator = system.getInfo( "environment" ) == "simulator"

-- Pass 'nil' for providerName to get default provider
function Library:requireProvider( providerName )
	local modName = self:providerNameToModuleName( providerName )
	local success, result = pcall( require, modName )

	if not success then
		if not isSimulator then
			assert( success, string.format( "ERROR: Could not load provider (%s) due to the following reason: %s.", tostring(providerName), tostring(result) ) )
		else
			print( "WARNING: The '" .. tostring( self.name ) .. "' provider (" .. tostring( providerName ) .. ") is not available on the simulator" )
		end

		result = nil
	end
	
	return result
end

function Library:getProvider()
	return self._provider
end

-- To the caller, providers are just names. Internally, they are Lua objects
function Library:getCurrentProvider()
	return self._providerName
end

-- To the caller, providers are just names. Internally, they are Lua objects
function Library:setCurrentProvider( providerName )
	local result = false

	if providerName ~= self._providerName then
		local provider, msg = self:requireProvider( providerName )
		if provider then
			result = true
			self._provider = provider
			self._providerName = providerName
		else
			local modName = self:providerNameToModuleName( providerName )
			local isSimulator = system.getInfo( "environment" ) == "simulator"
			if not isSimulator then
				print( "ERROR: Could not find provider (" .. modName .. ") for library (" .. self.name .. ")." )
			end
---			print( "[foo]", msg )
		end
	end

	return result
end

--[[
local function CopyParamTypes( src )
	local result
	if ( src and #src > 0 ) then
		result = {}
	
		for i=1,#src do
			local value = src[i]
			assert( "string" == type( value ) )
			table.insert( result, value )
		end
	end

	return result
end

local charForType =
{
	boolean = b,
	string = s,
	integer = d,		-- 'd' for %d (int) in printf 
	number = f,			-- 'f' for %f (double) in printf
	self = o,			-- 'o' for object
	table = t,
	userdata = p,		-- 'p' for pointer
	lightuserdata = l,
}

local function CreateSignature( paramTypes, resultTypes )
	local result
	if ( paramTypes and #paramTypes > 0 ) then
		local types = {}

		for i=1,#paramTypes do
			local t = paramTypes[i]
			local value = charForType[t]
			table.insert( types, value )
		end

		table.insert( types, '>' )

		if ( resultTypes ) then
			for i=1,#resultTypes do
				local t = paramTypes[i]
				local value = charForType[t]
				table.insert( types, value )
			end
		end

		result = table.concat( types )
	end

	return result
end

-- Export function 'name' so it's available in C
function Library:defineSignature( name, paramTypes, resultTypes )
	assert( type( name ) == "string" )
	assert( type( paramTypes ) == "nil" or type( paramTypes ) == "table" )

	-- Lazily create methods table
	local signatures = self._signatures
	if ( not signatures ) then
		signatures = {}
		self._signatures = signatures
	end

	-- Ensure not already defined
	local result = not signatures[name]
	if ( result ) then
		signatures[name] = CreateSignature( paramTypes, resultTypes )
	else
		local msg = string.format(
			'WARNING: Signature (%s) already defined in library (%s).', name, self.name )
		print( msg )
	end

	return result
end

function Library:getSignature( name )
	local result

	local signatures = self._signatures
	if ( signatures ) then
		result = signatures[name]
	end

	return result
end

-- Export methods to C
Library:defineSignature( 'new', { "self", "table" }, { "table" } )
Library:defineSignature( 'defineFunction', { "self", "string", "table", "table" }, { "boolean" } )
--]]

return Library
