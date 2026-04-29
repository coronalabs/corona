--- Provides a reuseable and convenient framework for creating classes in Lua.
-- Two possible notations: <br> <code> B = class(A) </code> or <code> class.B(A) </code>. <br>
-- <p>The latter form creates a named class. </p>
-- See the Guide for further <a href="../../index.html#class">discussion</a>
-- @module pl.class

local error, getmetatable, io, pairs, rawget, rawset, setmetatable, tostring, type =
    _G.error, _G.getmetatable, _G.io, _G.pairs, _G.rawget, _G.rawset, _G.setmetatable, _G.tostring, _G.type
-- this trickery is necessary to prevent the inheritance of 'super' and
-- the resulting recursive call problems.
local function call_ctor (c,obj,...)
    -- nice alias for the base class ctor
    local base = rawget(c,'_base')
    if base then obj.super = rawget(base,'_init') end
    local res = c._init(obj,...)
    obj.super = nil
    return res
end

local function is_a(self,klass)
    local m = getmetatable(self)
    if not m then return false end --*can't be an object!
    while m do
        if m == klass then return true end
        m = rawget(m,'_base')
    end
    return false
end

local function class_of(klass,obj)
    if type(klass) ~= 'table' or not rawget(klass,'is_a') then return false end
    return klass.is_a(obj,klass)
end

local function _class_tostring (obj)
    local mt = obj._class
    local name = rawget(mt,'_name')
    setmetatable(obj,nil)
    local str = tostring(obj)
    setmetatable(obj,mt)
    if name then str = name ..str:gsub('table','') end
    return str
end

local function tupdate(td,ts)
    for k,v in pairs(ts) do
        td[k] = v
    end
end

local function _class(base,c_arg,c)
    c = c or {}     -- a new class instance, which is the metatable for all objects of this type
    -- the class will be the metatable for all its objects,
    -- and they will look up their methods in it.
    local mt = {}   -- a metatable for the class instance

    if type(base) == 'table' then
        -- our new class is a shallow copy of the base class!
        tupdate(c,base)
        c._base = base
        -- inherit the 'not found' handler, if present
        if rawget(c,'_handler') then mt.__index = c._handler end
    elseif base ~= nil then
        error("must derive from a table type",3)
    end

    c.__index = c
    setmetatable(c,mt)
    c._init = nil

    if base and rawget(base,'_class_init') then
        base._class_init(c,c_arg)
    end

    -- expose a ctor which can be called by <classname>(<args>)
    mt.__call = function(class_tbl,...)
        local obj = {}
        setmetatable(obj,c)

        if rawget(c,'_init') then -- explicit constructor
            local res = call_ctor(c,obj,...)
            if res then -- _if_ a ctor returns a value, it becomes the object...
                obj = res
                setmetatable(obj,c)
            end
        elseif base and rawget(base,'_init') then -- default constructor
            -- make sure that any stuff from the base class is initialized!
            call_ctor(base,obj,...)
        end

        if base and rawget(base,'_post_init') then
            base._post_init(obj)
        end

        if not rawget(c,'__tostring') then
            c.__tostring = _class_tostring
        end
        return obj
    end
    -- Call Class.catch to set a handler for methods/properties not found in the class!
    c.catch = function(handler)
        c._handler = handler
        mt.__index = handler
    end
    c.is_a = is_a
    c.class_of = class_of
    c._class = c
    -- any object can have a specified delegate which is called with unrecognized methods
    -- if _handler exists and obj[key] is nil, then pass onto handler!
    c.delegate = function(self,obj)
        mt.__index = function(tbl,key)
            local method = obj[key]
            if method then
                return function(self,...)
                    return method(obj,...)
                end
            elseif self._handler then
                return self._handler(tbl,key)
            end
        end
    end
    return c
end

--- create a new class, derived from a given base class. <br>
-- Supporting two class creation syntaxes:
-- either <code>Name = class(base)</code> or <code>class.Name(base)</code>
-- @class function
-- @name class
-- @param base optional base class
-- @param c_arg optional parameter to class ctor
-- @param c optional table to be used as class
local class
class = setmetatable({},{
    __call = function(fun,...)
        return _class(...)
    end,
    __index = function(tbl,key)
        if key == 'class' then
            io.stderr:write('require("pl.class").class is deprecated. Use require("pl.class")\n')
            return class
        end
        local env = _G
        return function(...)
            local c = _class(...)
            c._name = key
            rawset(env,key,c)
            return c
        end
    end
})


return class

