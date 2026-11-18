# SWOS Expanded 0.13.102 — risultati OG, controlli FORWARD, journal CAR e cache 8192
- Corretto anche il limite legacy `stride <= 256` dei fixup `cache_strides/cache_scores`: una singola voce con 30 partite occupa 370 byte. I nuovi controlli richiedono stride e conteggio record coerenti e mai oltre la capacità residua (8.192 byte); test isolato aggiunto per il caso 370 byte.

Data 18 novembre 2026. Ramo `dev/0.10`. Base HEAD `4caaa0361b662ccee04d3aca126d522f4961447a`.

## Scopo approvato
Questo lavoro serve a rendere possibile, in futuro, l'importazione di competizioni tramite manifest e regole dati senza dover intervenire in C per ogni coppa. **OG produce i risultati UNA sola volta:** il witness e' una copia osservazionale del risultato gia' calcolato e applicato. Non esiste un secondo calendario, simulatore LWE o orologio.

## Percorso di una gara
1. `DIY_PlayOrSimulateLeagueFixture`: dopo il risultato OG, gli effetti tabellari e il cache score, chiama `Expanded_OGWorld_ObserveFixture(0,...)`.
2. `DIY_PlayCupMatchAndAdvance`: dopo la scrittura di punteggi/rigori nel tabellone OG e prima dell'avanzamento del cursore, chiama `Expanded_OGWorld_ObserveFixture(1,...)`.
3. Se `s_expOGCurrent` o `s_expOGCupCurrent` e' attivo, l'origine e' il mondo CPU residente. Altrimenti si registra solo la vera partita del club allenato, mai un incontro CLASSIC/DIY/SEASON. I risultati del manager **non vengono applicati automaticamente** al residente: questo richiede identita' e side effects unificati, che restano da completare.
4. Ogni witness contiene tipo competizione, paese, slot/divisione, anno-edizione, fase OG, progressivo nativo, coppia club orientata, punteggio, variante turno/gamba, origine CPU/manager e **giorno civile di osservazione**. Quest'ultimo e' un limite superiore del tick, NON una data di kickoff inventata.
5. L'indice di identita' e' hash/open addressing su 131072 bucket: il replay della medesima chiave e' `TWO_WRITERS` o `SCORE_CONFLICT`. Non simula di nuovo: blocca la nuova scrittura CAR per proteggere il vecchio autosave. Piu' calendari/fasi con nomi simili non vengono equiparati per nome o posizione.
6. Per il manager il confronto `[og-manager-world-fixture]` legge la stessa coppia orientata nel journal CPU e distingue `CPU_FIXTURE_NOT_OBSERVED`, `SAME_SCORE_NEEDS_ID_RECONCILIATION`, `AMBIGUOUS_MULTI_LEG_OR_CYCLE` e `SCORE_OR_PHASE_DIVERGENCE`. NON chiama `CalculateViewResult`, NON muta classifiche/RNG.

## Avanzamento e controllo mondiale, anche DISOCCUPATO
`Expanded_UnemployedWait -> Expanded_OGWorld_AdvanceToDate` rimane il percorso canonico. `Expanded_OGWorld_FixtureTickBegin/End` fa il snapshot dei contatori OG campionati/coppe prima e dopo ogni avanzamento effettuato tramite `AdvanceToDate` e lo confronta con i callback OG registrati nello stesso tick.
- `[og-fixture-tick] status=MATCH_COUNTERS_AND_CALLBACKS_EQUAL` significa *solo* delte native campionati + coppe domestiche identiche ai witness; NON certifica il manager o tutte le UEFA.
- `NOT_RECONCILED` segnala una differenza da indagare, non finge PASS.
- `CROSS_SEASON_SNAPSHOT` e' distinto, perche' al 1 luglio i contatori domestici OG vengono reinizializzati.
- I salto debug 1 mese / 5 anni / 70 anni sono nello stesso percorso: nessuna simulazione aggiuntiva e nessuna data parallela.

## CAR ampliato e compatibile
- `sorgenti/src/save_format_json.c` prima di 0.13.102 salvava `career_scalars.team_selection_key_cache` con **1870** byte e applicava fixup cross-endian a stride e punteggi nei primi **1870**. Ora salva **8192** byte e i fixup usano 8192. Le entry attive devono superare il validatore bounded `ExpandedOGCache_Measure` prima di qualsiasi cambio al file.
- Reader accetta la vecchia array esatta da 1870 oppure la nuova da 8192. Prima di mutare il mondo, controlla byte numerici, numero entry, stride/limiti e che tutto il prefisso attivo sia incluso. Sul load di file vecchio azzera la coda; i score e stride continuano a essere reimpostati dai campi JSON esistenti. Un dato malformato blocca il load, non modifica il CAR esistente.
- La sezione opzionale `og_resident.fixture_journal` e' nel **medesimo CAR** con `format=1,record_size=20,count,partial,hash,hex`. Il wire e' endian-neutral. Writer e reader verificano hash FNV64, wire e chiavi duplicate; il preflight OG dell'intero JSON avviene prima di sostituire atomicamente il save esistente.
- Un CAR 0.13.101 senza `fixture_journal` resta leggibile se contiene lo stato OG valido, ma il journal parte con `partial=1`: **nessun risultato retroattivo inventato**. Ogni nuovo record segue l'avanzamento vero.
- La fase della singola gara di campionato viene fotografata **prima** di `DIY_AdvanceLeagueProgressAfterMatch`: OG puo' avanzare la fase proprio alla chiusura del turno. Il journal conserva dettagli della stagione corrente e precedente *secondo il giorno osservato*; gli stati OG e le qualificazioni preesistenti rimangono separati. Gli summary minimi di edizioni antiche previsti dal futuro Competition Engine non sono ancora generati da questo witness.

## Evidenza vecchi CAR / limiti
Nel ZIP diagnostico 18/11 ore 11:05:
- `AUTO2627.CAR`: cache JSON 1870 byte, 36 voci, 792 byte attivi; `og_resident` presente.
- `AUTO9697.CAR`: cache JSON 1870 byte, 22 voci, 484 byte attivi; `og_resident` presente.
- `AUTOSAVE.CAR`: cache JSON 1870 byte, 22 voci, 484 byte attivi; `og_resident` assente. Il formato legacy puo' restare recuperabile secondo i percorsi gia' esistenti, ma **non** e' una prova di ripristino OG valido.
L'espansione del CAR e' additiva e non prova da sola che ogni competizione abbia fixture datate.

## Verifica e criteri NON raggiunti
- Controlli C isolati su 1870, 8192, cache corrupt/ambiguous: sorgente test esteso, esecuzione Windows **NON PROVATA**.
- Full build Windows, warning 0, partita, fino a fine stagione, save/CONTINUE cross-process per 96 e 26: **NON PROVATI**.
- Il journal registra i risultati OG gia' finalizzati e confronta delte CPU: **codice integrato, comportamento Windows NON PROVATO**.
- Single-writer universale che prevenga l'esecuzione della stessa partita fra manager/CPU PRIMA del kickoff: **NO**; il witness puo' rilevare ma non risolvere un conflitto di precedenti callback.
- Coppe UEFA con manager, qualificazioni cross-edition, storico con champion/finalist, fixture kickoff datate e importazione generale: **NO**. Fase 3 NON e' dichiarata completa.

## Protocollo Windows
Backup esterno di `AUTO9697.CAR`, `AUTO2627.CAR`, `AUTOSAVE.CAR`. GitKraken `dev/0.10`, COMPILA_SWOS.bat (full build e tutti i warning), AVVIA_DIAGNOSTICA.bat. Su salvataggi DI PROVA in 96/97 e 26/27: START disoccupato -> FORWARD TIME piu' volte -> controllare `[og-fixture-tick]`; assumere un club -> almeno una partita campionato, calendario/DEBUG WORLD e `[og-manager-world-fixture]`; salvare -> uscire -> CONTINUE -> verificare `[og-fixture-journal] SERIALIZED/RESTORE`, score e assenza doppioni. Provare un passaggio 30 giugno -> 1 luglio. Se compare `TWO_WRITERS`, `SCORE_CONFLICT`, `WRITE_BLOCKED`, `LOAD_REJECTED` o divergenza, STOP, conservare i save e allegare log/ZIP.

