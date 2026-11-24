# FASE 2 — Competition Engine universale (v00.13.296)

**Stato:** SOURCE + TEST ISOLATI + COMPILAZIONE LINUX x86_64 = SI. Build Windows MSYS2 = NON PROVATO. Gameplay Windows = NON PROVATO. SAVE/EXIT/CONTINUE tra processi reali = NON PROVATO (provato solo in test isolato con il codec CAR reale).
**Base:** dev/0.10 `ce0c8ad920ff0725c2658f92a806d86cc237b90b` (v00.13.295). Regola UI OG assoluta rispettata: nessuna schermata, tabellone, pulsante, riga, font, focus o navigazione OG modificati. Stadi: nessuna modifica (decisione utente 24/11/2026, vedi GAME_DESIGN).

## 1. Architettura e flusso (writer / reader)

| Dato | Writer unico | Reader |
|---|---|---|
| Clock mondiale | registro CE `scheduler.clock` (FORWARD `expanded_universal_live.c`) | tutti (LWE solo specchio) |
| Definizioni competizioni | `expanded_ce_definitions.c` (da `leagues.json`, `league_calendar.json`, `ce_competitions.json`) | motore, test dati |
| Regole (tabella/partita) | `expanded_ce_rules.c` (parse/validazione) dentro le definizioni | `run_match` (OG), tabellone CE, motore, viewer |
| Edizioni + lifecycle + blocchi + entranti + classifiche finali | `expanded_ce_engine.c::ExpCe_Tick/BlockEdition` | live, CAR (UL01), test |
| Fixture/ledger/eventi | registro CE: `ExpandedUniversal_CarInsertFixtures` (motore), `RunDue` (risultati), `CarPruneEdition`, `CarHoldEdition` | manager calendar, board, WORLD, motore |
| Risultato di una partita | SOLO OG: QUICK (`CalculateViewResult` + `ExtraTimeSecondPhaseScoring` + `PenaltyShootoutScorerPair`) o 50 Hz (`InitializeInGameTeamsAndStartGame`), witness → `RunDue` | ledger |
| Classifiche | nessun writer: ricalcolate da `expanded_ce_table.c` sul ledger con le regole | board CE, VIEW COMPETITION, motore |
| Archivio edizioni | `ExpandedUniversal_ArchiveCloseEdition` chiamato dal motore SOLO al vero close | CAR UA01 |

Ciclo: **definizione → edizione CREATED → PARTICIPANTS_CONFIRMED (entranti con provenienza) → SCHEDULED (fasi generate con date) → ACTIVE (prima gara giocata) → fasi successive generate quando la precedente è completa e decisa → COMPLETED (classifica finale) → ARCHIVED (alla scadenza, prova registro) → potatura fixture quando la successiva è schedulata.** Il Tick gira a START, a ogni nuovo giorno civile del FORWARD e alla fine del FORWARD; non gioca mai partite. Il 30/06 non chiude nulla.

## 2. Matrice type0 / type1 / type2

| Capacità | type0 lega | type1 coppa KO | type2 multifase |
|---|---|---|---|
| Girone unico / andata-ritorno (meetings 1–4) | SI (catalogo + motore) | — | SI (fase TABLE) |
| Gironi multipli | — (divisioni = definizioni separate) | — | SI (`groups`, LISTED/POTS) |
| Punti V/N/P per competizione | SI (OG record) | — | SI (JSON) |
| Spareggi ordinati incl. H2H, NAME OG, LOT | SI (OG: punti, DR, GF, nome) | — | SI |
| Divisioni a catena promozione/retrocessione | SI (90/91 famiglie 2627) | — | — |
| Gironi paralleli (Italia C/D) | BLOCCO LOCALE: regola distribuzione mancante | — | — |
| Playoff/playout | NO: formato non definito nei dati (0 divisioni 2627 li usano) | — | componibili come fasi, dati assenti |
| Split | NO (Fase 3 dati; fasi componibili) | — | componibili |
| Gara unica / andata-ritorno | — | SI | SI |
| Aggregato, gol in trasferta (solo se regola) | — | SI | SI |
| Supplementari e rigori (OG QUICK e 50 Hz) | — | SI | SI |
| Bye | — | SI (`byes`) | SI |
| Accoppiamenti LISTED/SEEDED/BRACKET/DRAW | — | SI | SI |
| Gironi → KO (CROSS_GROUPS A1-B2) | — | — | SI |
| Finale, vincitore, classifica finale | SI | SI | SI |
| Edizione a cavallo d'anno | SI | SI | SI (test continentale ott→mar) |
| Seconda edizione automatica | SI (rollover famiglia) | SI (`next_entrants`) | SI |
| Qualificazioni da risultati CE con provenienza | fonte | SI | SI |
| Manager: tabellone OG | SI (invariato) | **NO: richiede tabellone coppa OG (approvazione UI)** | **NO (idem)** |
| Dati reali 2627 presenti | 134 divisioni | nessuna coppa completa | Copa Bolivia, Libertadores: DATI MANCANTI |

## 3. Regole partita: panchina, sostituzioni, finestre (requisito 4)

**Audit OG (V):** `InitializeInGameTeamsAndStartGame(rnd, minSubs, maxSubs, paramD3, paramD7, …)`: `minSubs → _maxSubsPerTeam` = sostituzioni consentite (`bench_subs.c::isThisPlayerOkToSelect`), `maxSubs → _gameMaxSubstitutes` = panchina (`game_init.c`: slot ≥ 11+max indisponibile). Record lega `subsAndMaxSubs` (0x35 = 3 da 5), coppa `subsAndPointsForWin` (hi cambi / lo panchina). `paramD3` = byte 0x3C disciplina (1 domestiche, 0 europee/nazionali; `player_injury_status`). OG non ha finestre né cambi CPU in 50 Hz.
**Bug V corretti:** il CE passava `minSubs=0` (0 cambi in ogni 50 Hz CE) e `paramD3=3` (squalifiche da cartellini ignorate). Ora: regole della fixture.
**Adattatore finestre (CE, attivo solo in gara CE con `windows>0`):** una finestra = una sessione panchina con almeno un cambio confermato; intervallo libero se `halftime_free`; `et_extra_subs` aggiunti a `_maxSubsPerTeam` all'inizio dei supplementari; `et_extra_windows` aggiunte. Stesso rifiuto OG del limite cambi (nessun pulsante/schermata nuova). CPU e manager stesse regole (la CPU OG non sostituisce in 50 Hz). Dati 2627 attuali: `windows=0` → comportamento OG invariato salvo cambi corretti. NON PROVATO in gioco.

## 4. Campi JSON (`ce_competitions.json`) e regole

| Campo | Significato | Validazione |
|---|---|---|
| `id` | id competizione CE stabile, byte basso = 2 | univoco, ≠ leghe (byte basso 1) |
| `type`, `country`, `name`, `source` | identità/provenienza | type 0..2 |
| `missing` | lacuna dati dichiarata | definizione caricata ma BLOCCATA |
| `table.points[3]`, `table.tiebreak[]`, `table.provenance` | punti V/N/P, spareggi, VERIFIED/GAME_TEMPLATE/OG_NATIVE | vinta>pari≥persa; ultimo criterio NAME/LOT (ordine totale) |
| `match.{legs,knockout,extra_time,penalties,away_goals,bench,subs,windows,halftime_free,et_extra_subs,et_extra_windows,discipline,provenance}` | regole partita (base competizione + override per fase) | panchina ≤ 5 (OG), cambi ≤ panchina, finestre ≤ cambi, KO senza rigori rifiutato, trasferta solo 2 gare |
| `phases[].{kind,groups,meetings,qualify,pairing,byes,higher_home,window,first_edition_days,missing}` | struttura e date | gironi 1..16, meetings 1..4, KO con pairing e casa, date presenti |
| `window.{first,last,weekdays,extra_weekdays,rest,last_year_offset}` | finestra generazione date (GAME_TEMPLATE) | MM-DD validi |
| `first_edition.{year,entrants[club,source=VERIFIED,group,join_phase]}` | partecipanti certificati della prima edizione | chiave club `paese:slot`, solo VERIFIED |
| `next_entrants[].{source,competition|league,from,to,reallocate}` | qualificazione edizioni successive | fonte definita; duplicato senza riallocazione = blocco |
| `expiry.{date,year_offset}` | scadenza edizione | MM-DD |

Leghe (`leagues.json`): `points_for_win`, `substitutes` (cambi), `max_subs` (panchina), divisioni `promoted/relegated/(_po)`; date da `league_calendar.json`. Mancanti → definizione bloccata (mai default europeo).

## 5. Implementato / dati mancanti / decisione necessaria

| Voce | Stato |
|---|---|
| Regole universali versionate + validatore con campo | IMPLEMENTATO, test isolati SI |
| Classifiche con regole e H2H | IMPLEMENTATO; board CE le usa (UI invariata) |
| KO QUICK/50 Hz con ET/rigori/aggregato/trasferta via OG | IMPLEMENTATO; QUICK/50 Hz reale NON PROVATO |
| Lifecycle edizioni, fasi, archivio, potatura | IMPLEMENTATO, test SI |
| Promozioni/retrocessioni a catena | IMPLEMENTATO (90 famiglie) |
| Qualificazioni con provenienza e riallocazione | IMPLEMENTATO, test SI |
| Blocchi locali persistenti + dipendenze (eventi HELD) | IMPLEMENTATO, test SI |
| Rinvio/riprogrammazione con data originale e motivo | INTERFACCIA (`CarRescheduleEvent`, registro rinvii UL01); sblocco automatico NON implementato |
| Italia (gironi paralleli C/D) | DATO MANCANTE: regola di distribuzione promozioni |
| 28 campionati con finestra 2026 iniziata prima dello START (anno solare, Sud America, ecc.) | prima edizione PARZIALE: si gioca, rollover BLOCCATO finché mancano i risultati reali pre-START (Fase 3); se la stagione 2026 è tutta prima dello START la lega non ha prima edizione → **DECISIONE**: bloccare (attuale) o avviare il 2027 con le divisioni dello START come provenienza START_DIVISION |
| Copa Bolivia 2026 | DATI MANCANTI: 16 interserie, regola accoppiamenti KO, slot CE delle squadre |
| Libertadores 2027 | DATI MANCANTI: entranti provvisori, regole sorteggio/ET/trasferta; date reali per turno inserite |
| Coppe/type2 del manager | **DECISIONE UI**: presentazione OG coppa/girone per il manager (tabellone) non approvata → nessuna coppa attiva nei dati 2627 |
| 9697 | DATI MANCANTI: nessun JSON CE (CE inattivo, OG nativo) |
| Nomi/terreni/stadi | FASI SUCCESSIVE (decisione 24/11), nessuna modifica |

## 6. Persistenza: nuovo campo CAR `universal_lifecycle` ("UL01:" hex)

1. **Perché non derivabile:** stato delle edizioni (fase, blocco e motivo, successore creato, parziale), entranti con provenienza delle edizioni non ancora generate, classifiche finali delle edizioni archiviate/potate (le fixture vengono rimosse), hash delle regole con cui l'edizione è partita, registro rinvii.
2. **Record massimi:** ≤ 2 edizioni attive + ≤ 1 archiviata per definizione (potatura) → 2627 ≈ 136 definizioni → ≤ ~400 edizioni; entranti ≤ 128 per edizione.
3. **Costo:** ~45 byte + 17 byte/entrante + 4 byte/club classifica per edizione; 2627 tipico ≈ 50–60 KB binario (≈ 110 KB hex nel JSON prima di zstd; compressibile). RAM ≈ 2,7 KB/edizione (array fissi 128) → ≈ 1 MB. Stime, NON misurate in gioco.
4. **Writer/reader:** writer `ExpCe_EncodeHex` (via `ExpandedUniversal_CarEmit` e snapshot v288); reader `ExpCe_ValidateHex` (preflight CAR) + `ExpCe_DecodeHex` (`ExpandedUniversalLive_EngineRestore`).
5. **Creazione/modifica/rimozione:** create a START/rollover; modificate dal Tick e dai blocchi; edizioni restano finché esistono definizioni (pulizia storica oltre 2 edizioni: da fare con la Fase 4).
6. **Compatibilità:** campo assente (CAR ≤ v295) → ricostruzione dal registro (`ExpCe_BeginWorld`); CAR v296 non leggibile da build precedenti per il nuovo stato evento HELD solo se un'edizione è bloccata (versioni vecchie rifiutano lo stato 2 → file intatto). Formato con magic, versione, FNV-1a/64.
7. **RG-T:** isolato `run_test_expanded_ce_engine.sh` (SAVE→EXIT→CONTINUE equivalente, UL01 corrotto rifiutato, cambio regole tra sessioni); Windows: vedi §8.
Stato evento nuovo **HELD (=2)** nel registro (persistito nella colonna stato evento UE02): evento di edizione bloccata, mai giocato né completato.

## 7. Test eseguiti (Linux gcc 13, isolati, una volta sullo stato finale)

| Runner | Esito | Copre |
|---|---|---|
| `run_test_expanded_ce_engine.sh` (NUOVO) | 14/14 PASS | validatore campi; 2 edizioni lega con promozioni/retrocessioni, archivio, potatura; famiglia non a catena bloccata localmente; prima edizione parziale; coppa KO gara unica + andata/ritorno con trasferta + ET + rigori + bye; type2 gironi→KO→finale a cavallo d'anno; qualificazioni con provenienza e riallocazione; dipendenza da competizione bloccata; SAVE/EXIT/CONTINUE equivalente senza replay; UL01 corrotto; cambio regole tra sessioni; classifiche con regole; risoluzione KO senza vincitore inventato |
| `run_test_expanded_ce_data.sh` (NUOVO) | PASS | dati reali 2627: 136 definizioni (134 leghe pronte, 2 JSON con lacune), 90 famiglie a catena, 1 da regola |
| `run_test_expanded_universal_live.sh` (Fase 1, aggiornato) | 15/15 PASS | ciclo CPU/manager con motore attivo; rosa mancante/tipo incoerente → blocco locale durevole |
| universal_car, fixture_registry, car_writer, save_car_zstd, clock_mirror, world_catalog, league_calendar, car_compact, edition_archive | PASS | regressioni registro/CAR/date |
| `make X86_64=1 -k` | tutti i .c compilano, nessun nuovo warning (netplay escluso: sottomodulo GGPO assente) | compilazione Linux |

**Bug trovati dai test e corretti:** regole base JSON rese KO quando `legs` assente; propagazione dei blocchi che bloccava anche prime edizioni con entranti certificati; testo `missing` lungo perso.
**NON PROVATI:** build Windows, OG QUICK/50 Hz reali con KO, adattatore finestre, tabellone, prestazioni del Tick su 27k fixture (stima O(edizioni×fixture) per giorno civile del FORWARD), CLASSIC/DIY/SEASON (codice toccato solo dietro guardie CE: `bench_subs.c`, `game_state.c`).

## 8. RG-T Windows (BAT ufficiali)

1. START 2627 occupato e disoccupato: log `[ce-engine] DEFINITIONS leagues=134 json=2`, `FIRST_EDITION_PARTIAL` per i campionati iniziati prima del 01/07, `EDITION_BLOCKED … COPA BOLIVIA/LIBERTADORES … missing`.
2. Partita manager 50 Hz: ora si possono fare **3 cambi su 5 panchinari** (prima 0); un giocatore squalificato per cartellini non è schierabile (disciplina 1).
3. VIEW RESULT/CPU QUICK invariati nei risultati di lega; classifica tabellone identica salvo pari punti con ordine OG (nome).
4. FORWARD fino a fine stagione di un campionato a catena (es. Inghilterra): `EDITION_COMPLETED`, `EDITION_PARTICIPANTS league family rollover`, `PHASE_GENERATED … year=2027`; alla scadenza `EDITION_ARCHIVED`, poi `EDITION_PRUNED`; manager promosso/retrocesso gioca nella nuova divisione.
5. Italia: `EDITION_BLOCKED … not a simple chain` solo per l'Italia; resto del mondo prosegue.
6. SAVE/EXIT/CONTINUE prima e dopo un rollover: `[ce-engine] RESTORED editions=N source=UL01 changed=0`; CAR v295 caricato: `source=REGISTRY_REBUILD`.
7. CLASSIC/DIY/SEASON/amichevoli: cambi e panchina come prima.

## 9. Matrice dati reali 2627 (generata da `run_test_expanded_ce_data.sh`, 24/11/2026)

| id | origin | type | name | phases | table | match (bench/subs/win/disc) | rollover | status |
|---|---|---|---|---|---|---|---|---|
| 356319489 | leagues.json | 0 | LEAGUE 20/61/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | DISTRIBUTION_RULE_MISSING | READY |
| 356319745 | leagues.json | 0 | LEAGUE 20/61/2 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | DISTRIBUTION_RULE_MISSING | READY |
| 356320001 | leagues.json | 0 | LEAGUE 20/61/3 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | DISTRIBUTION_RULE_MISSING | READY |
| 356320257 | leagues.json | 0 | LEAGUE 20/61/4 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | DISTRIBUTION_RULE_MISSING | READY |
| 356320513 | leagues.json | 0 | LEAGUE 20/61/5 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | DISTRIBUTION_RULE_MISSING | READY |
| 356320769 | leagues.json | 0 | LEAGUE 20/61/6 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | DISTRIBUTION_RULE_MISSING | READY |
| 356321025 | leagues.json | 0 | LEAGUE 20/61/7 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | DISTRIBUTION_RULE_MISSING | READY |
| 356321281 | leagues.json | 0 | LEAGUE 20/61/8 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | DISTRIBUTION_RULE_MISSING | READY |
| 356321537 | leagues.json | 0 | LEAGUE 20/61/9 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | DISTRIBUTION_RULE_MISSING | READY |
| 356321793 | leagues.json | 0 | LEAGUE 20/61/10 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | DISTRIBUTION_RULE_MISSING | READY |
| 356322049 | leagues.json | 0 | LEAGUE 20/61/11 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | DISTRIBUTION_RULE_MISSING | READY |
| 356322305 | leagues.json | 0 | LEAGUE 20/61/12 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | DISTRIBUTION_RULE_MISSING | READY |
| 356322561 | leagues.json | 0 | LEAGUE 20/61/13 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | DISTRIBUTION_RULE_MISSING | READY |
| 356322817 | leagues.json | 0 | LEAGUE 20/61/14 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | DISTRIBUTION_RULE_MISSING | READY |
| 19202305 | leagues.json | 0 | LEAGUE 0/37/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 727056641 | leagues.json | 0 | LEAGUE 42/86/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 816709889 | leagues.json | 0 | LEAGUE 47/174/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 743899393 | leagues.json | 0 | LEAGUE 43/87/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 743899649 | leagues.json | 0 | LEAGUE 43/87/2 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 743899905 | leagues.json | 0 | LEAGUE 43/87/3 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 900661505 | leagues.json | 0 | LEAGUE 52/175/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 760742145 | leagues.json | 0 | LEAGUE 44/88/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 760742401 | leagues.json | 0 | LEAGUE 44/88/2 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 760742657 | leagues.json | 0 | LEAGUE 44/88/3 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 760742913 | leagues.json | 0 | LEAGUE 44/88/4 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 36045057 | leagues.json | 0 | LEAGUE 1/38/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 917504257 | leagues.json | 0 | LEAGUE 53/176/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 52953345 | leagues.json | 0 | LEAGUE 2/40/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 934347009 | leagues.json | 0 | LEAGUE 54/177/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1294401793 | leagues.json | 0 | LEAGUE 76/39/1 | 1 | OG_NATIVE 3-1-0 | 5/4/0/1 OG_NATIVE | CHAIN_OK | READY |
| 777584897 | leagues.json | 0 | LEAGUE 45/89/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 794427649 | leagues.json | 0 | LEAGUE 46/90/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 794427905 | leagues.json | 0 | LEAGUE 46/90/2 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 69796097 | leagues.json | 0 | LEAGUE 3/41/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 967966977 | leagues.json | 0 | LEAGUE 56/178/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 828047617 | leagues.json | 0 | LEAGUE 48/91/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 984809729 | leagues.json | 0 | LEAGUE 57/179/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 844890369 | leagues.json | 0 | LEAGUE 49/92/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1001652481 | leagues.json | 0 | LEAGUE 58/180/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 86638849 | leagues.json | 0 | LEAGUE 4/42/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 103481601 | leagues.json | 0 | LEAGUE 5/43/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 120324353 | leagues.json | 0 | LEAGUE 6/44/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 137167105 | leagues.json | 0 | LEAGUE 7/45/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 861733121 | leagues.json | 0 | LEAGUE 50/93/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1018495233 | leagues.json | 0 | LEAGUE 59/181/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 154009857 | leagues.json | 0 | LEAGUE 8/46/1 | 1 | OG_NATIVE 3-1-0 | 3/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 154010113 | leagues.json | 0 | LEAGUE 8/46/2 | 1 | OG_NATIVE 3-1-0 | 3/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 154010369 | leagues.json | 0 | LEAGUE 8/46/3 | 1 | OG_NATIVE 3-1-0 | 3/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 154010625 | leagues.json | 0 | LEAGUE 8/46/4 | 1 | OG_NATIVE 3-1-0 | 3/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 609091841 | leagues.json | 0 | LEAGUE 35/78/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 609092097 | leagues.json | 0 | LEAGUE 35/78/2 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 187826433 | leagues.json | 0 | LEAGUE 10/50/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 221511937 | leagues.json | 0 | LEAGUE 12/52/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 238354689 | leagues.json | 0 | LEAGUE 13/53/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 238354945 | leagues.json | 0 | LEAGUE 13/53/2 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 204669185 | leagues.json | 0 | LEAGUE 11/51/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1052115201 | leagues.json | 0 | LEAGUE 61/182/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 255197441 | leagues.json | 0 | LEAGUE 14/54/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 255197697 | leagues.json | 0 | LEAGUE 14/54/2 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 255197953 | leagues.json | 0 | LEAGUE 14/54/3 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1349255425 | leagues.json | 0 | LEAGUE 79/108/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1085735169 | leagues.json | 0 | LEAGUE 63/183/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 272040193 | leagues.json | 0 | LEAGUE 15/55/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1169686785 | leagues.json | 0 | LEAGUE 68/184/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1203306753 | leagues.json | 0 | LEAGUE 70/185/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 289014017 | leagues.json | 0 | LEAGUE 16/58/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1281294593 | leagues.json | 0 | LEAGUE 75/95/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1281294849 | leagues.json | 0 | LEAGUE 75/95/2 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 323354881 | leagues.json | 0 | LEAGUE 18/70/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1270481153 | leagues.json | 0 | LEAGUE 74/186/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 305856769 | leagues.json | 0 | LEAGUE 17/59/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 339476737 | leagues.json | 0 | LEAGUE 19/60/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 945815809 | leagues.json | 0 | LEAGUE 55/96/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 945816065 | leagues.json | 0 | LEAGUE 55/96/2 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1471873281 | leagues.json | 0 | LEAGUE 86/187/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1488716033 | leagues.json | 0 | LEAGUE 87/188/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1505558785 | leagues.json | 0 | LEAGUE 88/189/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1522401537 | leagues.json | 0 | LEAGUE 89/190/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1539244289 | leagues.json | 0 | LEAGUE 90/191/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 390004993 | leagues.json | 0 | LEAGUE 22/63/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 406847745 | leagues.json | 0 | LEAGUE 23/64/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 373162241 | leagues.json | 0 | LEAGUE 21/62/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1556087041 | leagues.json | 0 | LEAGUE 91/192/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1572929793 | leagues.json | 0 | LEAGUE 92/193/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1029767425 | leagues.json | 0 | LEAGUE 60/97/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1029767681 | leagues.json | 0 | LEAGUE 60/97/2 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1589772545 | leagues.json | 0 | LEAGUE 93/194/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 423690497 | leagues.json | 0 | LEAGUE 24/65/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1606615297 | leagues.json | 0 | LEAGUE 94/195/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 439877889 | leagues.json | 0 | LEAGUE 25/56/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 439878145 | leagues.json | 0 | LEAGUE 25/56/2 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 457310465 | leagues.json | 0 | LEAGUE 26/66/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 474153217 | leagues.json | 0 | LEAGUE 27/67/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1063387393 | leagues.json | 0 | LEAGUE 62/98/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1063387649 | leagues.json | 0 | LEAGUE 62/98/2 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1063387905 | leagues.json | 0 | LEAGUE 62/98/3 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1623458049 | leagues.json | 0 | LEAGUE 95/196/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1097007361 | leagues.json | 0 | LEAGUE 64/99/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1113850113 | leagues.json | 0 | LEAGUE 65/100/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 490995969 | leagues.json | 0 | LEAGUE 28/68/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 507838721 | leagues.json | 0 | LEAGUE 29/69/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 507838977 | leagues.json | 0 | LEAGUE 29/69/2 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1640300801 | leagues.json | 0 | LEAGUE 96/197/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 524747009 | leagues.json | 0 | LEAGUE 30/71/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1181024513 | leagues.json | 0 | LEAGUE 69/101/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 541589761 | leagues.json | 0 | LEAGUE 31/72/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 575275265 | leagues.json | 0 | LEAGUE 33/74/1 | 1 | OG_NATIVE 3-1-0 | 3/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 575275521 | leagues.json | 0 | LEAGUE 33/74/2 | 1 | OG_NATIVE 3-1-0 | 3/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 575275777 | leagues.json | 0 | LEAGUE 33/74/3 | 1 | OG_NATIVE 3-1-0 | 3/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 575276033 | leagues.json | 0 | LEAGUE 33/74/4 | 1 | OG_NATIVE 3-1-0 | 3/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 878575873 | leagues.json | 0 | LEAGUE 51/94/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 878576129 | leagues.json | 0 | LEAGUE 51/94/2 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 878576385 | leagues.json | 0 | LEAGUE 51/94/3 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 878576641 | leagues.json | 0 | LEAGUE 51/94/4 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 878576897 | leagues.json | 0 | LEAGUE 51/94/5 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 558432513 | leagues.json | 0 | LEAGUE 32/73/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 710213889 | leagues.json | 0 | LEAGUE 41/85/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 642842881 | leagues.json | 0 | LEAGUE 37/81/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1130758401 | leagues.json | 0 | LEAGUE 66/102/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1330381057 | leagues.json | 0 | LEAGUE 78/76/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 592249089 | leagues.json | 0 | LEAGUE 34/77/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 626000129 | leagues.json | 0 | LEAGUE 36/80/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1147601153 | leagues.json | 0 | LEAGUE 67/103/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1147601409 | leagues.json | 0 | LEAGUE 67/103/2 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1657143553 | leagues.json | 0 | LEAGUE 97/198/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 659685633 | leagues.json | 0 | LEAGUE 38/82/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 659685889 | leagues.json | 0 | LEAGUE 38/82/2 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1673986305 | leagues.json | 0 | LEAGUE 98/199/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 676528385 | leagues.json | 0 | LEAGUE 39/83/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1214775553 | leagues.json | 0 | LEAGUE 71/104/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1214775809 | leagues.json | 0 | LEAGUE 71/104/2 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1248461057 | leagues.json | 0 | LEAGUE 73/106/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 1315635457 | leagues.json | 0 | LEAGUE 77/107/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 693371137 | leagues.json | 0 | LEAGUE 40/84/1 | 1 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | CHAIN_OK | READY |
| 2949378 | ce_competitions.json | 2 | COPA BOLIVIA | 4 | OG_NATIVE 3-1-0 | 5/3/0/1 OG_NATIVE | - | 16 interseries (clasico) fixtures not authenticated/dated; group->knockout pairing rule not ver |
| 13107458 | ce_competitions.json | 2 | COPA LIBERTADORES | 8 | MISSING 0-0-0 | 5/3/0/0 GAME_TEMPLATE | - | 2027 entrants provisional (Q1/Q2 allocation not approved per club); draw/pairing rules of every |
