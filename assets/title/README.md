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


## 0.10.53_03 — layout ridotto e nuovo spessore focus (attuale)
- [CODICE, WINDOWS DA RITESTARE] L'ordine e gli sprite PNG sono invariati. Nuova colonna tutta centrata sotto il logo (asse x=1230): CAREER x1005 y308 450×187, HALL OF FAME x1014 y507 432×82, CLASSIC MODES x1014 y601 432×82, OPTIONS x1014 y695 432×82, EXIT GAME x1014 y789 432×82. Il logo finisce a y=244; 64 px liberi sopra CAREER. Le posizioni restano perfezionabili.
- Il focus giallo contorna l'alpha completo degli sprite con raggio **5 px** nell'immagine originale (prima 4); nessuna cornice rettangolare per i pulsanti sagomati. Non cambiare o ricreare i PNG già approvati.
- [PULIZIA] Le istruzioni F8/RAW sopra descrivono soltanto le build 0.10.52–0.10.53 storiche, **non sono più valide per il Title moderno**. I due BAT `AVVIA_E_TESTA_*` sono stati rimossi: usare esclusivamente `AVVIA_SWOS.bat` per avviare e testare il gioco.
- `data/expanded/expanded_title_bg.raw` resta temporaneamente una dipendenza tecnica del caricamento interno e del build script; la sua vecchia anteprima PNG è stata rimossa perché superata. Nessun altro asset approvato è stato cancellato. I cinque bottoni canonici, lo sfondo e il logo continuano ad essere su GitHub.


## 0.10.53_04 — CONTINUE CAREER (attuale)
- [APPROVATO] Nuovo pulsante base blu `assets/title/buttons/continue_career.png` con testo statico completo `CONTINUE CAREER`, **senza outline gialla**. Il file approvato è PNG lossless RGBA 8 bit **2065×761 px**, non interlacciato, SHA-256 `19e3858030e140ec5f55e9211816da59b9f12cb86b78a9c44cc87ddcfd474df0`. Non ricrearlo o appiattire il bordo tridimensionale.
- [RUNTIME, WINDOWS DA RITESTARE] CAREER e CONTINUE CAREER sono due sprite distinti. Lo stato carriera seleziona l'uno o l'altro, senza scritte disegnate dal motore sopra il PNG. Il focus giallo è un overlay separato derivato dall'alpha: raggio 6 pixel sorgenti nei cinque bottoni precedenti e 12 nel CONTINUE, per compensare la diversa risoluzione sorgente.
- [LAYOUT 1664×936] CAREER/CONTINUE (1028,350,405×168), HALL OF FAME (1036,529,389×74), CLASSIC MODES (1036,611,389×74), OPTIONS (1036,693,389×74), EXIT GAME (1036,775,389×74). Asse comune x=1230.5, distanza logo→CAREER 106px, interspazi inferiori 8px. La versione CONTINUE mantiene proporzioni naturali con `contain`, senza distorcere l'artwork.
- [CONSEGNA] `SWOS_Expanded_Continue_Career_0.10.53_04_ASSET.zip` da estrarre nella cartella con `AVVIA_SWOS.bat`. **Solo il codice viene committato da ChatGPT** finché il PNG non risulta su GitHub via push GitKraken; ZIP locale non equivale a commit asset.

## Nuova build 0.10.54 — controllo video (attuale)
- Due icone approvate **senza testo e senza contorno giallo baked-in**: `assets/title/buttons/fullscreen_expand.png` (arrows outward, finestra→fullscreen) e `assets/title/buttons/windowed_collapse.png` (arrows inward, fullscreen→finestra). Il renderer SDL2 usa `Platform_GetFullscreenMode()` e mostra una sola icona alla volta; Alt+Invio aggiorna lo stesso stato.
- I PNG nuovi derivano per crop senza resize dalle due immagini approvate, con alpha esterno pulito e margine trasparente sufficiente all'alone runtime. PNG RGBA 8 bit non interlacciati: fullscreen 1044×1009 / SHA-256 `6bc13d1df16ea7331dac769ff8cd02919248b4ea2f39be8162529f3f9c306559`; windowed 838×802 / SHA-256 `4808d10a02ed70a97bea7d76709c8239c96224f0dc131dc7970f3962e1fd2863`.
- Nel canvas 1664×936 l'icona è in alto a sinistra (58,56) misura 92×88, con selezione dal joypad/tastiera tramite SU da CAREER o GIÙ da EXIT GAME. Focus giallo generato a runtime sull'alpha della silhouette e stessa pressione sobria dei pulsanti.
- Nuova posizione/scala degli altri bottoni: CAREER (1048,405,365×151), HALL (1055,570,350×67), CLASSIC (1055,648,350×67), OPTIONS (1055,726,350×67), EXIT (1055,804,350×67), centro x=1230.5. Distanza libera dal logo 161px; nessuna variazione ai file dei cinque pulsanti principali.
- Consegna `SWOS_Expanded_0.10.54_ASSETS.zip` in cui i tre PNG (due icone e CONTINUE CAREER) hanno già i percorsi definitivi. **Lo ZIP non dimostra che i nuovi PNG sono su GitHub**; verificare il push successivo.

## 0.10.54_01 — icona spostata, pulsanti ravvicinati, focus e bounce
- [CODICE / DA TESTARE WINDOWS] L'icona video in alto a sinistra passa a **(34,34,92×88)** nel canvas 1664×936. Le due immagini originali PNG rimangono inalterate, così come il cambio di modalità e Alt+Invio.
- [CODICE / DA TESTARE WINDOWS] Stessa posizione orizzontale e stessa altezza finale del gruppo; nuove coordinate: CAREER (1048,426,365×151), HALL (1055,585,350×67), CLASSIC (1055,658,350×67), OPTIONS (1055,731,350×67), EXIT (1055,804,350×67). Il primo intervallo verticale è 8 px, i successivi 6 px.
- [CODICE / DA TESTARE WINDOWS] Raggio del contorno dinamico dalla sagoma alpha: PNG principali 8 (prima 6), CONTINUE CAREER 16 (prima 12), icona espansione 30 (prima 22), icona riduzione 24 (prima 18). Lo spessore è in **pixel del file PNG sorgente**, non dell'output.
- [CODICE / DA TESTARE WINDOWS] Pressione con rebound verticale breve (1,3,4,2,-1,0 px) senza deformare i PNG; preservata l'illustrazione e il logo. **Stile canonico** per i prossimi pulsanti Expanded: frontali, profondità a destra e sotto, animazione leggera, mai focus giallo nel PNG base generato.

## Logo WORLD OF corretto — sostituzione nello stesso percorso (2026-11-08)
- L'asset canonico `swos_expanded_logo_approved_2026-11-08.png` ora deve mostrare **Sensible / WORLD OF / SOCCER / EXPANDED**, correggendo la O finale di WORLO in D. L'utente ha approvato la versione con modifica locale minima, non una nuova interpretazione del logo.
- Sostituzione del **blob** sotto lo stesso filename GitHub: precedente `e5335fa4507ccca3e101802c544c53a8fff37287`, corretto `7358520ea0b72206988d3bbc16a9e70c0afe633c`. La correzione diretta sul logo in repository cambia solo 1062 pixel nell'area x642..672, y143..180; esterno preservato integralmente. Non rinominare il PNG e non cancellare file storici non collegati.
- 1200×440 PNG RGBA 8bit non interlacciato, senza resize; posizione approvata invariata x920 y17, visualizzato a 620×227. Il caricatore `Expanded_TitleOverlay_Initialize` e il renderer non richiedono modifiche.
- Il pull GitKraken sostituisce il file tracciato sul computer. In alternativa lo ZIP locale con il PNG approvato sovrascrive **lo stesso percorso**, senza un nuovo nome: `SWOS_Expanded_Logo_WORLD_corretto_0.10.54_01.zip`. Nota tecnica: la codifica PNG del file ZIP approvato e del PNG GitHub ricostruito non è identica byte per byte, perché l'asset GitHub preserva direttamente tutti i pixel del suo vecchio blob fuori dalla lettera; il risultato visuale e la singola correzione sono gli stessi.


## Release 0.20.00 — Title Menu quasi finale (8 novembre 2026)

[ASSET APPROVATO E PUSH VERIFICATO] Il file canonico del logo è `assets/title/swos_expanded_logo_approved_2026-11-08.png` PNG RGBA **2048×750**, SHA-256 `8fd34795962704cee3e3242a264d82eeffa4221ab31b1c583bd6df174a9352c6` (Git blob `1326eb21f87e760ef8eac747515b238ffff4b105`). È la versione **caricata dall'utente e testata funzionante**. Il precedente PNG 1200×440 è superato e non è ammesso dal loader di questa release. Posizione x=920, y=17, dimensione visiva 620×227 nel canvas 1664×936; sfondo e altri PNG invariati.

[ASSET SU GITHUB VERIFICATI] Con il push `dev/0.10` `3e18786`, gli sprite necessari `continue_career.png`, `fullscreen_expand.png`, `windowed_collapse.png` sono stati pubblicati nei rispettivi percorsi. Il ramo `release/0.20` è stato aggiornato a partire da quel commit. Tutti e cinque gli altri bottoni e lo sfondo restano invariati.

[CODICE RELEASE] `SWOS_EXPANDED_VERSION=0.20.00`; profilo launcher `CONFIGURA_4K.ps1 -Release`, fullscreen desktop vero (`--fullscreen=desktop`), variabile `SWOS_EXPANDED_DEV_MAXIMIZED` disattivata, navigazione debug disattivata. `COMPILA_SWOS.bat`, `PREPARA_SWOS.bat`, `BUILD_SWOS.sh` aggiornati. Prima della compilazione `BUILD_SWOS.sh` verifica la presenza di tutti i 10 PNG Title e SHA-256 esatto del logo; così eventuali vecchie copie locali vengono rifiutate. `dev/0.10` mantiene le impostazioni development. Non modificare gameplay, controlli, salvataggi e UI preesistenti.

[PULIZIA CONTROLLATA] La directory `assets/title/` contiene una sola copia canonica del logo; il vecchio contenuto è stato sostituito dal push e rimane soltanto nella cronologia Git, non nell'albero corrente. Non rimuovere `data/expanded/expanded_title_bg.raw`, perché ancora caricato da `expanded_economy.c` e richiesto da `BUILD_SWOS.sh`. Non eliminare anteprime e risorse storiche senza prova che non siano utilizzate da codice, script o documentazione.

[DA TESTARE SU WINDOWS] Compilazione MinGW64, eseguibile `SWOS_Expanded.exe`, avvio fullscreen desktop, Alt+Invio, Title PNG/logo, CONTINUE, CLASSIC MODES e competizioni, navigazione completa e ritorno alla partita e ai menu. La presenza di codice e asset su GitHub non equivale a una release Windows collaudata.
