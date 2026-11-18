# Competition Engine — 0.13.100: retention sicura e gate di riconciliazione OG
Data: 18 novembre 2026. Branch: dev/0.10. Versione codice 0.13.100.

## Scopo di prodotto
**Il fine non e' costruire una seconda simulazione.** Vogliamo poter IMPORTARE ogni torneo degli SWOS storici/mod o DEFINIRE un nuovo torneo usando solo dati, calendario e regole comuni, senza patch C dedicate per ciascuna coppa. Il criterio di accettazione finale e' una coppa reale che completa tre edizioni, con qualificazioni derivate, fixture per ID, manager/CPU, save e CONTINUE; poi secondo formato indipendente.

## Modifica implementata
- `expanded_competition_history.h`: il filtro di conservazione usa `ExpandedFixtureDetail.played_day`, non `ExpandedEditionKey.start_day`; l'anno civile della singola partita viene convertito nella stagione luglio-giugno. Blocco preflight per risultati nel futuro, edizioni non chiuse, chiusure successive all'oggi e fatti di qualificazione non protetti.
- `test_expanded_competition_history.c`: due gare della medesima edizione con date aprile 2027 e novembre 2027, verificando che al 1 luglio 2028 resti la gara del novembre (stagione 2027/28) e che sia compattata solo dal 1 luglio 2029; controllo sui future dates. Coppa febbraio 2027 / chiusura agosto 2029 deve rimanere intatta prima della chiusura.
- `run_all_diagnostic_tests.sh`: gli script `run_test_expanded_event_scheduler.sh` e `run_test_expanded_competition_history.sh` passano da NOT_RUN_UNCLASSIFIED a esecuzione esplicita con scope `COMPETITION_ENGINE_ISOLATED`. Un loro PASS non prova il save CAR o il gameplay Windows.

## Audit codice — perche' NON collegare due writer ora
- `career_status.c::Career_AutoAdvanceLeagueFixtures` chiama `DIY_PlayOrSimulateLeagueFixture`, che calcola il risultato OG, aggiorna classifica e match count. `Career_AutoAdvanceCupMatches` richiama `DIY_PlayCupMatchAndAdvance` che applica risultato e avanzamento del tabellone. I loro effetti non si possono rieseguire semplicemente.
- `career_status.c::Expanded_OGWorld_BindClub` copia una vista residente nel menu locale; `Expanded_OGWorld_DebugCanAdvanceClub` attualmente blocca un salto mensile se la classifica o i match count divergono. NON esiste ancora merge per-fixture del risultato del manager nel contesto residente.
- `Expanded_OGWorld_AdvanceUefa` rileva il club allenato e non simula il contesto residente per evitare un secondo match. Il flag `managed_excluded` sulle coppe nazionali ha analogo scopo. Sono precauzioni, NON prova di un registro comune.
- `save_format_json.c` scrive `og_resident` con snapshot native checksummate nella stessa CAR e fa preflight prima della write atomica: esso non contiene pero' ancora `ExpandedFixtureLedger`, `ExpandedEventScheduler` o `ExpandedCompetitionHistory`. Il format CAR NON viene cambiato da 0.13.100.
- **Conseguenza:** agganciare lo scheduler come chiamante di routine OG oggi consentirebbe duplicazioni oppure blocchi del manager; agganciare solo un journal in RAM senza restore/reconciliation CAR offrirebbe false garanzie. NON farlo per considerare la fase 3 completa.

## Prossimo gate tecnico (PENDENTE, necessario prima di attivare nuovi tornei)
1. Mappare una fixture OG dalla struttura di competizione originale in un ID stabile immutabile: mondo, competizione, edizione, fase, gara/andata/ritorno/replay, club reali. Ricondurre locale manager e CPU/residente allo STESSO ID prima di qualsiasi chiamata che muta OG. Non usare solo `compId`, team ordinal, partita #N o fase mensile.
2. Un solo proprietario/claim per fixture con rollback completo se simulazione non e' iniziata; se un effetto OG e' iniziato, bloccare eventuale riesecuzione fino a checkpoint o rollback verificato. Un conflitto ferma il mondo, non sceglie uno score.
3. Salvare in **un unico CAR atomico**: version/data, registri ID, fixture journal per ultime due stagioni, summary vecchie, pending events, RNG reali, OG resident/side-effects e fatti di qualificazione. Preflight read-only e restore senza dispatch OG. Mai aggiornare il save legacy automaticamente.
4. Test golden isolati con partite reali, stesso giorno e stessa fixture manager/CPU, cross-year, roundtrip CAR cross-process sui due mondi, post-hire, no doppio score e nessuna regressione CLASSIC/DIY/SEASON.
5. Solo dopo questi gate dichiarare fase 3 CHIUSA e procedere con resolver qualificazioni e coppa pilota.

## Stato e prove
- Modifica sorgente alla compattazione: SI (codice preparato per test).
- Script diagnostici attivati: SI (da eseguire).
- Compilazione C Linux/Windows di questa build: NON PROVATO.
- Warning GCC 0.13.100: NON PROVATO, contare dal prossimo rebuild completo.
- Single-writer OG: NO.
- CAR del nuovo scheduler/registro: NO.
- Zero double-play con manager: NON PROVATO.
- Fase 3 completata: **NO**.

Rispettare la checklist Windows SI/NO/NON PROVATO, i vecchi CAR, la partita OG 50 Hz, e non introdurre un secondo LWE.
