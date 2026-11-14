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
REM Parser C portatile: non richiede awk, Python, PowerShell o nuovi tool.
REM Compila con lo STESSO compilatore MinGW del gioco, in %%TEMP%%.
set "AUDIT_BIN=%MSYS_ROOT%\mingw64\bin"
if not exist "%AUDIT_BIN%\gcc.exe" set "AUDIT_BIN=%MSYS_ROOT%\ucrt64\bin"
set "AUDIT_CC=%AUDIT_BIN%\gcc.exe"
REM cc1.exe needs MinGW runtime DLLs (including libgmp-10.dll) in PATH.
REM Invoking gcc.exe by absolute path alone does not resolve its DLLs.
set "PATH=%AUDIT_BIN%;%MSYS_ROOT%\usr\bin;%PATH%"
if not exist "%AUDIT_CC%" (
 echo [INCOMPLETE] Compiler MinGW non trovato, audit non eseguito.
 if not "%SWOS_DIAGNOSTIC_SESSION%"=="1" pause
 exit /b 2
)
set "AUDIT_EXE=%TEMP%\SWOS_runtime_audit_%RANDOM%_%RANDOM%.exe"
"%AUDIT_CC%" -std=c11 -O2 -Wall -Wextra -Werror "sorgenti/tests/audit_windows_runtime_log.c" -o "%AUDIT_EXE%"
if errorlevel 1 (
 echo [INCOMPLETE] Errore compilazione audit C, nessuna prova Windows.
 if exist "%AUDIT_EXE%" del /q "%AUDIT_EXE%" >nul 2>&1
 if not "%SWOS_DIAGNOSTIC_SESSION%"=="1" pause
 exit /b 2
)
"%AUDIT_EXE%" "SWOS.log" "0.13.39_01" > "SWOS_RUNTIME_AUDIT.log" 2>&1
set "RESULT=%ERRORLEVEL%"
if exist "%AUDIT_EXE%" del /q "%AUDIT_EXE%" >nul 2>&1
type "SWOS_RUNTIME_AUDIT.log"
echo.
echo Resoconto salvato in SWOS_RUNTIME_AUDIT.log
if "%RESULT%"=="2" echo [INCOMPLETE] Non tutti i controlli Windows sono stati eseguiti.
if "%RESULT%"=="1" echo [FAIL] Errore reale rilevato: leggere FIRST_RUNTIME_FAILURE.
echo PASS solo se il comportamento specifico e' stato dimostrato; il resto NOT_TESTED.
if not "%SWOS_DIAGNOSTIC_SESSION%"=="1" pause
exit /b %RESULT%
