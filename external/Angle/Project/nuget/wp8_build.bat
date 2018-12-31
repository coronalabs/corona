@echo off

@echo off

echo./*
echo. * Check VC++ environment...
echo. */
echo.

set FOUND_VC=0

if defined VS120COMNTOOLS (
    set VSTOOLS="%VS120COMNTOOLS%"
    set VC_VER=120
    set FOUND_VC=1
) else if defined VS110COMNTOOLS (
    set VSTOOLS="%VS110COMNTOOLS%"
    set VC_VER=110
    set FOUND_VC=1
)

set VSTOOLS=%VSTOOLS:"=%
set "VSTOOLS=%VSTOOLS:\=/%"

set VSVARS="%VSTOOLS%vsvars32.bat"

if not defined VSVARS (
    echo Can't find VC2012 or VC2013 installed!
    goto ERROR
)

rmdir .\..\src\WP8 /s/q

echo./*
echo. * Building libraries...
echo. */
echo.

call %VSVARS%
if %FOUND_VC%==1 (
    msbuild  ..\src\ANGLE_phone.sln /t:Clean
    msbuild  ..\src\ANGLE_phone.sln /p:Configuration="Debug" /p:Platform="Win32"
    msbuild  ..\src\ANGLE_phone.sln /p:Configuration="Release" /p:Platform="Win32"
    msbuild  ..\src\ANGLE_phone.sln /p:Configuration="Debug" /p:Platform="ARM"
    msbuild  ..\src\ANGLE_phone.sln /p:Configuration="Release" /p:Platform="ARM"
    msbuild  ..\src\winrtcompiler\winrtcompiler.sln /p:Configuration="Release" /p:Platform="Win32"
) else (
    echo Script error.
    goto ERROR
)



goto EOF

:ERROR
pause

:EOF