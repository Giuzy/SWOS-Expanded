@echo off
setlocal
cd /d "%~dp0"
where py >nul 2>&1
if errorlevel 1 (set "PY=python") else (set "PY=py -3")
if /i "%~1"=="--self-test" goto base
if /i "%~1"=="--export-only" goto base
if /i "%~1"=="--prepare-import" goto base
if not exist "SWOS_Roster_Research\.venv\Scripts\python.exe" %PY% -m venv "SWOS_Roster_Research\.venv"
if not exist "SWOS_Roster_Research\.venv\Scripts\python.exe" goto base
"SWOS_Roster_Research\.venv\Scripts\python.exe" -c "import duckdb" >nul 2>&1
if errorlevel 1 "SWOS_Roster_Research\.venv\Scripts\python.exe" -m pip install "duckdb>=1.1,<2"
"SWOS_Roster_Research\.venv\Scripts\python.exe" "SWOS_Roster_Research\collect_names.py" %*
goto done
:base
%PY% "SWOS_Roster_Research\collect_names.py" %*
:done
set "RC=%ERRORLEVEL%"
pause
exit /b %RC%
