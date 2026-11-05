# SWOS Expanded 0.10.1 — correzioni sorgenti (NON eseguibile)

Questa e' una revisione del codice di SWOS Expanded 0.10. Non include un nuovo SWOS.exe: il file Windows deve essere ricompilato prima del test in gioco. **Non sostituire il tuo eseguibile 0.10 con uno vecchio spacciato per 0.10.1.**

## Correzioni applicate

1. **Crash dopo ACCEPT:** il selettore delle offerte ora esclude i club *fuori lega*. Nel formato SWOS, `league=4` puo' significare "fuori dal campionato" e non "quinta divisione". Prima la ricerca sceglieva la divisione numericamente piu' alta senza controllare se fosse giocabile; questo poteva inviare un club non valido a `InitCareer()`.
2. **Crash VIEW SQUAD / VIEW LEAGUE:** rosa e lista di divisione vengono copiate in uno snapshot quando l'offerta e' creata. Aprire le schermate di anteprima e premere ACCEPT non richiama piu' `LoadTeamFile()` dentro un callback del menu: si evita di mutare il buffer globale delle squadre mentre il menu e' attivo.
3. **Accettazione sicura:** l'offerta viene controllata sia sullo snapshot sia nel file squadra, quest'ultima verifica solo dopo che i menu sono stati chiusi. Un'offerta non valida viene rifiutata prima di avviare la carriera.
4. **Data senza sovrapposizioni:** nel calendario normale e in quello del disoccupato, la data viene disegnata alla coordinata y=148, nell'intervallo libero sotto il pannello partite (che termina a y=147) e sopra i pulsanti (che iniziano a y=156). Rimane invariato il pannello originale.
5. **Diagnostica:** due messaggi `[expanded11] InitCareer: starting/completed` delimitano l'inizializzazione del club per facilitare l'analisi di un eventuale ulteriore crash.

## Test eseguiti

- Test C `tests/test11_offer.c` con AddressSanitizer e UndefinedBehaviorSanitizer: PASS. Verifica selezione del club giocabile, rifiuto e seconda offerta, anteprima rosa/lega, nessun caricamento file dentro i menu, accettazione e coordinata della data.
- Lo stesso test sul sorgente 0.10 originale si ferma all'asserzione "nessun caricamento file dentro il menu", mostrando la regressione.
- Controllo sintattico C dei file modificati: PASS (avvisi gia' presenti in altri moduli).

## Da verificare su Windows

La compilazione Windows e il test grafico interattivo **non sono stati eseguiti** in questo ambiente, che non dispone del compilatore MinGW e degli header SDL2. La correzione delle due cause plausibili non e' ancora una garanzia di assenza di crash. Servono un test dell'eseguibile ricompilato e, se necessario, `SWOS.log`.

### Compilazione in MSYS2 MINGW64

Con MSYS2 MINGW64 installato, installa toolchain e SDL2 (se non gia' presenti):

```
pacman -S --needed mingw-w64-x86_64-gcc mingw-w64-x86_64-SDL2 mingw-w64-x86_64-pkgconf make
```

Dalla cartella con `Makefile`, compila:

```
make -j4 NETPLAY=0
```

Il risultato atteso e' `bin/SWOS.exe`. Copialo nella cartella del gioco 0.10 insieme ai dati e `SDL2.dll`, **in una nuova cartella di test**. Usa `AVVIA_EXPANDED_010.bat` per le risorse e i parametri gia' forniti, senza sovrascrivere i salvataggi.

## Non ancora implementato

Questa revisione e' una correzione mirata, non implementa ancora la simulazione di tutti i campionati durante la disoccupazione o l'ingresso a stagione gia' iniziata. Non corregge eventuali altri problemi grafici non riprodotti in questa revisione.
