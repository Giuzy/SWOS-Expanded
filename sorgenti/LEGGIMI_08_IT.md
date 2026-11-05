# SWOS Expanded 0.8 — Windows x64

## INSTALLAZIONE
Estrai l'archivio in una CARTELLA NUOVA. Avvia `AVVIA_EXPANDED_08.bat`.
NON sovrascrivere la 0.7: conservala come backup.
Richiede Windows x64. Il pacchetto contiene SDL2.dll, SWOS.exe, grafica e dati.

## NOVITA' 0.8
- Avvio diretto senza schermata animata del loader (si puo' ripristinare con SWOS_SHOW_INTRO=1, senza SWOS_SKIP_INTRO).
- Nel titolo HALL OF FAME prende il posto di SAVE DISK FILING; nascosto il duplicato precedente.
- CLUB OVERVIEW: board e fans (faccine), tifosi stimati, stadio, otto infrastrutture selezionabili con mouse o tastiera.
- Selezionando un'icona si apre la relativa scheda di upgrade, che usa il saldo SWOS condiviso col mercato trasferimenti.
- Bilancio mensile e annuale; settimana finanziaria indicata (non sempre coincide con la settimana del calendario).
- Fame UNKNOWN: schermata iniziale disoccupato con calendario vuoto, si avanza una settimana per volta;
  dopo quattro settimane appare un'offerta da una squadra inglese dell'ultima divisione presente in TEAM.008.
  Accettando, inizia la carriera originale. Fama superiore a UNKNOWN: normale scelta squadra.
- Nota sul menu carriera: UNKNOWN FAME: START UNEMPLOYED.
- CURRENCY riallineata verso la voce PITCH nelle opzioni.

## LIMITI E PRECAUZIONI
- Il numero tifosi e' STIMATO da capienza e risultati; non e' un contatore reale di abbonati.
- Il sentimento tifosi e' un indicatore iniziale basato su vittorie/sconfitte; board usa il punteggio originale.
- I quattro turni disoccupato non simulano campionati e trasferimenti: la carriera viene creata solo all'accettazione.
- I costi TRAVEL rappresentano per ora i costi generali SWOS, non una contabilità separata delle trasferte.
- La settimana finanziaria viene dal contatore originale di gestione costi, non da una data assoluta.
- Non sono stati aggiunti nuovi record di statistiche storiche dei club: dati non disponibili.
- Gli interventi alle finestre non sono ancora stati verificati su un PC Windows reale.
- Conserva i salvataggi originali e fai backup prima di caricare vecchie carriere.

## TEST E PROVENIENZA
- Cross-compilazione MinGW Windows x86-64 eseguita con successo.
- Test automatici C: geometria pixel art 320x256, finanze, otto pulsanti, attesa e offerta club valido.
- Controllo integrita' ZIP e formato PE32+ x86-64 eseguiti.
- Nessuna prova di gameplay o navigazione mouse/tastiera su Windows: da verificare dall'utente.
- Codice derivato da OpenSWOS di Ben Baker e contributori, GPL; sorgenti e patch disponibili a parte.
- Asset originali SWOS: diritti ai rispettivi titolari. SDL2: relativa licenza inclusa.

## PROVA CONSIGLIATA
1. Avvia e verifica che non appaia l'animazione del loader.
2. Apri HALL OF FAME nel vecchio spazio di SAVE DISK FILING, torna con ESC/BACK.
3. Crea allenatore UNKNOWN, premi CHOOSE TEAM: aspetta 4 settimane, accetta offerta.
4. Apri CLUB BUSINESS/CLUB OVERVIEW, seleziona TRAIN, torna indietro, prova UPGRADE.
5. Verifica partita e salvataggio; in caso di crash invia SWOS.log.
