local Object = {}
function Object:new(o)
	o = o or {}
	setmetatable(o,self)
	self.__index = self
	return o
end

function Object:newClass(o)
	o = self:new(o)
	o._super = self
	return o
end


local Foo = Object:newClass()
local Bar = Foo:newClass()

function Foo:baz()
	print( "Foo:baz" )
end

function Bar:baz()
	print( "Bar:baz() begin" )
	self._super:baz()
	print( "Bar:baz() end" )
end

local foo = Foo:new()
local bar = Bar:new()

foo:baz()
bar:baz()
