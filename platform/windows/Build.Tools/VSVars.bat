@echo off

echo Setting up Visual Studio C++ CLI
for %%V in ("%VS120COMNTOOLS%vsvars32.bat"
           ,"C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvars32.bat"
           ,"C:\Program Files (x86)\Microsoft Visual Studio\2019\Enterprise\VC\Auxiliary\Build\vcvars32.bat"
           ,"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars32.bat"
           ,"C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvars32.bat"
           ,"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars32.bat"
           ) do (
    where signtool >nul 2>nul
    if %errorlevel% neq 0 (
        echo trying %%V
        if exist %%V (
            echo Using %%V
            call %%V
    ))
)   

where signtool >nul 2>nul
if %errorlevel% neq 0 (
    echo unable to find signtool, exiting
    exit /b 1
)
