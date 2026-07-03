@echo off
cmake -S . -B build-win -G "Visual Studio 17 2022"
if errorlevel 1 ( echo Configure failed & pause & exit /b 1 )

cmake --build build-win --config Debug
if errorlevel 1 ( echo Build failed & pause & exit /b 1 )

.\build-win\Debug\SpenceDew.exe
