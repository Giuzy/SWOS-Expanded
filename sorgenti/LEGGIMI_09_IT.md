# SWOS Expanded 0.9 — Windows x64

## INSTALLAZIONE
Estrai lo ZIP in una CARTELLA NUOVA, senza sovrascrivere la 0.8.
Avvia `AVVIA_EXPANDED_09.bat`. Include SWOS.exe, SDL2.dll e tutti i dati.
Tieni un backup delle carriere prima di provare una versione nuova.

## CORREZIONI
- NATIONALITY: riga e pagina vengono convertite immediatamente in un indice assoluto.
  La scelta viene applicata dopo il ritorno al menu allenatore, non durante
  lo smontaggio della finestra. Un tasto tenuto premuto non seleziona la
  prima voce della pagina successiva automaticamente.
- La nota UNKNOWN e' spostata in alto, sotto CAREER. Il testo e' visibile
  soltanto con fama UNKNOWN. Spiegazione: BOARD: FINANCES / FANS: FAME AND RESULTS.
- Con fama UNKNOWN il pulsante diventa START GAME; con le altre fame resta CHOOSE TEAM.
- CLUB OVERVIEW: il disegno in pixel art viene renderizzato DOPO gli sfondi
  dei pulsanti, per evitare le due barre blu sopra la grafica.
- Ogni riquadro apre direttamente la propria scheda UPGRADE; nascosto il
  pulsante generale. BACK resta al centro in basso.

## CARRIERA DA DISOCCUPATO
- La schermata usa la geometria a tre colonne del calendario/contest originale,
  con area centrale FIXTURES / CALENDAR e nessuna partita programmata.
- Il motore originale del calendario NON viene inizializzato finche' non
  accetti un club. Questa e' una rappresentazione coerente del calendario,
  non ancora il vero calendario che simula tutte le leghe durante l'attesa.
- FORWARD TIME avanza esattamente una settimana per pressione.
- TRANSFERS, CLUB BUSINESS e SQUAD sono disabilitati (grigi e non cliccabili).
- Una nuova offerta arriva casualmente dopo 1-5 settimane; se rifiutata,
  un'altra arriva dopo 1-5 settimane. La selezione usa club reali di basse
  divisioni dai file TEAM disponibili, non squadre inventate.
- VIEW CLUB OFFER apre una finestra con nome club e paese, ACCEPT, DECLINE,
  BACK. BACK torna all'attesa senza prendere decisioni.

## CLUB: BOARD E FANS
- Board e tifosi hanno due valori separati da 0 a 100, salvati nel formato JSON
  della carriera. I tifosi hanno un conteggio persistente, non piu' solo
  una stima ricavata ogni volta dallo stadio.
- Valori iniziali: board 50 + 5 per livello fama; fan mood 40 + 9 per fama.
  Il numero iniziale tifosi dipende dalla capienza e dalla fan mood.
- Dopo una vittoria: board +6, fan mood +7, tifosi +125.
  Dopo una sconfitta: board -7, fan mood -8, tifosi -90.
  Pareggio: board invariato (prima dell'effetto finanziario), fan mood +1,
  tifosi +15. Board: aggiustamento di -5/-2/+2 per saldo finanziario.
  A inizio stagione ulteriore aggiustamento del board in base al saldo.
- Questi sono threshold PROVVISORI; li perfezioneremo con il gameplay.
- Le finanze esistenti e il saldo del mercato trasferimenti non vengono duplicati.

## VERIFICHE
- Cross-compilazione Windows PE32+ x86-64: OK.
- Test C: tutte le 68 righe di nationality, attesa casuale con rifiuto e
  accettazione, geometria grafica 320x256, infrastrutture, board/fans: OK.
- ZIP CRC: OK.
- Nessuna esecuzione interattiva del gioco su Windows in questo ambiente.
  La stabilita' dei tasti e della navigazione reale deve essere confermata
  sul tuo PC. Se c'e' un problema, invia `SWOS.log` e descrivi i tasti premuti.

## PROVENIENZA
OpenSWOS di Ben Baker e contributori (GPL), grafica e dati SWOS originali
con diritti dei rispettivi titolari, SDL2 con relativa licenza.
La patch e i sorgenti modificati sono nel pacchetto sorgenti separato.
