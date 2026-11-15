@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title SWOS Expanded - Audit 70 stagioni OG reali
echo ===========================================================
echo  TEST 70 STAGIONI - STATO RESIDENTE OPEN SWOS
echo ===========================================================
echo Questo BAT non inventa partite e non simula il mondo LWE.
echo Per generare dati: AVVIA_DIAGNOSTICA.bat - nuova carriera
echo UNEMPLOYED - F12+8 - chiudi F12 - FORWARD TIME una volta.
echo Il salto AVANZA LA CARRIERA REALE, un anno OG per fotogramma.
echo Non usare save importanti.
echo Il gioco deve completare il test e uscire normalmente.
echo Ora analizzo solo SWOS.log presente: non avvio un'altra carriera.
echo ===========================================================
set "SWOS_DIAGNOSTIC_SESSION=1"
call "%~dp0ANALIZZA_SWOS_LOG.bat" > "OG_70_RUNTIME_DRIVER.log" 2>&1
set "AUDIT_EXIT=%ERRORLEVEL%"
type "OG_70_RUNTIME_DRIVER.log"
if not exist "SWOS_RUNTIME_AUDIT.log" goto incomplete
echo.
findstr /B /C:"RUNTIME_LOG=READ" "SWOS_RUNTIME_AUDIT.log"
findstr /B /C:"OG_SEASON_TRANSITIONS=" "SWOS_RUNTIME_AUDIT.log"
findstr /B /C:"OG_70_NATIVE_SEASONS=" "SWOS_RUNTIME_AUDIT.log"
findstr /B /C:"OG_RETIREMENTS=" "SWOS_RUNTIME_AUDIT.log"
findstr /B /C:"OG_CUP_RESIDENT=" "SWOS_RUNTIME_AUDIT.log"
findstr /B /C:"OG_CUPS_BEFORE_AFTER=" "SWOS_RUNTIME_AUDIT.log"
findstr /B /C:"OG_CUP_NATIVE_MATCHES=" "SWOS_RUNTIME_AUDIT.log"
findstr /B /C:"OG_SCORERS_BEFORE_AFTER=" "SWOS_RUNTIME_AUDIT.log"
findstr /B /C:"OG_LAST_WORLD_STEP=" "SWOS_RUNTIME_AUDIT.log"
findstr /B /C:"OG_OFFER_NATIVE_VIEW=" "SWOS_RUNTIME_AUDIT.log"
if "%AUDIT_EXIT%"=="1" goto failure
findstr /B /C:"RUNTIME_LOG=READ" "SWOS_RUNTIME_AUDIT.log" | findstr /C:"scope=CURRENT_BUILD" >nul
if errorlevel 1 goto incomplete
findstr /B /C:"OG_70_NATIVE_SEASONS=PASS_TRANSITIONS_ONLY" "SWOS_RUNTIME_AUDIT.log" >nul
if errorlevel 1 goto incomplete
echo.
echo [PASS] 70 chiusure e 70 aperture di stagione OG e date esatte.
echo [INCOMPLETE] Coppe residenti, save/reload e pensionamenti separati.
set "RESULT=0"
goto done
:failure
echo.
echo [FAIL] Un errore OG risulta nel log. Invia i file diagnostici.
set "RESULT=1"
goto done
:incomplete
echo.
echo [INCOMPLETE] Manca il completamento di 70 stagioni OG Windows.
echo Eseguire la carriera tramite AVVIA_DIAGNOSTICA.bat e riprovare.
set "RESULT=2"
:done
echo Log utile: SWOS_RUNTIME_AUDIT.log / OG_70_RUNTIME_DRIVER.log.
pause
exit /b %RESULT%
