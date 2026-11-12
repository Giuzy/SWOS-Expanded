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
echo Test isolato: il tuo save/AUTOSAVE.CAR non viene toccato.
echo Test 70 stagioni + registro eventi coppe OG sintetici + save/load.
echo Log: LIVING_WORLD_70_TEST.log
"%MSYS_ROOT%\usr\bin\bash.exe" -lc "bash sorgenti/tests/run_test_living_world_70.sh" > "LIVING_WORLD_70_TEST.log" 2>&1
set "RESULT=%ERRORLEVEL%"
type "LIVING_WORLD_70_TEST.log"
if not "%RESULT%"=="0" goto test_failed
for %%M in (MIDWEEK_EVENT_SAVE_RELOAD WORLD_EVENT_CURSOR GLOBAL_OFFERS_DISCOVERY CUP_NATIVE_JOURNAL CUP_ARCHIVE_70 STRESS_RESULT) do (
 findstr /X /C:"%%M=PASS" "LIVING_WORLD_70_TEST.log" >nul
 if errorlevel 1 (
  echo [FAIL] Manca %%M=PASS
  set "RESULT=1"
 )
)
if not "%RESULT%"=="0" goto test_failed
echo.
echo [PASS] 70 stagioni, eventi, scoperta paesi, registro coppe e save/load.
goto test_done
:test_failed
echo.
echo [FAIL] Test NON superato. Invia LIVING_WORLD_70_TEST.log.
:test_done
pause
exit /b %RESULT%
