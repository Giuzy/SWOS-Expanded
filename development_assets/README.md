# SWOS Expanded — development assets

Organize approved artwork by the screen or function that owns it. This directory contains **source and approved reference assets**, not necessarily the paths used by the running game.

| Directory | Content |
| --- | --- |
| `branding/windows/` | Windows application/taskbar icon: `SWOS_EX_approved.png` (approved 1024×1024 transparent source) and `SWOS_EX.ico` (16, 20, 24, 32, 40, 48, 64, 96, 128, 256 px). |
| `title/` | Title screen artwork and references. |
| `career/` | Career creation and welcome/sacked screens. |
| `calendar/` | Career calendar UI and artwork. |
| `business/` | Club business screen and facility artwork. |
| `squad/` | Squad screens and player UI. |
| `training/` | Training UI. |
| `transfer/` | Transfer Market UI; keep approved UI and background as separate files. |
| `match_stats/` | Approved Match Stats menu artwork and UI references. |

Do not move existing runtime assets without updating and testing every code reference. Existing asset locations are left unchanged pending a separate inventory and migration. Keep approved assets in their own category and use descriptive, correctly spelled filenames.

The Windows build script `BUILD_SWOS.sh` reads `development_assets/branding/windows/SWOS_EX.ico`, copying it to `sorgenti/SWOS.ico` for `SWOS.rc` to embed into the executable. If the icon is missing, the build currently falls back to the old icon and logs a notice. **The two binary icon files must be uploaded to the paths above before this integration is complete.**
