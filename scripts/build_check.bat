@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvarsall.bat" x64 >nul 2>&1
REM resolve repo root from this script's location (<repo>\scripts\)
cd /d "%~dp0.."
echo === Compiling injector.cpp (full build check) ===
cl /nologo /std:c++latest /EHsc /W3 /c /Fo:NUL tools\injector.cpp
if %ERRORLEVEL% EQU 0 (echo BUILD OK) else (echo BUILD FAILED)
del _test.cpp 2>nul
