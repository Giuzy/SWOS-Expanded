@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title SWOS Expanded - Preparazione

REM Un eseguibile 0.10 precedente non e considerato valido.
REM Forza sempre la compilazione dei sorgenti aggiornati.
REM Non fidarti dei marker delle versioni precedenti.

set "MSYS_ROOT="
call :cerca_msys
if defined MSYS_ROOT goto :compila

echo.
echo Serve MSYS2 per creare SWOS_Expanded.exe dai sorgenti aggiornati.
echo MSYS2 va installato UNA SOLA VOLTA sul computer.
echo.
where.exe winget >nul 2>nul
if errorlevel 1 goto :manuale
choice /C SN /N /M "Vuoi installare MSYS2 automaticamente con winget? [S/N] "
if errorlevel 2 goto :manuale
winget install --id MSYS2.MSYS2 -e --source winget --accept-package-agreements --accept-source-agreements
if errorlevel 1 (
  echo Installazione non riuscita. Visita https://www.msys2.org
  exit /b 1
)
call :cerca_msys
if defined MSYS_ROOT goto :compila
echo MSYS2 installato ma non trovato. Chiudi e riprova.
echo Se e installato altrove imposta MSYS2_ROOT.
exit /b 1

:manuale
echo Scarica MSYS2 da https://www.msys2.org e installalo.
echo Poi fai doppio clic su AVVIA_SWOS.bat.
exit /b 1

:compila
echo.
echo Uso MSYS2: %MSYS_ROOT%
echo Compilazione automatica. Al primo avvio puo richiedere qualche minuto.
set "MSYSTEM=MINGW64"
set "CHERE_INVOKING=1"
set "SWOS_SRC_WIN=%~dp0sorgenti"
set "SWOS_GAME_WIN=%~dp0"
"%MSYS_ROOT%\usr\bin\bash.exe" -lc "bash -s" < "%~dp0BUILD_SWOS.sh"
if errorlevel 1 (
  echo.
  echo COMPILAZIONE FALLITA: invia COMPILAZIONE_0140.log.
  exit /b 1
)
if not exist "SWOS_Expanded.exe" (
  echo ERRORE: compilazione terminata senza SWOS_Expanded.exe.
  exit /b 1
)
> ".SWOS_0140_COMPILATO" echo SWOS Expanded - compilazione completata
exit /b 0

:cerca_msys
if defined MSYS2_ROOT if exist "%MSYS2_ROOT%\usr\bin\bash.exe" set "MSYS_ROOT=%MSYS2_ROOT%"
if not defined MSYS_ROOT if exist "C:\msys64\usr\bin\bash.exe" set "MSYS_ROOT=C:\msys64"
if not defined MSYS_ROOT if exist "%ProgramFiles%\msys64\usr\bin\bash.exe" set "MSYS_ROOT=%ProgramFiles%\msys64"
if not defined MSYS_ROOT if exist "%LOCALAPPDATA%\Programs\MSYS2\usr\bin\bash.exe" set "MSYS_ROOT=%LOCALAPPDATA%\Programs\MSYS2"
if not defined MSYS_ROOT if exist "%USERPROFILE%\scoop\apps\msys2\current\usr\bin\bash.exe" set "MSYS_ROOT=%USERPROFILE%\scoop\apps\msys2\current"
exit /b 0
