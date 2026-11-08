# Title Menu — PNG approvato (build 0.10.52)

Asset richiesto dal renderer: `assets/title/title_background_approved_2026-11-08.png`.

- Immagine approvata: **prima versione senza tabellone** del 2026-11-08.
- Generazione: 1670×941 pixel (**avviso:** 2 px meno dello standard nativo 1672×941).
- Crop senza resize: **1664×936**, PNG RGB lossless.
- SHA-256 del file finale: `34ab54c7548f546ee2cc531be54a5ad366ec4bf35dba4fab2e2010c1e6c4d9b5`.
- Lo sfondo si disegna sotto le reali voci/focus del Title Menu; **F8** alterna PNG/RAW.
- Il PNG non va ricreato dal preview: deve essere trasferito **byte per byte** tramite blob a blocchi ≤ 1 MiB, manifest e Action secondo `asset-transfer/README.md`.
- Se il PNG non è presente, è malformato o non misura 1664×936, il gioco mantiene il vecchio titolo.
- [0.10.53 — IMPLEMENTATO NEL CODICE, DA TESTARE SU WINDOWS] Logo **PNG RGBA indipendente**: `assets/title/swos_expanded_logo_approved_2026-11-08.png`, canonico 1200×440, con ® rimossa, sopra il menu originale solo in modalità PNG.
- [POSIZIONE C APPROVATA] Sullo sfondo 1664×936 il logo è visualizzato a **x=920, y=17, larghezza=620, altezza=227**, senza coprire lo stadio. Coordinate scalate proporzionalmente al renderer; gli hitbox SWOS non si spostano.
- SHA-256 locale del logo: `a194160cdbc9a4d3315b926b8dbee75df2dea9b7aac1d92433929279639f6b96`. Il PNG finale è RGBA lossless, 1200×440; è stato ricavato dall'artwork generato a 2071×759 e poi ridotto, senza dichiararlo 1200×440 nativo. Il vincolo 1672×941 → 1664×936 si applica agli sfondi full-screen, non ai loghi/sprite.
- L'opzione video in-game resta futura e non implementata. Se manca il PNG del logo, il nuovo sfondo continua a caricarsi senza crash; F8 permette il confronto con il vecchio sfondo e disattiva anche il logo nuovo.

**Trasferimento binario in sospeso:** non considerare l'asset archiviato finché il binario non compare su questo ramo e lo SHA-256 non è verificato. La build di codice 0.10.52 può essere sincronizzata indipendentemente.
