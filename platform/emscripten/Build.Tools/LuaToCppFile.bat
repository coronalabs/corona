@echo off
REM ---------------------------------------------------------------------------------------
REM Batch file used to convert a Lua file to a C++ file, such as "init.lua".
REM ---------------------------------------------------------------------------------------


REM Validate arguments. If incorrect, then echo out how to use this batch file.
if "%~1"=="" goto OnShowCommandLineHelp
if "%~2"=="" goto OnShowCommandLineHelp
if "%~3"=="" goto OnShowCommandLineHelp
if "%~4"=="" goto OnShowCommandLineHelp


REM Fetch the arguments and remove any double quotes around them.
REM Note: The "shift" command pops off the leading argument.
setlocal EnableDelayedExpansion
set batchFileDirectoryPath=%~dp0
set luaAppPath=%~1
shift
set intermediatePath=%~1
shift
set sourceFilePath=%~1
set sourceFileName=%~n1
shift
set targetPath=%~1
shift


REM Fetch optional arguments.
:OnFetchNextOptionalArgument
set optionalArgument=%1
if NOT "%optionalArgument%"=="" (
	if "%optionalArgument%"=="/debug" (
		set isDebugFlagSet=1
	)
	if "%optionalArgument%"=="/cso" (
		set isCsoFlagSet=1
	)
	if "%optionalArgument%"=="/functionName" (
		set baseFunctionName=%~2
		shift
	)
	if "%optionalArgument%"=="/moduleName" (
		set moduleName=%~2
		shift
	)
	shift
	goto OnFetchNextOptionalArgument
)


REM If we were not given a valid /functionName argument, then use the source file's name.
if "!baseFunctionName!"=="" (
	set baseFunctionName=%sourceFileName%
)


REM Remove any trailing slashes at the end of the given paths.
if "%luaAppPath:~-1%"=="\" set luaAppPath=%luaAppPath:~0,-1%
if "%intermediatePath:~-1%"=="\" set intermediatePath=%intermediatePath:~0,-1%
if "%targetPath:~-1%"=="\" set targetPath=%targetPath:~0,-1%


REM Log that we're now attempting to generate a C++ file from Lua.
echo Generating C++ file for "%sourceFilePath%"


REM Delete the last generated C++ file and *.lu file.
if EXIST "%targetPath%\%sourceFileName%_luaload.cpp" del /Q "%targetPath%\%sourceFileName%_luaload.cpp"
if EXIST "%intermediatePath%\%sourceFileName%.lu" del /Q "%intermediatePath%\%sourceFileName%.lu"


REM Create the output directories if they don't already exist.
if NOT EXIST "%intermediatePath%" mkdir "%intermediatePath%"
if NOT EXIST "%targetPath%" mkdir "%targetPath%"


REM Go to this batch file's directory.
cd "%batchFileDirectoryPath%"


REM If the "/cso" flag was set, then check if we were given a "kernel_*.lua".
REM If so, then we need to convert its OpenGL shader code into compiled Direct3D HLSL.
if NOT DEFINED isCsoFlagSet goto OnSkipShaderCompilation
if "%sourceFileName:~0,7%" NEQ "kernel_" goto OnSkipShaderCompilation
set compiledKernelFilePath=%intermediatePath%\%sourceFileName%_compiled.lua
"%luaAppPath%\lua.exe" LuaKernelGlslToCso.lua "%sourceFilePath%" "%compiledKernelFilePath%" "%intermediatePath%" "..\..\..\external\Angle\bin\Win32\winrtcompiler.exe"
if ERRORLEVEL 2 goto OnError
if ERRORLEVEL 1 goto OnSkipShaderCompilation
set sourceFilePath=%compiledKernelFilePath%
:OnSkipShaderCompilation
set ERRORLEVEL=0

echo "%luaAppPath%\lua.exe"

REM Compile the Lua script to byte code and generate a C++ file for it.
set luaDebugReleaseFlag=-ORELEASE
if DEFINED isDebugFlagSet set luaDebugReleaseFlag=-ODEBUG
if ERRORLEVEL 1 goto OnError
"%luaAppPath%\lua.exe" "..\..\..\bin\rcc.lua" -c "%luaAppPath%" %luaDebugReleaseFlag% -o "%intermediatePath%\%sourceFileName%.lu" "%sourceFilePath%"
if ERRORLEVEL 1 goto OnError
if NOT "%sourceFilePath%"=="%sourceFilePath:plugins=%" (
	REM Generate a CoronaPluginLuaLoad_*() C function for Lua files under the "plugins" directory.
	"%luaAppPath%\lua.exe" ..\..\..\bin\lua2c.lua "%intermediatePath%\%sourceFileName%.lu" !baseFunctionName! "%targetPath%\%sourceFileName%_luaload.cpp"
) else (
	REM Generate a luaload_*() C function for all other Lua files.
	"%luaAppPath%\lua.exe" -epackage.path=[[..\..\..\external\loop-2.3-beta\lua\?.lua]] ..\..\..\external\loop-2.3-beta\lua\precompiler.constant.lua -d "%targetPath%" -o %sourceFileName%_luaload -l "%intermediatePath%\%sourceFileName%.lu" -n -m "!moduleName!" !baseFunctionName!
)
if ERRORLEVEL 1 goto OnError
goto:eof


:OnShowCommandLineHelp
echo Compiles a Lua file to byte code and then generates a C++ file containing
echo that byte code and a function for executing it.
echo.
echo Usage:    LuaToCppFile.bat [LuaAppPath] [IntermediatePath] [SourceFile] [TargetPath] [/debug] [/cso] [/functionName = "FunctionName"] [/moduleName = "modulename"]
echo   [LuaAppPath]       The path to the lua.exe and luac.exe files.
echo   [IntermediatePath] Path to where temporary files are built to.
echo   [SourceFile]       The Lua path\file name to be converted.
echo   [TargetPath]       The path to where the C++ file should be created.
echo   [/debug]           Optional argument compiling Lua scripts with debug symbols.
echo   [/cso]             Optional argument which tells this build tool to convert
echo                        the glsl shader code in "kernel_*.lua" files into
echo                        compiled Direct3D hlsl binary (aka: *.cso files).
echo                        Expected to be used by Windows Phone 8.0 builds.
echo   [/functionName]    The C function name to generate, excluding the
echo                        "lua_load_" or "CoronaPluginLuaLoad_" prefix.
echo                        Will use the source file's name if not provided.
echo   [/moduleName]      A module name to include in the C function name which is generated.
exit /b 1

:OnError
echo Failed to generate C++ file for %3
exit /b 1
