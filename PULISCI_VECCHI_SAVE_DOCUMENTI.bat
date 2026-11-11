@echo off
setlocal EnableExtensions DisableDelayedExpansion
cd /d "%~dp0"
set "LEGACY=%USERPROFILE%\Documents\SWOS\save"
echo.
echo SWOS Expanded - pulizia MANUALE dei vecchi save in Documenti
echo Nuova cartella usata dal gioco: "%~dp0save"
echo Vecchia cartella condivisa con OpenSWOS originale: "%LEGACY%"
echo.
if not exist "%LEGACY%\" (
  echo Nessuna cartella legacy trovata. Nessun file eliminato.
  pause
  exit /b 0
)
echo ATTENZIONE: puo' contenere salvataggi di altre installazioni.
echo PRIMA verifica che AUTOSAVE.CAR nella cartella del gioco si carichi!
echo Verranno cancellati solo: .CAR .SEA .PRE .DIY .TAC .HIL .EDT .SRP .RPL .TEAM
echo.
set "ANSWER="
set /p "ANSWER=Per confermare scrivi esattamente ELIMINA: "
if /I not "%ANSWER%"=="ELIMINA" (
  echo Annullato, nessun file cancellato.
  pause
  exit /b 0
)
for %%E in (CAR SEA PRE DIY TAC HIL EDT SRP RPL TEAM) do (
  if exist "%LEGACY%\*.%%E" del /q "%LEGACY%\*.%%E"
)
echo Pulizia eseguita; nessuna cartella esterna modificata.
echo SWOS.ini e SWOS.log precedenti restano intatti.
pause
exit /b 0
