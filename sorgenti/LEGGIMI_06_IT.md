# SWOS EXPANDED 0.6 — BUILD DI PROVA WINDOWS x64

## Installazione
1. CONSERVA la cartella 0.5 e la 0.6 Originale funzionante. NON sovrascriverle.
2. Estrai tutto questo ZIP in una cartella nuova e avvia AVVIA_EXPANDED_06.bat.
3. Prima di provare nuove carriere, fai una copia dei salvataggi in Documenti\SWOS.

## Prova dei comandi (IMPORTANTE)
- Schermata titolo > HALL OF FAME > BACK o ESC: ritorno al titolo senza barre colorate extra.
- Nuova carriera > NATIONALITY > seleziona con frecce + INVIO o clic, oppure CANCEL/ESC.
- Nuova carriera > FAME: apre una finestra con cinque livelli, CANCEL/ESC torna indietro.
- Nuova carriera > AGE (18-70): digita due cifre (es. 25); ESC annulla.
- Carriera > CLUB BUSINESS: e' il menu ORIGINALE, ripristinato per evitare il crash.
  Nella schermata CLUB BUSINESS premi il tasto I per aprire INFRASTRUCTURE.
- INFRASTRUCTURE > scegli una delle 8 strutture > UPGRADE oppure BACK.
  ESC/BACKSPACE devono tornare indietro. I nuovi menu usano font e colori SWOS.

## Novita' 0.6
- Finestre HALL, NATIONALITY, FAME, INFRASTRUCTURE: ricostruzione del menu
  padre e blocco del click trattenuto dopo il ritorno, per ridurre flicker/re-entry.
- HALL OF FAME: titolo, righe azzurre e BACK centrati orizzontalmente.
- NATIONALITY: righe e pulsanti centrati, selezione diretta e annullamento.
- AGE: etichetta AGE (18-70), inserimento numerico come in 0.5.
- FAME: finestra con UNKNOWN, AMATEUR, RISING STAR, ESTABLISHED, LEGEND.
- CLUB BUSINESS: stream dati originale 0.4 ripristinato. Nessuna voce aggiunta
  al suo formato packed; infrastrutture apribili con I e relativo suggerimento.
- Otto strutture con cinque livelli: STADIUM, TRAINING GROUND, TRANSPORT
  (MINIBUS > CLUB BUS > LUXURY COACH > CHARTER PLANE > CLUB AIRCRAFT),
  CLUB OFFICES, FITNESS CENTRE, SCOUTING, MEDICAL CENTRE, YOUTH ACADEMY.
- Le spese di upgrade sono sottratte al saldo originale usato dai trasferimenti.
  Sponsor e diritti TV sono accreditati una sola volta per stagione; gli uffici
  incrementano i ricavi. Il mercato dei giocatori usa lo stesso saldo.
- MEDICAL livello 3+: un passaggio extra di recupero per i giocatori del club
  gestito nelle routine che applicano il normale decadimento degli infortuni.
- SCOUTING: aumenta la probabilita' di arrivo di trialist generati dal motore
  SWOS, con una piccola probabilita' di talento superiore dal livello 3.
  Non crea ancora un database globale di giovani nei campionati minori.
- I livelli delle otto strutture sono serializzati nei salvataggi carriera JSON.
- Restano il titolo EXPANDED pixel-art, video 4K nitido, valuta, salario,
  pensionamento a 85 anni e fix audio x64.
- Le nuove voci sono in INGLESE nel gioco.

## Limiti dichiarati
- Non e' stato possibile eseguire il gioco su Windows in questo ambiente.
  Compilazione e test automatici NON garantiscono i comandi mouse/tastiera.
- La pagina infrastrutture usa temporaneamente il tasto I da CLUB BUSINESS,
  non un pulsante aggiunto allo schermo banca: questa scelta evita di modificare
  nuovamente la struttura packed che nella 0.5 causava un crash.
- Campo di allenamento, trasporto, fitness e vivaio hanno livelli e costi reali,
  ma NON modificano ancora i valori di simulazione della partita. Stadio non
  cambia ancora il rendering delle partite o l'affluenza calcolata da SWOS.
- Scouting agisce sui trialist del club gestito; un sistema completo di
  rigenerazione annuale dei giovani per TUTTI i piccoli campionati resta da fare.
- Compatibilita' dei salvataggi a lungo termine e durata di 67 stagioni NON
  ancora verificata giocando su Windows. Fai una copia dei salvataggi.

## Verifiche eseguite
- LLVM-MinGW cross-build Windows PE32+ x86-64: OK.
- Test C AddressSanitizer + UndefinedBehaviorSanitizer per infrastrutture,
  pagamenti annuali, saldi, costi e cambio club: OK.
- Controlli statici su menu, ritorno da popup, packed bank statement: OK.
- ZIP integrita' CRC: OK.
- Prova reale Windows dei nuovi menu e partite: NON ESEGUITA.

## Licenze
OpenSWOS di Ben Baker e contributori (GPL); sorgenti modificati distribuiti
separatamente. SDL2 secondo licenza originale. Gli asset grafici e audio SWOS
sono quelli forniti per il progetto e restano dei rispettivi titolari.
