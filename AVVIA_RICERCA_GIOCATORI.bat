@echo off
setlocal
cd /d "%~dp0"
where py >nul 2>&1
if not errorlevel 1 (
  py -3 "SWOS_Roster_Research\collect_names.py" %*
  goto done
)
where python >nul 2>&1
if errorlevel 1 (
  echo Installare Python 3.10 o superiore.
  pause
  exit /b 1
)
python "SWOS_Roster_Research\collect_names.py" %*
:done
set "RC=%ERRORLEVEL%"
pause
exit /b %RC%
