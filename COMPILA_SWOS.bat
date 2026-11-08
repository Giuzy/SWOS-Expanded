@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title SWOS Expanded 0.20.00 - Compilazione locale
call "%~dp0PREPARA_SWOS.bat"
if errorlevel 1 (
  echo.
  echo Compilazione fallita. Controlla COMPILAZIONE_0200.log.
  pause
  exit /b 1
)
echo.
echo Compilazione completata: SWOS_Expanded.exe pronto.
exit /b 0
