## 21/11/2026 — RICONCILIAZIONE FINALE PREPARATI (RG-001/RG-014)

Nota di integrazione versione 00.13.229: il precedente helper booleano Expanded_BoliviaManagerActionableFixture e renderer 5f013159... sono preparati ST superati, NON il codice scelto. Il commit deve mantenere Expanded_BoliviaManagerActionableCup come risultato signed di exp_bolivia_manager_plan (1=esatta Copa OG, 0=League, negativo=conflitto/overdue: niente blu fittizio). Header 12a7cb..., source 1dda425..., overlay d7b93c..., e getter pre-01/07 readonly career_status.h3991af/c.ce52a4. L'overlay d7b93c conserva il controllo puntatori TSKC entro 8192 e mostra score solo se witness native OG univoco, altrimenti '?'. La successiva variante 5f013.. eliminava involontariamente il lookup storico: NON sovrascrivere. Niente nuove gare/simulazioni/CAR. Patch preparata ST; compilazione originale Windows, GP e dati storici REAL 2026 non provati.

## 21/11/2026 — REGRESSION VISUALE 00.13.228: CURSORE LIGA SU 04/09 QUANDO COPA 03/09 È LA PROSSIMA GARA

**Fonte USER screenshot reale Windows:** il 03/09/2026, manager Nacional Potosí, Copa Bolivia GROUP STAGE vs Real Tomayapo 03/09 visibile in elenco, ma il rettangolo blu seleziona la DPN contro GV San José 04/09. I pulsanti PLAY MATCH e SIMULATE MATCH sono visibili; il problema segnalato è il **selezionatore**, non prova di partita falsa o di crash. **Fonte codice verificata** `dev/0.10` HEAD `58058d9926da9af8aadf15ccbb22660a9a78bfce` build `00.13.228`: `menus.c::SelectActiveContestAndRound` evidenzia la futura Liga tramite TSKC; `expanded_economy.c::Expanded_UI_CompactFixtureRow` registra `_menuEntries[37]` come native highlight; `ui_render_merged_calendar` inserisce le gare COPA con highlight=0 e ordina per data ma **non trasferisce** l'highlight sul primo evento Copa. Parallelamente `expanded_midseason_join.c::exp_bolivia_manager_plan` seleziona già correttamente lo stesso OG residente Copa per i tasti PLAY/SIM: UI e azione hanno due proprietari diversi della riga attiva.

**Fix ST preparato, non nel branch / NON commit:** esporre `Expanded_BoliviaManagerActionableFixture` read-only, wrapper dello STESSO `exp_bolivia_manager_plan(...)` (ritorna 1 solo quando la gara Copa è quella azionabile). Alla visualizzazione ordinata, cercare **UN SOLO** record Copa con `year/month/day/stage/fixtureId/home/away` uguale a quell'evento: spostare unicamente il `UICompactFixture.highlight` da Liga a Copa. Niente `_teamSelectionKeyCacheIndex`, `_currentGame`, TSKC, resident OG, contatori, date, score, RNG o CAR alterati. In caso gara futura, conflitto di giornata, evento scaduto, identità dubbia/duplicata o altra modalità/mondo, lasciare l'evidenziazione originale nativa (fail closed). Se la Copa è fuori dalla finestra di 14 righe, spostare soltanto la finestra visuale per renderla visibile, mantenendo ordinamento cronologico; **le righe aggiunte sono un overlay readonly e non nuovi slot selezionabili OG/cliccabili**.

**Blobs completi:** `sorgenti/include/expanded_midseason_join.h` `eec3278059008e109936107bf5f335ac2c4b7272`; `sorgenti/src/expanded_midseason_join.c` `859c0fb72aee153750efe38cb524dedee712a759`; `sorgenti/src/expanded_economy.c` `5f013159818b1a430754bfa486ea9607e0add9b6`. Quest'ultimo conserva tutti i precedenti fix calendario DPN, COPA fase, date DD/MM, score Liga e Copa e i pulsanti. **QA isolato JS equivalente al ramo highlight:** 6/6 PASS (Copa 03/09 prioritaria, Liga prioritaria, duplicato fail-closed, identità avversario errata, scroll 14 righe, sorting). **NON è compilazione C reale**, WIN/GP su 00.13.229 NP.

**Test obbligatori:** screenshot 02/09 e 03/09 (selezione Copa 03/09 vs Liga 04/09), annullamento e conseguente selezione Copa rimasta pending; completamento manuale e simulazione, risultato OG una volta e successiva Liga 04/09 selezionata; Copa 09/09 e derby 19/09 con corretto focus; stessi giorni e doppie gare/club bloccate, scroll native + visual, focus gamepad/mouse, 9697 e Classic/DIY/Season identici, SAVE/EXIT/CONTINUE. Questa patch è **SOLO focus VISIVO**: non risolve separatamente il pre-01/07 storico con V ancora aperto RG-001. Stato ST bug V, fix blob preparati, Win build NO, GP corrected NO, commit NO.

## 21/11/2026 — PREPARATA BUILD 00.13.228: COPA MANAGER SIM, FASI INFO, SCORE, GIUGNO (NON WIN-CERTIFICATA)

**Fonte USER:** due screenshot Windows del lineup Nacional Potosí 03/09/2026, ZIP diagnostica 20261121-013154 e chiarimento: la partita è stata interrotta dal manager prima della fine. `NO_NATIVE_COMMIT` atteso; non dimostra crash o mancato commit della gara interamente giocata. L'immagine è il menu OpenSWOS `play_match_menu.c`, `UNIFORM` aggiunto dalla parte SDL/OpenSWOS (`uniform_menu.c`), `EDIT TACTICS` base, non un menu speciale Expanded.

**Problema SIMULATE — causa V 227:** stesso intercept ordinal7=PLAY e ordinal10=NEXT entravano entrambi nell'azione interattiva OG selector/mode=2. **Patch ST preparata 228**: contest ordinal10 cambia a `SIMULATE MATCH`, ordinal7 resta `PLAY MATCH`. La stessa funzione OG `Expanded_OGWorld_BoliviaManagerPlay(expected,simulate)` sceglie `DIY_PlayOrSimulateLeagueFixture(1,0)` per gruppo o `DIY_PlayCupMatchAndAdvance(1,0)` per KO SOLO nell'azione simulate; manual resta mode2. `Expanded_OGWorld_BoliviaManagerAutoSim()` blocca SetupPlayers solo in questo contesto. Non cambia RNG, algoritmo, scoring, TSKC ABI, 50Hz, classe torneo, altre carriere e file CAR. Il successo esige *un solo* incremento native, witness identico e postmatch save unico. PROVE GP full e WIN build sono NP.

**Fasi — regola nuova permanente USER:** la fase di ogni partita nel calendario deve combaciare con quella mostrata in COMPETITION INFO **per tutte le competizioni**, dalla stessa fonte/regola. La Copa ha classificazioni `GROUP STAGE`, `DERBY` (cross-group, resta un incontro della fase gruppi), `QUARTER-FINAL`, `SEMI-FINAL`, `FINAL`. Header `bolivia_cup_sequence.h::BoliviaCup_PhaseName` è dictionary unico, mentre i valori per-fixture provengono da `BoliviaCup_ChronologicalFixtures.isDerby` o `kBoliviaKO2026[serial].round` solo quando OG conosce coppia/qualificati. `diy_competition.c::DesignDIYCupOnDraw` usa la stessa table per QF/SF/F; le informazioni 4 gruppi/3 punti/6+2 derby/prime due restano nel viewer. **Limite font 320×256, 14×9px, colonne x65 36px e x102 68px:** row mostra `COPA` + fase, abbreviando la competizione; nome completo `COPA BOLIVIA` nel viewer. Test screenshot a font reale obbligatorio, non dichiarare full-name in row se manca (compromesso). Non creare un secondo calendario/type2 stage.

**Score OG:** `expanded_economy.c` conserva precedente patch Liga che ricostruisce numeri entry39/41 anziché separatore '-', e per Copa proiettata usa la nuova `Expanded_OGWorld_BoliviaManagerScore` readonly sul journal OG. Chiave riconcilia world2627/country45/slot2/edition/date/ordered pair e global serial gruppo 0..63, KO home/away+day+edition (round native confermato dal getter KO). Mostra solo punteggi esistenti, 0-0 valido; future `V`, conflitto `?`. Nessun score inventato, nessuna modifica al record CAR o alla fonte OG. Test post-03/09 + KO e SAVE/EXIT/CONTINUE ancora NP.

**Date giugno:** fonti 2026 FBF/Unitel 03/06 e La Razón 01/06 confermano calendari sostituiti e rinvii per proteste/Mondiale. Nacional Potosí non risulta aver disputato Liga a giugno; vs Aurora (programma giugno) disputata 09/07 2–0, vs Real Oruro (12/06 previsto) 20/07 1–4; una gara Liga 01/06 Univ. Vinto–Aurora giocata, quindi nessuna pausa globale automatica. Il template OG 227 ha solo date rappresentative delle fasi, NON 240 fixture verificati: preservare data originale, rinvio e motivo; non modificare in questa build i bytes delle fasi o giocare in giugno una gara annullata.

**Sorgenti PREPARATI, non ancora tree/commit**: career_status.h `59cc9f2802e37d3c6701a15134e42f9b9f428cdf`; career_status.c `b9d6207a48b72dc9f08b6e3392c68780e0e5f9c0`; diy_competition.c `9da33b7daa75e8e7989d21bc97f7080bae7353fa`; expanded_midseason_join.c `cfc1c10284a8b6bd9c7957f9e5379dc2a396a0f7`; expanded_economy.c `be16b268c32de65a85415aa74cae4c64756eb448` (comprende tutte le precedenti patch calendario!); bolivia_cup_sequence.h `3728c86d69d87f669134c140255716dde61f211d`; build_info.h `79252a3ddfcdd043b3a889e1689ad8b22f9e957f`. **Non è un EXE, non è stato eseguito gameplay o compilazione Windows nuova.** Gate: Windows MSYS2 SDL2 full build, native fixtures 64+14 sim+manual, 0/1 witness canceled/completed, scrollbar 14 righe, group/derby/KO/info, CAR legacy e CURRENT SAVE/CONTINUE, 9697/2627, CLASSIC/DIY/SEASON, RNG, no collisioni. RG-001/004/005/006/014/015/016/022 e RG-T prima di dichiarare PASS.

## 21/11/2026 — PRECHECK AGGIUNTIVO: LA COPA GIOCATA CONTINUA A MOSTRARE V (SCORE OG NON PROIETTATO)

**Nuovo V da sorgente 227:** `career_status.c::Expanded_OGWorld_BoliviaManagerFixtures` legge tutte le 8 partite Cup del manager dalla tabella 64 gruppi e scrive un `ExpandedBoliviaManagerFixture` con data, coppia, serial, fixtureId, stage, opponent, MA non score. `expanded_economy.c::ui_render_merged_calendar` copia ciascuna nel calendario visuale con `mark=\"V\"` INCONDIZIONATO, anche quando `s_expOGBolivia.comp.matchesPlayedCount` ha gia' avanzato e OG ha prodotto un vero risultato. **Questo e' distinto** dalla correzione `-` Liga nel source preparato 33fec535663e3969aa52ce0a1f981dec0e1be9af: la Copa played non viene risolta dai punteggi locali `_menuEntries[39]/[41]` (appartengono ai soli slot TSKC).

**Next build necessario:** API readonly sul registro risultati *OG residente/journal*, indicizzata da mondo/paese/slot/edizione/tipo/fase/fixtureId e coppia home/away verificata; per le righe Copa, usare punteggio OG solo quando witness+record sono presenti e coerenti; pendenti mostrano V, quelle di stato incoerente NON devono inventare punteggi. Preservare packed ABI, manager vs CPU unica gara e save cross-process. Nuovi test RG-014/227-COPA-PAST-SCORE-PROJECTION, RG-004/005/006/022: prima e dopo 03/09, gruppi+KO con result/witness native, salvataggio/caricamento, scroll. WIN/GP NP. **Non dichiarare che source blob del renderer abbia gia' corretto questo secondo bug.**

## 21/11/2026 — SEGNALAZIONE UI DA SCREENSHOT 227, NEXT BUILD: COPA BOLIVIA, RISULTATI E STORICO 01/07

**Fonte WIN/USER:** screenshots 01/07 Nacional Potosí e 08/08 David Wilkin, ramo dev/0.10 HEAD c1ec6b6021adf7db9878b08f0b01b738dddcc61e v00.13.227. Su 01/07 compaiono gare Liga 08/05, 12/05,16/05 con `V`; su 08/08 28/07 e 04/08 mostrano `-` invece del punteggio; etichetta della Copa proiettata e' `COPPA` italiana. **Volonta' USER:** sostituire etichetta con **COPA BOLIVIA**, visualizzare i risultati veri gia' posseduti da OG al posto del trattino, e verificare che le pre-01/07 siano archiviate/importate senza risimulazione. Correggere nella prossima build e non confondere visuale con import storico.

**Causa V solo GUI per `-`:** `menus.c::CareerGameListCommon` (source lines 8452-8509) imposta per un match played `_menuEntries[39].fgWord` (punteggio del club) e `[41].fgWord` (opposto), con `[40].fg` separatore `_aMinus` e tutti e tre visibili. `expanded_economy.c::Expanded_UI_CompactFixtureRow` (source lines 2291-2298) legge entry40 in `r->mark`, poi sintetizza `x-y` solo con `if(!r->mark[0] && ...)`: la condizione e' falsa per `-`. Fix visual-only: quando score39+41 VISIBILI e validi, mostra `x-y` nel renderer, altrimenti `V` per match pending; mantenere orientamento scelto da native (NON rileggere teamId raw, evitare RG-004/006). Nessuna modifica a result, fixture o CAR.

**Causa V etichetta:** due hardcoded `snprintf(...,\"COPPA\")` nel renderer (row tipo1/2 Bolivia, merged cup). La colonna competizione e' 36px x65 mentre il campo secondario x102 ha 68px (FONTS): per Copa Bolivia residente usare `COPA` nella prima e `BOLIVIA` nella seconda, ottenendo il nome completo su due campi con il font originale senza overflow. Per altra Cup nazionale non sostituire il nome con Copa Bolivia se diverso; CLASSIC/DIY/SEASON e 9697 invariati. **Questa e' la proposta di layout per la prossima build, da certificare con screenshot.**

**Import storico ancora APERTO:** `career_init.c::Expanded_OGWorld_Start` usa `expanded_og_world_run_year(year,0)` poi `Expanded_OGWorld_AdvanceToDate(01/07)` che fa avanzare OG fino al cutoff. Presenza delle pre-luglio e' normale, ma risultati REALI verificati al 01/07 non si deducono da un catch-up OG: serve distinguere score resid/matchFlags/TKSC e fonte storica reale per paese; non copiare real-score preluglio per template 2027+, non inventare match. RG-001/PRE-JULY-BOLIVIA-HISTORIC-SCORES e RG-014/227-SCREENSHOTS-RESULT-DASH-COPA-LABEL. **Gate** 01/07 e 08/08, V vs 0-0 vs 1-0, score native e journal, home/away, 14 row scroll, REAL 2026 vs future template, WIN/GP e CAR legacy no-replay. Stato ST V per cause UI, NO Windows fix tested, import storico NP.

## 21/11/2026 — ROADMAP MOTORE: CALENDARIO CIVILE PER GARA APPROVATO, DOPO CHIUSURA BUG 227 (NO CODE)

**Decisione USER vincolante per FUTURA fase Expanded/Career:** sostituire le fasce native a 1/8 di mese come autorita' della DATA con il calendario SWOS Expanded: OG non perde la propria autorita' sportiva (gara 50 Hz, manager/CPU, accoppiamenti e progressione sportiva, risultati, classifiche, KO, qualificati, trofei). **Non creare due calendari, due clock o due simulatori.** Le vecchie indicazioni «OG autorita' anche della data» restano storia del codice 227, NON criterio target dopo migrazione. L'utente dispone esplicitamente che questo sia il lavoro **SUCCESSIVO alla build che risolve gli ultimi bug**: non introdurre il nuovo sistema nella build di chiusura/QA 227 a meta'.

**ESISTE GIA' UNA PARTE DEL DISPATCHER, NON REIMPLEMENTARLA (verifica ST, dev/0.10 HEAD c1ec6b6021adf7db9878b08f0b01b738dddcc61e, 00.13.227):**
- `sorgenti/include/expanded_event_scheduler.h`: `ExpandedScheduledEvent` con `id`, `requires_id`, `day_index`, `priority`, `kind`, `state`; `ExpandedEventScheduler_Init/Schedule/Next/Complete/ValidateRestore` pure inline gia' presenti. E' un **ordinatore di eventi** deterministico, non ancora un dispatcher OG live, e non serializza il CAR; `Complete` puo' essere chiamato soltanto dopo commit OG+CAR affidabile.
- `sorgenti/src/career_init.c::Expanded_CompetitionCoordinator_Start/Advance/PreflightFixture`: oggi coordina `Expanded_OGWorld_AdvanceToDate` seguito da `ExpandedWorldLive_AdvanceToDate`, ha recovery-latch e preflight; `exp_coordinator_shadow_july` usa lo scheduler in pila solo per PEEK al 30/06, `fixtures=0 dispatched=0 CAR-ACK=NONE`. E' un pezzo reale del collegamento OG/clock, **non** un calendario per-fixture live.
- `sorgenti/src/expanded_world_live.c::ExpandedWorldLive_AdvanceToDate` possiede la data civile, ma aggiornare il cursore NON risolve un incontro OG; `expanded_midseason_join.c::native_cutoff_at` conserva il mapping date→fase; `diy_competition.c::DIY_AdvanceLeagueProgressAfterMatch` lega contatori OG, progressione, `phaseStartMonths/viewRandomPhase` in una routine. Migrazione necessaria alla frontiera fra evento e OG, non patch cosmetica della UI.

**Priorita' cronologica e deliverable misurabili:**
1. **Gate corrente / build 227:** ottenere da utente un vero WIN/GP per Copa Bolivia manager, KO, corretta visibilita' da COMPETITIONS e calendario DPN/COPPA/DD/MM, no collisioni Liga/Copa, CAR; affrontare separatamente RG-012 (cup future: ranking OG non certificato). Non dichiarare i bug risolti dal solo codice/ISO.
2. **Prossima fase motore, solo dopo gate:** definire un registro UNICO di fixture/eventi con data civile DD/MM/YYYY e identita' stabile world/paese/slot/edizione/type/fase/fixtureId/coppia, ordinato dall'EventScheduler esistente; l'unico clock civile gia' presente va condiviso e non duplicato. `NEXT MATCH` prossimo fixture del manager, `FORWARD TIME` prossimo evento mondiale, anche eventi fissi di precampionato quando non ci sono gare.
3. Collegare il `CompetitionCoordinator` al dispatcher OG per giocare/simulare lo **stesso** fixture una volta, consumare result/witness OG e stato evento in maniera atomica col CAR; non usare LWE, viewer o keycache come score-writer. Per type0/1/2 separare progressione sportiva dei round dai byte delle date, preservando abbinamenti, risultati, ranking e KO nativi. Nessun vecchio CAR riscritto/reinterpretato arbitrariamente.
4. Risolvere date individuali dentro la giornata (8 gare Bolivia non un unico giorno) e per tutte le competizioni, incluso no due partite/club/giorno a livello mondiale, rinvii con originale/motivo, date preluglio storiche importate al 1 luglio, eventi KO solo dopo qualificazioni, edizioni oltre 1 luglio/anno civile, template REAL_2026 vs GAME_TEMPLATE_2026.
5. Vertical slice Bolivia TYPE0 Liga 30 giornate (240 fixture) + TYPE2 Copa 64+KO14, manager vs CPU, pre-season senza partita, prossima partita / eventi mondiali, sorting e collisioni, fase/standings OG. Poi estensione progressiva 9697/2627 e tutte le competizioni: non certificare il motore globale dopo una sola lega.
6. Regression precheck OG RG-001/004/005/009/012/014/017/022/026, packed ABI, RNG, TSKC, locale/residente, qualifica, club change, F12 readonly; RG-T01/02/03/04/05/09/14/16/19/20 + cross-process SAVE→EXIT→CONTINUE (legacy CAR), anno multiplo, 9697/2627, CLASSIC/DIY/SEASON invariati e 50Hz. ST/ISO prima, poi build Windows+GP, nessun falso PASS.

**Risultato attuale:** ROADMAP APPROVATA; nuovo dispatcher per-fixture NON IMPLEMENTATO, nessuna Windows build o gameplay in questa richiesta, nessun commit. Guide correlate: `GAME_DESIGN.md` blob b54dc1dbda47b70c7a1ee401964f8ae4949192b4; RG blob fdc6a2370c0c7c2611ee23bcb3183225fafd490c in Issue #1. Le seguenti sezioni precedenti documentano il comportamento reale 227 e non vanno cancellate.

### 21/11/2026 — Bolivia 00.13.227: calendario TYPE0 reale approssimato, Copa club/manager (PREPARATO, NON CERTIFICATO GP)

**Precheck RG obbligatorio:** il campionato Bolivia `_bolivianLeague` è TYPE0, NON la Copa TYPE2. Il writer che popola realmente le 30 fasi OG è `diy_competition.c::DIY_RecomputeCompetitionDerivedProgressionTable` chiamato da `InitDIYCompetitionFromTableEntry`, NON `DIYTournament_BuildMatchesPerTeamProgressionTable` (TYPE2). Il primo hook temporaneo preparato su quest'ultimo era un TENTATIVO FALLITO individuato prima del commit; usare SOLTANTO il source blob sostitutivo `0f49edac6c15b7c40b48c31597197c5d2d557ace`. `data_json/worlds/2627/competitions/leagues.json` precedeva il baked record imponendo 32/88 (May1–Dec1). Il nuovo preparato JSON `b802957b9f5eea4d5a34980ba65561ed3e17a9fa` allinea 24/92 al baked; per NEW START l'OG TYPE0 usa 30 fasi in `sorgenti/include/bolivia_league_2026_phases.h` (`ddfcda2...`), J18=04/09/2026, range Apr1–Dec12. Non dichiarare date civili esatte per le otto gare di ogni round: sono un SOLO evento di giornata con precisione 1/8 mese, i recuperi reali 2026 J6–10 e lo snapshot 01/07 non sono ricostruiti. Le date esterne 2027+ sono GAME_TEMPLATE_2026 e NON ufficiali future. 0 coincidenze Liga–Copa nel calendario 2026 rappresentativo dei 30 round vs 64+14 date Copa (test ST/JS), non garanzia di collisoni per tutte le competizioni del mondo.

**Copa type2 / manager assunto:** diagnostica WIN 226 `03/09 2d06 vs 2d0a` → `INVALID teamA=-1 teamB=6` a causa di guard CPU non eccezionata durante l'ORIGINALE OG giocato dal manager. Il source preparato 227 `bolivia_manager_offset_gate.h` (`226b722...`) consente un solo `-1` solo con `Expanded_OGWorld_BoliviaManagerInteractive()`; writer `diy_competition.c` `0f49ed...`, restore positivo nel CAR del bridge esistente. Non sostituire il match OG né rimuovere il guard per CPU/KO/CLASSIC. Test positivo/negativo e screenshot gameplay 50Hz + journal + SAVE/EXIT/CONTINUE necessari prima di chiamarlo funzionante.

**Limite di applicazione UI:** solo nella CAREER assunta di club Bolivia nel mondo2627 il nuovo renderer usa DD/MM/DPN/COPPA e merge: il primo preparato `expanded_economy.c f186d1...` era troppo ampio per 9697/CLASSIC/DIY/SEASON ed e' stato sostituito con `9aae4c...` che conserva nomi e colonne della normale UI negli altri contesti. Non trattare come modifica globale dei menu.

**Viewer e calendario del club:** `menus.c::ViewCompetitionsOnReturn` non conosce la Copa type2 perché usa 4 slot locali. `career_status.c` `8d0921...` aggiunge callback readonly `Expanded_OGWorld_ViewBoliviaClubCup` al residente GIA mostrato nel VIEW WORLD; `menus.c` `69f854...` aggiunge `COPA BOLIVIA` nel secondo slot grafico solo per club boliviano assunto e locale vuoto, SENZA trasformare tipo2 in tipo1. `Expanded_OGWorld_BoliviaManagerFixtures` enumera le 8 partite del girone (e soltanto KO effettivamente noto) dal contesto OG; `expanded_economy.c` `9aae4c...` fonde COPIE VISIVE delle righe TSKC con i veri match Copa e le ordina per data, limitando il pannello a 14 righe e scroll originale. `DPN` invece nome lungo Liga, `COPPA` giallo IN riga, data DD/MM a sinistra, nessun banner giallo esterno; `PLAY MATCH`/NEXT MATCH restano etichette originali. La ricostruzione delle coppie non deve MAI cambiare TSKC/CAR, team selection o risultati. Test ISO JS simula 8/8 Cup visibili per tutti i 16 club tramite scorrimento pagina, NON è prova SDL renderer.

**Gate QA:** compilare ISO C `tests/run_test_bolivia_227.sh`, poi full Windows MSYS2 build e debug F12 vs GP, 96/26 assunto/disoccupato, SAVE/CONTINUE e anni multipli, Copa gruppi/KO/terze fasi, calendario per club unico giorno, 30 round e 64+14 una sola volta, vecchi CAR intatti, CLASSIC/DIY/SEASON, font glifi/hitbox. Prima di altri fix leggere sottocasi RG aggiornati e log fresco, aggiornare Issue #1 e guide dopo ogni scoperta.

> **AGGIORNAMENTO OPERATIVO 20/11/2026 — REGRESSION GATE (VIGENTE).** Le [dieci regole di collaborazione](AGENTS.md), i [gate di analisi e test](DEVELOPMENT_WORKFLOW.md) e il [registro trasversale di 32 incidenti + 24 test e dipendenze](REGRESSION_HISTORY_AND_DEPENDENCIES.md) sono obbligatori prima di modificare una routine del motore. Il presente manuale mantiene gli approfondimenti effettivi dei commit 00.13.217–225 e non va duplicato nel registro. Ogni spiegazione distingue causa V/I/?, intervento ST/ISO, prova CPU Windows e prova manager gameplay/CONTINUE. La diagnostica 225 oggi documentata in Issue #1 dimostra per Copa Bolivia 2026 e 2027 **64+14 match OG CPU**, ma **non** collegamento al calendario manager; WorldCups next-year ha ancora quote senza prova di final places. Nessuna patch al motore è implementata da questa nota; il lavoro va preceduto da impact map writer/reader, scelta test e consenso sui compromessi.

### 21/11/2026 — RG-014 00.13.226 prepared bridge, QA checklist (NO gameplay PASS)

Nuovo live path preparato: readonly Expanded_OGWorld_BoliviaManagerNext legge dal contesto natìvo type2/KO le coppie e le date verificate 2026 con 2026_GAME_TEMPLATE per anni successivi; menu CAREER espone il prossimo evento e intercetta PLAY/NEXT/FINISH senza inserire type2 in slot1/2 (panic 0x4459). Expanded_OGWorld_BoliviaManagerPlay invoca l'ORIGINALE OG sul residente una sola volta, richiede un nuovo witness e la progressione esatta prima di mutare il CAR; managerTeam offset -1 e' solo transitorio, poi indice CPU. TSKC packed locale non si tocca: guardia stats sul suo writer; selectedTeams private heap snapshot (SaveSelectedTeams ha solo un backup statico), RNG byte privato. Date: Cup prima del prossimo native match si gioca; stesse date/overdue bloccati senza inventare rinvii; il coordinatore unico aggiorna la data civile. Documentare eventuali collisioni dopo Windows. La UI attualmente mostra SOLO il prossimo fixture, non 8 righe in un elenco.

Test Linux ISO: cinque casi gate calendario su copia manuale PASS con GCC -Werror. New test+runner nel repository preparati ma non eseguiti qui senza checkout. Test **OBBLIGATORI Windows, NON PROVATI**: nuovo assunto Nacional Potosi 2D0D il 01/07/2026, primo vero fixture 18/08 Guabira-Nacional, non salta Liga del manager; gioco MANUALE 50Hz e punteggio in resident OG; KO andata/ritorno QF/SF/finale 14 match 1 volta; 2027 promoted; KO dopo eliminazione; CANCEL no journal/save; week/date/collision audits; F12 CPU/manager, SAVE/EXIT/CONTINUE; 9697, CLASSIC/DIY/SEASON e CAR legacy. In caso di fallimento non qualificare engine come completo, documentare subito nuova scheda RG e fix. 00.13.226 e' codice PREPARATO, non EXE PASS.


### 21/11/2026 — RG-014 preflight: calendario manager Bolivia, stato PREPARAZIONE

Mappa verificata (HEAD d19694a0, codice 00.13.225): il residente OG type2 ha le 64 coppie datate e il KO OG type1 le 14; lo STOP CPU `DEFER_MANAGER` precede il risultato. CAREER usa solo 5 slot originali e TeamSelectionKeyCache; career_teams.c::Career_PlayMatchMenuLoop e NextMatch non possono dereferenziare un sesto slot, e slot1/2 rigettano type2 con panic 0x4459. Ogni route manager deve: (a) leggere l'esatto prossimo fixture dal contesto residente e confrontarlo col prossimo evento locale; (b) presentarlo nel calendario come evento OG, non simulare un calendario alternativo; (c) invocare il SOLO DIY_PlayOrSimulateLeagueFixture o DIY_PlayCupMatchAndAdvance sull'identica coppia/fase con un solo witness; (d) rispettare il sentinel -1 della careerTeam, senza lasciarlo nei residenti o nel CAR; (e) evitare scritture TSKC da Career_UpdateStatsAfterMatch senza slot, e preservare RNG, viewer readonly, selectedTeams e input; (f) provare cancel/retry, KO, 2027 promoted, sacking, collisioni giornaliere, SAVE/EXIT/CONTINUE e 9697/CLASSIC prima di dichiarare completato. Il test CPU della 225 NON prova nessuno dei punti manager.

Fonte: sorgenti/src/career_status.c (6021+, 6333+, 9604+); career_teams.c (1850+, 2003+, 2076+); diy_competition.c (12333+, 11893+); menus.c (3910+, 3980+); expanded_midseason_join.c (902+, 981+); registro RG-014/NATIVE-MANAGER-TSKC. Causa V dal codice; gameplay Windows e CAR NP. **Questo checkpoint e' mappa/decisione tecnica, non una build.**
>
# SWOS Expanded — Manuale operativo per implementare e modificare competizioni OG
## Lezioni reali dalle build 00.13.217–00.13.225 (20 novembre 2026)

**Scopo:** ridurre il numero di build necessarie per ogni futura competizione. Questo è un playbook tecnico riutilizzabile, non la dichiarazione che tutte le competizioni siano già integrate.

**Ramo di riferimento:** `dev/0.10`; ultimo commit esaminato: [00.13.225 — `57c75f4455077086148ed3631a7a4dd50dae97d5`](https://github.com/Giuzy/SWOS-Expanded/commit/57c75f4455077086148ed3631a7a4dd50dae97d5). Serie studiata: 217, commit intermedio di soli dati, 218, 219, 220, 221, 222, 223, 224 e 225. Sono stati confrontati diff GitHub, codice effettivo, guide nate con ogni versione, deposito [Issue #1](https://github.com/Giuzy/SWOS-Expanded/issues/1) e discussioni del 19–20 novembre 2026.

**Evidenza al momento della redazione:** i log Windows della 223/224 avevano verificato **Copa Bolivia 2026: 64 partite OG dei gironi + 14 OG KO e campione**. La 224 era stata compilata su Windows, ma nel 2027 offriva soltanto l'elenco delle squadre. L'utente riferisce oggi che nella **225 sembrano funzionare anche la seconda e la terza stagione**; non sono ancora stati analizzati i nuovi ZIP di diagnostica, quindi NON dichiarare PASS Windows 225 per CAR, manager o cicli pluriennali sulla sola base del messaggio. Distinguere sempre **CODICE**, **C11 isolato**, **Windows compilation**, **gameplay**, **SAVE/CONTINUE**.

---

## 1. Le cinque regole che non si possono infrangere

1. **OG (OpenSWOS) è l'unica autorità.** Solo i veri algoritmi OG inizializzano competizioni residenti, giocano o simulano partite, assegnano risultati, ordinano classifiche, producono qualificati, trofei e avanzano i turni. L'Expanded World/Competition Coordinator può **selezionare evento, identità, coppia, data e stato**, non creare un secondo risultato o orologio.
2. **Ogni incontro è un evento unico con identità stabile.** Chiave concettuale: `world + paese + slot/identità torneo + edizione + fase + fixtureId/turno/leg`. `compId` da solo **non basta**: il 164 boliviano ha già causato semifinali duplicate. Il journal OG e il cursore CAR impediscono replay e doppie callback.
3. **START inizializza la situazione corretta anche da disoccupato.** Quando le partecipanti sono note, il residente nasce con tutte le gare future a **zero risultati**. Il manager che firma o cambia club si collega allo stesso mondo OG, senza rifare sorteggi, resettare la stagione o inventare partite.
4. **Viewer e REGOLE sono letture, non un secondo torneo.** Mostrare partecipanti appena noti, senza aspettare una cerimonia di sorteggio; se sconosciuti, ultima edizione OG **veramente archiviata** in sola lettura e con anno esplicito, altrimenti «indisponibile». L'apertura non deve cambiare stato, RNG, squadre o contatori. Un tabellone visualizzato NON prova una gara giocabile.
5. **Non rompere dati/ABI/salvataggi/classici.** Preservare gioco OG 50 Hz, controlli, file CAR, CLASSIC/DIY/SEASON; niente netplay non richiesto. Niente refactor arbitrario o modifiche ai rami `main`, `release/0.20` o backup. Ogni file/blob/test/decisione preparata va registrato **subito** nella stessa Issue #1 prima della cosa successiva; commit unico **solo** dopo «committa».

### 1.1 Calendari: distinguere tempo reale, prima carriera e futuro

- **Prima carriera 2026/27:** per tutte le competizioni ricostruire le **date reali di ciascuna gara/turno**, inclusi preliminari, recuperi, andata, ritorno, finali. Se al **01/07/2026** partite o tornei erano già iniziati, importare il **vero stato storico** paese per paese senza risimularlo né trasferire il calendario passato dopo luglio. Un'edizione a cavallo del 1 luglio sopravvive al cambio anno del manager.
- **Nuova regola approvata il 20/11/2026, mondo 2627:** una volta decisi formato e date per-gara del **2026**, ripeterli nei successivi anni di **gioco** (stesso giorno/mese, anno aggiornato) finché un aggiornamento esplicito non cambia le definizioni. `REAL_2026` non deve essere confuso con `2026_GAME_TEMPLATE`: per 2027+ **non** si finge di conoscere il calendario ufficiale futuro. Nuove squadre, qualificati e punteggi **non** si copiano dal 2026; provengono dalla stagione OG corrente.
- **Mondo 1996/97:** l'utente preferirebbe cambiare formati **nell'anno storico in cui cambiarono veramente**, con dati/regolamenti verificati per nazione e torneo. **È una proposta ancora da definire in dettaglio**; costruire prima una timeline delle riforme e sottoporla alla decisione, senza anticipare una logica.
- **Una squadra, al massimo una partita nello stesso giorno civile**, considerando TUTTE le competizioni. La sola verifica interna di una coppa non certifica le collisioni con la lega. Dove necessario è autorizzata la correzione della **sola data** di una gara di campionato, conservando data originale, nuova data e ragione; non alterare abbinamenti, risultati o fisica.
- Le fasce native OG `beginMonth/endMonth` sono **ottavi di mese**, non date civili esatte: `56` corrisponde ad agosto; cambiare le fasce non equivale a creare 64 fixture con giorno vero. Usare date civili per il dispatcher.

## 2. Dove intervenire: mappa del repository

| Scopo | File/strutture reali | Perché |
|---|---|---|
| Definizioni dati mondo | `data_json/worlds/2627/competitions/*.json`, `data_json/worlds/2627/teams/*.json`; equivalenti 9697 | Catalogo da validare; JSON presente **non** significa residente/giocabile |
| Import dal mondo | `scripts/world_import.py` → `sorgenti/src/world_data.c` | `world_data.c` è generato: non modificarlo manualmente come unica definizione permanente |
| Composizione START e avanzamenti | `sorgenti/src/career_init.c`, `expanded_og_world_run_year` | Ordine: residenti OG lega, coppe, dipendenze e tempo civile; fallimenti bloccano l'avanzamento |
| Residenti, manager, journal, save e viewer | `sorgenti/src/career_status.c`, `ExpandedOGLeague`, `ExpandedOGCup`, `s_expOGBolivia`, `s_expOGCups` | Stato autorevole delle edizioni, fixture OG, hash, RNG, contatori, ritorno alla UI |
| Funzioni match e menu OG | `sorgenti/src/diy_competition.c`, `DIYCompetitionEx`, `DIYTournamentState` | Inizializzazione type2/type1, righe standings, `leaguePhase.cal`, REGOLE |
| Vecchi hook di coppa | `sorgenti/src/world_cups.c` (`cup_under_way`) | Possono identificare impropriamente un'altra fase per solo `compId` |
| Fixture e date gruppi Bolivia | `sorgenti/include/bolivia_cup_sequence.h` | `kBoliviaCupGroups`, `kBoliviaCup2026`, `BoliviaCup_BuildGroupFixtures`, `BoliviaCup_ChronologicalFixtures`, validator |
| KO Bolivia | `sorgenti/include/bolivia_cup_knockout.h` | `kBoliviaKOGroups`, `kBoliviaKO2026`, `BoliviaKO_Validate` |
| Tipi/ABI | `sorgenti/include/game_types.h` | `CompetitionCup` type1 ≠ `CompetitionTournament` type2; `TeamFileEx` contiene 16 giocatori OG |
| Test C isolati | `sorgenti/tests/test_bolivia_cup_sequence.c`, `test_bolivia_ko_sequence.c` e rispettivi `run_test_*.sh` | Validano soli dati/regole, non partita/Windows |
| Test suite e sviluppo | `sorgenti/tests/run_test_engine_suite.sh`, `docs/DEVELOPMENT_WORKFLOW.md` | Suite statica/headless e matrice test Windows |
| Salvataggi e spazio | `sorgenti/docs/SAVE.md`, `docs/ZSTANDARD/`, F12 in `sorgenti/src/platform_sdl.c` | CAR unico compresso Zstandard statico livello 3, journal e archivio |
| Font e UI | `sorgenti/docs/FONTS.md`, `sorgenti/src/diy_competition.c` | Glifi, `GetTextWidth`, `DrawMenuText`, buffer e larghezze delle celle |
| Stato/protocollo | `docs/AGENTS.md`, `docs/PROJECT_STATE.md`, `docs/ENGINE_CODE_DISCOVERIES.md`, `docs/GAME_DESIGN.md`, `docs/IMPROVEMENTS.md`, Issue #1 | Vincoli, fatti reali, checkpoint e decisioni |

**Non copiare automaticamente le costanti Bolivia in tutti i paesi.** Sono un esempio di **tecnica** (adattatore a un solo OG), non un formato universale. Controllare divisioni, tipo nativo, quote, fasi, regole di ogni competizione.

## 3. La sequenza giusta: quattro fasi prima del «giocabile»

### A — Catalogare prima di programmare

1. Inventariare **tutte** le competizioni dei due mondi, distinguendo leghe, coppe nazionali, continentali, nazionali, qualificazioni, spareggi e tornei con due edizioni nello stesso anno.
2. Per ogni torneo registrare: `world`, paese/ente, slot OG, ID stabile, denominazione, edizione, stagione civile, partecipanti OG e criterio d'ingresso, forma (type0/1/2), fasi, bye, promozioni, tie-break, andata/ritorno, regole 3 punti/sostituzioni/away goals, date match-by-match, regolamenti e fonti.
3. Validare i veri **ID del database** del mondo specifico, mai usare ordinals trovati in un'altra versione come identità immutabili. Le promosse possono avere ordinal >15.
4. Registrare stato al 1 luglio e fonti reali dei risultati **antecedenti**. Distinguere dati verificati, stimati, sconosciuti e calendario-template futuro.
5. Archiviare dati e fonti nel JSON dedicato, con campi di stato (`DATA_ONLY`, `NATIVE_RESIDENT`, `DATED_OG`, `MANAGER_CONNECTED`, `CAR_VERIFIED`). **Non chiamare implementato ciò che il runtime non consuma.**

**Errore reale:** il commit dati `5c66d9b3` conteneva già gruppi boliviani e calendari Argentina/Paraguay, ma nessuno dei tre tornei era ancora un nuovo evento OG. Lo schema dati è il primo passo, non l'ultimo.

### B — Validare senza crash

Validatore puro, prima dell'inizializzazione OG:
- Squadre esistenti in quel mondo/paese, identificatori distinti, nessuna iscrizione fuori quota, count esatto per formato, niente due fasi incompatibili nello stesso slot/anno.
- Gruppi/formato: numero club; calendario coppie; home/away; derby; eventuali riposi; qualificati plausibili, esatta cardinalità dei turni; date civili valide incluse bisestili/recuperi.
- Identità fixture `(world, paese, slot, edizione, fase, fixtureId, leg)` unica; un `compId` numerico può essere usato anche in altro contesto. Nessuna doppia gara per club/giorno **all'interno del torneo** e poi **tra tornei**. Verificare il matchday originale e la data cronologica separatamente.
- Capacità vera delle strutture OG, non limiti arbitrari: per esempio `EXP_OG_MAX_TEAMS 36` per alcuni residenti type2, `EXP_OG_MAX_CUP_TEAMS 64` per gli OG cup bracket; non reinterpretare/sovrascrivere buffer oltre capienza. Il journal ha crescita dinamica con controlli di memoria/disco.
- Verificare compatibilità dei campi e puntatori **packed**/64-bit/ABI, dimensioni reali, indici e buffer `MenuEntry`. In caso di errore: identificare file, campo, ID, contesto e `FAIL_CLOSED`, mai panico o dati fabbricati.
- Eseguire test C11 isolati per dati; non dichiarare gameplay con un `PASS` puramente statico.

### C — Collegare a UN SOLO residente OG e renderlo giocabile

**C1. Individuare il vero tipo nativo.**
- **Type0:** lega e relativi risultati/stan­dings; niente secondo calendario di campionato.
- **Type1:** coppa/KO con round, bye, tie-leg; usare il residente `ExpandedOGCup` e le API OG cup.
- **Type2:** gironi e fasi successive; usare `CompetitionTournament` e il vero stato OG di league-format. NON castare byte di `CompetitionCup` type1 a type2! Nella Bolivia l'originale `kWorld2627_worldBolCup` era type1 (10 entranti +6 bye) e mostrava falsi ottavi; doveva essere disabilitato prima di costruire type2.
- Competizioni con formato diverso richiedono adattatori e test adeguati, senza rimuovere il supporto OG classico.

**C2. Creare residenti dopo le dipendenze reali.** A `START`, anche senza lavoro, registrare tutte le competizioni OG; la Copa Bolivia richiede che `exp_find(45,0,year)`, la prima divisione OG corrente, sia già residente. Salvare le partecipanti (prima edizione ufficiale, edizioni successive da OG), seed/RNG, torneo, squadre, fixture, anno e phase. Se manca una divisione o non coincide il numero di squadre, dichiarare torneo non inizializzabile: non inventare sedici squadre.

**C3. Scrivere esclusivamente il calendario delle coppie OG.** Nell'esempio type2:
- `InitCompetitionTeamSelectionFromAltTableHeader` → `InitDIYLeagueFromRecord` → `InitTeams` inizializzano il vero motore, con `SaveSelectedTeams/SaveRand2State` e successivo restore dei buffer temporanei.
- `BoliviaCup_OGStageStart` trasferisce **64 coppie** in `_diyCompetitionStart.leaguePhase.cal`, risolvendo gli indici delle vere righe OG `leagueStandings` e dei club iscritti.
- 4 gruppi di 4, 8 giornate e 8 gare per club: **48** interne ai gruppi + **16** intergruppo (derby nelle giornate 2 e 4). Il dato statico non produce gol.
- Importante: il torneo si spezzava dopo 32 gare perché `totalMatchesPerSection` era `16`. Impostare `totalMatchesPerSection=32` e `totalMatchesPerSectionLive=32`: 32+32=64; bloccare la transizione automatica anticipata dopo l'ultimo section.
- ABI critica: i caller OG aggiungono `DIY_STANDINGS_OFFSET` a un **puntatore base-comp + 18*row**. Restituire già `&leagueStandings[row]` e far riaggiungere l'offset provoca accesso alla memoria sbagliata; non correggere questo solo a colpi di cast.

**C4. Dispatcher datato e una sola callback.**
- Non confondere gli **8 matchday logici** con l'ordine cronologico delle **64 gare reali**. Un recupero della seconda giornata può avvenire dopo partite della quarta. `BoliviaCup_ChronologicalFixtures` ordina per `(month, day, fixtureId)` ma conserva `fixtureId` e le coppie del CAR.
- Il cursore OG `matchesPlayedCount` determina il primo serial non giocato. L'evento è eseguibile solo se la data OG `s_expOGFixtureAsOf` l'ha raggiunto; una stessa fixture non va ripetuta dopo `CONTINUE`.
- Usare `DIY_PlayOrSimulateLeagueFixture(0,0)` per la CPU e verificare **insieme**: delta contatore OG = 1, journal witness = 1, fixtureCount +1. Un mismatch deve impedire il nuovo CAR; preservare il salvataggio precedente.
- **Manager:** se il club umano compare nella coppia, la fase CPU deve deferire (`DEFER_MANAGER`), NON simularla e poi farla rigiocare. È solo un meccanismo antiripetizione: la **connessione completa del match manager alla Copa Bolivia resta da implementare/verificare**. Non segnare la competizione «manager playable» sulla sola base del percorso CPU.

**C5. Gironi → KO dello stesso torneo, senza secondo motore.**
- Al termine delle vere **64 gare**, leggere le standings OG: le **prime due di ogni gruppo** hanno 8 gare ciascuna; validare ordine, ID, gruppo e assenza duplicati.
- `kBoliviaKOGroups` definisce gli incroci dei quarti: D2–A1, A2–D1, C2–B1, B2–C1. Il risultato/tie winner non è in questo header.
- Creare **un contesto OG type1 separato per la fase KO**, sempre Copa Bolivia, `country=45, slot=2, edition=anno`, con 8 club effettivamente qualificati. `InitDIYCupFromRecord` inizializza il bracket autentico; `DIY_PlayCupMatchAndAdvance(0,0)` gioca gli incontri OG; `DIY_UpdateCompetitionProgress` passa al turno successivo. I round OG sono **3** (QF/SF/Final), non quattro: il girone è mostrato solo come riga aggiuntiva nel viewer REGOLE.
- Per andata+ritorno: **8 QF + 4 SF + 2 finali = 14** gare native; garantire cursor `round/leg/tie/serial` corretto e identità di quattro semifinaliste distinte. La 222 sbagliava perché `world_cups.c::cup_under_way` identificava lo stesso `compId=164` come la vecchia coppa e rimescolava la semifinale. La 223 impedisce SOLO quell'hook mentre è attivo esattamente il KO Bolivia residente; non disabilitare `WorldCups` globalmente.
- Mai importare le qualificate reali 2026 come vincitrici del gioco; il campione e l'eventuale finalista si derivano **solo da OG**. I posti Libertadores/Sudamericana richiedono ancora un bridge delle quote e delle precedenze FBF, non basta registrare il trofeo.

**C6. Distinguere campi delle regole.** Nel type2 Bolivia i **3 punti vittoria** si ottengono impostando il corretto campo nativo del torneo (`awayGoalsMode=3` nel suo encoding), non invertendo un nibble della coppa KO. Per la schermata KO l'editor interpreta `0x35` come **3 sostituzioni da 5 in panchina**: la 222 aveva `0x53` e il risultato visualizzato era invertito. Verificare **i valori del runtime e la UI** dopo `InitDIYCupFromRecord`, non il commento o il numero decimale.

**C7. Viewer REGOLE: memoria valida dall'ingresso al menu.**
- I widget del tipo2 usano `MENU_CMD_TEXT_NO_PTR` e sentinelle di puntatore `0xFFFFFFFF` / `-1`. Il menu OG può chiamare `DesignDIYTournamentReinit` e `Int2Ascii` **prima** di `OnDraw`. Se l'entry richiede un buffer scrivibile, allocarlo stabile **prima** del callback che la usa.
- Alcuni campi vengono letti/scritti con **doppia indirezione** (`uint16_t **`, `**pp`). Il semplice cambio del buffer sul primo livello o un `sentinels-rebound=0` non dimostra che tutte le scritture siano sicure. La riparazione efficace sul viewer readonly Bolivia converte i campi necessari a `ENTRY_STRING` con stringhe **owned e stabili**, senza modificare i menu classici.
- REGOLE KO deve presentare **riga gruppi + QF + SF + finale** con 4 righe visuali, ma i round reali KO restano 3. La stringa compatta `6+2 DERBIES` evita overflow nella cella centrale rispetto a una frase più lunga. Validare font reali, glifi, celle e margini usando `sorgenti/docs/FONTS.md`; un asset di esempio approvato non è una nuova UI implementata.
- Aprire un viewer con `s_expOGViewing` e copie/scratch, salvarne e ripristinarne RNG e squadre, verificare hash di tutto il residente **prima/dopo** (`[og-viewer-check] unchanged=1`). Non usare la sola uguaglianza del numero partite per dimostrare read-only.

**C8. Rinnovo annuale, promozioni e risultati precedenti.**
- Il bug 224: `Expanded_OGWorld_AdvanceBolivia` loggava `NEW_EDITION_ENTRANTS` e ritornava senza inizializzare il 2027; `year!=World_FirstYear()` bloccava nuove edizioni e il viewer mostrava solo i nomi. Le squadre visibili **non** erano prove di competizione 2027 giocabile.
- Nella 225 si crea un nuovo `s_expOGBolivia` annuale con anno, 16 club della prima divisione `exp_find(45,0,year)`, fixture OG a zero risultati; **format e giorni/mesi 2026** si ripetono nel gioco con anno aggiornato. Le squadre rimaste mantengono i loro **slot template** e le promosse riempiono gli slot vacanti, senza riutilizzare il ranking dell'edizione chiusa.
- Bug nascosto: `BoliviaCup_OGStageStart` indicizzava `row[teamOrdinal]` e rifiutava ordinal ≥16. Dopo una promozione, la vecchia mappatura falliva. La 225 usa `Expanded_OGWorld_BoliviaTemplateOrdinal` e `s_expBoliviaInitMap` **solo durante l'inizializzazione** per mappare i 16 slot del calendario ai veri club OG dell'anno; la matrice delle 64 coppie rimane OG. Fare test anche con 1, 2 e 16 squadre nuove.
- Controllare che il precedente girone sia terminato **64/64** e l'eventuale ultimo KO **14/14 con competizione conclusa**, prima di rigenerare; in caso contrario non cancellare uno stato incompleto. Usare backup e rollback se init fallisce.
- Il reset stagionale deve conservare il KO precedente ancora necessario e il journal anti-replay; la 225 mantiene l'ultimo KO `country45/slot2/year` fra i residenti CAR. I vecchi CAR della 224 potevano aver già perso quello snapshot: compatibilità **senza inventare campioni o risultati mancanti**. Le edizioni più vecchie passano a registri/archivio sintetico quando non servono più. Non mutare un CAR dell'utente per correggere il design.

### D — Estendere rose/giocatori solo dopo il motore

Non confondere il registro delle nuove competizioni con quello degli atleti: `TeamFileEx` OG ha **16 PlayerFile** e ABI determinata. Il percorso approvato separa rosa club, distinta 11+7/9/12/15 e tetto sostituzioni 5 (+eventuale 6 nei supplementari se consentita); **non è implementato**. Prima eseguire audit lettori/scrittori OG, manager, CPU, save/replay, buffer UI e modalità classiche. Non modificare il numero di giocatori nel record storico per fare spazio alla panchina.

---

## 4. Cronologia verificata: commit, regressioni, correzioni

I link portano all'effettivo diff, non a un riassunto. «Implementato» indica codice presente; «Windows SI» solo quando supportato da log/screenshot; non attribuire a un commit test svolti su una build successiva.

| Build/commit | Cosa fu preparato | Errore, limite e rimedio successivo |
|---|---|---|
| [00.13.217 — `17e6c427`](https://github.com/Giuzy/SWOS-Expanded/commit/17e6c4270de5b047630b6edba68de7d2b149b23e) | Audit calendario CONMEBOL/cutoff; hash completo dei viewer OG; guardia sui range `DIY_ValidateLeagueScratchBuffer` per il panic 0x5555; solo fascia temporale Bolivia corretta | **Date/fonte/stato storico non diventano automaticamente torneo residente**; causa mobilità 5555 non dichiarata risolta. Libertadores 2027 non giocabile |
| [Dati — `5c66d9b3`](https://github.com/Giuzy/SWOS-Expanded/commit/5c66d9b331b65e9aa81c3a5433744415c836165c) | JSON quattro gironi reali e club Bolivia, calendari Argentina/Paraguay Apertura/Clausura e stato al 1 luglio | **SOLO DATI**; nessun tipo2, fixture OG, titolo o CAR giocabile |
| [00.13.218 — `7cc39882`](https://github.com/Giuzy/SWOS-Expanded/commit/7cc39882826fd2586df4b10df58e5b427fb22875) | `bolivia_cup_sequence.h`, 4×4 + derby, 64 fixture/8 giornate, test C11; disattiva vecchi KO type1 con 10+6/byes e falsi ottavi | Fixture statica ≠ partita. Viewer mostrava `OG GROUPS NOT RESIDENT`. Bisognava creare il residente type2 |
| [00.13.219 — `49df5392`](https://github.com/Giuzy/SWOS-Expanded/commit/49df53923c97c859b64cf8285a9d1000c1ca0096) | `s_expOGBolivia` type2 nazionale, 16 club OG, `InitDIYLeagueFromRecord`, 64 coppie in `leaguePhase.cal`, viewer read-only, campo opzionale CAR `og_resident.bolivia` | Ancora **0 gare OG disputate**; attesa sorteggio nascondeva partecipanti già noti. REGOLE Windows crash `0xC0000005` leggendo `-1` |
| [00.13.220 — `61b735a9`](https://github.com/Giuzy/SWOS-Expanded/commit/61b735a986f5a204f35ebbb10fa5e50470480ac0) | 64 pairing **reali** 18/08–08/10 con recuperi; buffer iniziali REGOLE per sentinelle; rimuove attesa 22 luglio | 64 date in JSON/C non erano ancora esecuzione OG; il fix REGOLE copriva solo alcuni campi e la pagina continuava a crashare; offset del pair richiedeva audit ABI |
| [00.13.221 — `8619b47a`](https://github.com/Giuzy/SWOS-Expanded/commit/8619b47af1f0c49ba6f0f37ffc1b39c48ff08966) | Dispatcher per data, ordine cronologico reale, OG `DIY_PlayOrSimulateLeagueFixture`, witness journal, contatori, `32+32` partite, correzione base+row ABI, sentinelle aggiuntive; limite finestra Liga Bolivia | **Windows**: 64/64 gironi e callback coerenti; ma **KO non parte**; REGOLE ancora `0xC0000005` con puntatore `0xFFFFFFFFFFFFFFFF`, RVA `0x15A645`; nessuna prova globale collisioni |
| [00.13.222 — `746d7626`](https://github.com/Giuzy/SWOS-Expanded/commit/746d7626776f847f0f3859ed047c44f03dba6fc3) | 8 qualificate OG → 14 KO residenti OG, header KO datato, `DIY_UpdateCompetitionProgress`; REGOLE doppia indirezione convertita per il readonly | **Windows**: 64 gruppi e 8 QF, poi due SF duplicate per hook legacy `WorldCups` su `compId=164`; `FAIL_CLOSED` evita scrittura CAR sbagliata. KO manager ancora DEFER |
| [00.13.223 — `1102427a`](https://github.com/Giuzy/SWOS-Expanded/commit/1102427ad8001d579ee60bc79a2e6bd7480af03e) | Guardia specifica `Expanded_OGWorld_BoliviaKnockoutActive` in `world_cups.c`; 3 punti vittoria gironi, 6+2 derby, byte KO `0x35`; F12 spostato/opaco | **Windows ZIP 20/11 11:10:** 64+14, due semifinali distinte, campione OG 25 novembre. A rollover 2027 rimaneva il residente 2026/stessa classifica |
| [00.13.224 — `fa892421`](https://github.com/Giuzy/SWOS-Expanded/commit/fa892421c8474aaa32ecfc5d72412d0616b86d48) | Viewer 2027 legge 16 partecipanti OG dell'anno corrente (promosse incluse) e non spaccia tabella 2026 per 2027; riga GIRONE/DERBY anche nella REGOLE del KO; screenshot UI gradito | **Windows ZIP 12:16:** `NEW_EDITION_ENTRANTS` e `PENDING_EDITION 2027 entrants=16 matches=0`, **solo lista club**, NON competizione giocabile |
| [00.13.225 — `57c75f44`](https://github.com/Giuzy/SWOS-Expanded/commit/57c75f4455077086148ed3631a7a4dd50dae97d5) | Anno successivo crea nuovo type2 OG con squadre correnti; template mesi/giorni 2026 ripetuto; mappa promosse ordinal>15; KO annuale, guard specifica ogni anno, conservazione ultimo KO/rollback; sei guide aggiornate | 4/4 mappature e 20/20 guardie **statiche** al commit. Oggi utente riferisce funzionamento delle stagioni 2–3; **nuova diagnostica Windows ancora da esaminare**. Mancano manager KO, qualificazioni CONMEBOL e audit collisioni globali |

### 4.1 Incidenti concreti: riconoscimento immediato e prevenzione

| Sintomo/diagnostica | Vera causa recuperata | Test/fix obbligatorio la prossima volta |
|---|---|---|
| Falso `ROUND OF 16` al posto dei gruppi Bolivia | Tipo1 legacy 10 club + 6 bye; non corrisponde ai 4 gruppi | Catalogare tipo nativo, disabilitare selettivamente legacy, registrare type2 OG; non cambiare solo `type` |
| Viewer elenca squadre ma non gioca gare | Solo dati/fixture oppure residente privo di dispatcher (218/219/220, poi 224) | Misurare `matchesPlayedCount`, `OG_PLAYED`, witness e CAR; partecipanti/JSON non sono prova di gameplay |
| Crash REGOLE `0xC0000005` (219, 221) | `MENU_CMD_TEXT_NO_PTR` sentinella e routine che scrivono PRIMA del draw; alcune entry con `**pp` | Audit ogni entry e callback Reinit/OnDraw, possesso buffer e tipo `ENTRY_STRING`; test Windows pagina REGOLE sia gironi sia KO |
| 32 partite, non 64 | `totalMatchesPerSection=16` in due sezioni | Configurare `32` per sezione, testare 32→33→64 e il limite senza replay |
| Crash/corruzione sulle righe standings | ABI `base + 18*row` vs puntatore già a `leagueStandings`; doppio offset | Testare contratto puntatore del caller OG, usare `DIY_STANDINGS_OFFSET` una sola volta, controllare memoria packed |
| Recuperi nell'ordine sbagliato | Iterazione per matchday invece di data civile; recuperi dopo giornate successive | Calendario OG in ordine fixture originale + permutazione esecuzione cronologica; `fixtureId` invariabile e test con date out-of-order |
| Classifica con 0 punti vittoria | Campo type2 punti non impostato correttamente | Verificare dopo Init e dopo partite 3 punti via `awayGoalsMode=3` del record type2; non confondere con byte KO |
| Sostituzioni 5/3 invertite | `0x53` vs `0x35` nell'encoding della coppa KO | Leggere davvero le struct OG e UI: `0x35` = 3 sostituzioni da 5 nel contesto KO; testare in Windows |
| SF 11/12 novembre uguali, 2d00–2d07 ripetuti | `WorldCups_ArrangeNextRound` sceglie torneo dal solo `compId=164` e ricrea bracket su cup sbagliata | Guardia su mondo, paese, slot, anno, 8 club, stato KO; proteggere solo hook legacy, testare 4 squadre distinte e 14 gare |
| CAR bloccato a metà KO | Incoerenza cursor/witness, per esempio duplicate semis `due=13` | `FAIL_CLOSED`, non scrivere nuovo CAR; log serial/round/leg/tie; tenere vecchio save valido |
| F12 nasconde valori | Overlay trasparente/posizione errata | Pannello opaco e test layout con dati reali/font, evitare coprire tabelle |
| Nel 2027 classifica 2026 | Viewer seleziona vecchio `ctx->year` invece dell'edizione richiesta | Scegliere l'anno corretto, se non residente mostrare solo vera lista di partecipanti, mai riciclare punteggi |
| Nel 2027 SOLO lista di 16 | 224: `NEW_EDITION_ENTRANTS` senza inizializzare nuova type2 OG; guardia `year!=World_FirstYear()` | Avviare nuovo residente OG per ogni edizione e il suo calendario, non limitarsi al viewer |
| Promossa respinta nel 2027 | `BoliviaCup_OGStageStart` rifiutava `teamOrdinal>=16` | Mappare slot template ai club OG effettivi, test con promosse/retrocesse, niente ID hardcoded passati |
| Perdita ultimo KO al rollover | Reset dell'array `s_expOGCups` senza retention del KO Bolivia | Conservare ultimo KO necessario, journal, archive; bloccare reset di torneo ancora attivo |
| «Date officiali 2027 non disponibili», niente nuova coppa | Si confonde previsione storica con calendario di gioco | Regola approvata: riusare formato/date 2026 nel futuro 2627 fino a update, etichettare `2026_GAME_TEMPLATE` |
| Il manager gioca due volte o non partecipa | CPU residente e OG locale non ancora unificati per quella coppa | Bridge manager al **medesimo** match OG, witness esatto; prima della realizzazione usare DEFER, mai sim successiva |
| Qualificati continentali mancanti | Un campione nazionale presente nel KO non basta a far esistere quote/precedenze/edizione CONMEBOL | Risolvere dipendenze OG, quote sportive e calendari continentali, poi testare l'accesso vero senza team fittizi |
| Falso `PASS` da C11/headless | Test esegue fixture/funzione pura, non Windows manager/UI/CAR | Distinguere test statico, C isolato, headless, compilazione Windows e partita manuale |

### 4.2 Quali log e ZIP hanno effettivamente provato che cosa

- `DIAGNOSTICA_SESSIONE_1(20261120-030725).zip`, build 219: REGOLE `0xC0000005`, lettura `-1`, altre schermate viewer utilizzabili; 64 accoppiamenti ma gare ancora a zero. Non prova 220.
- `DIAGNOSTICA_SESSIONE_1/2(20261120-085711).zip`, build 221: **64/64** gare OG gironi con contatori/callback coerenti; la fase successiva non iniziava; `REGOLE` falliva (RVA 0x15A645).
- `DIAGNOSTICA_SESSIONE_2(20261120-103544).zip`, build 222: 8 QF OG, poi semifinali duplicate per l'hook legacy, fallo bloccato per sicurezza. Non equiparare test delle prime otto gare a 14/14.
- `DIAGNOSTICA_SESSIONE_1(20261120-111014).zip`, build 223: completamento 64+14 e campione OG; 2027 `NEW_EDITION_PENDING` e viewer ancora legato a 2026.
- `DIAGNOSTICA_SESSIONE_1(20261120-121600).zip`, build 224: `[build] SWOS Expanded 00.13.224`, conclusione 2026, `PENDING_EDITION year=2027 entrants=16 ... matches=0` quattro volte. La build Windows è presente, con warning `taking address of packed member`. Screenshot REGOLE conferma disposizione, ma non in modo indipendente il codice/versione dell'EXE.
- **00.13.225:** l'utente ha riferito verbalmente che anche stagioni 2 e 3 sembrano funzionare; **non** confondere questa buona notizia con audit del nuovo ZIP, SAVE/CONTINUE, manager o certificazione completa di 2027/2028.

## 5. Caso Copa Bolivia come test di integrazione end-to-end

**Dati 2026 e fonte:** 16 squadre della División Profesional, gruppi A/B/C/D con composizione storica del sorteggio 22 luglio; tutte entrano automaticamente. 4 gruppi di 4; 6 partite interne per club (andata/ritorno) + 2 derby intergruppo; 8 incontri per club, 64 nel gruppo, prime due per gruppo ai quarti. Fonti repertoriate: FBF [Copa Paceña 2026](https://fbf.com.bo/copa-pacena-2026/), [stagione División Profesional](https://fbf.com.bo/temporada-2026-de-la-division-profesional/), risultati/dati partite [Bolivia.com](https://www.bolivia.com/futbol/liga-boliviana/copa-bolivia/resultados), quarti [Unitel](https://futbol.unitel.bo/division-profesional/copa-pacena-cuando-y-a-que-hora-se-juegan-los-cruces-de-cuartos-de-final-MF24033780), [Bolivia.com](https://www.bolivia.com/futbol/liga-boliviana/revisa-el-fixture-oficial-de-los-cuartos-de-final-de-la-copa-pacena-606528). I risultati reali **non** sono risultati del nuovo gioco OG.

**Date civili fixture dei gruppi 2026:** dal **18 agosto** all'**8 ottobre**, inclusi recuperi (esempio giornata 2 spostata al **22 settembre**). Il vecchio 19 agosto come prima data è stato corretto nella 220. **Date KO** nel calendario del codice: andate QF **20, 21, 21, 21 ottobre**; ritorni **2, 4, 4, 5 novembre**; semifinali andata **11/12 novembre** e ritorni **15 novembre**; finali **18/25 novembre**. Non sostituire le date singole con un generico intervallo di `endMonth`. Per 2027+ ripetere esattamente questi giorni/mesi **nel calendario di gioco** salvo update.

**ID e mapping:** in prima edizione gli ordinals effettivi sono i 16 del DB TEAM.045; nel 2027+ recuperare i club OG della divisione corrente, con 16 ID unici. La mappa 225 conserva slot storici delle squadre rimaste, sostituendo solo quelle retrocesse con le promosse; **non significa riusare le stesse qualificate o gli stessi risultati**. Non reintrodurre la vecchia ipotesi 10+6 del KO nazionale.

**Qualificazione FBF da implementare completamente:** la documentazione 219 indica vincitore Copa Bolivia per Libertadores Bolivia 3 (con regola di scorrimento al meglio classificato non qualificato se già Bolivia 1/2) e finalista verso Sudamericana Bolivia 1 se non già Libertadores. Questo è **regolamento/catalogo**, non prova di quote assegnate in partita OG. Verificare precedenze e anno di competizione prima di toccare la CONMEBOL.

**Regole vs schermo:** nella pagina INFO/REGOLE dei gironi mostrare i 4 gironi, punti vittoria, 6+2, prime due. Nella pagina KO mostrare una riga storica riassuntiva dei gruppi e tre round OG QF/SF/FINAL; NON creare una fase finta solo per ottenere la quarta riga sullo schermo.

## 6. Rinnovo, archive e compatibilità CAR: protocollo comune

1. Conservare **l'ultima edizione reale utile**, tutte le edizioni ancora attive e quelle dalle cui standings dipendono qualificazioni o turni successivi. Il 1 luglio non cancella un torneo che deve proseguire dopo; un manager assunto/esonerato non resetta il mondo.
2. Identificare la nuova edizione OG da paese/slot/anno, con iscritti e stato **derivati** dai risultati OG (o, per torneo senza qualificazioni, dall'attuale prima divisione OG). Non generare un secondo sorteggio da una vecchia cache, e non ripetere partite.
3. Solo dopo verifica che fase precedente/KO siano concluse, inizializzare nuova edizione con **zero gare svolte**, hash, seed, RNG, club e calendario OG. In errore ripristinare il precedente residente/salvataggio.
4. Salvataggi **AUTO9697.CAR** e **AUTO2627.CAR** nella cartella `save/` accanto all'EXE; `AUTOSAVE.CAR` solo recupero legacy. Non sovrascrivere/cancellare save esistenti. Scrittura atomica, integrità, Zstandard statico livello 3, nessuna DLL aggiuntiva.
5. `og_resident.bolivia` è stato introdotto come contesto opzionale; non cambia il numero dei tre slot UEFA, `league_size` o `cup_size`. Nei vecchi CAR 220 non giocati si può correggere `totalMatchesPerSection` 16→32 **solo con zero match**; mai reinterpretare a posteriori un CAR già giocato. CAR 224 potrebbe non avere snapshot KO passato: registrare il limite, non creare un trofeo falso.
6. Journal OG a capacità dinamica e anti-replay per-edizione; dettagli partita per la stagione corrente, archiviazione sintetica quando nessuna competizione/qualificazione ne dipende più. Oggetto archivio: ID edizione, vincitore, eventuale finalista, data chiusura, conteggio gare, riferimenti anti-replay. F12 deve separare spazio disco/RAM, `DETT`/`ARCH`, conteggi e dimensione CAR compressa/RAW.
7. **Test cross-process obbligatorio:** gioca, salva, EXIT, riapri EXE, CONTINUE, confronta fixture e hash OG/RNG, avanza nuovamente, assicurati che nessun record si ripeta. F12 e journal devono concordare. Fare prove anche con un vecchio CAR, senza cancellarlo.

## 7. Tre pattern di competizione: come adattare il metodo

### A. Campionato nazionale type0

Avviare il residente OG del campionato con club esatti, promozioni/retrocessioni e calendario. Identità `country/division/year`, RNG, standings, giornate e partite OG. La CPU e il manager condividono lo stesso risultato per ogni singolo fixture. Le date della prima edizione sono quelle reali (compresi recuperi); in 2627 i calendari futuri possono usare il template 2026 **solo dopo una decisione completa delle singole date**. In collisione con la coppa, documentare l'eventuale rinvio della sola partita di lega, con data originale/motivo. Non chiamare prova cronologica il solo numero di fase nativa.

**Attenzione Argentina e Paraguay:** nel 2026 hanno Apertura e Clausura e, per l'Argentina, zona/playoff/tabla anual; l'attuale record type0 unico e le 58 gare per club non corrispondono alle due vere edizioni. Non dimezzare `beginMonth/endMonth` e considerare risolto il formato: servono i residenti/risultati OG separati con stabili identità e stato storico Apertura al 1 luglio.

### B. Coppa nazionale type1

Definire quote, preliminari, eventuali bye, seed/turni, regole tie-leg, finale ed eventuale terzo posto (se previsto). Usare `Expanded_OGWorld_AdvanceCup`/`ExpandedOGCup` e le funzioni OG cup; validare che il numero degli iscritti non ecceda le reali strutture OG, che i vincitori vengano da OG e che i posti esterni non siano inseriti a priori. Record type1 non può rappresentare arbitrariamente un torneo 4×4 con derby: non cambiare semplicemente il suo byte `type`.

### C. Torneo nazionale/internazionale type2, gruppi più eliminazione

Usare adattatore residente OG type2 per standings e calendario dei gruppi, poi (quando il native schema lo richiede) il contesto KO OG della stessa competizione. Ogni fase e ogni anno hanno identità verificabili. Se una fase termina dopo il 1 luglio, il relativo residente sopravvive. Per competizioni continentali controllare accessi da campionati/coppe OG, anno dell'edizione e competizioni a cavallo tra due stagioni. La Copa Bolivia è solo un esempio di come attaccare 64+14 gare al motore: non assume il formato della Libertadores o della Champions.

### D. Storico 1996/97 e competizioni con formato che cambia

Preparare una **timeline fontata per torneo**: anno iniziale/finale di ciascun formato, numero club, modalità qualificazione, fasi, date reali, tie-break e cambi regolamentari. Presentarla all'utente prima di decidere cosa implementare. Le versioni storiche non vanno convertite automaticamente ai regolamenti del 2026. Distinguere una stagione già iniziata al 1 luglio, una competizione ancora attiva tra anni e l'edizione successiva che inaugura una riforma.

## 8. Test obbligatori: evita di bruciare altre sei-sette build

### 8.1 Test di dati prima di qualsiasi commit

- Eseguire `sorgenti/tests/run_test_bolivia_cup_sequence.sh` e `run_test_bolivia_ko_sequence.sh` (in ambiente C compatibile). Suite generale `sorgenti/tests/run_test_engine_suite.sh`, ma riportare test che si sono potuti eseguire davvero. Sono **C11 isolati**, non test Windows del gioco.
- Verificare almeno: ID unici, club esistenti, 4×4, 48+16, 64+14, 8 gare per club, quattro casa/quattro fuori, due derby nelle giornate previste, ordinamento cronologico anche con rinvii, zero doppie gare **intra-competizione** nello stesso giorno, duplicati di fixture rifiutati.
- Simulare nel **validator puro** le promozioni: 0, 1, 2 e fino a 16 club cambiati con ordinal >15; 225 ne ha verificati quattro scenari statici. Verificare che nessun ID scompaia e tutte le 64 coppie rimangano assegnate.
- Verificare che il testo UI e i campi di ogni round corrispondano all'OG, che le callback e i puntatori abbiano buffer validi dall'ingresso al menu. Ricordare che i test statici non intercettarono tutte le regressioni di REGOLE.

### 8.2 Test gioco Windows: matrice minima prima di segnare «SI»

| Scenario | Prova richiesta |
|---|---|
| Compilazione | Windows 10/11 x64, MSYS2/SDL2, `AVVIA_DIAGNOSTICA.bat`; leggere **errori E warning**; screenshot versione corretta, log `[build]` |
| START disoccupato | Residenti di tutte le competizioni presenti; zero incontri futuri già disputati; viewer partecipanti visibile subito, REGOLE e BACK senza crash |
| Prima gara gruppo | Data vera corretta, club/avversario esatto, OG risultato, contatori +1 e un solo witness, CAR |
| Posticipo | Partita di giornata precedente eseguita alla vera data civile dopo una giornata successiva, senza cambiare `fixtureId` |
| Limite metà calendario | 32ª e 33ª partita OG, nessun reset prematuro; 64ª/64ª, classifiche e qualificati reali |
| KO | Quattro QF unici, 8 gare OG; due SF tra quattro club distinti (non ripetere 2d00–2d07), 4 gare; finale 2 gare, campione OG e quote solo quando collegate |
| Manager | Club in gara: stesso incontro OG che la CPU avrebbe processato, nessun replay; se bridge manca, `DEFER_MANAGER` significa **NON IMPLEMENTATO** |
| REGOLE e UI | Pagine gironi e KO, 3 punti, 6+2 derby, 3 sostituzioni/5 panchina, 4 righe visive; niente `-1`/`0xC0000005`, font/colonne/F12 corretti |
| Viewer readonly | Hash prima/dopo identico, RNG e scratch ripristinati, apertura prima del sorteggio quando club noti, vecchia edizione etichettata se necessaria |
| Rollover | 2026→2027→2028→2029: vecchio KO non perso, edizioni con risultati distinti, partecipanti OG aggiornate e zero gare all'inizio nuova edizione |
| SAVE/CONTINUE | Iniziale, dopo 1/32/64 gare, a metà KO, a fine KO e dopo rollover; EXIT e nuovo processo, nessuna gara/rng replicati; vecchio CAR protetto |
| Collisioni | Per ogni club e giorno verificare anche campionato, altre coppe, nazionali e internazionali; loggare ogni spostamento di lega con data originale/motivo |
| Regressione | Mondo 1996/97, 2026/27, assunto/disoccupato/esonerato, offerte, CLASSIC/DIY/SEASON, Switch Pro A/B/X, BACK e F12 |

Per monitorare i log cercare `[og-bolivia-resident] INITIALIZED year=... played=0`, `[og-bolivia-fixture] OG_PLAYED`, `[og-bolivia-ko] PREPARED`, `[og-bolivia-ko] OG_PLAYED`, `[og-bolivia-ko] OG_CHAMPION`, `[og-viewer-check]`, `[og-fixture-tick]`, `FAIL_CLOSED`, `DEFER_MANAGER` e `[og-season-retain] BOLIVIA_LAST_KO`. Un log `NEW_EDITION_ENTRANTS` o `PENDING_EDITION` senza partite non è successo. Cercare sempre gli errori anche se il gioco «sembra» funzionare.

### 8.3 Come leggere un test e come pubblicare un risultato

Usare **SI / NO / NON PROVATO** separati per: dati statici, compilazione C, build Windows, UI, CPU OG, manager OG, KO, rollover, CAR, anti-replay, date/collisioni globali, qualificazioni e regressioni. Non spacciare «70 stagioni headless» per test di una partita manager, né «commit fatto» per «EXE compilato». ZIP/log/screenshot Windows dell'utente hanno priorità sull'ipotesi del codice. Se emerge un crash, annotare versione, SHA, RVA/codice errore, schermo esatto, ultimo evento OG/serial e data; correggere **la causa precisa**, non nasconderla con un fallback.

## 9. Scheda standard da compilare per OGNI nuova competizione

Copiare questo schema in un file di audit dati prima di scrivere C:

| Campo | Da specificare/provare |
|---|---|
| Identità | world, nazione/ente, slot, compId, editionKey, tipo OG; differenziare stesso compId in altri contesti |
| Fonte storica | fonte primaria e data, eventuali correzioni; stato reale al 1 luglio; incertezza esplicita |
| Partecipanti | club OG/ID, numero, qualificazioni, promosse/retrocesse, fallback viewer |
| Formato | type0/1/2, fasi/round, gruppi/derby/bye, round trip, tie-break, criteri promozione e ranking |
| Calendar 2026 | tutte le partite, fixtureId, home/away, originale e posticipo; prima e ultima data |
| Calendar 2027+ | template ripetuto 2026 per mondo 2627 fino a update, non descriverlo come ufficiale; storico 9697 secondo decisione specifica |
| Dipendenze | quale classifica/cup OG produce iscritti, vincitori, quote, premio e anno dell'edizione successiva |
| Motore | quale codice originale inizializza/simula/risolve/avanza; un solo OG, CPU e manager stesso fixture |
| Save | versione CAR, resident context, fixture cursor, RNG, anti-replay, retention, archive, legacy restore e fallimento atomico |
| Viewer | rendering/REGOLE e fonte dati, readonly hash, anno/senza partecipanti, finestre di data, input |
| Limiti | capacità OG, buffer/campi packed, compatibilità e collisioni, funzionalità manager non ancora legate |
| Test | statici C, headless, Windows build, CPU/manager, gironi→KO, SAVE/CONTINUE, 3+ stagioni, regressioni |
| Stato | Proposta / Dato / Codice preparato / Commit / Build Windows / Gameplay SI o NO, con SHA e ZIP |

## 10. Procedura di lavoro/commit per la prossima coppa

1. Se il lavoro riprende **dopo un'interruzione**: leggere **integralmente** la [Issue #1](https://github.com/Giuzy/SWOS-Expanded/issues/1) prima di rifare qualcosa. Durante una fase continua, non rileggerla ad ogni micro-passo.
2. Verificare **ramo, HEAD, versione**, sei guide obbligatorie (`AGENTS`, `PROJECT_STATE`, `ENGINE_CODE_DISCOVERIES`, `GAME_DESIGN`, `IMPROVEMENTS`, `DEVELOPMENT_WORKFLOW`), file codice e asset toccati; `FONTS` per nuova UI. In caso di **conflitto reale tra regole/decisioni**, esporlo e chiedere quale prevale prima di agire su quel punto, continuando sul resto.
3. Inventario A e validazione B **prima** della parte C. Conservare gli SHA dei blob e i risultati tecnici. NON modificare `main`/release/backup, né fare merge/checkout/refactor non richiesti.
4. Dopo **ogni singola cosa preparata** (file, blob, patch, verifica, guida, test, decisione), **sovrascrivere cumulativamente la stessa Issue #1 prima di preparare altro**: motivo, posizione, SHA, contenuto recuperabile, prova SI/NO/NON PROVATO, limiti, prossimo passo. Se l'update fallisce, segnalarlo.
5. Preparare patch e test minimali, verificare dipendenze, formati, date, ABI, contatore OG e CAR. Se una funzione manca, **non** dichiarare la coppa completa: esplicitare il blocco. Solo dopo autorizzazione «committa»: un unico commit finale su `dev/0.10`, versione `build_info.h` aggiornata **per codice**, non per sole guide.
6. Dopo commit, verificare **GitHub compare** (esattamente i file attesi), HEAD/versione e Issue aggiornata; l'utente sincronizza GitKraken e compila/prova Windows con gli script ufficiali. Nessun EXE Windows o gameplay PASS senza la sua diagnostica.

### Riferimenti estesi, da non perdere

- [Audit di date e cutoff 2026](COMPETITION_DATES_AND_JULY_2026_CUTOFF.md): competizioni CONMEBOL, dipendenze, primo luglio e matrice reali.
- [Definizioni Bolivia, Argentina e Paraguay](COPA_BOLIVIA_ARG_PAR_2026_DATA_AUDIT.md): SOLO DATI e casi Apertura/Clausura.
- [Build 218 — fixture format](BOLIVIA_2026_SEQUENCE_0013218.md), [build 219 — residente](BOLIVIA_2026_NATIVE_RESIDENT_0013219.md), [build 220/221 — REGOLE e 64 date](BOLIVIA_2026_RULES_AND_FIXTURES_0013220.md).
- [Scoperte nel codice](ENGINE_CODE_DISCOVERIES.md), [stato corrente](PROJECT_STATE.md), [progettazione](GAME_DESIGN.md), [workflow Windows](DEVELOPMENT_WORKFLOW.md), [migliorie/versioni](IMPROVEMENTS.md), [regole per collaboratori](AGENTS.md), [salvataggi e archivio](../sorgenti/docs/SAVE.md).
- Il deposito unico [Issue #1](https://github.com/Giuzy/SWOS-Expanded/issues/1) conserva prove, blob intermedi, decisioni e motivazioni non necessariamente presenti in un commit.

**Non ancora risolto dalla 225:** integrazione manager delle partite di Copa Bolivia, assegnazione qualificazioni Libertadores/Sudamericana dai risultati OG e relative priorità, calendario della Liga boliviana match-by-match/collisioni globali, generalizzazione reale della ricorrenza a tutte le altre competizioni e transizioni storiche 9697, nonché certificazione Windows completa delle stagioni 2 e 3 e di SAVE/CONTINUE. Non riaprire gli stessi errori confondendo questa guida con un'implementazione.