@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title SWOS Expanded - Analisi del log Windows
set "MSYS_ROOT="
if defined MSYS2_ROOT if exist "%MSYS2_ROOT%\usr\bin\bash.exe" set "MSYS_ROOT=%MSYS2_ROOT%"
if not defined MSYS_ROOT if exist "C:\msys64\usr\bin\bash.exe" set "MSYS_ROOT=C:\msys64"
if not defined MSYS_ROOT if exist "%ProgramFiles%\msys64\usr\bin\bash.exe" set "MSYS_ROOT=%ProgramFiles%\msys64"
if not defined MSYS_ROOT if exist "%LOCALAPPDATA%\Programs\MSYS2\usr\bin\bash.exe" set "MSYS_ROOT=%LOCALAPPDATA%\Programs\MSYS2"
if not defined MSYS_ROOT if exist "%USERPROFILE%\scoop\apps\msys2\current\usr\bin\bash.exe" set "MSYS_ROOT=%USERPROFILE%\scoop\apps\msys2\current"
if not defined MSYS_ROOT (
 echo ERRORE: MSYS2 non trovato. Il log SWOS.log non e' stato analizzato.
 if not "%SWOS_DIAGNOSTIC_SESSION%"=="1" pause
 exit /b 2
)
echo Verifica diagnostica gameplay Windows in SWOS.log.
echo Non avvia il gioco, non modifica save, non usa dati sintetici.
REM Bash riceve un percorso RELATIVO alla cartella corrente del BAT.
REM NON usare -lc: una login shell MSYS2 puo' cambiare cwd in HOME.
"%MSYS_ROOT%\usr\bin\bash.exe" "sorgenti/tests/audit_windows_runtime_log.sh" "SWOS.log" "0.13.38_05" > "SWOS_RUNTIME_AUDIT.log" 2>&1
set "RESULT=%ERRORLEVEL%"
type "SWOS_RUNTIME_AUDIT.log"
echo.
echo Resoconto salvato in SWOS_RUNTIME_AUDIT.log
if "%RESULT%"=="2" echo [INCOMPLETE] Non tutti i controlli Windows sono stati eseguiti.
if "%RESULT%"=="1" echo [FAIL] Errore reale rilevato: leggere FIRST_RUNTIME_FAILURE.
echo PASS solo se il comportamento specifico e' stato dimostrato; il resto NOT_TESTED.
if not "%SWOS_DIAGNOSTIC_SESSION%"=="1" pause
exit /b %RESULT%
