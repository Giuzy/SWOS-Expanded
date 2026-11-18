# SWOS Expanded — Competition Engine: architettura, TODO e gate

## Punto 2 — 0.13.88: base codice, integrazione live ancora DA FARE
Dettagli: [COMPETITION_ENGINE_POINT2_20261118.md](COMPETITION_ENGINE_POINT2_20261118.md).
- [x] 2a Shared source path dei file storici 96/97 CAREER, fallback OG preservato.
- [x] 2b Cache TeamDb per WORLD e modo carriera/classico, senza liberare puntatori CUSTOM.
- [x] 2c Tipo di ID club/competizione/edizione/fixture e protocollo header-only Schedule/Begin/Finish + test isolato.
- [ ] 2d Registro reale degli ID club/competizione da world-pack; mapping verificato di record/rose.
- [ ] 2e Un solo writer live per fixture (manager e CPU), senza callback OG concorrenti.
- [ ] 2f Claim/risultato/effetti/RNG con salvataggio CAR atomico e CONTINUE (punto 3).
- [ ] 2g Test Windows 96/26/CLASSIC/DIY/SEASON/CUSTOM e vecchi CAR.
- [ ] 2h Golden OG e dimostrazione pratica di idempotenza delle partite.
Il punto 2 NON è completo sul gameplay: gli header sono volutamente scollegati dalle competizioni fino al gate CAR. B evoluzioni storiche DOPO il sistema basico.


## Punto 1 — audit statico CHIUSO 18/11/2026
**[x] ATTIVITÀ 1 COMPLETATA COME AUDIT OG STATICO; NON è un gate di implementazione.** [Relazione finale](COMPETITION_ENGINE_PHASE1_CLOSURE_20261118.md) e [reuse map](COMPETITION_ENGINE_OG_AUDIT_20261118.md).
- [x] Mappa OG RIUSA/ADATTA/NON CHIAMARE LIVE, side effects principali, RNG, calendario, save, seed Champions OG 16 / quote 4+5+5+2.
- [x] Binary preflight: France TEAM.013 18→20 league0, Turkey TEAM.038 17→18; cause path mismatch `teams.c` vs `team_db.c` documentate; nessuna patch runtime.
- [x] **Scelta B approvata:** regole/formati storici evolutivi nel tempo, da fare DOPO engine e coppe basiche; Champions 16 oltre la prima annata solo golden sandbox.
- [ ] **PUNTO 2 in poi:** ClubKey stabile, cache corretta per mondo, fixture unique, golden OG/adapter, manager bind, CAR, 3 edizioni, Windows. TUTTO DA IMPLEMENTARE; non fare PASS finché non esiste prova. Sette attività in tre traguardi, non sette release.

Data: 18 novembre 2026. Snapshot consultato: dev/0.10, HEAD f3ff905d9, build 0.13.87.
**STATO: decisione architetturale APPROVATA; nuovo codice NON implementato; test della nuova architettura NON PROVATI.**

## Decisione (18/11/2026) e principio anti-duplicazione
L'utente approva di rivedere il principio 14/11: Expanded potra' essere l'**unico gestore** della data civile, calendario mondiale, ciclo di vita delle competizioni, edizioni, qualificazioni, fixture, risultati persistenti, storico e save. Il motore partita OG a 50 Hz rimane intatto (fisica, input, replay, gameplay); riutilizzare gli algoritmi OG di simulazione, gironi, sorteggi, tie/replay, classifiche, spareggi dove possibile con adapter verificati. Non introdurre un secondo mondo LWE, una seconda autorita' sui risultati o un replay silenzioso.
**Invarianti:** una fixture, una identita', una sola esecuzione e un solo risultato; nessun match retroattivo inventato; firma/esonero/cambio club collegano o scollegano il manager SENZA reset di mondo; doppio CAR 9697/2627 protetto; CLASSIC/DIY/SEASON invariati. Migrazione **per competizione**, mai big-bang: quando una competizione e' gestita dal nuovo percorso, disabilitare il vecchio callback che produrrebbe le stesse partite, ma lasciare intatte le altre. Fermarsi se non si puo' dimostrare la singola autorita'.

## Risposta: una coppa pilota e sette punti ≠ sette versioni
**Coppa pilota = UNA coppa GIA' presente negli SWOS**, preferibilmente continentale con qualificazioni, non una coppa inventata; scegliere il caso concreto soltanto dopo audit degli algoritmi e disponibilita' dei dati autentici. **Successo:** TRE EDIZIONI CONSECUTIVE reali della stessa coppa, partecipanti della nuova edizione derivati da classifiche/vincitori/detentori reali, fixture/scores per ID, tutti i turni e finale, save atomico, chiusura/CONTINUE anche fra le stagioni e verifica con e senza club allenato. Poi un secondo torneo di FORMATO diverso. Una coppa visibile in VIEW non e' un test del calendario.
Le sette attivita' sotto NON significano sette build, commit o versioni. **Tre traguardi tecnici**, raggruppabili senza numerazione preventiva di build: A fondamenta; B coppa pilota; C importazione. Nessun tempo garantito; un gate fallito blocca l'ampliamento, non autorizza scorciatoie.

## Perche' le leghe sono piu' semplici delle coppe
- `career_season.c::processPrimaryCompetitionForNextSeason` legge divisioni, tabelle e movimenti; `processDiyLeagueQualificationEntry` e `LeaguePlayoffs_Decide` gestiscono passaggi locali; `career_init.c::InitializeNewSeason` applica `LeagueGroups_Apply`. `career_status.c::Expanded_OGWorld_AdvanceLeague` usa contesto OG paese/divisione/anno e fase idempotente. Non significa che OG abbia gia' uno storico per-fixture e roundtrip completo certificato.
- Le coppe internazionali dipendono dai piazzamenti di MOLTE leghe, vincitori di altre coppe, detentori, coefficienti, quote, preliminari, bye, esclusioni, ripescaggi e trasferimenti fra tornei. Il calendario puo' attraversare il 1 luglio. Questo e' un grafo di dipendenze, non solo un reset di tabellone.
- `career_status.c::Expanded_OGWorld_AdvanceUefa` ha un guard `year != World_FirstYear()` che stampa `NEW_EDITION_PENDING qualification=NOT_CONNECTED` e conserva il precedente torneo: non nasce la seconda edizione. `Expanded_OGWorld_ResetSeason` conserva l'ultimo stato continentale per non perderlo nel CAR; le leghe e le coppe domestiche hanno percorsi nazionali di stagione separati. **Le qualificazioni sono il primo blocco da affrontare, ma non l'unico:** servono anche fixture, binding del manager, save e storici.
- `world_cups.c::WorldCups_Entrants` riutilizza classifiche e regole native per diverse coppe domestiche dal secondo anno. `world_conti.c` possiede CT_SPEC, CT_IN, CT_ST e CT_ED: il percorso SDL `ct_world_first_ready()` e' ancora un placeholder zero nello snapshot. Non dichiarare nessuna coppa residente per definizione/visualizzazione.
- `expanded_world_time.c` ha gia' la data civile giornaliera e la conversione da/su chiavi OG con 8 segmenti per mese; `expanded_world_live.c::ExpandedWorldLive_AdvanceToDate` non deve riattivare il simulatore LWE disabilitato. `match_sim.c::CalculateViewResult` produce risultati OG, ma NON e' una funzione pura da due numeri: incide su RNG, statistiche dei calciatori, eventuali stati e variabili globali.

## Architettura da realizzare
1. **World-pack e definition**: per ogni epoca e stagione specificare competizione, fasi, iscritti, turni, date reali, regole e fonti delle qualificazioni in manifest validati; niente nuovo C per ogni caso rappresentabile dalle regole generiche. 96/97 e 26/27 NON condividono automaticamente format, club o regolamenti.
2. **Competition Edition State**: stato unico per ciascuna edizione, anche cross-year. Separare anno civile, stagione manageriale al 1 luglio e date di inizio/fine della competizione. Tabelloni, rose riferite per ID, fasi, RNG, risultati, vincitori e qualificati diventano fatti persistenti.
3. **Scheduler giornaliero a eventi**: un cursore civile, ma esecuzione per PROSSIMO EVENTO (o per tutti gli eventi entro il target), senza tick di giorni vuoti. Alla stessa data ordine deterministico; nessun evento puo' consumare una classifica/finale non ancora chiusa. Chiavi phase OG solo tramite adapter, non data civile assoluta.
4. **Qualification resolver**: primitive proposte LEAGUE_POSITION, CUP_WINNER, TITLE_HOLDER, GROUP_POSITION, WINNER_OF, BEST_ELIGIBLE, TRANSFER_FROM, quote regionali/nazionali, preliminari, tie-break, esclusione duplicati. Prima edizione usa seed reali verificati, edizioni successive solo fatti generati e archiviati. Quando mancano input, fermare l'edizione con diagnostica **senza inventare squadre**.
5. **Match adapter**: per match AI input squadra e rosa originali, forza derivata dalle routine OG, eventuale forma/morale soltanto secondo comportamento approvato; esito, marcatori, squalifiche, supplementari/rigori e effetti applicati UNA volta. Per partita dell'utente OG 50 Hz invariato, risultato rientra nello stesso journal.
6. **Fixture journal + CAR**: identita' composta da (worldId, competitionId stabile, editionId, stageId, tie/fixtureId, leg/replay). Stato SCHEDULED/PLAYED e checksum, no duplicate/no rewind, ordine indipendente dove necessario. Save atomico, preflight, checksum, RNG, nessun azzeramento RAM nel test; CAR del mondo distinto. Database statici nei pack, nel CAR solo delta, RNG/stati ed eventi irripetibili. Ultime stagioni per-match; vecchie stagioni riepiloghi e fatti permanenti. Misurare compressione completa di TUTTE le gare prima di decidere se mantenerla per decenni: non deciso ora.
7. **Importer/editor**: sorgenti OpenSWOS, SWOS2020, Mod, piu' futuri tornei originali; convertire in definizioni versionate con ID normalizzati e report verificabile, NON dipendere da EXE esterni. Un nuovo tipo di regola C si implementa una volta e si riusa; editor grafico custom futuro, non prerequisito della prima coppa.
8. **Data NEW CAREER**: proposta approvata come direzione, NON implementata: inizio scelto dal manager dopo l'origine attendibile del world-pack, pre-simulazione cronologica di TUTTI gli eventi dall'origine fino alla data scelta. L'origine 1 giugno 2026 e' solo un esempio da verificare; default 1 luglio invariato. Nessun mondo antecedente all'origine inventato.

## TODO CANONICA — sette ATTIVITA', tre TRAGUARDI
Il solo PUNTO 1, audit statico, è [x]. Le attività 2–7 e tutti i gate di esecuzione rimangono [ ] e NON PROVATI; nessun adapter/fixture journal è stato implementato.

### TRAGUARDO A — fondamenta/sicurezza (1–3)
- [x] **1. Audit OG e matrice REUSE/ADAPTER/NEW (statico).** Mappare chiamate/side effect di `CalculateViewResult`, `DIY_PlayOrSimulateLeagueFixture`, `DIY_PlayCupMatchAndAdvance`, `Career_AdvanceToNextSeason_UpdateCompetitions`, `world_cups.c`, `world_conti.c`, `league_playoffs.c`, `career_init.c` e libreria upstream OpenSWOS `src/rules/`; documentare RNG, ID, overlay, slot, dati e licenze. **GATE:** si sa precisamente chi scrive risultato, calendario, classifica, save.
- [ ] **2. Identita' e single-writer per competizione/edizione/fixture.** Definire ID stabili per epoca, gara, fase/andata/ritorno/replay; integrare il binding manager senza reset; non attivare insieme old/new sim della medesima gara. **GATE:** zero gare duplicate, stesso contenuto aprendo due volte VIEW senza avanzamento.
- [ ] **3. Scheduler data ed esatta persistenza.** Riutilizzare clock civile gia' esistente; eventi ordinati, phase adapter OG, date cross-year, checkpoint CAR atomico e ripresa non mutante; proteggere save esistenti. **GATE:** stesso giorno, fase, RNG, fixture e classifica prima/dopo CONTINUE, nessun auto-reset il 1 luglio per le coppe ancora attive.

### TRAGUARDO B — coppa e tre edizioni (4–6)
- [ ] **4. Qualification resolver comune.** Da campionati e coppe residenti leggere partecipanti e quote; prevalenza detentori, posti a scalare, preliminari e nessun duplicato. **GATE:** campo dell'edizione seguente riproducibile dalle fonti corrette, senza seed dell'anno 1 riciclati.
- [ ] **5. UNA COPPA OG PILOTA — TRE EDIZIONI DI FILA.** Selezionare una coppa esistente dopo audit, non inventarla; gironi/turni, sorteggi/replay/andata-ritorno, fixture reali, manager vs CPU, premi e vincitori. Salvare e CONTINUE anche nei passaggi di stagione; il viewer e l'offerta leggono gli stessi dati. **GATE:** tre edizioni COMPLETE, ogni fixture simulata una sola volta, qualificazioni diverse quando i risultati lo determinano, nessun reset o doppia partecipazione accidentale. Confrontare il comportamento contro OG per regolamenti supportati.
- [ ] **6. Secondo formato e regressioni.** Altra coppa/lega con struttura diversa; test 96/26, cambi club, disoccupato, anni multipli, save/reload, cronologia e conversioni data. **GATE:** nessuna regressione del gameplay OG/CLASSIC/DIY/SEASON, no grafica/asset alterati e classifiche coerenti dopo hire.

### TRAGUARDO C — riutilizzabilita' (7)
- [ ] **7. Importatore + validatore e import batch.** Per ogni definizione: paese/epoca/team count/dates/qualificazioni/verifica formato/ID/seed/roundtrip. Rendicontare separatamente DEFINED, VALIDATED, RESIDENT, FIRST_EDITION_PLAYED, NEXT_QUALIFIED, SAVE_RESTORED e MULTI_YEAR_TESTED. Nessun PASS automatico per import puro. Solo allora importare molti tornei da SWOS diversi e aggiungerne custom.

**STOP IMMEDIATO:** perdita o sovrascrittura di save precedente; doppia partita/risultato, deriva classifiche, RNG non ripristinato, qualificate non fondate, mancata seconda edizione o regressione motore OG. Non iniziare il lotto successivo senza gate, diagnosi e decisione utente. Test statici, test C isolati e test Windows diversi; test Windows eseguiti dall'utente via GitKraken. Non aggiungere toolchain obbligatorie.

## Aperto / da decidere prima del punto pertinente
Pilota tecnico Champions OG 1996/97 scelto, con formato storico evolutivo B rinviato; origine documentata dei world-pack e data iniziale UI; conservazione per-gara di tutte le annate o archivio piu' compatto; precise quote/regolamenti delle coppe custom; applicazione forma/morale; compatibilita' CAR se lo schema cambiera'. Non decidere da soli sui casi ambigui.

## Fonti e distinzione stato
Audit 494 righe (NON 494 tornei): [SWOS_CROSS_VERSION_COMPETITION_AUDIT_20261117.md](SWOS_CROSS_VERSION_COMPETITION_AUDIT_20261117.md); mondi: [COMPETITIONS_TWO_WORLDS_SPEC.md](COMPETITIONS_TWO_WORLDS_SPEC.md); regole: [AGENTS.md](AGENTS.md); design: [GAME_DESIGN.md](GAME_DESIGN.md); codice/stato: [ENGINE_CODE_DISCOVERIES.md](ENGINE_CODE_DISCOVERIES.md), [PROJECT_STATE.md](PROJECT_STATE.md); test: [DEVELOPMENT_WORKFLOW.md](DEVELOPMENT_WORKFLOW.md). Questo commit cambia SOLO DOC, non `build_info.h`.
