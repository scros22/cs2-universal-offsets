@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvarsall.bat" x64
cd /d "C:\Users\Samuel\License-Loader\Loader\Products\CS2\tools"
rc /nologo app.rc
if errorlevel 1 (echo RC FAILED & exit /b 1)
cd /d "C:\Users\Samuel\License-Loader\Loader\Products\CS2"
cl /EHsc /O2 /std:c++20 /DNDEBUG /MT /I tools /I tools\byovd /I features /I render /I sdk tools\injector.cpp tools\app.res /Fe:x64\Lucid_tmp.exe /link /SUBSYSTEM:CONSOLE shell32.lib advapi32.lib user32.lib ole32.lib
