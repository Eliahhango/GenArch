@echo off
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
set PATH=C:\Program Files\CMake\bin;%PATH%
cd /d "C:\Users\hango\Documents\Programming\2026\Achicad Automation\addon"
set DEVKIT=C:\Users\hango\Documents\Programming\2026\Achicad Automation\addon\Build\DevKit\APIDevKit-29\Support
set BUILDDIR=C:\Users\hango\Documents\Programming\2026\Achicad Automation\addon\Build\AC29_NMake
if not exist "%BUILDDIR%" mkdir "%BUILDDIR%"
cmake -B "%BUILDDIR%" -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DAC_API_DEVKIT_DIR="%DEVKIT%" -DAC_VERSION=29 -S "C:\Users\hango\Documents\Programming\2026\Achicad Automation\addon"
if %ERRORLEVEL% NEQ 0 exit /b %ERRORLEVEL%
cmake --build "%BUILDDIR%" --config RelWithDebInfo
