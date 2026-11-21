@echo off
rem Starts the City Route Planner: builds the engine if needed, starts the bridge,
rem and opens the page in your browser.
cd /d "%~dp0"
if not exist backend\planner.exe (
  echo Engine not built yet. Building it first...
  g++ -std=c++17 -Wall -o backend\planner.exe backend\main.cpp
  if errorlevel 1 (
    echo BUILD FAILED.
    pause
    exit /b 1
  )
)
rem Open the browser after 2 seconds, giving the server time to start.
start "" cmd /c "timeout /t 2 /nobreak >nul & start http://localhost:8000"
python bridge\server.py
pause
