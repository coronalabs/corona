@echo off
REM ---------------------------------------------------------------------------------------
REM Batch file used to convert a Lua file to a C file, such as "init.lua".
REM ---------------------------------------------------------------------------------------


REM Validate arguments. If incorrect, then echo out how to use this batch file.
if "%~1"=="" goto OnShowCommandLineHelp
if "%~2"=="" goto OnShowCommandLineHelp
if "%~3"=="" goto OnShowCommandLineHelp
if "%~4"=="" goto OnShowCommandLineHelp
if "%~5"=="" goto OnShowCommandLineHelp
if NOT "%~6"=="" goto OnShowCommandLineHelp

REM Determine if target needs to be rebuilt
REM 
REM "lua_file" is the source Lua, "gen_file" is the file to be generated from it
REM
REM http://ss64.com/nt/delayedexpansion.html
Setlocal EnableDelayedExpansion

set lua_file="%~3"
set gen_file="%~4\%~n3.c"
REM echo lua_file: !lua_file!
REM echo gen_file: !gen_file!
if EXIST !gen_file! (
	for %%f in (!lua_file!) do set lua_file_date=%%~tf
	REM echo lua_file_date: !lua_file_date!
	set lua_tmp_date=!lua_file_date:~6,4!!lua_file_date:~0,2!!lua_file_date:~3,2!
	set lua_hours=!lua_file_date:~11,2!
	set lua_mins=!lua_file_date:~14,2!
	set lua_ampm=!lua_file_date:~17,2!
	REM echo lua_hours: !lua_hours!
	REM echo lua_mins: !lua_mins!
	REM echo lua_ampm: !lua_ampm!
	if "!lua_ampm!" == "PM" (
	  if "!lua_hours!" == 01 (set lua_hours=13)
	  if "!lua_hours!" == 02 (set lua_hours=14)
	  if "!lua_hours!" == 03 (set lua_hours=15)
	  if "!lua_hours!" == 04 (set lua_hours=16)
	  if "!lua_hours!" == 05 (set lua_hours=17)
	  if "!lua_hours!" == 06 (set lua_hours=18)
	  if "!lua_hours!" == 07 (set lua_hours=19)
	  if "!lua_hours!" == 08 (set lua_hours=20)
	  if "!lua_hours!" == 09 (set lua_hours=21)
	  if "!lua_hours!" == 10 (set lua_hours=22)
	  if "!lua_hours!" == 11 (set lua_hours=23)
	  ) else (
	  if "!lua_hours!" == 12 (set lua_hours=00)
	  ) 

	set lua_date=!lua_tmp_date!!lua_hours!!lua_mins!

	for %%f in (!gen_file!) do set gen_file_date=%%~tf
	REM echo gen_file_date: !gen_file_date!
	set gen_tmp_date=!gen_file_date:~6,4!!gen_file_date:~0,2!!gen_file_date:~3,2!
	set gen_hours=!gen_file_date:~11,2!
	set gen_mins=!gen_file_date:~14,2!
	set gen_ampm=!gen_file_date:~17,2!
	REM echo gen_hours: !gen_hours!
	REM echo gen_mins: !gen_mins!
	REM echo gen_ampm: !gen_ampm!
	if "!ampm!" == "PM" (
	  if "!gen_hours!" == 01 (set gen_hours=13)
	  if "!gen_hours!" == 02 (set gen_hours=14)
	  if "!gen_hours!" == 03 (set gen_hours=15)
	  if "!gen_hours!" == 04 (set gen_hours=16)
	  if "!gen_hours!" == 05 (set gen_hours=17)
	  if "!gen_hours!" == 06 (set gen_hours=18)
	  if "!gen_hours!" == 07 (set gen_hours=19)
	  if "!gen_hours!" == 08 (set gen_hours=20)
	  if "!gen_hours!" == 09 (set gen_hours=21)
	  if "!gen_hours!" == 10 (set gen_hours=22)
	  if "!gen_hours!" == 11 (set gen_hours=23)
	  ) else (
	  if "!gen_hours!" == 12 (set gen_hours=00)
	  ) 

	set gen_date=!gen_tmp_date!!gen_hours!!gen_mins!

	REM echo lua_date: !lua_date!
	REM echo gen_date: !gen_date!

	if "!lua_date!" LSS "!gen_date!" (
		goto OnUptodate
	)
)

REM Log that we're now attempting to generate a C file from Lua.
echo Generating C file for %3

REM Delete the last generated C file and *.lu file.
del "%~4\%~n3.c"
del "%~2\%~n3.lu"

REM Compile the Lua script to byte code and generate a C file for it.

if ERRORLEVEL 1 goto OnError

if EXIST "%~dp0\..\rcc.lua" (
	echo Using "rcc.lua" to create bytecodes
	"%~1\lua.exe" "%~dp0\..\rcc.lua" -c "%~1" "-O%~5" -o "%~2\%~n3.lu" "%~3"
	if ERRORLEVEL 1 goto OnError
	"%~1\lua.exe" "%~dp0\..\lua2c.lua" "%~2\%~n3.lu" %~n3 "%~4\%~n3.c"
	if ERRORLEVEL 1 goto OnError
	goto:eof
) else (
	echo Using "lua2c.lua" to create bytecodes
	"%~1\lua.exe" "%~dp0\..\..\shared\bin\lua2c.lua" "%~3" %~n3 "%~4\%~n3.c"
	if ERRORLEVEL 1 goto OnError
	goto:eof	
)

:OnShowCommandLineHelp
echo Compiles a Lua file to byte code and then generates a C file containing
echo that byte code and a function for executing it.
echo.
echo Usage:    lua2c.bat [LuaAppPath] [IntermediatePath] [SourceFile] [TargetPath]
echo   [LuaAppPath]       The path to the lua.exe and luac.exe files.
echo   [IntermediatePath] Path to where temporary files are built to.
echo   [SourceFile]       The Lua path\file name to be converted.
echo   [TargetPath]       The path to where the C file should be created.
echo   [Configuration]    Debug or Release
exit /b 1

:OnError
echo Failed to generate C file for %3
exit /b 1

:OnUptodate
echo Generated files up to date for %3
exit /b 0
