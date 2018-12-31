
require "luac2c.read_dump"
require "luac2c.make_blocks"
require "luac2c.opcodes"
require "luac2c.write_c"

function output_c(input)
   writer:include('lua.h')
   writer:include('lualib.h')
   writer:include('lauxlib.h')
   writer:include('stdio.h', GLOBAL)
   writer:include('math.h', GLOBAL)
   writer:include('stdlib.h', GLOBAL)
   writer:include('string.h', GLOBAL)
   writer:define('MAX', '(a,b)', '((a)>(b)?(a):(b))')
   writer:prototype('void','show_stack','(lua_State* L, char* fn, int ins, int stack)')
   for fn_id, fn in pairs(input.functions) do
      writer:prototype('int','f'..fn.location,'(lua_State *L)')
   end
   local stack = 0
   for fn_id, fn in pairs(input.functions) do
      writer:begin_function('int','f'..fn.location,'(lua_State *L)')
      writer:var('int','top')
      if fn.is_vararg then
         if fn.params == 0 then
            writer:var('int','base','lua_gettop(L)')
         else
            writer:var('int','base','lua_gettop(L) - ',fn.params)
            writer:open_block()
               writer:var('int','i')
               writer:begin_for('i = 1','i <= '..fn.params,'i++')
                  writer:lua('lua_pushvalue',"i")
               writer:end_block()
            writer:end_block()
         end
         shift_base = true
      else
         shift_base = false
      end
      blocks = make_blocks(fn)
      local block = blocks[1]
      local closureskip = 0
      local prevblock = 0
      for ins_id, ins in ipairs(fn.code) do
         if ins.block_start then
            if block then prevblock = block.block_start end
            block = blocks[ins_id]
         end
         if block and closureskip == 0 then
            if ins.block_start then
               writer:label(ins.id)
               stack = block.stack_entry
               if block.adjust_stack then
                  writer:lua('lua_settop',idx(stack-1))
               end
            end
            local code = ops[ins.op].code
            assert(code, "Error: unimplemented instruction "..ins.op)
            code(ins, fn, stack)
            stack = ops[ins.op].stack(ins, stack)
            if ins.op == "CLOSURE" then
               local f = fn.functions[-ins.bx]
               closureskip = input.functions[f].n_upvalues
               writer:comment('skipping ',closureskip,' instructions')
            end
         elseif closureskip > 0 then
            closureskip = closureskip - 1
         end
      end
      writer:end_block()
   end
   return true
end

function output_main(input, as_lib, name)
   if as_lib then
      writer:begin_function('int','luaopen_'..name,'(lua_State *L)')
         writer:lua('lua_pushcclosure',"f"..input.main,0)
         writer:lua('lua_call',0,0)
         writer:ret(0)
      writer:end_block()
   else
      io.write([[
struct Smain {
   int argc;
   char** argv;
   int status;
}; 

int pmain(lua_State *L) {
   int i;
   struct Smain *s = (struct Smain *) lua_touserdata(L, 1);
   luaL_checkstack(L, s->argc + 3, "too many arguments to script");
   lua_newtable(L);
   for (i = 0; i < s->argc; i++) {
      lua_pushstring(L, s->argv[i]);
      lua_rawseti(L, -2, i);
   }
   lua_setglobal(L, "arg");
   luaL_openlibs(L);
   lua_pushcclosure(L, f]]..input.main..[[, 0);
   lua_call(L, 0, 0);
   /* TODO: forward exit status code */
   return 0;
}

int main(int argc, char** argv) {
   struct Smain s;
   lua_State *L;
   int err = 0;
   L = luaL_newstate();
   if (!lua_checkstack(L, 100)) {
      fprintf(stderr, "Could not set up stack!\n");
      exit(1); 
   }
   s.argc = argc;
   s.argv = argv;
   err = lua_cpcall(L, pmain, &s);
   if (err) {
      unsigned int len = 0;
      const char* msg = lua_tolstring(L, -1, &len);
      fprintf(stderr, "Error: %s\n", msg);
   }
   lua_close(L);
   return err;
}
]])
   end
end

----------------------------------------

if not arg[1] then
   print("Usage: luac2c.lua <program.lua>")
   os.exit(1)
end

input = read_dump(arg[1])
if not input then os.exit(1) end
ok, err = output_c(input)
if not ok then
   print("")
   print("Error: "..tostring(err))
   os.exit(1)
end
output_main(input, arg[2] == "-l", arg[3])
