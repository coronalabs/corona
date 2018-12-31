This is a self-running Lua interpreter. It is meant to be combined with a
Lua program (which may be in either text or precompiled form) into a single,
stand-alone program that will run the given Lua program when it is run.

The command-line arguments will be available to the Lua program in a table
called "arg" and also as '...'. All standard Lua libraries will be available
too. If you want to use a different set of libraries, just copy linit.c from
the Lua source, add it to srlua.c and edit luaL_openlibs to suit your needs.

The Makefile is targeted at Unix systems. Just edit it to reflect your
installation of Lua. Then run make. This will build srlua and glue, and run
a simple test. For each Lua program that you want to turn into a stand-alone
program, do
	glue srlua prog.lua a.out
	chmod +x a.out
Of course, you can use any name instead of a.out.

For Windows, you need to create srlua.exe and glue.exe first. Then for each
Lua program that you want to turn into a stand-alone program, do
	glue srlua.exe prog.lua prog.exe
Of course, you can use any name instead of prog.exe.

This code is hereby placed in the public domain.
Please send comments, suggestions, and bug reports to lhf@tecgraf.puc-rio.br .
