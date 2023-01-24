@echo off
REM ---------------------------------------------------------------------------------------
REM This script replaces place holders in Lua templates with the current size of the default splash screen
REM ---------------------------------------------------------------------------------------

REM Validate arguments. If incorrect, then echo out how to use this batch file.
if "%~1"=="" goto OnShowCommandLineHelp
if "%~2"=="" goto OnShowCommandLineHelp

set SRC=%1
set DST=%2
rem echo SRC=%SRC%
rem echo DST=%DST%

rem clear the output file
break>"%~2"

set FILESIZE = 0
set FULLPATH = ""

call :getFullPath "%SRC%"
rem echo FULLPATH1=%FULLPATH%
call :getFullPath "%FULLPATH%..\..\..\..\"
rem echo FULLPATH2=%FULLPATH%

set PNG_FILE_ANDROID=%FULLPATH%platform\android\sdk\res\drawable\_corona_splash_screen.png
echo PNG_FILE_ANDROID=%PNG_FILE_ANDROID%

call :getFileSize "%PNG_FILE_ANDROID%"

rem if %FILESIZE% GTR 0 (
    echo %FILESIZE%
    rem replace DEFAULT_ANDROID_SPLASH_IMAGE_FILE_SIZE
    setLocal EnableDelayedExpansion
    For /f "tokens=* delims= " %%a in (%~1) do (
	    Set str=%%a
	    set str=!str:DEFAULT_ANDROID_SPLASH_IMAGE_FILE_SIZE=%FILESIZE%!
	    echo !str!>>"%~2"
    )
    endlocal
rem ) else (
rem     echo Failed to get file size
rem    exit /b 1
)

goto:eof

rem
rem Usage
rem
:OnShowCommandLineHelp
echo Generates Lua from its template
echo.
echo Usage:    TemplateToLua.bat [TemplatePath] [LuaFile]
exit /b 1

rem
rem getFileSize of 1st argument in %size% variable, and return
rem
:getFileSize
  set FILESIZE=%~z1
  exit /b 0

rem
rem
rem
:getFullPath
  set FULLPATH=%~dp1
  exit /b 0
