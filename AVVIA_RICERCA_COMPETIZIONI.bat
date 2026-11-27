@echo off
setlocal
cd /d "%~dp0"
echo Verifica automatica e ripresa ricerca competizioni...
where py >nul 2>&1
if not errorlevel 1 (
  py -3 "tools\competition_research\strict_online.py" %*
  goto done
)
where python >nul 2>&1
if errorlevel 1 (
  echo Installare Python 3.10 o superiore.
  pause
  exit /b 1
)
python "tools\competition_research\strict_online.py" %*
:done
set "RC=%ERRORLEVEL%"
pause
exit /b %RC%
