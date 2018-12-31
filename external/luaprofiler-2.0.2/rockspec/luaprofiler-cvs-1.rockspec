package="LuaProfiler"
version="cvs-1"
source = {
   url = "cvs://:pserver:anonymous@cvs.luaforge.net:/cvsroot/luaprofiler",
   cvs_tag = "HEAD"
}
description = {
   summary = "Profiling for Lua scripts",
   detailed = [[
   LuaProfiler is a time profiler designed to help finding bottlenecks on your Lua program.
    If you use LuaProfiler into your program, it will generate a log of all your function calls
 and their respective running times. You can use this log to generate a summary of the functions
 your program uses, along with how long it stayed in each of them, or you can import the log into
 a spreadsheet for further analysis.
   ]],
   homepage = "http://www.keplerproject.org/luaprofiler/",
   license = "MIT/X11"
}
dependencies = {
   "lua >= 5.1"
}
build = {
   type = "module",
   modules = {
      profiler = {
         sources = { "src/clocks.c", "src/core_profiler.c", "src/function_meter.c", "src/stack.c", "src/lua50_profiler.c" }
      }
   }
}
