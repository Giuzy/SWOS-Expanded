# 00.13.209 — Edizioni europee 1996/97 indipendenti; guardie identità gare
19 novembre 2026 — dev/0.10, parent a0b9306988abdfd522c017f29e058664b118d7cf, autore utente autorizza «prosegui e committa». Test Windows da utente, non eseguiti dall'assistente.

## Fonte
ZIP diagnostica reale `DIAGNOSTICA_SESSIONE_1(20261119-191040).zip` (00.13.208). L'utente conferma precampionato 96/26 funzionante. SWOS.log contiene 7 `[og-manager-calendar] BLOCK_PAIR_RECONCILIATION` con OG risultato reale e tabelle coerenti ma identità calendario cache non provata; 4 `[og-qual-next] ... FAIL_NEW_EDITION_MISSING` nelle coppe storiche: Champions 1997 o CWC 1997 + UEFA 1997 (due percorsi). In 2627 le tre coppe UEFA moderne hanno field 36/36 con confronto esatto, non modificate. Coppe domestiche 2627 riportano 192 `FAIL_OG_ACCESS_NOT_DERIVED` e 10 `FAIL_OG_FINAL_PLACES_UNAVAILABLE`: significa fonte ID/criterio assente o non verificata, non 202 squadre dimostrate errate. `OG_RESIDENT_SAVE_RELOAD=NOT_TESTED`: un controllo checksum in RAM non equivale a chiudere/riaprire.

## Root cause verificata nel sorgente
`career_status.c::Expanded_OGWorld_AdvanceUefa` prima richiedeva TUTTE E TRE le liste delle competizioni europee 1996/97 anche se quel ramo inizializza unicamente Champions tipo 2 e Copa Libertadores indipendente. Quando una lista CWC/UEFA mancava, la Champions con 16 ID propri già verificati era bloccata. `career_init.c::expanded_og_world_run_year` utilizzava un `classicReady` unico per CWC e UEFA Cup (due coppe tipo 1 separate); una lista mancante bloccava entrambe. La condizione all-or-nothing non è una regola OG, ma un gate Expanded.

## Fix autorizzato e perimetro
- `sorgenti/src/career_status.c`: per 9697 la Champions legge solo i propri 16 ID con `Expanded_OGWorld_QualReadNext`; moderna 2627 mantiene il gate esistente per tutti e tre i campi. Copa 96 mantiene validazione indipendente dei 20 ID.
- `sorgenti/src/career_init.c`: CWC slot1 e UEFA Cup slot2 ciascuna legge i propri 32 ID veritieri e valida prima di creare/proseguire la singola edizione; con fonte assente SALTA SOLTANTO quella coppa e registra `EDITION_PENDING`. Prima edizione OG nativa invariata. Nessun fallback ai seed dell'anno precedente, nessun elenco inventato e nessuna seconda simulazione.
- `sorgenti/include/expanded_qualification_transition.h`: helper C puro per campo storicamente atteso 16/32/32 (dopo che `QualReadNext` ha già verificato ID/anno/mondo/unicità).
- `sorgenti/tests/test_expanded_qualification_transition.c`: regressioni gate indipendenti per Champs=16,CWC=0,UEFA=32 e casi 0, errati e fuori-range (isolate; non sono prova del runtime).
- `career_status.c::Expanded_OGWorld_AuditManagerLeagueResult`: su `BLOCK_PAIR_RECONCILIATION`, logga tutti i gate reali (bridge, map, table, match counts, native/cache phase, active flag, local slot e risultato), senza modificare score né tentare correzioni rischiose. I 7 mismatch restano aperti finché la prossima ZIP non espone il gate esatto.

## QA e limiti
Controlli statici: sostituzioni ancorate a sezioni uniche, dipendenze `expanded_qualification_transition.h` già incluse in entrambe le unità, non cambia API pubblica, save format, JSON, DLL, engine 50 Hz, date, precampionato, CLASSIC/DIY/SEASON. Nessun duplicato di gare intenzionale; da verificare in Windows. Test `run_test_expanded_qualification_transition.sh`, GCC completo, warnings Windows, gameplay/edizioni residenti e cross-process CAR **NON PROVATI** finché user non esegue.
Non implementato: derivazione fonte OG per 202 controlli di accesso coppe nazionali; sette identità cache da riconciliare; claim univoco preventivo, scheduler dispatcher, transazione CAR atomica, tre edizioni consecutive, Libertadores 2627. La 209 **non** conclude il Competition Engine, A/B JSON 208 restano autonomi.

## Protocollo Windows
1. GitKraken -> compile 00.13.209; controllare errori e tutti i warning.
2. 9697: START disoccupato -> FINISH/30 giugno -> 1 luglio -> verifica `[og-qual-next]` per Champions / CWC / UEFA; ove campo vero non c'è, `EDITION_PENDING` individuale, non blocco delle altre. Gara del manager dopo luglio e assenza PANIC/replay.
3. 9697: percorso MANAGER FINISH identico, confronta per ID qualificati con la fonte OG. Non assegnare PASS per coppa mancante.
4. 2627: Champions/Europa/Conference 36 ID in nuova edizione, precampionato invariato; report `[og-manager-calendar]` guardie complete per eventuali mismatch.
5. Salva CAR prima e dopo, chiudi EXE, riapri CONTINUE per ENTRAMBI i mondi; confronta date, ID, risultati e fingerprint `[og-state-fp]` prima/dopo. Nessuna autosave dopo fault.
Checklist: codice preparato SI; compile Windows NON PROVATO; resident revised PASS NON PROVATO; manager identity fixes NO; domestic cups solved NO; CAR full roundtrip NON PROVATO.
