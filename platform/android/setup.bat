@echo off
setlocal enableextensions
md "%APPDATA%\Corona Labs\Corona Simulator\Android Build\sdk\licenses"
copy /Y ..\sdk\licenses\android-sdk-license "%APPDATA%\Corona Labs\Corona Simulator\Android Build\sdk\licenses\android-sdk-license"
echo sdk.dir=%APPDATA%\Corona Labs\Corona Simulator\Android Build\sdk > local.properties
endlocal
