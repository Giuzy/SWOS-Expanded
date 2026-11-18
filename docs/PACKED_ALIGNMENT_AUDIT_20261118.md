# SWOS Expanded — Packed alignment audit, versione 0.13.89
18/11/2026. Base di confronto: dev/0.10 HEAD 2fef8fc26c1ab5d37e4a8e2b412f3450eb7210c3, versione 0.13.88. **Correzioni 0.13.89 incluse nel nuovo commit; compilazione e test Windows non effettuati qui.**

## Source of warnings
`DIAGNOSTICA_SESSIONE_1(20261118-070243).zip`, `DIAGNOSTICA_SESSIONE_2(9).zip`, `DIAGNOSTICA_SESSIONE_3(6).zip`, inside `COMPILAZIONE_0140.log`: 29 identical GCC `-Waddress-of-packed-member` warnings in each of the 3 builds.

| File | Instances | Audit / correzione 0.13.89 |
|---|---:|---|
| `src/save_format_json.c` | 12 | SeasonInformations ptr fields use aligned locals and byte-wise memcpy; name[] arrays of packed competition declarations use temporary aligned pointer arrays and memcpy; cup int32 offset table read by memcpy; readonly DIY base snapshot; loaded base fields read/write directly through packed parent; standings helper proof |
| `src/career_status.c` | 9 | Aligned copies for readonly base/stats, ptr-by-ptr memcpy for packed QualifyingGroup.compCup; auto-advance reads live packed parent (never a stale copy), debug comp-slot word memcpy |
| `include/diy_competition.h` | 5 | One helper included five times; static alignment assertions plus byte offset/stride for correctly aligned 2-byte DIYTeamStats |
| `src/menus.c` | 1 | Read packed parent directly, use unsigned bytes/BE_READ_U16 for team IDs |
| `src/statistics.c` | 1 | Snapshot base into local aligned DIYCompetition for readonly fields; FindTeamInDIY still receives original full OG context, its wider API is a separate deep-audit concern |
| `src/career_teams.c` | 1 | Read working OG base directly AFTER side-effecting callbacks |
| **Total** | **29** | Existing packed layout, algorithm, CAR format unchanged |

No blanket `-Wno-address-of-packed-member` suppression, no removal of packed attributes, no second simulation, no edit of existing save files. Adjustments are local to the access sites. This is **not proof** of global unaligned-pointer safety; e.g. `FindTeamInDIY` and older direct casts need separate targeted review.

## Test status
- `SI`: source locations verified against exact Windows logs; changes compared with 0.13.88; the 29 warning-triggering expressions were replaced in the six published source/header blobs. This does not establish a new count after full compilation.
- `SI (isolated only)`: a small C harness mimicking packed(2) fields/pointers, compiled on Linux using `cc -std=c11 -Wall -Wextra -Werror -pedantic -fsanitize=undefined,alignment`, output `PACKED_ACCESS_ISOLATED=PASS`.
- `NON PROVATO`: compile the full `SWOS_Expanded.exe` using Windows MSYS2, count updated GCC warnings, Windows gameplay, full CAR roundtrip, both worlds and compatibility with previous saves.
- `SI`: versione sorgenti 0.13.89 e commit unico richiesto dall'utente su `dev/0.10`. `NON PROVATO`: compilazione integrale, salvataggio e gameplay Windows.

`sorgenti/tests/test_expanded_packed_alignment.c` is included as an isolated regression check, using real `game_types.h` and its existing size/offset assertions.

**Windows checklist after GitKraken sync:** run COMPILA_SWOS.bat and AVVIA_DIAGNOSTICA.bat; retain originals of AUTO9697.CAR/AUTO2627.CAR; verify no new warnings/errors, old save restore and save in 96/26, START, league/cup advance, CLASSIC/DIY/SEASON and no regression of controller/match graphics.

Related: [DEVELOPMENT_WORKFLOW.md](DEVELOPMENT_WORKFLOW.md), [PROJECT_STATE.md](PROJECT_STATE.md), [ENGINE_CODE_DISCOVERIES.md](ENGINE_CODE_DISCOVERIES.md).
