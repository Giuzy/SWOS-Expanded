# Rose reali 2026/27 — archivio integrativo dati (17/11/2026)

Origine: **OpenSWOS-World-2627.zip** ufficiale caricato dall'utente.
Pacchetto derivato: aggiunge soltanto file in `data_json/worlds/2627/squad_extras/`,
NON sostituisce `teams/` o `competitions/`, non tocca 1996/97 né i save.

Ogni record extra conserva il giocatore reale, la sua UUID originale e tutti i
campi come nella fonte; il record sorgente contiene l'UUID del club, il checksum
dei **16 giocatori base**, l'ordinale e il nome. In fase di implementazione il
loader deve confrontare **UUID + checksum** con il record Expanded, e scartare
quelli non corrispondenti. Non mappare per semplice ordinale.

**I dati preparati non funzionano ancora in gioco senza il nuovo loader e la
serializzazione dello stato dei giocatori extra.** Non usare il patch preliminare
come sostituto della gestione mercato/cambi club/salvataggio. Le versioni con
licenze diverse non vanno confuse: il codice upstream è MIT, i dati calcistici
restano attribuiti ai rispettivi autori/contributori della Mod.

Per importare in Expanded: verificare prima la corrispondenza completa dei club,
quindi integrare nel codice il caricamento `squad_extras`, il tracking dei giocatori
ceduti, il mercato 17..30, il salvataggio CAR e il caricamento compatibile, senza
modificare la logica OG delle partite. Un test statico dell'archivio non prova
la compilazione Windows o il gameplay.
