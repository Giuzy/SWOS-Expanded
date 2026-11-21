**NON-NEGOTIABLE USER RULE (21/11/2026):** NEVER replace the requested native OG feature or interaction flow with an unapproved workaround, partial simulation, different UI button, shortcut, fake visual flow, or artificially restricted native loop just because the correct implementation is difficult. Report blockers transparently, ask before any compromise, and leave the item OPEN rather than committing a substitute. See docs/AGENTS.md and RG-014 case history. Failed preparatory blobs are not authorized fixes.


# SWOS Expanded — entry point for coding agents

This is a **pointer**, not a second copy of the rules. Applicable to the whole repository, including Roo Code and other coding agents.

**READ BEFORE CODE:**
1. [docs/AGENTS.md](docs/AGENTS.md) — binding collaboration, OG invariants, branch and commit policy.
2. [docs/DEVELOPMENT_WORKFLOW.md](docs/DEVELOPMENT_WORKFLOW.md) — mandatory pre-change impact analysis, test levels, and checkpoint procedure.
3. [docs/REGRESSION_HISTORY_AND_DEPENDENCIES.md](docs/REGRESSION_HISTORY_AND_DEPENDENCIES.md) — historical bugs, failed approaches, file→dependency→test lookup.
4. [docs/PROJECT_STATE.md](docs/PROJECT_STATE.md) and [docs/ENGINE_CODE_DISCOVERIES.md](docs/ENGINE_CODE_DISCOVERIES.md) — current proven state and code contracts.
5. For competitions, [docs/COMPETITION_ENGINE_IMPLEMENTATION_GUIDE.md](docs/COMPETITION_ENGINE_IMPLEMENTATION_GUIDE.md); for CAR, [sorgenti/docs/SAVE.md](sorgenti/docs/SAVE.md).

**Interrupted/new-session SWOS work:** FIRST read the entire [GitHub Issue #1 — operational recovery deposit](https://github.com/Giuzy/SWOS-Expanded/issues/1), including prepared Git object SHAs, *before* assuming a patch is lost or starting again. Verify branch/HEAD/version afterwards. Update this same Issue **immediately after each distinct prepared file, analysis, test, check, decision, blob or commit**, before preparing the next; never overwrite earlier references.

**Before EVERY change (code, data, tests, scripts, assets, UI or docs), and again after discovering a bug/problem:** open the relevant cases and dependency map in `docs/REGRESSION_HISTORY_AND_DEPENDENCIES.md`, check historical failed attempts, writers/readers, shared invariants and RG-T regression gates. Explain cause and evidence (VERIFIED / HYPOTHESIS / OPEN), what could break in OG calendar, fixture, manager, qualifications, both worlds, CAR and UI, and which regression checks are planned.

**Every newly discovered bug, dependency or implementation lesson:** record it immediately in the topic-specific guide and in the canonical regression register when applicable, with source, affected code/data, evidence level, tests, limits and cross-references. Deposit the prepared documentation SHA in Issue #1 before doing more work, and include the updated guides with the related code/build in the single user-authorized commit. Never leave the finding only in chat.

Do not silently adopt game-design choices. No second match simulator or calendar; no unsupported PASS. Preserve 50 Hz, OG classic modes and existing saves. Do not modify `main`, release or backup branches; **never commit without explicit user permission**. A Git blob, a commit, a Windows build and a gameplay PASS are four different things.

For Roo Code: open this repository as the workspace root and ensure project `AGENTS.md` loading is enabled in your installed client. This entrypoint does not claim that any agent actually read the linked files: verify their contents before each change.
