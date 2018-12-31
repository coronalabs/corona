
GLOBAL = true

writer = {

   indent = "",

   inc_indent = function(self)
      self.indent = self.indent .. "   "
   end,

   dec_indent = function(self)
      self.indent = string.sub(self.indent, 4)
   end,

   writeln = function(self, ...)
      io.write(self.indent, ...)
      io.write("\n")
   end,

   write = function(self, ...)
      io.write(self.indent, ...)
   end,

   include = function(self, name, global)
      if global then
         self:writeln("#include <",name,">")
      else
         self:writeln('#include "',name,'"')
      end
   end,

   define = function(self, name, param, value)
      self:writeln()
      self:writeln("#ifndef ",name)
      self:writeln("#define ",name,param," ",value)
      self:writeln("#endif")
   end,

   prototype = function(self, typ, name, args)
      self:writeln()
      self:writeln(typ," ",name,args,";")
   end,

   begin_function = function(self, typ, name, args)
      self:writeln()
      self:writeln(typ," ",name,args," {")
      self:inc_indent()
   end,

   var = function(self, typ, name, init, ...)
      if init then
         if #{...} > 0 then
            self:writeln(typ," ",name," = ",init,...,";")
         else
            self:writeln(typ," ",name," = ",init,";")
         end
      else
         self:writeln(typ," ",name,";")
      end
   end,

   begin_for = function(self, from, test, inc)
      self:writeln("for(",from,"; ",test,"; ",inc,") {")
      self:inc_indent()
   end,

   begin_block = function(self)
      self:writeln("{")
      self:inc_indent()
   end,

   end_block = function(self)
      self:dec_indent()
      self:writeln("}")
   end,

   lua = function(self, fn, ...)
      self:write(fn,"(L")
      for _, par in pairs({...}) do
         io.write(", ",par)
      end
      io.write(");\n")
   end,

   set_lua = function(self, var, fn, ...)
      self:write(var," = ",fn,"(L")
      for _, par in pairs({...}) do
         io.write(", ",par)
      end
      io.write(");\n")
   end,

   set = function(self, var, ...)
      self:writeln(var," = ",...,";")
   end,

   if_lua = function(self, fn, ...)
      self:write("if (",fn,"(L")
      for _, par in pairs({...}) do
         io.write(", ",par)
      end
      io.write(")) {\n")
      self:inc_indent()
   end,

   begin_if = function(self, cond)
      self:writeln("if (",cond,") {")
      self:inc_indent()
   end,

   else_if = function(self, cond)
      self:dec_indent()
      self:writeln("} else if (",cond,") {")
      self:inc_indent()
   end,

   or_else = function(self)
      self:dec_indent()
      self:writeln("} else {")
      self:inc_indent()
   end,

   label = function(self, num)
      io.write("I",num,":\n")
   end,

   comment = function(self,...)
      self:writeln('/* ',...,' */')
   end,

   ret = function(self, ...)
      self:writeln("return ",...,";")
   end,

   brk = function(self)
      self:writeln("break;")
   end,

   goto = function(self, num)
      self:writeln("goto I",num,";")
   end,

   inc = function(self, var, rate)
      if rate then
         self:writeln(var,' += ',rate,";")
      else
         self:writeln(var,'++;')
      end
   end,

   dec = function(self, var, rate)
      if rate then
         self:writeln(var,' -= ',rate,";")
      else
         self:writeln(var,'--;')
      end
   end
}
