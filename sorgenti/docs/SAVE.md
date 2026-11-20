### 20/11/2026 — REGRESSION GATE CAR / ROLLOVER / NO-REPLAY (VIGENTE)

Prima di modificare il writer, il reader, il codec Zstandard o i record OG consultare: [regole AGENTS](../../docs/AGENTS.md), [workflow](../../docs/DEVELOPMENT_WORKFLOW.md), [registro bug e mappa](../../docs/REGRESSION_HISTORY_AND_DEPENDENCIES.md) (in particolare RG-006/012/017/022–026/028 e test RG-T14–T17/T23). **Non dedurre il PASS dai tre hash validi 00.13.225**: è stata verificata l'integrità delle copie e una decodifica preesistente, non un nuovo SAVE → EXIT → CONTINUE del manager 2028.

**Dependency gate:** `SaveJson_WriteCareer/LoadCareer` → `Expanded_OGWorld_WriteSaveJson/ReadSaveJson` → snapshot comp/fixture/witness/RNG → mondo/edizione/club IDs → `swos_io_save_atomic` → test indipendente legacy e cross-process. Ogni nuova sezione CAR versionata è opzionale e validata; un CAR vecchio senza dato non deve ricevere risultati o classifiche inventate. Il file esistente non viene mai distrutto su errore, l'autosave è atomico, non richiamare OG per riprodurre gare già registrate. Distinguere test C isolati da compilazione e gameplay Windows.

### 20/11/2026 — RIFERIMENTO SALVATAGGI PER NUOVE COMPETIZIONI (GUIDA 217–225)
Prima di modificare un torneo o il suo ciclo annuale consultare **[docs/COMPETITION_ENGINE_IMPLEMENTATION_GUIDE.md](../../docs/COMPETITION_ENGINE_IMPLEMENTATION_GUIDE.md)**, sezioni CAR, rollover, anti-replay, archivio e test: 217–225 dimostrano perché resident type1/type2, fixtureId, fase, RNG, roster, witness e dati KO delle edizioni precedenti devono sopravvivere a SAVE/EXIT/CONTINUE. La 221 poteva correggere `totalMatchesPerSection` 16→32 **solo** sui vecchi CAR senza gare; la 225 preserva l'ultimo KO Bolivia, mentre i 224 che lo hanno già perso non autorizzano trofei sintetici. Per ogni torneo verificare blob/og_resident/legacy con test Windows **cross-process**. Questa appendice è solo documentazione, non un nuovo schema CAR né una build testata.

### 20/11/2026 — 00.13.221: Copa Bolivia group-state CAR; REGRESSIONE WINDOWS DA PROVARE
Unico CAR Zstd schema 6 invariato. Il residente OG nazionale Bolivia 2627 tipo2 (`s_expOGBolivia`) rimane nello stesso `og_resident` che dal 220 salva competizione, ordine delle 64 coppie, squadre, RNG e numero incontri `matchesPlayedCount`; non introdurre nuovi sidecar. Il 221 esegue le coppie dalla tabella esistente seguendo la data reale con permutazione calcolabile da `fixtureId` (invariabile per old CAR). Il journal OG conserva una prova per ogni match CUP country45 slot2 e la sua data civile esatta; contatori OG/journal devono coincidere. L'upgrader al primo inoltro corregge `totalMatchesPerSection` da 16 a 32 SOLO sui CAR 220 a ZERO incontri: niente reinterpretazione di file gia' giocati. In caso di fallimento witness/native, `expanded_og_world_run_year` propaga FAIL e deve bloccare nuovi CAR, lasciando il precedente valido. DA VERIFICARE: SAVE dopo 6/12/64 gare, EXIT, CONTINUE, F12 conteggi, idempotenza dopo ulteriore avanzamento, limite 64, compatibilita' legacy 220 e blocco gara manager; non attestare PASS senza test Windows. KO e manager cup bridge non salvati perche' NON ANCORA INTEGRATI.

### 20/11/2026 — 00.13.217: audit date sudamericane, guard viewer/5555; IMPLEMENTAZIONE PARZIALE
La nuova decisione approvata dal manager e': il 1 luglio non deve resettare una competizione OG ancora attiva; ogni torneo termina ed apre la successiva edizione alla propria data con gli stessi ID, fixture, punteggi e RNG, e SAVE/CONTINUE deve ricostituirli senza replay. Per la prima carriera 2026/27 fotografare i risultati REALI precedenti al 01/07/2026 per tutti i paesi; tra i titoli gia' noti Belgrano (43:21), Apertura argentino 24/05/2026. **NON ancora implementato nel runtime**: tabella dei 30 club, snapshot storici di coppe, ciclo per competizione e Libertadores 2027: ct_world_first_ready=0; 6 Q1/13 Q2 e sorteggi non approvati, CT round mensile non per-gara.
Intervento sicuro: Copa Bolivia in mondo 2627 `world_data.c` endMonth 32→87 (finale reale 25/11, nessun match preluglio; inizio OG 56 Agosto); viewer LEAGUE/CUP/TYPE2 firma l'intero contesto OG prima/dopo e riporta `[og-viewer-check]`; guard di memoria in `DIY_ValidateLeagueScratchBuffer` per range 5555 e delta positivi/negativi, **NON fix della causa 5555**. Manifest `data_json/worlds/2627/competitions/conmebol_2026_cutoff_audit.json` contiene 10 paesi e 11 coppe come **catalogo audit NON CONSUMATO DA OG**, tutte le altre coppe con primi incontri prima di luglio richiedono fonte fixture completa e veri stati residenti. Documentazione/QA: [COMPETITION_DATES_AND_JULY_2026_CUTOFF.md](COMPETITION_DATES_AND_JULY_2026_CUTOFF.md). Audit generale di date di TUTTE le competizioni nei due mondi, inclusi preliminari delle coppe nazionali e Europee, obbligatorio successivamente. Distinguere questo catalogo dalla partita e dal CAR: build/test Windows 217 **NON PROVATI**. Un solo commit, mai altro ramo.

### 00.13.215 debug spazio — DETT e ARCH
Il F12 (solo lettura con cache 5 secondi) mostra la dimensione compressa reale sul filesystem, la dimensione JSON dopo decompressione, le partite OG ancora dettagliate + KB di record wire20, e partite riassunte + numero edizioni + KB di record wire24. Non stimare separatamente quanti byte compressi appartengono a una sezione: Zstandard livello3 produce UN SOLO frame per l'intero CAR. Il nuovo JSON è schema6 e legge precedenti schema5/4/3. Lo stato OG, i risultati e le qualificazioni sono comunque verificabili soltanto da test reali Windows.

## 00.13.215 — un CAR compresso Zstandard livello 3 (codice; Windows NON PROVATO)
I due slot `save/AUTO9697.CAR` e `save/AUTO2627.CAR` sono ancora file UNICI portatili. Nuovo header `SWOSZC3A` + frame Zstandard 1.5.7 livello 3; interni JSON schema5 invariati, checksum FNV64 raw, controllo lunghezza, OG/civile e rimpiazzo atomico del solo AUTO. I vecchi `.CAR` JSON sono letti in automatico, il vecchio AUTOSAVE.CAR non viene sovrascritto. Sorgenti Zstd compilati staticamente, licenza BSD/documentazione in `../../docs/ZSTANDARD/`, niente DLL/installazione utente. Hash-index partite si espande; alla stagione seguente il journal delle leghe chiuse e comprovate diventa archivio sintetico `og_resident.fixture_archive`, con ID, conteggio e hash e tombstone anti-replay; le coppe e qualificazioni non provate restano dettagliate per evitare perdita. F12 mostra BYTE del CAR COMPRESSO reale sul disco, JSON raw e COUNT fixture giornaliere salvate. Lo stato RAM 2028 perso nella Windows 214 NON e' ricostruibile e va ripreso dal CAR preesistente 2027. SAVE/EXIT/CONTINUE e la compilazione completa Windows restano test utente.

# Save Data — SWOS Expanded

## SWOS Expanded Windows — regola vigente dal 14/11/2026
Le sezioni storiche sottostanti su un solo `save/AUTOSAVE.CAR` descrivono le build precedenti. Attualmente ci sono due slot indipendenti accanto a `SWOS_Expanded.exe`: `save/AUTO9697.CAR` e `save/AUTO2627.CAR`. Il vecchio `save/AUTOSAVE.CAR` resta **solo leggibile come recupero compatibile**, mai sovrascritto o cancellato automaticamente. `SWOS.ini` e `SWOS.log` sono accanto all'EXE. Il percorso `Documents\\SWOS` vale per la base OpenSWOS upstream, non per Extended Windows. Ancora da completare e collaudare: salvataggio di tutti gli stati OG residenti, coppe, singoli risultati storici e ripresa pluristagionale.

## Windows portable mode — STORICO build 0.13.20

Windows SWOS Expanded stores everything beside `SWOS_Expanded.exe`:
`save/AUTOSAVE.CAR`, other `save/*.CAR`, `save/*.SRP`, tactics/edits/highlights,
`SWOS.ini` and `SWOS.log`. The EXE has already anchored cwd to its directory
before I/O starts. Use a **writable** folder (not Program Files).
If `save/` cannot be created/written, the game reports an error; it never
silently changes to Documents or AppData.

The native OpenSWOS legacy location (`%USERPROFILE%\Documents\SWOS\save`)
may also be used by another installation. The game does not automatically
remove any data there. After a successful portable save/load test, the user
may explicitly run `PULISCI_VECCHI_SAVE_DOCUMENTI.bat`, read the warning,
and type ELIMINA to delete only supported legacy save extensions there.
GitHub commits cannot remove files from the user's computer.

## Automatic career slot — STORICO precedente ai due mondi

`save/AUTOSAVE.CAR` uses the existing JSON .CAR schema 5 and Living World
snapshot. A normal save writes a temporary file in the same folder, flushes
it, and atomically replaces the old slot. The Title checks for the slot and
loads it when CONTINUE CAREER is chosen, including after a new process starts.
Checkpoints occur after first job, completed career matches/simulations,
safe career-menu exits, native midseason catch-up, and new season.
**EXIT GAME preserves the slot.** Final retirement deletes it. Explicit
"ABANDON CAREER" with confirmation needs a future UI implementation.
An unfinished live OG match is NOT saved mid-tick: if the program quits or
crashes during a manual match, the last stable checkpoint can be reloaded,
and that fixture can be retried. True anti-save-scum/in-match resume is
pending a compatible original-engine match-state serializer.

No new DLL or compiler, and no changes to match physics. Static database
files remain installed in data/ rather than copied into each career save.
Minimum delta-only world saves are an approved final target, NOT done in this
build. Windows compiler/runtime validation is pending.

---

# Save Data

Where SWOS stores your saves, what it stores, and how the path is chosen.

## Where saves live

Windows Expanded intentionally uses a **portable game directory**.
For other native platforms, SWOS still uses **per-user data locations** to
avoid writing inside read-only signed app bundles. A Windows portable install
must not be placed inside `C:\Program Files` or another protected folder.

| Platform | Save directory |
|----------|----------------|
| **Windows Expanded** | `<directory containing SWOS_Expanded.exe>/save` |
| **macOS** | `~/Library/Application Support/SWOS/save` |
| **Linux** | `~/.swos/save` |
| **Web (WASM)** | Browser IndexedDB, mounted at `/save` (see `wasm/pre.js`) |

`SWOS.ini` sits in the parent of the save folder (`./SWOS.ini` on portable Windows; `swos_ini_path()`); the Switch uses the flat `save:/` mount for both.

The directory is created automatically on first launch. On portable Windows
the directory is always `./save/` next to the executable; write errors
are reported rather than moving data elsewhere. Other native platforms
can fall back to a relative directory if their user folder is unavailable.

### Finding your saves

- **Windows Expanded** — open the `save` folder next to `SWOS_Expanded.exe`.
- **macOS** — Finder → **Go → Go to Folder…** → paste
  `~/Library/Application Support/SWOS/save`.
- **Linux** — `~/.swos/save` (a dotfolder; `ls -a ~` or show hidden files).
- **Web** — saves live in the browser's IndexedDB for the page's origin; there's
  no file on disk. Clearing site data for the page erases them.

## What's stored there

Every file the game reads/writes with a save-type extension resolves to the
save directory:

| Extension | Contents |
|-----------|----------|
| `.CAR` | Career |
| `.SEA` `.PRE` `.DIY` | Career-related overlays (season / pre-season / DIY competition) |
| `.TAC` | Custom tactics |
| `.HIL` | Match highlights |
| `.EDT` | Edited teams (e.g. `CUSTOMS.EDT`; also a SWOS-community interop format) |
| `.SRP` | OpenSWOS whole-match replay (`src/replay_v2.c`): `LATEST.SRP` after every match, plus a dated `MMDDhhmm.SRP` copy under `[replays] auto_save` |
| `.RPL` | PC (DOS / SWOS 2020) whole-match replay — still played by the highlights viewer (`src/dos_replay_load.c`), no longer written by new matches |

## Save format & versioning

- **Career saves (`.CAR`/`.SEA`/`.PRE`/`.DIY`) are JSON** (`src/save_format_json.c`),
  carrying a `schema_version`. This decouples the on-disk format from the runtime
  memory layout, so a save survives rebuilds and 32↔64-bit / LE↔BE.
- The loader verifies `schema_version`: build 0.13.20 writes schema 5
  and can read previous schemas 3/4. Unsupported schemas are rejected
  with the normal incompatible-save error. Older schema 3 careers
  cannot reconstruct a world ledger that was never stored.
- The small fixed-layout records keep their binary form: `.EDT` is binary;
  `.TAC` and `.HIL` are *written* as JSON by default (binary with
  `make SWOS_SAVE_FORMAT_JSON=0`) and *read* in either form, auto-detected — see
  `docs/SAVE_DATA.md` "Format migration / read-support policy".

## No bundled default saves

A fresh install ships with an **empty save directory** — no default career
or tactics files. On Windows Expanded, the first accepted contract creates
a portable `save/AUTOSAVE.CAR` checkpoint.

## Implementation

- `swos_save_dir()` returns the base save directory; `swos_save_dir_init()`
  resolves it once at startup (`src/main.c`, early, before `Settings_Load` reads `SWOS.ini` and before `SDL_Init`). Both live
  in `src/swos_io.c`.
- The path is consulted by **both** resolvers so load and save always agree:
  `resolve_save_path()` (reads, `swos_io.c`) and `resolve_path()` (writes,
  `resload.c`).
- Platform selection is compile-time (`_WIN32`/`__CYGWIN__`, `__APPLE__`, else
  Unix; `SWOS_WASM_BUILD` keeps the relative `save` that maps to the IDBFS
  `/save` mount; `SWOS_SWITCH_BUILD` uses `SDL_GetPrefPath`'s `save:/` mount).
  On macOS/Linux, when `HOME` is unset (e.g. a `.app` launch) the path falls
  back to `SDL_GetPrefPath(NULL, "SWOS")`.

## Amiga

The Amiga target (same source tree, `make -f Makefile.amiga`) does not use
`swos_save_dir()`: its `RobNorthenLoader` path (the `!SWOS_SDL_BUILD` half of
`src/resload.c`) writes to a relative `save/` folder in the game directory —
`bin/save` when FS-UAE mounts `bin/` as `DH1:`. The binary will not create that
folder, so it must already exist (the Amiga drop ships an empty one). This
document otherwise covers the portable SDL/WASM builds.


## Fix 0.13.21 — older fixtures from non-European calendars

The world's initial career clock is July 1, but the already-installed
OpenSWOS schedule for country 21 begins in **May** (native phase 32,
end phase 64). The v0.13.20 reader incorrectly rejected those
pre-July fixture rows. This was a loader-validation bug, not
corrupted autosave data or an incompatible `schema_version`.
The 0.13.21 loader accepts fixtures from January 1 of the
initial calendar year and still validates club identities, all dates,
ordering, compressed payload size and results. No schema migration is
required; previously saved `AUTOSAVE.CAR` is not overwritten by the fix.


## Living World v3 (0.13.22)
The single `AUTOSAVE.CAR` retains **external schema_version=5**, with
`living_world.version=3`, `window_first_year`, and compact `history`
rows: [season year, country, division, winner club ID, winner points,
matches played, deterministic 31-bit digest]. Results and unplayed
fixtures for the **current and next** season retain their original
compressed fixture data; old seasons are summarized before their
reusable 16-bit OG slot is used for another year. This avoids a
70-season resident fixture allocation and makes cumulative result
counts stay monotonic across archives. Older world v1/v2 data stays
readable. Exact individual scores from archived past seasons are NOT
retained in the summary; a richer per-match delta/event archive is
a future milestone.
The dedicated `TEST_LIVING_WORLD.bat` is an isolated 70-season stress
harness; it does not touch player `save/` and does not test native
Windows career state reconstruction, which remains a separate manual
smoke test. Athletic probability tuning is deferred for user approval.


## 0.13.24 — historical matches before first July
The original OpenSWOS May–September league schedules include matches
before a new manager begins on July 1. Previously those pre-career
fixtures were present but unplayed, so rolling out the first resident
season could fail when it attempted to archive an incomplete league.
The live engine now deterministically completes those **pre-initial**
fixtures on initial seal and when loading older snapshots, preserving
the global clock, native match engine and all future fixtures unchanged.
The test runner's mjson allocation override is isolated to test builds;
the real `AUTOSAVE.CAR` allocator/writer is unchanged.
