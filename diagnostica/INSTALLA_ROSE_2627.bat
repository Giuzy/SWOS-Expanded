@echo off
setlocal EnableExtensions
rem SWOS Expanded 0.13.81: copy the validated add-only 2026/27 real-squad overlay.
rem Does not touch TEAM/competition data, CAR saves, or Git metadata.
for %%I in ("%~dp0..") do set "ROOT=%%~fI"
set "PACK=%~1"
if not defined PACK set "PACK=%ROOT%\SWOS_Expanded_Rose_Reali_2627_DATI.zip"
if not exist "%PACK%" (
    echo [ERRORE] Pacchetto non trovato: "%PACK%"
    echo Trascina il file ZIP su questo BAT, oppure copialo nella root del progetto.
    exit /b 2
)
if exist "%ROOT%\data_json\worlds\2627\squad_extras" (
    echo [STOP] Esiste gia' la cartella delle rose extra: non sovrascrivo dati.
    echo Confronta manualmente prima di qualsiasi aggiornamento.
    exit /b 3
)
where tar.exe >nul 2>&1
if errorlevel 1 (
    echo [ERRORE] Manca tar.exe incluso in Windows 10/11.
    exit /b 4
)
tar.exe -tf "%PACK%" | findstr /C:"data_json/worlds/2627/squad_extras/ANDORRA.json" >nul
if errorlevel 1 (
    echo [ERRORE] Lo ZIP non e' il pacchetto rose Expanded 2026/27 previsto.
    exit /b 5
)
echo [ROSE 2627] Estrazione additiva in "%ROOT%" ...
tar.exe -xf "%PACK%" -C "%ROOT%"
if errorlevel 1 (
    echo [ERRORE] Estrazione non completata: controllare file e cartella.
    exit /b 6
)
if not exist "%ROOT%\data_json\worlds\2627\squad_extras\ANDORRA.json" (
    echo [ERRORE] Nessun file di controllo dopo l'estrazione.
    exit /b 7
)
echo [OK] File reali 2026/27 estratti. Prima del test, controlla in GitKraken.
echo [ATTENZIONE] Non prova il gameplay o la compatibilita' del salvataggio.
exit /b 0
