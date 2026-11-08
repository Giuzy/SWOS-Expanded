# SWOS Expanded — development procedures and rationale

Status: approved working rules. Primary integration branch: `dev/0.10`.

## 1. Local working copy, GitHub for synchronization

**Procedure:** At the start of a development session, obtain the latest complete project snapshot from the integration branch and prepare a local workspace containing source code, assets, build configuration and relevant documentation. Inspect the actual repository structure instead of assuming paths. Work on the local snapshot when possible; use GitHub to synchronize, review and commit approved changes. Before editing, confirm that the needed sources and assets are present. Do not describe a workspace as complete until verified.

**Why:** Reading related files locally is faster, supports project-wide searches and coordinated changes, and reduces repeated remote API calls and mistakes caused by missing context. GitHub remains the durable, shared source of truth rather than the editor for every file.

**Between chats:** A chat runtime's local files are not guaranteed to survive. A new session must restore or recreate the workspace from the latest committed state. Never assume uncommitted local work persists. Record important design decisions and outstanding work in versioned documentation. At the end of a session, commit approved work where feasible and report any uncommitted changes.

## 2. Approved image assets: automatic upload

**Procedure:** Once the user explicitly approves a newly created graphical asset, transfer it to the repository without requesting another upload or confirmation. Preserve the approved visual result and use the appropriate asset directory. For binary transfers, use the documented chunk/manifest/reassembly workflow in `asset-transfer/README.md`; verify the final file and its SHA-256 before reporting success.

**Why:** This removes manual downloading and re-uploading by the user, avoids losing the exact approved version, and works around the observed single-transfer size ceiling.

**Observed limit:** 3,145,472 bytes per upload in our tests (not a universal GitHub limit). Use chunks of at most 1,048,576 bytes.

## 3. Asset approval versus implementation

**Procedure:** Approval of artwork authorizes storing that asset. Modify code or replace an existing in-game asset only when the user requests implementation or replacement. Keep previously approved UI elements unchanged unless specifically requested.

**Why:** Approval of an image is not automatically approval of unrelated code or layout changes; separating the steps prevents regressions and preserves agreed visual designs.

## 4. Fixes, verification and commits

**Procedure:** For a requested bug fix, identify the actual active source files and input mappings, make a targeted change, inspect the diff, run available syntax/build checks and relevant tests, then commit on the agreed branch. Report precisely what was tested, what could not be tested, and the commit identifier. Do not claim a fix or successful build merely because a commit exists.

**Why:** A repository edit does not prove runtime behavior; focused changes and explicit checks reduce regressions and make troubleshooting reproducible.

**Current pending issue:** In the Business screen, selecting an expense item with the joypad and pressing X should open its details popup immediately, without requiring cursor movement or an additional keypress. Preserve the approved popup artwork. This remains unverified and must not be reported as fixed until the code and behavior have been checked.

## 5. Temporary transfer tests and cleanup

**Procedure:** After validating a transfer, remove test BMPs and their test manifests, while retaining the reusable workflow and documentation. Do not remove production assets or files needed for recovery.

**Why:** Keeps the repository clean without losing the automated transfer capability.

## 6. Versioning and handoff

**Procedure:** Work on `dev/0.10` unless instructed otherwise. Preserve the project's established version and interim-commit conventions. Keep project notes and procedures current, including the rationale for significant workflow decisions. When changing chats, restore the committed baseline and consult these notes before continuing.

**Why:** The next session can reconstruct both the technical state and the reasons behind decisions, without relying on temporary chat storage or recollection.

## 7. Modalità video development e release

**Procedura development:** il launcher `AVVIA_SWOS.bat` imposta `SWOS_EXPANDED_DEV_MAXIMIZED=1`, usa `--fullscreen=off` e applica `CONFIGURA_4K.ps1` senza `-Release`. SDL2 massimizza una finestra Windows **con bordi e barra del titolo**, mantenendo taskbar/Alt+Tab. Non impostare una finestra borderless fullscreen: non è la stessa cosa.

**Persistenza della massimizzazione durante le partite:** il cambio di vista DOS/menu tramite `MatchViewEnter`/`MatchViewLeave` chiama `Platform_ViewChanged`, che aggiorna la scala logica ma non deve chiamare `SDL_SetWindowSize` se `SDL_WINDOW_MAXIMIZED` è attivo. Le finestre normali continuano a ridimensionarsi come prima; le release fullscreen non sono interessate. Test Windows: avviare via BAT, entrare e uscire da una partita, verificare dimensioni/bordi/taskbar e Alt+Tab sia all'ingresso che all'uscita.

**Procedura release (obbligatoria):** preparare il profilo release con `CONFIGURA_4K.ps1 -Release`, non impostare `SWOS_EXPANDED_DEV_MAXIMIZED` e avviare il gioco con `--fullscreen=desktop` (o impostazione equivalente nel pacchetto release). Verificare che ogni launcher/release distribuita parta effettivamente a schermo intero, anche se una precedente sessione development aveva impostato `fullscreen=off`. Conservare il rendering 4K e Alt+Invio.

**Perché:** la finestra massimizzata rende più rapido passare dal gioco agli strumenti di sviluppo senza nascondere l'interfaccia Windows. La release deve invece conservare l'esperienza fullscreen approvata; separare le configurazioni evita che il debug cambi accidentalmente il comportamento finale.

## 8. Progettazione e approvazione delle nuove UI PNG

La regola del BMP nativo obbligatorio per i nuovi asset frontend è **annullata** dal commit 0.10.52. Conservare intatti i BMP, RAW e PNG storici già approvati.

1. Leggere il menu attivo, `sorgenti/docs/FONTS.md`, i font e le funzioni effettivi, incluse spaziature, coordinate, dimensioni e hitbox; misurare le stringhe tramite `GetTextWidth` / varianti e dati massimi.
2. Prima creare il mockup **PNG lossless** con dati dimostrativi riproducibili nel motore per approvazione; poi creare il PNG UI pulito RGBA trasparente, con gli elementi dinamici gestiti dal renderer.
3. Conservare ogni artwork illustrato separato da logo, UI e dati. Per gli sfondi 1672×941 generazione e crop 1664×936 senza resize salvo richiesta.
4. Il nuovo renderer SDL2 carica 8-bit PNG RGB/RGBA non interlacciati con miniz; il titolo è modernizzato prima degli altri menu, senza modificare partite e salvataggi.

**Workflow preferito asset (8 novembre):** l'assistente consegna un **ZIP con i nuovi file già nei loro percorsi definitivi**, da estrarre nella cartella con `AVVIA_SWOS.bat`. L'utente pubblica periodicamente con GitKraken; il ZIP non è un commit GitHub. `ASSETS_INBOX/` resta alternativa autorizzata con archivio e commit separati dopo push segnalato. Gli eventuali vecchi blocchi/manifest descrivono un metodo storico, non imposto come procedura predefinita.

## 9. Frontend moderno PNG e prova Title Menu (build 0.10.52)

I nuovi sfondi sono PNG lossless nativi; per gli sfondi a schermo intero riferimento 1672×941 di generazione e 1664×936 dopo il solo crop 16:9. Se una delle due dimensioni differisce occorre avvertire l'utente prima dell'integrazione. Le componenti UI PNG sono separate dallo sfondo; il logo canonico sarà un asset condiviso, i testi dinamici restano del renderer.

Il Title Menu in 0.10.52 carica `assets/title/title_background_approved_2026-11-08.png` tramite il decoder PNG in `sorgenti/src/expanded_png.c` (miniz esistente, nessuna nuova DLL), estrae i soli pixel non identici allo sfondo RAW legacy e lascia le reali callback/tasti/focus nello stesso menu 8-bit. **F8** alterna PNG/originale esclusivamente sul Title Menu. In assenza dell'asset, o con PNG non conforme a 1664×936, usa senza crash il vecchio sfondo. Non modificare la partita.

Dopo sincronizzazione GitKraken, eseguire `AVVIA_SWOS.bat` per compilare/avviare come già previsto. Verificare con joypad, tastiera e mouse i pulsanti, il ciclo TITLE→sottomenu→TITLE, F8, build number, windowed maximized e fullscreen desktop, ed eventuali residui del vecchio sfondo. La compilazione/test Windows può essere confermata solo dall'utente. Per il PNG usare il trasferimento a blocchi documentato in `asset-transfer/README.md` con verifica SHA-256 del file ricostruito; non dichiarare asset su GitHub finché l'Action non ha scritto e verificato il binario.


## 10. Logo Title Menu condiviso — build 0.10.53
Il file `assets/title/swos_expanded_logo_approved_2026-11-08.png` è il logo canonico, non deve essere incorporato negli sfondi. Dim. originale 1200×440 RGBA trasparente; in Title Menu la posizione approvata C corrisponde a (920,17) con dimensioni 620×227 sullo sfondo di coordinate 1664×936. Lo scaling segue le dimensioni del renderer senza toccare gli hitbox; i dati dinamici e i controlli rimangono originali. F8 passa dall'insieme PNG+logo alla schermata storica, e viceversa.

Per test Windows: sincronizzare `dev/0.10`, copiare i due PNG esatti sotto `assets/title/` (non sostituire il RAW), avviare `AVVIA_SWOS.bat`, verificare logo, F8 e ogni pulsante con tastiera/joypad/mouse. Non considerare i nuovi PNG su GitHub finché il relativo trasferimento binario, a blocchi di massimo 1 MiB e SHA-256, non risulti verificato. Conservare asset storici e salvataggi invariati.


## 11. Title Menu 0.10.53_01: due livelli e pressione SDL2
Il Title non disegna più grafica dei pulsanti SWOS 8-bit e non offre più F8. Mantiene background e logo approvati e carica cinque PNG RGBA indipendenti sotto `assets/title/buttons/`. Il primo livello presenta CAREER, HALL OF FAME, CLASSIC MODES, OPTIONS, EXIT GAME. Il secondo livello temporaneo, senza immagine di sfondo, presenta le modalità originali e CONTINUE/REPLAY/SAVE condizionali. La pressione è un movimento di circa sei tick via renderer SDL2, senza GIF.

Asset in ZIP da estrarre accanto a `AVVIA_SWOS.bat`; compilazione e test Windows ancora necessari. Non affermare che gli asset sono su GitHub prima di un push verificato.


## 12. Pulizia launcher/anteprime superati (0.10.53_03)
Per il gioco corrente il launcher unico è `AVVIA_SWOS.bat`; `COMPILA_SWOS.bat`, `PREPARA_SWOS.bat` e `BUILD_SWOS.sh` restano script attivi. Le note più sopra su **F8** e sui vecchi BAT di test riguardano esclusivamente le build storiche 0.10.52 e 0.10.53; NON applicarle al Title moderno.

I file temporanei di quei test e il PNG di anteprima del Title RAW sono stati rimossi dal repository. Sono stati eliminati anche tre manifest di asset già ricomposti e verificati, senza rimuovere gli asset definitivi. Il vecchio sistema Action/manifest è disponibile soltanto come metodo secondario quando lo ZIP o il deposito `ASSETS_INBOX/` non bastano.

**Windows:** dopo pull con GitKraken, i file tracciati cancellati vengono normalmente rimossi dal working tree; i file personali o altre copie non tracciate possono restare. `.gitignore` previene l'aggiunta accidentale degli specifici file di test superati. Se occorre cancellare copie residue, farlo solo per i percorsi specifici presenti nel registro `PROJECT_STATE.md`, mai con una cancellazione generica di cartelle di gioco. Conservare tutte le risorse RAW/BMP approvate ancora richieste dal codice e i salvataggi locali.


## 13. Pulsanti base PNG vs stato selezionato — Title 0.10.53_04
Per i pulsanti UI del nuovo frontend conservare file PNG RGBA **privi di outline gialla**. Il bordo illustrato proprio del bottone (azzurro/3D) rimane. Il layer SDL2 del focus giallo deriva dinamicamente dall'alpha completo dello sprite e non deve essere disegnato nel PNG di partenza. Se due PNG di uno stesso pulsante hanno risoluzioni sorgenti differenti, compensare il raggio del focus per mantenere lo stesso spessore visivo (CAREER raggio 6 px, CONTINUE CAREER raggio 12 px).

Per la carriera attiva il Title usa uno sprite separato `assets/title/buttons/continue_career.png` con testo già presente nel file, non una scritta sovrapposta via renderer. Lo stato seleziona lo sprite; usare contenimento proporzionale in presenza di rapporti geometrici differenti e condividere le hitbox con CAREER.

Consegna approvata in ZIP con percorso finale, estrazione accanto ad `AVVIA_SWOS.bat`, successivo push GitKraken e verifica GitHub. Non indicare l'asset come remoto finché non è verificato.
