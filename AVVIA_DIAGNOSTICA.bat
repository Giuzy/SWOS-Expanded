@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"
title SWOS Expanded - Diagnostica unificata OG

REM Un unico punto d'avvio per TUTTI i test correnti. La compilazione,
REM il gameplay reale e le verifiche post-partita rimangono distinti.
REM Non sovrascrive, sposta o cancella alcun salvataggio.
set "SWOS_DIAGNOSTIC_SESSION=1"
set "SWOS_DIAGNOSTIC_CAPTURE=1"
set "RESULTS_ROOT=%~dp0RISULTATI_DIAGNOSTICI"
if not exist "%RESULTS_ROOT%" mkdir "%RESULTS_ROOT%"
if not exist "%RESULTS_ROOT%" (
 echo ERRORE: impossibile creare RISULTATI_DIAGNOSTICI.
 pause
 exit /b 2
)
set "SESSION_NAME="
for /L %%N in (1,1,999) do if not defined SESSION_NAME if not exist "%RESULTS_ROOT%\DIAGNOSTICA_SESSIONE_%%N" set "SESSION_NAME=DIAGNOSTICA_SESSIONE_%%N"
if not defined SESSION_NAME (
 echo ERRORE: nessuno slot libero per la diagnostica.
 pause
 exit /b 2
)
set "SESSION=%RESULTS_ROOT%\%SESSION_NAME%"
mkdir "%SESSION%"
if errorlevel 1 (
 echo ERRORE: impossibile creare la cartella della sessione.
 pause
 exit /b 2
)
set "SWOS_DIAGNOSTIC_DIR=%SESSION%"
set "EXPECTED=UNKNOWN"
for /f "tokens=3" %%V in ('findstr /B /C:"#define SWOS_EXPANDED_VERSION" "sorgenti\include\build_info.h"') do set "EXPECTED=%%~V"
(
 echo SWOS EXPANDED - RISULTATI DIAGNOSTICI
 echo BUILD_ATTESA=!EXPECTED!
 echo SESSIONE=%SESSION_NAME%
 echo DATA_INIZIO=%DATE% %TIME%
 echo SORGENTI=dev/0.10; il commit esatto deve essere verificato con GitKraken.
 echo TEST_AUTOMATICI=compilazione, runtime Windows, parser, calendario/date, suite OG, 70 anni, coppe, marcatori.
 echo Test 70 anni: solo un trigger F12+8 e FORWARD TIME nel gioco su carriera sacrificabile.
 echo TUTTI I PASS SONO LIMITATI AL CONTROLLO EFFETTIVAMENTE ESEGUITO.
 echo SAVE: nessun salvataggio viene cancellato o sovrascritto dal raccoglitore.
 echo Le coppe non residenti e il salvataggio completo OG non possono essere dichiarati PASS.
) > "%SESSION%\LEGGIMI_RISULTATI.txt"

echo ==========================================================
echo SWOS EXPANDED - DIAGNOSTICA UNICA
echo Tutti i risultati: %SESSION%
echo ==========================================================
echo Il gioco sara compilato e avviato come da AVVIA_SWOS.bat.
echo Per stress reale: NUOVA carriera senza club, F12+8, FORWARD TIME.
echo Per la continuita: guarda marcatori e coppe PRIMA e DOPO il contratto.
echo I controlli automatizzati partono dopo la chiusura del gioco.
echo ==========================================================
echo.
call "%~dp0AVVIA_SWOS.bat" > "%SESSION%\AVVIO_COMPILAZIONE.log" 2>&1
set "GAME_EXIT=!ERRORLEVEL!"
echo [DIAGNOSTICA] AVVIO/COMPILAZIONE exit=!GAME_EXIT!

if exist "SWOS.log" (
 copy /Y "SWOS.log" "%SESSION%\SWOS.log" >nul
) else (
 > "%SESSION%\SWOS_LOG_MANCANTE.txt" echo SWOS.log non trovato. Gameplay NON PROVATO.
)

REM Compilazione: BUILD_SWOS.sh usa SWOS_DIAGNOSTIC_DIR per generare
REM COMPILAZIONE_0140.log direttamente qui. Compatibilita coi vecchi log:
REM non eliminarli ne considerarli prove di QUESTA sessione.
if not exist "%SESSION%\COMPILAZIONE_0140.log" (
 > "%SESSION%\COMPILAZIONE_NON_COMPLETATA.txt" echo Nessun log di build di questa sessione. Vedere AVVIO_COMPILAZIONE.log.
)

REM Test statici e audit reali sono separati: il parser legge SOLO SWOS.log.
call "%~dp0ANALIZZA_SWOS_LOG.bat" > "%SESSION%\ANALISI_RUNTIME_DRIVER.log" 2>&1
set "AUDIT_EXIT=!ERRORLEVEL!"
call "%~dp0TEST_OG_WORLD.bat" > "%SESSION%\TEST_OG_WORLD_DRIVER.log" 2>&1
set "STATIC_EXIT=!ERRORLEVEL!"
call "%~dp0TEST_OG_70.bat" > "%SESSION%\TEST_OG_70_DRIVER.log" 2>&1
set "STRESS_EXIT=!ERRORLEVEL!"
call "%~dp0TEST_OG_CUPS_SCORERS.bat" > "%SESSION%\TEST_OG_CUPS_SCORERS_DRIVER.log" 2>&1
set "CUPS_EXIT=!ERRORLEVEL!"

REM TEST_LIVING_WORLD.bat e' un alias storico di TEST_OG_WORLD.bat:
REM non rilanciare la stessa suite una seconda volta.
> "%SESSION%\TEST_LIVING_WORLD_LEGACY_ALIAS.txt" echo Alias di TEST_OG_WORLD: eseguito UNA volta, nessun secondo simulatore.
REM Copia i log storici che gia esistevano prima della sessione senza
REM confonderli con i risultati nuovi ne cancellare originali.
mkdir "%SESSION%\LOG_PRECEDENTI" >nul 2>&1
for %%L in (*.log) do (
 if /I not "%%~nxL"=="SWOS.log" (
  copy /Y "%%~fL" "%SESSION%\LOG_PRECEDENTI\%%~nxL" >nul
 )
)
(
 echo.
 echo DATA_FINE=%DATE% %TIME%
 echo AVVIO_COMPILAZIONE_EXIT=!GAME_EXIT!
 echo RUNTIME_AUDIT_EXIT=!AUDIT_EXIT!
 echo SUITE_OG_EXIT=!STATIC_EXIT!
 echo TEST_70_STAGIONI_EXIT=!STRESS_EXIT!
 echo COPPE_E_MARCATORI_EXIT=!CUPS_EXIT!
 echo 0=PASS DEL SINGOLO TEST; 1=FAIL; 2=INCOMPLETE/NOT_TESTED.
 echo COMPILAZIONE_WINDOWS: verificare COMPILAZIONE_0140.log e AVVIO_COMPILAZIONE.log.
 echo GAMEPLAY_WINDOWS: verificare SWOS.log e SWOS_RUNTIME_AUDIT.log.
 echo ATTENZIONE: test statici da soli non provano il mondo OG.
) >> "%SESSION%\LEGGIMI_RISULTATI.txt"
set "RESULT=0"
if not "!GAME_EXIT!"=="0" set "RESULT=1"
if "!AUDIT_EXIT!"=="1" set "RESULT=1"
if "!STATIC_EXIT!"=="1" set "RESULT=1"
if "!STRESS_EXIT!"=="1" set "RESULT=1"
if "!CUPS_EXIT!"=="1" set "RESULT=1"
if "!RESULT!"=="0" (
 if not "!AUDIT_EXIT!"=="0" set "RESULT=2"
 if not "!STATIC_EXIT!"=="0" set "RESULT=2"
 if not "!STRESS_EXIT!"=="0" set "RESULT=2"
 if not "!CUPS_EXIT!"=="0" set "RESULT=2"
)
REM Windows 10/11 tar: ZIP compatto di TUTTI i risultati.
if exist "%SystemRoot%\System32\tar.exe" (
 "%SystemRoot%\System32\tar.exe" -a -cf "%RESULTS_ROOT%\%SESSION_NAME%.zip" -C "%RESULTS_ROOT%" "%SESSION_NAME%" >nul 2>&1
)
echo.
echo DIAGNOSTICA_CONCLUSA risultato=!RESULT!
echo Cartella: %SESSION%
if exist "%RESULTS_ROOT%\%SESSION_NAME%.zip" echo ZIP: %RESULTS_ROOT%\%SESSION_NAME%.zip
echo Nessun save e' stato modificato dal raccoglitore diagnostico.
echo.
pause
exit /b !RESULT!
