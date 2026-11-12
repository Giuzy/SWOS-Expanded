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
echo test regressione C e inventario dei dati reali installati.
echo Un unico log: LIVING_WORLD_70_TEST.log
"%MSYS_ROOT%\usr\bin\bash.exe" -lc "bash sorgenti/tests/run_test_engine_suite.sh" > "LIVING_WORLD_70_TEST.log" 2>&1
set "RESULT=%ERRORLEVEL%"
type "LIVING_WORLD_70_TEST.log"
if not "%RESULT%"=="0" goto test_failed
for %%M in (MIDWEEK_EVENT_SAVE_RELOAD WORLD_EVENT_CURSOR GLOBAL_OFFERS_DISCOVERY CUP_NATIVE_JOURNAL CUP_ARCHIVE_70 CUP_HISTORY_70 CUP_RESULT_IDEMPOTENCE STRESS_RESULT PYRAMID_MOVEMENT_NATIVE_RULES PYRAMID_FULL_UP_DOWN PYRAMID_IDENTITY_SAVE_LOAD PYRAMID_BOUNDARIES PYRAMID_STRESS_RESULT DB_INVENTORY ENGINE_UNIT_REGRESSIONS ENGINE_SUITE) do (
 findstr /X /C:"%%M=PASS" "LIVING_WORLD_70_TEST.log" >nul
 if errorlevel 1 (
  echo %ESC%[91m[FAIL] Manca %%M=PASS%ESC%[0m
  set "RESULT=1"
 )
)
if not "%RESULT%"=="0" goto test_failed
echo.
echo %ESC%[92m[PASS] Test automatici campionati, coppe, save/load e inventario DB.%ESC%[0m
echo %ESC%[93m[PENDING] Spareggi OG e coppe mondiali offscreen: NON ancora completati.%ESC%[0m
goto test_done
:test_failed
echo.
echo %ESC%[91m[FAIL] Test NON superato. Invia LIVING_WORLD_70_TEST.log.%ESC%[0m
:test_done
pause
exit /b %RESULT%
