@echo off
setlocal
cd /d "%~dp0"
where py >nul 2>&1
if errorlevel 1 (set "PY=python") else (set "PY=py -3")
if "%~1"=="" (
  echo Verifica automatica ricerca statistiche EA...
  %PY% "SWOS_Roster_Research\collect_ea.py" --self-test
  if errorlevel 1 goto failedtest
)
%PY% "SWOS_Roster_Research\collect_ea.py" %*
set "RC=%ERRORLEVEL%"
goto done
:failedtest
echo Verifica non superata. Dati esistenti preservati. Inviare il log.
set "RC=1"
:done
pause
exit /b %RC%
