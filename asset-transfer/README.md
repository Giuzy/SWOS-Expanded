# SWOS Expanded — metodo storico di trasferimento via blob/manifest (opzionale)

**Stato 2026-11-08:** la procedura normale per i nuovi asset frontend è **ZIP con percorsi definitivi**, da estrarre nella cartella con `AVVIA_SWOS.bat`, seguito dal push GitKraken dell'utente. `ASSETS_INBOX/` resta l'alternativa autorizzata. La vecchia ricomposizione GitHub Action descritta qui è disponibile come **ripiego tecnico**, non è più obbligatoria né parte di ogni approvazione. Non sovrascrivere asset approvati tramite manifest storici. I nuovi asset frontend sono PNG lossless; i BMP storici restano invariati.

Per usarla come ripiego, verificare prima il ramo attuale (normalmente `dev/0.10`), i formati, la necessità concreta e i binari già archiviati. I tre manifest di trasferimenti conclusi nel novembre 2026 sono stati rimossi durante la pulizia 0.10.53_03; gli asset definitivi restano su GitHub. Nessun push di asset viene presunto senza verifica.

Known tested single-upload ceiling: **3,145,472 bytes** (observed limit, not a universal GitHub platform limit). For safety split binary assets into chunks of at most **1,048,576 bytes** each, including when uploading BMPs larger than the ceiling.

**Opzione per trasferimenti tramite payload compresso (solo trasporto):** quando il collegamento non permette il passaggio diretto dei byte BMP, è consentito archiviare un blob Git `xz` e dichiarare `"compression": "xz"` nel manifest, con `encoded_size` e `size` originale. L'azione ricostruisce il BMP byte per byte, con verifica obbligatoria dello SHA-256 dell'originale. Non è una conversione dell'asset e il file finale resta il BMP nativo approvato. I manifest senza `compression` continuano a usare chunk grezzi senza modifiche.

For each approved asset:
1. Generate the exact final binary file and calculate its total byte count and SHA-256.
2. Split it into sequential binary chunks; upload each chunk using the GitHub Git Blobs API with base64 encoding.
3. Create or update a JSON manifest in `asset-transfer/manifests/` containing `asset_path`, `size`, `sha256`, and ordered `parts` with each chunk's Git blob SHA and size.
4. The `reassemble-assets.yml` GitHub Action fetches each Git blob, concatenates bytes in order, validates byte count and SHA-256, and commits the verified complete asset under `assets/`.
5. Verify the workflow result and final file before claiming completion. If it fails, report the failure, do not claim an asset was saved.
6. Avoid touching unrelated gameplay code, screens or assets.

This is a documented fallback, not the default transfer process. It must never overwrite an existing approved asset without an explicit replacement request.

## Why this workflow exists

The user should not have to download an approved asset from the chat and manually upload it again. Chunking accommodates the observed per-transfer ceiling; ordered manifests plus SHA-256 checks protect against incomplete or corrupted reconstruction. GitHub keeps the approved result available across chats, even when temporary local workspaces disappear. The upload rule is distinct from permission to modify gameplay code or other approved graphics.

For the full local-workspace, session handoff, verification and commit procedures **and the reason behind each**, see [`docs/DEVELOPMENT_WORKFLOW.md`](../docs/DEVELOPMENT_WORKFLOW.md).
