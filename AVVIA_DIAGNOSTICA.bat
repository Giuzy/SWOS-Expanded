@echo off
setlocal EnableExtensions EnableDelayedExpansion
cd /d "%~dp0"
title SWOS Expanded - Due avvii + ZIP automatico

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
 echo MOTORE_CARRIERA=CE_JSON_2627_OG_SOLO_50HZ
 echo Una compilazione, due processi gioco distinti (solo dopo chiusura regolare), audit e ZIP.
 echo Tutti i test isolated hanno log separato nella cartella TEST_ESEGUITI.
 echo Il test reale F12+8 / 70 stagioni richiede sempre gameplay Windows.
 echo PASS dei test legacy non equivale al PASS del gameplay OG.
 echo No reset, modifica o cancellazione dei file save da parte del BAT.
 echo Coppe internazionali, nazionali e save OG: audit read-only, NON PASS automatico.
 echo [TEST MANUALI] Verifica entrambi i mondi in sessioni distinte senza sovrascrivere save.
 echo [TEST MANUALI] Prima/dopo contratto: classifica, marcatori, coppe del club, BACK.
 echo [TEST MANUALI] SAVE: annota data, squadre, risultati, marcatori e coppe; chiudi normalmente il gioco e riaprilo nella stessa diagnostica per CONTINUE.
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
REM Gating esplicito: la compilazione non riuscita NON deve avviare un vecchio EXE.
set "BUILD_EXIT=2"
set "RUN1_EXIT=2"
set "RUN2_EXIT=2"
set "RUN1_LOG_EXIT=2"
set "RUN2_LOG_EXIT=2"
set "RUN1_CLEAN=0"
set "RUN2_CLEAN=0"
set "RUN2_STATUS=NOT_RUN"
set "CONTINUE_MARKER=NOT_OBSERVED"
set "SAVE_SNAPSHOT_COUNT=0"
set "SAVE_SNAPSHOT_ERRORS=0"
set "AUDIT_RUN1_EXIT=2"
set "AUDIT_RUN2_EXIT=2"
set "AUDIT_EXIT=2"
set "STATIC_EXIT=2"
set "STRESS_EXIT=2"
set "CUPS_EXIT=2"
set "RESULT=2"
mkdir "%SESSION%\RUN1" >nul 2>&1
mkdir "%SESSION%\RUN2" >nul 2>&1
if not exist "%SESSION%\RUN1" goto :setup_failed
if not exist "%SESSION%\RUN2" goto :setup_failed

echo 1/5 Compilazione della build corrente SENZA avviare il gioco...
set "SWOS_DIAGNOSTIC_RUN_ONLY="
set "SWOS_DIAGNOSTIC_BUILD_ONLY=1"
call "%~dp0AVVIA_SWOS.bat" > "%SESSION%\AVVIO_COMPILAZIONE.log" 2>&1
set "BUILD_EXIT=!ERRORLEVEL!"
set "SWOS_DIAGNOSTIC_BUILD_ONLY="
echo COMPILAZIONE_EXIT=!BUILD_EXIT!
>> "%SESSION%\LEGGIMI_RISULTATI.txt" echo COMPILAZIONE_EXIT=!BUILD_EXIT!
if not "!BUILD_EXIT!"=="0" goto :build_failed
if not exist "SWOS_Expanded.exe" goto :build_failed
if not exist "SDL2.dll" goto :build_failed
if not exist "data\TEAM.008" goto :build_failed
echo [BUILD OK] EXE presente. Solo ora sono ammessi gli avvii gioco.
call :snapshot_cars BEFORE_RUN1
if not "!SAVE_SNAPSHOT_ERRORS!"=="0" (
 set "RUN2_STATUS=SKIPPED_SNAPSHOT_ERROR"
 goto :after_games
)

echo.
echo 2/5 Primo avvio: prova il gioco e CHIUDI NORMALMENTE.
set "SWOS_DIAGNOSTIC_RUN_ONLY=1"
call "%~dp0AVVIA_SWOS.bat" > "%SESSION%\RUN1\AVVIO.log" 2>&1
set "RUN1_EXIT=!ERRORLEVEL!"
call :capture_log RUN1
set "RUN1_LOG_EXIT=!ERRORLEVEL!"
call :snapshot_cars AFTER_RUN1
echo PRIMO_AVVIO_EXIT=!RUN1_EXIT! LOG_EXIT=!RUN1_LOG_EXIT!
if not "!RUN1_EXIT!"=="0" (
 set "RUN2_STATUS=SKIPPED_CRASH_OR_ABNORMAL_EXIT"
 goto :after_games
)
if not "!RUN1_LOG_EXIT!"=="0" (
 set "RUN2_STATUS=SKIPPED_FIRST_LOG_UNAVAILABLE"
 goto :after_games
)
REM Un Windows "Il programma non risponde -> Chiudi", End Task, crash
REM o terminazione forzata NON attraversa main.c::CLEAN_SHUTDOWN_COMPLETED.
REM Il codice di uscita da solo NON certifica la chiusura normale.
findstr /C:"[diagnostic] CLEAN_SHUTDOWN_COMPLETED=1" "%SESSION%\RUN1\SWOS.log" >nul 2>&1
if errorlevel 1 (
 set "RUN2_STATUS=SKIPPED_NO_CLEAN_SHUTDOWN"
 goto :after_games
)
set "RUN1_CLEAN=1"
if not "!SAVE_SNAPSHOT_ERRORS!"=="0" (
 set "RUN2_STATUS=SKIPPED_SNAPSHOT_ERROR"
 goto :after_games
)

echo.
echo 3/5 Primo gioco chiuso REGOLARMENTE. La diagnostica resta aperta.
echo Il secondo avvio NON ricompila: seleziona CONTINUE CAREER, prova e chiudi.
echo Nessun salvataggio viene cancellato o ripristinato automaticamente.
choice /C SN /N /M "Avvia lo stesso EXE per verificare CONTINUE? [S/N] "
set "CHOICE_EXIT=!ERRORLEVEL!"
if "!CHOICE_EXIT!"=="2" (
 set "RUN2_STATUS=SKIPPED_BY_USER"
 goto :after_games
)
if not "!CHOICE_EXIT!"=="1" (
 set "RUN2_STATUS=SKIPPED_PROMPT_ERROR"
 goto :after_games
)

set "RUN2_STATUS=STARTED"
call "%~dp0AVVIA_SWOS.bat" > "%SESSION%\RUN2\AVVIO.log" 2>&1
set "RUN2_EXIT=!ERRORLEVEL!"
call :capture_log RUN2
set "RUN2_LOG_EXIT=!ERRORLEVEL!"
call :snapshot_cars AFTER_RUN2
if not "!RUN2_EXIT!"=="0" (
 set "RUN2_STATUS=FAILED_OR_CRASHED"
) else if not "!RUN2_LOG_EXIT!"=="0" (
 set "RUN2_STATUS=NO_CURRENT_LOG"
) else (
 findstr /C:"[diagnostic] CLEAN_SHUTDOWN_COMPLETED=1" "%SESSION%\RUN2\SWOS.log" >nul 2>&1
 if errorlevel 1 (
  set "RUN2_STATUS=ABNORMAL_CLOSE_NO_SENTINEL"
 ) else (
  set "RUN2_CLEAN=1"
  set "RUN2_STATUS=FINISHED"
 )
)
echo SECONDO_AVVIO_EXIT=!RUN2_EXIT! LOG_EXIT=!RUN2_LOG_EXIT!

:after_games
set "SWOS_DIAGNOSTIC_RUN_ONLY="
>> "%SESSION%\LEGGIMI_RISULTATI.txt" echo RUN1_EXIT=!RUN1_EXIT! RUN1_LOG_EXIT=!RUN1_LOG_EXIT! RUN1_CLEAN=!RUN1_CLEAN!
>> "%SESSION%\LEGGIMI_RISULTATI.txt" echo RUN2_STATUS=!RUN2_STATUS! RUN2_EXIT=!RUN2_EXIT! RUN2_LOG_EXIT=!RUN2_LOG_EXIT! RUN2_CLEAN=!RUN2_CLEAN!
>> "%SESSION%\LEGGIMI_RISULTATI.txt" echo SAVE_SNAPSHOT_COUNT=!SAVE_SNAPSHOT_COUNT! errors=!SAVE_SNAPSHOT_ERRORS!
> "%SESSION%\CONTINUE_CONFRONTO.txt" echo RUN1_EXIT=!RUN1_EXIT! RUN1_CLEAN=!RUN1_CLEAN!
>> "%SESSION%\CONTINUE_CONFRONTO.txt" echo RUN2_STATUS=!RUN2_STATUS! RUN2_EXIT=!RUN2_EXIT! RUN2_CLEAN=!RUN2_CLEAN!
>> "%SESSION%\CONTINUE_CONFRONTO.txt" echo CAR_SNAPSHOTS=SAVE_COPIE\BEFORE_RUN1\,SAVE_COPIE\AFTER_RUN1\,SAVE_COPIE\AFTER_RUN2\
>> "%SESSION%\CONTINUE_CONFRONTO.txt" echo TWO_LAUNCHES_ALONE_DO_NOT_PROVE_RESTORE=1
if exist "%SESSION%\RUN2\SWOS.log" (
 findstr /I /C:"CONTINUE_RESTORED" /C:"RESTORE_AFTER" /C:"[autosave] restored" "%SESSION%\RUN2\SWOS.log" > "%SESSION%\RUN2\RESTORE_MARKERS.txt"
 if not errorlevel 1 set "CONTINUE_MARKER=OBSERVED_NEEDS_FIXTURE_REPLAY_AUDIT"
)
>> "%SESSION%\CONTINUE_CONFRONTO.txt" echo CONTINUE_MARKER=!CONTINUE_MARKER!
>> "%SESSION%\CONTINUE_CONFRONTO.txt" echo RESTORE_CERTIFIED=NOT_AUTOMATICALLY_PROVEN
>> "%SESSION%\LEGGIMI_RISULTATI.txt" echo CONTINUE_MARKER=!CONTINUE_MARKER!

echo.
echo 4/5 Audit dei log di entrambi gli avvii, poi test disponibili...
REM Il parser storico legge SESSION\SWOS.log. Ogni processo viene controllato
REM SEPARATAMENTE, senza concatenare o scambiare RUN1/RUN2.
for %%R in (RUN1 RUN2) do (
 if exist "%SESSION%\%%R\SWOS.log" (
  copy /Y "%SESSION%\%%R\SWOS.log" "%SESSION%\SWOS.log" >nul
  call "%~dp0diagnostica\ANALIZZA_SWOS_LOG.bat" > "%SESSION%\%%R\ANALISI_RUNTIME_DRIVER.log" 2>&1
  set "AUDIT_%%R_EXIT=!ERRORLEVEL!"
  if exist "%SESSION%\SWOS_RUNTIME_AUDIT.log" (
   move /Y "%SESSION%\SWOS_RUNTIME_AUDIT.log" "%SESSION%\%%R\SWOS_RUNTIME_AUDIT.log" >nul
  )
 )
)
if exist "%SESSION%\RUN2\SWOS.log" (
 copy /Y "%SESSION%\RUN2\SWOS.log" "%SESSION%\SWOS.log" >nul
 if exist "%SESSION%\RUN2\SWOS_RUNTIME_AUDIT.log" copy /Y "%SESSION%\RUN2\SWOS_RUNTIME_AUDIT.log" "%SESSION%\SWOS_RUNTIME_AUDIT.log" >nul
 set "AUDIT_EXIT=!AUDIT_RUN2_EXIT!"
) else if exist "%SESSION%\RUN1\SWOS.log" (
 copy /Y "%SESSION%\RUN1\SWOS.log" "%SESSION%\SWOS.log" >nul
 if exist "%SESSION%\RUN1\SWOS_RUNTIME_AUDIT.log" copy /Y "%SESSION%\RUN1\SWOS_RUNTIME_AUDIT.log" "%SESSION%\SWOS_RUNTIME_AUDIT.log" >nul
 set "AUDIT_EXIT=!AUDIT_RUN1_EXIT!"
)

REM Suite C11/test indipendenti solo se la build EXE corrente e' stata compilata.
call "%~dp0diagnostica\TEST_OG_WORLD.bat" > "%SESSION%\TEST_OG_WORLD_DRIVER.log" 2>&1
set "STATIC_EXIT=!ERRORLEVEL!"
if exist "%SESSION%\TEST_ESEGUITI\RIEPILOGO_TEST.txt" type "%SESSION%\TEST_ESEGUITI\RIEPILOGO_TEST.txt"
call "%~dp0diagnostica\TEST_OG_70.bat" > "%SESSION%\TEST_OG_70_DRIVER.log" 2>&1
set "STRESS_EXIT=!ERRORLEVEL!"
call "%~dp0diagnostica\TEST_OG_CUPS_SCORERS.bat" > "%SESSION%\TEST_OG_CUPS_SCORERS_DRIVER.log" 2>&1
set "CUPS_EXIT=!ERRORLEVEL!"
> "%SESSION%\TEST_LIVING_WORLD_ALIAS.txt" echo TEST_LIVING_WORLD.bat = TEST_OG_WORLD.bat: already executed once.
if exist "docs\reference\COMPETITION_FORMAT_AUDIT.csv" (
 if not exist "%SESSION%\TEST_ESEGUITI" mkdir "%SESSION%\TEST_ESEGUITI"
 copy /Y "docs\reference\COMPETITION_FORMAT_AUDIT.csv" "%SESSION%\TEST_ESEGUITI\COMPETITION_FORMAT_AUDIT.csv" >nul
)

set "RESULT=0"
if not "!RUN1_EXIT!"=="0" set "RESULT=1"
if not "!RUN1_CLEAN!"=="1" set "RESULT=1"
if "!RUN2_STATUS!"=="FAILED_OR_CRASHED" set "RESULT=1"
if "!RUN2_STATUS!"=="NO_CURRENT_LOG" set "RESULT=1"
if "!RUN2_STATUS!"=="ABNORMAL_CLOSE_NO_SENTINEL" set "RESULT=1"
if not "!SAVE_SNAPSHOT_ERRORS!"=="0" set "RESULT=1"
if "!AUDIT_RUN1_EXIT!"=="1" set "RESULT=1"
if "!AUDIT_RUN2_EXIT!"=="1" set "RESULT=1"
if "!STATIC_EXIT!"=="1" set "RESULT=1"
if "!STRESS_EXIT!"=="1" set "RESULT=1"
if "!CUPS_EXIT!"=="1" set "RESULT=1"
if "!RESULT!"=="0" (
 if not "!RUN2_STATUS!"=="FINISHED" set "RESULT=2"
 if not "!AUDIT_EXIT!"=="0" set "RESULT=2"
 if not "!STATIC_EXIT!"=="0" set "RESULT=2"
 if not "!STRESS_EXIT!"=="0" set "RESULT=2"
 if not "!CUPS_EXIT!"=="0" set "RESULT=2"
)
goto :final_report

:build_failed
set "RESULT=1"
set "RUN2_STATUS=NOT_RUN_BUILD_FAILED"
> "%SESSION%\TESTS_NON_ESEGUITI.txt" echo ALL_TEST_SUITES=NOT_RUN_BUILD_FAILED
>> "%SESSION%\TESTS_NON_ESEGUITI.txt" echo NO_STALE_EXECUTABLE_LAUNCHED=1
>> "%SESSION%\LEGGIMI_RISULTATI.txt" echo COMPILAZIONE_FALLITA=1 NO_GAME_RUN=1
if exist "%SESSION%\COMPILAZIONE_0140.log" (
 echo [COMPILAZIONE FALLITA] Log GCC: %SESSION%\COMPILAZIONE_0140.log
) else (
 echo [COMPILAZIONE FALLITA] Log: %SESSION%\AVVIO_COMPILAZIONE.log
)
goto :final_report

:setup_failed
set "RESULT=1"
set "RUN2_STATUS=NOT_RUN_SETUP_FAILED"
>> "%SESSION%\LEGGIMI_RISULTATI.txt" echo ERRORE_SESSIONE=IMPOSSIBILE_CREARE_CARTELLE_RUN
goto :final_report

:final_report
set "SWOS_DIAGNOSTIC_BUILD_ONLY="
set "SWOS_DIAGNOSTIC_RUN_ONLY="
(
 echo DATA_FINE=%DATE% %TIME%
 echo BUILD_EXIT=!BUILD_EXIT!
 echo RUN1_EXIT=!RUN1_EXIT! CLEAN=!RUN1_CLEAN!
 echo RUN2_STATUS=!RUN2_STATUS! RUN2_EXIT=!RUN2_EXIT! CLEAN=!RUN2_CLEAN!
 echo AUDIT_RUN1_EXIT=!AUDIT_RUN1_EXIT! AUDIT_RUN2_EXIT=!AUDIT_RUN2_EXIT!
 echo TEST_OG_SOURCE_AND_LEGACY_EXIT=!STATIC_EXIT!
 echo NATIVE_70_SEASONS_EXIT=!STRESS_EXIT!
 echo CUPS_SCORERS_EXIT=!CUPS_EXIT!
 echo SAVE_SNAPSHOT_COUNT=!SAVE_SNAPSHOT_COUNT! errors=!SAVE_SNAPSHOT_ERRORS!
 echo RESTORE_MARKER=!CONTINUE_MARKER!
 echo CONTINUE_RESULT=NOT_AUTOMATICALLY_CERTIFIED
 echo RUN1_LOG=RUN1\SWOS.log
 echo RUN2_LOG=RUN2\SWOS.log
 echo CAR_COPIES=SAVE_COPIE\ [never touches source]
 echo 0=SUCCESS_SPECIFIC_TEST 1=FAIL 2=INCOMPLETE_OR_NOT_TESTED
) >> "%SESSION%\LEGGIMI_RISULTATI.txt"
echo RISULTATO_SESIONE=!RESULT!>> "%SESSION%\LEGGIMI_RISULTATI.txt"

echo.
echo 5/5 ZIP unico e verifica archivio...
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


REM Below are read-only diagnostic subroutines: never write source CAR files.
:capture_log
set "CURRENT_RUN=%~1"
if not exist "%SWOS_LIVE_LOG%" (
 > "%SESSION%\%CURRENT_RUN%\LOG_MANCANTE.txt" echo SWOS.log nuovo non generato: GAMEPLAY_NOT_PROVEN.
 exit /b 2
)
move /Y "%SWOS_LIVE_LOG%" "%SESSION%\%CURRENT_RUN%\SWOS.log" >nul
if errorlevel 1 exit /b 2
if exist "%SWOS_LIVE_LOG%" exit /b 2
if not exist "%SESSION%\%CURRENT_RUN%\SWOS.log" exit /b 2
exit /b 0

:snapshot_cars
set "SNAPSHOT_LABEL=%~1"
if not exist "%SESSION%\SAVE_COPIE" mkdir "%SESSION%\SAVE_COPIE" >nul 2>&1
if not exist "%SESSION%\SAVE_COPIE\%SNAPSHOT_LABEL%" mkdir "%SESSION%\SAVE_COPIE\%SNAPSHOT_LABEL%" >nul 2>&1
if not exist "%SESSION%\SAVE_COPIE\%SNAPSHOT_LABEL%" (
 set /a SAVE_SNAPSHOT_ERRORS+=1 >nul
 exit /b 1
)
> "%SESSION%\SAVE_COPIE\%SNAPSHOT_LABEL%\MANIFEST.txt" echo SNAPSHOT=%SNAPSHOT_LABEL% TIME=%DATE% %TIME%
>> "%SESSION%\SAVE_COPIE\%SNAPSHOT_LABEL%\MANIFEST.txt" echo READ_ONLY_SOURCE=save\ next to EXE
>> "%SESSION%\SAVE_COPIE\%SNAPSHOT_LABEL%\MANIFEST.txt" echo WARNING=CAR_MAY_INCLUDE_MANAGER_PERSONAL_DATA
for %%S in (AUTO9697.CAR AUTO2627.CAR AUTOSAVE.CAR) do (
 if exist "save\%%S" (
  copy /B /Y "save\%%S" "%SESSION%\SAVE_COPIE\%SNAPSHOT_LABEL%\%%S" >nul
  if errorlevel 1 (
   >> "%SESSION%\SAVE_COPIE\%SNAPSHOT_LABEL%\MANIFEST.txt" echo FILE=%%S result=COPY_FAILED
   set /a SAVE_SNAPSHOT_ERRORS+=1 >nul
  ) else (
   fc /B "save\%%S" "%SESSION%\SAVE_COPIE\%SNAPSHOT_LABEL%\%%S" >nul
   if errorlevel 1 (
    >> "%SESSION%\SAVE_COPIE\%SNAPSHOT_LABEL%\MANIFEST.txt" echo FILE=%%S result=BYTE_COMPARE_FAILED
    set /a SAVE_SNAPSHOT_ERRORS+=1 >nul
   ) else (
    >> "%SESSION%\SAVE_COPIE\%SNAPSHOT_LABEL%\MANIFEST.txt" echo FILE=%%S result=BYTE_IDENTICAL_COPY
    set /a SAVE_SNAPSHOT_COUNT+=1 >nul
    if exist "%SystemRoot%\System32\certutil.exe" (
     "%SystemRoot%\System32\certutil.exe" -hashfile "%SESSION%\SAVE_COPIE\%SNAPSHOT_LABEL%\%%S" SHA256 > "%SESSION%\SAVE_COPIE\%SNAPSHOT_LABEL%\%%S.SHA256.txt" 2>&1
    )
   )
  )
 ) else (
  >> "%SESSION%\SAVE_COPIE\%SNAPSHOT_LABEL%\MANIFEST.txt" echo FILE=%%S result=NOT_PRESENT
 )
)
exit /b 0
