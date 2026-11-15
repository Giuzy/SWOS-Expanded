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
set "ORIGINAL_DIAG=%SWOS_DIAGNOSTIC_SESSION%"
set "SWOS_DIAGNOSTIC_SESSION=1"
set "AUDIT_LOG=SWOS_RUNTIME_AUDIT.log"
set "DRIVER_LOG=OG_70_RUNTIME_DRIVER.log"
if defined SWOS_DIAGNOSTIC_DIR (
 set "AUDIT_LOG=%SWOS_DIAGNOSTIC_DIR%\SWOS_RUNTIME_AUDIT.log"
 set "DRIVER_LOG=%SWOS_DIAGNOSTIC_DIR%\OG_70_RUNTIME_DRIVER.log"
)
REM Main AVVIA_DIAGNOSTICA already ran the C parser exactly once.
REM Standalone BAT still creates its own audit when necessary.
if defined SWOS_DIAGNOSTIC_DIR if exist "%AUDIT_LOG%" goto audit_already_done
call "%~dp0ANALIZZA_SWOS_LOG.bat" > "%DRIVER_LOG%" 2>&1
set "AUDIT_EXIT=%ERRORLEVEL%"
goto audit_available
:audit_already_done
set "AUDIT_EXIT=2"
findstr /B /C:"FIRST_RUNTIME_FAILURE:" "%AUDIT_LOG%" >nul
if not errorlevel 1 set "AUDIT_EXIT=1"
> "%DRIVER_LOG%" echo [REUSE] SWOS_RUNTIME_AUDIT.log generated once by AVVIA_DIAGNOSTICA.bat.
:audit_available
type "%DRIVER_LOG%"
if not exist "%AUDIT_LOG%" goto incomplete
echo.
findstr /B /C:"RUNTIME_LOG=READ" "%AUDIT_LOG%"
findstr /B /C:"OG_SEASON_TRANSITIONS=" "%AUDIT_LOG%"
findstr /B /C:"OG_70_NATIVE_SEASONS=" "%AUDIT_LOG%"
findstr /B /C:"OG_RETIREMENTS=" "%AUDIT_LOG%"
findstr /B /C:"OG_CUP_RESIDENT=" "%AUDIT_LOG%"
findstr /B /C:"OG_CUPS_BEFORE_AFTER=" "%AUDIT_LOG%"
findstr /B /C:"OG_CUP_NATIVE_MATCHES=" "%AUDIT_LOG%"
findstr /B /C:"OG_SCORERS_BEFORE_AFTER=" "%AUDIT_LOG%"
findstr /B /C:"OG_RESIDENT_SAVE_RELOAD=" "%AUDIT_LOG%"
findstr /B /C:"OG_INTERNATIONAL_NATIONAL=" "%AUDIT_LOG%"
findstr /B /C:"OG_LAST_WORLD_STEP=" "%AUDIT_LOG%"
findstr /B /C:"OG_OFFER_NATIVE_VIEW=" "%AUDIT_LOG%"
if "%AUDIT_EXIT%"=="1" goto failure
findstr /B /C:"RUNTIME_LOG=READ" "%AUDIT_LOG%" | findstr /C:"scope=CURRENT_BUILD" >nul
if errorlevel 1 goto incomplete
findstr /B /C:"OG_70_NATIVE_SEASONS=PASS_TRANSITIONS_ONLY" "%AUDIT_LOG%" >nul
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
if not "%ORIGINAL_DIAG%"=="1" pause
exit /b %RESULT%
