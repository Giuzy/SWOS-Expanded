@echo off
setlocal
cd /d "%~dp0"
rem Double-click: resume the new work DB, never the blocked legacy DB.
set "ROSTER_DEFAULT_ARGS="
if "%~1"=="" (
    set "ROSTER_DEFAULT_ARGS=--restart-from-json"
    if not exist "SWOS_Roster_Research\results_2627\research.sqlite3.restart" set "ROSTER_DEFAULT_ARGS=--restart-from-json --refresh-wiki"
)
if /i "%~1"=="--export-only" if "%~2"=="" set "ROSTER_DEFAULT_ARGS=--restart-from-json"
if /i "%~1"=="--recover-snapshot" if "%~2"=="" set "ROSTER_DEFAULT_ARGS=--restart-from-json"
where py >nul 2>&1
if errorlevel 1 (set "PY=python") else (set "PY=py -3")
if /i "%~1"=="--self-test" goto base
if /i "%~1"=="--export-only" goto base
if /i "%~1"=="--recover-snapshot" goto base
if /i "%~1"=="--prepare-import" goto base
if not exist "SWOS_Roster_Research\.venv\Scripts\python.exe" %PY% -m venv "SWOS_Roster_Research\.venv"
if not exist "SWOS_Roster_Research\.venv\Scripts\python.exe" goto base
"SWOS_Roster_Research\.venv\Scripts\python.exe" -c "import duckdb" >nul 2>&1
if errorlevel 1 "SWOS_Roster_Research\.venv\Scripts\python.exe" -m pip install "duckdb>=1.1,<2"
"SWOS_Roster_Research\.venv\Scripts\python.exe" "SWOS_Roster_Research\collect_names.py" %ROSTER_DEFAULT_ARGS% %*
goto done
:base
%PY% "SWOS_Roster_Research\collect_names.py" %ROSTER_DEFAULT_ARGS% %*
:done
set "RC=%ERRORLEVEL%"
pause
exit /b %RC%
