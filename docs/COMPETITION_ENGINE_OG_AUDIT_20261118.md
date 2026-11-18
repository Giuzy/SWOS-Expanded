# Competition Engine — audit OG iniziale e scelta pilota (18/11/2026)

**Ramo e base osservata:** `dev/0.10`, HEAD `b5758c41b7bd03b684b6de4fe46db374dcfbec08`, versione `0.13.87`.
**Natura:** analisi statica dei sorgenti e confronto documentale con OpenSWOS; nessuna nuova architettura implementata, nessun codice toccato, nessun test di compilazione o gameplay Windows. **Non dichiarare completato il gate 1**: rimangono l'audit completo delle chiamate, i golden test e la conferma degli ingressi del pilot.

## Esito e scelta

**Pilota proposto: Champions League OG storica nel mondo 1996/97**: `_euroCup` (compId 0, `CompetitionTournament` type 2) con 16 club e quattro gironi da quattro, seguiti dal tabellone KO. Il record OG originale e i 16 partecipanti sono in `sorgenti/src/competition_data.c:206-230`. Lo stato residente della prima annata usa `career_status.c::Expanded_OGWorld_AdvanceUefa()`, `s_expOGUefa[0]`. La fonte delle qualificazioni storiche e' `career_eurocups.c`, insieme a `Career_AdvanceToNextSeason_UpdateCompetitions()`. Consente di testare diverse leghe di origine, gironi, knockout, vincitore e seconda/terza edizione senza dipendere dalla complessita' simultanea delle tre competizioni UEFA moderne.

**Nessuna assunzione implicita sui formati 1997/98 e 1998/99.** Il database 96/97 fornisce la prima edizione OG; occorre stabilire con l'utente se nei decenni del mondo storico si mantengono le regole del *modello 1996/97* oppure si applicano evoluzioni storiche anno per anno, recuperando i dati verificati per ogni anno. Non chiamare le nuove edizioni "storicamente esatte" senza questa decisione. Non forzare una formula moderna nel mondo 96.

**Gate dati preliminare:** le quattro discrepanze originarie TEAM Finlandia/Francia/Turchia/USA sono documentate come **corrette nella 0.13.69** (`docs/SWOS_CROSS_VERSION_COMPETITION_AUDIT_20261117.md`), MA il preflight `NATIVE_DATA_FORMAT_MISMATCH` e' ancora in `career_status.c:4475-4507`; non e' prova Windows di 3 anni e delle posizioni qualificanti. Verificare le classifiche reali dei campionati europei necessari prima di costruire il campo 1997/98, senza ricavare club da un campionato `UNSUPPORTED`.

**Secondo formato, dopo il pilota:** Coppa delle Coppe OG 1996/97 (type 1, 32 ingressi); necessita' di vincitori reali di coppe domestiche e di regole per doppiette/detentori. Se il primo pilota e' bloccato dai dati delle classifiche, usare una **coppa domestica OG** solo per provare le API fixture/journal, senza dichiarare superato il gate di qualificazioni continentali.

## Matrice OG: RIUSA / ADATTA / NON CHIAMARE SUL MONDO LIVE

| Sorgente/funzione | Categoria | Cosa produce o modifica | Condizione di riuso |
|---|---|---|---|
| `competition_data.c::_euroCup` | RIUSA dati | Record storico type 2, 16 seed, struttura gironi/KO | Convalidare IDs, date e format per epoca/edizione |
| `career_eurocups.c::qualifyTeamForEuroCup` e `Career_ProcessCompetitionForEuropeanCupQualification` | ADATTA | Legge finalisti/leghe correnti dai buffer `_primaryQualifiedTeamGrids`; aggiorna liste globali, detentore e allocazioni TMD | Solo da snapshot di classifiche residenti, senza rielaborare match |
| `career_eurocups.c::Career_DrawEuropeanCupTeamsAndCopyGrids` | ADATTA | Gestisce detentore, rimpiazzo/estrazione `Rand2`, pool e liste | Isolare il RNG e la mutazione dei seed; salvare sorteggio |
| `career_season.c::Career_AdvanceToNextSeason_UpdateCompetitions` | **NON CHIAMARE LIVE** | Auto-avanza coppe e nazionali, elabora campionati e qualificati, richiama `UefaQual_Fill`, `Conti_SeasonEnd`, `WorldCups_SeasonEnded`; verifica i vecchi vincoli TMD (`0x50`, 16/32/32 o 36/36/36, panic `0x8998/8999`) | Estrarre una fase *read-only input di fine anno* e componenti dati; non eseguire l'intera funzione per chiedere i qualificati |
| `diy_competition.c::DIY_PlayCupMatchAndAdvance` | ADATTA | Simula o avvia partita giocabile, salva gol/aggregate/penalty, avanza la fase e cambia `competitionFinished`; muta buffer condivisi e aggiorna carriera | Adapter per-fixture con contesto OG isolato, cattura risultato una sola volta; proteggere CLASSIC/PRESET |
| `diy_competition.c::DIY_PlayOrSimulateLeagueFixture` | ADATTA | Simula/gioca, aggiorna risultati, punti, classifica, progressi, promozioni/split e buffer | Non riutilizzare come semplice calcolo gol; e' proprietaria dello stato del torneo |
| `match_sim.c::CalculateViewResult` | ADATTA | Simula score da rose/skills ma anche status/infortuni, marcatori, RNG e stato di modulo | Non funzione pura `strengthA,strengthB -> score`; snapshot/applica una volta gli effetti |
| `career_status.c::Expanded_OGWorld_AdvanceUefa` | NON CHIAMARE come engine nuova edizione | Inizializza/avanza solo l'annata `World_FirstYear()`, conserva successivi `NEW_EDITION_PENDING`; blocca match quando il manager e' nel campo (`PENDING_MANAGER_BIND`) | Eccellente riferimento golden/format iniziale e confronto; non un resolver di edizioni future |
| `career_init.c::expanded_og_world_run_year`, `Expanded_OGWorld_AdvanceToDate` | NON DOPPIO SCHEDULER | Per anno/phase avanza leghe, coppe nazionali e UEFA; il 1 luglio chiude/stacca contesti nazionali e conserva continentali | Sostituire progressivamente writer per *singola* competizione, mai parallelamente; nessun reset di una coppa cross-year |
| `career_status.c::Expanded_OGWorld_AdvanceLeague/Cup` | RIUSA come riferimento/ADATTA | Immagini OG residenti, contesto squadra, RNG e `lastPhase`; preflight per record anomali | Non cambiare i campionati funzionanti nella fase iniziale; reader snapshot + percorso unico |
| `world_cups.c::WorldCups_Entrants` / `WorldCups_SeasonEnded` | ADATTA | Il primo usa seed reali; dal secondo usa quote, classifiche, promozioni e bye; muta liste | Non richiamare da viewer o audit; estrarre criterio senza generazione implicita |
| `uefa_coef.c::UefaQual_Country`, `UefaQual_Fill` e `uefa_access.c::uq_season` | ADATTA (solo moderno) | Coefficienti, piazzamenti, titolari, quote UEFA, preliminari e liste, scritture su `UQ_IN/UQ_OUT/UQ_S` e RNG | Confrontare Mod versionata, snapshot stati, non usare per 1996/97 |
| `world_conti.c::access,match,tie,ct_round,ct_world_prepare,ct_world_advance_month` | ADATTA (solo moderno) | Simulatore differente OG/Mod con `CT_IN/CT_ST/CT_ED`, RNG privato, risoluzione interi turni di coppa | Non forzare `CalculateViewResult` al posto di questo: cambierebbe algoritmo e RNG. `ct_world_first_ready()` in SDL attualmente torna 0; prova del campo iniziale pendente |
| `world_cupd.c::cd_sim,cd_make`, `league_playoffs.c`, `world_playoff.c` | ADATTA | Generazione e simulazioni particolari (Serie D, play-out, promozioni) con modelli di forza/RNG diversi | Reuse come regole specifiche, non sostituire con score generici |
| `expanded_world_time.c` | RIUSA | Data civile e adattatore alle 8 phase/mese OG | Date effettive da manifesto/fixture, non presumere che ogni phase sia una data precisa |
| `career_status.c::Expanded_OGWorld_WriteSaveJson/ReadSaveJson`; `save_format_json.c::SaveJson_WriteCareer/LoadCareer` | RIUSA + ADATTA | Gia' serializza snapshot OG, team, RNG e context checksum con formato/ABI controllati; controlla buffer e scrive atomico | **Non rifare il CAR da zero**: introdurre in modo compatibile nuove edizioni/journal e testare roundtrip cross-process |
| `expanded_world_live.c` | NON RIATTIVARE | Vecchio journal legacy e vecchio simulatore LWE (disabilitato) | Nessuna seconda autorita' per match, classifiche e tempo |

### Evidenza aggiuntiva: esistono piu' risolutori OG
1. La simulazione normale usa `CalculateViewResult()` da `match_sim.c`, modificando stati delle squadre e RNG.
2. Il sottosistema Mod `world_conti.c:139-177` contiene **`match()` e `tie()` autonomi**, generatore LCG interno e varianti away goals/extratime/penalty.
3. `world_cupd.c:184-208` calcola spareggi con un **altro** modello per forza; `uefa_access.c:294-352` risolve preliminari/modelli di ingresso; `league_playoffs.c` implementa formule ancora diverse.
**Decisione tecnica:** un'interfaccia di risultati comune con `ruleset_id` / `simulator_kind` identificato per format, ma NON un nuovo algoritmo unico di calcolo punteggi. Preferire estrazione di routine esistenti e "esito/effetti" separati, poi journal idempotente.

### Biblioteche condivise/licenze
Il repository OpenSWOS upstream espone `src/rules/README.md` e `docs/RULES_LIBRARY.md`: il vecchio submodule di `aminta/swos-competition-engine` e' stato incorporato in `src/rules/` (MIT per file originari con `LICENSE-MIT`, future modifiche OpenSWOS GPL-3.0-or-later). Nella copia attuale di Expanded le funzioni derivate Mod sono gia' in `league_playoffs.c` e `world_cups.c`, e il documento `sorgenti/docs/RULES_LIBRARY.md` ne descrive i confronti golden (20.000 stagioni dichiarate). **Non importare ciecamente i nuovi sorgenti**: auditare differenze di versione e preservare le licenze/crediti pertinenti. `sorgenti/scripts/league_playoffs_check.sh` richiede DATA/MOD esterni e restituisce `SKIP` quando assenti; non scambiare `SKIP` per `PASS` e non imporre nuove toolchain Windows.

## I cinque blocchi specifici prima di scrivere il pilot
1. **Isolare il risultato dai side effects:** documentare in dettaglio gli stati letti e scritti da simulazione OG e qualifica storica (inclusi RNG, `_selectedTeams`, key cache, big endian, `_careerFileBuffer`, detentori e turni). Preparare un callback/per-evento che applica effetti UNA volta.
2. **Fonti qualificati:** progettare un `SeasonFacts` immutabile per club/piazzamenti/campioni/vincitori da classifiche residenti e fixtures reali; confrontare l'output dei qualificati originali e dei detentori del primo passaggio, senza chiamare `Career_AdvanceToNextSeason_UpdateCompetitions()` integralmente.
3. **Fixture ownership:** un ID `world+competition+edition+stage+tie+leg/replay` e stato in corso; il percorso manager e quello AI condividono lo stesso writer. Il ramo attuale `PENDING_MANAGER_BIND` non puo' restare come soluzione definitiva. Il viewer e' SOLO read-only.
4. **Data ed edizione:** conservare i risultati e i passaggi del torneo indipendentemente dal 1 luglio; gli eventi schedulati hanno DATE civili esplicite; decidere quale format vale nei 3 anni storici in verifica con l'utente prima di attivarli.
5. **Salvataggio/rollback:** estendere il `og_resident` JSON esistente in modo versionato e compatibile, non sostituirlo; proteggere i CAR vecchi e salvare RNG, fixture, qualifiche prima del commit. Prova cross-process (utente Windows) essenziale.

## Piano operativo ridotto (evitare 7 build obbligatorie)
**Traguardo A — primo intervento codice da autorizzare:** solo API di identita'/journal e audit side effects; niente seconda schedulazione. Gate: ogni chiamante ha un writer, controllo identita' read-only, golden fixture senza alterazioni alla partita OG.
**Traguardo B — coppa pilota:** prima ottenere le fonti delle qualificazioni, poi tre edizioni con fixtures/reload, manager bind, format approvato. Gate: confronti OG, nessuna doppia partita e nessuna perdita CAR.
**Traguardo C — secondo formato/importer:** Coppa delle Coppe o coppa domestica, poi import batch dai manifest senza codice per torneo.
Non partire dai 264 manifest moderni e non migrare tutte le leghe contemporaneamente.

## Stato e test
| Voce | Esito |
|---|---|
| Reuse map statica, funzioni e principali effetti identificati | **SI (analisi statica)** |
| Candidati confrontati, pilot Champions 96/97 proposto | **SI (proposta motivata; preflight PENDENTE)** |
| Formato e qualificati storici 1997/98–1998/99 decisi e validati | **NO** |
| Mappa side effect esaustiva per API e test golden compilabili | **NON PROVATO** |
| Nuovo scheduler, fixture journal, qualifier resolver implementati | **NO** |
| Build/test Windows 0.13.87 o nuovo motore in questo intervento | **NON PROVATO** |
| Test tre edizioni e chiusura/CONTINUE | **NON PROVATO** |

**Ulteriori guide:** [COMPETITION_ENGINE_ARCHITECTURE_TODO_20261118.md](COMPETITION_ENGINE_ARCHITECTURE_TODO_20261118.md), [PROJECT_STATE.md](PROJECT_STATE.md), [ENGINE_CODE_DISCOVERIES.md](ENGINE_CODE_DISCOVERIES.md), [GAME_DESIGN.md](GAME_DESIGN.md), [DEVELOPMENT_WORKFLOW.md](DEVELOPMENT_WORKFLOW.md). L'analisi NON modifica il motore e NON obbliga a installare nuovi strumenti.
