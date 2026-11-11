# Save Data — SWOS Expanded

## Windows portable mode — build 0.13.20

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

## Automatic career slot

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
