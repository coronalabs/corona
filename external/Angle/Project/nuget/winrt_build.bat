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

rmdir .\..\src\WinRT /s/q

echo./*
echo. * Building libraries...
echo. */
echo.

call %VSVARS%
if %FOUND_VC%==1 (

    msbuild  ..\src\ANGLE_winrt.sln /p:Configuration="Debug" /p:Platform="Win32" /t:Clean,Build
    msbuild  ..\src\ANGLE_winrt.sln /p:Configuration="Release" /p:Platform="Win32" /t:Clean,Build
    msbuild  ..\src\ANGLE_winrt.sln /p:Configuration="Debug" /p:Platform="ARM" /t:Clean,Build
    msbuild  ..\src\ANGLE_winrt.sln /p:Configuration="Release" /p:Platform="ARM" /t:Clean,Build

    msbuild  ..\src\ANGLE_winrt_2013.sln /p:Configuration="Debug" /p:Platform="Win32" /t:Clean,Build
    msbuild  ..\src\ANGLE_winrt_2013.sln /p:Configuration="Release" /p:Platform="Win32" /t:Clean,Build
    msbuild  ..\src\ANGLE_winrt_2013.sln /p:Configuration="Debug" /p:Platform="ARM" /t:Clean,Build
    msbuild  ..\src\ANGLE_winrt_2013.sln /p:Configuration="Release" /p:Platform="ARM" /t:Clean,Build

    xcopy "..\src\WinRT\redist\vs2012\Win32\Debug\libEGL_winrt\libEGL.dll" "..\src\WinRT\redist\vs2012\Win32\Debug\libEGL_winrt\Desktop\" /iycq
    xcopy "..\src\WinRT\redist\vs2012\Win32\Debug\libGLESv2_winrt\libGLESv2.dll" "..\src\WinRT\redist\vs2012\Win32\Debug\libGLESv2_winrt\Desktop\" /iycq
    xcopy "..\src\WinRT\redist\vs2012\Win32\Release\libEGL_winrt\libEGL.dll" "..\src\WinRT\redist\vs2012\Win32\Release\libEGL_winrt\Desktop\" /iycq
    xcopy "..\src\WinRT\redist\vs2012\Win32\Release\libGLESv2_winrt\libGLESv2.dll" "..\src\WinRT\redist\vs2012\Win32\Release\libGLESv2_winrt\Desktop\" /iycq

    xcopy "..\src\WinRT\redist\vs2013\Win32\Debug\libEGL_winrt_2013\libEGL.dll" "..\src\WinRT\redist\vs2013\Win32\Debug\libEGL_winrt_2013\Desktop\" /iycq
    xcopy "..\src\WinRT\redist\vs2013\Win32\Debug\libGLESv2_winrt_2013\libGLESv2.dll" "..\src\WinRT\redist\vs2013\Win32\Debug\libGLESv2_winrt_2013\Desktop\" /iycq
    xcopy "..\src\WinRT\redist\vs2013\Win32\Release\libEGL_winrt_2013\libEGL.dll" "..\src\WinRT\redist\vs2013\Win32\Release\libEGL_winrt_2013\Desktop\" /iycq
    xcopy "..\src\WinRT\redist\vs2013\Win32\Release\libGLESv2_winrt_2013\libGLESv2.dll" "..\src\WinRT\redist\vs2013\Win32\Release\libGLESv2_winrt_2013\Desktop\" /iycq

    link.exe /edit /appcontainer:NO ..\src\WinRT\redist\vs2012\Win32\Debug\libEGL_winrt\Desktop\libEGL.dll
    link.exe /edit /appcontainer:NO ..\src\WinRT\redist\vs2012\Win32\Debug\libGLESv2_winrt\Desktop\libGLESv2.dll
    link.exe /edit /appcontainer:NO ..\src\WinRT\redist\vs2012\Win32\Release\libEGL_winrt\Desktop\libEGL.dll
    link.exe /edit /appcontainer:NO ..\src\WinRT\redist\vs2012\Win32\Release\libGLESv2_winrt\Desktop\libGLESv2.dll

    link.exe /edit /appcontainer:NO ..\src\WinRT\redist\vs2013\Win32\Debug\libEGL_winrt_2013\Desktop\libEGL.dll
    link.exe /edit /appcontainer:NO ..\src\WinRT\redist\vs2013\Win32\Debug\libGLESv2_winrt_2013\Desktop\libGLESv2.dll
    link.exe /edit /appcontainer:NO ..\src\WinRT\redist\vs2013\Win32\Release\libEGL_winrt_2013\Desktop\libEGL.dll
    link.exe /edit /appcontainer:NO ..\src\WinRT\redist\vs2013\Win32\Release\libGLESv2_winrt_2013\Desktop\libGLESv2.dll
	
) else (
    echo Script error.
    goto ERROR
)



goto EOF

:ERROR
pause

:EOF