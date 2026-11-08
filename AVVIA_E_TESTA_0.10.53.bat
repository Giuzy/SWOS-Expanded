@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title SWOS Expanded 0.10.53 - Title HD con logo C

echo.
echo SWOS Expanded 0.10.53 - PNG HD e logo in posizione C.
echo Prima aggiorna il ramo dev/0.10 da GitKraken al commit 48c53dd.
echo.
if not exist "AVVIA_SWOS.bat" (
    echo ERRORE: estrai lo ZIP nella cartella principale SWOS-Expanded.
    pause
    exit /b 1
)
if not exist "assets\title\title_background_approved_2026-11-08.png" (
    echo ERRORE: sfondo PNG non trovato in assets\title\.
    pause
    exit /b 1
)
if not exist "assets\title\swos_expanded_logo_approved_2026-11-08.png" (
    echo ERRORE: logo PNG non trovato in assets\title\.
    pause
    exit /b 1
)
echo F8 nel Title Menu: nuova immagine + logo / titolo storico.
echo.
call "%~dp0AVVIA_SWOS.bat"
exit /b %ERRORLEVEL%
