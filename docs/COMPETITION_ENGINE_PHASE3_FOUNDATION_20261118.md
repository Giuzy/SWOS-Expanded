## 18/11/2026 — 0.13.100: archiviazione per DATA DELLA GARA e test diagnostici
- Corretto il contratto di compattazione del journal: l'anno-stagione e' calcolato sul giorno effettivo della **singola gara**, NON sull'anno d'inizio dell'edizione. Una coppa iniziata a febbraio 2027 puo' avere partite sia nella stagione 2026/27 sia nella stagione 2027/28; queste ultime restano dettagliate fino al termine della finestra biennale.
- Prima di compattare: rifiutare risultati datati nel futuro, edizioni ancora in corso, conclusioni ancora future e fatti di qualificazione non confermati. Test aggiornato sulle partite di aprile/novembre e sulla protezione del calendario luglio-giugno.
- Il runner Windows diagnostico riconosce ed esegue due test isolati Competition Engine (scheduler e retention), riportando scope COMPETITION_ENGINE_ISOLATED, MA NON certifica gara reale, checkpoint CAR o gameplay.
- **OBIETTIVO VINCOLANTE DI QUESTO LAVORO:** poter aggiungere/importare competizioni OG, altre versioni SWOS e originali tramite definizione e regole parametrizzate, senza dover modificare codice specifico per ogni torneo; facilitarne diagnostica, calendario, qualificazioni, storico, salvataggi e nuove edizioni. Il collegamento OG e' un prerequisito, non il traguardo.
- Audit: nei residenti `Career_AutoAdvanceLeagueFixtures` e `Career_AutoAdvanceCupMatches` chiamano `DIY_PlayOrSimulateLeagueFixture` e `DIY_PlayCupMatchAndAdvance`, che gia' producono ed applicano risultati OG. In `Expanded_OGWorld_AdvanceUefa` il caso manager usa `PENDING_MANAGER_BIND` e NON deve essere sostituito con un secondo writer; per le coppe nazionali esiste `managed_excluded`. Nel binding campionato e' documentato il blocco quando locale/residente divergono. **Non esiste oggi un ID fixture comune verificato per entrambi i percorsi, ne' un CAR che checkpointi lo stesso risultato col nuovo scheduler**. Il gate 2d/2e/2f e quello della fase 3 restano APERTI; non attivare un nuovo dispatcher o millantare completamento.
- Stato consegna: modifica minima a codice isolato e runner, nessuna modifica ai callback OG, al formato CAR o al gameplay. Test Linux/Windows della nuova build e warning nuovi NON PROVATI fino a esecuzione. [Rapporto tecnico](COMPETITION_ENGINE_RECONCILIATION_GATE_013100_20261118.md).

# Competition Engine — fase 3: nucleo scheduler a eventi (18/11/2026)

Stato aggiornato 0.13.100: **NUCLEO SCHEDULER E CONTRATTO ARCHIVIO IMPLEMENTATI ISOLATAMENTE**, **INTEGRAZIONE LIVE/CAR NON IMPLEMENTATA**. [Regola due stagioni](COMPETITION_ENGINE_HISTORY_RETENTION_20261118.md).
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
