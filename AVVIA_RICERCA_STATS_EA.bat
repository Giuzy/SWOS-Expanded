@echo off
setlocal
cd /d "%~dp0"
where py >nul 2>&1
if errorlevel 1 (set "PY=python") else (set "PY=py -3")
%PY% "diagnostica\research_runner.py" STATS_EA %*
set "RC=%ERRORLEVEL%"
pause
exit /b %RC%
