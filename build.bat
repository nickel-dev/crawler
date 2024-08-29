@echo off

if not exist build mkdir build
if not exist build-int mkdir build-int

set opts=/FC /GR- /EHa- /nologo /Zi
set code=%cd%\source

pushd build-int
cl %opts% %code%\win32_platform.c /link opengl32.lib gdi32.lib user32.lib shell32.lib /out:..\build\win32_crawler.exe
popd

pushd data
if %ERRORLEVEL% EQU 0 ..\build\win32_crawler.exe
popd