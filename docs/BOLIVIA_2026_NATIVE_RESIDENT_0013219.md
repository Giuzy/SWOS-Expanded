> **RETROSPETTIVA 20/11/2026:** questo file documenta esattamente i limiti della **00.13.219** (type2 OG e viewer senza esecuzione datata; attesa sorteggio e REGOLE a rischio). Non usarlo da solo come procedura definitiva. La storia completa e le soluzioni 220–225, le prove Windows e il metodo generalizzabile sono nel **[manuale Competition Engine](COMPETITION_ENGINE_IMPLEMENTATION_GUIDE.md)**. La cronologia originale rimane intatta.

# SWOS Expanded 00.13.219 — Copa Bolivia: native OG type-2 initial resident, group viewer and CAR

## Scope and reason

User requested "collega e committa" after the Windows 00.13.218 viewer displayed "COPA BOLIVIA: OG GROUPS NOT RESIDENT". The correct 2026 access rule was clarified and checked against the Bolivian Football Federation (FBF): all 16 professional first-division clubs enter the domestic cup **automatically**, no playoff/qualifying stage. The winner has Copa Libertadores Bolivia 3 (if already Bolivia 1/2, goes to best unqualified League club); cup runner-up gets Copa Sudamericana Bolivia 1 if not already Libertadores. Sources: https://fbf.com.bo/copa-pacena-2026/ and https://fbf.com.bo/temporada-2026-de-la-division-profesional/

## Implementation

- `career_status.c`: separate `s_expOGBolivia` `ExpandedOGLeague` native type-2 tournament resident. It is NOT stored in `s_expOGUefa[3]` (which is a fixed, existing CAR ABI). `Expanded_OGWorld_AdvanceBolivia` runs after Bolivia's original OG first-division league has been initialized; first edition 2026 checks that all sixteen seeded official draw IDs are members of that **same** division, exactly once.
- Creates `CompetitionTournament` type2 with original engine `InitCompetitionTeamSelectionFromAltTableHeader`, `InitDIYLeagueFromRecord`, `InitTeams` and OG's RNG; group 4×4, 8/4/2 KO type2 stage descriptors. Native first-edition source team IDs grouped A/B/C/D; no new results.
- `diy_competition.c`: group-stage hooks `BoliviaCup_OGStageDays`, `BoliviaCup_OGStageStart`, `BoliviaCup_OGPair`. The 64 fixture pairs from `bolivia_cup_sequence.h` are resolved to live OG team rows and written into `_diyCompetitionStart.leaguePhase.cal`. OG remains the only legal result/standings engine. There are 48 intra-group + 16 interseries matches in eight days. This **does not dispatch/simulate matches** until the exact real per-fixture dates and single-writer arbitration exist.
- `career_init.c` ticks this national type2 initializer after its domestic league and cup loops (never type1 old knockout). Old 10+6 bye bracket stays blocked.
- `career_status.c::exp_og_view_bolivia` now replaces the red "NOT RESIDENT" popup with the **read-only original type2 OG tournament viewer**, when its resident has been successfully initialized. Original parent record is a legacy type1 label only; not executed. Before the real draw date 22 July 2026, the viewer says the draw is pending, avoiding retroactive knowledge at 1 July cutoff. After 22 July it can show real groups and 0 played matches.
- CAR `og_resident.bolivia` optional separate, native state checksum same as other OG contexts, saved and restored without changing the old `league_size`, `cup_size`, `uefa` array length, or format=1. Older CAR missing field remains readable, with no fictitious results. At a July-time first edition before August the resident may safely initialize on a later tick. July rollover preserves it. The saved context includes OG tournament, roster, RNG and fixture schedule.
- `sorgenti/include/build_info.h` changes from 00.13.218 to **00.13.219**. No classic/DIY/season changes, and no scoring/ticket/qualification rewrite.

## Strict limitations

1. **NOT YET PLAYABLE / NOT FULLY CONNECTED**: the match-by-match genuine 2026 dates, home/away order and OG calendar event dispatch are still not verified. Native fixtures exist inside an OG type2 resident, but remain at 0 results; don't simulate six ordinary games instead of eight true ones.
2. **Manager match participation NOT CONNECTED**: existing `WorldCups_Plays` blocks the old type1 club slot; native type2 is not yet a manager-playable career slot. MUST reconcile manager and CPU to one OG fixture before dispatching.
3. **Knockout matches NOT EXECUTED**: group merit-to-QF + QF/SF/F two legs (14 games) and tie-break rules still need authenticated rules and native tests; native round descriptors alone aren't evidence.
4. **Later years' entrants NOT CONNECTED**: the rule is 16 CURRENT OG first-division clubs (promoted in, relegated out) and a new draw, rather than recurring first-year seeds. Only first 2026 edition is initialized. On year rollover, never generate phantom next-edition entrants, retain the uncompleted real edition.
5. **World and CAR Windows end-to-end test NON PROVATO**: requires the user's MSYS2 build, starting new world 2627, career offered / unemployed, DEBUG WORLD after 22 July, club matching, SAVE/EXIT/CONTINUE across different processes, subsequent weekly FORWARD, plus regression world9697 and other cups. Older CAR's optional field parser was updated; preservation of previous saves must be proven, not presumed.

## QA

| Verification | State |
|---|---|
| Verified HEAD before changes | SI: dev/0.10, 7cc3988, version 00.13.218 |
| Official cup access rule: exactly all 16 OG first-division clubs | SI, FBF source; coded for first edition |
| Native type-2 structure distinct from UEFA + 64 pair stage hooks | SI, code authored; not built in Windows |
| Save optional Bolivia context, native pointer rebase and retention | SI, code authored; SAVE/LOAD untested |
| Previous fake knockout blocked | SI, already in 218 |
| Real group fixtures dated and executed by original match engine | NO |
| Manager plays real Bolivian cup fixture exactly once | NO |
| 14 KO matches, international qualification | NO |
| Later-year 16 participants and new OG draw | NO |
| Native Windows compilation and gameplay 00.13.219 | NON PROVATO |
| Cross-process save/continue and no double match | NON PROVATO |

Expected Windows log `[og-bolivia-resident] INITIALIZED ... type=2 ... played=0`, in DEBUG WORLD after 22 July `[og-bolivia-view] NATIVE_OG_TYPE2`; before the draw, `COPA BOLIVIA: DRAW ON JULY 22`. If `INITIALIZE_REJECTED`, upload the build/diagnostics ZIP and do not claim gameplay. Zero OG scores until dated routing is deliberately expected.

## Engineering rationale (compact checkpoint)

The native type1 Cup ABI cannot be changed to type2 by flipping the record's type. The original national knockout registration selects 10 clubs plus six byes and its manager/save/type1 parser cannot legally play interseries group games. The native OpenSWOS **type2** engine already understands round-table standings, group sorting, knockout slots and both automated/manual result paths. So 219 attaches a separate national OG type2 resident and injects only fixture pairs, with an optional CAR context and read-only viewer. It declines to dispatch fixtures prematurely. The next change must attach authentic dates and manager arbiter, then complete the 14 KO ties and 2027 entry from live OG league promotions. No second engine, no invented scores.

