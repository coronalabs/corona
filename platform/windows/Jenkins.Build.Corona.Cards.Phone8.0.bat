echo Update all project files with the current daily build number.
cd "%WORKSPACE%\platform\windows"
call UpdateFileVersions.bat %BUILD_NUMBER%

echo Adjust rtt_version to have the build number in the about box, this should be pulled into something like updateFileVersions.bat in the future
cd %WORKSPACE%
c:\cygwin\bin\sed.exe -i.bak -r -e "s/^#define[[:space:]]*Rtt_BUILD_REVISION.*$/#define Rtt_BUILD_REVISION %BUILD_NUMBER%/" "%WORKSPACE%\librtt\Core\Rtt_Version.h"
c:\cygwin\bin\sed.exe -i.bak -r -e "s/^#define[[:space:]]*Rtt_BUILD_YEAR[[:space:]]*[[:digit:]]*$/#define Rtt_BUILD_YEAR %YEAR%/" "%WORKSPACE%\librtt\Core\Rtt_Version.h"
c:\cygwin\bin\sed.exe -i.bak -r -e "s/^#define[[:space:]]*Rtt_BUILD_MONTH[[:space:]]*[[:digit:]]*$/#define Rtt_BUILD_MONTH %MONTH%/" "%WORKSPACE%\librtt\Core\Rtt_Version.h"
c:\cygwin\bin\sed.exe -i.bak -r -e "s/^#define[[:space:]]*Rtt_BUILD_DAY[[:space:]]*[[:digit:]]*$/#define Rtt_BUILD_DAY %DAY%/" "%WORKSPACE%\librtt\Core\Rtt_Version.h"

echo Building CoronaCards framework, documentation, and Visual Studio extension.
cd "%WORKSPACE%\platform\windows"
call "%VS120COMNTOOLS%vsvars32.bat"
devenv "Corona.Cards.Phone8.0.sln" /rebuild "Release|Mixed Platforms"

echo Renaming the CoronaCards binary to include the daily build number.
rename ".\Bin\CoronaCards\CoronaCards.WindowsPhone.zip" "CoronaCards.WP8.%YEAR%.%BUILD_NUMBER%.zip"
