# SWOS Expanded 00.13.193 — OG season transition / F12 performance

19 November 2026. Work branch: dev/0.10. Base HEAD: 613c6f323b6291b8b845b47c2efe46a4fcef2273 (00.13.192).
Scope: native OG calendar, CAREER only; no new calendar/simulation, no format change, no edits to CLASSIC/DIY/SEASON or netplay.

## Changes and scope

1. F12+9 introduces a separate, read-only performance page. F12+0 returns to OG integrity; F12 closes, F9 captures screenshots. Measurements are grouped by currently selected 96/97 or 26/27 world (no mixing when switching worlds). Samples contain number of calls, last/average/worst/total milliseconds.
2. Native OG world tick and per-family league/cup/international/overhead timings record in-session observations. The total unemployed FORWARD measures the real menu callback through its redraw. FINISH SEASON measures the complete manager callback; the legacy career rollover is measured separately. SAVE/LOAD wrappers time the actual JSON operations, including error returns. Boot 0–100 and frame rendering also provide observations, not gameplay certifications.
3. A resident league context may only contain valid team indices. The local manager's -1 index must never enter the resident index array; unexpected values are logged and cause a safe early return (rather than an out-of-bounds team access).
4. After manual OG rollover, re-read the civil date and require precisely 1 July of the requested year. The existing s_ogManualSeasonReady guard still protects the previous CAR if the native or civil state cannot be synchronized. This does NOT rewind a wrongly advanced clock.
5. During the old employed career year-end qualification callback, avoid the read-only league result menu. All original qualification, promotion/relegation and other callback instructions still run. This is not proof of zero redundant scratch work; F12 exposes remaining costs.

## Constraints and limits

- Resident OG is sole result/calendar authority. No fabricated matches, rankings, qualifiers or credits.
- Session timing data is not written to CAR. Existing AUTO9697.CAR/AUTO2627.CAR and legacy saves are not deleted or rewritten by these changes.
- The 1996/97 crash may have other causes besides an invalid resident team index; if the new guard fires, do not silently replace it with a different team.
- The existing national/international eligibility gaps, manager/CPU integration for non-standard competitions and multi-year saves remain subject to Windows tests.
- A performance sample is not a test verdict. The UI frame sample measures an actual Platform_RenderFrame call, including the render/present path, but excludes game tick and any later pacing sleep.
- The original manual season callback may remain expensive; F12 will distinguish OG world execution from legacy year-end work.

## Checks / release gate

| Check | Status |
|---|---|
| Source anchors and edited C/preprocessor nesting | SI, static inspection (baseline-comparison for career_init.c) |
| F12+9 timings and F12+0 health routing present in staged source | SI, static inspection |
| Independent timing samples per world | SI, static inspection |
| Resident offset preflight and no automatic result rewriting | SI, static inspection |
| No new CAR schema or new simulator | SI, static inspection of changed paths only |
| Full SWOS_Expanded.exe Windows build / zero warnings | NON PROVATO |
| User input, F12 rendering, keyboard and controller | NON PROVATO |
| Both manual year transitions 96/26, with actual dates | NON PROVATO |
| FORWARD TIME unemployed and employed | NON PROVATO |
| SAVE/CONTINUE roundtrip across both worlds | NON PROVATO |
| Historical crash and entire OG world correctness | NON PROVATO |

Suggested manual verification after GitKraken sync and a full Windows build:
- F12+9 before advancing: all not-yet-executed samples show N/D. Run unemployed FORWARD, open F12+9 and capture F9 plus SWOS.log.
- Repeat for 1996/97 and 2026/27, both unemployed and employed; FINISH SEASON should not show the repeated standings menus and should keep the date consistent with OG.
- Check "ROLLOVER LEGACY" vs "FINISH SEASON" vs "OG WORLD TICK" for the two-minute pause.
- Try SAVE/CONTINUE on both worlds and verify actual fixtures/scores, rather than trusting timing samples or CAR preflight alone.
- Attach three session diagnostics including any compiler warnings or new [og-resident-id]/[og-manual-season] messages.

Do not claim a compiled Windows executable or tested gameplay in this commit.