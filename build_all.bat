@echo off
REM Build script for CS2 cheat - builds DLL and injector with embedded resource
REM This MUST be run every time you change the DLL code
REM 
REM IMPORTANT: The DLL is built to x64\Release\CS2.dll and embedded from there

echo ========================================
echo Building CS2 DLL and Lucid Injector
echo ========================================

call "C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvarsall.bat" x64

cd /d "%~dp0"

echo.
echo [1/2] Building CS2.dll to x64\Release\...
msbuild CS2.vcxproj /p:Configuration=Release /p:Platform=x64 /m /v:minimal
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: DLL build failed!
    pause
    exit /b 1
)

echo.
echo [2/2] Building Lucid.exe with embedded DLL from x64\Release\...
cd tools
rc.exe app.rc
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Resource compilation failed!
    pause
    exit /b 1
)

cl.exe /EHsc /O2 /std:c++17 /MT /Fe:..\x64\Lucid.exe injector.cpp app.res /link advapi32.lib shell32.lib ole32.lib Psapi.lib
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Injector build failed!
    pause
    exit /b 1
)

cd ..

echo.
echo ========================================
echo Build completed successfully!
echo Output: x64\Lucid.exe
echo DLL embedded from: x64\Release\CS2.dll
echo ========================================
pause
