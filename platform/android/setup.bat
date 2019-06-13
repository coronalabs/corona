@echo off
setlocal enableextensions enabledelayedexpansion
if not exist "%APPDATA%\Corona Labs\Corona Simulator\Android Build\sdk\licenses" (
md "%APPDATA%\Corona Labs\Corona Simulator\Android Build\sdk\licenses"
)
copy /Y ..\sdk\licenses\android-sdk-license "%APPDATA%\Corona Labs\Corona Simulator\Android Build\sdk\licenses\android-sdk-license" >nul
set PT=sdk.dir=%APPDATA%\Corona Labs\Corona Simulator\Android Build\sdk
echo|set /p="%PT:\=/%" > local.properties
endlocal
