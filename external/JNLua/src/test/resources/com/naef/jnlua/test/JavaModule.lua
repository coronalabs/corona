--[[
$Id: JavaModule.lua,v 1.1 2008/10/28 16:36:48 anaef Exp $
See LICENSE.txt for license terms.
]]

module(..., package.seeall)

-- java.require() test
function testRequire ()
	-- No import
	local class, imported = java.require("java.lang.System")
	assert(class)
	assert(not imported)
	
	-- Import
	class, imported = java.require("java.lang.System", true)
	assert(class)
	assert(class == java.lang.System)
	assert(imported)
	
	-- Primitive type, no import
	class, imported = java.require("byte")
	assert(class)
	assert(not imported)
	
	-- Primitive type, import
	class, imported = java.require("byte", true)
	assert(class)
	assert(class == byte)
	assert(imported)
end

-- java.new
function testNew ()
	local byte = java.require("byte")
	assert(byte)
	
	-- Simple byte array
	local byteArray = java.new(byte, 10)
	assert(#byteArray == 10)
	local byteArray = java.new("byte", 10)
	assert(#byteArray == 10)
	
	-- Multi-dimensional byte array
	byteArray = java.new(byte, 10, 10)
	assert(#byteArray == 10)
	assert(#byteArray[1] == 10)
end

-- java.instanceof
function testInstanceOf ()
	local TestObject = java.require("com.naef.jnlua.test.fixture.TestObject")
	local testObject = TestObject:new()
	assert(java.instanceof(testObject, TestObject))
	assert(java.instanceof(testObject, "com.naef.jnlua.test.fixture.TestObject"))
	assert(not java.instanceof(testObject, "java.lang.System"))
end

-- java.cast
function testCast ()
	local StringBuilder = java.require("java.lang.StringBuilder")
	local sb = StringBuilder:new()
	sb:append(java.cast(1, "int"))
	assert(sb:toString() == "1")  
end

-- java.proxy
function testProxy ()
	local privilegedAction = { hasRun = false }
	function privilegedAction:run()
		self.hasRun = true
	end
	local proxy = java.proxy(privilegedAction, "java.security.PrivilegedAction")
	assert(not privilegedAction.hasRun)
	local AccessController = java.require("java.security.AccessController")
	AccessController:doPrivileged(proxy)
	assert(privilegedAction.hasRun)
end

-- java.pairs
function testPairs ()
	-- Create map
	local HashMap = java.require("java.util.HashMap")
	local hashMap = HashMap:new()
	hashMap:put("k", "v")
	
	-- Iterate
	local count = 0
	for k, v in java.pairs(hashMap) do
		assert(k == "k")
		assert(v == "v")
		count = count + 1
	end
	assert(count == 1)
	
	-- Create map (navigable)
	local TreeMap = java.require("java.util.TreeMap")
	local treeMap = TreeMap:new()
	treeMap:put("k", "v")
	
	-- Iterate
	local count = 0
	for k, v in java.pairs(treeMap) do
		assert(k == "k")
		assert(v == "v")
		count = count + 1
	end
	assert(count == 1)
end

-- java.ipairs
function testIPairs ()
	-- Create list
	local list = java.new("java.util.ArrayList")
	for i = 1, 10 do
		list:add(i)
	end
	
	-- Iterate
	local count = 0
	local sum = 0
	for k, v in java.ipairs(list) do
		count = count + 1
		assert(k == count)
		sum = sum + v
	end
	assert(count == 10)
	assert(sum == 55)
	
	-- Create array
	local array = java.new("byte", 10)
	for i = 1, 10 do
		array[i] = i
	end
	
	-- Iterate
	local count = 0
	local sum = 0
	for k, v in java.ipairs(array) do
		count = count + 1
		assert(k == count)
		sum = sum + v
	end
	assert(count == 10)
	assert(sum == 55)
end

-- java.tottable
function testToTable ()
	-- Test list
	local arrayList = java.new("java.util.ArrayList")
	local list = java.totable(arrayList)
	for i = 1, 10 do
		list[i] = i
	end
	for i = 1, 10 do
		assert(arrayList:get(i - 1) == i)
	end
	assert(#list == 10)
	local count = 0
	for k, v in java.ipairs(list) do
		count = count + 1
	end
	assert(count == 10)
	
	-- Test map
	local hashMap = java.new("java.util.HashMap")
	local map = java.totable(hashMap)
	map["k"] = "v"
	assert(hashMap:get("k") == "v")
	local count = 0
	for k, v in java.pairs(map) do
		count = count + 1
	end
	assert(count == 1)
end

-- java.elements
function testElements()
	local set = java.new("java.util.HashSet")
	for i = 1, 10 do
		set:add(i)
	end
	sum = 0
	for value in java.elements(set) do
		sum = sum + value
	end
	assert(sum == 55)
end

-- java.fields
function testFields()
	-- Static
	local TestObject = java.require("com.naef.jnlua.test.fixture.TestObject")
	local fields = {}
	local count = 0	
	for k, v in java.fields(TestObject) do
		count = count + 1
		fields[k] = v
	end
	assert(fields["TEST_FIELD"])
	assert(count == 1)

	-- Non-static
	local testObject = java.new(TestObject)
	fields = {}
	count = 0	
	for k, v in java.fields(testObject) do
		count = count + 1
		fields[k] = v
	end
	assert(fields["testField"])
	assert(count == 12)
end

-- java.methods
function testMethods()
	-- Static
	local TestObject = java.require("com.naef.jnlua.test.fixture.TestObject")
	local methods = {}
	local count = 0	
	for k, v in java.methods(TestObject) do
		count = count + 1
		methods[k] = v
	end
	assert(methods["testStatic"])
	assert(count == 2)

	-- Non-static
	local testObject = java.new(TestObject)
	methods = {}
	count = 0	
	for k, v in java.methods(testObject) do
		count = count + 1
		methods[k] = v
	end
	assert(methods["test"])
	assert(count == 16)
end

-- java.properties
function testProperties()
	local TestObject = java.require("com.naef.jnlua.test.fixture.TestObject")
	local testObject = java.new(TestObject)
	local properties = {}
	local count = 0	
	for k, v in java.properties(testObject) do
		count = count + 1
		properties[k] = v
	end
	assert(properties["value"])
	assert(count == 3)
end
