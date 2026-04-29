------------------------------------------------------------------------------
--
-- This file is part of the Corona game engine.
-- For overview and more information on licensing please refer to README.md 
-- Home page: https://github.com/coronalabs/corona
-- Contact: support@coronalabs.com
--
------------------------------------------------------------------------------

-- A sample of using JS native plugin for Corona

local p = require "myplugin"
local json = require "json"

 
print(" --- Properties Getters: --- ")
print("strProp:",     p.strProp                  , '('.. type(p.strProp) .. ')')
print("u8StrProp:",   p.u8StrProp                , '('.. type(p.u8StrProp) .. ')')
print("boolProp:",    p.boolProp                 , '('.. type(p.boolProp) .. ')')
print("floadProp:",   p.floadProp                , '('.. type(p.floadProp) .. ')')
print("intProp:",     p.intProp                  , '('.. type(p.intProp) .. ')')
print("unexistant:",  p.unexistant               , '('.. type(p.unexistant) .. ')')

print("arrProp:",     json.encode(p.arrProp)     , '('.. type(p.arrProp) .. ')')
print("objProp:",     json.encode(p.objProp)     , '('.. type(p.objProp) .. ')')


-- simple function calling
print("empty", p.empty, p.empty());
print("echo", p.echo("Hello", nil, 1, 2, 3, true, {1,2,3}, {ob="ject"}))


-- function with return
print('get("strProp")', p.get("strProp"))
print(p.echo("Hello", nil, 1, 2, 3, true, {1,2,3}, {ob="ject"}))

print(json.encode(p.echo1({"Hello", nil, 1, 2, 3, true, {1,2,3}, {ob="ject"}})))

-- assigning functions to properties
p.sayHello = function(from)
	print("Hello from "..from)
	return from
end
local f1 = p.callSayHello('JS')
local f2 = p.sayHello('Lua')
print("Bffs forever: " .. tostring(f1) .. " & " .. tostring(f2))


local widget = require "widget"
local toggleBtn = widget.newButton({
	label = "test widget callbacks",
	textOnly = true,
	x = display.contentCenterX,
	y = display.contentCenterY + 80,
	onRelease = p.log
})
  


function cmp(a,b)
	local t = type(a)
	if t ~= type(b) then return false end
	if t ~= 'table' then return a == b end
	for k1,v1 in pairs(a) do
		local v2 = b[k1]
		if not cmp(v1,v2) then return false end
	end
	for k2,v2 in pairs(b) do
		local v1 = a[k2]
		if not cmp(v1,v2) then return false end
	end
	if #a ~= #b then return false end
	for i=1,#a do
		if not cmp(a[i], b[i]) then return false end
	end
	return true
end


local r = display.newRect( display.contentCenterX, display.contentCenterX, 200, 100 )
local t = display.newText( "test leaks", display.contentCenterX, display.contentCenterX, nil, 20 )
t:setFillColor( 0,0,0 )

local function fnc()end
local i = 1

local function runSynchronousTests( additionalTests )
	-- simple property types
	assert(type(p.strProp) == 'string', "type of property 'strProp' is wrong: " .. type(p.strProp) )
	assert(type(p.u8StrProp) == 'string', "type of property 'u8StrProp' is wrong: " .. type(p.u8StrProp) )
	assert(type(p.boolProp) == 'boolean', "type of property 'boolProp' is wrong: " .. type(p.boolProp) )
	assert(type(p.floadProp) == 'number', "type of property 'floadProp' is wrong: " .. type(p.floadProp) )
	assert(type(p.intProp) == 'number', "type of property 'intProp' is wrong: " .. type(p.intProp) )
	assert(type(p.unexistant) == 'nil', "type of property 'unexistant' is wrong: " .. type(p.unexistant) )
	assert(type(p.arrProp) == 'table', "type of property 'arrProp' is wrong: " .. type(p.arrProp) )
	assert(type(p.objProp) == 'table', "type of property 'objProp' is wrong: " .. type(p.objProp) )

	-- simple property value checks
	assert(p.strProp == "Hello World!", "value of property 'strProp' is wrong: " .. tostring(p.strProp) )
	assert(p.u8StrProp == "👋 🌎!", "value of property 'u8StrProp' is wrong: " .. tostring(p.u8StrProp) )
	assert(p.boolProp == true, "value of property 'boolProp' is wrong: " .. tostring(p.boolProp) )
	assert(p.floadProp == 3.14, "value of property 'floadProp' is wrong: " .. tostring(p.floadProp) )
	assert(p.intProp == 42, "value of property 'intProp' is wrong: " .. tostring(p.intProp) )
	assert(p.unexistant == nil, "value of property 'unexistant' is wrong: " .. tostring(p.unexistant) )
	assert(cmp(p.arrProp, {1,2,3}), "value of property 'arrProp' is wrong: " .. json.encode(p.arrProp) )
	assert(cmp(p.objProp, {message = "Hello World", arr = {1,2,3}}), "value of property 'objProp' is wrong: " .. json.encode(p.objProp) )

	-- skipping functions in objects and array properties
	assert(type(p.skipArr) == 'table', "type of property 'skipArr' is wrong: " .. type(p.skipArr) )
	assert(type(p.skipObj) == 'table', "type of property 'skipObj' is wrong: " .. type(p.skipObj) )

	assert(cmp(p.skipArr, {1,2,3,nil,nil,4,nil,5}), "value of property 'arrProp' is wrong: " .. json.encode(p.skipArr) )
	assert(cmp(p.skipObj, {arr={1,2,nil,3}, trailing={}, message="Hello World"} ), "value of property 'skipObj' is wrong: " .. json.encode(p.skipObj) )
	assert(p.skipObj.f == nil, "value of property 'skipObj.f' is wrong: " .. json.encode(p.skipObj.f) )


	-- basic function properties
	assert(type(p.empty) == 'function', 'epty is not a function')
	assert(p.empty() == nil, 'Empty returned something!')
	assert(type(p.echo) == 'function', 'echo is not a function')
	assert(p.echo("Hello", nil, 1, 2, 3, true, {1,2,3}, {ob="ject"}) == '{"data":["Hello",null,1,2,3,true,[1,2,3],{"ob":"ject"}]}', 'Echo return is rong' .. tostring(p.echo("Hello", nil, 1, 2, 3, true, {1,2,3}, {ob="ject"})))


	-- return type checks
	assert(type(p.r_strProp()) == 'string', "return type of 'r_strProp' is wrong: " .. type(p.r_strProp()) )
	assert(type(p.r_u8StrProp()) == 'string', "return type of 'r_u8StrProp' is wrong: " .. type(p.r_u8StrProp()) )
	assert(type(p.r_boolProp()) == 'boolean', "return type of 'r_boolProp' is wrong: " .. type(p.r_boolProp()) )
	assert(type(p.r_floadProp()) == 'number', "return type of 'r_floadProp' is wrong: " .. type(p.r_floadProp()) )
	assert(type(p.r_intProp()) == 'number', "return type of 'r_intProp' is wrong: " .. type(p.r_intProp()) )
	assert(type(p.r_arrProp()) == 'table', "return type of 'r_arrProp' is wrong: " .. type(p.r_arrProp()) )
	assert(type(p.r_objProp()) == 'table', "return type of 'r_objProp' is wrong: " .. type(p.r_objProp()) )

	-- return value checks
	assert(p.r_strProp() == "Hello World!", "return value of 'r_strProp' is wrong: " .. tostring(p.r_strProp()) )
	assert(p.r_u8StrProp() == "👋 🌎!", "return value of 'r_u8StrProp' is wrong: " .. tostring(p.r_u8StrProp()) )
	assert(p.r_boolProp() == true, "return value of 'r_boolProp' is wrong: " .. tostring(p.r_boolProp()) )
	assert(p.r_floadProp() == 3.14, "return value of 'r_floadProp' is wrong: " .. tostring(p.r_floadProp()) )
	assert(p.r_intProp() == 42, "return value of 'r_intProp' is wrong: " .. tostring(p.r_intProp()) )
	assert(cmp(p.r_arrProp(), {1,2,3}), "return value of 'r_arrProp' is wrong: " .. json.encode(p.r_arrProp()) )
	assert(cmp(p.r_objProp(), {message = "Hello World", arr = {1,2,3}}), "return value of 'r_objProp' is wrong: " .. json.encode(p.r_objProp()) )

	-- this binding
	assert(p.get("strProp") == "Hello World!", 'get("strProp") returned: ' .. tostring(p.get("strProp")));

	-- property setting/deleting
	p.set("anotherProp", "Hello 🌎!")
	assert(p.anotherProp == "Hello 🌎!", "set didn't work" .. p.anotherProp)

	p.set("anotherProp", nil)
	assert(p.anotherProp == nil, "p.set didn't remove property")

	p.newProp = {message = "Hello World", arr = {1,2,3}}
	assert(cmp(p.get("newProp"), {message = "Hello World", arr = {1,2,3}}), "setting proeprty didn't work " .. json.encode(p.get("newProp")))

	p.newProp = nil
	assert(cmp(p.get("newProp"), nil), "deleting proeprty didn't work")

	-- setting function properties
	p.sayHello = fnc
	p.callSayHello()
	
	-- this is for memory leaks mostly
	p.sayHello = function() end
	p.callSayHello()

	p.sayHello = function() return 1 end
	assert(1 == p.callSayHello())

	p.sayHello = function(n) return n end
	assert(i == p.callSayHello(i))
	i = i + 1

	p.testFnc = function(a)
		return a
	end
	assert(type(p.testFnc) == 'function', 'property isn not function')
	assert(cmp({1,2,3}, p.testFnc({1,2,3})), 'test echo func failed ' .. json.encode(p.testFnc({1,2,3})))

	-- passing functions as parameters
	p.empty(fnc)
	p.simpleCallbacker(fnc)

	local cb1, cb2 = 0, 0
	assert(p.withComplexCallbacks(function(v) cb1 = v end, { func = function(v) cb2 = v end}), "Error while invoking callbackers")
	assert(cb1 == 42, "Callback 1 didn't set value")
	assert(cb2 == 42, "Callback 2 didn't set value")

	-- function return function
	assert(42 == p.doubleFun(42, function(b) return function() return b end end), "double function trouble")

	if not additionalTests then
		return
	end
	
	local displayObject = display.newRect(0,0, 100,100)
	p.log(displayObject)
	displayObject:removeSelf()

end

local function asyncTests()
	-- passing function as callback for later
	local var = 0
	p.asyncTest({param=42, delay=500, callback=function(param) var = param end})
	assert(var == 0, "Async set var too early")
	timer.performWithDelay(1000, function()
		assert(var == 42, "Async din't set var!")

		r:setFillColor(0,1,0)
	end)
end

local function testMemLeaks(nCount)

	nCount = nCount or 3000
	print("Measuring memory leaks on " .. tostring(nCount) .. " runs. It can take a while.")

	runSynchronousTests()

	collectgarbage()
	local n = collectgarbage("count")
	for i=1,nCount do
		runSynchronousTests()
	end
	print('waiting for releases')
	-- js releases garbage after 1ms
	timer.performWithDelay(1000, function()
		collectgarbage()
		local n2 = collectgarbage("count")
		print("Memory: ", n, " -> ", n2, ' difference ', n2-n)
		local l = (n2-n)*1000/nCount
		print("Difference per cycle is " .. tostring(l) .. " bytes")

		t.text = "Leak: " .. string.format("%.2f", l)
		assert(l<5, "Leaking more than 5 byte per cycle...")
		r.isRunningTest = false
		r:setFillColor( 1,1,1 )
	end)
end

--tests:
if system.getInfo( 'platform' ) == 'html5' then
	r.isRunningTest = true
	timer.performWithDelay(300, function()
	print("Running tests...")
		runSynchronousTests(true);
		asyncTests()
		r.isRunningTest = false
	end)

end


r:addEventListener( "tap", function(  )
	if not r.isRunningTest then
		r.isRunningTest = true
		r:setFillColor( 1,0,0 )
		timer.performWithDelay( 100, function( )
			testMemLeaks(math.random(500,3000))
		end )
	end
end )

require('scripts.s1').test()
require('scripts.s2').test()

timer.performWithDelay(100, function() 
	require('scripts.undefined').test()
end)

timer.performWithDelay(100, function() 
	require('scripts.missing').test()
end)
