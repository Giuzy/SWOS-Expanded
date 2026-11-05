SWOS EXPANDED 0.10.6 - PIXEL ART, MENU ORIGINALI
================================================

AVVIO WINDOWS x64
1. Estrai tutto lo ZIP in una cartella nuova (non sovrascrivere la 0.10.5).
2. Apri SWOS_Expanded_0_10_6.
3. Fai doppio clic su AVVIA_SWOS.bat.
4. Il primo avvio compila SWOS.exe con MSYS2 MINGW64 (gia' installato sul tuo PC).
   I successivi avvii usano lo stesso EXE, senza ricompilazione.
5. Non devi applicare patch o spostare i file grafici.

SCHERMATE 0.10.6
- TITOLO: allenatore e squadra in pixel art, logo completo senza riquadro,
  niente testa duplicata. Pulsanti compatti in una colonna a destra.
  Tutte le 16 voci/callback originali sono mantenute nel menu sorgente.
  La Hall of Fame appare una sola volta; il suo pulsante reale e' la voce 5.
  Continue/Replay e Save Game compaiono soltanto con una competizione attiva.
  Multiplayer compare soltanto nelle build con NETPLAY abilitato (NETPLAY=0
  in questo BAT, come nella 0.10.5). Non si inventano pulsanti alternativi.
- NUOVA CARRIERA: frame spostato 11 pixel a sinistra, bordi interamente
  visibili. Restano sesso/titolo, nome, cognome, nazionalita', ruolo, eta',
  fama, scelta squadra/disoccupazione, Back e l'azione Load Liverpool.
- CALENDARIO: pannelli centrati, partita sullo sfondo, data nella striscia
  dedicata sopra le partite. Rimangono il calendario nativo con scorrimento,
  i comandi reali e il calendario del disoccupato con Forward Time.
- CLUB BUSINESS: INVARIATO rispetto alla 0.10.5 approvata, con Back in alto,
  stadio dinamico a cinque livelli, sette strutture selezionabili, finanze,
  board, tifosi e miglioramenti.

VERIFICHE
- Controllo sintattico GCC dei file modificati: superato.
- Test del framebuffer con ASan/UBSan: superato.
- Test dei 16 callback originali e visibilita' contestuale: superato.
- Test delle offerte e navigazione della versione precedente: superato.
- Menu originali src/menu_data.c e src/menus.c identici alla 0.10.5.
- Il nuovo eseguibile Windows NON e' precompilato: viene creato dal BAT.
  Il test interattivo completo su Windows non e' stato effettuato qui.

IN CASO DI PROBLEMI
- Se il BAT non compila, invia COMPILAZIONE_0106.log.
- Se il gioco si blocca, invia SWOS.log dalla cartella Documenti\SWOS.
- Se un pulsante non risponde, indica quale e se usavi mouse o tastiera.

NOTA SULLA SIMULAZIONE
Questa release aggiorna interfaccia e hitbox. Non introduce ancora la
simulazione completa delle competizioni durante la disoccupazione o il nuovo
motore economico, che implementeremo successivamente.
