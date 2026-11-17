@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"
title SWOS Expanded - Tutti i test + ZIP automatico

REM ONE CLICK, all historical and current tests, no save manipulation.
REM No test executable is allowed to act as a second gameplay engine.
set "SWOS_DIAGNOSTIC_SESSION=1"
set "SWOS_DIAGNOSTIC_CAPTURE=1"
set "RESULTS_ROOT=%~dp0diagnostica\RISULTATI_DIAGNOSTICI"
if not exist "%RESULTS_ROOT%" mkdir "%RESULTS_ROOT%"
if not exist "%RESULTS_ROOT%" (
 echo ERRORE: impossibile creare RISULTATI_DIAGNOSTICI.
 pause
 exit /b 2
)
set "SESSION_NAME="
for /L %%N in (1,1,999) do if not defined SESSION_NAME if not exist "%RESULTS_ROOT%\DIAGNOSTICA_SESSIONE_%%N" set "SESSION_NAME=DIAGNOSTICA_SESSIONE_%%N"
if not defined SESSION_NAME (
 echo ERRORE: nessuno slot libero per le sessioni diagnostiche.
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
mkdir "%SESSION%\LOG_PRECEDENTI" >nul 2>&1
set "SWOS_LIVE_LOG=%~dp0diagnostica\log\SWOS.log"
if exist "%SWOS_LIVE_LOG%" (
 move /Y "%SWOS_LIVE_LOG%" "%SESSION%\LOG_PRECEDENTI\SWOS_PRE_SESSIONE_DIAGNOSTICA.log" >nul
 if exist "%SWOS_LIVE_LOG%" (
  echo [ERRORE] Log precedente non archiviabile.
  exit /b 2
 )
)
set "SWOS_DIAGNOSTIC_DIR=%SESSION%"
set "EXPECTED=UNKNOWN"
for /f "tokens=3" %%V in ('findstr /B /C:"#define SWOS_EXPANDED_VERSION" "sorgenti\include\build_info.h"') do set "EXPECTED=%%~V"
set "REVISION=NON_DISPONIBILE"
for /f "delims=" %%H in ('git rev-parse --verify HEAD 2^>nul') do set "REVISION=%%H"
(
 echo SWOS EXPANDED - TUTTI I TEST - RISULTATI DIAGNOSTICI
 echo BUILD_ATTESA=!EXPECTED!
 echo HEAD_GIT_LOCALE=!REVISION!
 echo SESSIONE=%SESSION_NAME%
 echo DATA_INIZIO=%DATE% %TIME%
 echo MOTORE_CARRIERA=OG_UNICO
 echo Un avvio: build, gioco, OG current, vecchi test inclusi, audit, ZIP.
 echo Tutti i test isolated hanno log separato nella cartella TEST_ESEGUITI.
 echo Il test reale F12+8 / 70 stagioni richiede sempre gameplay Windows.
 echo PASS dei test legacy non equivale al PASS del gameplay OG.
 echo No reset, modifica o cancellazione dei file save da parte del BAT.
 echo Coppe internazionali, nazionali e save OG: audit read-only, NON PASS automatico.
 echo [TEST MANUALI] Verifica entrambi i mondi in sessioni distinte senza sovrascrivere save.
 echo [TEST MANUALI] Prima/dopo contratto: classifica, marcatori, coppe del club, BACK.
 echo [TEST MANUALI] SAVE: annota data, squadre, risultati, marcatori e coppe; chiudi e riapri con una nuova diagnostica.
 echo [TEST MANUALI] 70 stagioni F12+8 soltanto in carriera sacrificabile.
 echo [TEST MANUALI] Dopo autosave: torna al TITLE; il parser verifica la compatibilita' CAR sul disco.
 echo [TEST MANUALI] Champions/Europa/Conference, UEFA storica, Libertadores e tornei nazionali NON sono provati dal solo inventario.
) > "%SESSION%\LEGGIMI_RISULTATI.txt"

REM A previous SWOS.log must NEVER be mistaken for gameplay of this build.
REM Move historical LOG ONLY into this session; no saves are touched.
for %%L in (*.log) do (
 if /I not "%%~nxL"=="SWOS.log" (
  move /Y "%%~fL" "%SESSION%\LOG_PRECEDENTI\%%~nxL" >nul
 )
)
if exist "SWOS.log" (
 move /Y "SWOS.log" "%SESSION%\LOG_PRECEDENTI\SWOS_PRE_SESSIONE.log" >nul
 if exist "SWOS.log" (
  echo [ERRORE] SWOS.log precedente non archiviabile. Prove runtime non sicure.
  >> "%SESSION%\LEGGIMI_RISULTATI.txt" echo ERRORE_LOG_PRECEDENTE_NON_ARCHIVIATO=1
  pause
  exit /b 2
 )
)
echo ===============================================================
echo SWOS EXPANDED - DIAGNOSTICA UNICA !EXPECTED!
echo Tutti i risultati: %SESSION%
echo Nessun save viene toccato dal raccoglitore diagnostico.
echo ===============================================================
echo 1/5 Compilazione e tentativo di avvio gioco...
echo In caso di errore, eseguo comunque tutti i test indipendenti.
call "%~dp0AVVIA_SWOS.bat" > "%SESSION%\AVVIO_COMPILAZIONE.log" 2>&1
set "GAME_EXIT=!ERRORLEVEL!"
echo AVVIO_COMPILAZIONE_EXIT=!GAME_EXIT!
if not "!GAME_EXIT!"=="0" (
 echo [FAIL] La compilazione o l'avvio non sono riusciti.
 echo [DETTAGLI] Errori dal log attuale:
 findstr /I /C:"error:" /C:"errore:" /C:"compilazione fallita" /C:"il gioco non e stato avviato" "%SESSION%\AVVIO_COMPILAZIONE.log"
 echo Vedi %SESSION%\AVVIO_COMPILAZIONE.log
)
if exist "%SWOS_LIVE_LOG%" (
 copy /Y "%SWOS_LIVE_LOG%" "%SESSION%\SWOS.log" >nul
 echo [GAMEPLAY] Log corrente acquisito.
) else (
 > "%SESSION%\SWOS_LOG_MANCANTE.txt" echo SWOS.log di questa sessione non prodotto. GAMEPLAY=NOT_TESTED.
 echo [GAMEPLAY] Nessun SWOS.log nuovo: NON PROVATO.
)
if not exist "%SESSION%\COMPILAZIONE_0140.log" (
 > "%SESSION%\COMPILAZIONE_NON_COMPLETATA.txt" echo Build log mancante: leggere AVVIO_COMPILAZIONE.log.
)
REM Capture only known career slots AFTER game exit. These are COPIES, never
REM moves/loads/rewrites. No save is read before the game closes.
mkdir "%SESSION%\SAVE_COPIE" >nul 2>&1
> "%SESSION%\SAVE_COPIE\MANIFEST.txt" echo SWOS EXPANDED SAVE SNAPSHOT AT SESSION END (READ-ONLY SOURCE)
>> "%SESSION%\SAVE_COPIE\MANIFEST.txt" echo SAVE_SOURCE=save\ next to EXE
>> "%SESSION%\SAVE_COPIE\MANIFEST.txt" echo CAPTURE_TIME=%DATE% %TIME%
>> "%SESSION%\SAVE_COPIE\MANIFEST.txt" echo WARNING=CAR_MAY_INCLUDE_MANAGER_PERSONAL_DATA
set "SAVE_SNAPSHOT_COUNT=0"
set "SAVE_SNAPSHOT_ERRORS=0"
for %%S in (AUTO9697.CAR AUTO2627.CAR AUTOSAVE.CAR) do (
 if exist "save\%%S" (
  copy /B /Y "save\%%S" "%SESSION%\SAVE_COPIE\%%S" >nul
  if errorlevel 1 (
   >> "%SESSION%\SAVE_COPIE\MANIFEST.txt" echo FILE=%%S result=COPY_FAILED
   set /a SAVE_SNAPSHOT_ERRORS+=1 >nul
  ) else (
   fc /B "save\%%S" "%SESSION%\SAVE_COPIE\%%S" >nul
   if errorlevel 1 (
    >> "%SESSION%\SAVE_COPIE\MANIFEST.txt" echo FILE=%%S result=BYTE_COMPARE_FAILED
    set /a SAVE_SNAPSHOT_ERRORS+=1 >nul
   ) else (
    >> "%SESSION%\SAVE_COPIE\MANIFEST.txt" echo FILE=%%S result=BYTE_IDENTICAL_COPY
    set /a SAVE_SNAPSHOT_COUNT+=1 >nul
    if exist "%SystemRoot%\System32\certutil.exe" (
     "%SystemRoot%\System32\certutil.exe" -hashfile "%SESSION%\SAVE_COPIE\%%S" SHA256 > "%SESSION%\SAVE_COPIE\%%S.SHA256.txt" 2>&1
    )
   )
  )
 )
)
>> "%SESSION%\LEGGIMI_RISULTATI.txt" echo SAVE_SNAPSHOT_COUNT=!SAVE_SNAPSHOT_COUNT! errors=!SAVE_SNAPSHOT_ERRORS!
echo [SAVE] Copie archiviate: !SAVE_SNAPSHOT_COUNT!, errori: !SAVE_SNAPSHOT_ERRORS! (nessun save sorgente modificato).
echo.
echo 2/5 Audit del SOLO log Windows di questa sessione...
call "%~dp0diagnostica\ANALIZZA_SWOS_LOG.bat" > "%SESSION%\ANALISI_RUNTIME_DRIVER.log" 2>&1
set "AUDIT_EXIT=!ERRORLEVEL!"
echo AUDIT_EXIT=!AUDIT_EXIT!
echo.
echo 3/5 Tutti i test OG attuali e storici, anche quando la build fallisce...
call "%~dp0diagnostica\TEST_OG_WORLD.bat" > "%SESSION%\TEST_OG_WORLD_DRIVER.log" 2>&1
set "STATIC_EXIT=!ERRORLEVEL!"
echo TEST_SUITE_EXIT=!STATIC_EXIT!
if exist "%SESSION%\TEST_ESEGUITI\RIEPILOGO_TEST.txt" (
 type "%SESSION%\TEST_ESEGUITI\RIEPILOGO_TEST.txt"
)
echo.
echo 4/5 Controlli 70 stagioni, coppe e marcatori sullo STESSO audit...
call "%~dp0diagnostica\TEST_OG_70.bat" > "%SESSION%\TEST_OG_70_DRIVER.log" 2>&1
set "STRESS_EXIT=!ERRORLEVEL!"
call "%~dp0diagnostica\TEST_OG_CUPS_SCORERS.bat" > "%SESSION%\TEST_OG_CUPS_SCORERS_DRIVER.log" 2>&1
set "CUPS_EXIT=!ERRORLEVEL!"
REM Compatibility alias, never run the same OG tests a second time.
> "%SESSION%\TEST_LIVING_WORLD_ALIAS.txt" echo TEST_LIVING_WORLD.bat = TEST_OG_WORLD.bat: already executed once.
REM Prefer the freshly generated format report, if available. Otherwise
REM copy the historical baseline moved under docs/reference to keep root clean.
if exist "docs\reference\COMPETITION_FORMAT_AUDIT.csv" (
 copy /Y "docs\reference\COMPETITION_FORMAT_AUDIT.csv" "%SESSION%\TEST_ESEGUITI\COMPETITION_FORMAT_AUDIT.csv" >nul
)
(
 echo DATA_FINE=%DATE% %TIME%
 echo AVVIO_COMPILAZIONE_EXIT=!GAME_EXIT!
 echo RUNTIME_AUDIT_EXIT=!AUDIT_EXIT!
 echo TEST_OG_SOURCE_AND_LEGACY_EXIT=!STATIC_EXIT!
 echo NATIVE_70_SEASONS_EXIT=!STRESS_EXIT!
 echo CUPS_SCORERS_EXIT=!CUPS_EXIT!
 echo AUDIT_FILE=SWOS_RUNTIME_AUDIT.log
 echo CURRENT_SOURCE_TESTS=TEST_ESEGUITI\RIEPILOGO_TEST.txt
 echo COMPILER_WARNING_AUDIT=COMPILER_WARNING_AUDIT.log
 echo SAVE_CONTINUITY_AUDIT=OG_SAVE_CONTINUITY_AUDIT.log
 echo TITLE_AUTOSAVE=OG_TITLE_AUTOSAVE in SWOS_RUNTIME_AUDIT.log
 echo SAVE_COPIES=SAVE_COPIE\ (read-only, separate CAR per world)
 echo EXTERNAL_CUPS=OG_EXTERNAL_COMPETITION per named international/national cup in SWOS_RUNTIME_AUDIT.log
 echo NATIVE_CUP_FORMAT=OG_CUP_FORMAT_RULES in SWOS_RUNTIME_AUDIT.log
 echo INTERNATIONAL_NATIONAL=OG_INTERNATIONAL_NATIONAL in SWOS_RUNTIME_AUDIT.log
 echo 0=SUCCESS_SPECIFIC_TEST 1=FAIL 2=INCOMPLETE_OR_NOT_TESTED
) >> "%SESSION%\LEGGIMI_RISULTATI.txt"
set "RESULT=0"
if not "!GAME_EXIT!"=="0" set "RESULT=1"
if "!AUDIT_EXIT!"=="1" set "RESULT=1"
if "!STATIC_EXIT!"=="1" set "RESULT=1"
if "!STRESS_EXIT!"=="1" set "RESULT=1"
if "!CUPS_EXIT!"=="1" set "RESULT=1"
if not "!SAVE_SNAPSHOT_ERRORS!"=="0" set "RESULT=1"
if "!RESULT!"=="0" (
 if not "!AUDIT_EXIT!"=="0" set "RESULT=2"
 if not "!STATIC_EXIT!"=="0" set "RESULT=2"
 if not "!STRESS_EXIT!"=="0" set "RESULT=2"
 if not "!CUPS_EXIT!"=="0" set "RESULT=2"
)
echo RISULTATO_SESIONE=!RESULT!>> "%SESSION%\LEGGIMI_RISULTATI.txt"

echo.
echo 5/5 ZIP automatico e verifica archivio...
set "ZIP_FILE=%RESULTS_ROOT%\%SESSION_NAME%.zip"
set "ZIP_OK=0"
> "%SESSION%\ZIP_STATUS.txt" echo ZIP=CREAZIONE_IN_CORSO
if exist "%SystemRoot%\System32\tar.exe" (
 "%SystemRoot%\System32\tar.exe" -a -cf "%ZIP_FILE%" -C "%RESULTS_ROOT%" "%SESSION_NAME%" > "%SESSION%\ZIP_CREAZIONE.log" 2>&1
 if not errorlevel 1 (
  "%SystemRoot%\System32\tar.exe" -tf "%ZIP_FILE%" > "%SESSION%\ZIP_VERIFICA.log" 2>&1
  if not errorlevel 1 (
   findstr /C:"LEGGIMI_RISULTATI.txt" "%SESSION%\ZIP_VERIFICA.log" >nul
   if not errorlevel 1 set "ZIP_OK=1"
  )
 )
 if "!ZIP_OK!"=="1" (
  > "%SESSION%\ZIP_STATUS.txt" echo ZIP=REPORT_INCLUSI_IN_ARCHIVIO
  REM Repack to INCLUDE the verification report and ZIP_STATUS itself.
  "%SystemRoot%\System32\tar.exe" -a -cf "%ZIP_FILE%" -C "%RESULTS_ROOT%" "%SESSION_NAME%" >nul 2>&1
  if errorlevel 1 set "ZIP_OK=0"
  if "!ZIP_OK!"=="1" (
   "%SystemRoot%\System32\tar.exe" -tf "%ZIP_FILE%" >nul 2>&1
   if errorlevel 1 set "ZIP_OK=0"
  )
  REM A listing alone does not prove that archived bytes can be extracted.
  if "!ZIP_OK!"=="1" (
   "%SystemRoot%\System32\tar.exe" -xOf "%ZIP_FILE%" "%SESSION_NAME%/LEGGIMI_RISULTATI.txt" >nul 2>&1
   if errorlevel 1 set "ZIP_OK=0"
  )
 )
) else (
 > "%SESSION%\ZIP_CREAZIONE.log" echo tar.exe Windows mancante, ZIP non creato.
)
if "!ZIP_OK!"=="1" (
 echo [ZIP OK] %ZIP_FILE%
) else (
 echo [ZIP FAIL] Archivio non verificato. Tutti i log restano nella cartella.
 > "%SESSION%\ZIP_STATUS.txt" echo ZIP=NON_VERIFICATO
 if "!RESULT!"=="0" set "RESULT=2"
)
echo.
echo DIAGNOSTICA_CONCLUSA risultato=!RESULT!
echo Cartella: %SESSION%
echo ZIP: %ZIP_FILE%
echo Se il gioco non e partito, invia lo ZIP e leggi AVVIO_COMPILAZIONE.log.
pause
exit /b !RESULT!
