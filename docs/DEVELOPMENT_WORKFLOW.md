# SWOS Expanded — development procedures and rationale

Status: approved working rules. Primary integration branch: `dev/0.10`.

## 1. Workflow ChatGPT/GitHub/GitKraken (ripristinato 2026-11-09)

Il metodo attivo usa la chat normale e il repository GitHub, sincronizzato dall’utente con GitKraken. Il precedente metodo Codex locale e` storico. Prima di modificare verificare ramo, HEAD, stato locale, documentazione e dipendenze. Preservare modifiche non committate anche tra sessioni; non ricreare automaticamente la copia di lavoro dal remoto.

Si possono ripetere modifiche, compilazioni e test locali senza commit intermedi. Commit e push richiedono ciascuno autorizzazione esplicita dell'utente. Registrare risultati reali e lavoro non committato nel passaggio di sessione. Rispettare i limiti della richiesta corrente: installazioni/aggiornamenti e avvio del gioco richiedono autorizzazione.

### Applicazione della procedura permanente

La sequenza obbligatoria e le autorizzazioni sono definite in [AGENTS.md](../AGENTS.md), sezione 0; non duplicare qui l'elenco delle regole.

- **Preparazione:** leggere il diff esistente e identificare i file necessari. Usare controlli Git in sola lettura; non sincronizzare, ripristinare o cambiare ramo automaticamente. GitKraken resta lo strumento di sincronizzazione scelto dall'utente.
- **Contraddizioni:** riportare le fonti e le opzioni concrete, attendere la decisione sulla parte controversa e proseguire soltanto sulle parti indipendenti già autorizzate.
- **Alternative:** presentare un confronto proporzionato alla richiesta, distinguendo fatti verificati e ipotesi. Indicare benefici, impatto, compatibilità Windows, costi, tempi e rischi della soluzione raccomandata; attenderne l'approvazione esplicita prima di adottarla.
- **UI:** raccogliere misure e mappatura input/callback dal codice reale secondo la sezione 8; mostrare il PNG con dati riproducibili prima dell'asset pulito. Distinguere approvazione visiva, autorizzazione all'integrazione e approvazione di eventuali cambi tecnici.
- **Sviluppo e verifica:** eseguire le iterazioni locali autorizzate senza commit intermedi; scegliere controlli pertinenti e verificare che la toolchain sia già disponibile. Nessuna installazione o aggiornamento automatico. Un limite specifico della richiesta, come sola lettura o divieto di avvio, prevale sulle verifiche normalmente previste.
- **Consegna:** elencare file modificati/rimossi, verifiche svolte e relativo esito, prove ancora necessarie e problemi aperti. Non dichiarare collaudi non eseguiti; lasciare le modifiche locali in attesa di eventuali richieste esplicite di commit e push.

## 2. Asset approvati nei percorsi definitivi

Salvare direttamente i nuovi PNG approvati nelle cartelle definitive locali e verificarne formato, dimensioni, trasparenza e contenuto. Conservare gli asset approvati esistenti fino a richiesta di sostituzione.

ZIP, ASSETS_INBOX e trasferimenti GitHub non sono più il workflow ordinario. Sono revocate le autorizzazioni automatiche a committare gli asset. ASSETS_INBOX, verificata contenente soltanto `.gitkeep`, viene rimossa. La documentazione di trasferimento resta solo per memoria storica; il relativo job GitHub è disabilitato.

**Motivo:** lavorare sulla copia Windows reale consente iterazioni e verifiche locali senza trasferimenti né commit intermedi obbligatori.

## 3. Asset approval versus implementation

**Procedure:** Approval of artwork authorizes storing that asset. Modify code or replace an existing in-game asset only when the user requests implementation or replacement. Keep previously approved UI elements unchanged unless specifically requested.

**Why:** Approval of an image is not automatically approval of unrelated code or layout changes; separating the steps prevents regressions and preserves agreed visual designs.

## 4. Fixes, verification and commits

**Procedure:** For a requested bug fix, identify the actual active source files and input mappings, make a targeted change, inspect the diff, run available syntax/build checks and relevant tests, leave the changes local until the user explicitly authorizes a commit; push requires explicit authorization too. Report precisely what was tested, what could not be tested, and any uncommitted changes; report a commit identifier only if a commit was explicitly authorized and created. Do not claim a fix or successful build merely because a commit exists.

**Why:** A repository edit does not prove runtime behavior; focused changes and explicit checks reduce regressions and make troubleshooting reproducible.

**Current pending issue:** In the Business screen, selecting an expense item with the joypad and pressing X should open its details popup immediately, without requiring cursor movement or an additional keypress. Preserve the approved popup artwork. This remains unverified and must not be reported as fixed until the code and behavior have been checked.

## 5. Conservazione e pulizia

Conservare asset approvati e file necessari al progetto. Le vecchie procedure di trasferimento sono documentazione storica, non istruzioni per caricare, committare o cancellare automaticamente file.

## 6. Versioni e continuità

Verificare la versione in `sorgenti/include/build_info.h`. Modifiche locali e test ripetuti non richiedono commit intermedi né incrementi automatici della versione. Applicare le convenzioni di numerazione di AGENTS.md quando viene autorizzato un commit di codice o richiesta una nuova build. Questa manutenzione documentale e degli script conserva `0.10.54_09`.

I nomi `COMPILAZIONE_0140.log` e `.SWOS_0140_COMPILATO` sono mantenuti per compatibilità; non indicano la versione corrente. BUILD_SWOS.sh verifica la toolchain già installata senza installare o aggiornare pacchetti. PREPARA_SWOS.bat conserva il percorso di installazione iniziale MSYS2 con conferma esplicita: non autorizzarlo se la richiesta vieta installazioni.

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

**Workflow corrente (9 novembre):** salvare i PNG approvati direttamente nei percorsi definitivi locali; commit e push solo su esplicita autorizzazione.

> **Cronologia tecnica delle build (sezioni 9–23):** consegne ZIP, trasferimenti, push e limitazioni di accesso locale descritti sotto sono storici, superati dal workflow del 9 novembre. Non costituiscono autorizzazioni operative; conservare i dettagli tecnici pertinenti e distinguere test storici e correnti.

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

## 14. Icona video Title e build 0.10.54
La regola di assenza di contorno giallo si applica **al momento della generazione di ogni nuovo PNG base della UI**: nessun focus baked-in; non ritoccare retroattivamente gli asset approvati. Il focus dinamico SDL2 è un effetto separato che segue la silhouette alpha.

Il Title usa un sesto punto selezionabile in alto a sinistra, con immagini fullscreen_expand/windowed_collapse a stati mutuamente esclusivi e navigazione ciclica: SU da CAREER o GIÙ da EXIT seleziona il controllo. Premendo il tasto si richiama lo stesso Platform_ToggleFullscreen() usato da Alt+Invio. La finestra development massimizzata resta una finestra decorata anche dopo il ritorno da fullscreen.

Il nuovo codice si trova nel branch `dev/0.10`; due PNG dell'icona e CONTINUE CAREER sono consegnati nel ZIP versione 0.10.54. Verificare questi file dopo il push GitKraken, poi testare su Windows tastiera/joypad/mouse, stato icona in entrambi i versi, Alt+Invio, finestra massimizzata durante menu→partita→menu, release fullscreen desktop e scaling del Title.

## 15. Canonical button style — 0.10.54_01 (8 novembre 2026)
Quando si **genera** un nuovo pulsante PNG del frontend Expanded, rispettare lo stile della serie Title approvata: pixel art blu/ciano vista frontalmente, con profondità scura a destra e sotto, alpha trasparente attorno alla sagoma e nessun bordo giallo di focus/hover nel PNG base. Il renderer produce la selezione separatamente dalla silhouette e anima la pressione mediante traslazione (breve bounce sul rilascio) senza riprocessare i pixel artistici. Non applicare modifiche automatiche agli asset storici approvati.

Per il Title 0.10.54_01: icona video (34,34,92×88); CAREER (1048,426,365×151), HALL (1055,585,350×67), CLASSIC (1055,658,350×67), OPTIONS (1055,731,350×67), EXIT (1055,804,350×67) nel canvas 1664×936. La grafica originale non è stata rigenerata. Testare su Windows focus, bounce, giù/su da EXIT/CAREER e transizione fullscreen↔finestra massimizzata.


## 16. Ripristino finestra dalla modalità fullscreen — 0.10.54_02
Nel ramo development `AVVIA_SWOS.bat` imposta `SWOS_EXPANDED_DEV_MAXIMIZED=1`: `Platform_Init` forza l'avvio in finestra SDL2 decorata anche se un profilo precedente ha memorizzato fullscreen in `SWOS.ini`. Il flag non va impostato nei launcher release.
All'uscita dal fullscreen, una sessione inizialmente fullscreen usa per impostazione di ritorno la finestra decorata massimizzata, invece della vecchia geometria piccola `_screen_width × s_win_scale_current`. Se la sessione proveniva da una finestra normale non massimizzata, conserva le regole di ripristino della dimensione utente. Verificare su Windows icona Title, Alt+Invio, taskbar/titolo e menu→partita→menu.


## 17. Primo frame SDL desktop — 0.10.54_03
Il renderer desktop crea la finestra con `SDL_WINDOW_HIDDEN` e chiama `SDL_ShowWindow` una volta dopo il primo `SDL_RenderPresent` completo; nessun cambiamento al canvas WASM. Scopo: non mostrare il client bianco non inizializzato prima dello splash. Testare su Windows la visibilità dello splash, la massimizzazione vera e le transizioni menu/partita e fullscreen; l'esito non è garantito finché non è verificato su Windows.


## 18. Visibilità iniziale dello splash — 0.10.54_04
La prima chiamata a `SDL_RenderPresent` può precedere `Platform_ExpandedSplashSet(0)`, creando un fotogramma nero anche con la finestra inizialmente nascosta. Mostrare la finestra solo dopo aver composto e presentato il primo frame dello splash Expanded; se l'asset è indisponibile mostrare un fallback, e consentire l'apertura diretta del Title. Non modificare i layer approvati né inventare percentuali di caricamento. Controllare realmente su Windows il percorso normale e il percorso senza splash.


## 19. Primo present dello splash (0.10.54_05)
Per il frontend desktop, `SDL_ShowWindow` viene chiamato subito **prima** del primo `SDL_RenderPresent` contenente lo splash Expanded: i frame di bootstrap privi di splash restano nascosti. Si preservano i percorsi fallback, la versione WASM e la finestra massimizzata development. Il risultato contro il breve flash nero resta da verificare su Windows.


## 20. Title glow e splash anticipato — 0.10.54_06
Il caricamento dei PNG Title ad alta risoluzione, incluso il calcolo degli outline alpha, è spostato dopo la prima presentazione dello splash Expanded 0%, ora eseguita sul renderer SDL già inizializzato in `Platform_Init`. `mainC` mantiene la progressione del caricamento reale. La luminosità di `CONTINUE CAREER` è una sovrapposizione cyan SDL2 separata e permanente soltanto con carriera attiva, senza modificare asset, contorno giallo o bounce. Verificare il nero prima dello splash, il messaggio di tempo `[boot] Title PNG preparation after splash`, e qualità del focus su Windows.


## 21. Colore CONTINUE CAREER — 0.10.54_07
Tolto il precedente glow azzurro esterno e la sovrapposizione additiva. Si usa una semplice modulazione SDL2 `RGB(255,120,215)` sul PNG canonico mentre `CONTINUE CAREER` è presente; immediato ripristino dello stato colore dopo il disegno. Il focus giallo continua indipendente e lo sprite PNG non è stato editato.


## 22. CONTINUE CAREER rosso — 0.10.54_08
Per colori di pulsanti con iscrizioni bianche, NON applicare SDL_TextureColorMod all'intera texture: altera anche le lettere. La tinta rossa del solo `CONTINUE CAREER` è ottenuta a runtime tramite conversione selettiva dei pixel blu nel buffer RGBA decodificato, lasciando invariati bianchi, nero, alpha e profondità. Nessuna modifica al PNG canonico su disco. Verificare l'effetto sul rendering Windows; il focus dinamico resta separato.


## 23. CONTINUE CAREER verde — 0.10.54_09
Il colore rosso della build precedente è sostituito dal verde mediante riconversione selettiva degli stessi pixel blu/indaco del PNG canonico già decodificato. Non applicare tint globale: il testo resta bianco. Non cambiare asset né geometria; mantenere focus e animazione precedenti.

### Consegna attuale
La chat normale puo' creare commit GitHub su richiesta dell'utente, sul ramo verificato. GitKraken sincronizza e l'utente compila/testa su Windows. Gli ZIP con asset/file e le istruzioni in chat sono **separati**. Le istruzioni precedenti sul workflow locale Codex non sono piu' prescrittive.

---

## SWOS_DOC_SYNC_2026_11_10 — Documentazione e pulizia grafica (10/11/2026)

- Prima di una modifica leggere `AGENTS.md`, `PROJECT_STATE.md`, `docs/GAME_DESIGN.md` e i documenti tecnici pertinenti. `GAME_DESIGN` specifica funzionalità approvate e idee; `PROJECT_STATE` descrive cosa è nel gioco; `IMPROVEMENTS` registra modifiche effettivamente introdotte. Quando cambia una decisione progettuale, allineare le guide nel **prossimo commit pertinente autorizzato**, non in commit documentali autonomi.
- **Grafica finale:** TITLE, NEW CAREER e LOADING. Gli altri menu e asset pre-switch sono **visivamente obsoleti**, ma rimangono funzionanti e presenti finché non saranno riprogettati; il nome storico `approved` non autorizza a riusarli come layout definitivo.
- **Prossimo BAT di pulizia:** prima creare un inventario dei caricamenti e riferimenti in C, JSON, script, menu e fallback, classificandoli `IN_USE`, `REPLACED`, `ORPHAN_CANDIDATE`. Produrre un elenco esatto dei file per revisione; solo gli orfani verificati possono essere cancellati dal BAT. Nessuna pulizia per pattern generale `*.raw`, `*_approved*`, directory intere o per confronto di data. Preservare `.gitkeep` e asset/shared runtime. Controllare la compilazione e il ciclo completo di menu; ricordare l'utente al prossimo BAT di pulizia programmato. Distinguere gli asset obsoleti nel **design** dagli asset inutilizzati nel **runtime**.
- **Tempo:** uno scheduler di date/eventi, con risultati e salvataggi coerenti. Test d'integrazione con i club attuali, prima di allargare il database. WEEK visibile nella maggioranza dei menu e NEXT EVENT in CALENDAR/UNEMPLOYED; posizione grafica ancora da concordare; nessuna modifica all'orologio partita a 50 Hz.
- **Consegna ZIP:** contenere esclusivamente file nuovi o realmente modificati nei relativi percorsi. L'utente estrae nella cartella contenente `AVVIA_SWOS.bat`, sincronizza su GitHub via GitKraken quando decide. Uno ZIP non equivale a commit o push; verificare HEAD/branch alla sessione seguente. Script di aggiornamento documentale e BAT di rimozione asset sono **operazioni diverse**.


## Workflow vigente dal 10 novembre 2026 — sostituisce le indicazioni ZIP precedenti

- **Codice C, script, BAT, JSON e documenti:** lavorare sul repository GitHub `Giuzy/SWOS-Expanded`, sul branch verificato, con **commit/push solo su richiesta esplicita «committa»**. L'utente sincronizza via GitKraken. Non rinominare automaticamente il ramo `dev/0.10` quando cambia la versione.
- **ZIP soltanto per nuovi asset grafici** da estrarre con i percorsi definitivi. Non predisporre pacchetti ZIP per semplici modifiche di codice/documentazione.
- **Versionamento:** primo commit serie nuova `0.13.01`; ogni commit successivo di codice incrementa .01 senza suffissi, nuovo milestone +0.1 soltanto su richiesta. Se il formato esatto della successiva milestone non è chiaro, confrontarsi con l'utente prima di cambiarlo.
- **Fase 1:** testare `bash sorgenti/tests/run_test_expanded_world_time.sh` (o aprire la shell nella directory `sorgenti` e chiamare `bash tests/run_test_expanded_world_time.sh`). La compilazione completa e il test gameplay su Windows rimangono distinti dai test isolati.
