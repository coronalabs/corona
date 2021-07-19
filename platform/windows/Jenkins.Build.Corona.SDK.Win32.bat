echo Update all project files with the current daily build number.
cd "%WORKSPACE%\platform\windows"
call UpdateFileVersions.bat %BUILD_NUMBER%

echo Copy the docs in from the Jenkins job
cd "%WORKSPACE%\.."
rmdir /s /q "%WORKSPACE%\..\docs"
mkdir "%WORKSPACE%\..\docs"
unzip -a docs.zip -d "%WORKSPACE%\..\docs"

echo Copy the Native in from the Jenkins job
if EXIST "%WORKSPACE%\..\CoronaEnterprise-%YEAR%.%BUILD_NUMBER%.tgz" (
	mkdir "%WORKSPACE%\platform\windows\Bin"
	cd "%WORKSPACE%\platform\windows\Bin"
	rmdir /s /q "Native"
	tar -xvzf ..\..\..\..\CoronaEnterprise-%YEAR%.%BUILD_NUMBER%.tgz
	rm ._CoronaEnterprise
	mv CoronaEnterprise Native
	del /q /f /a Native\.*
	del /q /f /a Native\Icon?
) else (
	echo WARNING! Skipping Native copy, because "%WORKSPACE%\..\CoronaEnterprise-%YEAR%.%BUILD_NUMBER%.tgz" is not found!
)

if EXIST "%WORKSPACE%\..\webtemplate.zip" (
    echo Copying webtamplate.zip
    copy "%WORKSPACE%\..\webtemplate.zip" "%WORKSPACE%\platform\resources" /Y
) else (
    echo WARNING! webtemplate is not found!
)

if EXIST "%WORKSPACE%\..\nxtemplate" (
    echo Copying nxtemplate
    copy "%WORKSPACE%\..\nxtemplate" "%WORKSPACE%\platform\resources" /Y
) else (
    echo WARNING! nxtemplate is not found!
)

if EXIST "%WORKSPACE%\..\linuxtemplate.tar.gz" (
    echo Copying linuxtemplate.tar.gz
    copy "%WORKSPACE%\..\linuxtemplate.tar.gz" "%WORKSPACE%\platform\resources" /Y
) else (
    echo WARNING! linuxtemplate is not found!
)

if EXIST "%WORKSPACE%\..\raspbiantemplate.tar.gz" (
    echo Copying raspbiantemplate.tar.gz
    copy "%WORKSPACE%\..\raspbiantemplate.tar.gz" "%WORKSPACE%\platform\resources" /Y
) else (
    echo WARNING! linuxtemplate is not found!
)


cd "%WORKSPACE%\..\docs"
c:\cygwin\bin\find ./SampleCode \( -iname '*.lua' -or -iname '*.txt' -or -iname '*.settings' \) -print0 | xargs -0 -n1 -P4 unix2dos

cd "%WORKSPACE%\platform"
c:\cygwin\bin\find ./resources \( -iname '*.lua' -or -iname '*.txt' -or -iname '*.settings' -or -iname '*.properties' -or -iname '*.xml' -or -iname '*.ccscene' -or -iname '*.json' \) -print0 | xargs -0 -n1 -P4 unix2dos

cd "%WORKSPACE%\simulator-extensions"
c:\cygwin\bin\find . \( -iname '*.lua' -or -iname '*.txt' -or -iname '*.settings' -or -iname '*.properties' -or -iname '*.xml' -or -iname '*.ccscene' -or -iname '*.json' \) -print0 | xargs -0 -n1 -P4 unix2dos

cd %WORKSPACE%
echo Adjust rtt_version to have the build number in the about box, this should be pulled into something like updateFileVersions.bat in the future
c:\cygwin\bin\sed.exe -i.bak -r -e "s/^#define[[:space:]]*Rtt_BUILD_REVISION.*$/#define Rtt_BUILD_REVISION %BUILD_NUMBER%/" "%WORKSPACE%\librtt\Core\Rtt_Version.h"
c:\cygwin\bin\sed.exe -i.bak -r -e "s/^#define[[:space:]]*Rtt_BUILD_YEAR[[:space:]]*[[:digit:]]*$/#define Rtt_BUILD_YEAR %YEAR%/" "%WORKSPACE%\librtt\Core\Rtt_Version.h"
c:\cygwin\bin\sed.exe -i.bak -r -e "s/^#define[[:space:]]*Rtt_BUILD_MONTH[[:space:]]*[[:digit:]]*$/#define Rtt_BUILD_MONTH %MONTH%/" "%WORKSPACE%\librtt\Core\Rtt_Version.h"
c:\cygwin\bin\sed.exe -i.bak -r -e "s/^#define[[:space:]]*Rtt_BUILD_DAY[[:space:]]*[[:digit:]]*$/#define Rtt_BUILD_DAY %DAY%/" "%WORKSPACE%\librtt\Core\Rtt_Version.h"

echo Building Corona.
if daily==%Release% (
	if defined CUSTOM_ID ( echo customBuildId="%CUSTOM_ID%" >> "%WORKSPACE%\platform\resources\AppSettings.lua" )
	if defined S3_BUCKET ( echo buildBucket="%S3_BUCKET%" >> "%WORKSPACE%\platform\resources\AppSettings.lua" )
)
cd "%WORKSPACE%\platform\windows"
call "%VS120COMNTOOLS%vsvars32.bat"
devenv "Corona.SDK.sln" /rebuild "Release|x86"
if release==%Release% (
	rename ".\Bin\Corona.SDK.Installer\Corona.msi" "Corona-%YEAR%.%BUILD_NUMBER%-Release.msi"
)
if daily==%Release% (
	rename ".\Bin\Corona.SDK.Installer\Corona.msi" "Corona-%YEAR%.%BUILD_NUMBER%.msi"
)
