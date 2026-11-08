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
