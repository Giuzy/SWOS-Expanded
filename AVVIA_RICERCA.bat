@echo off
setlocal EnableExtensions DisableDelayedExpansion
pushd "%~dp0" || exit /b 1
set "TOOL=%CD%\tools\competition_research"
set "VENV=%TOOL%\.venv"
set "VENV_PY=%VENV%\Scripts\python.exe"
set "OUT=%TOOL%\output"
if not exist "%OUT%" mkdir "%OUT%"
set "LOG=%OUT%\bootstrap.log"
echo [%date% %time%] Avvio ricerca SWOS>>"%LOG%"
echo.
echo SWOS EXPANDED - RICERCA COMPETIZIONI
echo Fonti: "%CD%\sources"
echo Output: "%OUT%"
echo Il gioco e i salvataggi non vengono modificati.
echo.
if exist "%VENV_PY%" goto checkvenv
set "PYLAUNCH=py"
set "PYARGS=-3.12"
py -3.12 -c "import sys" >nul 2>&1
if not errorlevel 1 goto createvenv
set "PYLAUNCH=%LocalAppData%\Programs\Python\Python312\python.exe"
set "PYARGS="
if exist "%PYLAUNCH%" goto createvenv
echo Python 3.12 non trovato. Installazione per utente tramite winget.
where winget >nul 2>&1
if errorlevel 1 goto nopython
winget install --id Python.Python.3.12 --exact --scope user --accept-package-agreements --accept-source-agreements
if exist "%PYLAUNCH%" goto createvenv
py -3.12 -c "import sys" >nul 2>&1
if errorlevel 1 goto nopython
set "PYLAUNCH=py"
set "PYARGS=-3.12"
:createvenv
echo Creo ambiente Python isolato...
"%PYLAUNCH%" %PYARGS% -m venv "%VENV%" >>"%LOG%" 2>&1
if errorlevel 1 goto failed
:checkvenv
"%VENV_PY%" -c "import sys; assert sys.version_info[:2]==(3,12)" >>"%LOG%" 2>&1
if errorlevel 1 goto failed
echo Verifico e installo dipendenze...
"%VENV_PY%" -m pip --disable-pip-version-check --log "%LOG%" install -r "%TOOL%\requirements.txt"
if errorlevel 1 goto failed
echo.
echo Avvio analisi. Le fonti GitHub vengono scaricate nella cache locale.
"%VENV_PY%" -u "%TOOL%\research.py" %*
if errorlevel 1 goto failed
echo.
echo Operazione terminata. Controlla summary.json e source_log.json nello ZIP.
echo Se hai eseguito il test, il risultato e' indicato nella console.
echo Cartella risultati: "%OUT%"
popd
pause
exit /b 0
:nopython
echo Installazione automatica Python non riuscita. Installa Python 3.12 x64 da python.org e riavvia questo BAT.
echo Python 3.12 non disponibile>>"%LOG%"
goto failed
:failed
echo.
echo ERRORE: consulta "%LOG%" e gli eventuali ERRORE_RICERCA_*.zip in output.
echo [%date% %time%] ERRORE avvio o elaborazione>>"%LOG%"
popd
pause
exit /b 1
