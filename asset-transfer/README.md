# SWOS Expanded — approved asset transfer rule

When the user approves an image asset generated for SWOS Expanded, upload it to GitHub without requesting separate approval. Preserve the approved pixels and format. Use branch `dev/0.10` unless the user specifies another branch.

Known tested single-upload ceiling: **3,145,472 bytes** (observed limit, not a universal GitHub platform limit). For safety split binary assets into chunks of at most **1,048,576 bytes** each, including when uploading BMPs larger than the ceiling.

For each approved asset:
1. Generate the exact final binary file and calculate its total byte count and SHA-256.
2. Split it into sequential binary chunks; upload each chunk using the GitHub Git Blobs API with base64 encoding.
3. Create or update a JSON manifest in `asset-transfer/manifests/` containing `asset_path`, `size`, `sha256`, and ordered `parts` with each chunk's Git blob SHA and size.
4. The `reassemble-assets.yml` GitHub Action fetches each Git blob, concatenates bytes in order, validates byte count and SHA-256, and commits the verified complete asset under `assets/`.
5. Verify the workflow result and final file before claiming completion. If it fails, report the failure, do not claim an asset was saved.
6. Avoid touching unrelated gameplay code, screens or assets.

This transfer process is automatic after approval, not a request to modify game code or to replace an existing approved asset unless that was explicitly approved.

## Why this workflow exists

The user should not have to download an approved asset from the chat and manually upload it again. Chunking accommodates the observed per-transfer ceiling; ordered manifests plus SHA-256 checks protect against incomplete or corrupted reconstruction. GitHub keeps the approved result available across chats, even when temporary local workspaces disappear. The upload rule is distinct from permission to modify gameplay code or other approved graphics.

For the full local-workspace, session handoff, verification and commit procedures **and the reason behind each**, see [`docs/DEVELOPMENT_WORKFLOW.md`](../docs/DEVELOPMENT_WORKFLOW.md).
