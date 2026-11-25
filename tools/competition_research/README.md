# Competition research — batch collector, version 1
Run **AVVIA_RICERCA.bat** in the repository root. It uses Python 3.12 in a local virtual environment (installs Python via winget per-user if absent, then dependencies from PyPI). Internet is required for first setup and optional GitHub downloads. No new game toolchain.
 
## Inputs (read-only, in repository root sources/)
1. corpus.zip
2. games.parquet
3. Matches.txt
4. worldfootballR_data-master.zip
5. all-euro-data-1996-1997.xls
6. all-euro-data-2026-2027.xlsx

Missing files are reported, not fatal. ZIPs can remain ignored by Git; do not push large archives. FootballCSV, Schochastics, withqwerty and linked football.json, xgabora, openfootball/world are attempted in priority order after local files; some repositories only supply directories/links or unsupported formats. Each cached source is hashed; a failed download or unknown schema is logged. Remote copies may duplicate local evidence and are not independent corroboration. Cache covers remote downloads, not incremental parsing; each run rebuilds evidence from sources. Archive parsers are initial adapters, not comprehensive RSSSF/Football.TXT grammars.
 
## Outputs
Send the newest tools/competition_research/output/RICERCA_*.zip to ChatGPT (if a run fails, send ERRORE_RICERCA_*.zip or bootstrap.log). It contains summary.json, source_manifest.json, source_log.json, SQLite evidence, matrix_candidates.csv, source_groups.csv, native_join_candidates.csv, cross_source_exact_pairs.csv, observed_changes.csv, observed_membership_changes.csv, july1_candidates.csv, same_source_score_conflicts.csv, rule_candidates.csv.
 
The matrix includes all indexed inventory rows, existing certified statuses when explicitly audited, candidate source-group counts and closure priority. Unknown fields remain unknown; no candidate changes existing certification. Dates inferred from edition headers, mixed season buckets, duplicates, possible names, exact-label joins and possible movement remain candidates. No invented rules, no actual post-2026-07-01 scores in fixture columns, no real future winners imported to game, no inferred ranking or league access rules. Historical results are evidence only (never write to OG, runtime data, CAR, viewer or existing research JSON). Calendar-year groups are not automatically seasons; roster changes do not prove promotion/relegation. July candidate file identifies groups spanning cutoff, not a certified historical league table. Exact source overlap is not proof of independent sources or verified match identity. Text excerpts require manual review.
 
## Options
- AVVIA_RICERCA.bat --self-test (synthetic core and SQLite candidate test)
- AVVIA_RICERCA.bat --no-download (local sources only)
- AVVIA_RICERCA.bat --refresh (refresh downloaded source archives)
 
No source archive is uploaded automatically. The user shares the results ZIP manually. Output, downloaded cache, venv and Python bytecode are ignored by Git.
 
Initial version: full Windows execution, dependency installation and complete archive coverage NOT yet demonstrated; review source_log and output before any acceptance. No automatic closure or integration.
