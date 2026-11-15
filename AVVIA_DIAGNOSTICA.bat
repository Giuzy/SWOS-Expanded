@echo off
setlocal EnableExtensions
cd /d "%~dp0"
title SWOS Expanded - Avvio diagnostico completo

REM Un solo BAT: usa ESATTAMENTE l'avvio normale, che compila sempre tramite
REM PREPARA_SWOS.bat; attende poi la chiusura del gioco e raccoglie i log.
REM Il salvataggio save\ non viene mai modificato dal raccoglitore.
set "SWOS_DIAGNOSTIC_SESSION=1"
set "SWOS_DIAGNOSTIC_CAPTURE=1"
set "EXPECTED="
for /f "tokens=3" %%V in ('findstr /B /C:"#define SWOS_EXPANDED_VERSION" "sorgenti\include\build_info.h"') do set "EXPECTED=%%~V"
if not defined EXPECTED set "EXPECTED=UNKNOWN"
set "SESSION="
for /L %%N in (1,1,999) do if not defined SESSION if not exist "DIAGNOSTICA_SESSIONE_%%N" set "SESSION=DIAGNOSTICA_SESSIONE_%%N"
if not defined SESSION (
 echo ERRORE: nessuno slot per una nuova sessione diagnostica.
 pause
 exit /b 2
)
mkdir "%SESSION%"
if errorlevel 1 (
 echo ERRORE: impossibile creare la cartella della sessione.
 pause
 exit /b 2
)
(
 echo SWOS EXPANDED - DIAGNOSTICA WINDOWS
 echo Build attesa: %EXPECTED%
 echo Avvio: %DATE% %TIME%
 echo Raccolta: F12 OG resident leagues, F12+C OG cups, F10 OG data.
 echo Eventi del mondo OG: START, TICK, view, bind, cup definitions.
 echo Il BAT avvia AVVIA_SWOS.bat, che esegue PREPARA_SWOS.bat.
 echo NESSUN TEST NON ESEGUITO PUO ESSERE DICHIARATO PASS.
 echo Il gioco non viene modificato dal raccoglitore.
) > "%SESSION%\README_DIAGNOSTICA.txt"

echo ==========================================================
echo  SWOS EXPANDED - SESSIONE DIAGNOSTICA
echo ==========================================================
echo  Cartella di uscita: %SESSION%
echo  AVVIA_SWOS.bat compilera e avviera il gioco.
echo  Durante il test apri F12, F12+C e F10 per acquisire dati OG.
echo  Da DEBUG WORLD usa BACK nei paesi: un livello per volta.
echo  F12+8 e poi FORWARD TIME: 70 anni OG, uno per frame menu.
echo  TEST_OG_70 e TEST_OG_CUPS_SCORERS leggono questo SWOS.log.
echo  ATTENZIONE: questo comando cambia realmente la data della carriera.
echo  Dopo la chiusura verra analizzato SWOS.log automaticamente.
echo ==========================================================
echo.
call "%~dp0AVVIA_SWOS.bat" > "%SESSION%\AVVIO_COMPILAZIONE.log" 2>&1
set "GAME_EXIT=%ERRORLEVEL%"
echo Avvio e sessione terminati, codice=%GAME_EXIT%.
echo Dettagli in %SESSION%\AVVIO_COMPILAZIONE.log

if exist "SWOS.log" (
 copy /Y "SWOS.log" "%SESSION%\SWOS.log" >nul
) else (
 echo ATTENZIONE: SWOS.log mancante. > "%SESSION%\SWOS_LOG_MANCANTE.txt"
)
call "%~dp0ANALIZZA_SWOS_LOG.bat" > "%SESSION%\ANALISI_AVVIO.log" 2>&1
set "AUDIT_EXIT=%ERRORLEVEL%"
if exist "SWOS_RUNTIME_AUDIT.log" copy /Y "SWOS_RUNTIME_AUDIT.log" "%SESSION%\SWOS_RUNTIME_AUDIT.log" >nul
(
 echo Avvio/gioco exit: %GAME_EXIT%
 echo Analizzatore exit: %AUDIT_EXIT%
 echo Il codice di avvio non certifica il mondo OG.
 echo Coppe residenti e save globale OG restano incompleti.
 echo Un test statico non certifica la simulazione Windows.
 echo Consiglio: invia SOLO il file ZIP della sessione, se creato.
) >> "%SESSION%\README_DIAGNOSTICA.txt"

REM Tar di Windows 10/11 usa automaticamente ZIP con il suffisso .zip.
REM Se non disponibile, la cartella resta intatta e contiene i medesimi file.
if exist "%SystemRoot%\System32\tar.exe" (
 "%SystemRoot%\System32\tar.exe" -a -cf "%SESSION%.zip" "%SESSION%" >nul 2>&1
)
echo.
if exist "%SESSION%.zip" (
 echo PACCHETTO PRONTO: %SESSION%.zip
) else (
 echo Non ho potuto creare lo ZIP. Invia la cartella %SESSION%.
)
echo Il test reale Windows e completo solo quando il rapporto lo dimostra.
echo.
pause
if not "%GAME_EXIT%"=="0" exit /b 1
exit /b %AUDIT_EXIT%
