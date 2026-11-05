# SWOS Expanded

Private development repository for SWOS Expanded.

## Windows build

The workflow in `.github/workflows/windows-build.yml` builds with MSYS2/MinGW64 and SDL2 and uploads a portable Windows artifact.

**Current state:** The repository does not yet contain the full 0.10.38 game sources and assets, so the workflow is not expected to succeed until they are imported. Do not interpret a failed initial workflow as a game regression.

Expected root structure:
- `sorgenti/` (OpenSWOS-derived source and Makefile)
- `data/`, `assets/` and other original runtime asset folders
- `source_images/approved/` for approved transfer-menu artwork

The artifact is intended to contain `SWOS-Expanded.exe`, `SDL2.dll`, and the runtime data folders.

## Planned 0.10.39

Transfer 1 approved UI/background and navigation, preservation of existing transfer functionality, match statistics and squad popup fixes, plus later scouting and player development integration. These are **planned**, not yet implemented in this repository.

The original 0.10.38 archive remains the stable baseline.
