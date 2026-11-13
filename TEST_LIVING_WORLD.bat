@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title SWOS Expanded - Living World 70 Season Stress Test
set "MSYS_ROOT="
if defined MSYS2_ROOT if exist "%MSYS2_ROOT%\usr\bin\bash.exe" set "MSYS_ROOT=%MSYS2_ROOT%"
if not defined MSYS_ROOT if exist "C:\msys64\usr\bin\bash.exe" set "MSYS_ROOT=C:\msys64"
if not defined MSYS_ROOT if exist "%ProgramFiles%\msys64\usr\bin\bash.exe" set "MSYS_ROOT=%ProgramFiles%\msys64"
if not defined MSYS_ROOT if exist "%LOCALAPPDATA%\Programs\MSYS2\usr\bin\bash.exe" set "MSYS_ROOT=%LOCALAPPDATA%\Programs\MSYS2"
if not defined MSYS_ROOT if exist "%USERPROFILE%\scoop\apps\msys2\current\usr\bin\bash.exe" set "MSYS_ROOT=%USERPROFILE%\scoop\apps\msys2\current"
if not defined MSYS_ROOT (
 echo ERRORE: MSYS2 non trovato. Usa PREPARA_SWOS.bat per installarlo.
 pause
 exit /b 1
)
set "MSYSTEM=MINGW64"
set "CHERE_INVOKING=1"
for /f "delims=" %%E in ('echo prompt $E^| cmd') do set "ESC=%%E"
echo.
echo %ESC%[96m========================================================%ESC%[0m
echo %ESC%[96m   SWOS EXPANDED - ENGINE TEST SUITE%ESC%[0m
echo %ESC%[96m========================================================%ESC%[0m
echo Test non distruttivo: save/AUTOSAVE.CAR rimane intatto.
echo Campionati sintetici 70 anni, registro coppe OG, save/load,
echo test regressione C, tutti i cataloghi JSON in un mondo e inventario DB.
echo Un unico log: LIVING_WORLD_70_TEST.log
"%MSYS_ROOT%\usr\bin\bash.exe" -lc "bash sorgenti/tests/run_test_engine_suite.sh" > "LIVING_WORLD_70_TEST.log" 2>&1
set "RESULT=%ERRORLEVEL%"
type "LIVING_WORLD_70_TEST.log"
echo.
echo RISULTATI - un controllo per ogni funzione:
call :check_marker MIDWEEK_EVENT_SAVE_RELOAD "Salvataggio durante una giornata"
call :check_marker WORLD_EVENT_CURSOR "Avanzamento del calendario"
call :check_marker GLOBAL_OFFERS_DISCOVERY "Offerte dalle squadre del mondo"
call :check_marker CUP_NATIVE_JOURNAL "Registrazione delle coppe SWOS"
call :check_marker CUP_ARCHIVE_70 "Archivio coppe dopo 70 stagioni"
call :check_marker CUP_HISTORY_70 "Vincitori delle coppe conservati"
call :check_marker CUP_RESULT_IDEMPOTENCE "Nessun risultato di coppa duplicato"
call :check_marker CUP_MULTICOMPETITION_LEGS_REPLAY_70 "Due coppe: ritorni, replay e vincitori su 70 stagioni"
call :check_marker CUP_NEGATIVE_REJECTIONS_70 "Dati coppe incompatibili respinti realmente per 70 stagioni"
call :check_marker STRESS_RESULT "Campionati su 70 stagioni"
call :check_marker PYRAMID_MOVEMENT_NATIVE_RULES "Regole originali di promozione"
call :check_marker PYRAMID_FULL_UP_DOWN "Promozioni e retrocessioni complete"
call :check_marker PYRAMID_IDENTITY_SAVE_LOAD "Squadre conservate nei salvataggi"
call :check_marker PYRAMID_BOUNDARIES "Limiti delle divisioni rispettati"
call :check_marker PYRAMID_STRESS_RESULT "Movimenti tra divisioni su 70 stagioni"
call :check_marker DB_INVENTORY "Inventario del database"
call :check_marker DATABASE_ALL_RECORDS_AUDIT "Tutti i record: squadre, giocatori, coppe e pool"
call :check_marker REAL_JSON_DOMESTIC_70_SEASONS "Campionati JSON reali: 70 stagioni - escluse coppe e nazionali"
call :check_marker GLOBAL_DOMESTIC_CLUB_COVERAGE "Copertura di tutti i club domestici (singleton segnalati)"
call :check_marker REAL_JSON_GLOBAL_70_SEASONS "Tutti i campionati domestici nello stesso mondo, 70 stagioni"
call :check_marker SOUTHERN_JULY_NATIVE_PHASE_RUNTIME "Calcolo luglio per campionati con partite da gennaio (solo test C)"
call :check_marker ENGINE_UNIT_REGRESSIONS "Regressioni del motore"
call :check_marker ENGINE_SUITE "Suite runtime isolata (non Windows OG)"
echo.
echo =========================================================
echo   AUDIT WINDOWS REALE - SWOS.log (se disponibile)
echo =========================================================
echo Verifica prima divergenza OG/LWE, coppe e assunzione.
echo Log indipendente: SWOS_RUNTIME_AUDIT.log
"%MSYS_ROOT%\usr\bin\bash.exe" -lc "bash sorgenti/tests/audit_windows_runtime_log.sh SWOS.log 0.13.38_02" > "SWOS_RUNTIME_AUDIT.log" 2>&1
if errorlevel 1 set "RESULT=1"
type "SWOS_RUNTIME_AUDIT.log"
if not "%RESULT%"=="0" goto test_failed
echo.
echo %ESC%[92m[PASS] Suite isolata e analisi del log Windows, se presente, senza FAIL.%ESC%[0m
echo %ESC%[93m[ATTENZIONE] Senza SWOS.log completo, integrazione Windows ancora NOT_TESTED.%ESC%[0m
echo %ESC%[93m[NOT_TESTED] Coppe OG Windows, F10 Windows, join OG/LWE Windows e spareggi.%ESC%[0m
goto test_done
:test_failed
echo.
echo %ESC%[91m[FAIL] Test NON superato. Invia LIVING_WORLD_70_TEST.log.%ESC%[0m
:test_done
pause
exit /b %RESULT%

:check_marker
rem Exact marker recognition; supports CRLF and LF from the MSYS log.
rem Avoid IF parentheses around an ECHO label that contains "(...)".
set "MARKER_FOUND=0"
for /f "usebackq delims=" %%L in ("LIVING_WORLD_70_TEST.log") do (
 if "%%L"=="%~1=PASS" set "MARKER_FOUND=1"
)
if "%MARKER_FOUND%"=="1" goto marker_ok
echo %ESC%[91mFAIL%ESC%[0m - %~2
set "RESULT=1"
exit /b 0
:marker_ok
echo %ESC%[92mPASS%ESC%[0m - %~2
exit /b 0
