REM Clean the "bin" directory.
rmdir /S /Q ".\bin"
mkdir "bin"

REM Add the Visual Sutdio 2013 command line tools to the PATH environment variable.
call "%VS120COMNTOOLS%vsvars32.bat"

REM Build the Win32 Angle libraries for the Windows Phone 8.0 emulator.
devenv ".\Project\src\ANGLE_phone.sln" /rebuild "Release|Win32"
mkdir "bin\Phone8.0"
mkdir "bin\Phone8.0\Win32"
copy /Y ".\Project\src\WP8\redist\vs2012\Win32\Release\libEGL_phone\libEGL_phone.dll" ".\bin\Phone8.0\Win32"
copy /Y ".\Project\src\WP8\redist\vs2012\Win32\Release\libEGL_phone\libEGL_phone.lib" ".\bin\Phone8.0\Win32"
copy /Y ".\Project\src\WP8\redist\vs2012\Win32\Release\libGLESv2_phone\libGLESv2_phone.dll" ".\bin\Phone8.0\Win32"
copy /Y ".\Project\src\WP8\redist\vs2012\Win32\Release\libGLESv2_phone\libGLESv2_phone.lib" ".\bin\Phone8.0\Win32"

REM Build the ARM version of the Angle libraries for Windows Phone 8.0.
devenv ".\Project\src\ANGLE_phone.sln" /rebuild "Release|ARM"
mkdir "bin\Phone8.0\ARM"
copy /Y ".\Project\src\WP8\redist\vs2012\ARM\Release\libEGL_phone\libEGL_phone.dll" ".\bin\Phone8.0\ARM"
copy /Y ".\Project\src\WP8\redist\vs2012\ARM\Release\libEGL_phone\libEGL_phone.lib" ".\bin\Phone8.0\ARM"
copy /Y ".\Project\src\WP8\redist\vs2012\ARM\Release\libGLESv2_phone\libGLESv2_phone.dll" ".\bin\Phone8.0\ARM"
copy /Y ".\Project\src\WP8\redist\vs2012\ARM\Release\libGLESv2_phone\libGLESv2_phone.lib" ".\bin\Phone8.0\ARM"

REM Build the Win32 command line tool used to convert OpenGL shaders to compiled Direct3D HLSL.
set CL=/D "ANGLE_DISABLE_TRACE"
devenv ".\Project\src\winrtcompiler\winrtcompiler_vs2013.sln" /rebuild "Release|Win32"
mkdir "bin\Win32"
copy /Y ".\Project\src\winrtcompiler\bin\*.exe" ".\bin\Win32"
copy /Y ".\Project\src\winrtcompiler\bin\*.dll" ".\bin\Win32"

pause
