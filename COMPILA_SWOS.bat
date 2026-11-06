@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title SWOS Expanded 0.10.40 - Compilazione locale
call "%~dp0PREPARA_SWOS.bat"
if errorlevel 1 (
  echo.
  echo Compilazione fallita. Controlla COMPILAZIONE_0140.log.
  pause
  exit /b 1
)
echo.
echo Compilazione completata: SWOS_Expanded.exe pronto.
pause
exit /b 0
