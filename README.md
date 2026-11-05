# SWOS Expanded

Progetto amatoriale in sviluppo, basato su OpenSWOS. Prima di redistribuire codice e risorse, verificare le rispettive licenze e i diritti.

## Sviluppo Windows — compilazione locale

Il repository **non compila piu su GitHub Actions** e **non pubblica SWOS.exe nei commit**. Il compilatore MSYS2/MinGW64 e SDL2 sono richiesti sul PC di sviluppo.

1. Apri il repository in GitKraken e seleziona `release/0.10.40`.
2. Esegui **Pull** per ricevere le modifiche.
3. Chiudi il gioco prima della compilazione.
4. Fai doppio clic su **`AVVIA_SWOS.bat`**: esegue `PREPARA_SWOS.bat`, che usa `BUILD_SWOS.sh` per compilare localmente, quindi avvia il gioco.
5. In alternativa, esegui **`COMPILA_SWOS.bat`** per compilare senza avviare il gioco; successivamente puoi eseguire `AVVIA_SWOS.bat`.

La compilazione usa `make -j4 NETPLAY=0` nella cartella `sorgenti` e produce `SWOS.exe` nella radice locale. `make` ricompila soltanto i file modificati. Il log e `COMPILAZIONE_0140.log`.

Il file `SDL2.dll` e i dati di gioco devono essere presenti localmente. I binari generati (`SWOS.exe`, oggetti e log) sono esclusi da Git; `SDL2.dll` resta una dipendenza runtime e non viene eliminata.

### Nota su Git

`git pull` aggiorna i sorgenti; **non scarica l'eseguibile**. Compila con il BAT dopo ogni Pull. La cronologia precedente puo contenere eseguibili gia pubblicati: rimuoverli dal ramo corrente non li cancella dalla cronologia Git.
