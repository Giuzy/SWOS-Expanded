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
set "SWOS_DIAGNOSTIC_SESSION=1"
call "%~dp0ANALIZZA_SWOS_LOG.bat" > "OG_CUPS_SCORERS_DRIVER.log" 2>&1
set "ANALYSIS=%ERRORLEVEL%"
type "OG_CUPS_SCORERS_DRIVER.log"
if not exist "SWOS_RUNTIME_AUDIT.log" goto incomplete
echo.
findstr /B /C:"OG_CUP_DEFINITIONS=" "SWOS_RUNTIME_AUDIT.log"
findstr /B /C:"OG_CUP_RESIDENT=" "SWOS_RUNTIME_AUDIT.log"
findstr /B /C:"OG_CUPS_BEFORE_AFTER=" "SWOS_RUNTIME_AUDIT.log"
findstr /B /C:"OG_CUP_NATIVE_MATCHES=" "SWOS_RUNTIME_AUDIT.log"
findstr /B /C:"OG_SCORERS_BEFORE_AFTER=" "SWOS_RUNTIME_AUDIT.log"
findstr /B /C:"OG_TOP_GOAL_SCORERS_VIEWER=" "SWOS_RUNTIME_AUDIT.log"
findstr /B /C:"OG_OFFER_CUPS_SCORERS_BEFORE_AFTER=" "SWOS_RUNTIME_AUDIT.log"
if "%ANALYSIS%"=="1" goto failure
findstr /B /C:"RUNTIME_LOG=READ" "SWOS_RUNTIME_AUDIT.log" | findstr /C:"scope=CURRENT_BUILD" >nul
if errorlevel 1 goto incomplete
findstr /B /C:"OG_CUPS_BEFORE_AFTER=" "SWOS_RUNTIME_AUDIT.log" | findstr /C:"NOT_IMPLEMENTED" >nul
if not errorlevel 1 (
 echo [INCOMPLETE] La coppa esiste nei dati ma non e' residente.
)
findstr /B /C:"OG_SCORERS_BEFORE_AFTER=" "SWOS_RUNTIME_AUDIT.log" | findstr /C:"PLAYER_COUNTERS_EMPTY" >nul
if not errorlevel 1 (
 echo [FAIL] Il campionato OG ha gol ma i contatori giocatori sono vuoti.
 goto failure
)
findstr /B /C:"OG_CUPS_BEFORE_AFTER=" "SWOS_RUNTIME_AUDIT.log" | findstr /C:"AFTER_MISSING" >nul
if not errorlevel 1 goto incomplete
findstr /B /C:"OG_SCORERS_BEFORE_AFTER=" "SWOS_RUNTIME_AUDIT.log" | findstr /C:"AFTER_MISSING" >nul
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
pause
exit /b %RESULT%
