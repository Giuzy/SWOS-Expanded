@echo off
setlocal
cd /d "%~dp0"
if not exist "AVVIA_SWOS.bat" (
  echo ERRORE: estrai lo ZIP nella radice di SWOS Expanded accanto a AVVIA_SWOS.bat.
  pause
  exit /b 1
)
if not exist "assets\title\title_background_approved_2026-11-08.png" (
  echo ERRORE: manca il PNG nuovo in assets\title.
  pause
  exit /b 1
)
echo SWOS Expanded 0.10.52 -- F8 confronta lo sfondo nuovo e quello originale.
call "AVVIA_SWOS.bat"
