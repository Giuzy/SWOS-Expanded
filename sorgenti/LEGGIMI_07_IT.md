# SWOS EXPANDED 0.7 — ANTEPRIMA CLUB OVERVIEW (Windows x64)

## Avvio
1. Estrai lo ZIP in una cartella NUOVA. Non sovrascrivere la 0.6.
2. Avvia `AVVIA_EXPANDED_07.bat`. Il pacchetto contiene asset e SDL2.dll.
3. Copia i salvataggi prima di provare una carriera modificata.

## Novita'
- NATIONALITY: selezione basata sul riquadro realmente cliccato (non sul numero ordinale del menu). NEXT/PREVIOUS cambiano pagina senza chiudere e riaprire la finestra: elimina il doppio ridisegno responsabile di parte del flicker.
- CLUB BUSINESS apre CLUB OVERVIEW al posto di BANK STATEMENT. La struttura originale del vecchio Bank Statement rimane nel codice per compatibilita' ma non viene mostrata da questo ingresso.
- CLUB OVERVIEW: stadio pixel art disegnato dal motore SWOS, con cinque configurazioni e capienza in base al livello; otto piccoli disegni per infrastrutture (stadio, allenamento, trasporto, uffici, fitness, scouting, medico, vivaio). Trasporto diventa aereo dal livello 4.
- Riepilogo delle finanze: introiti mensili medi da sponsor, TV, incassi, premi; spese mensili medie per stipendi e costi generali/trasferte; saldo totale del club. I valori mensili sono importi annuali divisi per 12, NON un registro di cassa mese per mese.
- UPGRADE apre il menu delle otto infrastrutture gia' presente; BACK ritorna alla carriera.
- In FACILITY UPGRADE i pulsanti UPGRADE e BACK hanno riquadri separati senza sovrapposizione.
- Le scritte aggiunte in gioco sono in INGLESE. Lo stile grafico usa palette, bitmap font e rettangoli a pixel originali.

## Limiti
- Non ci sono statistiche storiche attendibili dei titoli vinti dal club prima dell'arrivo dell'allenatore nel record dati disponibile: appare 'HISTORIC CLUB TITLES: NOT AVAILABLE'. Non vengono inventati numeri.
- Le spese di viaggio sono incluse nelle spese generali originali: il riepilogo le etichetta RUNNING/TRAVEL. Non e' ancora un bilancio dettagliato per ogni trasferta.
- I ricavi annuali degli sponsor e della TV sono quelli calcolati dal modulo Expanded; i ricavi originali di gare e premi sono solo visualizzati e non accreditati una seconda volta.
- La grafica dello stadio nella pagina club NON cambia lo stadio visto durante la partita.
- La compilazione PE32+ x64 e i test unitari sono riusciti. Non e' stato possibile verificare click, navigazione o una carriera reale su Windows: serve il tuo test.

## Test richiesti
A. Nuova carriera > NATIONALITY > ITALY > verifica il codice della nazione, poi cambia pagina con NEXT/PREVIOUS, CANCEL e ESC.
B. Carriera > CLUB BUSINESS > controlla stadio, 8 icone, finanze > UPGRADE > scegli una struttura > BACK > BACK fino alla carriera.
C. Verifica che il saldo in CLUB OVERVIEW sia uguale al saldo usato per comprare giocatori.
D. Prova una partita dopo aver usato i menu.

## Ripristino
Conserva la cartella 0.6 completa. Il pacchetto include anche il precedente SWOS.exe in `backup_06`.

## Crediti
OpenSWOS di Ben Baker e contributori, licenza GPL; asset SWOS dei rispettivi titolari. SDL2 redistribuita secondo la relativa licenza. I sorgenti modificati sono distribuiti in un archivio separato.
