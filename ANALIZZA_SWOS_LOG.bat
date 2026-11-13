@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title SWOS Expanded - Analisi del log Windows
set "MSYS_ROOT="
if defined MSYS2_ROOT if exist "%MSYS2_ROOT%\usr\bin\bash.exe" set "MSYS_ROOT=%MSYS2_ROOT%"
if not defined MSYS_ROOT if exist "C:\msys64\usr\bin\bash.exe" set "MSYS_ROOT=C:\msys64"
if not defined MSYS_ROOT if exist "%ProgramFiles%\msys64\usr\bin\bash.exe" set "MSYS_ROOT=%ProgramFiles%\msys64"
if not defined MSYS_ROOT if exist "%LOCALAPPDATA%\Programs\MSYS2\usr\bin\bash.exe" set "MSYS_ROOT=%LOCALAPPDATA%\Programs\MSYS2"
if not defined MSYS_ROOT (
 echo ERRORE: MSYS2 non trovato. Il log SWOS.log non e' stato analizzato.
 pause
 exit /b 2
)
echo Verifica diagnostica gameplay Windows in SWOS.log.
echo Non avvia il gioco, non modifica save, non usa dati sintetici.
"%MSYS_ROOT%\usr\bin\bash.exe" -lc "bash sorgenti/tests/audit_windows_runtime_log.sh SWOS.log 0.13.38_03" > "SWOS_RUNTIME_AUDIT.log" 2>&1
set "RESULT=%ERRORLEVEL%"
type "SWOS_RUNTIME_AUDIT.log"
echo.
echo Resoconto salvato in SWOS_RUNTIME_AUDIT.log
echo PASS solo se il comportamento specifico e' stato osservato; gli altri NON TESTATI.
pause
exit /b %RESULT%
