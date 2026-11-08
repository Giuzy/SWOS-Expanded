# Title Menu — PNG approvato (build 0.10.52)

Asset richiesto dal renderer: `assets/title/title_background_approved_2026-11-08.png`.

- Immagine approvata: **prima versione senza tabellone** del 2026-11-08.
- Generazione: 1670×941 pixel (**avviso:** 2 px meno dello standard nativo 1672×941).
- Crop senza resize: **1664×936**, PNG RGB lossless.
- SHA-256 del file finale: `34ab54c7548f546ee2cc531be54a5ad366ec4bf35dba4fab2e2010c1e6c4d9b5`.
- Lo sfondo si disegna sotto le reali voci/focus del Title Menu; **F8** alterna PNG/RAW.
- Il PNG non va ricreato dal preview: deve essere trasferito **byte per byte** tramite blob a blocchi ≤ 1 MiB, manifest e Action secondo `asset-transfer/README.md`.
- Se il PNG non è presente, è malformato o non misura 1664×936, il gioco mantiene il vecchio titolo.
- Il logo HD separato e l'opzione video nel gioco non sono ancora implementati.

**Trasferimento binario in sospeso:** non considerare l'asset archiviato finché il binario non compare su questo ramo e lo SHA-256 non è verificato. La build di codice 0.10.52 può essere sincronizzata indipendentemente.
