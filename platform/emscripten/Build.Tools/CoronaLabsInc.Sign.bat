@echo off
REM ---------------------------------------------------------------------------------------
REM Batch file used to digitally sign a given file with Corona Labs' certificate.
REM ---------------------------------------------------------------------------------------


REM Validate arguments. If incorrect, then echo out how to use this batch file.
if "%~1"=="" goto OnShowCommandLineHelp


REM Attempt to fetch the optional 2nd argument "Program Name".
if NOT "%~2"=="" set programNameArgument=/d "%~2"


REM Digitally sign the given file.
echo Digitally signing file: %~1
call "%VS120COMNTOOLS%vsvars32.bat"

signtool sign /f "%~dp0CoronaLabsInc.pfx" /p "zxcvbnm,./" /t http://timestamp.digicert.com %programNameArgument% "%~1"
if errorlevel 0 goto:eof
echo "Signin failed. Using fallback timestamp server 1!"
signtool sign /f "%~dp0CoronaLabsInc.pfx" /p "zxcvbnm,./" /t http://timestamp.verisign.com/scripts/timestamp.dll %programNameArgument% "%~1"
if errorlevel 0 goto:eof
echo "Signin failed. Using fallback timestamp server 2!"
signtool sign /f "%~dp0CoronaLabsInc.pfx" /p "zxcvbnm,./" /t http://timestamp.globalsign.com/scripts/timestamp.dll %programNameArgument% "%~1"
if errorlevel 0 goto:eof
echo "Signin failed. Using fallback timestamp server 3!"
signtool sign /f "%~dp0CoronaLabsInc.pfx" /p "zxcvbnm,./" /t http://tsa.starfieldtech.com %programNameArgument% "%~1"
if errorlevel 0 goto:eof
echo "Signin failed. Using fallback timestamp server 4!"
signtool sign /f "%~dp0CoronaLabsInc.pfx" /p "zxcvbnm,./" /t http://timestamp.geotrust.com/tsa %programNameArgument% "%~1"
if errorlevel 0 goto:eof
echo "NO TIMESTAMP FALLBACK WORKED!"
exit /b 1
goto:eof


:OnShowCommandLineHelp
echo Signs a given file with Corona Labs' digital certificate.
echo.
echo Usage:    CoronaLabsInc.Sign.bat [FilePath] [ProgramName]
echo   [FilePath]         Path to the file that you want to digitally sign.
echo   [ProgramName]      Optional: Program name to be displayed in the UAC prompt.
exit /b 1
