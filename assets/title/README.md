# Title Menu — PNG approvato (build 0.10.52)

Asset richiesto dal renderer: `assets/title/title_background_approved_2026-11-08.png`.

- Immagine approvata: **prima versione senza tabellone** del 2026-11-08.
- Generazione: 1670×941 pixel (**avviso:** 2 px meno dello standard nativo 1672×941).
- Crop senza resize: **1664×936**, PNG RGB lossless.
- SHA-256 del file finale: `34ab54c7548f546ee2cc531be54a5ad366ec4bf35dba4fab2e2010c1e6c4d9b5`.
- Lo sfondo si disegna sotto le reali voci/focus del Title Menu; **F8** alterna PNG/RAW.
- Il PNG non va ricreato dal preview: deve essere trasferito **byte per byte** tramite blob a blocchi ≤ 1 MiB, manifest e Action secondo `asset-transfer/README.md`.
- Se il PNG non è presente, è malformato o non misura 1664×936, il gioco mantiene il vecchio titolo.
- [0.10.53 — IMPLEMENTATO NEL CODICE, DA TESTARE SU WINDOWS] Logo **PNG RGBA indipendente**: `assets/title/swos_expanded_logo_approved_2026-11-08.png`, canonico 1200×440, con ® rimossa, sopra il menu originale solo in modalità PNG.
- [POSIZIONE C APPROVATA] Sullo sfondo 1664×936 il logo è visualizzato a **x=920, y=17, larghezza=620, altezza=227**, senza coprire lo stadio. Coordinate scalate proporzionalmente al renderer; gli hitbox SWOS non si spostano.
- SHA-256 locale del logo: `a194160cdbc9a4d3315b926b8dbee75df2dea9b7aac1d92433929279639f6b96`. Il PNG finale è RGBA lossless, 1200×440; è stato ricavato dall'artwork generato a 2071×759 e poi ridotto, senza dichiararlo 1200×440 nativo. Il vincolo 1672×941 → 1664×936 si applica agli sfondi full-screen, non ai loghi/sprite.
- L'opzione video in-game resta futura e non implementata. Se manca il PNG del logo, il nuovo sfondo continua a caricarsi senza crash; F8 permette il confronto con il vecchio sfondo e disattiva anche il logo nuovo.

**Trasferimento binario in sospeso:** non considerare l'asset archiviato finché il binario non compare su questo ramo e lo SHA-256 non è verificato. La build di codice 0.10.52 può essere sincronizzata indipendentemente.


## Pulsanti HD del Title Menu — sviluppo 0.10.53_01 (2026-11-08)
- [CODICE, DA COMPILARE/TESTARE SU WINDOWS] Cinque sprite PNG RGBA indipendenti in `assets/title/buttons/`: `career.png`, `hall_of_fame.png`, `classic_modes.png`, `options.png`, `exit_game.png`.
- Origine: immagine statica blu con spessore scuro a destra e sotto, scelta dall'utente; ritaglio dei singoli pulsanti senza resize e trasparenza solo all'esterno. **Non utilizzare la GIF ridotta.**
- La prima schermata disegna direttamente questi PNG e il logo e sfondo già approvati; la pressione di un pulsante è animata via SDL2 a ~6 tick, senza spritesheet e con texture in scala nearest. Nessun vecchio pulsante/fondo RAW è visualizzato, F8 non è più usato.
- Ordine: CAREER (mostra CONTINUE separato quando la carriera è attiva), HALL OF FAME, CLASSIC MODES, OPTIONS, EXIT GAME. L'icona fullscreen/windowed è rimandata; Alt+Invio rimane.
- CLASSIC MODES: sottomenu temporaneo senza background illustrato. Preservate CREDITS, LOAD OLD COMPETITION, EDIT TACTICS, EDIT CUSTOM TEAM, HIGHLIGHTS, FRIENDLY, DIY COMPETITION, PRESET COMPETITION, SEASON, BACK; CONTINUE/REPLAY/SAVE per una competizione classica attiva.
- I nuovi PNG vengono consegnati tramite ZIP da estrarre dove si trova `AVVIA_SWOS.bat`; **la presenza del codice nel repository non prova che i binari PNG siano già su GitHub**.
- Verificare input joypad/mouse/tastiera, geometria hitbox, animazione e ritorno dai sottomenù, funzionalità carriera e file di gara, rendering PNG e comportamento development/release su Windows.


## 0.10.53_02 — posizione sul lato destro e profilo del focus
- [MODIFICA CODICE / DA RITESTARE WINDOWS] La colonna di pulsanti viene centrata sotto il logo già approvato, su asse x=1230 nel canvas 1664×936. Coordinate: CAREER x982 y261 496×206; HALL OF FAME x990 y479 480×91; CLASSIC MODES x990 y581 480×91; OPTIONS x990 y683 480×91; EXIT GAME x990 y785 480×91. Queste posizioni sono **provvisorie e spostabili** su richiesta.
- Per il focus giallo, il renderer crea una maschera RGBA separata dalla sagoma alpha di ciascun pulsante (include il bordo tridimensionale, anche destro e inferiore), non un rettangolo sull'hitbox. Disegna l'alone dietro al PNG e nella stessa posizione anche durante la pressione.
- Navigazione UP/DOWN corretta secondo il compass SWOS; 5 tick a 50Hz con movimento verticale breve senza scaling dei singoli PNG. Il sottomenu classico resta provvisorio e privo di sfondo illustrato.
- **Nessuna variazione ai binari degli asset:** rimane valido il ZIP già consegnato per la 0.10.53_01. Test Windows dell'aggiornamento ancora pendente.

- [ARCHIVIO GITHUB VERIFICATO] Un nuovo commit asset `2c39dd3` pubblicato dall'utente su `dev/0.10` contiene già i cinque PNG nei loro percorsi definitivi, con blob hash identici allo ZIP originale. Non serve estrarre di nuovo lo ZIP se la cartella locale è già aggiornata tramite GitKraken.
