
require "luac2c.opcodes"

--[[
Reads a luac dump and returns the data formatted like this:
{
   functions = {
      "1002a658" = {
         location = "1002a658",
         is_main = true,
         params = 0,
         is_vararg = true,
         n_slots = 4,
         n_upvalues = 0
         n_locals = 0
         n_code = 14
         n_constants = 7
         n_functions = 3
         code = {
            1 = { id = 1, op = "LOADK", a = 0, bx = -2 },
            2 = { id = 2, op = "SETGLOBAL", a = 0, bx = -1 },
            3 = ...
         },
         locals = {
            1 = { name = "x", from = 2, to = 10 },
            2 = ...
         },
         constants = {
            1 = "delta",
            2 = 1e-06,
            3 = ...
         }
         upvalues = {}, -- not available when binary is stripped
         functions = {
            0 = "1002a880",
            1 = ...
         }
      },
      ...
   }
}
]]

function read_dump(input_file)

   local input = {
      functions = {}
   }
   local current_function

   lines = {}
   for line in io.lines(input_file) do
      table.insert(lines, line)
   end

   local current_line = 0
   local function get_line() 
      current_line = current_line + 1
      return lines[current_line]
   end

   -- forward declarations
   local state_header
   local state_header_b
   local state_ins
   local state_constant
   local state_local
   local state_upvalue

   local function state_start()
      get_line()
      return state_header()
   end

   state_header = function()
      local line = get_line()
      if not line then return false, "Unexpected EOF" end
      local item_type, n_code, location = string.match(line, "(%w+) <[%w/.:,_-]+> %((%d+)% instruction[s]?, %d+ byte[s]? at 0x(%x+)%)")
      if not location then
         return false, "Unrecognized input file."
      end
      current_function = {
         code = {},
         locals = {},
         constants = {},
         upvalues = {},
         functions = {}
      }
      if item_type == "main" then
         current_function.is_main = true
         input.main = location
      else
         current_function.is_main = false
      end
      current_function.n_code = n_code
      current_function.location = location
      input.functions[location] = current_function
      return state_header_b()
   end

   state_header_b = function()
      local line = get_line()
      if not line then return false, "Unexpected EOF" end
      local params, vararg, slots, upvalues, locals, constants, functions = string.match(line, "(%d+)(%+?) params?, (%d+) slots?, (%d+) upvalues?, (%d+) locals?, (%d+) constants?, (%d+) functions?")
      current_function.params = tonumber(params)
      current_function.is_vararg = (vararg == "+")
      current_function.n_slots = tonumber(slots)
      current_function.n_upvalues = tonumber(upvalues)
      current_function.n_locals = tonumber(locals)
      current_function.n_constants = tonumber(constants)
      current_function.n_functions = tonumber(functions)
      return state_ins()
   end

   state_ins = function()
      local line = get_line()
      if not line then return false, "Unexpected EOF" end
      if string.sub(line, 1, 1) == "\t" then
         local id, ins, operands, rest = string.match(line, "\t(%d+)\t%[[%d-]+%]\t(%u+)%s+([%d-%s]+)(.*)")
         local a, b, c, bx, sbx
         if param_type(ins) == "A" then
            a = string.match(operands, "([%d-]+)")
            a = tonumber(a)
         elseif param_type(ins) == "AB" then
            a, b = string.match(operands, "([%d-]+) ([%d-]+)")
            a, b = tonumber(a), tonumber(b)
         elseif param_type(ins) == "AC" then
            a, c = string.match(operands, "([%d-]+) ([%d-]+)")
            a, c = tonumber(a), tonumber(c)
         elseif param_type(ins) == "ABx" then
            a, bx = string.match(operands, "([%d-]+) ([%d-]+)")
            a, bx = tonumber(a), tonumber(bx)
         elseif param_type(ins) == "AsBx" then
            a, sbx = string.match(operands, "([%d-]+) ([%d-]+)")
            a, sbx = tonumber(a), tonumber(sbx)
         elseif param_type(ins) == "ABC" then
            a, b, c = string.match(operands, "([%d-]+) ([%d-]+) ([%d-]+)")
            a, b, c = tonumber(a), tonumber(b), tonumber(c)
         elseif param_type(ins) == "sBx" then
            sbx = string.match(operands, "([%d-]+)")
            sbx = tonumber(sbx)
         end
         if ins == "CLOSURE" then
            addr = string.match(rest, "; 0x([%x]+)")
            current_function.functions[-bx] = addr
         end
         id = tonumber(id)
         current_function.code[id] = {
            id = id, op = ins, a = a, b = b, c = c, bx = bx, sbx = sbx
         }
         return state_ins()
      elseif string.match(line, "(%w+)") == "constants" then
         return state_constant()
      end
   end

   state_constant = function()
      local line = get_line()
      if not line then return false, "Unexpected EOF" end
      if string.sub(line, 1, 1) == "\t" then
         local id, value = string.match(line, "\t(%d+)\t([^%c]+)")
         if string.sub(value, 1, 1) == "\"" then
            value = string.match(value, "\"(.*)\"")
            -- Convert Lua-style \ddd escapes to C-style \ooo escapes
            local at = 1
            local input = value
            value = ""
            while true do
               local st, en = string.find(input, "\\%d+", at)
               if not st then break end
               value = value .. string.sub(input, at, st - 1)
                     .."\\"..string.format("%03o", tonumber(string.sub(input, st+1,en)))
               at = en + 1
            end
            value = value .. string.sub(input, at)
         elseif value == "true" then
            value = true
         elseif value == "false" then
            value = false
         else
            value = tonumber(value)
         end
         current_function.constants[tonumber(id)] = value
         return state_constant()
      elseif string.match(line, "(%w+)") == "locals" then
         return state_local()
      end
   end

   state_local = function()
      local line = get_line()
      if not line then return false, "Unexpected EOF" end
      if string.sub(line, 1, 1) == "\t" then
         local id, name, from, to = string.match(line, "\t(%d+)\t([^\t]+)\t(%d+)\t(%d+)")

         current_function.locals[tonumber(id)] = {
            name = name, from = tonumber(from), to = tonumber(to)
         }
         return state_local()
      elseif string.match(line, "(%w+)") == "upvalues" then
         return state_upvalue()
      end
   end

   state_upvalue = function()
      local line = get_line()
      if not line then return true end
      if string.sub(line, 1, 1) == "\t" then
         local id, value = string.match(line, "\t(%d+)\t([^%c]+)")
         current_function.upvalues[tonumber(id)] = {
            value = value
         }
         return state_upvalue()
      elseif #line == 0 then
         return state_header()
      end
   end

   ok, err = state_start()
   if ok
   then
      return input
   else 
      print(err)
      return nil
   end
end

