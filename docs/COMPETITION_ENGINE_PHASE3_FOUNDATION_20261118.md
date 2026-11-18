# Competition Engine — fase 3: nucleo scheduler a eventi (18/11/2026)

Stato aggiornato 0.13.99: **NUCLEO SCHEDULER E CONTRATTO ARCHIVIO IMPLEMENTATI ISOLATAMENTE**, **INTEGRAZIONE LIVE/CAR NON IMPLEMENTATA**. [Regola due stagioni](COMPETITION_ENGINE_HISTORY_RETENTION_20261118.md).
Ramo dev/0.10. La fase 3 non e' completa nei gate della TODO architetturale.

## Funzionalita' effettivamente implementate
- sorgenti/include/expanded_event_scheduler.h: scheduler C header-only per un singolo cursore civile e un array di eventi con ID stabili fornito dal chiamante.
- Eventi con data civile, categoria, priorita', ID, dipendenza e stato PENDING/COMPLETE; ordinamento deterministico giorno, priorita', ID.
- Next(target) salta direttamente al primo evento non completato oppure al target senza iterare sui giorni vuoti; una dipendenza aperta blocca i successivi.
- Schedule respinge ID duplicati con definizioni diverse, e' idempotente e rifiuta i nuovi eventi retroattivi. Complete autorizza solo il primo evento pronto ed evita doppio completamento.
- ValidateRestore controlla in sola lettura ordinamento, identita', date, stati, dipendenze degli eventi completati e cursore.
- Test C isolato con il compilatore Linux locale e piccolo stub data: PASS_ISOLATED_ONLY con -Wall -Wextra -Werror -pedantic. Script repository con vera libreria Gregorian, build Windows e gameplay NON ESEGUITI.

## Limiti e gate NON completati
- Nessun dispatcher live OG: il vecchio Expanded_OGWorld_AdvanceToDate e' ancora il percorso delle partite. Collego il dispatcher solo dopo che i punti 2d/2e/2f avranno un solo writer manager/CPU, ID reali e CAR transazionale. Nessun risultato duplicato introdotto.
- Nessun nuovo campo CAR: il nuovo Complete rappresenta soltanto un ack; match, effetti OG, fixture journal, RNG, evento e save vanno resi atomici dal futuro chiamante. Non e' un nuovo processo di save.
- Nessuna fase OG inventata da date civili, nessuna simulazione LWE, nessuna partita OG, coppa cross-year, UI, RNG o CLASSIC/DIY/SEASON modificati.
- Gate 3 aperto: risultati, RNG, fixture e classifiche identici al CONTINUE cross-process; nessun reset di coppe attive al 1 luglio; test Windows entrambi i mondi.

## Correzione precedente inclusa nella build
OG_CUP_START_COVERAGE legge ora solo [og-cup-start] domestico (definitions/resident/unsupported/failed) anziche' il totale di tutti i contesti OG. Non scambia piu' le 51 coppe domestiche 1996/97 con le 53 includenti le due coppe storiche extra; se il log scoped manca rimane NOT_TESTED.

## Conflitto di autorita' da risolvere prima dell'integrazione live
Le istruzioni storiche definiscono OG unica autorita' del mondo; il piano architetturale del 18/11 consente Expanded unico gestore calendario mondiale con algoritmi OG. Il nucleo e' compatibile con entrambe le opzioni poiche' non esegue gare. Non migrare il writer live senza decisione esplicita e gate di persistenza.
