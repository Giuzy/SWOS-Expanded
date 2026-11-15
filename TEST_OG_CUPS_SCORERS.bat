@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title SWOS Expanded - Coppe e marcatori OG prima/dopo
echo ==============================================================
echo  COPPE E MARCATORI ORIGINALI OG - TEST PRIMA / DOPO
echo ==============================================================
echo Avvia prima AVVIA_DIAGNOSTICA.bat con una carriera di prova.
echo Per confrontare 70 anni: UNEMPLOYED, F12+8, FORWARD TIME.
echo Questo BAT legge SOLO SWOS.log, non crea partite né punteggi.
echo Coppe senza contesti OG: NON IMPLEMENTATO, non PASS.
echo Contatori OG dei gol: non certificano la schermata marcatori.
echo ==============================================================
set "ORIGINAL_DIAG=%SWOS_DIAGNOSTIC_SESSION%"
set "SWOS_DIAGNOSTIC_SESSION=1"
set "AUDIT_LOG=SWOS_RUNTIME_AUDIT.log"
set "DRIVER_LOG=OG_CUPS_SCORERS_DRIVER.log"
if defined SWOS_DIAGNOSTIC_DIR (
 set "AUDIT_LOG=%SWOS_DIAGNOSTIC_DIR%\SWOS_RUNTIME_AUDIT.log"
 set "DRIVER_LOG=%SWOS_DIAGNOSTIC_DIR%\OG_CUPS_SCORERS_DRIVER.log"
)
REM Main AVVIA_DIAGNOSTICA already ran the C parser exactly once.
REM Standalone BAT still creates its own audit when necessary.
if defined SWOS_DIAGNOSTIC_DIR if exist "%AUDIT_LOG%" goto audit_already_done
call "%~dp0ANALIZZA_SWOS_LOG.bat" > "%DRIVER_LOG%" 2>&1
set "ANALYSIS=%ERRORLEVEL%"
goto audit_available
:audit_already_done
set "ANALYSIS=2"
findstr /B /C:"FIRST_RUNTIME_FAILURE:" "%AUDIT_LOG%" >nul
if not errorlevel 1 set "ANALYSIS=1"
> "%DRIVER_LOG%" echo [REUSE] SWOS_RUNTIME_AUDIT.log generated once by AVVIA_DIAGNOSTICA.bat.
:audit_available
type "%DRIVER_LOG%"
if not exist "%AUDIT_LOG%" goto incomplete
echo.
findstr /B /C:"OG_CUP_DEFINITIONS=" "%AUDIT_LOG%"
findstr /B /C:"OG_CUP_RESIDENT=" "%AUDIT_LOG%"
findstr /B /C:"OG_CUP_COVERAGE=" "%AUDIT_LOG%"
findstr /B /C:"OG_CUP_START_COVERAGE=" "%AUDIT_LOG%"
findstr /B /C:"OG_CUP_COUNTRIES=" "%AUDIT_LOG%"
findstr /B /C:"OG_CUP_NATIVE_BYE_EVENTS=" "%AUDIT_LOG%"
findstr /B /C:"OG_CUP_FORMAT_RULES=" "%AUDIT_LOG%"
findstr /B /C:"OG_INTERNATIONAL_NATIONAL=" "%AUDIT_LOG%"
findstr /B /C:"OG_RESIDENT_SAVE_RELOAD=" "%AUDIT_LOG%"
findstr /B /C:"OG_CUP_DIVERGENCE " "%AUDIT_LOG%"
findstr /B /C:"OG_CUPS_BEFORE_AFTER=" "%AUDIT_LOG%"
findstr /B /C:"OG_CUP_NATIVE_MATCHES=" "%AUDIT_LOG%"
findstr /B /C:"OG_SCORERS_BEFORE_AFTER=" "%AUDIT_LOG%"
findstr /B /C:"OG_TOP_GOAL_SCORERS_VIEWER=" "%AUDIT_LOG%"
findstr /B /C:"OG_OFFER_CUPS_SCORERS_BEFORE_AFTER=" "%AUDIT_LOG%"
if "%ANALYSIS%"=="1" goto failure
findstr /B /C:"RUNTIME_LOG=READ" "%AUDIT_LOG%" | findstr /C:"scope=CURRENT_BUILD" >nul
if errorlevel 1 goto incomplete
findstr /B /C:"OG_CUPS_BEFORE_AFTER=" "%AUDIT_LOG%" | findstr /C:"NOT_IMPLEMENTED" >nul
if not errorlevel 1 (
 echo [INCOMPLETE] La coppa esiste nei dati ma non e' residente.
)
findstr /B /C:"OG_SCORERS_BEFORE_AFTER=" "%AUDIT_LOG%" | findstr /C:"PLAYER_COUNTERS_EMPTY" >nul
if not errorlevel 1 (
 echo [FAIL] Il campionato OG ha gol ma i contatori giocatori sono vuoti.
 goto failure
)
findstr /B /C:"OG_CUPS_BEFORE_AFTER=" "%AUDIT_LOG%" | findstr /C:"AFTER_MISSING" >nul
if not errorlevel 1 goto incomplete
findstr /B /C:"OG_SCORERS_BEFORE_AFTER=" "%AUDIT_LOG%" | findstr /C:"AFTER_MISSING" >nul
if not errorlevel 1 goto incomplete
echo [INCOMPLETE] I test dei dati sono separati dalla prova dei menu.
echo [INCOMPLETE] Le coppe OG residenti e TOP GOAL SCORERS restano aperti.
set "RESULT=2"
goto done
:failure
echo [FAIL] Controllare FIRST_RUNTIME_FAILURE e i contatori nativi.
set "RESULT=1"
goto done
:incomplete
echo [INCOMPLETE] Dati prima/dopo insufficienti. Necessario nuovo log.
set "RESULT=2"
:done
echo Invia SWOS_RUNTIME_AUDIT.log e OG_CUPS_SCORERS_DRIVER.log.
if not "%ORIGINAL_DIAG%"=="1" pause
exit /b %RESULT%
