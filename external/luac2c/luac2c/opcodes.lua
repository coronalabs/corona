
require "luac2c.write_c"

LFIELDS_PER_FLUSH = 50

-- code generation ---------------------------------------------------

local function push_value_or_constant(fn, x)
   if x >= 0 then
      writer:lua('lua_pushvalue', idx(x))
   elseif type(fn.constants[-x]) == "number" then
      writer:lua('lua_pushnumber', fn.constants[-x])
   elseif type(fn.constants[-x]) == "boolean" then
      if fn.constants[-x] then
         writer:lua('lua_pushboolean', 1)
      else
         writer:lua('lua_pushboolean', 0)
      end
   elseif type(fn.constants[-x]) == "string" then
      count = string.gsub(fn.constants[-x],"\\%d%d%d", "x")
      count = string.gsub(count, "\\.", "x")
      len = string.len(count)
      writer:lua('lua_pushlstring','"'..fn.constants[-x]..'"',len)
   elseif type(fn.constants[-x]) == "nil" then
      writer:lua('lua_pushnil')
   end
end

local function code_result_at(r, stack)
   if r ~= stack then
      writer:lua('lua_replace', idx(r))
   end
end

local function code_unary_operation(ins, fn, c_op, meta_op, stack)
   local a, b = ins.a, ins.b
   writer:begin_block()
   writer:var('lua_Number','o1')
   if b < 0 and type(fn.constants[-b]) == "number" then
      writer:lua('lua_pushnumber',c_op.." "..fn.constants[-c])
   else
      if b >= 0 then
         writer:if_lua('lua_isnumber',idx(b))
         writer:set_lua('o1','lua_tonumber',idx(b))
         writer:lua('lua_pushnumber',c_op..' o1')
         writer:or_else()
      elseif type(fn.constants[-b]) == "number" then
         writer:lua('lua_pushnumber',c_op..fn.constants[-b])
         writer:or_else()
      else
         -- operation on string constants; use metatable
         writer:begin_block()
      end
      writer:var("int","m",0)
      if b >= 0 then
         writer:set_lua("m","luaL_getmetafield",idx(b),'"'..meta_op..'"')
      end
      writer:begin_if('!m')
         writer:lua('lua_pushstring','"operation not defined on given operand"')
         writer:lua('lua_error')
      writer:end_block()
      push_value_or_constant(fn, b)
      writer:lua('lua_call',1,1)
      writer:end_block()
   end
   code_result_at(a, stack)
   writer:end_block()
end

local function code_binary_operation(ins, fn, c_op, meta_op, stack)
   local a, b, c = ins.a, ins.b, ins.c

   writer:begin_block()
   writer:var('lua_Number','o1')
   writer:var('lua_Number','o2')
   if b < 0 and c < 0 and type(fn.constants[-b]) == "number" and type(fn.constants[-c]) == "number" then
      writer:lua('lua_pushnumber',fn.constants[-b].." "..c_op.." "..fn.constants[-c])
   else
      if b >= 0 and c >= 0 then
         writer:begin_if("lua_isnumber(L, "..idx(b)..") && lua_isnumber(L, "..idx(c)..")")
         writer:set_lua('o1','lua_tonumber',idx(b))
         writer:set_lua('o2','lua_tonumber',idx(c))
         if (string.sub(c_op, -1) == "(") then
            writer:lua('lua_pushnumber',c_op..'(o1, o2)')
         else
            writer:lua('lua_pushnumber','o1 '..c_op..'o2')
         end
         writer:or_else()
      elseif b >= 0 and type(fn.constants[-c]) == "number" then
         writer:if_lua('lua_isnumber',idx(b))
         writer:set_lua('o1','lua_tonumber',idx(b))
         writer:set('o2',fn.constants[-c])
         writer:lua('lua_pushnumber','o1 '..c_op..' o2')
         writer:or_else()
      elseif c >= 0 and type(fn.constants[-b]) == "number" then
         writer:if_lua('lua_isnumber',idx(c))
         writer:set('o1',fn.constants[-b])
         writer:set_lua('o2','lua_tonumber',idx(c))
         writer:lua('lua_pushnumber','o1 '..c_op..' o2')
         writer:or_else()
      else
         -- operation on string constants; use metatable
         writer:begin_block()
      end
      writer:var('int','m',0)
      if b >= 0 then
         writer:set_lua("m","luaL_getmetafield",idx(b),'"'..meta_op..'"')
      end
      if c >= 0 then
         writer:begin_if("!m")
         writer:set_lua("m","luaL_getmetafield",idx(c),'"'..meta_op..'"')
         writer:end_block()
      end
      writer:begin_if("!m")
         writer:lua('lua_pushstring','"operation not defined on given operands"')
         writer:lua('lua_error')
      writer:end_block()
      push_value_or_constant(fn, b)
      push_value_or_constant(fn, c)
      writer:lua('lua_call',2,1)
      writer:end_block()
   end
   code_result_at(a, stack)
   writer:end_block()
end

local function push_if_constant(fn, x)
   if x < 0 then
      push_value_or_constant(fn, x)
      return true
   else
      return false
   end
end

local function make_comparison_params(ins, fn)
   local idx_b = idx(ins.b)
   local idx_c = idx(ins.c)
   local pop = 0
   if push_if_constant(fn, ins.b) == true then
      idx_b = -1
      pop = 1
   end
   if push_if_constant(fn, ins.c) == true then
      idx_c = -1
      pop = pop + 1
      if idx_b == -1 then idx_b = -2 end
   end
   return idx_b, idx_c, pop
end

local function comparison(fn, ins, lua_fn)
   writer:begin_block()
   writer:var('int','result',0)
   local idx_b, idx_c, pop = make_comparison_params(ins, fn)
   writer:set_lua('result',lua_fn,idx_b,idx_c)
   if pop > 0 then
      writer:lua('lua_pop',pop)
   end
   writer:begin_if((ins.a == 1 and '!' or '')..'result')
      writer:goto(ins.id+2)
   writer:end_block()
   writer:end_block()
end

local function lessequal(ins, fn)
   local a, b, c = ins.a, ins.b, ins.c

   local may_be_string = true
   writer:begin_block()
      writer:var('int','result',0)
      if b < 0 and type(fn.constants[-b]) ~= "string" then
         may_be_string = false
      elseif c < 0 and type(fn.constants[-b]) ~= "string" then
         may_be_string = false
      end
      local idx_b, idx_c, pop = make_comparison_params(ins, fn)
      writer:begin_if('lua_isnumber(L, '..idx_b..') && lua_isnumber(L, '..idx_c..")")
         writer:var('lua_Number','o1')
         writer:var('lua_Number','o2')
         writer:set_lua('o1','lua_tonumber',idx_b)
         writer:set_lua('o2','lua_tonumber',idx_c)
         writer:set('result','(o1 <= o2)')
      if may_be_string then
         writer:else_if('lua_isstring(L, '..idx_b..') && lua_isstring(L, '..idx_c..")")
            writer:var('const char*',"l")
            writer:var('const char*',"r")
            writer:var('size_t',"ll")
            writer:var('size_t',"lr")
            writer:set_lua("l",'lua_tolstring',idx_b,'&ll')
            writer:set_lua("r",'lua_tolstring',idx_c,'&lr')
            writer:comment("Lua string comparison, based on lvm.c")
            writer:begin_for("","","")
               writer:var('int','temp','strcoll(l, r)')
               writer:begin_if('temp != 0')
                  writer:set('result','temp <= 0')
                  writer:brk()
               writer:or_else()
                  writer:var('size_t','len','strlen(l)')
                  writer:begin_if('len == lr')
                     writer:set('result','((len == ll) ? 0 : 1) <= 0')
                     writer:brk()
                  writer:else_if('len == ll')
                     writer:set('result','-1 <= 0')
                     writer:brk()
                  writer:end_block()
                  writer:inc('len')
                  writer:inc("l",'len')
                  writer:dec("ll",'len')
                  writer:inc("r",'len')
                  writer:dec("lr",'len')
               writer:end_block()
            writer:end_block()
      end
      writer:or_else()
         writer:var('int',"m",0)
         if b >= 0 then
            writer:set_lua("m",'luaL_getmetafield',idx_b,'"__le"')
         end
         if c >= 0 then
            writer:begin_if("!m")
               writer:set_lua("m",'luaL_getmetafield',idx_c,'"__le"')
            writer:end_block()
         end
         writer:begin_if("m")
            push_value_or_constant(fn, b)
            push_value_or_constant(fn, c)
            writer:lua('lua_call',2,1)
            writer:set_lua('result','lua_toboolean',-1)
            writer:lua('lua_pop',1)
         writer:or_else()
            if b >= 0 then
               writer:set_lua("m",'luaL_getmetafield',idx_b,'"__lt"')
            end
            if c >= 0 then
               writer:begin_if("!m")
                  writer:set_lua("m",'luaL_getmetafield',idx_c,'"__lt"')
               writer:end_block()
            end
            writer:begin_if("m")
               push_value_or_constant(fn, c)
               push_value_or_constant(fn, b)
               writer:lua('lua_call',2,1)
               writer:set_lua('result','!lua_toboolean',-1)
               writer:lua('lua_pop',1)
            writer:end_block()
         writer:end_block()
         writer:begin_if("!m")
            writer:lua('lua_pushstring','"operation not defined on given operands"')
            writer:lua('lua_error')
         writer:end_block()
      writer:end_block()
      if pop > 0 then
         writer:lua('lua_pop',pop)
      end
      writer:begin_if((a == 1 and '!' or '')..'result')
         writer:goto(ins.id+2)
      writer:end_block()
   writer:end_block()
end

function idx(x)
   if shift_base then
      return "(base+"..(x+1)..")"
   else
      return (x+1)
   end
end

-- opcode parameter types --------------------------------------------

function param_type(op)
   return ops[op].param_type
end

-- stack analysis ----------------------------------------------------

local function stack_result_at_a(ins, stack)
   assert(ins.a <= stack, ins.op.." storing result beyond ("..ins.a..") top of stack ("..stack..")")
   if ins.a == stack then
      stack = stack + 1
   end
   return stack
end

local function stack_unchanged(ins, stack)
   return stack
end

-- register analysis -------------------------------------------------

function read_r(r, x)
   if x < 0 then return end
   if (not r[x]) then r[x] = "read" end
end

function write_r(r, x)
   if x < 0 then return end
   if (not r[x]) then r[x] = "write" end
end

local function register_unchanged()
   -- no op
end

local function register_read_b_write_a(ins, r)
   read_r(r,ins.b)
   write_r(r, ins.a)
end

local function register_read_b_c(ins, r)
   read_r(r,ins.b)
   read_r(r,ins.c)
end

local function register_read_b_c_write_a(ins, r)
   read_r(r,ins.b)
   read_r(r, ins.c)
   write_r(r, ins.a)
end

local function register_write_a(ins, r)
   write_r(r, ins.a)
end

local function register_read_a(ins, r)
   read_r(r, ins.a)
end

-- block boundaries --------------------------------------------------

function block_boundary_next(ins)
   return ins.id + 1
end

local function block_boundary_next_and_following(ins, curr, add_edge)
   if curr then
      add_edge(curr, ins.id + 1)
      add_edge(curr, ins.id + 2)
   else
      return ins.id + 1, ins.id + 2
   end
end

local function block_boundary_next_and_jump(ins, curr, add_edge, add_edge_if_needed)
   if curr then
      add_edge(curr, ins.id + ins.sbx + 1)
      add_edge_if_needed(ins.id + ins.sbx)
   else
      return ins.id + 1, ins.id + ins.sbx + 1
   end
end

local function block_boundary_forloop(ins, curr, add_edge, add_edge_if_needed)
   if curr then
      add_edge(curr, ins.id + 1)
      add_edge(curr, ins.id + ins.sbx + 1)
      add_edge_if_needed(ins.id + ins.sbx)
   else
      return ins.id + 1, ins.id + ins.sbx + 1
   end
end

local function block_boundary_loadbool(ins, curr, add_edge, add_edge_if_needed)
   if ins.c == 1 then
      if curr then
         add_edge(curr, ins.id + 2)
         add_edge_if_needed(ins.id + 1)
      else
         return ins.id + 1, ins.id + 2
      end
   end
end

----------------------------------------------------------------------

ops = {

   -- R(A) := R(B)
   MOVE = {
      param_type = "AB",
      -- block_boundary = nil,
      register = register_read_b_write_a,
      stack = stack_result_at_a,
      code = function(ins, fn, stack)
         push_value_or_constant(fn, ins.b)
         code_result_at(ins.a, stack)
      end
   },

   -- R(A) := Kst(Bx)
   LOADK = {
      param_type = "ABx",
      -- block_boundary = nil,
      register = register_write_a,
      stack = stack_result_at_a,
      code = function(ins, fn, stack)
         push_value_or_constant(fn, ins.bx)
         code_result_at(ins.a, stack)
      end
   },

   -- R(A) := (Bool)B; if (C) pc++
   LOADBOOL = {
      param_type = "ABC",
      block_boundary = block_boundary_loadbool,
      register = register_write_a,
      stack = stack_result_at_a,
      code = function(ins, fn, stack)
         writer:lua('lua_pushboolean',ins.b)
         code_result_at(ins.a, stack)
         if ins.c ~= 0 then
            writer:goto(ins.id + 2)
         end
      end
   },

   -- R(A) := ... := R(B) := nil
   LOADNIL = {
      param_type = "AB",
      -- block_boundary = nil,
      register = function(ins, r)
         for x = ins.a, ins.b do
            write_r(r, x)
         end
      end,
      stack = function(ins, stack)
         return math.max(stack, ins.b+1)
      end,
      code = function(ins, fn, stack)
         if ins.b < stack then
            for i = ins.a, ins.b do
               writer:lua('lua_pushnil')
               writer:lua('lua_replace',idx(i))
            end
         else
            if ins.a < stack then
               writer:lua('lua_pop','stack - '..idx(ins.a))
            end
            writer:lua('lua_settop',idx(ins.b))
         end
      end
   },

   -- R(A) := UpValue[B]
   GETUPVAL = {
      param_type = "AB",
      register = register_write_a,
      stack = stack_result_at_a,
      code = function(ins, fn, stack)
         writer:lua('lua_pushvalue','lua_upvalueindex('..idx(ins.b)..")")
         code_result_at(ins.a, stack)
      end
   },

   -- R(A) := Gbl[Kst(Bx)]
   GETGLOBAL = {
      param_type = "ABx",
      -- block_boundary = nil,
      register = register_write_a,
      stack = stack_result_at_a,
      code = function(ins, fn, stack)
         local k = fn.constants[-ins.bx]
         assert(type(k) == "string", "Expected string for global name")
         writer:lua('lua_getfield','LUA_GLOBALSINDEX','"'..k..'"')
         code_result_at(ins.a, stack)
      end
   },

   -- R(A) := R(B)[RK(C)]
   GETTABLE = {
      param_type = "ABC",
      -- block_boundary = nil,
      register = register_read_b_c_write_a,
      stack = stack_result_at_a,
      code = function(ins, fn, stack)
         push_value_or_constant(fn, ins.c)
         writer:lua('lua_gettable',idx(ins.b))
         code_result_at(ins.a, stack)
      end
   },

   -- Gbl[Kst(Bx)] := R(A)
   SETGLOBAL = {
      param_type = "ABx",
      -- block_boundary = nil,
      register = register_read_a,
--[[
      stack = function(ins, stack)
         assert(stack > 0, "Stack empty at SETGLOBAL")
         --assert(ins.a == stack-1, "{"..ins.id.."} Expected A of SETGLOBAL right below top of stack (stack = "..stack..")")
         return stack - 1
      end,
--]]
      stack = stack_unchanged,
      code = function(ins, fn, stack)
         local k = fn.constants[-ins.bx]
         push_value_or_constant(fn, ins.a)
         assert(type(k) == "string", "Expected string for global name")
         writer:lua('lua_setfield','LUA_GLOBALSINDEX','"'..k..'"')
      end
   },

   -- UpValue[B] := R(A)
   SETUPVAL = {
      param_type = "AB",
      -- block_boundary = nil,
      register = register_read_a,
      stack = stack_unchanged,
      code = function(ins, fn, stack)
         writer:lua('lua_setupvalue','lua_upvalueindex('..idx(ins.b)..")",idx(ins.a))
      end
   },

   -- R(A)[RK(B)] := RK(C)
   SETTABLE = {
      param_type = "ABC",
      -- block_boundary = nil,
      register = function(ins, r)
         read_r(r,ins.a)
         read_r(r,ins.b)
         read_r(r,ins.c)
      end,
      stack = function(ins, stack)
         if ins.c > 0 and ins.c+1 == stack then
            stack = stack - 1
         end
         return stack
      end,
      code = function(ins, fn, stack)
         push_value_or_constant(fn, ins.b)
         push_value_or_constant(fn, ins.c)
         writer:lua('lua_settable',idx(ins.a))
         if ins.c > 0 and ins.c+1 == stack then
            writer:lua('lua_pop',1)
         end
      end
   },

   -- R(A) := {} (size = B,C)
   NEWTABLE = {
      param_type = "ABC",
      -- block_boundary = nil,
      register = register_write_a,
      stack = stack_result_at_a,
      code = function(ins, fn, stack)
         writer:lua('lua_createtable',ins.b,ins.c)
         code_result_at(ins.a, stack)
      end
   },

   -- R(A+1) := R(B); R(A) := R(B)[RK(C)]
   SELF = {
      param_type = "ABC",
      -- block_boundary = nil,
      register = function(ins, r)
         read_r(r, ins.b)
         read_r(r, ins.c)
         write_r(r, ins.a)
         write_r(r, ins.a + 1)
      end,
      stack = function(ins, stack)
         assert(ins.a <= stack, ins.op.." storing result beyond ("..ins.a..") top of stack ("..stack..")")
         stack = math.max(stack, ins.a + 2)
         return stack
      end,
      code = function(ins, fn, stack)
         if stack <= ins.a+1 then
            writer:lua('lua_settop',idx(ins.a+1))
         end
         push_value_or_constant(fn, ins.b)
         writer:lua('lua_replace',idx(ins.a + 1))
         push_value_or_constant(fn, ins.c)
         writer:lua('lua_gettable',idx(ins.b))
         writer:lua('lua_replace',idx(ins.a))
      end
   },

   -- R(A) := RK(B) + RK(C)
   ADD = {
      param_type = "ABC",
      -- block_boundary = nil,
      register = register_read_b_c_write_a,
      stack = stack_result_at_a,
      code = function(ins, fn, stack)
         code_binary_operation(ins, fn, "+", "__add", stack)
      end
   },

   -- R(A) := RK(B) - RK(C)
   SUB = {
      param_type = "ABC",
      -- block_boundary = nil,
      register = register_read_b_c_write_a,
      stack = stack_result_at_a,
      code = function(ins, fn, stack)
         code_binary_operation(ins, fn, "-", "__sub", stack)
      end
   },

   -- R(A) := RK(B) * RK(C)
   MUL = {
      param_type = "ABC",
      -- block_boundary = nil,
      register = register_read_b_c_write_a,
      stack = stack_result_at_a,
      code = function(ins, fn, stack)
         code_binary_operation(ins, fn, "*", "__mul", stack)
      end
   },

   -- R(A) := RK(B) / RK(C)
   DIV = {
      param_type = "ABC",
      -- block_boundary = nil,
      register = register_read_b_c_write_a,
      stack = stack_result_at_a,
      code = function(ins, fn, stack)
         code_binary_operation(ins, fn, "/", "__div", stack)
      end
   },

   -- R(A) := RK(B) % RK(C)
   MOD = {
      param_type = "ABC",
      -- block_boundary = nil,
      register = register_read_b_c_write_a,
      stack = stack_result_at_a,
      code = function(ins, fn, stack)
         code_binary_operation(ins, fn, "%", "__mod", stack)
      end
   },

   -- R(A) := RK(B) ^ RK(C)
   POW = {
      param_type = "ABC",
      -- block_boundary = nil,
      register = register_read_b_c_write_a,
      stack = stack_result_at_a,
      code = function(ins, fn, stack)
         code_binary_operation(ins, fn, "pow(", "__pow", stack)
      end
   },

   -- R(A) := -R(B)
   UNM = {
      param_type = "AB",
      -- block_boundary = nil,
      register = register_read_b_write_a,
      stack = stack_result_at_a,
      code = function(ins, fn, stack)
         code_unary_operation(ins, fn, "-", "__unm", stack)
      end
   },

   -- R(A) := not R(B)
   NOT = {
      param_type = "AB",
      -- block_boundary = nil,
      register = register_read_b_write_a,
      stack = stack_result_at_a,
      code = function(ins, fn, stack)
         writer:lua('lua_pushboolean','!lua_toboolean(L, '..idx(ins.b)..")")
         code_result_at(ins.a, stack)
      end
   },

   -- R(A) := length of R(B)
   LEN = {
      param_type = "AB",
      -- block_boundary = nil,
      register = register_read_b_write_a,
      stack = stack_result_at_a,
      code = function(ins, fn, stack)
         writer:lua('lua_pushinteger','lua_objlen(L, '..idx(ins.b)..")")
         code_result_at(ins.a, stack)
      end
   },

   -- R(A) := R(B).. ... ..R(C)
   CONCAT = {
      param_type = "ABC",
      -- block_boundary = nil,
      register = function(ins, r)
         for x = ins.b, ins.c do
            read_r(r, x)
         end
         write_r(r, ins.a)
      end,
      stack = function(ins, stack)
         assert(stack == ins.c+1, "Expected items to CONCAT at top of stack ("..ins.c..", got "..stack..")")
         stack = ins.b
         return stack_result_at_a(ins, stack)
      end,
      code = function(ins, fn, stack)
         writer:lua('lua_concat',(ins.c - ins.b + 1))
         stack = ins.b
         code_result_at(ins.a, stack)
      end
   },

   -- pc+=sBx
   JMP = {
      param_type = "sBx",
      block_boundary = block_boundary_next_and_jump,
      register = register_unchanged,
      stack = stack_unchanged,
      code = function(ins, fn, stack)
         writer:goto(ins.id + ins.sbx + 1)
      end
   },

   -- if ((RK(B) == RK(C)) ~= A) then pc++
   -- For comparisons, A specifies what condition the test should accept
   -- (true or false).
   EQ = {
      param_type = "ABC",
      block_boundary = block_boundary_next_and_following,
      register = register_read_b_c,
      stack = stack_unchanged, -- could it be smarter and pop temporaries?
      code = function(ins, fn, stack)
         comparison(fn, ins, 'lua_equal')
      end
   },

   -- if ((RK(B) <  RK(C)) ~= A) then pc++
   -- For comparisons, A specifies what condition the test should accept
   -- (true or false).
   LT = {
      param_type = "ABC",
      block_boundary = block_boundary_next_and_following,
      register = register_read_b_c,
      stack = stack_unchanged, -- could it be smarter and pop temporaries?
      code = function(ins, fn, stack)
         comparison(fn, ins, 'lua_lessthan')
      end
   },

   -- if ((RK(B) <= RK(C)) ~= A) then pc++
   -- For comparisons, A specifies what condition the test should accept
   -- (true or false).
   LE = {
      param_type = "ABC",
      block_boundary = block_boundary_next_and_following,
      register = register_read_b_c,
      stack = stack_unchanged, -- could it be smarter and pop temporaries?
      code = lessequal
   },

   -- if not (R(A) <=> C) then pc++
   TEST = {
      param_type = "ABC",
      block_boundary = block_boundary_next_and_following,
      register = register_read_a,
      stack = stack_unchanged,
      code = function(ins, fn, stack)
         writer:begin_if('(!lua_toboolean(L, '..idx(ins.a)..')) == '..ins.c)
            writer:goto(ins.id + 2)
         writer:end_block()
      end
   },

   -- if (R(B) <=> C) then R(A) := R(B) else pc++
   TESTSET = {
      param_type = "ABC",
      block_boundary = block_boundary_next_and_following,
      register = register_read_b_write_a,
      stack = stack_result_at_a,
      code = function(ins, fn, stack)
         writer:begin_if('(lua_toboolean(L, '..idx(ins.b)..')) == '..ins.c)
            writer:lua('lua_pushvalue',idx(ins.b))
            code_result_at(ins.a, stack)
         writer:or_else()
            writer:goto(ins.id+2)
         writer:end_block()
      end
   },

   -- R(A), ... ,R(A+C-2) := R(A)(R(A+1), ... ,R(A+B-1))
   -- In OP_CALL, if (B == 0) then B = top. C is the number of returns + 1, [-1 actually]
   -- and can be 0: OP_CALL then sets `top' to last_result+1, so
   -- next open instruction (OP_CALL, OP_RETURN, OP_SETLIST) may use `top'.
   CALL = {
      param_type = "ABC",
      -- block_boundary = nil,
      register = function(ins, r)
         local limit = ins.a + ins.b - 1
         if ins.b == 0 then limit = top end
         for x = ins.a, limit do
            read_r(r, x)
         end
         if ins.c ~= 0 then
            for x = ins.a, ins.a + ins.c - 2 do
               write_r(r, x)
            end
         else
            top = ins.a
         end
      end,
      stack = function(ins, stack)
         assert(stack >= ins.b - 1, "Not enough elements in stack for CALL")
         if ins.c ~= 0 then
            stack = ins.a + ins.c - 1
         else
            -- FIXME: what to do with stack here
         end
         return stack
      end,
      code = function(ins, fn, stack)
         if ins.b ~= 0 then
            if stack >= ins.a + ins.b then
               writer:lua('lua_settop', idx(ins.a + ins.b-1))
            end
         end
         local params = ins.b - 1
         local returns = ins.c - 1
         if ins.b == 0 then params = 'lua_gettop(L) - '..idx(ins.a) end
         if ins.c == 0 then returns = 'LUA_MULTRET' end
         writer:lua('lua_call', params, returns)
      end
   },

   -- return R(A)(R(A+1), ... ,R(A+B-1))
   TAILCALL = {
      param_type = "ABC",
      block_boundary = block_boundary_next,
      register = function(ins, r)
         for x = ins.a, ins.a + ins.b - 1 do
            read_r(r, x)
         end
      end,
      stack = function(ins, stack)
         return ops["CALL"].stack(ins, stack)
      end,
      code = function(ins, fn, stack)
         writer:comment('TAILCALL not available, downgrading to CALL.')
         ops["CALL"].code(ins, fn, stack)
         if ins.c == 0 then 
            writer:ret('lua_gettop(L) - '..ins.a)
         else
            writer:ret(ins.c-1)
         end
      end
   },

   -- return R(A), ... ,R(A+B-2)
   -- In OP_RETURN, if (B == 0) then return up to `top'
   RETURN = {
      param_type = "AB",
      block_boundary = block_boundary_next,
      register = function(ins, r)
         local limit = ins.a + ins.b - 2
         if ins.b == 0 then limit = top end
         for x = ins.a, limit do
            read_r(r, x)
         end
      end,
      stack = stack_unchanged,
      code = function(ins, fn, stack)
         if ins.b ~= 0 then
            writer:lua('lua_settop',idx(ins.a + ins.b - 2))
            writer:ret(ins.b - 1)
         else
            writer:ret('lua_gettop(L) - '..(ins.a))
         end
      end
   },

   -- R(A)+=R(A+2);
   -- if R(A) <?= R(A+1) then { pc+=sBx; R(A+3)=R(A) }
   FORLOOP = {
      param_type = "AsBx",
      block_boundary = block_boundary_forloop,
      register = function(ins, r)
         read_r(r, ins.a + 3) -- not actually, but needs to be marked
         read_r(r, ins.a + 2)
         write_r(r, ins.a)
         read_r(r, ins.a + 1)
         write_r(r, ins.a + 3)
      end,
      stack = function(ins, stack)
         assert(stack >= ins.a+3, "A+3 must either be in stack or be next to be stacked")
         if stack == ins.a+3 then
            stack = stack + 1
         end
         return stack
      end,
      code = function(ins, fn, stack)
         writer:begin_block()
            writer:var('int','a0')
            writer:var('int','a1')
            writer:var('int','a2')
            writer:set_lua('a0','lua_tonumber',idx(ins.a))
            writer:set_lua('a2','lua_tonumber',idx(ins.a+2))
            writer:inc('a0','a2')
            writer:lua('lua_pushnumber','a0')
            writer:lua('lua_replace',idx(ins.a))
            writer:set_lua('a1','lua_tonumber',idx(ins.a+1))
            writer:begin_if('a2 > 0 ? a0 <= a1 : a1 <= a0')
               writer:lua('lua_pushnumber','a0')
               writer:begin_if('lua_gettop(L) != '..idx(ins.a+3))
                  writer:lua('lua_replace',idx(ins.a+3))
               writer:end_block()
               writer:goto(ins.id + ins.sbx + 1)
            writer:end_block()
         writer:end_block()
      end
   },

   -- R(A)-=R(A+2); pc+=sBx
   FORPREP = {
      param_type = "AsBx",
      block_boundary = block_boundary_next_and_jump,
      register = function(ins, r)
         read_r(r, ins.a + 2)
         write_r(r, ins.a)
         write_r(r, ins.a+3) -- not actually, but needs to be marked
      end,
      stack = stack_unchanged,
      code = function(ins, fn, stack)
         writer:begin_block()
            writer:var('int','a0')
            writer:var('int','a2')
            writer:set_lua('a0','lua_tonumber',idx(ins.a))
            writer:set_lua('a2','lua_tonumber',idx(ins.a+2))
            writer:dec('a0','a2')
            writer:lua('lua_pushnumber','a0')
            writer:lua('lua_replace',idx(ins.a))
         writer:end_block()
         writer:goto(ins.id + ins.sbx + 1)
      end
   },

   -- R(A+3), ... ,R(A+3+C) := R(A)(R(A+1), R(A+2));
   -- if R(A+3) ~= nil then { R(A+2)=R(A+3); pc++; }
   TFORLOOP = {
      param_type = "AC",
      block_boundary = block_boundary_next_and_following,
      register = function(ins, r)
         -- TODO: untested
         read_r(r, ins.a)
         read_r(r, ins.a + 1)
         read_r(r, ins.a + 2)
         for x = ins.a+3, ins.a+3+ins.c do
            write_r(r, x)
         end
      end,
      stack = function(ins, stack)
         -- In Lua, locals do not survive iterations of the loop
         return ins.a + 3 + ins.c
      end,
      code = function(ins, fn, stack)
         local a, c = ins.a, ins.c
         writer:lua('lua_pushvalue',idx(a))
         writer:lua('lua_pushvalue',idx(a+1))
         writer:lua('lua_pushvalue',idx(a+2))
         writer:lua('lua_call',2,c)
         writer:if_lua('!lua_isnil',idx(a+3))
            writer:lua('lua_pushvalue',idx(a+3))
            writer:lua('lua_replace',idx(a+2))
         writer:or_else()
            writer:goto(ins.id + 2)
         writer:end_block()
      end
   },

   -- R(A)[(C-1)*FPF+i] := R(A+i), 1 <= i <= B
   -- In OP_SETLIST, if (B == 0) then B = `top';
   -- if (C == 0) then next `instruction' is real C
   SETLIST = {
      param_type = "ABC",
      register = function(ins, r)
         local limit = ins.a + ins.b
         if ins.b == 0 then limit = top end
         for i = ins.a, limit do
            read_r(r, i)
         end
      end,
      stack = function(ins, stack)
         return ins.a + 1
      end,
      code = function(ins, fn, stack)
         writer:begin_block()
         writer:var('int',"i")
         if ins.b ~= 0 then
            writer:begin_for("i = 1", "i <= "..ins.b, "i++")
         else
            writer:var('int','top')
            writer:set_lua('top','lua_gettop')
            writer:begin_for("i = 1", "i <= top", "i++")
         end
         writer:lua('lua_pushvalue',idx(ins.a).." + i")
         -- TODO: handle C == 0
         local mult = "i"
         if ins.c > 1 then
            mult = ins.c * LFIELDS_PER_FLUSH .. ' + i'
         end
         writer:lua('lua_rawseti',idx(ins.a),mult)
         writer:end_block()
         writer:end_block()
         writer:lua('lua_settop',idx(ins.a))
      end
   },

   -- close all variables in the stack up to (>=) R(A)
   CLOSE = {
      param_type = "A",
      register = register_unchanged,
      stack = function(ins, stack)
         return ins.a + 1
      end,
      code = function(ins, fn, stack)
         writer:lua('lua_settop',idx(ins.a))
      end
   },

   -- R(A) := closure(KPROTO[Bx], R(A), ... ,R(A+n))
   CLOSURE = {
      param_type = "ABx",
      -- block_boundary = nil,
      register = register_write_a, -- FIXME: deal with n
      stack = function(ins, stack)
         return math.max(stack, ins.a+1)
      end,
      code = function(ins, fn, stack)
         local f = fn.functions[-ins.bx]
         local n = input.functions[f].n_upvalues
         for i = 1, n do
            local x = fn.code[ins.id+i].b
            push_value_or_constant(fn, x)
         end
         writer:lua('lua_pushcclosure',"f"..f, n)
         code_result_at(ins.a, stack)
      end
   },

   -- R(A), R(A+1), ..., R(A+B-1) = vararg
   -- In OP_VARARG, if (B == 0) then use actual number of varargs and
   -- set top (like in OP_CALL with C == 0).
   VARARG = {
      param_type = "AB",
      -- block_boundary = nil,
      register = function(ins, r)
         if ins.b ~= 0 then
            for i = ins.a, ins.a + ins.b - 1 do
               write_r(r, x)
            end
         else
            top = ins.a
         end
      end,
      stack = function(ins, stack)
         assert(stack >= ins.b - 1, "Not enough elements in stack for CALL")
         if ins.b ~= 0 then
            stack = ins.a + ins.b - 1
         else
            -- stack is resolved in following instruction
         end
         return stack
      end,
      code = function(ins, fn, stack)
         local args
         if ins.b == 0 then
            args = 'base'
         else
            args = ins.b
         end
         writer:begin_block()
            writer:var('int',"i")
            writer:begin_for("i = 1","i <= "..args,"i++")
               writer:lua('lua_pushvalue',fn.params.." + i")
               if stack ~= ins.a then
                  writer:lua('lua_replace',idx(ins.a).." + i")
               end
            writer:end_block()
         writer:end_block()
      end
   }

}
