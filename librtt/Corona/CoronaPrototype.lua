-------------------------------------------------------------------------------
--
-- Corona Labs
--
-- easing.lua
--
-- Code is MIT licensed; see https://www.coronalabs.com/links/code/license
--
-------------------------------------------------------------------------------

local Prototype = {}

-- Base case:
-- When we create child objects, we set their mt to Prototype,
-- so mt.__index must be Prototype to inherit from Prototype
Prototype.__index = Prototype
Prototype.super = Prototype
Prototype.class = Prototype

--[[
# Summary

Allocates a new prototype-based object based on the receiver

# Examples

## Common case

The table passed to new() is fwd'd to initialize():

`````
local t = { value = 3 }

local Foo = Prototype:newClass( "Foo" )
function Foo:initialize( params )
	print( params.value )
end
Foo:new( t )                --> pass t to initialize() which prints '3'
`````

## Conversion case

The table passed to new() is converted to a class instance:

`````
local t = { value = 3 }

local Bar = Prototype:newClass( "Bar", { shouldNewConvertFirstParameterToInstance = true } )

local b1 = Bar:new( t )     --> convert t to Bar instance instead of passing to initialize()
print( b1.value )           --> 3

local b2 = Bar:new( {}, t ) --> convert table to Bar instance. Pass t to initialize()
print( b2.value )           --> nil
`````
--]]
function Prototype:new( arg1, arg2 )
	-- TODO: Benchmark vararg to table: local args = { ... }
	assert( not arg1 or type(arg1) == "table", "Bad argument #1 to new() (Expected table. Got " .. type( arg1 ) .. ".)" )
	assert( not arg2 or type(arg2) == "table", "Bad argument #2 to new() (Expected table. Got " .. type( arg2 ) .. ".)" )

	-- If this class' constructor accepts parameters,
	-- then pass 'o' to initialize instead of using it as the object
	local params = arg1
	local object = nil
	if self.shouldNewConvertFirstParameterToInstance then
		params = arg2
		object = arg1
	end

	object = object or {}

	setmetatable( object, self )	-- Inherit from receiver's prototype
	object.__index = object			-- Recursive case: Similar to base case above
	object.__metatableOrig = self

	-- assert( self.__index == getmetatable(object).__index )

	-- Initialize 'class' property
	-- otherwise, just propagate self.class
	if self:isClass() then
		object.class = self
	else
		object.class = self.class
	end

	object:initialize( params )

---print( "Prototype:new()", object.name, object )
	return object
end

function Prototype:initialize()
	-- empty stub
end

Prototype._defaults = {
	newClassOptions = {},
	anonymousCount = 0,
}

-- Prototype:newClass( name [, options] )
function Prototype:newClass( name, options )
	options = options or self._defaults.newClassOptions

	local class = self.class:new()
	if not name then
		local anonymousCount = self._defaults.anonymousCount
		name = "CoronaPrototype.anonymous." .. anonymousCount
		self._defaults.anonymousCount = anonymousCount + 1
	end
	class.name = name
	class.super = self
	class.class = class

	class.shouldNewConvertFirstParameterToInstance = options.shouldNewConvertFirstParameterToInstance

	return class
end

function Prototype:isClass()
	-- self is a class if its 'class' property is itself
	return rawget( self, "class" ) == self
end

function Prototype:setExtension( indexFunc )
	-- local indexOrig = getmetatable( self ).__index

	-- Get original metatable
	local mt = self.__metatableOrig
	local indexOrig = mt.__index

	-- Prototype already uses __index for the prototype chain,
	-- so the only way to inject dynamic property accesses is
	-- by injecting indexFunc before the original __index.
	if indexFunc then
		local indexNew
		if type( indexOrig ) == "function" then
			indexNew = function( t, k )
				local v = indexFunc( t, k )
				if nil == v then
					v = indexOrig( t, k )
				end
				return v
			end
		else
			indexNew = function( t, k )
				local v = indexFunc( t, k )
				if nil == v then
					v = indexOrig[ k ]
				end
				return v
			end
		end

		-- Override original metatable
		mt =
		{
			__index = indexNew,
			-- __newindex = getmetatable( self ).__newindex,
		}
	end

	setmetatable( self, mt )
end

function Prototype:instanceOf( class )
	if not class:isClass() then
		return false
	end

	local c = self.class
	while ( c ) do
		if ( c == class ) then
			return true
		elseif ( c == c.super ) then
			break
		end
		c = c.super
	end
--[[
	local selfClass = getmetatable( self )
	while ( selfClass ) do
		if ( selfClass == class ) then
			return true
		else
			selfClass = selfClass.super
		end
	end
--]]
	return false
end

function Prototype:isRoot()
	return self == Prototype
end

return Prototype