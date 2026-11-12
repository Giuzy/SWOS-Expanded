# SWOS EXPANDED — IMPROVEMENTS

**Development history, implemented changes and technical progress**

**Project:** SWOS Expanded  
**Original game:** Sensible World of Soccer (SWOS)  
**Technical foundation:** OpenSWOS — C / SDL2  
**Target platform:** Windows x64  
**Document updated:** 6 November 2026  
**Latest known development branch:** `dev/0.10` (previous branch retained temporarily)

---

## 1. Purpose of this document

This document records the development history of SWOS Expanded, from the first experiments to the latest documented builds. It explains what was added to original SWOS, which systems were changed, what changed in each build, which bugs were addressed and which experiments were replaced or removed.

**This is a development history, not a list of future features.** Future ideas and incomplete systems belong in `PROJECT_STATE.md`; agent instructions belong in `AGENTS.md`.

**Terminology:** **Documented** means described in a build package or historical record; **User-tested** means tested by the user on Windows; **Superseded** means replaced in a later build; **Unverified** means the current implementation or behavior has not been conclusively checked. Documented does not mean fully tested.

## 2. Main improvements over original SWOS

### 2.1 Career management

Historical Expanded builds introduced or extended:
- Managerial career beginning without a club.
- Manager creation, age, nationality, fame and currency.
- Annual salary offers and negotiations.
- Job offers while unemployed, with squad and competition previews.
- Separate Board and Fans satisfaction, dismissal and return to unemployment.
- Management Record and Hall of Fame integration.
- Removal of the fixed 20-season career limit in the Expanded design.

These systems have appeared in historical builds. Long careers, dismissal thresholds and retirement still require verification against current code.

### 2.2 Club management and finances

- Dedicated Club Business screen and five stadium levels.
- Seven upgradeable facility categories.
- Sponsor and television revenue, upgrade costs, supporter counts.
- WAGES, STAFF, TRAVEL and later ADMINISTRATION and contractual manager salary.
- Weekly and seasonal financial views, monthly and annual expense projections.
- Board/Fans indicators and facility hover panels accessible via mouse and joypad.

The approved no-debt policy is a design decision; full current implementation is unverified.

### 2.3 Squad and team performance

- Player strength displayed as up to five stars.
- Detailed statistics popup, mouse/controller hover, improved selection and swapping.
- TEAM FORM and TEAM MORALE, dynamic expressions and status colors.
- Temporary match-performance modifier based on Form/Morale.
- Indicative bonus/malus values in player information.

The experimental Expanded individual condition from 0.10.20 was superseded by team-based Form and Morale.

### 2.4 Interface and controls

- Pixel-art career and management screens, original SWOS font.
- Joypad navigation, fullscreen widescreen Expanded menus.
- Removal of original framebuffer borders and graphical residues.
- Dynamic text separated from illustrated backgrounds.
- Match-statistics and exit-confirmation overlays.
- Dedicated loading screen with real bootstrap progress.

Original SWOS match gameplay remains the project's foundation.

### 0.10.51_07 — keep development window maximized across matches (7 November 2026)

**User-reported:** a maximized decorated Windows development window shrank to SWOS's original scaled size on entering/leaving a match. `Platform_ViewChanged` was explicitly calling `SDL_SetWindowSize` on every DOS/menu logical-view change, without excluding maximized windows. The SDL path now skips physical resizing when `SDL_WINDOW_MAXIMIZED` is set, while preserving logical scaling, normal window resizing and release fullscreen. Also formalized the already-approved reusable A/B/X controller legend rule in `AGENTS.md`. **Verification:** static source review; Windows compilation and actual match transitions still require user testing.

### 0.10.51_06 — fix Windows compilation of Business integration (7 November 2026)

[USER-REPORTED BUILD FAILURE IN 0.10.51_05] GCC 16.2.0 MinGW64 rejected the new `Expanded_ClubOverviewReapplyLayout` because `_currentMenu` was referenced before its extern declaration. The 0.10.51_06 change moves the extern declaration before the first SDL use and removes the extraneous `_currentMenu` access in the non-SDL legacy fallback. No asset, navigation, gameplay, or save-format changes. Not yet retested on Windows; warnings about packed members are unrelated.

### 0.10.51_05 — Business Screen BMP integration (7 November 2026)

**Code updated, not yet Windows-tested.** The approved native BMP Business Screen and approved A/B/X legend are loaded directly without converting source art. Runtime redraw preserves actual stadium level, financial figures, date, supporters and Board/Fans faces. The facility menu keeps its established upgrade callbacks and cost logic while matching the new alphabetical two-column art layout. WEEK/YEAR use separate focusable controls; X opens WAGES/TRAVEL/MAINTENANCE popups immediately, and B closes open details before leaving Business. Mouse geometry and controller focus follow the new coordinates; the legacy art remains a fallback. Build-time preflight checks presence of both approved BMP files. Build ID increments to `0.10.51_05`.

**Verification:** source correspondence reviewed and native BMP loader tested locally against both approved files; Windows build, SDL2 frame presentation, gamepad/mouse interaction and save/load not tested in this session.

## 3. Documented build history

The table includes every version identifiable in recovered project sources. Missing information is marked rather than invented.

| Version | Main documented changes |
|---|---|
| Initial experiments | Investigation of Amiga SWOS and modern PC adaptation; later OpenSWOS adopted as foundation. |
| Early OpenSWOS builds | Windows/SDL2 integration, compilation, startup and runtime troubleshooting. |
| 0.1–0.7 | Initial startup and integration experiments; insufficient reliable per-version detail. |
| 0.8 | Early career/unemployment and Club Business features, according to recovered summaries. |
| 0.9 | Manager creation, nationality, unemployment and offer-flow improvements, according to summaries. |
| 0.10–0.10.1 | Career and offer stability work; detailed changelog incomplete. |
| 0.10.2 | Menu/stability and nationality-selection corrections. |
| **0.10.3** | Reused calendar for unemployed manager; Forward Time, Wait for a Club Offer, Back, date and Business fixes. |
| **0.10.4** | Pixel-art Club Business, five stadium appearances and seven selectable facilities. |
| **0.10.5** | Four-menu pixel-art redesign; details in historical package notes. |
| **0.10.6** | Title, New Career and Calendar redesign retaining original callbacks. |
| **0.10.7** | Compact menus, original SWOS font, alignment, career fields and partial redraw. |
| **0.10.8** | Name/surname input fixes, conditional title buttons, calendar and unemployment UI. |
| **0.10.9** | Dynamic Board/Fans percentages and faces, supporter count, week number, calendar clipping. |
| **0.10.10** | Calendar row rendering without overlaps, abbreviated divisions and scroll arrows. |
| **0.10.11** | Alignment refinements; Business heading COSTS, action still UPGRADE. |
| **0.10.12** | Prevented Enter from reopening name/surname editor; dynamic calendar height and name truncation. |
| **0.10.13** | Up to 14 calendar matches, improved scroll controls and panel sizing. |
| **0.10.14** | Redesigned Club Offer, salary negotiation, View Squad and View Competitions. |
| **0.10.15** | Approved offer illustration, annual salary presentation and button hitboxes. |
| **0.10.16** | Welcome/Sacked, Board/Fans satisfaction, dismissal and persistence. |
| **0.10.17** | Management Record, country/region/salary, temporary forced-dismissal test. |
| **0.10.18** | View Squad redesign, player stars, statistics popup, controller hover, offer/sacked corrections. |
| **0.10.19** | Squad swapping and goals navigation, facilities, sponsor/TV and management effects. |
| **0.10.20** | Experimental individual Expanded condition and fitness/training recovery; later superseded. |
| **0.10.21** | WAGES/STAFF/TRAVEL accounting, week numbering, projections and two-column squad stats. |
| **0.10.22** | SEAS → YEAR, Business joypad navigation, STAFF hover and Squad access. |
| **0.10.23** | Smaller WEEK/YEAR button, STAFF popup, TRAVEL label, calendar arrows and fullscreen stretch. |
| **0.10.24** | Original SWOS font on WEEK/YEAR; removed map/shield residual borders using 320×256 menu viewport. |
| **0.10.25** | Stadium loading screen and real progress phases; Team Form/Morale replacing individual condition; temporary performance modifiers. |
| **0.10.26** | Business/Squad corrections incorporated into subsequent build; independent details incomplete. |
| **0.10.27** | Match-stat label changes, joypad adjustments, ADMINISTRATION and manager salary, Squad layout and facial colors. |
| **0.10.28** | STAFF popup columns, Board/Fans/Form/Morale faces, Squad alignment and stadium background. |
| **0.10.29** | No reliable version-specific changelog recovered. |
| **0.10.30** | Loading graphic, STAFF popup and Squad alignment; indicative Form/Morale bonus/malus. |
| **0.10.31** | Separate approved Squad/Loading image and UI layers; experimental START pause dialog. |
| **0.10.32** | Separate loading background/logo/caption; Business/Squad refinements; START unresolved. |
| **0.10.32b** | START restored to original pause action; experimental dialog removed. |
| **0.10.33** | START exit-confirmation popup, BACK/EXIT, STAFF and Squad tooltip refinements. |
| **0.10.34** | START/ESC and joypad fixes, Form/Morale modifier consistency. |
| **0.10.35** | Fixed off-screen exit-popup coordinates; SELECT match statistics. |
| **0.10.36** | Approved MATCH STATS and EXIT MATCH pixel-art overlays, live values over frozen pitch. |
| **0.10.37** | Duplicate-stat removal, border refinements, temporary exit selection arrow. |
| **0.10.38** | MATCH STATS cleanup, yellow EXIT MATCH selection border instead of arrow. |
| **0.10.39** | No independently verified version-specific changelog recovered. |
| **0.10.40** | Known development branch; exact source changes from preceding builds not checked for this document. |
| GitHub repository setup | Version-controlled source/asset archive and more durable project history. |
| Windows build workflow | MSYS2 compilation and BAT launcher/packaging scripts. |
| Later `main` commits | Local JSON menu settings, Windows menu layouts and executable packaging documented; do not assume branch parity. |

## 4. Important technical milestones

### Original SWOS integration

Development moved from early experiments to changes integrated with OpenSWOS. The objective remains to extend career management while retaining the original match engine.

### Management persistence

Historical builds introduced JSON persistence for salary, Board/Fans, sponsorship and Team Form/Morale. Current save compatibility and restoration of every Expanded field remain unverified.

### Financial accounting

Versions 0.10.21–0.10.28 refined player wages, staff/facility expenses, actual away travel, administration, manager salary and WEEK/YEAR presentation. The key requirement was to avoid double charging and distinguish forecasts from realized costs.

### Team Form and Morale

The 0.10.25 system replaced Expanded individual condition with team values. The documented performance modifier uses average M=(Form+Morale)/2: neutral at 65, maximum −20% at low values and maximum +20% at 100. Later faces use five expressions and three color bands: **0–39 red, 40–59 yellow, 60–100 green**.

### Widescreen/4K rendering

Versions 0.10.23–0.10.24 rendered the 320×256 Expanded menu region across widescreen displays instead of exposing residual pixels from the original 336×272 framebuffer. Match rendering remains separate.

### Pause, statistics and exit

Versions 0.10.27–0.10.38 iterated START/SELECT/ESC and the MATCH STATS/EXIT MATCH overlays. These are successive experiments, not simultaneously active alternatives. Latest documented EXIT MATCH selection uses a yellow border.

## 5. Approved visual implementations

| Screen | Historical integration | Main change |
|---|---|---|
| Title | 0.10.5–0.10.9 | Original logo, Expanded identity, compact menu, Hall of Fame |
| New Career | 0.10.6–0.10.12 | Pixel-art background, editable fields, SWOS font |
| Calendar | 0.10.3–0.10.13 | Match list, scrolling, date and unemployed variant |
| Club Business | 0.10.4 onward | Stadium, facilities, finances, satisfaction indicators |
| Club Offer | 0.10.14–0.10.15 | Contract illustration, salary, squad/competition previews |
| Welcome / Sacked | 0.10.16 | Dynamic management information and transitions |
| Management Record | 0.10.17 | Career history and annual salary |
| View Squad | 0.10.18 onward | Stars, statistics, Form/Morale and controller focus |
| Loading Screen | 0.10.25 onward | Approved stadium artwork, separate overlays, real progress |
| Match Stats | 0.10.36–0.10.38 | Pixel-art overlay with dynamic values |
| Exit Match | 0.10.33–0.10.38 | Confirmation flow and graphical panel |

**TRANSFER 1** has an approved visual reference; the available evidence does not establish a completed new worldwide transfer system.

## 6. Superseded experiments

| Earlier implementation | Replacement or later decision |
|---|---|
| Standalone desktop management UI experiments | Direct OpenSWOS integration |
| Expanded individual player condition (0–100) | Team Form and Team Morale |
| Forced dismissal test | Normal dismissal rules; final thresholds to verify |
| SEAS label | YEAR |
| AWAY financial label | TRAVEL |
| BUS STAFF terminology | Revised travel/staff terminology |
| OTHER general expenses | ADMINISTRATION |
| START mapped directly to ESC | Dedicated exit confirmation |
| Experimental START pause dialogs | Revised popup and controller handling |
| EXIT MATCH selection arrow | Yellow selection border |
| Loading holds of 5 or 20 seconds for screenshots | Temporary test behavior, not permanent requirement |

Exact removal of temporary behavior must be checked in current code before being called complete.

## 7. Known limitations and unverified features

| Feature | Evidence |
|---|---|
| Persistent worldwide league simulation while unemployed | Historically documented as incomplete |
| Joining a club in an already simulated season | Historically documented as incomplete |
| Complete worldwide league database | Long-term objective |
| Worldwide transfers | Design and partial interface work; full implementation unverified |
| Scouting with hidden `???` statistics | Rules still undefined |
| POTENTIAL-based player development | Design decision; implementation unverified |
| Fully redesigned Training | Not yet defined in detail |
| No-debt AI club finances | Approved rule; implementation unverified |
| Final dismissal thresholds | Multiple historical revisions |
| Retirement at 85 | Approved design; full flow unverified |
| Full latest Windows/joypad regression test | Not established by historical checks |

## 8. Testing and reliability history

User feedback across the project confirms historical successful menu startup, entry into career and partial match gameplay. Reported issues addressed in subsequent development packages include missing SDL2 runtime dependencies, crash on accepting offers, Back lock in unemployment, nationality flicker, calendar clipping, Squad focus/Exit, fullscreen borders and an exit popup rendered outside the screen.

Historical packages also describe GCC syntax checks, AddressSanitizer/UndefinedBehaviorSanitizer and asset/package checks. Such checks do not replace interactive Windows/joypad testing.

Future release verification should cover startup, friendly match, career creation, unemployment, offers, club acceptance, calendar, Business, Squad, loading, match stats, match exit, save and load.

## 9. Repository history and continuity

GitHub is the source/asset archive for code, data, approved art, Windows build scripts, commits and project documentation. `main` and `release/0.10.40` have different histories; changes on one branch should not be automatically attributed to the other.

- `AGENTS.md`: permanent working rules.
- `PROJECT_STATE.md`: approved decisions, current status, plans and open questions.
- `PROJECT_HISTORY.md`: historical build/package notes.
- `IMPROVEMENTS.md`: development history and changes actually introduced.

## Current workflow (6 November 2026)

Commits and builds are separate. Only explicit user requests authorize commits. Intermediate code revisions use `0.10.51_01`, `0.10.51_02` etc.; the next explicitly requested build is `0.10.52`. The title label displays `Build <version>`. This is a workflow and source change, not a verified Windows test.

## 10. Updating this document

For every new build, record:

- **Version and date**
- **Git commit**
- **Features added**
- **Existing behavior modified**
- **Bugs fixed**
- **Experimental behavior removed**
- **Tests performed**
- **Known limitations**

Do not describe mockups as implemented or syntax checks as successful Windows gameplay tests. Preserve superseded decisions and identify their replacement versions.

## 11. Sources and reconstruction notes

Compiled from recoverable SWOS Expanded project conversations, historical package README and test reports in `PROJECT_HISTORY.md`, `PROJECT_STATE.md` and predecessor notes, available GitHub commit history, and user-reported tests.

Early builds, some intermediate versions and branch-specific changes cannot be reconstructed conclusively. This is a detailed historical reconstruction, not a claim of complete line-by-line verification against every historical source archive.


## 0.10.52 — Title Menu PNG 16:9 (2026-11-08)
- Aggiunto decoder frontend per PNG RGB/RGBA 8 bit lossless con miniz esistente, senza SDL2_image o nuove DLL; BMP/RAW approvati intatti.
- Aggiunto renderer sfondo Title Menu RGB 1664×936 separato dal vecchio framebuffer, con mascheratura dell'immagine RAW e sovrapposizione dei pulsanti, testo dinamico e focus originali.
- F8 sul Title Menu: confronto immediato PNG/originale; fallback se PNG mancante, malformato o di dimensioni diverse.
- Build number `SWOS_EXPANDED_VERSION` a `0.10.52`; aggiornate regole PNG/risoluzione e relativa documentazione nel commit di sviluppo.
- Test: decoder RGB 1664×936 lossless rispetto a Pillow e mock SDL per alpha-mask/F8; **test su Windows/joypad e disponibilità dell'asset nel repository da verificare**, non dichiarati superati.


## 0.10.53 — Logo canonico del Title Menu (2026-11-08)
- Aggiunto il caricamento del logo PNG RGBA separato `assets/title/swos_expanded_logo_approved_2026-11-08.png` (1200×440, senza ®), condivisibile in futuro con altre schermate senza duplicare l'asset.
- Disegno in posizione C approvata dello sfondo 1664×936: x920 y17 620×227, scala relativa all'output e indipendente dai pulsanti; sfondo precedente e modalità F8 intatti.
- Fallback: il nuovo background rimane utilizzabile senza il logo; F8 mostra il titolo legacy originale per confronto.
- Incrementata `SWOS_EXPANDED_VERSION` a `0.10.53`.
- [TEST LOCALE] Decoder PNG RGBA con confronto pixel-per-pixel contro Pillow e harness C/SDL stub per ordine di rendering e coordinate, F8/mask; [DA TESTARE] compilazione Windows e interazione reale. Disponibilità dei binari PNG su GitHub da verificare separatamente.


## 0.10.53_01 — Title Menu moderno a due livelli (2026-11-08)
- [CODICE, WINDOWS NON TESTATO] Overlay titolo SDL2 non usa più lo sfondo RAW né i pulsanti SWOS nella presentazione. Rimossi toggle F8 e mascheratura storica. Sfondo RGB e logo trasparente approvati rimangono separati.
- [CODICE, WINDOWS NON TESTATO] Menu di primo livello a cinque pulsanti, sprite PNG RGBA derivati dalla grafica blu scelta dall'utente, pressione animata via SDL2; sottomenu classico provvisorio senza sfondo illustrato e callback originali mantenute.
- [CONSEGNA ESTERNA] I cinque PNG sono forniti in uno ZIP da estrarre nella cartella del gioco. Non considerarli committati in GitHub finché non sono realmente pubblicati dall'utente.


## 0.10.53_02 — Correzioni layout/navigazione/focus Title Menu (2026-11-08)
- [UTENTE — TEST WINDOWS] Segnalati Title Menu troppo a sinistra, focus giallo rettangolare e direzione GIÙ invertita anche nel sottomenu.
- [CODICE, DA RITESTARE SU WINDOWS] Pulsanti traslati sul lato destro e centrati sotto il logo approvato; maschera di focus giallo generata su tutti i pixel alpha della silhouette PNG completa, inclusi bordi smussati e spessore, senza toccare i file dei pulsanti.
- [CODICE, DA RITESTARE SU WINDOWS] Mappatura joystick/tastiera corretta (0 su, 4 giù), valida in entrambi i livelli; pressione più sobria di 5 tick senza deformare il PNG. Nessuna variazione di gameplay, salvataggi, sfondo o logo.


## 0.10.53_03 — Title più compatto e pulizia asset obsoleti (2026-11-08)
- [CODICE — TEST WINDOWS PENDENTE] Dimensioni dei cinque pulsanti ridotte circa del 10%, con centro orizzontale invariato (x=1230) e 64 px di spazio sotto il logo approvato; spostate le hitbox in accordo con le texture SDL2, senza toccare le PNG.
- [CODICE — TEST WINDOWS PENDENTE] Maschera esterna del focus giallo generata a raggio 5 px anziché 4 nei pixel sorgenti, continuando a seguire la sagoma alpha completa e la pressione.
- [PULIZIA REPOSITORY] Eliminati solo 11 file temporanei documentati/test/anteprima e vecchi manifest già consumati; il RAW del Title storico non è stato rimosso perché ancora richiesto da `expanded_economy.c` e `BUILD_SWOS.sh`. Conservati gli asset approvati e la procedura di trasferimento alternativa. `.gitignore` tutela contro il reinserimento di copie residue. Nessun test Windows effettuato dall'assistente.


## 0.10.53_04 — CONTINUE CAREER indipendente e menu ulteriormente compatto (2026-11-08)
- [ASSET APPROVATO / ZIP] Nuovo PNG RGBA `assets/title/buttons/continue_career.png` senza contorno giallo, consegnato via ZIP; deve essere pubblicato successivamente su GitHub dall'utente.
- [CODICE, WINDOWS DA TESTARE] CAREER diventa CONTINUE CAREER quando è attiva una carriera, con sprite specifico e senza sovrascritta runtime. Mantiene selezione, animazione e callback; rispetta le proporzioni dell'asset (contain).
- [CODICE, WINDOWS DA TESTARE] Altra riduzione ~10% di tutti e cinque i pulsanti, maggior respiro fra logo e pulsanti (106px) e spazi inferiori più compatti. Focus dinamico giallo da 5 a 6 pixel sorgente nei cinque sprite precedenti (12 pixel nel CONTINUE ad alta risoluzione per uno spessore a schermo uniforme), mai baked-in.
- [REGOLA] Un asset base UI non comprende la selezione: la sagoma del focus è un layer del renderer. Nessuna alterazione delle immagini statiche dei pulsanti precedentemente approvati.

## 0.10.54 — Title fullscreen/windowed con icone dinamiche (8 novembre 2026)
- [CODICE, DA TESTARE WINDOWS] Nuovo controllo video in alto a sinistra, due sprite PNG approvati senza testo (mostra solo l'azione possibile), controllo tastiera/joypad/mouse e ritorno dal fullscreen massimizzato nel profilo development.
- [CODICE, DA TESTARE WINDOWS] I cinque pulsanti principali sono ridotti di circa 10% rispetto a 0.10.53_04 e spostati verso il basso; background/logo/caption approvati invariati. Contorno di focus dinamico separato e generato in modo lineare rispetto ai pixel.
- [ASSET APPROVATO, ZIP] Entrambe le icone e CONTINUE CAREER in un unico ZIP strutturato da estrarre accanto a `AVVIA_SWOS.bat`; verifica del push GitHub separata.

## 0.10.54_01 — Stile canonico pulsanti e spaziatura Title (2026-11-08)
- [REGOLA UTENTE, DOCUMENTATA] Nuovi pulsanti pixel art blu frontali, con spessore scuro a destra/sotto e lieve bouncing sulla pressione. PNG base senza selezione gialla incorporata; focus dinamico dalla silhouette completa.
- [CODICE — TEST WINDOWS PENDENTE] Icona fullscreen/windowed trasferita da (58,56) a (34,34), mantenendo 92×88 logici. Hitbox e posizione della texture restano accoppiate.
- [CODICE — TEST WINDOWS PENDENTE] Spazi verticali dei cinque pulsanti principali ridotti, tenendo fisso il fondo di EXIT GAME (y871). Focus del Title ispessito (+2px nei PNG principali, aumento proporzionale per asset più densi). Bounce leggero di 1px sul rilascio, senza scaling del PNG.

## Correzione della scritta WORLD OF nel logo Title (2026-11-08)
- [ASSET APPROVATO] Corretto WORLO OF → WORLD OF, con intervento limitato alla forma dell'ultima lettera. Lo sprite approvato resta 1200×440 RGBA, logo e sfondo separati.
- [GITHUB] Sostituito il blob del PNG sotto il nome canonico esistente, conservando gli altri pixel del repository invariati; nessun ridisegno, cambio di posizione o revisione di gameplay, menu, callback, script o build.
- [WINDOWS DA VERIFICARE] GitKraken pull/sovrascrittura del vecchio PNG locale, render corretto del titolo in gioco.

### 0.10.56 — NEW CAREER corrections (2026-11-09)
[Code edited; Windows test pending] Upright integer-pixel bitmap text instead of fractionally scaled traced font atlas; checkbox inside unemployment toggle; direct FAME cycling without picker; arrow artwork not independently clickable. UI retains approved 88.5% scale and original background. No career or save-format changes. GitHub/GitKraken workflow restored.

### 0.10.57 — NEW CAREER visual fidelity fix (2026-11-09)
[CODE COMMITTED; WINDOWS TEST PENDING] Replaced stretched 5x8 font with offline-generated bold upright PNG atlases loaded at runtime with JSON glyph metrics; disabled age text popup on confirm and clamped age to 18-70; rendered legacy editing cursor 0xff as vertical caret rather than '?'; used narrow yellow alpha-derived toggle/start focus and a single thin form-field outline; separated touching input columns visually. Background and approved base graphics retained without resize; bitmap assets supplied separately for extraction.

---

## SWOS_DOC_SYNC_2026_11_10 — Aggiornamento documentale, senza modifiche al motore

**Tipo:** registrazione della manutenzione della documentazione progettuale, non implementazione. Preparati `docs/GAME_DESIGN.md` e aggiornamenti coerenti a `AGENTS.md`, `PROJECT_STATE.md`, `docs/DEVELOPMENT_WORKFLOW.md` per le decisioni del 10 novembre 2026: tre schermate graficamente finali, grafica obsoleta degli altri menu, calendario ad eventi, WEEK quasi ovunque, NEXT EVENT, chairman, YOUTH/RES/TITOLARE, completamento prioritario con database attuale e futuro BAT di pulizia asset verificati orfani.

**Codice/assets/build:** nessuna modifica e nessun file rimosso in questa operazione. **Test Windows:** non eseguiti. **GitHub:** nessun commit né push effettuato da questo pacchetto. Gli interventi funzionali descritti nel GDD sono futuri finché non confermati dal codice e dai test.


## 0.13.01 — Prima API calendario mondiale (10 novembre 2026)

- [CODICE] Aggiunto modulo C autonomo `expanded_world_time.c/.h`: date gregoriane 1900–2200, inizializzazione con anno/mese/giorno arbitrari, conversioni, day index, settimana relativa alla stagione, avanzamento solo in avanti.
- [CODICE] `Expanded_FormatDate` non implementa più un calendario separato; passa tramite l'API comune. I menu occupato/disoccupato usano `World_FirstYear()` per il mondo attivo; settimana visualizzata nel disoccupato ricalcolata dal reale inizio stagione (1 agosto nell'interfaccia provvisoria).
- [TEST GCC LOCALE] Test isolato di date 1996/2026/2034, bisestili, validazione e avanzamento; scansione completa dei 109.938 giorni 1900-2200. Nessun test Windows/joypad eseguito.
- [LIMITI] Nessun nuovo scheduler, risultati simulati, gestione eventi, salvataggio del clock o modifiche a fisica/finanze. La versione `0.13.01` applica la nuova numerazione richiesta dall'utente; la convenzione precedente con _01 è storica.

## 0.13.02 — Debug osservabile e ledger cronologico (10/11/2026)

- [CODICE] F12 mostra/nasconde debug testuale in sovraimpressione; F9 salva un PNG della **schermata realmente presentata**, debug incluso; F11/PrintScreen restano disponibili per screenshot legacy. Nessun nuovo asset.
- [CODICE] Tasti 1/2/3 mentre F12 è aperto: forza WIN o LOSS alternativamente per i risultati ufficiali della squadra gestita, oppure arma l'esonero al successivo ingresso nel percorso partita. Il motore fisico non cambia; i toggle non sono persistiti nel JSON e sono OFF al boot. I risultati forzati possono essere differenti dai gol effettivamente giocati: funzionalità solo per test.
- [CODICE] `expanded_world_ledger.h/.c`: registro cronologico indipendente con risultati immutabili, classifiche riproducibili, ID di incontri, doppia modalità 2/3 punti per vittoria e callback di simulazione. **Ancora scollegato da OpenSWOS** e dal salvataggio reale.
- [CODICE] La data secondaria del calendario nativo usa `World_FirstYear()` per il mondo scelto, al posto dell'anno fisso 1996.
- [TEST GCC ISOLATI] `tests/run_test_expanded_phase12.sh` verifica controlli e registro; simulazione artificiale di 35 settimane/140 risultati e classifiche. Nessuna compilazione completa e nessun test Windows in questa consegna.


## 0.13.03 — Pilot risultati mondiali durante UNEMPLOYED

- [CODICE] Simulazione offscreen indipendente dal manager, basata sulle squadre realmente lette da cinque paesi già sicuri del database e dalle relative divisioni supportate; ID squadra effettivi, rating aggregato dalla rosa, calendario round-robin **provvisorio** e risultati deterministici. Usa il registro cronologico introdotto in 0.13.02, avanzato a ogni settimana della disoccupazione.
- [CODICE] La schermata VIEW LEAGUE dell'offerta mostra ora classifica provvisoria e posizione del club con partite giocate e punti, anche prima della firma. I dati non sono risultati del motore carriera nativo.
- [CODICE] Gestione turni di riposo per divisioni con club dispari; rifiuto di costruire gironi parziali oltre 26 club; supporto a due stagioni-pilota. Nessun nuovo asset e nessun intervento al pannello debug F12.
- [TEST ISOLATI] GCC -Wall -Wextra -Werror + ASan/UBSan: 20 club/350 gare dopo 35 settimane; girone dispari 5 club e settimana 10; turni e classifiche validi. Non è ancora un test completo Windows.
- [ANCORA DA FARE] Calendario regolamentare, mondo completo, coppe, record nel salvataggio, classifica sincronizzata con motore SWOS e assunzione a metà stagione senza reset. Il vecchio InitCareer conserva il proprio comportamento.


## 0.13.04 — Fix sintattico bloccante nell'adapter (10 novembre 2026)

- [CODICE] Corretto `\\n` letterale introdotto accidentalmente in `expanded_world_live_adapter.c:55`, diventato un vero a capo nella condizione di validità del puntatore alla competizione. Nessuna modifica alla logica.
- [VERSIONE] Build aggiornato a `0.13.04`. Compilazione completa Windows e prove runtime da ripetere a cura dell'utente.


## 0.13.05 — Inizio luglio e precampionato pilota (11 novembre 2026)

- [CODICE] Il calendario UNEMPLOYED e l'orologio del mondo pilota iniziano il **1° luglio**. Le date delle partite native continuano per ora con il riferimento agosto: questo commit non cambia le chiavi del calendario SWOS.
- [CODICE] Quattro date estive nel mondo pilota per ogni divisione simulata, con abbinamenti amichevoli fra squadre, turni di riposo nei gironi dispari e aggregati autonomi di risultato, morale, fatica, infortuni potenziali e saldo stimato. Valori in RAM **non persistiti, né trasferiti nel gioco originale**.
- [CODICE] Prima delle partite ufficiali, VIEW LEAGUE mostra quante amichevoli pilota ha giocato la squadra offerta. Conversione più coerente alla data civile luglio quando il manager torna disoccupato dopo un esonero.
- [DESIGN DOCUMENTATO] Aggiornato `GAME_DESIGN`: precampionato globale, amichevoli organizzabili, prezzi biglietti in CLUB con elasticità domanda/FANS LIKING, unificazione CLASSIFICA/SQUAD e carriera europea dal 1° luglio. Funzionalità di prezzo biglietti e assunzione nativa a stagione iniziata non ancora implementate.
- [TEST] Test di regressione C estesi e disponibili nel repository; compilazione ed esecuzione Windows da verificare sull'utente. F12, salvataggi e fisica partita invariati.


## 0.13.06 — Recupero nativo del calendario alla prima assunzione (11 novembre 2026)

- [CODICE SPERIMENTALE] `expanded_midseason_join.c/.h`: automatizza le azioni native NEXT MATCH / VIEW RESULT durante l'ingresso in carriera dopo settimane di disoccupazione; conversione luglio → agosto e stop alla prima giornata futura. Non modifica la fisica delle partite.
- [CODICE] Guardia per non attribuire al nuovo allenatore statistiche, morale relativo a risultati e esoneri di partite antecedenti; registrazione delle gare lasciata alla normale logica OpenSWOS. Durante l'avanzamento automatico non aprire offerte economiche o trasferimenti di BUSINESS.
- [CODICE] Callback da `MenuProc` e dai due percorsi di nuova assunzione; regole di abort con log in `SWOS.log`. La classifica nel menu CARRIERA resta quella nativa, con progresso dalle partite del suo calendario.
- [TEST] Runner isolato per calcolo cutoff incluso, ma compilazione Windows e test in-game ancora da eseguire. Non garantito confronto esatto tra classifica VIEW LEAGUE pilota e quella nativa.
- [DESIGN] Documentati: mondo selezionabile da OPTIONS (non ancora implementato), sponsor preesistenti/scadenza/rinnovo (approvato ma non implementato), forma individuale+training collettivo (solo discussione futura). Nessun nuovo asset, F12 invariato.


### 0.13.07 — filtro offerte da risultati disponibili (11 novembre 2026)
- [CODICE, NON TESTATO WINDOWS] Quando la classifica pilota di una divisione è disponibile e sono state disputate almeno 3 partite, la selezione di club per offerte di lavoro pesca solo nella metà inferiore, evitando offerte casuali da capolista. Se i dati non sono sufficienti conserva il comportamento precedente, senza simulare risultati inesistenti.
- [DOCUMENTAZIONE] Registrati nel Game Design i due incidenti ChatGPT riportati dall'utente (Connection interrupted, Unknown error), con stato e limiti del recupero.
- [NON COMPLETATO] Unificazione reale pilot/native, sistema di vacancy persistenti di allenatori, nuove metriche database F12. Non attribuire a questa revisione una soluzione al reset del calendario dopo ingaggio.
- [TEST] Revisione statica della modifica e riscontro file GitHub; non compilato, non eseguito in Windows/joypad.


### 0.13.08 — controlli integrità native midseason catch-up (11 novembre 2026)
- [CODICE] Rafforzato l'accesso al keycache 8192 byte durante la scansione di pre-ingaggio: ogni stride è validato contro limiti e ordine delle fasi, e si richiedono incontri sia prima sia dopo il momento dell'assunzione. Controllo di bounds anche sul record corrente in ogni tick.
- [NON RISOLTO] Il calendario pilota differisce ancora dal calendario OpenSWOS inizializzato alla firma; non dichiarare completa la continuità dei risultati/classifiche o il salvataggio del mondo.
- [TEST] Revisione statica; compilazione e test reali Windows non eseguiti in questo ambiente.


### 0.13.09 — cutoff di fase nativo e diagnostica mondo F12 (11/11/2026)
- [CODICE] Corretto il confronto tra settimana civile e fase nativa OpenSWOS: codifica mesi ×8 e rollover +0x60; test unitari aggiornati.
- [CODICE] F12 esteso con dati del pilota (club, leghe, slot giocatori, fixture, risultati, RAM) e situazione native cache/fase e catch-up; dati mancanti (manager/chairman globali, dimensione su disco) dichiarati non disponibili.
- [REGOLA] Commit solo per cambiamenti significativi autorizzati; controllo diagnostiche F12 in ogni build.
- [PENDENTE] Compilazione Windows e test reale dell'assunzione. Pilot/world e carriera nativa restano non unificati; non dichiarare risolto il bug principale.


### 0.13.10 — simulazione CPU delle gare pregresse e coppe (11/11/2026)
- [CODICE] Il recupero di un ingaggio durante una stagione dirige le partite del club già trascorse verso la simulazione nativa anche se il menu presenta PLAY MATCH. SetupPlayers disabilitato soltanto in catch-up e percorso coppe native gestito nel driver automatico.
- [DEBUG] Contatori F12 per azioni catch-up e simulazioni forzate; logging di menu non riconosciuti e riepilogo dei motivi di arresto.
- [VERIFICA PENDENTE] Compilazione Windows, ingressi con coppe e confronti delle classifiche. Non è stata dimostrata l'identità tra le due classifiche pilota e nativa: il calendario doppio rimane in attesa dell'unificazione.
- [ROLLBACK] Precedente commit `fab201b98ae60ab37f105ee48589c66b234a809c` (0.13.09), da ripristinare manualmente con GitKraken soltanto su decisione dell'utente.


### 0.13.11 — data/carriera sincronizzata e recupero invisibile (11 novembre 2026)
- [CODICE] Corretto `Expanded_FormatDate`: i valori di sortKey sono ottavi di mese nativi, non settimane da agosto. Il caso mostrato (28 AUG 1997 nella stagione 1996/97) nasceva da questa conversione errata.
- [CODICE] UI carriera/business usa la fase della prossima fixture (keycache) per mostrare una data aggiornata, lasciando il contatore finanziario separato. Date d'ingaggio arrotondate alla successiva fase utile per comprendere tutte le partite anteriori.
- [CODICE] Driver: NEXT MATCH prioritario su VIEW RESULT; blocco input dell'utente durante recupero; overlay temporaneo che copre le classifiche mentre vengono simulate. F12 mostra esito dell'ultima esecuzione.
- [TEST] Controllati indipendentemente 94 casi nel calcolo delle fasi 1996/2026; test unitari C aggiornati ma non compilati, build/test Windows ancora da effettuare.
- [PENDENTE] Resta la divergenza fra risultati preview/pilota e risultati nativi; nessuna fusione di calendari o serializzazione mondo implementata.


### 0.13.12 — avanzamento nativo dopo menu lega completato (11/11/2026)
- [CODICE] Aggiunta la transizione automatica `DIYLeagueTournamentExit` quando nella schermata lega non sono disponibili NEXT MATCH/MATCH/RESULT/VIEW RESULT. Il driver precedente abortiva -8 dopo la giornata per assenza di callback, lasciando data e stagione incomplete.
- [DEBUG] Contatore F12 `LEAGUE MENUS COMPLETED` e log del ritorno al calendario per distinguere progressi reali da cicli nel menu. Interrotto ancora con -8 se non e' visibile neppure EXIT.
- [LIMITI] Solo controllo statico degli handler attivi, nessuna build Windows/test runtime; la preview del Living World Engine rimane separata dalla competizione nativa. Data, formule e renderer della 0.13.11 non modificati.


### 0.13.13 — ponte controllato tra risultati pilota e handler nativo (11/11/2026)
- [CODICE] Le gare di campionato del recupero nativo provano a riutilizzare un punteggio registrato nel pilota per la stessa coppia casa/trasferta, entro un intervallo temporale verificato di circa una settimana, una sola volta; se non c'e' corrispondenza, motore OpenSWOS invariato.
- [DEBUG] F12 `NATIVE REUSED` e `FALLBACK`; la dicitura `PILOT/NATIVE REPLAY PARTIAL` conferma che esistono ancora due calendari distinti.
- [TEST] Aggiunti casi di test C per abbinamento, errore temporale, mancato riuso e identita' invertite. Esecuzione completa e build Windows non verificate nell'ambiente corrente.
- [DOCUMENTAZIONE] Registrata nuova segnalazione `Connection interrupted` dell'11 novembre in `docs/GAME_DESIGN.md`.
- [SCELTA UTENTE] Lasciata invariata la posizione del benvenuto e il momento di avvio del recupero; l'accelerazione e l'unificazione globale saranno affrontate nella fase successiva, senza nuovi reset.


### 0.13.14 — rendering disaccoppiato dal recupero, ledger matching esteso (11/11/2026)
- [CODICE] In `Platform_RenderFrame` il recupero precedente alla firma disegna uno status statico alla prima iterazione; tutte le successive iterazioni processano eventi ma non SDL upload/render/present. In `Platform_Present` bypass esclusivo del pacing normale a 50Hz durante il catch-up. Timer-render fallback escluso durante l'operazione. Nessuna modifica al pacing della partita.
- [CODICE] La ricerca pilota→nativo non richiede più che le date degli stessi club siano entro una settimana; usa un unico risultato gia' simulato per ciascuna coppia esatta casa/trasferta nella stessa stagione. Il vecchio vincolo temporale aveva dato 12 match riutilizzati su 372 nella prova utente 0.13.13.
- [DEBUG] HEADLESS frames/time nel F12 per confrontare l'attesa reale; NATIVE REUSED/FALLBACK mantengono il conteggio degli abbinamenti. 
- [TEST] Aggiornate asserzioni pure-C per stagione corrente, non riutilizzo, invertire casa/trasferta e divieto di attraversare stagioni. Test C compilazione Windows **non eseguiti nell'ambiente attuale**.
- [NON RISOLTO] Le tabelle non possono essere garantite uguali finché `ExpandedWorldLive` e OpenSWOS non utilizzano un unico calendario/ledger. Non cancellare la preview né correggere artificialmente i punti per ottenere l'identità visiva.


### 0.13.15 — OG fixture schedule / shared results in both directions (11/11/2026)
- [CODICE] Il generatore mondiale ricicla le tabelle originali `_fixtureSchedulePtrTable`, anziché creare nuovi incontri con l'algoritmo circle separato. `ExpandedWorldLive_SetLeagueSchedule` importa da ogni Competition i mesi di inizio/fine e produce le date per fasi native (otto per mese).
- [CODICE] `DIY_ShuffleLeagueTeamOrder` non effettua un secondo sorteggio solo quando trova i club esattamente presenti nel ledger della stessa lega; riproduce la medesima numerazione per il calendario OG. Altrimenti il comportamento SWOS originale è invariato.
- [CODICE] `ExpandedWorldLive_RecordNativeLeagueResult`: dopo la firma, risultati originali del manager scritti nel ledger mondiale, in caso di match preciso nella fase nativa. Gli altri incontri CPU avanzano nello stesso ledger; nessun cambiamento alla fisica partita.
- [DEBUG] `OG LEAGUE CALENDARS / ORDERS` controlla in quali divisioni il collegamento è realmente attivo. Rimangono conteggi F12 dei fallback e del recupero.
- [TEST] Tabella originale per 4,20,24 squadre controllata staticamente; harness C ora linka fixture_data originale e modulo dei round dispari. Test C e Windows non eseguiti in questa sessione.
- [LIMITI] Non ancora rimossi stato delle competizioni nativo distinto, catch-up dopo assunzione e world-save. Leghe speciali, coppe e riassunzione da testare. Versione per rollback: 0.13.14 `e2cee6d`.


### 0.13.16 — importazione diretta di campionato e provenance manual result
- [CODICE] \`DIY_WorldDirectImportLeague\` ricostruisce lo stato originale del campionato a due gironi da risultati mondiali già registrati, con progressione fixture SWOS originale e KeyCache player match marcata solo se completa e consistente. Salvataggio del buffer nativo solo dopo controllo totale; fallback al vecchio driver quando incompatibile.
- [COPPE] Le coppe non hanno ancora un ledger mondiale; vengono mantenute nel percorso OG. Per storia senza coppe precedenti e import valido si elimina il recupero dopo firma (JOIN DIRECT). Con coppe precedenti il driver lavora solo sui turni OG residui e salta la lega già importata.
- [DEBUG] F12 mostra \`DIRECT OG IMPORT\`, \`LAST OG\` con goal e punti, \`RAW FINAL\` con score match/mismatch per gli incontri manuali. Nessuna modifica ai punti assegnati dagli handler originali.
- [TEST] Test read-only \`PeekNativeLeagueResult\` aggiunti; build e test Windows PENDENTI. Fallback/rollback alla \`0.13.15_01\` confermata dall'utente.


### 0.13.17 — persistenza JSON del world ledger
- [CODICE] Snapshot Living World dentro il normale .CAR: calendari originali, club, leghe, risultati e stato della simulazione, con unica fonte di verità nel ledger.
- [ROBUSTEZZA] Lettura transazionale con controlli di range e appartenenza esclusiva dei club alle leghe; fixture elaborate in tempo lineare.
- [SAVE] Schema 4 per i nuovi file, schema 3 ancora leggibile. Un vecchio save non conserva la precedente simulazione mondiale. Nella versione Amiga il campo mondiale è null.
- [DEBUG] F12 indica world snapshot pronto oppure assente.
- [LIMITI] Nessun motore completo: coppe mondiali, cicli multistagionali, allenatori/presidenti, mercato AI, generazioni e salvataggio nativo integrale sono ancora da sviluppare.
- [TEST] Nessuna compilazione MinGW64 o esecuzione Windows svolta dall'assistente; test interattivi rinviati con il resto della convalida.


### 0.13.18 — validazione salvataggi pluriennali
- Corretto un vincolo troppo stretto nel parser .CAR schema 4: settimane/date successive al secondo anno rimangono caricabili entro il range del calendario mondiale. La produzione di nuove stagioni resta un intervento separato.
- Windows compile e test save/load ancora pendenti.


### 0.13.19 — riduzione dello spazio del world snapshot
- [CODICE] Compressi solo i fixture del ledger condiviso nel .CAR JSON tramite miniz (zlib + Base64); nessuna DLL, file secondario o simulatore duplicato.
- [SAVE] Nuova chiave fixtures_zlib e lunghezza decompressa verificata; snapshot v2 in scrittura, lettura v1/v2; .CAR schema 5 in scrittura, schemi 3/4 ancora leggibili.
- [SICUREZZA] La decompressione ha un limite noto, conserva il parser esistente e non importa snapshot invalidi. Se la compressione fallisce, il salvataggio viene annullato anziché perdere il mondo senza avviso.
- [DIMENSIONI] Su dati sintetici, 8.384 fixture 276 KB -> 96 KB; 40.000 fixture 1,32 MB -> 0,46 MB. Non sono misure della build Windows e non includono gli altri dati della carriera.
- [PIANO] Per il motore globale finale serviranno archivi/delta ed eventualmente pack dati statici, ma non è stato cambiato l'assetto architetturale in questa build.
- [TEST] Nessuna compilazione MinGW64 o esecuzione Windows completata dall'assistente, richiede pull/compilazione in GitKraken/MSYS2; strumenti statistici rinviati alla fine dell'engine.


### 0.13.20 — portable autosave
- [CODE] Slot `save/AUTOSAVE.CAR` automatico: firma, fine incontro,
  ritorno sicuro ai menu, catch-up e stagione; CONTINUE dopo riavvio.
- [IO] Salvataggio .CAR tramite .tmp + sostituzione atomica; vecchio
  checkpoint valido preservato se una scrittura fallisce.
- [WINDOWS] save, config e log nella cartella dell'EXE; script separato
  confermato per ripulire i vecchi save condivisi in Documents.
- [LIMITI] Match live 50 Hz non salvato/ripristinato: anti-save-scum
  intrapartita ancora incompleto; niente forza-esito a crash.
- [TEST] Source review; build MinGW64 e Windows runtime pendenti.


### 0.13.21 — corretto controllo date caricamento world
- [CAUSE] Il campionato reale paese 21 (maggio-settembre) crea 45 gare antecedenti al 1° luglio, che il parser della 0.13.20 considerava impropriamente invalide al load.
- [CODE] Leggere fixture mondiali dal 1° gennaio dell'anno iniziale, preservando calendario OG multi-paese, risultati e controlli anti-corruzione.
- [TEST] Test C di round-trip snapshot compresso aggiunto. Verifica Python sul .CAR utente: 8.384 fixture, compressione integra; build Windows e test C compilati non ancora eseguiti.


### 0.13.22 — mondo 70 stagioni, senza nuove formule sportive
- [CODE] Finestra cronologica di due stagioni, roll automatico al 1 gennaio, riuso senza collisioni degli ID a 16 bit solo dopo archiviazione, nuova generazione dal calendario OG anche per leghe maggio-settembre; controllo della continuità dei risultati residenti.
- [HISTORY] Archivio compresso per stagione/lega: campione, punti, incontri, digest; mantiene riepiloghi permanenti senza fixture storiche complete. I risultati vecchi di dettaglio NON sono più accessibili, non nasconderlo.
- [SAVE] Snapshot living_world v3, campo window_first_year, history. Lettore ancora compatibile con v1/v2, schema carriera 5 invariato e autosave in save/AUTOSAVE.CAR.
- [INTEGRATION] Sincronizzazione del clock mondiale a nuovo anno carriera in ContinueCareerNextSeason, lasciando intatti comandi/fisica OG. F12 mostra finestre, archivio e residenti.
- [TEST] Nuovo TEST_LIVING_WORLD.bat e harness C da 70 stagioni, log LIVING_WORLD_70_TEST.log. Validazione ambiente Windows, 70 stagioni e caricamento .CAR reale PENDENTI.
- [DISCUSSIONE FUTURA] Tuning dei valori simulativi dopo struttura engine; nessuna nuova formula sportiva impostata in questa revisione.


### 0.13.23 — correzione test GCC 16.2
- [REPRO] Log reale del test da 70 stagioni: stop al compilatore per 6 warning elevati a errori (2 mjson, 2 miniz, 2 expanded_world_live), non errore della simulazione.
- [TEST RUNNER] Compilazione separata di mjson/miniz con esclusioni mirate dei warning legacy solo in quelle unità; mantenuti `-Wall -Wextra -Werror` in tutto il codice nostro. Aggiornati entrambi gli script C di test Living World.
- [CODE] Cast `long` dei limiti dei parser; validazione numero risultati native OG rispetto a tutti i risultati residenti e archiviati in carriera, non soltanto la finestra di due stagioni.
- [LIMITI] Test compilato e benchmark 70 stagioni ancora non verificati in questa sessione; atteso risultato Windows/MSYS2 dopo pull.


### 0.13.24 — ripara il collegamento dell'harness e i primi match storici
- [REPRO] Log reale del test 70 anni 0.13.23: GCC compila gli oggetti ma il linker non trova `swos_io_realloc` da mjson.o. Nessuna stagione simulata.
- [FIX TEST] Con solo `mjson.o` nei due harness isolati, `MJSON_REALLOC` punta alla `realloc` standard tramite macro + include forzato del relativo header. Il gioco usa ancora l'allocatore SWOS. Miniz e gli altri sorgenti non alterati.
- [FIX ENGINE] Al primo avvio `ExpandedWorldLive_Seal` e in lettura snapshot preesistenti, risultati deterministicamente completati per fixture con data anteriore al 1° luglio d'inizio carriera, necessario per archivio prima stagione dei campionati maggio-settembre. Il tempo non avanza all'indietro.
- [REGRESSION] Test C base e test 70 stagioni verificano la presenza di risultati maggio/giugno prima di procedere. Test di compilazione e run su Windows ancora da verificare.


### 0.13.25 — un solo avvio mondo a START; debug NEW CAREER
- [CODE] Il mondo dei club/competizioni parte al pulsante START di NEW CAREER indipendentemente dal contratto. Eliminata la re-inizializzazione durante la prima ricerca di offerte. Cancel selezione o ricerca cancella soltanto il mondo provvisorio in RAM.
- [DEBUG] F12+N al Title apre il form NEW CAREER, anche con autosave presente; vecchio .CAR conservato fino al primo successivo writer atomico. Nessuna nuova voce grafica Title, nessun gameplay/valore sportivo cambiato.
- [DA COMPLETARE] Differenti cronologie OG/Expanded e bridge calendario nativo, eventi infrasettimanali, coppe persistenti e carriera mondo ancora da unificare e validare. Non chiamare la singola inizializzazione un motore unificato completato.
- [PARAMETRI] Forma individuale, chairman e coach AI e tutti i valori iniziali rinviati alla penultima fase progettuale approvata.
- [TEST] Verifiche statiche; build Windows e test manager reali pendenti.

### 0.13.26 — single civil-day advancement and world league event diagnostics (12 November 2026)
- One `ExpandedWorldLive_AdvanceToDate` advances league fixtures and preseason through the existing world ledger; weekly and OG season-boundary callers share it. July 1 does not round into a future week. No new calendar generator, .CAR fields, competition handlers, or 50 Hz match changes.
- F12 shows WORLD CLOCK, NEXT LEAGUE EVENT and pending/overdue fixture counters. A real-time status query checks result counts without altering state.
- 70-season stress harness adds a midweek advance, save/load round-trip, backwards-time rejection and calendar audit checkpoints. Windows/MSYS2 compilation and gameplay verification pending.
- The original OG cup/knockout events and native career fixture callbacks remain active. Full calendar/event unification is still open and must be proven, not assumed.

### 0.13.27 — global club-offer coverage and native knockout outcome bridge (12 November 2026)
- The unemployed manager may advance time with a pending job offer; the offer expires and a fresh one may arrive later. New offer candidates come from actual country team files represented by the imported world leagues, without any manager-nationality filter or artificial >=3-division gate.
- Expand live league capacity and import all supported world/career-country lists using the existing official OG fixture tables. Capacity is raised, while unsupported/missing native formats remain excluded and must be reported rather than fabricated.
- Hook original `DIY_PlayCupMatchAndAdvance` after a completed score and after a real champion is declared. Preserve every OG bracket callback and score; store native knockout match/replay/leg results and trophies in the canonical world save, with a chronological two-season resident window and archived totals. World snapshot internal format v4 remains backward-readable for v1-v3 careers.
- The 70-year C runner now exercises a synthetic knockout outcome journal through season rollover and roundtrip, as well as source-country discovery; `TEST_LIVING_WORLD.bat` fails if any required PASS marker is absent. These static source changes are NOT yet a tested Windows build, nor a full offscreen global cup simulator.

### 0.13.28 — diagnosi fallback OG, test suite unica, debug leggibile (12 November 2026)
- Unemployed offer no longer auto-selects VIEW CLUB OFFER; keeps FORWARD TIME selected for keyboard/controller. Debug mode adds DRAW (F12+4, 1-1 persisted final score), with standalone regression assertions.
- Native join exposes historical non-league event count and direct import reason. The existing OG callbacks remain necessary where world snapshots lack cup history or native fixtures do not match; avoid presenting the old artificial full-screen catch-up card, keep the prior frame while replay continues. This is NOT a fix for global duplication.
- Two-column, color-coded F12 with legible section spacing and real TEAM-file import coverage; correct living-world snapshot display v4. Windows games and legacy renderer are otherwise untouched.
- One BAT runs previous 70-year synthetic stress plus independent midseason, world-live, debug/ledger tests and an installed-data inventory, writing one report. Synthetic results are labelled; no false PASS for worldwide promotion/relegation or offscreen cups.
- Validation: static edits only; Windows compile and runtime tests pending.
