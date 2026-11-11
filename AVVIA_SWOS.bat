@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title SWOS Expanded - Compilazione locale e avvio

REM Compila sempre i sorgenti locali prima di avviare il gioco.
REM make ricompila solo i file modificati.
call "%~dp0PREPARA_SWOS.bat"
if errorlevel 1 goto :errore

if not exist "SWOS_Expanded.exe" (
  echo ERRORE: SWOS_Expanded.exe non e stato generato.
  goto :errore
)
if not exist "SDL2.dll" (
  echo ERRORE: SDL2.dll mancante.
  goto :errore
)
if not exist "data\TEAM.008" (
  echo ERRORE: dati delle squadre mancanti.
  goto :errore
)

if exist "CONFIGURA_4K.ps1" powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0CONFIGURA_4K.ps1"
set "SWOS_SKIP_INTRO=1"
set "SWOS_DATA_JSON=0"
set "SWOS_MENU_JSON=0"
set "SWOS_TACTICS_JSON=0"
set "SWOS_MENU_NAV_LOG=1"
REM In sviluppo usa una vera finestra massimizzata, con barra titolo e taskbar.
REM In release NON impostare SWOS_EXPANDED_DEV_MAXIMIZED e avvia fullscreen.
set "SWOS_EXPANDED_DEV_MAXIMIZED=1"
echo.
echo === SWOS Expanded - avvio locale ===
REM Avvio indipendente: il gioco continua mentre questa finestra CMD si chiude.
REM Non usare /B: terrebbe il processo collegato alla console di compilazione.
start "" /D "%~dp0" "%~dp0SWOS_Expanded.exe" --fullscreen=off
if errorlevel 1 goto :errore_avvio
exit /b 0

:errore_avvio
echo.
echo ERRORE: impossibile avviare SWOS_Expanded.exe.
echo Controlla SWOS.log nella stessa cartella di AVVIA_SWOS.bat.
pause
exit /b 1

:errore
echo.
echo Il gioco non e stato avviato.
echo Se la compilazione e fallita, invia COMPILAZIONE_0140.log.
pause
exit /b 1
