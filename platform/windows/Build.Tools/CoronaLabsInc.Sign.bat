@echo on
REM ---------------------------------------------------------------------------------------
REM Sign a given file with Solar2D certificate.
REM ---------------------------------------------------------------------------------------

if "%AZURE_CLIENT_SECRET%"=="" goto :OnSkipSigning

if NOT "%~2"=="" set programNameArgument=/d "%~2"


"%WORKSPACE%\Microsoft.Windows.SDK.BuildTools\bin\10.0.22621.0\x86\signtool.exe" sign /v /debug /fd SHA256 /tr "http://timestamp.acs.microsoft.com" /td SHA256 /dlib "%WORKSPACE%\Microsoft.Trusted.Signing.Client\bin\x86\Azure.CodeSigning.Dlib.dll" /dmdf "%WORKSPACE%/platform/windows/Build.Tools/solar2d.json" %programNameArgument% "%~1"
goto :End

:OnSkipSigning
echo Skipping singing becaues password or key is not found
:End
