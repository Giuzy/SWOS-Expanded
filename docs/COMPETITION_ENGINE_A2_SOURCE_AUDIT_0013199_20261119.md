# SWOS Expanded — A2.1 Competition Engine integration (draft 00.13.199)
**19 novembre 2026 — PREPARATO, NON COMMITTATO** su branch base dev/0.10, HEAD `1e6f6b62f22e69752b0afa31e9b128a92ec2e9a9` (src 00.13.198). L'utente non ha autorizzato un commit in questo turno. Non dichiarare la 199 disponibile in GitKraken né compilata.

## Sorgenti Windows analizzate
* `DIAGNOSTICA_SESSIONE_1(20261119-121339).zip`: build effettiva **00.13.197**, HEAD Windows `1689a52f441a6edb1cab124390ce0104eff88128`, `COMPILAZIONE_0140.log` termina con compilazione completata e zero `warning:`/`error:` GCC espliciti. Mondo 2627: DEBUG 30 JUNE raggiunge **2027-06-30**, offerta reale club 3334, world native 134 leghe residenti, close 2026 con 33.287 match OG e OPEN 2027 134 leghe, invalid 0. Il driver segnala `OG_NEXT_EDITION_DOMESTIC_CUP_IDS=FAIL exact=0 wrong=101`, **NON 101 club ID dimostrati sbagliati**: in log `[og-cup-entry] ... source=NATIVE_NEXT_YEAR_ACCESS expected=0 ... status=FAIL_OG_ACCESS_NOT_DERIVED provenance=ACCESS_UNAVAILABLE`. Sono residenti 101 coppe 2027 senza provenienza verificata dei qualificati. UEFA 2027 CL, Europa e Conference 36 ID corretti per competizione; Copa 2027 fonte assente. SAVE/CONTINUE cross-process NOT_TESTED, 70 seasons NOT_TESTED. Totale 16 campioni performance FORWARD, 7410.721 ms; leagues 6113.996 ms, cups 721.639 ms, international 289.283 ms (unità misurate in driver).
* `DIAGNOSTICA_SESSIONE_2(20261119-121340).zip`: stessa 197, GCC completato senza warning/error espliciti. Include mondi 9697 e 2627; avanzamento disoccupato riuscito più volte; 5 `BLOCK_PAIR_RECONCILIATION` in manager/calendar (3/2). In 2627 dal manager occupato FINISH SEASON: `[og-yearend-resident] TYPE2_REUSED ... matches=54` poi `VerifyTeamLoaded(0x0000)`, `PANIC 0x0000`, exception `c000001d`. Nessun test della build 198/199 è contenuto qui.
* Non trasferire PASS driver `PASS_ISOLATED_ONLY` al gameplay. Nessuna diagnostica Windows 198/199 in questo intervento.

## Audit: problema strutturale e punti condivisi
1. `career_init.c:1602–1805` ha mondo OG resident con tick data unica; `career_init.c:1807–1935` coordinatore A1 (START, unemployed FORWARD, bind preflight, LOAD, SAVE latch). Manager/cup non passava dal medesimo contratto. Il calendario civile LWE è compatibilità e non deve simulare risultati.
2. `career_status.c:8706+` CPU OG league chiama `DIY_PlayOrSimulateLeagueFixture(0,0)`, come `DIYLeagueTournamentMatchCommon` per il manager. Cup OG residente e manager condividono `DIY_PlayCupMatchAndAdvance`. Sono gli **esecutori OG già esistenti**, non crearne un altro.
3. `career_status.c:7671–7760` contiene preflight locale manager **prima** delle gare, validazione delle classifiche/seriale/residente, poi `CommitLocalLeagueFixture` dopo OG per copiare una volta score, team e RNG. `diy_competition.c:12373+` osservava risultato e invocava il commit ma ne ignorava il fallimento. `career_status.c:3950+` mantiene witness OG 20-byte con indice anti-duplicato, ma solo **dopo** la partita: non equivale al claim preventivo universale.
4. `diy_competition.c:11774,11960` pubblicava anche risultati/vincitori delle coppe nel vecchio `ExpandedWorldLive_RecordNativeCupMatch/Champion` pur essendo `EXPANDED_WORLD_OG_ONLY=1`; `expanded_world_live.c:1364+` scrive realmente un vecchio journal LWE. Per CAREER OG-only è ridondante e non costituisce la fonte unica.
5. `world_cups.c:189,262–290` e `include/world_cups.h` tengono una sola coppia `placesCountry/placesSeason` e `place[512]`. Il parser CAR `save_format_json.c:6645–6659,7552–7575` serializza solo quel paese (nel 2627). Con 91 paesi/101 coppe non è possibile verificare in blocco i qualificati al nuovo anno. `league_playoffs.c:1403` cattura OG KeepPlace solo nel callback native, non una raccolta globale. `Expanded_OGWorld_ReadLeagueStandings` 7129–7188 legge tabella OG residente senza simulare e può servire da fonte prima reset, ma non è ancora stato dimostrato il mapping qualificazioni per tutti i formati.

## Modifiche A2.1 preparate in GitHub blob, NON nel branch
* `expanded_competition_coordinator.h`: nuove firme `PreflightFixture(kind)`, `ReportFixtureFault(reason)`.
* `career_init.c`: verifica pre-gara di data civile / anno OG / recovery-latch per match locale manager e CPU nel contesto di carriera; errore post-gara abilita recovery e blocco CAR. Nessuna altra simulazione.
* `career_status.c/.h`: pubblica `Expanded_OGWorld_LocalLeagueBridgeActive(local)` come predicato read-only per riconoscere il manager-league che ha vero residente, e prototipo `IsSimulating()`.
* `diy_competition.c`: preflight coordinator su match **LOCAL_LEAGUE e LOCAL_CUP** solo CAREER, quando non siamo nel tick residente OG né nel FINISH manuale; post-gara league con **owner bridge validato** verifica esito witness e commit resident; se fallito blocca SAVE/replay senza fingere un rollback. Stessa sicurezza su score di coppa coinvolgente il manager, quando manca il witness; mai marcare CPU-only senza identità come manager. Disattiva le due scritture nel vecchio LWE cup journal in OG_ONLY, lascia OG-native score, progress, cup and witness invariati.
* `build_info.h`: `00.13.199` **solo preparata**, non versionata sul branch. Non aggiornare la versione per i soli documenti.
* Nessuna modifica a `Career_AdvanceToNextSeason_UpdateCompetitions` o al manual FINISH per la priorità esplicita dell'utente. Nessuna modifica a CLASSIC/DIY/SEASON, multiplayer o partita OG 50Hz.
* Static check unique replacements, bilancio parentesi e direttive preprocessore sui 6 file iniziali PASS; non è GCC/Windows. Verificare ancora warning, prototipi, VLA, include order alla prossima build Windows.

## Perché A2.1 e NON scheduler live completo
Il contratto `expanded_competition_identity.h` è ancora prototipo di fixture key e CLAIMED/PLAYED non collegato al risultato OG e al CAR atomico; `expanded_event_scheduler.h` dichiara esplicitamente che non esegue partite e non è live. Attivarlo in parallelo farebbe due owner per le medesime gare. Il percorso più sicuro è integrare i callsite OG reali e sostituire il solo controllo di autorità, senza nuove partite. Un errore OG dopo mutazione RAM può essere bloccato dalla scrittura CAR, **non annullato**: solo il LOAD di un CAR valido preesistente recupera lo stato.

## Gate ancora APERTI
- A2.1: test Windows su entrambi i mondi con manager (manuale e CPU), cup locale e 30/06; questa build è solo staging. L'utente compila e prova.
- A2 completo: ownership per (mondo,competizione,edizione) e fixture universale per tutte le coppe/leghe, compresi casi type-2 e 96/97.
- B: ID stabili e claim PREVENTIVO prima della partita per manager+CPU, journal e risultato OG / stato squadre / RNG / ack eventi nel CAR atomico. SAVE/LOAD cross-process testato.
- C: transizione editon Close→FreezeFinal→ResolveQualified→ValidateEntrants→OpenNext, indipendente dall'impiego del manager, coppe cross-year non reset. In particolare snapshot OG final places **per paese e per stagione** in CAR, non un singolo `placesCountry`.
- D/E: manifest regolamento/data-driven validato e prova pilota tre edizioni consecutive per entrambe le epoche.

## QA Windows richiesta dopo autorizzazione commit
1. Sincronizzare con GitKraken, compilare usando script esistenti; zero warning/error nuovi, log completi.
2. START UNEMPLOYED entrambe epoche, DEBUG +1 MONTH e DEBUG 30 JUNE effettivamente visibili/cliccabili e date native coerenti; controllare righe `[competition-coordinator]`.
3. Ingaggio e NEXT MATCH locale giocato/simulato in campionato e coppa; zero score doppio, esplicite le verifiche `PreflightFixture` e bridge, nessun `WRITE_BLOCKED` inatteso.
4. SAVE/CONTINUE cross-process su entrambi i mondi; stesso risultato/fixture/RNG e stato resident prima/dopo; eventuale fault deve impedire di sostituire vecchio CAR.
5. Verificare l'elenco e gli ID reali delle coppe nazionali/continentali anno 2/3: `ACCESS_UNAVAILABLE` resta NOT_VERIFIED, non PASS; misurare performance FORWARD F12.
6. FINISH manuale rimane problema legacy non riparato in questa patch, riportare log se provato.

**Checklist:** SI: ZIP 197 analizzati, source audit, draft codice A2.1, check statici. NO: commit, integrazione universale, qualificazioni nazionali multi-paese, CAR atomico. NON PROVATO: Windows build/warning 199, gameplay 199, persist, completezza passaggio stagione.