
require "luac2c.opcodes"

function pairsByKeys (t, f)
local a = {}
for n in pairs(t) do table.insert(a, n) end
table.sort(a, f)
local i = 0      -- iterator variable
local iter = function ()   -- iterator function
   i = i + 1
   if a[i] == nil then return nil
   else return a[i], t[a[i]]
   end
end
return iter
end

local function detect_basic_blocks(fn)
   local code = fn.code
   local blocks = {}

   local function new_block(at)
      if at > #code or blocks[at] then return end
      blocks[at] = { block_start = at, go = {}, come = {}, n_come = 0 }
      code[at].block_start = true
   end

   new_block(1)
   for ins_id, ins in ipairs(code) do
      -- instructions that mark end of blocks
      local boundary = ops[ins.op].block_boundary
      if boundary then
         for _, dest in ipairs({boundary(ins)}) do
            new_block(dest)
         end
      end
   end
   return blocks
end

-- build graph of basic blocks
local function build_basic_blocks_graph(blocks, code)

   local function add_edge(from, to)
      if not (blocks[from] and blocks[to]) then return end
      blocks[from].go[to] = blocks[to]
      blocks[to].come[from] = blocks[from]
      blocks[to].n_come = blocks[to].n_come + 1
   end

   local function add_edge_if_needed(prev)
      if prev == 0 then return end
      -- instructions that never flow to the next instruction
      if code[prev].op ~= "JMP"
      and code[prev].op ~= "RETURN"
      and code[prev].op ~= "TAILCALL"
      and not (code[prev].op == "LOADBOOL" and code[prev].c == 1)
      then
         src = code[prev].block.block_start
         add_edge(src, prev + 1)
      end
   end

   local curr = 1

   local block = nil
   for ins_id, ins in ipairs(code) do
      if ins.block_start then
         block = blocks[ins_id]
      end
      ins.block = block
   end

   for ins_id, ins in ipairs(code) do
      if ins.block_start then
         if ins_id > 1 then blocks[curr].block_end = ins_id - 1 end
         curr = ins_id
      end
      -- instructions that affect pc
      local boundary = ops[ins.op].block_boundary
      if boundary then
         boundary(ins, curr, add_edge, add_edge_if_needed)
      end
   end
   blocks[curr].block_end = #code
end

local function analyze_live_variables(blocks, code)

   local function has_changed(old, new)
      if not old or not new then return true end
      if #old ~= #new then return true end
      for key,val in pairs(new) do
         if old[key] ~= val then return true end
      end
      return false
   end

   local function analyze_instructions(block, code)
      block.registers = {}
      for ins_id = block.block_start, block.block_end do
         local ins = code[ins_id]
         local register = ops[ins.op].register
         assert(register, "ERROR: Missing register function for "..ins.op)
         register(ins, block.registers)
      end
   end

   local function propagate_registers(block, code)
      local r = {}
      if block.registers then
         for reg, typ in pairs(block.registers) do
            r[reg] = typ
         end
      end
      -- propagate up reads/writes by blocks that follow this one
      for _, next_block in pairs(block.go) do
         if next_block.registers then
            for reg, typ in pairs(next_block.registers) do
               if typ and not r[reg] then r[reg] = typ end
            end
         end
      end
      local oldr = block.registers
      block.registers = r
--[[
   io.write("// oldr { ")
   for reg, typ in pairs(oldr) do
      io.write(reg..":"..typ.." ")
   end
   print("}")
   io.write("// newr { ")
   for reg, typ in pairs(r) do
      io.write(reg..":"..typ.." ")
   end
   print("}")

print("//   haschanged => "..tostring(has_changed(oldr, r)))
--]]
      return not has_changed(oldr, r)
   end

   for _, block in pairs(blocks) do
      analyze_instructions(block, code)
   end
   repeat
      local fixpoint = true
      for _, block in pairs(blocks) do
         fixpoint = propagate_registers(block, code) and fixpoint
      end
--[[
print("/* liveliness iteration ****")
for id, block in pairsByKeys(blocks) do
   io.write(id)
   io.write(" => ")
   for toid, toblock in ipairs(block.go) do
      print(toid.." ")
   end

   io.write("   { ")
   for reg, typ in pairs(block.registers) do
      io.write(reg..":"..typ.." ")
   end
   print("}")
end
print("// fixpoint = "..tostring(fixpoint))
print("***************************/")
--]]
   until fixpoint
end

local function propagate_dead_blocks(blocks, code)

   local function check_dead_block(id, block)
      if id ~= 1 and block.n_come == 0 then
         for id_dest, dest in pairs(block.go) do
            block.go[id_dest] = nil
            dest.come[id] = nil
            dest.n_come = dest.n_come - 1
            check_dead_block(id_dest, dest)
         end
         blocks[id] = nil
      end
   end

   for id, block in pairs(blocks) do
      check_dead_block(id, block)
   end
end

local function compute_stack_height(blocks, fn)
   local code = fn.code
   local function compute_block(block)
--print("// block "..block.block_start..": entry = "..block.stack_entry)
      local stack = block.stack_entry
      local maxread = -1
      for rnum, rtype in pairs(block.registers) do
         if rtype == "read" and rnum > maxread then maxread = rnum end
      end
      local locals = 0
      for _, loc in pairs(fn.locals) do
         if loc.from <= block.block_start
         and loc.to > block.block_start then
            locals = locals + 1
         end
      end
--print("// stack = "..stack.." vs maxread = "..maxread..", locals = "..locals)
      local new_stack = stack
      if stack > maxread + 1 then
         new_stack = maxread + 1
      end
      new_stack = math.max(new_stack, locals)
      if new_stack ~= stack then
         block.adjust_stack = true
         stack = new_stack
         block.stack_entry = stack
--print("// stack updated = "..stack)
      end
      local closureskip = 0
      for ins_id = block.block_start, block.block_end do
         if closureskip == 0 then
            local ins = code[ins_id]
            stack_fn = ops[ins.op].stack
            assert(type(stack_fn) == "function", "ERROR: Missing stack function for "..ins.op)
            stack = stack_fn(ins, stack, locals)
--print("//   at "..ins_id..": "..stack.."\t"..ins.op)
            assert(type(stack) == "number", "stack of "..ins.op.." is broken!")
            if ins.op == "CLOSURE" then
               local f = fn.functions[-ins.bx]
               closureskip = input.functions[f].n_upvalues
            end
         else
            closureskip = closureskip - 1 
         end
      end
      block.stack_exit = stack
      for _, next_block in pairs(block.go) do
         if not next_block.stack_entry then
            next_block.stack_entry = block.stack_exit
            compute_block(next_block)
         else
            if next_block.stack_entry ~= block.stack_exit then
               next_block.adjust_stack = true
--[[
               assert(false, "temp: stack analysis for "..next_block.block_start..
               ": "..next_block.stack_entry.." vs "..block.stack_exit)
--]]
--print("stack conflict at: "..next_block.block_start..": "..next_block.stack_entry.." vs "..block.stack_exit)
               next_block.block_start = math.min(next_block.stack_entry, block.stack_exit)
            end
         end
      end
   end
   local block = blocks[1]
   block.stack_entry = fn.params
   block.adjust_stack = true
   compute_block(block)
end

function make_blocks(fn)

   local blocks = detect_basic_blocks(fn)
--[[
print("/* blocks ******************")
for id, block in pairsByKeys(blocks) do
   print(id)
end
print("***************************/")
--]]
   build_basic_blocks_graph(blocks, fn.code)
--[[
print("/* blocks graph ************")
for id, block in pairsByKeys(blocks) do
   print(id)
   for toid, toblock in pairs(block.go) do
      print("   => "..toid)
   end
end
print("***************************/")
--]]
   propagate_dead_blocks(blocks, fn.code)
   analyze_live_variables(blocks, fn.code)
--[[
print("/* liveliness **************")
for id, block in pairsByKeys(blocks) do
   print(id)
   io.write("   { ")
   for reg, typ in pairs(block.registers) do
      io.write(reg..":"..typ.." ")
   end
   print("}")
end
print("***************************/")
--]]
   compute_stack_height(blocks, fn)

   -- note: block.n_come == 0 means an unreachable block
   return blocks
end
