@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title SWOS Expanded - Test reale OG World
set "MSYS_ROOT="
if defined MSYS2_ROOT if exist "%MSYS2_ROOT%\usr\bin\bash.exe" set "MSYS_ROOT=%MSYS2_ROOT%"
if not defined MSYS_ROOT if exist "C:\msys64\usr\bin\bash.exe" set "MSYS_ROOT=C:\msys64"
if not defined MSYS_ROOT if exist "%ProgramFiles%\msys64\usr\bin\bash.exe" set "MSYS_ROOT=%ProgramFiles%\msys64"
if not defined MSYS_ROOT if exist "%LOCALAPPDATA%\Programs\MSYS2\usr\bin\bash.exe" set "MSYS_ROOT=%LOCALAPPDATA%\Programs\MSYS2"
if not defined MSYS_ROOT if exist "%USERPROFILE%\scoop\apps\msys2\current\usr\bin\bash.exe" set "MSYS_ROOT=%USERPROFILE%\scoop\apps\msys2\current"
if not defined MSYS_ROOT (
 echo [INCOMPLETE] MSYS2 non trovato. Percorso usato normalmente: C:\msys64.
 pause
 exit /b 2
)
set "MSYSTEM=MINGW64"
set "CHERE_INVOKING=1"
echo ===========================================================
echo  SWOS EXPANDED - NATIVE OG WORLD TEST
echo ===========================================================
echo Nessuna modifica ai salvataggi. Un solo motore OG.
echo Test C: solo strutture OG, matematica date e database.
echo Gameplay: si legge il SWOS.log creato dal gioco Windows reale.
echo Per produrre il log usa AVVIA_DIAGNOSTICA.bat.
echo Test 70 stagioni OG: F12+8 poi FORWARD TIME in UNEMPLOYED.
echo E' un salto REALE della carriera: usare un nuovo test, non il save abituale.
echo Leggere OG_70_NATIVE_SEASONS e i confronti OG_CUPS / OG_SCORERS.
echo Il test dedicato e' TEST_OG_CUPS_SCORERS.bat.
echo ===========================================================
"%MSYS_ROOT%\usr\bin\bash.exe" -c "bash sorgenti/tests/run_test_engine_suite.sh" > "OG_WORLD_TEST.log" 2>&1
set "STATIC_RESULT=%ERRORLEVEL%"
type "OG_WORLD_TEST.log"
echo.
echo ===========================================================
echo  AUDIT OG WINDOWS - SWOS.log
echo ===========================================================
set "SWOS_DIAGNOSTIC_SESSION=1"
call "%~dp0ANALIZZA_SWOS_LOG.bat" > "OG_WORLD_RUNTIME_DRIVER.log" 2>&1
set "RUNTIME_RESULT=%ERRORLEVEL%"
type "OG_WORLD_RUNTIME_DRIVER.log"
if exist "SWOS_RUNTIME_AUDIT.log" type "SWOS_RUNTIME_AUDIT.log"
echo.
if not "%STATIC_RESULT%"=="0" goto failure
if "%RUNTIME_RESULT%"=="1" goto failure
if not "%RUNTIME_RESULT%"=="0" goto incomplete
echo [PASS] Statico e runtime verificati per i controlli previsti.
echo [NOTA] Le coppe globali e i salvataggi richiedono prova separata.
set "RESULT=0"
goto done
:failure
echo [FAIL] Uno dei test ha rilevato un problema. Invia i log.
set "RESULT=1"
goto done
:incomplete
echo [INCOMPLETE] Test C isolati non certificano il mondo OG Windows.
echo [INCOMPLETE] Coppe OG residenti/save globale ancora da completare.
set "RESULT=2"
:done
echo Log da inviare: OG_WORLD_TEST.log e SWOS_RUNTIME_AUDIT.log.
pause
exit /b %RESULT%
