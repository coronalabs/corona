@echo off

echo Setup command line
for %%V in ("%VS120COMNTOOLS%vsvars32.bat"
           ,"C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars32.bat"
           ,"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat" 
           ) do (
    where signtool >nul 2>nul
    if %errorlevel% neq 0 if exist %%V (
        echo Using %%V
        call %%V
    )
)   

where signtool >nul 2>nul
if %errorlevel% neq 0 (
    echo unable to find signtool, exiting
    exit /b 1
)
