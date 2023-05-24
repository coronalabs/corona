@echo off
REM ---------------------------------------------------------------------------------------
REM Batch file used to update the version and copyright of all Visual Studio project files.
REM ---------------------------------------------------------------------------------------


REM Validate arguments. If incorrect, then echo out how to use this batch file.
if "%1"=="" goto OnShowCommandLineHelp
if NOT "%2"=="" goto OnShowCommandLineHelp


REM Set the major version to the current year in 2 digit form.
REM Set minor and build number according to the given argument.
REM Warning: Windows install programs cannot have more than 3 digits for the major/minor version.
set tempBuildNumber=%BUILD_NUMBER:~0,5%
set /a majorVersion = %DATE:~-2%
set /a minorVersion = %tempBuildNumber% / 10000
set /a buildNumber = %tempBuildNumber% %% 10000


REM Update all project files.
cscript.exe //NoLogo Build.Tools\UpdateProjectFileVersion.vbs Corona.AppTemplate.Phone8.0\Properties\AssemblyInfo.cs %majorVersion% %minorVersion% %buildNumber%
if ERRORLEVEL 1 goto OnError
cscript.exe //NoLogo Build.Tools\UpdateProjectFileVersion.vbs Corona.Cards.AppTemplate.CSharp.Phone8.0\Corona.Cards.AppTemplate.CSharp.Phone8.0.csproj %majorVersion% %minorVersion% %buildNumber%
if ERRORLEVEL 1 goto OnError
cscript.exe //NoLogo Build.Tools\UpdateProjectFileVersion.vbs Corona.Cards.AppTemplate.VB.Phone8.0\Corona.Cards.AppTemplate.VB.Phone8.0.vbproj %majorVersion% %minorVersion% %buildNumber%
if ERRORLEVEL 1 goto OnError
cscript.exe //NoLogo Build.Tools\UpdateProjectFileVersion.vbs Corona.Cards.VSExtension.Phone8.0\VSIX\extension.vsixmanifest %majorVersion% %minorVersion% %buildNumber%
if ERRORLEVEL 1 goto OnError
cscript.exe //NoLogo Build.Tools\UpdateProjectFileVersion.vbs Corona.Controls.DotNet.Library.Phone8.0\Properties\AssemblyInfo.cs %majorVersion% %minorVersion% %buildNumber%
if ERRORLEVEL 1 goto OnError
cscript.exe //NoLogo Build.Tools\UpdateProjectFileVersion.vbs Corona.DotNet.Library\Properties\AssemblyInfo.cs %majorVersion% %minorVersion% %buildNumber%
if ERRORLEVEL 1 goto OnError
cscript.exe //NoLogo Build.Tools\UpdateProjectFileVersion.vbs Corona.Debugger\Corona.Debugger.rc %majorVersion% %minorVersion% %buildNumber%
if ERRORLEVEL 1 goto OnError
cscript.exe //NoLogo Build.Tools\UpdateProjectFileVersion.vbs Corona.Framework.VSExtension.Phone8.0\VSIX\extension.vsixmanifest %majorVersion% %minorVersion% %buildNumber%
if ERRORLEVEL 1 goto OnError
cscript.exe //NoLogo Build.Tools\UpdateProjectFileVersion.vbs Corona.Native.Library.Win32\resource.rc %majorVersion% %minorVersion% %buildNumber%
if ERRORLEVEL 1 goto OnError
cscript.exe //NoLogo Build.Tools\UpdateProjectFileVersion.vbs Corona.OutputViewer\Corona.OutputViewer.rc %majorVersion% %minorVersion% %buildNumber%
if ERRORLEVEL 1 goto OnError
cscript.exe //NoLogo Build.Tools\UpdateProjectFileVersion.vbs Corona.SDK.Installer\Main.wxs %majorVersion% %minorVersion% %buildNumber%
if ERRORLEVEL 1 goto OnError
cscript.exe //NoLogo Build.Tools\UpdateProjectFileVersion.vbs Corona.Simulator\Simulator.rc %majorVersion% %minorVersion% %buildNumber%
if ERRORLEVEL 1 goto OnError
cscript.exe //NoLogo Build.Tools\UpdateProjectFileVersion.vbs Corona.Simulator.Native.Library.Win32\resource.rc %majorVersion% %minorVersion% %buildNumber%
if ERRORLEVEL 1 goto OnError
cscript.exe //NoLogo Build.Tools\UpdateProjectFileVersion.vbs Corona.Shell\Corona.Shell.rc %majorVersion% %minorVersion% %buildNumber%
if ERRORLEVEL 1 goto OnError
cscript.exe //NoLogo Build.Tools\UpdateProjectFileVersion.vbs Corona.LiveServer\Corona.LiveServer.rc %majorVersion% %minorVersion% %buildNumber%
if ERRORLEVEL 1 goto OnError

@echo on

echo %BUILD_NUMBER% %majorVersion% %minorVersion% %buildNumber%

goto:eof


:OnShowCommandLineHelp
echo Updates the version and copyright of all Visual Studio project files.
echo Requires 1 command line argument to specify the minor version number.
echo The major version is set to the current year via the system clock.
echo.
echo Usage:    UpdateFileVersions.bat [BuildNumber]
exit /b 1

:OnError
exit /b 1



