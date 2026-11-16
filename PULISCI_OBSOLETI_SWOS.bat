@echo off
setlocal EnableExtensions DisableDelayedExpansion
cd /d "%~dp0"
title SWOS Expanded - Pulizia file obsoleti (stadi esclusi)
echo.
echo SWOS EXPANDED - PULIZIA CONSERVATIVA
echo Scopo: solo file esattamente elencati e gia marcati _OBSOLETO.
echo Stadi, salvataggi, screenshot, RAW OG, grafs, world e CLASSIC sono esclusi.
echo Nessuna cancellazione senza digitare ELIMINA. Il BAT NON usa wildcard.
echo.
echo ANTEPRIMA:
set "ACTION=PREVIEW"
call :Files
echo.
set /p "CONFIRM=Per eliminare SOLO questi file scrivi ELIMINA: "
if /I not "%CONFIRM%"=="ELIMINA" (
 echo Nessun file eliminato.
 exit /b 0
)
set "ACTION=APPLY"
echo.
echo ESECUZIONE:
call :Files
echo.
echo Terminato. Se GitKraken ha gia applicato il commit, i file potrebbero
echo essere gia assenti. Non toccare i file ancora richiesti dalle build.
exit /b 0

:Files
call :One "assets\title\buttons\career_OBSOLETO.png"
call :One "assets\title\buttons\classic_modes_OBSOLETO.png"
call :One "assets\title\buttons\continue_career_OBSOLETO.png"
call :One "assets\title\buttons\exit_game_OBSOLETO.png"
call :One "assets\title\buttons\hall_of_fame_OBSOLETO.png"
call :One "assets\title\buttons\options_OBSOLETO.png"
call :One "data\expanded\expanded_loading_bg_OBSOLETO.bmp"
call :One "data\expanded\expanded_loading_bg_OBSOLETO.png"
call :One "data\expanded\expanded_loading_caption_OBSOLETO.bmp"
call :One "data\expanded\expanded_loading_title_OBSOLETO.bmp"
exit /b 0

:One
if not exist "%~1" (
 echo ASSENTE: %~1
 exit /b 0
)
if /I "%ACTION%"=="PREVIEW" (
 echo CANDIDATO: %~1
 exit /b 0
)
del /F /Q "%~1" >nul 2>&1
if exist "%~1" (
 echo NON ELIMINATO: %~1
) else (
 echo ELIMINATO: %~1
)
exit /b 0
