@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title SWOS Expanded - Tutti i test OG e storici
set "ORIGINAL_DIAG=%SWOS_DIAGNOSTIC_SESSION%"
set "MSYS_ROOT="
if defined MSYS2_ROOT if exist "%MSYS2_ROOT%\usr\bin\bash.exe" set "MSYS_ROOT=%MSYS2_ROOT%"
if not defined MSYS_ROOT if exist "C:\msys64\usr\bin\bash.exe" set "MSYS_ROOT=C:\msys64"
if not defined MSYS_ROOT if exist "%ProgramFiles%\msys64\usr\bin\bash.exe" set "MSYS_ROOT=%ProgramFiles%\msys64"
if not defined MSYS_ROOT if exist "%LOCALAPPDATA%\Programs\MSYS2\usr\bin\bash.exe" set "MSYS_ROOT=%LOCALAPPDATA%\Programs\MSYS2"
if not defined MSYS_ROOT if exist "%USERPROFILE%\scoop\apps\msys2\current\usr\bin\bash.exe" set "MSYS_ROOT=%USERPROFILE%\scoop\apps\msys2\current"
if not defined MSYS_ROOT (
 echo [INCOMPLETE] MSYS2 non trovato. Nessuna nuova toolchain richiesta.
 if not "%ORIGINAL_DIAG%"=="1" pause
 exit /b 2
)
set "MSYSTEM=MINGW64"
set "CHERE_INVOKING=1"
set "PATH=%MSYS_ROOT%\usr\bin;%MSYS_ROOT%\mingw64\bin;%PATH%"
set "TEST_LOG=OG_WORLD_TEST.log"
set "AUDIT_LOG=SWOS_RUNTIME_AUDIT.log"
if defined SWOS_DIAGNOSTIC_DIR (
 set "TEST_LOG=%SWOS_DIAGNOSTIC_DIR%\OG_WORLD_TEST.log"
 set "AUDIT_LOG=%SWOS_DIAGNOSTIC_DIR%\SWOS_RUNTIME_AUDIT.log"
)
echo ==========================================================
echo SWOS EXPANDED - TUTTI I TEST: OG ATTUALI + LEGACY
echo Ogni test avviato una sola volta; log individuali.
echo Unico motore calcistico di gameplay OG.
echo ==========================================================
"%MSYS_ROOT%\usr\bin\bash.exe" "sorgenti/tests/run_all_diagnostic_tests.sh" > "%TEST_LOG%" 2>&1
set "STATIC_RESULT=%ERRORLEVEL%"
type "%TEST_LOG%"
echo.
echo CONTROLLO GAMEPLAY: rapporto Windows separato dai test isolati.
set "RUNTIME_RESULT=2"
if defined SWOS_DIAGNOSTIC_DIR (
 if exist "%AUDIT_LOG%" (
   findstr /B /C:"OVERALL=INCOMPLETE" "%AUDIT_LOG%" >nul
   if errorlevel 1 (
     findstr /B /C:"FIRST_RUNTIME_FAILURE:" "%AUDIT_LOG%" >nul
     if not errorlevel 1 set "RUNTIME_RESULT=1"
     findstr /B /C:"OVERALL=PASS" "%AUDIT_LOG%" >nul
     if not errorlevel 1 set "RUNTIME_RESULT=0"
   )
 )
) else (
 call "%~dp0ANALIZZA_SWOS_LOG.bat" > "OG_WORLD_RUNTIME_DRIVER.log" 2>&1
 set "RUNTIME_RESULT=%ERRORLEVEL%"
)
if not "%STATIC_RESULT%"=="0" (
 echo [FAIL] Alcuni test OG sorgente/dati non sono passati; consultare TEST_ESEGUITI.
 set "RESULT=1"
 goto done
)
if "%RUNTIME_RESULT%"=="1" (
 echo [FAIL] Audit runtime Windows: vedere SWOS_RUNTIME_AUDIT.log.
 set "RESULT=1"
 goto done
)
if "%RUNTIME_RESULT%"=="2" (
 echo [INCOMPLETE] Gameplay OG Windows non certificato.
 set "RESULT=2"
 goto done
)
echo [PASS] Test statici OG eseguiti. Controllare separatamente i test storici.
set "RESULT=0"
:done
echo [NOTE] I test LWE isolati sono regressioni storiche, non un secondo motore.
if not "%ORIGINAL_DIAG%"=="1" pause
exit /b %RESULT%
