@echo off
setlocal enableextensions enabledelayedexpansion
set SDK=%APPDATA%\Corona Labs\Corona Simulator\Android Build\sdk
FOR %%A IN (%*) DO (
    IF "%%A"=="/T" set SDK=%WINDIR%\Temp\Corona Labs\sdk
)
if not exist "%SDK%\licenses" (
md "%SDK%\licenses"
)
copy /Y ..\sdk\licenses\android-sdk-license "%SDK%\licenses\android-sdk-license" >nul
set /p="sdk.dir=%SDK:\=/%" > local.properties < NUL
@ver > NUL
