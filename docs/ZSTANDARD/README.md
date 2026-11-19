# Zstandard per i salvataggi CAR — SWOS Expanded

Libreria upstream: **Meta Zstandard v1.5.7**, https://github.com/facebook/zstd/releases/tag/v1.5.7

Licenza scelta: **BSD a 3 clausole**, testo originale in [LICENSE](LICENSE).
La DLL non e' necessaria: i sorgenti C di compressione e decompressione sono
inclusi in `sorgenti/third_party/zstd/lib` e compilati dal Makefile esistente.
Nessun download o installazione richiesti all'utente Windows.

Il progetto utilizza il livello **3**, approvato il 19 novembre 2026,
per privilegiare autosave rapidi. La libreria non calcola risultati ne'
modifica l'ordine delle gare OG.

Formato `SWOSZC3A`:
- magic 8 byte `SWOSZC3A`
- lunghezza originale JSON (LE32)
- lunghezza Zstd frame (LE32)
- checksum FNV-1a 64 su JSON decodificato (LE64)
- frame Zstd (dati effettivi)

Il loader convalida magic, lunghezze, frame Zstd, checksum e JSON OG/civile
prima di mutare i dati di carriera. Continua a leggere i precedenti `.CAR`
JSON schema 6 (lettura compatibile 5/4/3) senza compressione. Il CAR resta UNICO e PORTATILE.
L'atomicita' del rimpiazzo di AUTO9697.CAR/AUTO2627.CAR resta nel
`swos_io_save_atomic` originale, con file .tmp sullo stesso disco.

**Dati diagnostici:** dimensione compressed e raw e numero di record
fixture salvati a disco sono indicatori, non prove di completezza OG.
Non confondere la riduzione della dimensione con la regola di conservazione:
dettagli di una sola stagione corrente, eccezioni solo per competizioni
aperte e dipendenze ancora necessarie, sintesi edizioni chiuse. Nessuna
partita deve essere inventata o ripetuta.

**Prova Windows da fare personalmente:** avviare `AVVIA_DIAGNOSTICA.bat`
dopo GitKraken, controllare zero errori/warning, SAVE→chiusura processo→CONTINUE
in entrambi mondi, nuovi e vecchi CAR, date/partite/qualificati identici;
verificare il pannello F12 e i salvataggi in `save/`.
