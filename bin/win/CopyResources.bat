@echo off
setlocal

rem $1 is the build config (if it ends in template, all .lu files are removed from dst dir)
rem $2 is the src folder
rem $3 is the dst folder
rem $4 if "--preserve" preserves png's; default is to compress png's using pngcrush, or see $5
rem $5 explicit directory where luac is. If not set, will use `dirname $0`

echo CopyResources.bat started

set BUILD_CONFIG=%1
set SRC_DIR=%2
set DST_DIR=%3

rem echo %0
rem echo %cd%

rem full path to the batch file's directory
rem echo %~dp0

rem echo BUILD_CONFIG=%BUILD_CONFIG%
rem echo SRC_DIR=%SRC_DIR%
rem echo DST_DIR=%DST_DIR%

rem Initialize luac directory
rem NOTE: luac must be in same dir as this file

rem full path to the batch file's directory
set LUAC_DIR=%~dp0
if %4 == "--preserve" (
	set CORONA_COPY_PNG_PRESERVE = "--preserve"
	if not $5 == "" (
        rem Take the user's luac directory
		set LUAC_DIR=%5
	)
) else if not $4 == "" (
    rem Take the user's luac directory
    set LUAC_DIR=%4
)

rem echo LUAC_DIR=%LUAC_DIR%

pushd %SRC_DIR%
set "r=%__CD__%"
popd

if %BUILD_CONFIG% == "*-template*" (
rem	for file in $DST_DIR/*.lu
rem		do
rem			filebase=`basename "$file"`
rem			case "$filebase" in
rem				'main.lu') echo "Removing $file"; rm $file ;;
rem			esac
rem		done
) else (
    rem iterate through app files
    for /R %SRC_DIR% %%i in (*) do (
      rem echo %%i
      if "%%~xi" == ".lua" (
        echo compiling %%i

        set "str=%%~di%%~pi%%~ni.lu"
        setlocal EnableDelayedExpansion
        call set "str=%%str:!r!=%%"
        call set "str=%%str:\=.%%"
        rem Run luac
        pushd %LUAC_DIR%
        luac.exe -o %DST_DIR%/"!str!" "%%i"
        popd
        endlocal

        rem checkError
      ) else if "%%~xi" == ".settings" (
        rem skip this file
        echo skipping %%i
      ) else (
        echo copying %%i
        set "str=%%i"
        setlocal EnableDelayedExpansion
        call set "str=%%str:!r!=%DST_DIR%\"%%""
        echo F | xcopy /Y /f "%%i" !str!
        endlocal
      )
    )
)

echo CopyResources.bat ended

endlocal
exit /B 0
