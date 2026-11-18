# Competition Engine — chiusura punto 1 (audit OG), 18/11/2026

**Riferimento:** Giuzy/SWOS-Expanded, `dev/0.10`, HEAD precedente `b0ac50d1be9ae03bbfd67da9afeea0861df35507`, build `0.13.87`.
**STATO ESATTO:** audit statico e verifica isolata asset CONCLUSI; nuovo Competition Engine, adapter, golden test, tre edizioni e gameplay Windows **NON implementati / NON PROVATI**. Questo è un commit **solo documentale**.

## Decisione approvata: B, ma dopo il sistema di base
Il 18/11 l'utente sceglie **B — regolamenti storici evolutivi** nel mondo che parte nel 1996/97: formati, quote, turni e iscritti potranno cambiare tra edizioni, seguendo fonti attendibili (esempio storico: Champions con 16 partecipanti nel 1996/97 e 24 nel 1997/98). **Ordine esplicito:** prima far funzionare sistema mondiale single-writer, qualificazioni, fixture, manager binding e CAR/CONTINUE con le competizioni BASILARI; solo DOPO implementare transizioni dei formati storici, nuove competizioni, importatori e editor. Non inventare calendari, classifiche, detentori o qualificati.

**Pilota tecnico scelto:** `_euroCup`, Champions OG 96/97 (16 squadre, quattro gironi da quattro, knockout). Usare il formato OG per golden/sandbox delle API anche per tre iterazioni tecniche, ma **NON presentare le edizioni 1997/98 o 1998/99 della CAREER come storicamente fedeli a 16 squadre**. La carriera live richiederà i formati/data pack storici annuali quando saranno introdotti; in alternativa pilotare prima una coppa domestica basica con formato verificato.

## Preflight OG della prima Champions
Il record `sorgenti/src/competition_data.c::_euroCup` contiene 16 coppie `(paese, ordinale)` distinte, 15 federazioni (Italia due volte). Lista: Auxerre, Ajax, Grasshoppers, Rangers, Atletico Madrid, Steaua Bucharest, Borussia Dortmund, Widzew Lodz, Rapid Vienna, Fenerbahce, Juventus, Manchester United, IFK Gothenburg, Rosenborg, AC Milan, Porto. Tutte le 15 famiglie di file TEAM sono presenti in `data/` e la mappatura federazione-file è ricostruibile: **presenza dei file, non prova di caricamento gameplay di tutti gli ordinali**.

`sorgenti/src/swos_amiga_ptr_imports.c::_euroCupQualGroupSeedLists_4_6_11`: 4/4 + 5/6 + 5/11 + 2/22 = **16** federazioni; i gruppi sono mescolati con `Rand2`. `career_season.c::Career_PrepareEuropeanCupQualificationState` considera `_selTeamNumber` e `_teamNoOfferAccepted`: può favorire la federazione del manager; `Career_FillRandomTeamListExcludingSelected` modifica anche i bucket. `career_eurocups.c::qualifyTeamForEuroCup` legge il campione dalla classifica OG, e `Career_DrawEuropeanCupTeamsAndCopyGrids` modifica lista, pool TMD e gestione detentore. Per l'Expanded single-world il sorteggio futuro usa RNG del MONDO e `SeasonFacts` reali, **non** selezione allenatore o riavanzamento delle partite.

`career_status.c::Expanded_OGWorld_AdvanceUefa` realizza solo la prima edizione e poi `NEW_EDITION_PENDING qualification=NOT_CONNECTED`. L'attuale `PENDING_MANAGER_BIND` evita due partite ma salta incontri di coppa se è presente il manager: il nuovo journal deve unificare AI e partita manuale.

## Francia e Turchia: INCOERENZA CONFERMATA con asset reali
Metodo: recupero dei blob RNC1 direttamente dal ramo e decodifica isolata con la logica di `sorgenti/src/rnc.c`; ispezione di count, ordinali, nomi e `team.league` dei record `TeamFileEx` da 684 byte. **È una verifica dei dati sorgente, non un test del loader Windows.**

| Sorgente | Squadre nel file | Prima divisione (league=0) | Altre |
|---|---:|---:|---|
| `data/TEAM.013` generico | 42 | **18** | 22 in divisione 1, 1 in divisione 2, 1 fuori |
| `data/9697/TEAM.013` storico | 42 | **20** | 22 in divisione 1 |
| `data/TEAM.038` generico | 24 | **17** | 7 fuori |
| `data/9697/TEAM.038` storico | 24 | **18** | 6 fuori |

Stessa coppia `(country,ordinal)`, ma **10 record francesi e 6 turchi cambiano nome e/o divisione**. Esempi Francia: ord. 5 Caen (1→0), 8 Chateauroux (0→1), 16 Lille (1→0), ord. 31 Nimes→Perpignan, ord. 7 Sedan→Charleville, ord. 34 Wasquehal→Saint-Brieuc. Turchia: ord. 7 Denizlispor (fuori→0), 8 Eskisehirspor (fuori→0), 15 Karsiyaka (fuori→0), ord. 3 Sekerspor→Aydinspor. Nessun ordinale mancante/aggiunto nelle due coppie, ma **gli stessi ordinali possono rappresentare dati diversi**.

**Causa esatta nel codice:**
- `sorgenti/src/teams.c:432-459`, `LoadTeamFile`: in `WORLD_9697`+CAREER usa `data/9697/TEAM.012/.013/.038/.073` ove esistono, altrimenti il generico. Quindi il percorso principale della CAREER ha l'override corretto sul disco.
- `sorgenti/src/team_db.c:202-233`, `read_country`: quando non usa JSON 2627, legge **sempre** `data/team.%03d` generico, anche in CAREER 9697. `s_country[256]` indicizza per file soltanto, senza mondo o impronta dati e senza invalidazione visibile nel `world_switch.c`.
- `custom_team.c` chiama `TeamDb_Team/ByUuid`. La cache potrebbe rappresentare club, nomi e checksum differenti da quelli usati da OG; la gravità specifica nel gioco non è stata riprodotta su Windows. **Questo è un bug della coerenza delle sorgenti, non prova di corruzione CAR corrente.**
- `sorgenti/docs/TEAM_IDS.md`: UUID `96-97/team/<country>/<ordinal>` e checksum sono identità di **dati-squadra fissi**, NON il ClubKey che serve alla carriera attraverso trasferimenti, nuove rose e anni. Non basare il nuovo competitionId/ClubKey su team data UUID; usare world+stableClubKey e mapping per record e revisione del world-pack.

**TODO CORREZIONE (prima di integrare gli ID persistenti del nuovo engine; NON fatta ora):** unificare l'accesso read-only al file realmente scelto da `LoadTeamFile`, risolvere per mondo/modo/asset e invalidare o separare cache senza dangling pointer, preservando CLASSIC/DIY/SEASON e CUSTOM. Test isolati obbligatori 013→20, 038→18 e ord. (13,31) / (38,3); switch 9697→2627→9697. Non richiamare `LoadTeamFile` da un semplice viewer senza audit dei suoi side effects.

## Matrice OG (audit statico)
| Componente | Decisione | Principale rischio |
|---|---|---|
| `expanded_world_time.c` | **RIUSA** | Data civile; OG phase=1/8 mese non giorno esatto |
| `competition_data.c`, `diy_competition.c::DIY_PlayCupMatchAndAdvance` | **RIUSA/ADATTA** | Score, aggregati, rigori, tabellone, buffer `_diyCompetitionStart/_diyFileBufferCopy` |
| `match_sim.c::CalculateViewResult` | **ADATTA** | Remap/status giocatori, marcatori e RNG; non funzione pura |
| `career_season.c::Career_AdvanceToNextSeason_UpdateCompetitions` | **NON CHIAMARE COME QUERY LIVE** | Simula vecchie coppe/leghe, muta liste TMD, fa seed nuova stagione |
| `career_eurocups.c` / `swos_amiga_ptr_imports.c` | **ADATTA** | Seed draw, paese manager, detentore, `Rand2`, liste BE e TMD |
| `career_status.c::Expanded_OGWorld_AdvanceLeague/Cup/Uefa` | **REFERENCE/ADATTA** | Contesti residenti, `lastPhase`, snapshot team e `_seed2/_randXor*` |
| `world_cups.c` | **ADATTA** | Iscritti dal secondo anno, quote/piazzamenti/bye; muta seed |
| `world_conti.c`, `world_cupd.c`, `uefa_access.c`, `league_playoffs.c` | **ADATTA PER FORMATO** | Simulatori/RNG diversi, non usare un solo algoritmo forzato |
| `career_status.c::Expanded_OGWorld_WriteSaveJson/ReadSaveJson`, `save_format_json.c` | **ESTENDI** | Già esiste JSON OG+checksum+write atomico, non ancora fixtureId/editionId universale |
| `expanded_world_live.c` | **NON RIATTIVARE** | Niente secondo LWE autorevole o risultati duplicati |

Biblioteca regole upstream `src/rules/`: parti originarie MIT, evoluzioni OG GPL (verificare versioni e licenze prima di importare); Expanded ha già port di regole in `league_playoffs.c` e `world_cups.c`. Gli script statici e i golden preesistenti non provano il nuovo Competition Engine; `SKIP` non è `PASS`.

## Contratto per la fase 2 (NON implementato)
`ClubKey=(world, stableClubKey)`, distinto dall'UUID di dati squadra; `CompetitionKey=(world, stableCompetitionKey)`, **non** solo `compId` byte riusato fra paesi; `EditionId=(competition, editionStart)` e `FixtureId=(edition, stage, tie, leg, replay)`. Un unico calendario per eventi e un solo writer per fixture; tutte le partite del manager e AI nello stesso journal; `SCHEDULED→PLAYED` una volta. `SeasonFacts` da classifiche/titoli realmente giocati e persistiti. Save CAR separato per mondo con dati nuovi versionati + compatibilità vecchi CAR. Il 1 luglio non cancella una coppa cross-year.

**GATE IMPLEMENTATIVI (trasferiti ai punti 2–6 della TODO, tutti NON PROVATI):**
- [ ] Sorgente club canonica verificata nel runtime, cache separata per mondo e risultati serializzati identici dopo CONTINUE.
- [ ] Identità ClubKey/Edition/Fixture stabile, nessun doppio writer o risultato retroattivo.
- [ ] Golden OG/adapter a parità di input + RNG; indipendenza totale dall'assunzione del manager.
- [ ] Event scheduler / CAR roundtrip cross-process, salvataggi esistenti preservati.
- [ ] Tre edizioni consecutive di coppa (sandbox e poi reali con ruleset anno-corretto), classifiche/qualificati derivati da fatti.
- [ ] Test Windows dall'utente, senza regressione partita OG 50 Hz, CLASSIC/DIY/SEASON.

## Checklist di chiusura
| Voce | Stato |
|---|---|
| Reuse map, side effects principali, calendario e save | **SI — audit statico** |
| Coppa pilota scelta e seed / quote verificati nel codice | **SI — sorgente** |
| Francia e Turchia RNC: file, conteggi e divergenza `TeamDb` | **SI — isolato** |
| Decisione B e posticipo modifiche storiche | **SI — approvato** |
| Punto 1 (AUDIT STATICO) completato | **SI** |
| Bug `team_db.c` corretto in codice | **NO** |
| Nuovo Engine/3 edizioni/save nuovo implementati | **NO** |
| Golden adapter e test Windows | **NON PROVATO** |

Riferimenti: [COMPETITION_ENGINE_OG_AUDIT_20261118.md](COMPETITION_ENGINE_OG_AUDIT_20261118.md), [COMPETITION_ENGINE_ARCHITECTURE_TODO_20261118.md](COMPETITION_ENGINE_ARCHITECTURE_TODO_20261118.md), [ENGINE_CODE_DISCOVERIES.md](ENGINE_CODE_DISCOVERIES.md), [AGENTS.md](AGENTS.md), [PROJECT_STATE.md](PROJECT_STATE.md), [GAME_DESIGN.md](GAME_DESIGN.md).
