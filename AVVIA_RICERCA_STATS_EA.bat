@echo off
setlocal
cd /d "%~dp0"
where py >nul 2>&1
if errorlevel 1 (
  python "SWOS_Roster_Research\collect_ea.py" %*
) else (
  py -3 "SWOS_Roster_Research\collect_ea.py" %*
)
set "RC=%ERRORLEVEL%"
pause
exit /b %RC%
