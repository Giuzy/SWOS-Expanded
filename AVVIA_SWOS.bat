@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title SWOS Expanded - Compilazione locale e avvio
REM Preserve legacy artifacts when migrating the root; never touch career saves.
if not exist "%~dp0diagnostica\log" mkdir "%~dp0diagnostica\log" >nul 2>&1
for %%M in (.SWOS_0140_COMPILATO .SWOS_0200_COMPILATO) do (
 if exist "%~dp0%%M" if not exist "%~dp0diagnostica\%%M" move "%~dp0%%M" "%~dp0diagnostica\%%M" >nul
)
if exist "%~dp0SWOS.ini.prima_di_Expanded_4K.bak" if not exist "%~dp0diagnostica\SWOS.ini.prima_di_Expanded_4K.bak" move "%~dp0SWOS.ini.prima_di_Expanded_4K.bak" "%~dp0diagnostica\SWOS.ini.prima_di_Expanded_4K.bak" >nul
if exist "%~dp0COMPILAZIONE_0140.log" if not exist "%~dp0diagnostica\log\COMPILAZIONE_0140.log" move "%~dp0COMPILAZIONE_0140.log" "%~dp0diagnostica\log\COMPILAZIONE_0140.log" >nul
REM Other user-local BAT files are kept, not deleted or overwritten.
for %%B in ("%~dp0*.bat") do (
 if /I not "%%~nxB"=="AVVIA_SWOS.bat" if /I not "%%~nxB"=="AVVIA_DIAGNOSTICA.bat" (
  if not exist "%~dp0diagnostica\%%~nxB" move "%%~fB" "%~dp0diagnostica\%%~nxB" >nul
 )
)
if exist "%~dp0SWOS.log" if not exist "%~dp0diagnostica\log\SWOS_PRE_MIGRAZIONE.log" move "%~dp0SWOS.log" "%~dp0diagnostica\log\SWOS_PRE_MIGRAZIONE.log" >nul

REM La diagnostica puo separare COMPILA e AVVIA senza ricompilare al CONTINUE.
REM Fuori dalla diagnostica il percorso originale compila e avvia normalmente.
if "%SWOS_DIAGNOSTIC_RUN_ONLY%"=="1" goto :prerequisiti
call "%~dp0diagnostica\PREPARA_SWOS.bat"
if errorlevel 1 goto :errore

:prerequisiti

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

REM La compilazione validata termina qui: NON avviare il gioco.
if "%SWOS_DIAGNOSTIC_BUILD_ONLY%"=="1" (
  echo [DIAGNOSTICA] BUILD_ONLY completato: EXE e prerequisiti presenti.
  exit /b 0
)

if exist "%~dp0diagnostica\CONFIGURA_4K.ps1" powershell -NoProfile -ExecutionPolicy Bypass -File "%~dp0diagnostica\CONFIGURA_4K.ps1"
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
REM Solo il BAT diagnostico puo richiedere l'attesa del processo.
REM Il percorso normale resta identico: compilazione e avvio scollegato.
if "%SWOS_DIAGNOSTIC_SESSION%"=="1" (
  echo [DIAGNOSTICA] Avvio dopo compilazione, attesa chiusura per raccogliere i log.
  start "" /WAIT /D "%~dp0" "%~dp0SWOS_Expanded.exe" --fullscreen=off
  if errorlevel 1 goto :errore_avvio
  if not errorlevel 0 goto :errore_avvio
  exit /b 0
)
REM Avvio indipendente: il gioco continua mentre questa finestra CMD si chiude.
REM Non usare /B: terrebbe il processo collegato alla console di compilazione.
start "" /D "%~dp0" "%~dp0SWOS_Expanded.exe" --fullscreen=off
if errorlevel 1 goto :errore_avvio
exit /b 0

:errore_avvio
echo.
echo ERRORE: avvio o esecuzione SWOS_Expanded.exe terminati in modo anomalo.
echo Controlla SWOS.log nella cartella della sessione diagnostica.
if not "%SWOS_DIAGNOSTIC_SESSION%"=="1" pause
exit /b 1

:errore
echo.
echo Il gioco non e stato avviato.
if defined SWOS_DIAGNOSTIC_DIR (
 echo Log reale: %SWOS_DIAGNOSTIC_DIR%\AVVIO_COMPILAZIONE.log
 echo Log GCC: %SWOS_DIAGNOSTIC_DIR%\COMPILAZIONE_0140.log
) else (
 echo Se la compilazione e fallita, invia diagnostica\log\COMPILAZIONE_0140.log.
)
if not "%SWOS_DIAGNOSTIC_SESSION%"=="1" pause
exit /b 1
