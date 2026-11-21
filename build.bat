@echo off
rem Builds the C++ engine. Double-click this file, or run it from any folder.
cd /d "%~dp0"
echo Building the engine...
g++ -std=c++17 -Wall -o backend\planner.exe backend\main.cpp
if errorlevel 1 (
  echo.
  echo BUILD FAILED. Read the messages above.
  pause
  exit /b 1
)
echo.
echo Build OK: backend\planner.exe
pause
