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

**Procedura release (obbligatoria):** preparare il profilo release con `CONFIGURA_4K.ps1 -Release`, non impostare `SWOS_EXPANDED_DEV_MAXIMIZED` e avviare il gioco con `--fullscreen=desktop` (o impostazione equivalente nel pacchetto release). Verificare che ogni launcher/release distribuita parta effettivamente a schermo intero, anche se una precedente sessione development aveva impostato `fullscreen=off`. Conservare il rendering 4K e Alt+Invio.

**Perché:** la finestra massimizzata rende più rapido passare dal gioco agli strumenti di sviluppo senza nascondere l'interfaccia Windows. La release deve invece conservare l'esperienza fullscreen approvata; separare le configurazioni evita che il debug cambi accidentalmente il comportamento finale.

## 8. Progettazione e approvazione delle nuove UI (obbligatoria)

**Prima di disegnare un nuovo menu:** leggere la versione del codice realmente utilizzata e misurare font/glyph, altezza, avanzamento, scale e funzioni di rendering, allineamenti e coordinate; calcolare gli spazi per le etichette più lunghe e gli importi più grandi. Il layout grafico deve essere dimensionato su questi valori, non su font inventati. Se il design cambia le coordinate, prevedere espressamente le modifiche al renderer prima di approvarlo.

**Ordine di consegna:** (1) creare **direttamente un BMP sorgente con testi e numeri dimostrativi**, posizionati e dimensionati come nel gioco; (2) presentarlo per verifica/approvazione visiva; (3) soltanto dopo l'approvazione creare un **secondo BMP pulito**, identico per pannelli ma senza testi/valori dinamici, che saranno resi dal codice. Le eventuali illustrazioni di sfondo restano asset separati.

**Stile:** nuova pixel art coerente con gli sfondi illustrati di SWOS Expanded; i menu OG non sono il riferimento estetico. Non convertire PNG in BMP come fase finale.

**Perché:** evitare mockup impossibili da implementare fedelmente, ritagli di testi e numeri, disallineamenti, correzioni ripetute e doppia visualizzazione dei dati. Il mockup BMP con dati reali-simulati permette di approvare gli ingombri prima del BMP finale senza scritte.

**Vincoli confermati per gli artwork:** costruire ogni nuovo asset UI **da zero** su BMP nativo; usare le versioni precedenti come riferimento visivo, non come base da correggere, salvo richiesta esplicita. UI **senza sfondo incorporato**; sfondo illustrato separato soltanto se richiesto. Le righe UI con font alto 6 px e due linee da 1 px richiedono almeno 10 px totali con margini uguali sopra/sotto. Nella versione pulita mantenere dinamici testi/valori, faccine, stadio, hover e selezione. Conservare i BMP approvati e verificare loader/palette.

Fonte permanente: `AGENTS.md` §4 e §10. Non modificare o sostituire asset esistenti senza richiesta.
