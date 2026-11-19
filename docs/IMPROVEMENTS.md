### 20/11/2026 — 00.13.216: hotfix esclusivamente compilazione
Build Windows 215 non compilata: il preflight SDL senza `SWOS_CAR_ZSTD` non vede il prototipo `SaveCarZstd_IsFrame` usato in `SaveJson_CareerDiskStats`. F12 identifica ora lo stesso header `SWOSZC3A` con `memcmp`, già disponibile, lasciando al loader la verifica del contenuto; versione avanzata a 00.13.216. Nessuna variazione ad archivio, numeri F12, compressione, OG o formati CAR. Nuovo test Windows necessario prima di dichiarare una build funzionante.

## 19/11/2026 — 00.13.215, nuovo save CAR compresso e registro dinamico
Codice: Zstandard v1.5.7 livello 3 integrato staticamente senza nuova DLL; CAR schema5 interno invariato e riconosciuto anche non compresso, wrapper con checksum e raw size; single-file AUTO atomico. Journal OG: capacità array/hash dinamica, anti-duplicazione preservata, vecchi campionati OG chiusi rimossi al primo rollover con qualificazioni attestate; archive per edizione conteggio+hash+identità contro replay; fonti COPPE non verificate trattenute (politica piena ancora PENDING), nessuna partita duplicata o punteggio inventato. Pannello F12: COMPRESSO MB/RAW MB e fixture salvate DAVVERO sul disco. Compilazione/gameplay Windows 215 NON PROVATI. Testare entrambi mondi e carriera con/senza club, SAVE/EXIT/CONTINUE, copia vecchi CAR, oltre 65536 fixture / anno 2028; la Libertadores 2027 resta non collegata.

## 19/11/2026 — 00.13.214, correzione due viewer europei OG 9697
**Windows 213:** compilazione positiva, 19 PASS isolati, OG 1997 CWC32 e UEFA32 residenti, viewer in UNEMPLOYED mostra 1996 (`LAST_COMPLETED_EDITION`) e con contratto segnala `NOT RESIDENT compId=1/2 matches=2`. **Codice 214:** nello spettatore OG CWC/UEFA classiche, l'anno e' criterio obbligatorio; `exp_cup_find(country254,slot1/2,activeYear)` preferito a un indirizzo uguale fra edizioni. `Career_ViewOrEditCompetitionSlot` riconcilia originale/working copy/slot qualificato prima del dispatcher generico, e consulta l'OG residente senza far giocare alcuna partita. Isolato GCC C11 -Wall -Wextra -Werror -pedantic PASS; completo Windows non eseguito dall'assistente. 2027 Copa resta non collegata: messaggio specifico, nessuna simulazione/fixture fittizia. Non cambiano CAR, timer, RNG, risultati, date o fisica. Mancano attivazione OG della Libertadores 2027, SAVE→CONTINUE integrale, manager CONMEBOL e accessi nazionali.

## 19/11/2026 — 00.13.213: route viewer OG della Libertadores 1996/97
Dopo la Windows 212, l'incapacita' di aprire la Copa da un club non partecipante deriva dal **gate UI** `_euroCupSlotIndex==3` e dal mismatch originale/working copy, non da assenza delle 20 squadre residenti. Il codice 213 riconosce le due identita' prima della ricerca generic slot, reindirizza allo stesso viewer read-only OG canonico usato da DEBUG WORLD; nessun cambiamento a partite, competizioni, date, RNG, CAR, qualificazioni, rose e altri mondi. Aggiunto helper C puro, test routing positivo/negativo e sua esecuzione nel runner esistente con log esplicito `PASS_ISOLATED_ONLY`, differenziato da `VIEWER_GAMEPLAY=NOT_TESTED`.
**Non completato:** fixture manager per club CONMEBOL realmente qualificato, controllo anti-duplicate con resident, 2027 primo CT_IN/turni Q1/Q2/fixture OG datati e SAVE/CONTINUE. Non chiamare pass il modern torneo solo per compId/JSON. Test Windows 213 non disponibile all'assistente; utente usa GitKraken e AVVIA_DIAGNOSTICA.bat.

## 19/11/2026 — 00.13.212: autosave OG fail-closed e CWC 1996/97 da disoccupato (test Windows DA ESEGUIRE)
**Causa diagnosticata su ZIP Windows 00.13.211 `DIAGNOSTICA_SESSIONE_1(20261119-202740).zip`:** nello stesso tick OG phase80 1997, `[og-cup-resident] MANAGER_SLOT_UNBOUND country=33 slot=3` passa da 53 coppe residenti/668 incontri cumulativi a 52/611, mentre sono realmente state giocate altre 18 gare cup e 399 league. La coppa del manager e' esclusa correttamente dal residente per NON ripetere gare; il suo contatore storico `matchesPlayed` (~75) resta nel `ExpandedOGCup` ma `Expanded_OGWorld_CupMatches` lo ignorava perche' `used=0`. La conseguente *apparente* decrescita del contatore cup impostava `monotonic=0` per entrambe le categorie, `CALLBACK_COUNTER_MISMATCH` e 3 autosave 9697 rifiutati. **Correzione 212:** `ExpandedOGCup_ObservedMatchCount` include nel TOTALE STORICO gli incontri OG gia' osservati dei `managed_excluded`, ma NON considera tali coppe attive, NON le simula e NON le ripristina come partite da giocare. Le differenze vere di contatori continuano a bloccare il nuovo CAR, conservando quello precedente. Nuove diagnostiche riportano i contatori raw prima/dopo e `[og-fixture-journal] SAVE_REJECTED reason=...`; il precedente `[og-world-save] NO_RESIDENT` era un messaggio generico fuorviante, ora `OG_STATE_OR_JOURNAL_REJECTED`. Non cambiano formati CAR, RNG, date o risultati.
**CWC Classic unemployment:** `DIY_CacheSecondaryQualifiedTeamsGrid` OG usa i **primi tre indici del bracket concluso** dopo `DIY_ApplyHomeAwaySwapFlagsToTeamGridPairs`, ottenendo vincitore/finalista/terzo candidato autentici; `swos_globals.c` riserva due `TeamFileEx` contigui oltre al nono club e al vincitore (grid 9,10,11). Expanded copiava solo il vincitore e il callback OG quando era anche campione nazionale o titleholder tentava slot non inizializzati. La 212 legge i tre ID dall'**autentico** `cup->comp.teamGridPairings`, `cupRoundResults[0]` e `cupTeamIds`, richiede `matchPairsPerRound==1`, verifica vincitore con `comp.base.winnerTeam`, e ogni altro ID come partecipante unico. Copia le vere schede OG solo nel buffer di qualificazione *temporaneo*, usa l'ordine/esclusioni native del callback CWC e rifiuta dati insufficienti (nessun finalista inventato, nessuna gara nuova). Il manager OG normale resta intatto.
**QA:** helper C identici ai blob GitHub `3df7457b4a...` e `94b7a11f...`, smoke isolato GCC C11 -Wall -Wextra -Werror -pedantic PASS; 2 test C esistenti aggiornati per contatore unbound e podio OG/duplicati, ma esecuzione loro runner Windows e intero `SWOS_Expanded.exe` **NON PROVATA**. La 00.13.211 era compilata e aveva 19 test isolati PASS, ma `OVERALL=FAIL`: non attribuire quel collaudo alla 212. Testare subito 96/26, assunzione a meta' stagione, cup del manager, 1997 CWC 32/32 e Europa 32/32 da disoccupato, SAVE/CLOSE/CONTINUE con checksum e senza replay.
**Design confermato ma NON IMPLEMENTATO in questa build:** CAR minimo = una sola stagione dettagliata, piu' competizioni ancora aperte e qualificazioni club/nazionali solo finche' necessarie; archivio storico sintetico permanente. Sostituisce la vecchia regola delle due stagioni dettagliate e del biennio obbligatorio. Incremento dinamico limiti 65.536 fixture / 512 leghe / 256 coppe, coppe >64, e runtime JSON loader restano **PENDING**.

## 19/11/2026 — 00.13.211: vincitrici delle coppe OG, edizione esatta e calendario manager
La diagnostica Windows reale **00.13.210** `DIAGNOSTICA_SESSIONE_1(20261119-195348).zip` conferma build senza errori/warning osservati, 19/19 test isolati, nessun PANIC nel percorso, precampionato 96/26 e passaggio anno senza crash nei casi esercitati. Il gate 210 intercettava 98 coppe concluse (43 europee classic, 55 moderne) ma senza winner verificato. Root cause: `DIY_PlayCupMatchAndAdvance` scrive il vincitore in `_diyCompetitionStart.base.resultCode` (= `base.winnerTeam`), serializzato in `ctx->comp`; la qualifica Expanded leggeva erroneamente `ctx->tournament.header.winnerTeam`. La 211 legge **solo** il vincitore nativo da `ctx->comp.base.winnerTeam`, controlla id unico nelle squadre OG residenti (anche per title holders) e NON modifica i risultati. Evidenza CAR 210: 1996 CWC 0x0f00 e UEFA Cup 0x1428 risultano nella lista delle rispettive partecipanti.
Il verificatore del 1997 ora ignora le edizioni CWC/UEFA 1996 conservate e confronta `(country,slot,year)` richiesto, eliminando il falso `FAIL_NEW_EDITION_MISSING`. La riconciliazione di sette gare usa la chiave slot nativa OG (byte basso della word `compSlot/slotId`) distinta dal `field_2E` compId (46/74/88), mantenendo invariati tutti i controlli di fase, squadra manager, score e classifiche e riscrivendo, se tutto prova l'identità, **solo i quattro byte home/away del keycache**. Il calendario OG, il precampionato, le partite e le CAR preesistenti restano intatti.
**Limiti:** i sette rebound reali e l'effettivo ripristino dei 98 cup winner, la derivazione accesso delle coppe nazionali 2027, i SAVE/CLOSE/CONTINUE, le tre edizioni, il claim scheduler e il loader JSON nell'OG richiedono ancora diagnostica Windows 211. Non chiamare PASS gameplay i soli test isolati; il GCC locale ha eseguito unicamente un piccolo test dei nuovi helper, non una build completa. Riferimento tecnico di ripresa: Issue #1 (checkpoint unico sovrascritto).

## 19/11/2026 — 00.13.210: qualificazioni collegate, attendere vincitore nazionale OG
L'utente ha segnalato correttamente che le competizioni **non producono qualificazioni indipendenti**: chi vince una coppa può ottenere/riassegnare un posto europeo; Coppa Italia nel 2026/27 normalmente Europa League, non Champions direttamente, ma detentore Europa League accede a Champions. Verifica OG: `career_eurocups.c` ordina Championships→CWC→UEFA con esclusione delle precedenti; `uefa_access.c::uq_season` calcola insieme UCL/UEL/UECL con cup winner e holder. Il codice 209 era rischioso per field europei incompleti: `exp_og_qualification_country` accettava come vincitore nazionale anche il successivo **league cup slot3**, qualora slot2 non fosse finito. Correzione 210: SOLO national cup slot2, con finale/winner OG verificati per paesi europei; nessuna lista UEFA dichiarata completa in assenza di fonte. Se solo Copa Libertadores classica è verificata, nel CAR `qual_next` salva Copa e tre campi UEFA `invalid=1, count=0` (wire invariata); CWC 1997 richiede Champions+CWC verificate, UEFA Cup tutte e tre. Nessun nuovo regolamento o nuovo simulatore. A1/B1 JSON 208, 7 identity mismatch, nuove coppe nazionali, CAR atomic e test multipli restano PENDING. Windows 210 NON PROVATO. [Report](OG_QUALIFICATION_CROSS_CUP_PREREQUISITES_0013210_20261119.md).

## 19/11/2026 — 00.13.209: qualificazioni OG classiche indipendenti e audit identità
La diagnostica Windows 208 conferma precampionato 1996/97 e 2026/27 funzionante, senza PANIC nei percorsi provati. Le quattro verifiche `FAIL_NEW_EDITION_MISSING` 1997 corrispondono a un gate erroneamente globale: in `Expanded_OGWorld_AdvanceUefa` Champions tipo2 era bloccata dalla mancanza della CWC/UEFA, e `expanded_og_world_run_year` bloccava CWC+UEFA insieme. Nella 209 ciascuna coppa 9697 usa SOLO il proprio campo OG verificato (16 Champions, 32 CWC, 32 UEFA), lasciando pendente soltanto la coppa senza fonte. Mod 2627 rimane invariato. Le sette `BLOCK_PAIR_RECONCILIATION` ricevono log delle condizioni specifiche: senza prove non si riscrive l'identità delle squadre. Accesso coppe nazionali 2027, single-writer, CAR cross-process e tre edizioni sono ANCORA DA PROVARE/IMPLEMENTARE. Versione Windows e test di partita 209 NON PROVATI; dettagli [OG_CLASSIC_UEFA_INDEPENDENT_QUALIFICATION_0013209_20261119.md](OG_CLASSIC_UEFA_INDEPENDENT_QUALIFICATION_0013209_20261119.md).

## 19/11/2026 — 00.13.208: PUNTO 7 approvato; avviata Fase A/B importatore e validatore (NON motore OG completo)
**Decisione utente del 19/11, vincolante:** percorso A) convertire/catalogare definizioni di leghe, coppe, internazionali, nazionali, club e giocatori dei mondi 1996/97 e 2026/27 nei JSON esistenti; B) validare automaticamente ID, riferimenti, formati, quote, date, qualificazioni e limiti reali con errore preciso prima del runtime; C) collegare definizioni validate all'UNICO motore OG residente con gare/risultati/trofei veri, edizione successiva, SAVE/CONTINUE; D) estendere lo stesso metodo a club, giocatori, rose base ed extra. **Nessun editor grafico richiesto**: l'utente puo' descrivere in chat nuove competizioni, squadre o giocatori; l'assistente prepara JSON compatibili, sottopone a consenso regole nuove, modifica i dati e committa solo dopo «committa». Nessun nuovo simulatore di risultati, nessuna nuova toolchain da installare su Windows, CLASSIC/DIY/SEASON e salvataggi esistenti protetti. La sistemazione grafica del calendario e' rinviata. Questo inizio A/B di sola analisi dati e' indipendente dai gate gameplay B/C ancora aperti nel motore; **non** sblocca ancora nuove coppe giocabili.
**CODICE 208, primo passo reale A1/B1:** `sorgenti/scripts/validate_data.py` (facoltativo per chi mantiene gli asset, solo Python standard gia' usato dagli strumenti di import del repository; NON viene invocato dalla build Windows) inventaria i JSON classici sotto `data_json/competitions/`, le leghe overlay 2026/27 e l'edizione Libertadores provvisoria e verifica contatori, ID, riferimenti paese/competizione, seed, league-block, qualificazioni, date ISO, formati/partecipanti, percorsi team file, con severita' ERROR/WARNING/PENDING e output JSON opzionale. Con `--teams` analizza anche rose native e UUID, **non ancora** i riferimenti completi squad_extras. `sorgenti/tests/test_validate_data.py` aggiunge quattro test sintetici per casi validi, sentinel OG e dati errati. Formato JSON attuale riutilizzato, non migrato a nuovo schema incompatibile. E' un validatore **preflight offline** e non ancora un validator C a caricamento, né prova che una coppa sia RESIDENT o PLAYED. Per aggiungere una competizione veramente giocabile serviranno collegamenti OG verificati, ID/archivio/CAR sicuri e regole adottate.
**Scoperte verificate:** nel tree repo `data_json/competitions/` contiene 10 file classici, `data_json/teams/` 76 file, `data_json/worlds/2627/teams/` 101 file e `.../competitions/leagues.json` 91 definizioni (Italia 14 divisioni, contro alcuni limiti vecchi nelle guide). `libertadores_2027_first.json` ha 47 entranti (28 diretti e 19 preliminari) ma dichiara `DATA_ONLY_NOT_YET_LINKED_TO_OG_MATCHES`: NON giocabile per il solo JSON. La lista classica `countries.json` ripete identico BRAZIL/46, il nuovo validatore segnala WARNING, non confonde con due squadre diverse; le sentinel OG `-2` e `252` in `qualifying.comp_cup` non sono ID torneo. Nel seed bucket `bucket22Full` l'ulteriore slot e' il terminatore OG. File grandi di team/player non tutti letti dal connettore: controllo profondo reale **NON PROVATO**. Test Python e Windows 208 **NON ESEGUITI**. Dettaglio: [PUNTO7_IMPORT_VALIDATOR_A1_0013208_20261119.md](PUNTO7_IMPORT_VALIDATOR_A1_0013208_20261119.md).

## 19/11/2026 — 00.13.207: luglio manuale, intercettazione PLAY MATCH + NEXT MATCH
La **diagnostica Windows reale 00.13.206** `DIAGNOSTICA_SESSIONE_1(20261119-171631).zip` conferma compilazione, nessun PANIC/crash e clock post-partita operativo. Dopo FINISH manuale 2026/27: `[og-calendar] BOUND ... next-phase=56` (1 agosto), `[career-july-menu] eligible=1` al 1 luglio ma **zero** `[career-july-forward]`: il pulsante principale PLAY MATCH e' entry **7**, non la entry **10** su cui 205/206 installavano FORWARD. [CODICE 207] Entrambi i veri handler `PlayMatchMenu` (7) e `NextMatch` (10) intercettano prima di OG: quando world.today e' luglio e il prossimo incontro OG del club e' in agosto o oltre, avanzano 7 giorni col **solo coordinator OG**, salvano e non giocano la prima partita. Entrambi i pulsanti mostrano FORWARD TIME, usando font/hitbox OG; fino all'ultima fase sicura (28 luglio se la prima gara e' 1 agosto), poi il prossimo clic gioca **solo** la vera gara OG. Test puro aggiunto. **Eccezione importante:** nella diagnostica 1996/97, club `2c1b` (country44/div3) ha un vero incontro OG gia' il **4 luglio 1997** (phase49), e 66 risultati residenti al 1 luglio: non si possono posticipare queste gare fino ad agosto senza alterare il calendario OG; scelta da discutere separatamente. Nessuna amichevole ancora implementata. Nessuna modifica a match, CAR, CLASSIC/DIY/SEASON, automatico o disoccupato. **Windows 207 NON PROVATO**. [Report](OG_PRESEASON_MANUAL_ENTRY7_0013207_20261119.md).

## 19/11/2026 — 00.13.206, CAREER luglio e PANIC 6006 a fine stagione
Diagnostica **Windows 00.13.205** `DIAGNOSTICA_SESSIONE_1(20261119-161815).zip`: compilazione riuscita, ma nessuna chiamata `[career-july-forward]`; il test del target data era solo isolato. Il club dopo FINISH rimaneva su 1 luglio senza comando funzionante. La gara reale 2026/27 a phase80 veniva acquisita OG prima del crash; dopo `[world-cupd] round 3 decided` il motore segnalava `PANIC 0x6006` (FindTeamInDIY: club non trovato nella competizione). [CODICE 206] Riprova la preparazione dell'entry10 FORWARD TIME nell'effettivo `Expanded_UI_Prepare` (oltre a ContestMenuOnReturn), elimina la dipendenza da flag nativi di stagione potenzialmente obsoleti, mantiene il controllo OG residente, data/fixture reale e la barriera anti-replay; log `[career-july-menu]` diagnostica eleggibilita'. In `Career_FillCurrentSeasonInfo`, un nome storico della coppa NON basta a indicare che il club vi partecipa: verificare puntatore e contesto vivo prima di cercare la squadra; assenza in coppa opzionale -> stato originale `-104`, mai risultato inventato. `FindTeamInDIY` conserva PANIC per lega principale e contesti non riconosciuti, restituisce `0xffff` esclusivamente per assenza legittima in coppa opzionale CAREER e aggiunge log prima di altri PANIC. Nessuna modifica a score OG, CAR, CLASSIC/DIY/SEASON, mondo disoccupato o asset; restano irrisolti i mismatch del keycache gara `2c31` contro `2c30`. **Windows 206 e gameplay NON PROVATI**. Report: [OG_CAREER_JULY_PANIC_0013206_20261119.md](OG_CAREER_JULY_PANIC_0013206_20261119.md).

## 19/11/2026 — 00.13.205, FORWARD CAREER luglio + data post-partita
Dalla vera ZIP Windows 204: UNEMPLOYED AVANZA fino ad agosto in entrambi i mondi, mentre dopo FINISH SEASON il club in 2027 ha WORLD TODAY=1 luglio, NEXT OG=1 agosto, nessun comando weekly e la partita manager OG non aggiorna il clock civil. [CODICE] In CAREER impiegato il pulsante esistente NEXT MATCH diventa temporaneamente FORWARD TIME per il precampionato (stessa UI/callback ripristinato fuori scope); utilizza solo coordinator OG, non rigioca la prima partita. Ogni tick e' limitato alla **fase OG precedente** (28 luglio prima della phase56 del 1 agosto), non al 31 per evitare simulazione precoce con CEIL native phase. Un risultato vero della lega manager fa avanzare WORLD TODAY al giorno OG tramite stesso coordinator prima di autosave, solo se resident/local sono coerenti; nessuna variazione per UNEMPLOYED, DEBUG, CLASSIC/SEASON/DIY, salvataggi preesistenti. [TEST] nuovi casi C puri, **Windows 205 NON PROVATO**. Dettagli: [OG_CAREER_JULY_FORWARD_CLOCK_0013205_20261119.md](OG_CAREER_JULY_FORWARD_CLOCK_0013205_20261119.md).

## 19/11/2026 — 00.13.204: decisione July1 / Scotland stage0 e ingaggio
[APPROVATO] Normale FORWARD TIME da disoccupato, quando i 7 giorni attraversano la fine stagione, si ferma **esattamente al 1 luglio**; la pressione successiva avanza dalla nuova data. Saltare 30 giugno -> 7 luglio non e' piu' ammesso. [CODICE PREPARATO] Scozia 2627 div1-3: 4 cicli con `SP_TAB[7]=0` NON sono split reali; div0 con `SP_TAB[7]=1` resta protetta. Il residente contiene 15 risultati prima dell'ingaggio di Stirling Albion: l'handoff deve validarli per contatori/ID/classifiche senza ripeterli. Preflight anticipato prima di salary/autosave e della prima mutazione in PLAY MATCH. [QA] Windows 204 NON PROVATO; non concludere che il crash e' risolto finche' il test dell'utente non lo conferma. Report [OG_SCOTLAND_JULY1_HIRE_0013204_20261119.md](OG_SCOTLAND_JULY1_HIRE_0013204_20261119.md).

## 19/11/2026 — 00.13.200: corretto il rischio loop da recovery, engine ancora incompleto
198/199 ZIP Windows confermano GCC senza warnings espliciti. In 199 manual 2627 perde un partecipante El Salvador div4 (9/10, cup17/18) e apre resident2027 parziale con civil2026; poi 162932 blocchi `PreflightFixture`. Correzione 200: latch al primo errore, guard league/cup inclusi stage/catchup, report fault manual e log veri ID delle squadre; protegge CAR, NON risolve ancora roster/anno. Scheduler API usata solo per SHADOW July milestone senza dispatch, senza CAR e senza ack. **Priorità:** identificare 10° club dai log Windows200 e convalidare movimento OG senza cambiare di nascosto Mod; passare a atomic OPEN/country standings e claim fixture persistente; solo dopo dispatcher live e import data-driven. [Report](COMPETITION_ENGINE_SCHEDULER_SHADOW_AND_YEAREND_AUDIT_0013200_20261119.md).

## 19/11/2026 — A2.1 preparata, MA motore NON finito
BLOb 199 non committati: Coordinator PreflightFixture per leghe/coppe locali CAREER, latch su witness/commit resident falliti, elimina scrittura vecchio LWE journal per cup OG_ONLY; no nuovo simulatore. Blocchi reali per finire: A2 owner single-writer anche type2/cup manager, B fixture stable ID/claim+CAR transazionale manager+CPU, C capture qualificati OG ogni paese e anno prima reset, data-driven import e tre edizioni cross-year. Diag ZIP 197: zero GCC warning espliciti; 101 national cup 2027 ACCESS_UNAVAILABLE, non "101 ID errati"; Copa 2027 NON PROVATA; manual FINISH PANIC 2627. [Audit](COMPETITION_ENGINE_A2_SOURCE_AUDIT_0013199_20261119.md).

## 19/11/2026 — 00.13.198: avanzamento reale del motore, non fix manuale
Fase A1: controller unico OG+civile per START/FORWARD disoccupato, preflight del bind e LOAD; UI debug 4/5 resa visibile con disegno SWOS post-overlay. Regole approvate 18/11 intatte. Fase A2 PENDENTE: mappa owner mutanti di ogni (world,competition,edition) e integrazione reale NEXT MATCH manager/coppe; fase B: claim/risultato/stati/RNG/CAR atomico; fase C: ciclo edizione e qualificazioni generic data-driven; finale tre edizioni pilot. Finche' i gate sono aperti, impedire nuovi simulatori e duplicazioni. FINISH SEASON manuale deliberatamente rimandato su richiesta, NON considerato risolto. [Prove e limiti](COMPETITION_ENGINE_STEP1_COORDINATOR_0013198_20261119.md).

## 19/11/2026 — Priorita' Competition Engine dopo log 196
00.13.197 ripara l'accessibilita' del debug e aggiunge arresto protettivo per la sequenza type2 di fine anno; NON completa l'architettura. PRIORITA': (A) un writer reale per edizione (START/FORWARD/FINISH/HIRE/LOAD), (B) ID stabili + claim preventivo + risultato/stati OG e RNG/ack CAR atomici, (C) chiusura/qualifiche/movimenti/apertura transazionale senza dipendenza dall'occupazione del manager, (D) manifest e importatore con validazione formato, (E) tre edizioni continue della coppa pilota e secondo formato. Problemi reali: Champions/UEFA 1997 non residenti e Copa moderna senza fonte; 3 mismatch calendario/OG in ciascun log 196. Decisione prevalente 18/11 Expanded coordinator, OG executor. Non moltiplicare fix locali come sostituto dei gate.

## 19/11/2026 — 00.13.192: ingaggio a stagione OG conclusa e calendario club
- Diagnostiche Windows 00.13.191 (sessioni 1/2/3, 20261119-072627/072629): 2026/27 club 3333 lega a 90 gare, 1996/97 club 3327 lega a 132 gare, nessun evento futuro in keycache; sessione 3 poi 161.420 rifiuti della gara N+1 in un loop. Sessione 2 (1996, ottobre) conservava invece un evento futuro valido. Risultato non attribuito a coppe.
- \`PositionClubCalendar\` ora distingue INVALID=0, FUTURE=1, COMPLETED=2. Accetta COMPLETED solo con lega OG ordinaria gia' completa **sia nello stato residente sia nella copia locale**; posiziona sul vero ultimo header (non su un vecchio evento o su un puntatore oltre buffer), marca il menu di stagione terminata e invoca il riepilogo OG. Il manager NON genera una gara o un nuovo anno; l'utente termina tramite la vera azione FINISH SEASON. \`InitCareerClub\` gestisce COMPLETED senza falso warning. \`Expanded_MidseasonJoin_Begin\` evita il vecchio replay di una lega gia' copiata dal residente, soltanto se la completezza e' provata.
- \`Career_AutoAdvanceLeagueFixtures\` interrompe il percorso prima di leggere la cache terminata; \`DIY_PlayOrSimulateLeagueFixture\` rifiuta senza simulazione, segna il menu gia' concluso e registra **un solo evento per club/anno**; il callback del menu aggiorna visibilita'/focus per impedire il retry della stessa azione. Nessuna falsa vittoria, classifica, fixture o settimana aggiunta.
- \`CALENDAR_PAIR_MISMATCH\`: nel log 96, OG ha prodotto 2C14–2C1E (2–0) alla fase 80 mentre la keycache gia' giocata mostrava 2C14–2C24 (2–0). **Soltanto dopo** il vero score OG e il commit nel residente, il bridge corregge i QUATTRO byte degli ID casa/trasferta della singola riga keycache giocata, e solo con fase nativa esatta, score invariato, unico candidato, stesso manager, stessa competizione, tabelle e contatori residenti perfettamente coerenti. In caso incerto, \`BLOCK_PAIR_RECONCILIATION\` mantiene mismatch/errori; risultati, flags, fasi, classifiche, RNG e journal OG immutati. Le vecchie cache in CAR non vengono riparate silenziosamente.
- F12: \`CALENDARIO CLUB\` distingue prossimo evento futuro, stagione terminata (manual finish), cursore arretrato, cache invalidata e assenza manager. \`PAIR OG/CACHE\` mostra controlli/riallacciamenti/non risolti con stati OK/PARZIALE/ERRORE/N-D. Il parser Windows distingue \`PARTIAL_CACHE_IDS_REBOUND_NEEDS_WINDOWS_CHECK\` da errori reali, senza auto PASS.
- Test isolati **aggiunti ma non eseguiti in questa consegna**: semantica fine stagione/no-futuro e correzione identita' cache con score intatto, duplicati fase, corruzione, score diverso, club estraneo e idempotenza. Build Windows 00.13.192, warning, F12, gameplay, salvataggio e prove sui due mondi: **NON PROVATI**. Nessuna toolchain o EXE introdotto.
- Documentazione estesa: \`docs/OG_COMPLETED_HIRE_PAIR_GUARD_0013192_20261119.md\`.

## 18/11/2026 — 00.13.191: F12 con confronto assunzione compatto e verifiche OG/CAR
- Sostituita la colonna centrale duplicata: a sinistra solo copertura corrente R/DEF, F fase, G gare; a destra `PRIMA > DOPO` fissato sugli snapshot effettivi del bind (leghe, coppe nazionali, UEFA, altre, gare, contesti, view/contatori/fasi regredite) e `PRIMA > ORA` per regressioni successive. Se cambia mondo o anno, i numeri non vengono confrontati e mostrano N/D; snapshot non persistiti nel CAR.
- Pannello salute a destra, sempre read-only: mondo residente, unico orologio OG/civile, nuova stagione, origine qualificati OG, identita' dei qualificati Champions / seconda e terza coppa / Libertadores (ogni competizione con stato N/D/OK/ERRORE), coppe non residenti/fault, bridge locale, witness journal ed anomalie, aggregate classifiche, slot CAR su disco e ultimo SAVE/LOAD JSON realmente tornato con esito. Per qualifiche si confrontano ID e non soltanto quantita'; rotte prive di sorgenti non sono PASS. Altre confederazioni NON verificate.
- `CAR PREFLIGHT OK` significa soltanto read-only verifica dello slot del mondo corrente con schema/kind/world, orologio civile ed OG checksum. Non prova CONTINUE, un roundtrip o gameplay. Il test disco e' aggiornato ogni 5 secondi mentre F12 e' aperto (mai ad ogni frame). Gli esiti delle ultime chiamate SAVE/LOAD sono osservazioni transitorie con mondo associato, non inserite nei CAR e non includono test di avvio dopo caricamento.
- Status `OK`, `ERRORE`, `PARZIALE`, `N/D` limitati alla specifica verifica: nessun PASS sintetico di qualificazioni o salto stagione. Solo presentazione SDL F12, nuove sonde read-only e testimoni non persistenti SAVE/LOAD; nessun reset, simulazione, calendario, risultato, fisica o serializzazione modificati.
- Renderer reale: bitmap 5x7 a passo 7 px in 960x540, aggiunti glifi `>`, `Q`, `J`, `+`, `(`, `)`, una sola separazione verticale x378, layout interamente entro pannello logico 936x516. Nessun nuovo asset, nessun cambio font OG, un solo overlay F12; shortcut F12+1..8 e F9 invariati.
- Test isolato del puro confronto d'identita' `test_expanded_f12_compare.c` ampliato per distinguere `PRE->POST` e `PRE->LIVE`; correzione delle firme C del reader `SaveStarted/SaveYear` esplicite. **Compilazione Windows 00.13.191, warning globali, resa GUI, SAVE/CONTINUE e gameplay: NON PROVATI**. Utente compila/testa su Windows; nessun commit di toolchain o EXE.
- Dettagli e check: `docs/F12_HEALTH_COMPACT_0013191_20261118.md`.

## 18/11/2026 — 00.13.190: blocco 553ª gara OG dopo ingaggio a stagione conclusa
- Quattro ZIP diagnostici Windows della 00.13.189 (sessioni 1/2/3/4, ore 17:58): in sessione 2 (1996/97) e 4 (2026/27), dopo il bind del manager su un campionato OG da 24 club e 552 incontri residenti, l'avanzamento locale tenta la gara 553 (local 553, residente 552) e segnala `STALE_OR_DIVERGENT_LOCAL_BASE`. La ripetizione di `[bug23-fix] duplicate team fixture` prosegue nel ramo catch-up di `Career_AutoAdvanceLeagueFixtures`, privo del controllo di abort gia' presente nel ciclo principale. Non e' provata una causa hardware.
- Inserita guardia di fine stagione OG: solo per campionato ordinario con manager gia' collegato, formato 2–4 giri, contatori locale e residente entrambi uguali al numero teorico di incontri; verifica diretta in auto-advance e nel callback partita **prima** di risolvere/simulare un'ulteriore coppia. La guardia non crea incontri, classifiche, qualificati, trofei o nuovi stati e non altera CLASSIC.
- Chiuso anche il retry infinito del ramo catch-up quando il bridge gia' segnala il blocco: il CAR precedente rimane protetto. Test isolato del predicato per 552/552, 553/552, 552/551, 1104/1104 e leghe da 18 club. Non sono cambiati formato CAR, salvataggi, seed, coppe o regole OG.
- Il log Windows 00.13.189 riporta la compilazione completata senza GCC warning espliciti; **non** e' una compilazione della 00.13.190. Build Windows nuova, ingressi manager 96/26, SAVE/CONTINUE e correttezza completa delle coppe sono **NON PROVATI**. La presenza di residenti nazionali non dimostra tutte le qualificazioni post-luglio.
- Dettaglio: `docs/MANAGER_FINISHED_LEAGUE_GUARD_0013190_20261118.md`.

## 18/11/2026 — 00.13.189: fix TeamId in world_cups.h (build Windows non ancora provata)
- Da `DIAGNOSTICA_SESSIONE_1(20261118-174105).zip` / `COMPILAZIONE_0140.log`: compilazione 00.13.188 interrotta in `frontend.c`, che include `world_cups.h` prima di `game_types.h`, con `include/world_cups.h:89:35: error: unknown type name 'TeamId'`. Nessun altro error/warning GCC esplicito nel log prima dell'arresto; non e' una prova del resto della build.
- `TeamId` e' una `typedef struct TeamId` completa in `game_types.h`, 2 byte. In `world_cups.h` aggiunta la SOLA forward declaration `typedef struct TeamId TeamId;` vicino agli include: `WorldCups_PreviewEntrants` richiede un puntatore e non la definizione integrale. Nessun cambio ABI, layout, calendario, coppe, qualificate o CAR. Non forzare `frontend.c` a includere `game_types.h` ne' aggiungere una seconda struct.
- Nuovo test sorgente `sorgenti/tests/test_world_cups_header_order.c`, progettato per `-fsyntax-only` con due ordini degli header (anche con `-DSWOS_TEST_REVERSE_TEAMID_INCLUDE=1`); dichiarazione e definizione OG del tag validate in un test isolato GCC/Clang `-Wall -Wextra -Werror -pedantic` PASS. Test dell'header reale da eseguire sulla repository locale, build Windows e gameplay **NON PROVATI**.
- Versione `00.13.189`, singolo commit autorizzato; dettagli in `docs/BUILD_FIX_0013189_20261118.md`.

## 18/11/2026 — 00.13.188: fix compilazione GCC Windows da diagnostica sessione 1
- Input verificato: `DIAGNOSTICA_SESSIONE_1(20261118-173205).zip`, file `COMPILAZIONE_0140.log` e `AVVIO_COMPILAZIONE.log`. La 00.13.187 falliva nel preflight sintattico; l'EXE **non e' stato avviato**. I 18 test isolati risultavano PASS ma non provano build o gameplay.
- In `career_init.c`: prototipo `ShowErrorMenu(uint8_t *)` esplicito (stessa firma gia' usata nella viewer OG); conversione degli ARRAY byte legacy `_cupWinnersCup`/`_uefaCup` ai record `CompetitionCup *` eseguita sull'indirizzo dei dati invece che su `&array`; rimosso l'accesso alla variabile `phase` inesistente nel rollover, conservando il log quando il contatore cambia.
- In `career_status.c`: eliminata la dichiarazione locale non-`volatile` di `_isNationalSelector`, riusando la dichiarazione file-scope originale `extern volatile uint16_t`. NESSUNA modifica a RNG, fixture, classifiche, quali e CAR.
- Versione `00.13.188`. Isolato di compilazione delle firme/cast con GCC e Clang `-Wall -Wextra -Werror -pedantic`: PASS; non e' la compilazione reale di `SWOS_Expanded.exe`, che compete al test Windows. Stato: **COMPILAZIONE COMPLETA E GAMEPLAY NON PROVATI**. Report `docs/BUILD_FIX_0013188_20261118.md`.

## 18/11/2026 — 00.13.187: qualificate OG da disoccupato, entrambe le epoche (CODICE, NON GAMEPLAY PROVATO)
- Dopo la chiusura delle gare OG residenti dell'annata e PRIMA del reset del 1° luglio, il percorso senza manager legge piazzamenti, rose reali e vincitori OG dai contesti residenti; non richiama il callback che autosimula le vecchie gare. Usa gli esistenti selettori di qualificazione classic/UEFA e la Copa storica solo se presente l'effettiva sorgente. Il ramo moderno usa `UefaQual_Country/UefaQual_Fill` e non chiama `Conti_Country` (che potrebbe rigiocare turni continentali). Nessun altro motore di risultati.
- Nuovo witness `og_resident.qual_next` nel CAR: anno, mondo e quattro campi di ID in formato wire portatile; checksum e deduplicazione, carico senza risorteggio. CAR precedenti senza witness ancora leggibili ma con sorgente di qualificazione NON PROVATA; campo restaurato confrontato con le edizioni residenti se dello stesso anno e rifiutato su mismatch. Nessuna modifica ai seed iniziali approvati; gli slot internazionali ricevono gli ID solo se completi.
- La routine legacy dei qualificati non aggiunge calciatori al TMD del manager quando non esiste un manager; salva/ripristina lo scratch della UI. Nel 96, se il ranking necessita dei finalisti/coppe non presenti nel residente, si ferma invece di usare dati precedenti del manager. Vincitore preso da `tournament.header.winnerTeam`, non dall'alias dei codici risultato.
- Audit `[og-unemployed-qual]`, `[og-unemployed-qualification]`, `[og-qual-car]`; parser con separazione 96/26, campi mancanti, checksum SAVE e RESTORE. Test C isolati GCC/Clang -Werror: confronto ID, campi 16/32/36 e rilevamento duplicati: PASS, NON Windows.
- **Da verificare Windows**: passaggio 1996/97 e 2026/27 da disoccupato, presenza reale di finali/coppe OG per tutte le associazioni richieste, qualificati in prossima edizione, GAMEPLAY, SAVE/CONTINUE su ogni mondo; per access list incompleta la nuova edizione rimane bloccata come nella 186, senza qualificati falsi. Il resto delle confederazioni, il completamento della Copa moderna e mismatch marcatori/calendario rimangono separati.
- Relazione tecnica: `docs/UNEMPLOYED_OG_QUALIFICATION_0013187_20261118.md`.

## 18/11/2026 — 00.13.186: cambio stagione OG, coppe e diagnostica
- Corretto il percorso di cambio manuale per avanzare prima il mondo OG residente al 1 luglio, poi il calendario ausiliario, poi ricollegare il club; se il mondo o il bind non sono pronti l'autosave della nuova stagione e' **bloccato** (CAR precedente intatto). Interrotto il ri-tentativo senza fine quando il preflight bridge OG rifiuta una partita; guard sempre attivo, nessuna partita fittizia.
- Riuso degli ID qualificati dalle ORIGINALI funzioni `career_eurocups.c` / `uefa_access.c` per preparare nuove edizioni residenti UEFA 2026/27 e tre coppe storiche 1996/97, piu' Copa storica quando presente il campo nativo valido. No seed dell'anno precedente replicati; continuazione da CAR usa contesti OG gia' residenti.
- Rollover nativo del contatore stagioni delle coppe nazionali anche da disoccupato senza doppia simulazione. Audit `[og-cup-entry]` confronta seed primo anno e posti seconda stagione con gli ID del residente; senza piazzamenti OG finali completi segnala FAIL e non inventa qualificati. Il test `OG_FIRST_EDITION_CUP_SEED` distingue origine nativa o manuale approvata; dati manuali NON installati.
- Profiling `[og-forward-perf]` e `[og-forward-cycle]`: campionati, coppe, internazionali, overhead, millisecondi, confronto 1996/97 vs 2026/27. Parser Windows aggiornato. Mismatch del calendario e marcatori preservati come errori non risolti.
- **LIMITI APERTI**: qualificazioni senza manager NON connesse al nuovo campo UEFA; il test le segnala FAIL invece di PASS/NOT_TESTED neutro. CAF/AFC/OFC/CONCACAF/Libertadores moderna, accessi completi delle coppe nazionali, storico dei residenti precedenti e modalita' manuale richiedono lavoro ulteriore; nessun calendario/risultato inventato. Report completo: [OG_SEASON_ROLLOVER_0013186_20261118.md](OG_SEASON_ROLLOVER_0013186_20261118.md).
- Verifica isolata GCC/Clang del confronto ID e provenance prima edizione PASS; **build Windows, warning globali e gameplay 00.13.186 NON PROVATI**. Unico commit dopo autorizzazione `committa`; versione 00.13.186.

## 18/11/2026 — 00.13.185: test ID qualificati -> competizione OG dell'anno dopo
- Aggiunto audit read-only del pool qualificati prodotto dai percorsi OG di fine stagione, confrontato **per world, edition year, competition slot e club ID** con le squadre delle VERE edizioni residenti dopo luglio. Non bastano i conteggi. Log `[og-qual-next] SOURCE/ROUTE/SUMMARY` e `[og-qual-next-source]` per ciascun ID nativo, diagnostica aggregata `OG_NEXT_SEASON_QUALIFICATION`.
- Nuova edizione assente = **FAIL_NEW_EDITION_MISSING**; elenco diverso/duplicato = FAIL, mai PASS fittizio. Unoccupato senza sorgente, domestic cup access, altre confederazioni CT_ED e qualificazione 2026/27 Libertadores **NON TESTATI** da questo percorso. L'engine di qualificazione futura NON e' stato implementato; non simulare o inserire squadre per far passare l'audit.
- Test puro del matcher e parser sintetico nel runner; nessuna modifica a motore OG, RNG, calendario, CLASSIC o CAR. Full build e gameplay Windows **NON PROVATI**. Report: [OG_QUALIFICATION_ROLLOVER_TEST_0013185_20261118.md](OG_QUALIFICATION_ROLLOVER_TEST_0013185_20261118.md). Versione progressiva 00.13.185 dopo commit 184.

## 18/11/2026 — 00.13.184: correzione diagnostica OG, mismatch calendario e conteggio revisioni
- Verificati i CAR e i log delle tre sessioni Windows **0.13.104** (20261118-140823/140825). Il bridge locale OG -> residente ha 91 commit di stato (66 2627 + 25 9697), zero WRITE_BLOCKED, checksum journal validi. Questo e' un fatto dei log ricevuti; la 00.13.184 non e' ancora compilata/testata su Windows.
- Corretto il falso **CPU_FIXTURE_NOT_OBSERVED**: per una lega con manager, sia gare manager (source=2) sia CPU locali (source=3) sono prodotte dal medesimo OG e testimoniate nel CAR prima del commit del residente. Non cercare una seconda simulazione CPU residente, che sarebbe vietata. Log locali ora distinguono attesa commit snapshot da collisione con una gara OG residente della **stessa fase**. Un incontro residente della stessa coppia ma **di fase differente** e' un ciclo storico, non **SCORE_OR_PHASE_DIVERGENCE**: due stagioni/cicli non vengono fusi per sola coppia casa/trasferta. Collisione reale di stessa fase blocca il nuovo CAR.
- **CACHE_UNVERIFIED** non era solo una falsa segnalazione: nei 10 casi osservati il keycache presenta una gara DIVERSA nella fase della partita nativa; esempio 2627 fase 76: OG 0857-0847, calendario 0847-0844. Le classifiche native coincidono (row-diff=0) ma il calendario e' davvero disallineato. Nuova classificazione read-only **CALENDAR_PAIR_MISMATCH**, con ID, score e flag della riga calendario. Non sovrascrivere risultati, rose, RNG o fixture per occultare il problema. La riconciliazione calendario OG/keycache resta **APERTA**, non PASS.
- L'analizzatore Windows `audit_windows_runtime_log.c` distingue ora `OG_MANAGER_LOCAL_OG_BRIDGE` (witness/commit/blocchi) e `OG_MANAGER_CALENDAR_PAIR_IDENTITY=FAIL_REAL_PAIR_MISMATCH` (coppie realmente discordanti). Anche il test statico del parser contiene un caso di mismatch, senza spacciarlo per gameplay verificato.
- Nuovi test isolati `ExpandedOGCache_FindPhaseSlot` e `ExpandedOGFixtureHistory_Check`: calendario con fase valida ma coppia errata, coppia altrove solo scheduled, molteplici entry/corruzione, stessa coppia in fase storica diversa, collisione della stessa fase e score diverso. Contratti oggetto di test C senza nuova simulazione. Windows 00.13.184 **NON PROVATO**.
- **Regola numerazione richiesta dall'utente, prevalente sulla vecchia regola del 10/11**: il primo commit del ciclo 0.13 e' 0.13.01; nel ramo dev/0.10, fino alla 0.13.104 inclusa, 207 commit totali meno 24 merge = **183 revisioni effettive**. Conta anche commit di soli fix, docs e asset, senza contare merge tecnici. Il successivo commit diventa **00.13.184**; incrementare il terzo campo di 1 per ogni successiva revisione non-merge autorizzata, mantenendo la forma `00.13.xxx` finche' l'utente non modifica milestone. Questa regola sostituisce la precedente eccezione per commit solo documentali.
- Ripulita la copia duplicata `docs/reference/COMPETITION_FORMAT_AUDIT.csv` (stesso blob SHA di `docs/COMPETITION_FORMAT_AUDIT.csv`); mantenuta quest'ultima come copia canonica. Il CSV generato nei pacchetti diagnostici non viene modificato.
- Rapporto e procedura: [OG_DIAGNOSTIC_CORRECTIONS_0013184_20261118.md](OG_DIAGNOSTIC_CORRECTIONS_0013184_20261118.md). Nessun merge, checkout o modifica al ramo main/release.

## 18/11/2026 — 0.13.104: bridge campionato OG locale -> residente (manager + CPU)
- Sui campionati OG ordinari con manager attivo, il percorso **locale OG** esegue le gare; `Expanded_OGWorld_AdvanceLeague` NON lancia un secondo esecutore CPU residente per la stessa lega. Dopo ogni gara reale (manager **o** CPU locale), `Expanded_OGWorld_PreflightLocalLeague` verifica prima della partita che contatori e classifica per ID club coincidano con il residente; il callback finale registra l'esatto risultato OG nel journal e `Expanded_OGWorld_CommitLocalLeagueFixture` controlla l'incremento atteso di una gara, punti/reti/WDL, rimappa tutti gli `teamSelOffset` (manager `-1` -> indice nativo), quindi copia competizione, squadre e RNG nel residente **senza seconda simulazione**.
- Il journal portatile CAR resta `format=1`, `record_size=20`: nuovo `source=3` indica gare CPU eseguite nel **contesto locale OG** (1=residente, 2=manager). Per le leghe ordinarie si preserva il `DIYTournamentState` residente, poiche' OG non aggiorna il suo buffer torneo in assenza di gironi; non si importano indici di stage locali senza mappa validata. A carico del CAR esistente, nessun nuovo file di salvataggio.
- **Fail closed:** ID/riga duplicata o mancante, baseline locale diversa, salto di contatore, score/delta di classifica incoerenti, journal duplicato oppure residente mancante -> `[og-manager-bridge] WRITE_BLOCKED` e il CAR precedente e' preservato; nessun match riprodotto. Dopo esonero il residente torna al proprio percorso CPU.
- **Limiti da non nascondere:** bridge attivo SOLO campionati `hasGroups==0` e `s_expOGBound` valido; tornei a gironi/split, coppe domestiche/internazionali, transfer/post-match extra, caricamento di vecchi CAR **gia' divergenti**, e competizioni che superano il 1 luglio richiedono ancora integrazione specifica. `CPU_FIXTURE_NOT_OBSERVED` puo' comparire prima del commit snapshot locale; il segnale di applicazione e' `[og-manager-bridge] COMMITTED`. **Fase 3 NON conclusa**. Verifiche Windows della 0.13.104: **NON PROVATE**. Test C del remap/score isolato: disponibili nel nuovo runner; NON sono gameplay.
- Rapporto e protocollo: [OG_LOCAL_RESIDENT_BRIDGE_013104_20261118.md](OG_LOCAL_RESIDENT_BRIDGE_013104_20261118.md).

## 18/11/2026 — 0.13.103: audit CPU/manager, orientamento cache e test storico
- Corrette **cinque** inizializzazioni ad array annidati in `sorgenti/tests/test_expanded_competition_history.c` (`{{0}}` -> `{0}`) che avevano prodotto cinque errori `-Werror=missing-braces` in `run_test_expanded_competition_history` nelle tre sessioni Windows 0.13.102. Il test sulla 0.13.103 e' **NON PROVATO**.
- Il confronto read-only `[og-manager-fixture]` aggiunge `reverse-cache`: cerca anche la coppia orientata inversamente, con reti speculari; `PLAYED_MIRRORED` indica solo un candidato e **NON** risolve il binding, non autorizza merge, non cambia lo `status`. Nessuna simulazione, classifica, RNG, journal, CAR o gameplay modificati da questo audit.
- Anomalie 0.13.102: `CPU_FIXTURE_NOT_OBSERVED` 5/5 partite del manager significa assenza di controparte osservata nel journal residente a quell'istante, NON prova di doppia gara; `CACHE_UNVERIFIED` 4/5 significa che la ricerca della chiave orientata/fase nel cache non conferma il risultato, NON prova di CAR corrotto; `TABLES_DIVERGED` 1/5 (2627, country=51 div=4, 3-1; residente=15 incontri, locale=24; 10 righe differenti, 0 unmapped) indica stati OG non riconciliati. Il dato nativo non deve essere sovrascritto o risimulato sulla base del solo punteggio. Caso 9697: dopo gara 1-0, residente=45 e locale=48. La correzione manager/residente **NON E' ANCORA RISOLTA**, single-writer fase 3 aperto.
- Analisi e protocollo: [OG_MANAGER_DIVERGENCE_AND_HISTORY_TEST_013103_20261118.md](OG_MANAGER_DIVERGENCE_AND_HISTORY_TEST_013103_20261118.md). Full build Windows, warnings e runtime della nuova 0.13.103 **NON PROVATI**; l'utente compila/testa. Non eliminare CAR, non reimportare score e non avanzare automaticamente residenti divergenti.

## 18/11/2026 — 0.13.102: witness fixture OG, confronto FORWARD e cache CAR 8192
- Implementato **witness osservazionale** di risultati gia' prodotti da OG per campionato/coppa residenti e partite manageriali. Solo CAREER SDL; nessun callback di nuova simulazione o variazione di gameplay/RNG. Identita' con mondo implicito, competizione, anno-edizione, fase, progressivo nativo, coppia orientata, variante; indice hash per intercettare doppia scrittura o score divergente, bloccando un NUOVO CAR se rilevati. Questo NON e' ancora un single-writer preventivo tra manager e CPU.
- Ogni `Expanded_OGWorld_AdvanceToDate` (compresi FORWARD da disoccupato e salti debug) confronta prima/dopo contatori nativi e numero di partite osservate. Il confronto per coppia manager/residente e' registrato separatamente: esito esplicito anche quando ID o score non sono riconciliati.
- CAR: cache OG raw e fixup endian `1870 -> 8192` con validazione occupazione prima del write, preflight read-only di vecchi CAR 1870 e nuovi 8192, tail zero in LOAD Windows. Lo stesso CAR atomico include `og_resident.fixture_journal` con wire portatile/checksum; CONTINUE verifica checksum e ripristina testimoni senza rieseguire gare; i vecchi CAR senza il journal sono `partial=1`, senza inventare score.
- **Restano aperti:** binding single-writer preventivo manager/CPU, tutte le fixture datate esattamente, qualificazioni e tre edizioni, summary storico finale, test di regressione Windows. Non marcare fase 3 completa. Gli stati OG veri sono sempre autoritativi. Obiettivo invariato: nuove competizioni definite tramite dati/regole, senza codice C dedicato per ogni torneo.
- Report tecnico e protocollo SI/NO/NON PROVATO: [OG_RESULT_WITNESS_SAVE_013102_20261118.md](OG_RESULT_WITNESS_SAVE_013102_20261118.md). Full build e gameplay Windows 0.13.102 NON PROVATI; warning nuovi da verificare sulla compilazione completa.

## 18/11/2026 — 0.13.101: risultanze OG calendario/manager, prova di riconciliazione READ-ONLY
- Le gare visibili nel calendario del club non sono inventate: `Career_UpdateStatsAfterMatch` conserva punteggio/flag (piu' eventuali rigori) nella `teamSelectionKeyCache`, le routine DIY OG aggiornano classifiche/tabelloni, e il CAR ne conserva una rappresentazione. Questo **NON dimostra** ancora il journal datato di tutte le gare CPU del mondo.
- `Expanded_OGWorld_BindClub` copia il campionato residente nel contesto locale; il test `Expanded_OGWorld_DebugCanAdvanceClub` blocca divergenze. Nuova verifica runtime `[og-manager-fixture]` confronta score gia' prodotto, ID orientati, fase, conteggi e classifica per club; usa parser bounded della keycache e test C isolato. **E' un audit non mutante, NON un merge ne' una seconda simulazione.**
- Scoperta critica: `career_teams.c` dichiara una cache runtime di 8192 byte, `save_format_json.c` serializza il raw cache e gli override endianness con limite 1870. Il nuovo log misura `cache-used` / `match-end` e segnala `car-capacity-risk=YES` se eccedono; nessuna perdita nel CAR reale e' provata finche' non si esamina il suo contenuto. Prima di aumentare il salvataggio, definire restore compatibile con CAR preesistenti.
- Obiettivo centrale invariato: importare competizioni da OG/altri SWOS/nuove definizioni con regole condivise e senza patch C a torneo. Single-writer effettivo, CAR atomico con ledger ed eventi e fase 3 **NON COMPLETATI**. Compilazione completa e gioco Windows 0.13.101 **NON PROVATI**. [Rapporto e checklist](COMPETITION_ENGINE_OG_MANAGER_RECONCILIATION_013101_20261118.md).

## 18/11/2026 — 0.13.100: archiviazione per DATA DELLA GARA e test diagnostici
- Corretto il contratto di compattazione del journal: l'anno-stagione e' calcolato sul giorno effettivo della **singola gara**, NON sull'anno d'inizio dell'edizione. Una coppa iniziata a febbraio 2027 puo' avere partite sia nella stagione 2026/27 sia nella stagione 2027/28; queste ultime restano dettagliate fino al termine della finestra biennale.
- Prima di compattare: rifiutare risultati datati nel futuro, edizioni ancora in corso, conclusioni ancora future e fatti di qualificazione non confermati. Test aggiornato sulle partite di aprile/novembre e sulla protezione del calendario luglio-giugno.
- Il runner Windows diagnostico riconosce ed esegue due test isolati Competition Engine (scheduler e retention), riportando scope COMPETITION_ENGINE_ISOLATED, MA NON certifica gara reale, checkpoint CAR o gameplay.
- **OBIETTIVO VINCOLANTE DI QUESTO LAVORO:** poter aggiungere/importare competizioni OG, altre versioni SWOS e originali tramite definizione e regole parametrizzate, senza dover modificare codice specifico per ogni torneo; facilitarne diagnostica, calendario, qualificazioni, storico, salvataggi e nuove edizioni. Il collegamento OG e' un prerequisito, non il traguardo.
- Audit: nei residenti `Career_AutoAdvanceLeagueFixtures` e `Career_AutoAdvanceCupMatches` chiamano `DIY_PlayOrSimulateLeagueFixture` e `DIY_PlayCupMatchAndAdvance`, che gia' producono ed applicano risultati OG. In `Expanded_OGWorld_AdvanceUefa` il caso manager usa `PENDING_MANAGER_BIND` e NON deve essere sostituito con un secondo writer; per le coppe nazionali esiste `managed_excluded`. Nel binding campionato e' documentato il blocco quando locale/residente divergono. **Non esiste oggi un ID fixture comune verificato per entrambi i percorsi, ne' un CAR che checkpointi lo stesso risultato col nuovo scheduler**. Il gate 2d/2e/2f e quello della fase 3 restano APERTI; non attivare un nuovo dispatcher o millantare completamento.
- Stato consegna: modifica minima a codice isolato e runner, nessuna modifica ai callback OG, al formato CAR o al gameplay. Test Linux/Windows della nuova build e warning nuovi NON PROVATI fino a esecuzione. [Rapporto tecnico](COMPETITION_ENGINE_RECONCILIATION_GATE_013100_20261118.md).

## 18/11/2026 — 0.13.99: regola journal due stagioni nel Competition Engine (codice isolato)
- Decisione dell'utente: **solo la stagione corrente e quella precedente** conservano risultati delle singole gare nel nuovo journal; le edizioni precedenti rimangono nell'archivio con **solo vincitore, finalista se applicabile, giorno di chiusura, numero gare e ID immutabile dell'edizione**. L'archivio NON duplica formazione, reti per partita, statistiche, calendari, ranking o RNG.
- In `expanded_competition_history.h`: record giocati provenienti dal ledger single-writer, deduplica/anti-riscrittura, tombstone per edizioni concluse, chiusura verificata e compattazione condizionata; uso dell'anno stagionale luglio-giugno, nessun reset della coppa ancora attiva al 1 luglio. **Non comprimere finche' qualificazioni e dipendenze non sono persistite separatamente e confermate**.
- Test aggiunti per archiviazione, duplicati, world 9697/2627 e coppe cross-year; test C isolato e build Windows 0.13.99 **NON PROVATI in questa consegna**. Nessuna integrazione live OG o nuovo campo CAR; il gate fase 3 non e' completo fino a dispatcher single-writer, checkpoint CAR con RNG e CONTINUE verificato. [Rapporto](COMPETITION_ENGINE_HISTORY_RETENTION_20261118.md).

## 18/11/2026 — 0.13.98: fase 3 (nucleo scheduler isolato), controllo coppe
- Scheduler a eventi civili ordinati, jump al prossimo evento, blocco dipendenze, ID univoci e validazione stato; nessuna chiamata OG, nuovo writer o serializzazione CAR. [Dettagli e gate](COMPETITION_ENGINE_PHASE3_FOUNDATION_20261118.md).
- Incluso il fix gia' preparato del verificatore OG_CUP_START_COVERAGE: campi scoped da og-cup-start, evitando confronto 51 domestiche con 53 contesti anche storici.
- Test isolato C su Linux con GCC -Wall -Wextra -Werror -pedantic: PASS_ISOLATED_ONLY con piccolo date stub. Script con libreria Gregorian reale, build Windows e gameplay NON ESEGUITI. CAR, fixture single-writer e coppe cross-year NON PROVATI; fase 3 NON completa.

## 18/11/2026 — 0.13.97: registrazione completa club, viewer marker e rumore legacy
- I 8 `LEDGER_REJECTED` del runtime Windows 0.13.95 sono le squadre ordinali 26..29 di Argentina (file 43) e USA (file 73): `expanded_world_live.c` limitava a 26 `LIVE_MAX_LEAGUE_CLUBS` sebbene i due campionati abbiano 30 club OG. Limite del SOLO registro ausiliario portato a 32, coerente con le squadre native; `NATIVE_FIXTURE_MAX_CLUBS=24` resta separato e l'OG-only continua a NON generare incontri LWE. Verifica Windows nuova richiesta: nessun ledger rejected, 2003 club accettati contro 1995 prima (a parita' di database). Non dichiarare gameplay o save provati.
- `[og-international-resident] continental=VIEWER_NOT_BOUND` era INCONDIZIONATO, pur esistendo `exp_og_view_uefa` e `Expanded_OGWorld_ViewLeague` con ripristino del contesto residente OG; il messaggio ora riporta `viewer-route=NATIVE_OG_TYPE2_PRESENT` oppure `NO_RESIDENT_CONTEXT`, con `viewer-visual-test=NOT_TESTED`. La route presente non prova UX/controller/competizioni di altre famiglie.
- Log legacy ridondanti rimossi: il singolo `[world-import-cup]` per ogni definizione di coppa OG e il singolo `[world-import-exclusion]` per ogni club intenzionalmente `OUTSIDE_LEAGUE`. I conteggi originali restano in `[world-import]`, `[world-cup-load-audit]`, diagnostica F10; rifiuti inattesi `WRONG_FILE/UNSUPPORTED_DIVISION/LEDGER_REJECTED` conservano logging completo. Parser Windows aggiornato per tracciare marker vecchi e nuovi separatamente.
- Nuova regola: eliminare log obsoleti SOLO dopo verifica della causa, senza nascondere warning, errori, NO_RESIDENT o dati mancanti. Restano le 26 competizioni che attendono il motore futuro. La modifica non altera il calendario OG, il risultato delle gare, i CAR, il controllo del joypad o l'RNG.
- Statico: confronto sorgenti e cause effettuato. Build Windows 0.13.97, warnings e runtime NON PROVATI. Committare una volta su dev/0.10.

## 18/11/2026 — 0.13.96: audit stabilita' Windows persistente
- Individuato un errore del parser `sorgenti/tests/audit_windows_runtime_log.c`: ad ogni `[og-career] RESET` azzerava errori e primo fallimento, causando possibili falsi esiti positivi per log con due carriere o due mondi. Ora cumula i fallimenti delle sessioni precedenti, preserva il primo fallimento e non esprime un PASS se un'altra carriera ha gia' fallito.
- Aggiunto `OG_LOG_NOTICE_SUMMARY` per file SDL controller opzionale, title autosave skipped, esclusioni import per causa, `NO_RESIDENT`, stringhe `VIEWER_NOT_BOUND`, reset di fase e messaggi non verificati; i conteggi sono OCCORRENZE del log, non numero di gare o competizioni, e non sono trattati falsamente come errori.
- Restano 26 definizioni senza stato mondiale OG residente (12 coppe continentali, 13 stati brasiliani, Coppa Italia Serie D); approvato rinvio al nuovo motore, SENZA introdurre ora orologi/risultati paralleli. I messaggi `VIEWER_NOT_BOUND` sono marcatori hardcoded, non prove di fallimento: eventuale collegamento del viewer va testato su Windows. Gli 8 `LEDGER_REJECTED` appartengono al vecchio import LWE; nessuna rimozione/aggiunta di club OG senza audit separato.
- Codice runtime OG, save, regolamenti, controller, coppe e simulazione INVARIATI. Compilazione Windows 0.13.96, warning e test cross-career NON PROVATI, richiedere diagnostica reale. Riferire comunque TUTTI gli avvisi nei log e separare i limiti rinviati.

## 18/11/2026 — 0.13.95: gol OG e reset di fase, segnalazione integrale dei log
- Nuova regola: comunicare sempre tutti i warning/errori e messaggi anomali in log, inclusi SKIP, FAIL e diagnostica save, anche non bloccanti. Distinguere quantita' verificata da esito non verificato.
- 1996/97: nel log 0.13.94 i 475 autogol residenti riconciliano esattamente 11.881 gol individuali con 12.356 GF OG. Nessun dato partita deve cambiare; diagnostica resa esplicita.
- 2026/27: cinque anomalie dei totali riscontrate nelle divisioni (43,0),(49,0),(64,0),(65,0),(71,0). Le tabelle generate `kWorld2627_split*_0` contengono uno o piu' stage con flag F_RESET: `league_split.c::freeze` azzera gamesPlayed/W/D/L/GF/GA/points della classifica mentre `goalStats[0]` e `writesZeroHere` sono cumulativi stagionali. Comparare i due totali come se fossero la stessa finestra e' scorretto.
- `Expanded_OGWorld_ScorerAudit` ora legge in sola lettura `LeagueSplitState` e `World_LeagueSplits()`, identifica le competizioni che hanno attraversato uno stage F_RESET e classifica differenze come `STAGE_RESET_DIFFERENT_SCOPE`, separandole da `UNEXPLAINED_SAME_SCOPE`. Tutti i contatori native OG, i salvataggi CAR, il calendario, le partite, le regole e l'RNG rimangono invariati.
- Test statico: i cinque record sopra hanno F_RESET; non dimostra che ogni competizione sia riconciliata in runtime. Build completa, warning GCC e test gameplay Windows 0.13.95 NON PROVATI; richiedere diagnostica Windows per la nuova classificazione e riferire TUTTI i messaggi.

## 18/11/2026 — 0.13.94: OG-only standings audit and native own goals
- L'evento Windows 0.13.93 `[join-standings] SKIP world table missing country=51 div=4 club=332d` deriva da `DIY_WorldAuditStandingsAtMenu`, che cerca il vecchio ledger LWE mentre `EXPANDED_WORLD_OG_ONLY=1` forza `ExpandedWorldLive_Table()` a ritornare 0. Il confronto con il ledger disattivato e' ora saltato in OG-only, senza sopprimere verifiche OG e senza creare classifiche fittizie.
- In `match_sim.c`, il percorso `AssignSimulatedGoalsToScorers()` incrementa `TeamGeneralInfo.writesZeroHere` quando il tiro marcato come autogol non attribuisce `PlayerFile.goalStats[statCol]`. `teamInit()` azzera entrambi alla inizializzazione della squadra. L'audit in `career_status.c` aggrega gli autogol delle squadre residenti, confronta `player-goals + own-goals` contro `leagueStandings[].goalsFor`, e distingue leghe riconciliate da scarti residui; i primi sei scarti non spiegati vengono tracciati separatamente.
- Patch solo diagnostica: nessuna modifica a simulazione OG, classifiche, goalStats, autogol, save CAR, input o RNG. Windows e compilazione 0.13.94 NON PROVATI. Un successivo log deve stabilire quanti residui restano; non chiamare corretto cio' che non e' ancora dimostrato.

# SWOS EXPANDED — IMPROVEMENTS

## 18/11/2026 — warning GCC 0.13.90 e regola permanente
- L'utente richiede di NON ignorare nessun warning: correggerlo appena emerge, mantenere attivo il relativo controllo e fare una full build di verifica, senza esiti PASS simulati. Per 0.13.91 preparate riparazioni mirate dei 91 warning Windows 0.13.90 (72 packed, 12 pointer cast, 4 font, 3 commenti) e corretta anche l'opzione sconosciuta `-Wno-braces`. Nessuna modifica ai file CAR o al gameplay volontaria. [Audit/limiti](WARNINGS_AUDIT_01391_20261118.md); full Windows, input, save, OG e 0-warning **NON PROVATI**.


## 18/11/2026 — 0.13.90: warning reali e scrittura atomica
- Riparati nel codice i quattro warning GCC della full build 0.13.89 (initializer, dangling pointer, due commenti); `-Waddress-of-packed-member` reso visibile per il prossimo full rebuild. Attenzione: 29 warning precedenti non dichiarabili zero finche' il compilatore non li controlla senza soppressione.
- Salvataggio: singolo Win32 5 in 2627 su MoveFileExA dopo la serializzazione; retry con backoff limitato per errori 5/32/33 senza mai eliminare il CAR precedente. No conversione save, niente nuovo motore o cambi OG. [Rapporto](WARNINGS_AND_SAVE_RETRY_01390_20261118.md). Test simulati isolati SI; Windows, conteggio warning e persistenza **NON PROVATI**.


## 18/11/2026 — 0.13.89: accessi packed rivisti (Windows da verificare)
- Patch dei 29 punti di `-Waddress-of-packed-member` riscontrati nei tre log della 0.13.88, in sei sorgenti/header: accessi con allineamento garantito o `memcpy` e assert compile-time; nessuna alterazione volontaria di struct OG, gameplay e formato dei file CAR.
- [Rapporto](PACKED_ALIGNMENT_AUDIT_20261118.md) e test C isolato inclusi. Mancano build Windows della 0.13.89, nuovo conteggio warning, roundtrip dei CAR per entrambi i mondi e regressioni: **NON PROVATI**.

## 18/11/2026 — build 0.13.88: TeamDb source alignment + contract fixture (Windows DA PROVARE)
- teams.c e team_db.c usano stesso team_source.h per file storici 012/013/038/073 in 96/97 CAREER; TeamDb cache per mondo e mode evita letture legacy nel contesto corretto e mantiene puntatori esistenti. Asset e CAR non modificati.
- expanded_competition_identity.h: tipi di ID e pre-claim anti-doppio-writer, test C isolato. NON attivo nel gameplay/CONTINUE, non certifica assenza doppie partite in SWOS. [Dettagli e rimanenti](COMPETITION_ENGINE_POINT2_20261118.md).
- Build Windows, regressioni 96/26/CLASSIC e test save NON PROVATI. Nessun secondo simulatore e nessuna coppa migrata.


## 18/11/2026 — audit punto 1 chiuso, B rinviata, bug France/Turkey documentato
- RNC decodificati isolatamente: France generic/storico 18/20 league0, Turkey 17/18; `team_db.c` continua a leggere il generico e mantiene cache non separata per mondo mentre `teams.c` usa override in 9697. Bug di coerenza della sorgente, non test runtime. [Relazione](COMPETITION_ENGINE_PHASE1_CLOSURE_20261118.md).
- Scelta B approvata per formati storici evolutivi **dopo** le coppe basiche; audit statico completo, codice adapter e Windows non provati. Tutto documentale, versione 0.13.87 invariata.


## 18/11/2026 — audit tecnico DOCUMENTALE nuova infrastruttura competizioni, 0.13.87 invariata
- Audit statico e comparazione dei simulatori OG: [COMPETITION_ENGINE_OG_AUDIT_20261118.md](COMPETITION_ENGINE_OG_AUDIT_20261118.md). Pilot proposto Champions storica OG 96/97, 16 partecipanti/quattro gironi/KO, tre annate consecutive dopo verifiche di formati e qualificazioni.
- I risultati di `CalculateViewResult`, `world_conti.c`, `uefa_access.c` e `world_cupd.c` non sono intercambiabili; la funzione integrale OG di season-end simula e muta competizioni. Salvare nuove fixture nel CAR og_resident esteso, non creare doppio ledger/simulatore.
- TODO 7 attivita'/3 traguardi aggiornata; solo analisi statica del punto 1 fatta, prove side effect, goldens, build Windows e nuovo engine NON PROVATI. Nessuna modifica C/save/EXE/versione.


## 18/11/2026 — 0.13.86: correzione X A/B e deriva grafica del menu TRAINING (Windows DA PROVARE)
- Segnalazione reale Windows 0.13.85: screenshot `image(20261117-234106).png`, diagnostica `DIAGNOSTICA_SESSIONE_1(20261117-234115).zip`, HEAD locale `c186623`. La compilazione Windows 0.13.85 e' riuscita (scritto `Compilazione completata: SWOS_Expanded.exe aggiornato`); in SWOS.log ci sono `[training-cross] SOURCE` e `SWAPPED` sia A->B che B->A (esempi 10/15, 1/20), ma il log 0.13.85 dichiarava OK senza confrontare le identita', l'utente vedeva i nomi invariati e componenti grafici spostati.
- Radice grafica nel codice: `Expanded_TrainingCrossSwapX` della 0.13.84 chiamava `PlayMatchMenuReinit()` DOPO il secondo X, mentre il menu era gia' attivo; `PlayMatchMenuReinit` sottrae ripetutamente 9 px dalle Y delle entries 12/29/30 per panchine corte e non e' idempotente. Rimosso il richiamo nel percorso X: aggiornati soltanto i callback dati, focus e `DrawMenu` (come nello swap OG).
- Radice visiva dello scambio: `TrainingMatch_BuildCurrentTeamFromCareerTeam()` ricostruisce e RIORDINA automaticamente i giocatori B per ruolo/valore, percio' il giocatore arrivato dallo swap puo' finire in una riga diversa o fuori dalla parte visibile. Nel solo percorso X si impedisce l'auto-rebuild di B da `PlayMatchMenuSwapPlayers`. `TrainingMatch_SyncBForCareerSwap` aggiorna *in place* le COPIE B dei due slot coinvolti, mantiene le posizioni selezionate e conserva il numero di maglia di allenamento. Il cambio effettivo rimane sul canonico `_currentMatchPlayers` tramite callback OG, mai sostituendo i record del database.
- Lo snapshot `_lastMatchPlayers` della formazione ufficiale, normalmente ripristinato dopo TRAINING, riceve l'esplicito scambio X tramite identificativi PlayerFile (non ordinali, che possono differire). Se manca un ID non si inventa un giocatore: il log indica `official-match=NOT_IN_SAVED_LINEUP`. Anche le maglie dello snapshot sono scambiate solo nei mondi che non usano numeri reali.
- Log nuovi: `[training-cross] VERIFIED A-slot=... file=prima->dopo B-source-slot=... file=prima->dopo B-copies=... official-match=UPDATED/NOT_IN_SAVED_LINEUP B-order=RETAINED status=...`. In caso di indici non validi o scambio effettivo non avvenuto: `REJECT` / `FAILED_INVARIANT`, nessun falso OK.
- Revisione sorgenti e controlli statici: eseguiti, **non equivalgono a build e gameplay Windows**. La 0.13.86 resta NON PROVATA su Windows finche' l'utente non la compila e prova con controller. Nessuna modifica a formato CAR, altri mondi, fixture, trasferimenti, TRIAL/RES, partita OG o asset.


## 18/11/2026 — 0.13.84: scambio X bidirezionale TRAINING squadra A/B (Windows NON PROVATO)
- Nella schermata formazione TRAINING, X sul giocatore A o B memorizza la sorgente e apre l'altro lato; secondo X sul giocatore dell'altro lato effettua lo scambio. A/SELECT continua a scambiare giocatori solo all'interno della squadra corrente; X Info nel normale VIEW SQUAD resta invariato.
- Scoperta: TRAINING B (`_currentTeam`) e' una COPIA provvisoria da slot >=13 della rosa OG A (`_careerTeam`), non una squadra distinta. `training_match.c` associa ogni B file index allo slot OG di provenienza; `play_match_menu.c` scambia soltanto gli indici reali della formazione tramite `PlayMatchMenuSwapPlayers(&_careerTeam,...)` e ricostruisce B. Niente duplicazioni di PlayerFile e nessun nuovo algoritmo di partite.
- X tastiera e Switch Pro viene intercettato solo nel menu prepartita TRAINING; evento accodato in `input_sdl.c`, eseguito sul thread del menu in `menu_system.c`. Click su righe non giocatore, slot non valido o stesso giocatore non scambia nessuno. I comandi A e popup degli altri menu non cambiano.
- Verifiche statiche sui percorsi A->B e B->A e modello a 27 giocatori senza perdita/duplicazione. Build completa C, gameplay e pad Windows, persistenza save/CONTINUE e rose EXTRA restano DA PROVARE. Non dichiarare PASS di gameplay da tali verifiche.


## 17/11/2026 — 0.13.83: fix TITLE 96/26 dopo BACK da nuova carriera (Windows PENDENTE)
- Diagnostica 0.13.81: il TITLE riceve il click ma mostra `REFUSED active-game-type=4 checkpoint=NOT_VERIFIED` dopo autosave 2627 fallito; il JSON EXTRA che lo causava e' stato corretto nella 0.13.82, senza prova Windows successiva.
- Secondo bug verificato nel codice: uscire da START UNEMPLOYED o da CHOOSE TEAM con BACK terminava il calendario civile, ma lasciava `_gameType=4`. Al successivo `CommonMenuExit`, il sistema provava a salvare una carriera annullata senza orologio, bloccando il selettore.
- `career_init.c`: `Expanded_CareerDiscardAbortedStart` pulisce i dati OG/civili SOLO in RAM, azzera i flag del nuovo setup e richiama `InitCareerVariables(0)`; usato nei due BACK e nei fallimenti di import/START prima del contratto. Non cancella o sovrascrive i CAR. Non va chiamato in carriera attiva o su salvataggio fallito.
- `menus.c`: log `[autosave-title] status=SKIPPED` con motivi se il salvataggio e' inibito dagli altri guard. La verifica di sicurezza delle carriere attive e il vincolo di non scrivere CAR al click 96/26 restano invariati.
- 0.13.83: audit statico dei quattro call site e cambi di versione/documentazione. Build Windows, input mouse/pad, save/CONTINUE, mondo 96/26, gameplay NON PROVATI. In caso di blocco dopo carriera realmente avviata servono nuova diagnostica e CAR per individuare il motivo senza perdere progressi.


## 17/11/2026 — 0.13.82: fix serializzazione CAR con rose EXTRA 2026/27 (Windows DA TESTARE)
- Due diagnostiche Windows della 0.13.81 hanno osservato Gela 11 EXTRA/27 giocatori, ma `first-contract` e `menu-exit` 2026/27 rifiutati con `INCOMPLETE_CAR reason=MISSING_CIVIL_CLOCK`, proteggendo AUTO2627.CAR precedente.
- Causa identificata NEL CODICE 0.13.81: la stringa di formato appena introdotta per `squad_extras` usava due backslash nel sorgente C (`\\\\n`), producendo nel JSON i caratteri letterali backslash+n tra campi invece del separatore newline valido; `mjson_find("$.living_world")` non poteva parsare il JSON malformato. Sostituita con la sequenza C `\\n` che emette un vero newline. Restano attivi i preflight e il salvataggio atomico, senza toccare i CAR esistenti.
- Tolto il warning `-Wcomment` dal percorso di esempio `squad_extras/<COUNTRY>.json` nell'header (non tocca logica). Versione codice 0.13.82.
- Verifica isolata di serializzazione del frammento sorgente: JSON precedente RIFIUTATO dal parser, frammento corretto ACCETTATO. Non e' una prova di scrittura/CONTINUE Windows, CAR roundtrip, mercato ne' cambi stagione.
- Lo `clock-audit ALIGN_PRE phase=0 OG=Jan-01` di luglio deriva da `native_cutoff_at`: prima del primo agosto la fase OG locale e' un SENTINEL 0, NON una seconda data residente; `ALIGN_APPLIED` deve comunque essere verificato con gameplay e fixture. Non cambiare i calendari per azzerare il delta diagnostico.
- Non corretti e ancora DA INVESTIGARE: 26 competizioni senza residenza OG 2026/27, regola detentore Libertadores 1996, 5 `cup-conflict WINNER`, tracciamento delle eliminatorie e 1996/97/2026/27 roundtrip/stagioni. Non introdurre gare/risultati o qualificazioni inventati.


## 17/11/2026 — 0.13.81 CANDIDATA: rose reali 26/27, integrazione parziale (Windows NON PROVATO)
- La Mod 4.1.2/OpenSWOS forniscono fino a 14 extra reali oltre ai 16 del file TEAM: `DataJson_LoadTeamExtras` legge SOLO il 2026/27 e valida UUID+checksum dei 16 base, usando gli overlay additivi `data_json/worlds/2627/squad_extras/*.json`. Il 1996/97 rimane OG.
- `squad_extras.c` (port da OpenSWOS/MIT, attribuzione Davide Lorigliola) carica gli extra della squadra allenata, sopprime i 10 inventati dove ci sono extra reali, li paga come riserve al 66%, include i posti 17..30 nel mercato nazionale e registra le partenze per non ricreare calciatori già ceduti. Storia delle partenze nel CAR tramite oggetto opzionale `squad_extras`; vecchi salvataggi senza il campo sono letti come storia vuota senza migrazioni distruttive.
- Limiti: la pagina 2 di BUY OTHER PLAYERS/VIEW SQUAD di club esterni, convocazioni B-team/CPU REPL e gli eventuali trasferimenti autonomi OG dei club AI non sono integrati. Il mercato può selezionare extra nelle liste nazionali, ma **nessuna compilazione Windows o prova end-to-end ancora**.
- Il pacchetto dei 101 overlay reali preparato dai file ufficiali dell'utente NON è stato caricato nel ramo remoto tramite il connettore GitHub (non supporta il passaggio dei bytes ZIP locali). Si distribuisce separatamente come `SWOS_Expanded_Rose_Reali_2627_DATI.zip`; va importato nella root con `diagnostica/INSTALLA_ROSE_2627.bat`, poi registrato in GitKraken. Finché manca, caricamento 26 mantiene fallback OG a 16+10. Nessuna sovrascrittura dei file `teams/`.
- Test statici previsti: assenza overlay -> fallback OG; overlay con UUID/checksum validi -> 16+n reali; mismatch -> non caricare; vendita/cambio club/reload CAR -> no doppioni; 96 invariato. Risultato Windows: **NON PROVATO**.


## 0.13.80 — selettore mondo debug TITLE dopo ritorno dalla carriera
- Corretto il guard logico che rifiutava il click 96/26 al TITLE quando `_gameType==4`. Il titolo puo' ora cambiare banco OG senza riavviare, ma solo dopo autosave `menu-exit` scritto e verificato per il mondo corrente. Scollega la carriera soltanto dalla RAM; conserva entrambe le copie AUTO*.CAR e non le scrive al click. Nei casi di save fallito o CLASSIC attivo resta la protezione. Versione 0.13.80. Test C isolato PASS; integrazione/compilazione Windows NON PROVATE.


## 0.13.79 — ripristino CALENDAR club e test rapido al 30 giugno da disoccupato
- Rimossa l'aggiunta di DEBUG +1 MONTH al calendario con squadra, ripristinando la geometria originale senza spostare TRANSFERS.
- UNEMPLOYED ha il nuovo pulsante reale DEBUG 30 JUNE in posizione nativa libera, con selezione mouse/controller, accanto al DEBUG +1 MONTH preesistente. Passa per lo stesso tick OG e cursore civile di FORWARD TIME, senza cambio stagione automatico; da 30 giugno nessun salto ulteriore. Versione codice 0.13.79; compilazione/gameplay Windows PENDENTI.


## Audit documentale 17/11/2026 su base 0.13.78 — matrice cross-versione
- Catalogati e confrontati senza modificare gameplay 494 record di riferimento (264 Mod 26 + 230 altre righe epoca/storiche/nazionali/supercoppe) con numeri/format/cicli, promo-retro, menu CAREER vs PRESET distinti e marcatura NV per le fonti non estraibili. Output `docs/SWOS_CROSS_VERSION_COMPETITION_AUDIT_20261117.md` + `docs/reference/SWOS_CROSS_VERSION_COMPETITIONS_20261117.tsv`.
- Prova diretta upstream OpenSWOS 96: cinque JSON di competizioni identici; `tournaments.json` differisce solo nelle fasi di Libertadores (112..136 upstream, 16..40 Expanded). Confronto 26 record Mod: 134 divisioni e paesi allineati, 21 differenze numeriche promo/retro da valutare con playoff/split. Completata descrizione 13 statali, Coppa Serie D e tutte 15 continentali della Mod, ma la loro residenza e il gameplay sono altra cosa.
- SWOS2020 e altre versioni senza dump indipendente: NV, non falsa assenza; non confondere altre epoche come Europe 1984/85 o World Cup 1934 con i mondi 96 e 26. Documento e CSV/TSV soltanto; **no build_info, no CAR, no codice**. Compilazione e runtime Windows non coinvolti; audit NON certifica formato completo della Mod storica né un singolo percorso CAREER/PRESET non osservato.


## 0.13.78 — debug +1 mese CAREER diretto su motore OG
- Eliminato il vecchio uso di `Expanded_MidseasonJoin_Tick` in modalità `s_debug_career_month`: il pulsante non preme piu' automaticamente VIEW RESULT/NEXT MATCH e non modifica i controlli squadra per nascondere le partite. L'avanzamento di tutte le competizioni residenti e' affidato a `Expanded_OGWorld_AdvanceToDate`, una volta, seguito da rebind della lega locale OG e riposizionamento read-only del cursore OG. Il mese e' gregoriano, 30 giugno e' il limite; la transizione al nuovo anno richiede la normale scelta FINISH SEASON.
- Guard per non sovrascrivere risultati manager divergenti dal residente, verifica chiavi natie/stride, menu refresh e autosave normale. Il vecchio catch-up *dopo ingaggio* resta identico. Nuovo test C delle date/limite; niente nuove dipendenze, asset o formato CAR. Economie, score personali del vecchio calendario e coppe del manager **non sono ancora riconciliate**: solo debug del mondo OG, NON un simulatore carriera completo. Build Windows/test integrato NON PROVATI.


## 0.13.77 — ultimo mondo ricordato e selector 96/26 sul TITLE
- `WorldSwitch_SelectBeforeCareer` aggiorna immediatamente `SWOS.ini [general] world` con l'infrastruttura settings gia' presente e controlla la rilettura: scelta ripristinata al prossimo avvio senza schermate o altri file. Lo stesso comportamento vale al tasto debug di NEW CAREER e sul nuovo del TITLE; no modifiche ai CAR o schema save.
- Il TITLE mostra il pannello 96/26 (187x64, x1440 y365) vicino ai debug, con mouse/joypad/tastiera e focus; tutti i PNG titolo rimangono identici. Cambio solo quando `_gameType==0` (nessuna partita/CLASSIC/career caricata), per sicurezza OG. Verifica unit C del layout PASS; boot/scrittura SWOS.ini, grafica SDL e input Windows NON PROVATI.


## 0.13.76 — impedire perdita dei dati OG continentali al cambio stagione
- `Expanded_OGWorld_ResetSeason` non distrugge piu' i contesti continentali gia' giocati (tre UEFA type-2 moderne; Champions e Libertadores classiche; due coppe UEFA type-1 paese 254), conservando annualita', tabelloni, rose, RNG e match. Solo la nuova carriera ha il reset integrale. Coppe nazionali e leghe continuano la normale inizializzazione OG nuova stagione; la lista delle coppe domestiche vecchie e' eliminata per capienza, NON archiviata.
- `Expanded_OGWorld_AdvanceUefa` non sovrascrive piu' `uefa_year` con una annualita' senza qualificazione: quando la nuova edizione non puo' essere creata, la vecchia resta consultabile come ULTIMA EDIZIONE, con log esplicito, senza falsi nuovi incontri. Il CAR gia' supporta queste strutture, nessun cambiamento schema o conversione di file precedenti.
- Header/test decisionale `expanded_og_season_retention.h`, GCC C isolato PASS; build Windows, generazione nuove edizioni, multi-stagione, NAV, salvataggio/ripresa **NON PROVATI**. Questo e' un fix di conservazione, NON la completazione del calendario pluriennale.


## 0.13.75 — Libertadores 96/97: avanzamento OG datato a FORWARD TIME
- Problema: il torneo nativo possiede begin/end fasi 16..40 (MAR-GIU) ma la carriera parte a luglio 1996 (fase 48), causando anticipo degli incontri e nessun avanzamento successivo nel periodo visualizzato. Solo `s_expOGUefa[1]` traduce il cutoff mondiale in fase di torneo per il marzo-giugno successivo, senza toccare data OG, algoritmo, fixture, squadre o RNG.
- Audit diff: logs `[og-libertadores-96]` di delta match/stadi per fase. Il world `lastPhase` resta assoluto; le altre UEFA e coppe nazionali non cambiano. Test C isolato di date/phase PASS, build e gameplay Windows non eseguiti. La gestione del manager direttamente iscritto e le edizioni dopo la prima non sono completate da questo fix.


## 0.13.74 — Champions 1996/97 visualizzata da manager assunto via OG residente
- In `Career_ViewOrEditCompetitionSlot` eliminato il requisito di essere iscritti personalmente alla Champions per consultarla: record OG e relativa copia di carriera vengono entrambi inviati al viewer OG read-only `Expanded_OGWorld_ViewLeague(_euroCup,0)`, come nel DEBUG WORLD del disoccupato. Il menu VIEW WORLD occupato richiama gia' il dispatcher. Log della scelta e dell'esito, nessun nuovo simulatore/fixture/slot.
- Test C isolato della condizione di routing PASS; compilazione Windows, visibilita' del menu, calendario e LOAD restano NON PROVATI. Nessuna dichiarazione di risoluzione delle gare del manager.


## 17/11/2026 — 0.13.73: F12 OG per competizione e confronto prima/dopo contratto (Windows NON PROVATO)
- Il vecchio `COPPE INT` fondeva 3 UEFA con altre 12 definizioni; ora F12 mostra singolarmente Champions/Europa/Conference nel 2627 e Champions/Coppa Coppe/Coppa UEFA nel 9697. Le restanti coppe intercontinentali restano separate e NON sono dichiarate residenti solo perche' definite. La Libertadores OG 1996 compare nel gruppo distinto, non e' una quarta UEFA.
- Campionati: i 13 campionati statali brasiliani sono contati a parte, senza rendere rosso il rapporto dei soli campionati nazionali del Brasile; la sezione Sud America mostra anche la copertura delle singole divisioni. Restano i rapporti per paese delle leghe e delle coppe nazionali.
- Colonne F12 ATTUALE / PRIMA ASSUNZIONE / DOPO ASSUNZIONE: checkpoint transitori read-only attorno a primo ingaggio (immediato e dopo disoccupazione) e riassunzione; confronto per identita' OG (tipo, paese, slot, stagione), non per semplici totali. Segnala scomparsa, nuova comparsa, prontezza di consultazione persa/guadagnata, partite diminuite, fase regredita e duplicati. Il confronto successivo alla firma puo' essere rivalutato con F12 durante la stessa stagione: anomalie con ID nei log `[f12-hire-detail]`. Questi checkpoint NON entrano nei .CAR; dopo cambio stagione/nuova carriera/caricamento senza un checkpoint precedente il confronto e' N/D, mai PASS inventato.
- `VIEW` verifica solo i prerequisiti strutturali per il viewer OG. La presenza effettiva nel menu e la navigazione B/A con manager assunto DEVONO ancora essere verificate su Windows: un torneo strutturalmente residente puo' restare nascosto dalla UI. Un controllo sulle classifiche aggregate segnala incoerenze, ma NON certifica ogni singolo incontro: gare previste/fixture con data sono indicate N/D finche' non esiste un registro OG consultabile e affidabile per quella coppa. Nessuna nuova simulazione o modifica agli algoritmi OG, ai risultati o ai save.
- Test C isolato puro `test_expanded_f12_compare.c`: PASS per identita' sostituita a conteggio invariato, vista persa, regressione gare/fase e cambio stagione non confrontabile. Build Windows, menu visibili prima/dopo, progressione integrata e SAVE/CONTINUE: NON PROVATI dall'assistente.


## 0.13.72 — Verify SAVE/CAR read-only e prewrite OG
- Eliminato il verificatore distruttivo del solo CAR SDL che azzerava 129 campi in caso di rifiuto reload; controlli read-only del file post-write e del JSON ancora in RAM prima del salvataggio atomico. Non cambiato schema 5, file validi compatibili, nessuna alterazione OG. Build Windows da provare.


## 0.13.71 — guardia SAVE/CAR e compatibilita' titolo
- Blocca la scrittura CAR se non esiste un orologio civile residente coerente col mondo OG. TITLE/CONTINUE e loader condividono una validazione senza effetti collaterali per evitare il falso COMPATIBLE_ON_DISK. Non aggiunge/modifica campi, schema JSON sempre 5, save precedenti intatti; verifica su Windows pendente.


## 0.13.70_23 — recupero formazione prepartita
- In CAREER con club originale, se vi sono 11 sani e portiere, selezionare automaticamente riserve sane e nominare una panchina inferiore al massimo quando due o piu' indisponibili lo rendono necessario. Callback di scambio OG, nessun calciatore fittizio e nessuna guarigione. Recheck bottone alla rotazione del giocatore extra. Compilazione e test Windows pendenti.


## 0.13.70_22 — F12 fase e debug mese
- F12: stato in fase OG per data, conteggio GARE originali, ANOM aggregato. Non dichiara verificati singoli risultati. Debug +1 MONTH con club: corretto limite dopo luglio, sospesi autosave intermedi, latch input finale. Nessuna modifica ai giocatori.


## 0.13.70_21 — fix START CAREER / autosave mondo OG
- Rimosso specificatore orfano da `mjson_printf` in serializer resident OG (17 contro 15 parametri). Nessun cambiamento ai campi salvati. Windows e gameplay da verificare.


0.13.70_19 — Aggiunto DEBUG +1 MONTH alla carriera con club; usa il driver delle callback OG con controllo CPU temporaneo, oltre al precedente bottone UNEMPLOYED. Test Windows NON ESEGUITO.


## 0.13.70_18 — comando DEBUG +1 MONTH nel calendario disoccupato
- Pulsante visibile e raggiungibile via selezione, mouse e controller nel clone UNEMPLOYED del calendario; il percorso della normale FORWARD TIME OG viene riusato, non duplicato. Un clic calcola un mese di calendario esatto, clamp a 30 giugno prima della transizione: nessun salto automatico di stagione, nessun risultato sintetico.
- Aggiunta funzione pura testabile e test C `test_expanded_debug_month.c`; abilitazione ordinal13 nella difesa del menu. Non toccata la carriera con squadra, priva di un salto mensile che non salti i match del manager. Verifiche statiche e C isolato Linux non sostituiscono compilazione/test Windows.


## 0.13.70_17 — F12: gruppo OTHER
- COPPE INT e NAZIONALI sono ora sotto l'intestazione OTHER in caricamento e avanzamento. Nessuna modifica ai contatori, ai file .CAR, al tempo o agli algoritmi OG. Windows NON PROVATO.


## 0.13.70_16 — F12 mondo OG unico con SETTIMANA
- Codice UI read-only F12 unica, con copertura e avanzamento per 6 regioni, coppe INT e nazionali; elimina le pagine F12+C / F10 ispettore, conserva F9 screenshot e l'originale Arcade F10. Usa 5x7 sprite font interno, stesso renderer e 960x540. Non genera fixture, non converte salvataggi, non altera gameplay.
- Giorno/mese/anno, settimana stagionale 1-based calcolata dal 1 luglio e anno OG affiancati. Il +1 MESE nel calendario carriera rimane separato e NON implementato in questa build.
- Conteggi per nazioni con tutte le competizioni definite del gruppo residenti; avanzamento: nazioni con almeno una gara OG per ogni competizione, affiancando gare native osservate. INT/Nazionali per torneo, non per paese. Mock PNG con valori illustrativi; niente Windows test. Commit autorizzato dall'utente; Windows ancora da compilare e provare.


## 0.13.70_15 — mitigazione puntatori legacy durante il primo autosave + diagnosi
- Dalle sessioni Windows _14 osservato crash accesso `0xEC0` all'entrata nel club sia 96 sia 26, dopo il binding OG e prima di `InitCareer` completato. Il codice sospetto è il primo autosave, che la _14 ora prova a serializzare prima di archiviarne il vecchio file.
- Guard SDL x64 per stringhe storiche dal puntatore impossibile (NULL escluso, basse 64KiB, sentinelle); limite al numero dei `careerContests` salvati, scrittura di [] e messaggio diagnostico se copia irrisolvibile. Trace per stadi del writer abilitata solo quando `SWOS_DIAGNOSTIC_CAPTURE` è presente. Test C minimale del filtro PASS, Windows e reale sblocco crash NON PROVATI.
- Le Libertadores restano INCOMPLETE: la 1996 iniziale gioca 60 gare OG ma non prosegue al knockout, nel 1997 non viene creata nuova edizione; la 2026 è definita ma non residente, mai dichiarare OK sulla sola presenza in menu. Non alterati in questo commit partecipanti, calendari o risultati.


## 0.13.70_14 — disponibilità carriera e sblocco slot OG residenti
- Corretto l'errore Windows _13: il Title non segnala più caricabile un CAR con schema valido ma senza `og_resident`, e la nuova carriera non resta bloccata da precedenti `AUTO9697.CAR` / `AUTO2627.CAR`. Al primo salvataggio completo del nuovo mondo, file incompatibile rinominato con suffisso di archivio unico e poi nuova scrittura atomica; il legacy `AUTOSAVE.CAR` è ancora sola lettura e ogni mondo è isolato.
- Test isolato C dell'archiviazione PASS (Linux); Windows _14, COMPILA, LOAD e gameplay NON PROVATI. Non modifica calendari, dati storici, simulazione OG o stato Save di carriere già residenti.


## 0.13.70_13 — seconda correzione della compilazione GCC (career_init.c)
- Diagnostica Windows _12: `career_init.c:3179` usa `Comp_ById` senza una dichiarazione visibile; la dichiarazione preesistente in un altro blocco non aveva ambito file. Spostato l'extern a livello di file sotto SDL, evitando include incompatibili. Controllo C minimale PASS; compilazione dell'EXE, 2 mondi e gameplay NON PROVATI. Nessuna modifica a date, tornei, dati, save o motore OG.


## 0.13.70_12 — riparazione dichiarazione per GCC
- Risolto in `career_status.c` l'ordine del prototipo `Comp_ById` necessario al percorso Libertadores 1996 introdotto nella _11: dichiarazione a livello di file prima dell'uso, nessun header OG incompatibile. Controllo C minimale PASS, compilazione completa e Windows **NON PROVATI**. Nessun intervento sul gameplay, sui seed, sulle date o sui salvataggi.


## 0.13.70_11 — Libertadores 1996 OpenSWOS nel mondo residente
- Il template OG originale della Copa Libertadores a 20 club (5 gironi x4, 16 ottavi) e' compilato anche su SDL e installato soltanto nel bank 9697, senza contaminare il bank 2627. Registrazione compId162 in menu South America e copia manager; contesto OG type2 residente separato da Champions in `s_expOGUefa[1]`, con salvataggio nella struttura residente già disponibile. Viewer read-only e progressione attraverso il codice OG, senza calendario o simulazione sostitutiva.
- Verifica codice: il record originale OpenSWOS 1996 **non** contiene il bye del detentore; il bye di River Plate è caratteristico dell'edizione 1997 Mod. Date invariate per richiesta. Test Windows e avanzamenti reali dei gironi ancora NON PROVATI.


## 0.13.70_10 — Champions 1996/97, calendario dopo ingaggio
- Corretto l'enrolment della Champions tipo-2 nel mondo storico: lettura della partecipazione dai 16 club UEFA OG residenti invece di offset Amiga obsoleti. Il click sul calendario personale apre il viewer UEFA residente senza simulazioni da parte del menu. Il dispatcher del viewer ora legge il campo int16 competitionType, non il byte big-endian +1. Le altre due coppe europee mantengono le routine esistenti.
- Audit statico fonte codice effettuato. Build Windows, prova club partecipante, menu BACK, risultati post-manager e ripristino CAR: NON PROVATI. I calendari di gara OG e sincronizzazione locale-residente non sono completati da questo intervento.


## 17/11/2026 — 0.13.70_9, pulizia root (Windows NON PROVATO)
- Unici BAT di avvio root: AVVIA_DIAGNOSTICA.bat e AVVIA_SWOS.bat. COMPILA_SWOS.bat e PREPARA_SWOS.bat sono in diagnostica/; tutti i riferimenti di avvio e compilazione sono aggiornati. BUILD_SWOS.sh scrive fuori dalle sessioni in diagnostica/log/COMPILAZIONE_0140.log; main.c su Windows redirige stderr in diagnostica/log/SWOS.log anche con avvio diretto. La diagnostica cattura solo il log della sessione corrente, archivia quelli precedenti e lascia tutti i .CAR invariati. Marcatori .SWOS_*_COMPILATO e backup SWOS.ini sono spostati in diagnostica/ (i preesistenti sono migrati dal launcher soltanto senza collisioni). La copia ASSETS.ini root viene eliminata dal repo: quella effettivamente letta e' assets/ASSETS.ini.
- SDL2.dll e SWOS.ini rimangono accanto all'EXE: il loader Windows e la funzione swos_ini_path() li richiedono ancora nella posizione originale; nessun cambiamento ai salvataggi in save/. I file locali non tracciati non possono essere spostati dalla sola operazione Git: la migrazione avviene alla successiva esecuzione del launcher, quando non esiste un file di destinazione omonimo. Verifiche Windows e compilazione PENDENTI.

## 0.13.70_8 — Persistenza degli stati mondiali OG
- Aggiunto `og_resident` dentro il JSON .CAR per leghe, UEFA e coppe nazionali residenti, inclusi identificativi, stato partita/torneo, rose e RNG. Pointer rebasing e checksum per ogni contesto, controllo delle dimensioni del layout, caricamento prevalidato e nessun replay di partite; autosave per mondo sostituito atomicamente.
- Vecchi CAR senza questo segmento NON sono migrati automaticamente: non contengono la cronologia OG sufficiente, quindi sono mantenuti sul disco ma non proseguibili in modo coerente con il nuovo sistema.
- Conferme Windows e fixture storiche non ancora presenti: **NON PROVATE**.


## 0.13.70_5 — test reale salvataggio e ripristino delle entita' OG
- Aggiunti log fingerprint per leghe, coppe nazionali e UEFA residenti al SAVE/RESTORE e loro confronto cross-process nello script diagnostico. Distingue errore certo (contesti zero o hash divergente), prova insufficiente e campi osservati uguali, senza falsi PASS o simulazioni. Richiede due sessioni Windows utente dello stesso slot AUTO; l'eseguibile non è stato compilato/testato.


## 0.13.70_4 — manifest Libertadores 1997
- Verificati 21 iscritti originali (20 gruppi + detentore bye) e 6 ID remappati Mod→DB storico Expanded, manifest conservato per l'adattatore OG. Nessuna simulazione o calendario modificati; test Windows N/A sui dati, implementazione giocabile PENDENTE.


## 0.13.70_3 — correzione compilabilita' diagnostica, audit delle epoche
- Dichiarazione visibile di `_competitionsTable` per nuovo checker NO DATA e guardia sul vettore statali; non tocca calendari/rng. Documento di audit statico per epoca e collegamenti, incluso confronto delle 48 leghe con dimensioni diverse. Versione 0.13.70_3; verifica compilatore Windows e gameplay NON PROVATI.


## 0.13.70_2 — diagnostica permanente OG NO DATA
- Nuovo audit di sola lettura su definizioni vs contesti residenti per le due epoche, sia club sia nazionali da valutare: dettagli mondo, paese/slot/divisione, anno, compId, motivo. Le categorie non datate/nazionali sono marcate REVIEW invece di falso FAIL. Non cambia OG fixture, RNG, save o stato di carriera. Versione 0.13.70_2, Windows NON PROVATO.


## 0.13.70_1 — OFFERTA direttamente all'elenco competizioni del paese
- `menus.c`: riutilizzato `ChooseCompetitionMenuForCountry(country)` anche nel 1996/97. Rimangono invariati menu WORLD e competizioni continentali del club qualificato. Nessun campionato preselezionato, nessuna partita o classifica risimulata.
- `build_info.h`: versione `0.13.70_1`. Test Windows, focus, BACK e apertura 1996/2026 ancora NON PROVATI.


## 17/11/2026 — 0.13.69, interventi da collaudare
- Importati separatamente i quattro TEAM originali OpenSWOS 1996/97 per i campionati prima esclusi. I conteggi dei file sorgente corrispondono alle regole OG storiche; l'effettiva integrazione nel gioco e' da verificare su Windows dopo GitKraken.
- Nel menu solo lettura DEBUG WORLD eliminato il tasto ABORT nativo e i link; nel picker OFFERTA corretto il trattenimento di B, aperto il ramo EUROPE storico per la Champions 96. Pulsanti debug compatti aggiunti a TITLE e NEW CAREER senza generare nuovi PNG.
- **NON PROVATO**: build eseguibile, gameplay, save/restore, joypad e mouse, focus e B flicker, competizioni residenti 96 a lunga distanza. Aggiornare con diagnostica utente della build, mai confermare PASS a priori.


## 0.13.68 — correzione candidata navigazione DEBUG WORLD / OFFERTA -> VIEW SQUAD (17/11/2026)
- `sorgenti/src/menu_system.c`: passaggio circolare fra l'inizio e la fine **dell'intera lista** di squadre nel picker readonly Expanded, anziche' per singole colonne; mantenuta la gestione delle colonne per i paesi.
- Aggiunto wrap dei giocatori ordinal 28..43 nella rosa readonly dell'offerta; impedito il salto alla voce invisibile 0 quando il menu incontra il sentinel -1 nella catena di voci non selezionabili.
- `sorgenti/include/build_info.h`: versione 0.13.68.
- La **0.13.67** avvia 1996/97 e cambia mondo senza restart nel test dell'utente, ma quattro leghe OG 1996/97 sono isolate; nessuna correzione dei dati campionati in questa patch. **0.13.68 NON COMPILATA e NON TESTATA su Windows** al momento del commit.

## Stato attuale — 15/11/2026
Le prove storiche che assegnano al LWE risultati/calendari autonomi o un unico `AUTOSAVE.CAR` non sono piu' il comportamento previsto. OG e' l'unica autorita' sportiva per entrambi i mondi; i salvataggi Windows Expanded sono `save/AUTO9697.CAR` e `save/AUTO2627.CAR` (legacy `AUTOSAVE.CAR` in sola lettura). La 0.13.45_08 ha FALLITO la compilazione Windows per due campi `minSubstitutes` errati; la 0.13.46 contiene la correzione codice ma richiede nuova prova Windows.

**Development history, implemented changes and technical progress**

**Project:** SWOS Expanded  
**Original game:** Sensible World of Soccer (SWOS)  
**Technical foundation:** OpenSWOS — C / SDL2  
**Target platform:** Windows x64  
**Document updated:** 6 November 2026  
**Latest known development branch:** `dev/0.10` (previous branch retained temporarily)

---

## 1. Purpose of this document

This document records the development history of SWOS Expanded, from the first experiments to the latest documented builds. It explains what was added to original SWOS, which systems were changed, what changed in each build, which bugs were addressed and which experiments were replaced or removed.

**This is a development history, not a list of future features.** Future ideas and incomplete systems belong in `PROJECT_STATE.md`; agent instructions belong in `AGENTS.md`.

**Terminology:** **Documented** means described in a build package or historical record; **User-tested** means tested by the user on Windows; **Superseded** means replaced in a later build; **Unverified** means the current implementation or behavior has not been conclusively checked. Documented does not mean fully tested.

## 2. Main improvements over original SWOS

### 2.1 Career management

Historical Expanded builds introduced or extended:
- Managerial career beginning without a club.
- Manager creation, age, nationality, fame and currency.
- Annual salary offers and negotiations.
- Job offers while unemployed, with squad and competition previews.
- Separate Board and Fans satisfaction, dismissal and return to unemployment.
- Management Record and Hall of Fame integration.
- Removal of the fixed 20-season career limit in the Expanded design.

These systems have appeared in historical builds. Long careers, dismissal thresholds and retirement still require verification against current code.

### 2.2 Club management and finances

- Dedicated Club Business screen and five stadium levels.
- Seven upgradeable facility categories.
- Sponsor and television revenue, upgrade costs, supporter counts.
- WAGES, STAFF, TRAVEL and later ADMINISTRATION and contractual manager salary.
- Weekly and seasonal financial views, monthly and annual expense projections.
- Board/Fans indicators and facility hover panels accessible via mouse and joypad.

The approved no-debt policy is a design decision; full current implementation is unverified.

### 2.3 Squad and team performance

- Player strength displayed as up to five stars.
- Detailed statistics popup, mouse/controller hover, improved selection and swapping.
- TEAM FORM and TEAM MORALE, dynamic expressions and status colors.
- Temporary match-performance modifier based on Form/Morale.
- Indicative bonus/malus values in player information.

The experimental Expanded individual condition from 0.10.20 was superseded by team-based Form and Morale.

### 2.4 Interface and controls

- Pixel-art career and management screens, original SWOS font.
- Joypad navigation, fullscreen widescreen Expanded menus.
- Removal of original framebuffer borders and graphical residues.
- Dynamic text separated from illustrated backgrounds.
- Match-statistics and exit-confirmation overlays.
- Dedicated loading screen with real bootstrap progress.

Original SWOS match gameplay remains the project's foundation.

### 0.10.51_07 — keep development window maximized across matches (7 November 2026)

**User-reported:** a maximized decorated Windows development window shrank to SWOS's original scaled size on entering/leaving a match. `Platform_ViewChanged` was explicitly calling `SDL_SetWindowSize` on every DOS/menu logical-view change, without excluding maximized windows. The SDL path now skips physical resizing when `SDL_WINDOW_MAXIMIZED` is set, while preserving logical scaling, normal window resizing and release fullscreen. Also formalized the already-approved reusable A/B/X controller legend rule in `AGENTS.md`. **Verification:** static source review; Windows compilation and actual match transitions still require user testing.

### 0.10.51_06 — fix Windows compilation of Business integration (7 November 2026)

[USER-REPORTED BUILD FAILURE IN 0.10.51_05] GCC 16.2.0 MinGW64 rejected the new `Expanded_ClubOverviewReapplyLayout` because `_currentMenu` was referenced before its extern declaration. The 0.10.51_06 change moves the extern declaration before the first SDL use and removes the extraneous `_currentMenu` access in the non-SDL legacy fallback. No asset, navigation, gameplay, or save-format changes. Not yet retested on Windows; warnings about packed members are unrelated.

### 0.10.51_05 — Business Screen BMP integration (7 November 2026)

**Code updated, not yet Windows-tested.** The approved native BMP Business Screen and approved A/B/X legend are loaded directly without converting source art. Runtime redraw preserves actual stadium level, financial figures, date, supporters and Board/Fans faces. The facility menu keeps its established upgrade callbacks and cost logic while matching the new alphabetical two-column art layout. WEEK/YEAR use separate focusable controls; X opens WAGES/TRAVEL/MAINTENANCE popups immediately, and B closes open details before leaving Business. Mouse geometry and controller focus follow the new coordinates; the legacy art remains a fallback. Build-time preflight checks presence of both approved BMP files. Build ID increments to `0.10.51_05`.

**Verification:** source correspondence reviewed and native BMP loader tested locally against both approved files; Windows build, SDL2 frame presentation, gamepad/mouse interaction and save/load not tested in this session.

## 3. Documented build history

The table includes every version identifiable in recovered project sources. Missing information is marked rather than invented.

| Version | Main documented changes |
|---|---|
| Initial experiments | Investigation of Amiga SWOS and modern PC adaptation; later OpenSWOS adopted as foundation. |
| Early OpenSWOS builds | Windows/SDL2 integration, compilation, startup and runtime troubleshooting. |
| 0.1–0.7 | Initial startup and integration experiments; insufficient reliable per-version detail. |
| 0.8 | Early career/unemployment and Club Business features, according to recovered summaries. |
| 0.9 | Manager creation, nationality, unemployment and offer-flow improvements, according to summaries. |
| 0.10–0.10.1 | Career and offer stability work; detailed changelog incomplete. |
| 0.10.2 | Menu/stability and nationality-selection corrections. |
| **0.10.3** | Reused calendar for unemployed manager; Forward Time, Wait for a Club Offer, Back, date and Business fixes. |
| **0.10.4** | Pixel-art Club Business, five stadium appearances and seven selectable facilities. |
| **0.10.5** | Four-menu pixel-art redesign; details in historical package notes. |
| **0.10.6** | Title, New Career and Calendar redesign retaining original callbacks. |
| **0.10.7** | Compact menus, original SWOS font, alignment, career fields and partial redraw. |
| **0.10.8** | Name/surname input fixes, conditional title buttons, calendar and unemployment UI. |
| **0.10.9** | Dynamic Board/Fans percentages and faces, supporter count, week number, calendar clipping. |
| **0.10.10** | Calendar row rendering without overlaps, abbreviated divisions and scroll arrows. |
| **0.10.11** | Alignment refinements; Business heading COSTS, action still UPGRADE. |
| **0.10.12** | Prevented Enter from reopening name/surname editor; dynamic calendar height and name truncation. |
| **0.10.13** | Up to 14 calendar matches, improved scroll controls and panel sizing. |
| **0.10.14** | Redesigned Club Offer, salary negotiation, View Squad and View Competitions. |
| **0.10.15** | Approved offer illustration, annual salary presentation and button hitboxes. |
| **0.10.16** | Welcome/Sacked, Board/Fans satisfaction, dismissal and persistence. |
| **0.10.17** | Management Record, country/region/salary, temporary forced-dismissal test. |
| **0.10.18** | View Squad redesign, player stars, statistics popup, controller hover, offer/sacked corrections. |
| **0.10.19** | Squad swapping and goals navigation, facilities, sponsor/TV and management effects. |
| **0.10.20** | Experimental individual Expanded condition and fitness/training recovery; later superseded. |
| **0.10.21** | WAGES/STAFF/TRAVEL accounting, week numbering, projections and two-column squad stats. |
| **0.10.22** | SEAS → YEAR, Business joypad navigation, STAFF hover and Squad access. |
| **0.10.23** | Smaller WEEK/YEAR button, STAFF popup, TRAVEL label, calendar arrows and fullscreen stretch. |
| **0.10.24** | Original SWOS font on WEEK/YEAR; removed map/shield residual borders using 320×256 menu viewport. |
| **0.10.25** | Stadium loading screen and real progress phases; Team Form/Morale replacing individual condition; temporary performance modifiers. |
| **0.10.26** | Business/Squad corrections incorporated into subsequent build; independent details incomplete. |
| **0.10.27** | Match-stat label changes, joypad adjustments, ADMINISTRATION and manager salary, Squad layout and facial colors. |
| **0.10.28** | STAFF popup columns, Board/Fans/Form/Morale faces, Squad alignment and stadium background. |
| **0.10.29** | No reliable version-specific changelog recovered. |
| **0.10.30** | Loading graphic, STAFF popup and Squad alignment; indicative Form/Morale bonus/malus. |
| **0.10.31** | Separate approved Squad/Loading image and UI layers; experimental START pause dialog. |
| **0.10.32** | Separate loading background/logo/caption; Business/Squad refinements; START unresolved. |
| **0.10.32b** | START restored to original pause action; experimental dialog removed. |
| **0.10.33** | START exit-confirmation popup, BACK/EXIT, STAFF and Squad tooltip refinements. |
| **0.10.34** | START/ESC and joypad fixes, Form/Morale modifier consistency. |
| **0.10.35** | Fixed off-screen exit-popup coordinates; SELECT match statistics. |
| **0.10.36** | Approved MATCH STATS and EXIT MATCH pixel-art overlays, live values over frozen pitch. |
| **0.10.37** | Duplicate-stat removal, border refinements, temporary exit selection arrow. |
| **0.10.38** | MATCH STATS cleanup, yellow EXIT MATCH selection border instead of arrow. |
| **0.10.39** | No independently verified version-specific changelog recovered. |
| **0.10.40** | Known development branch; exact source changes from preceding builds not checked for this document. |
| GitHub repository setup | Version-controlled source/asset archive and more durable project history. |
| Windows build workflow | MSYS2 compilation and BAT launcher/packaging scripts. |
| Later `main` commits | Local JSON menu settings, Windows menu layouts and executable packaging documented; do not assume branch parity. |

## 4. Important technical milestones

### Original SWOS integration

Development moved from early experiments to changes integrated with OpenSWOS. The objective remains to extend career management while retaining the original match engine.

### Management persistence

Historical builds introduced JSON persistence for salary, Board/Fans, sponsorship and Team Form/Morale. Current save compatibility and restoration of every Expanded field remain unverified.

### Financial accounting

Versions 0.10.21–0.10.28 refined player wages, staff/facility expenses, actual away travel, administration, manager salary and WEEK/YEAR presentation. The key requirement was to avoid double charging and distinguish forecasts from realized costs.

### Team Form and Morale

The 0.10.25 system replaced Expanded individual condition with team values. The documented performance modifier uses average M=(Form+Morale)/2: neutral at 65, maximum −20% at low values and maximum +20% at 100. Later faces use five expressions and three color bands: **0–39 red, 40–59 yellow, 60–100 green**.

### Widescreen/4K rendering

Versions 0.10.23–0.10.24 rendered the 320×256 Expanded menu region across widescreen displays instead of exposing residual pixels from the original 336×272 framebuffer. Match rendering remains separate.

### Pause, statistics and exit

Versions 0.10.27–0.10.38 iterated START/SELECT/ESC and the MATCH STATS/EXIT MATCH overlays. These are successive experiments, not simultaneously active alternatives. Latest documented EXIT MATCH selection uses a yellow border.

## 5. Approved visual implementations

| Screen | Historical integration | Main change |
|---|---|---|
| Title | 0.10.5–0.10.9 | Original logo, Expanded identity, compact menu, Hall of Fame |
| New Career | 0.10.6–0.10.12 | Pixel-art background, editable fields, SWOS font |
| Calendar | 0.10.3–0.10.13 | Match list, scrolling, date and unemployed variant |
| Club Business | 0.10.4 onward | Stadium, facilities, finances, satisfaction indicators |
| Club Offer | 0.10.14–0.10.15 | Contract illustration, salary, squad/competition previews |
| Welcome / Sacked | 0.10.16 | Dynamic management information and transitions |
| Management Record | 0.10.17 | Career history and annual salary |
| View Squad | 0.10.18 onward | Stars, statistics, Form/Morale and controller focus |
| Loading Screen | 0.10.25 onward | Approved stadium artwork, separate overlays, real progress |
| Match Stats | 0.10.36–0.10.38 | Pixel-art overlay with dynamic values |
| Exit Match | 0.10.33–0.10.38 | Confirmation flow and graphical panel |

**TRANSFER 1** has an approved visual reference; the available evidence does not establish a completed new worldwide transfer system.

## 6. Superseded experiments

| Earlier implementation | Replacement or later decision |
|---|---|
| Standalone desktop management UI experiments | Direct OpenSWOS integration |
| Expanded individual player condition (0–100) | Team Form and Team Morale |
| Forced dismissal test | Normal dismissal rules; final thresholds to verify |
| SEAS label | YEAR |
| AWAY financial label | TRAVEL |
| BUS STAFF terminology | Revised travel/staff terminology |
| OTHER general expenses | ADMINISTRATION |
| START mapped directly to ESC | Dedicated exit confirmation |
| Experimental START pause dialogs | Revised popup and controller handling |
| EXIT MATCH selection arrow | Yellow selection border |
| Loading holds of 5 or 20 seconds for screenshots | Temporary test behavior, not permanent requirement |

Exact removal of temporary behavior must be checked in current code before being called complete.

## 7. Known limitations and unverified features

| Feature | Evidence |
|---|---|
| Persistent worldwide league simulation while unemployed | Historically documented as incomplete |
| Joining a club in an already simulated season | Historically documented as incomplete |
| Complete worldwide league database | Long-term objective |
| Worldwide transfers | Design and partial interface work; full implementation unverified |
| Scouting with hidden `???` statistics | Rules still undefined |
| POTENTIAL-based player development | Design decision; implementation unverified |
| Fully redesigned Training | Not yet defined in detail |
| No-debt AI club finances | Approved rule; implementation unverified |
| Final dismissal thresholds | Multiple historical revisions |
| Retirement at 85 | Approved design; full flow unverified |
| Full latest Windows/joypad regression test | Not established by historical checks |

## 8. Testing and reliability history

User feedback across the project confirms historical successful menu startup, entry into career and partial match gameplay. Reported issues addressed in subsequent development packages include missing SDL2 runtime dependencies, crash on accepting offers, Back lock in unemployment, nationality flicker, calendar clipping, Squad focus/Exit, fullscreen borders and an exit popup rendered outside the screen.

Historical packages also describe GCC syntax checks, AddressSanitizer/UndefinedBehaviorSanitizer and asset/package checks. Such checks do not replace interactive Windows/joypad testing.

Future release verification should cover startup, friendly match, career creation, unemployment, offers, club acceptance, calendar, Business, Squad, loading, match stats, match exit, save and load.

## 9. Repository history and continuity

GitHub is the source/asset archive for code, data, approved art, Windows build scripts, commits and project documentation. `main` and `release/0.10.40` have different histories; changes on one branch should not be automatically attributed to the other.

- `AGENTS.md`: permanent working rules.
- `PROJECT_STATE.md`: approved decisions, current status, plans and open questions.
- `PROJECT_HISTORY.md`: historical build/package notes.
- `IMPROVEMENTS.md`: development history and changes actually introduced.

## Current workflow (6 November 2026)

Commits and builds are separate. Only explicit user requests authorize commits. Intermediate code revisions use `0.10.51_01`, `0.10.51_02` etc.; the next explicitly requested build is `0.10.52`. The title label displays `Build <version>`. This is a workflow and source change, not a verified Windows test.

## 10. Updating this document

For every new build, record:

- **Version and date**
- **Git commit**
- **Features added**
- **Existing behavior modified**
- **Bugs fixed**
- **Experimental behavior removed**
- **Tests performed**
- **Known limitations**

Do not describe mockups as implemented or syntax checks as successful Windows gameplay tests. Preserve superseded decisions and identify their replacement versions.

## 11. Sources and reconstruction notes

Compiled from recoverable SWOS Expanded project conversations, historical package README and test reports in `PROJECT_HISTORY.md`, `PROJECT_STATE.md` and predecessor notes, available GitHub commit history, and user-reported tests.

Early builds, some intermediate versions and branch-specific changes cannot be reconstructed conclusively. This is a detailed historical reconstruction, not a claim of complete line-by-line verification against every historical source archive.


## 0.10.52 — Title Menu PNG 16:9 (2026-11-08)
- Aggiunto decoder frontend per PNG RGB/RGBA 8 bit lossless con miniz esistente, senza SDL2_image o nuove DLL; BMP/RAW approvati intatti.
- Aggiunto renderer sfondo Title Menu RGB 1664×936 separato dal vecchio framebuffer, con mascheratura dell'immagine RAW e sovrapposizione dei pulsanti, testo dinamico e focus originali.
- F8 sul Title Menu: confronto immediato PNG/originale; fallback se PNG mancante, malformato o di dimensioni diverse.
- Build number `SWOS_EXPANDED_VERSION` a `0.10.52`; aggiornate regole PNG/risoluzione e relativa documentazione nel commit di sviluppo.
- Test: decoder RGB 1664×936 lossless rispetto a Pillow e mock SDL per alpha-mask/F8; **test su Windows/joypad e disponibilità dell'asset nel repository da verificare**, non dichiarati superati.


## 0.10.53 — Logo canonico del Title Menu (2026-11-08)
- Aggiunto il caricamento del logo PNG RGBA separato `assets/title/swos_expanded_logo_approved_2026-11-08.png` (1200×440, senza ®), condivisibile in futuro con altre schermate senza duplicare l'asset.
- Disegno in posizione C approvata dello sfondo 1664×936: x920 y17 620×227, scala relativa all'output e indipendente dai pulsanti; sfondo precedente e modalità F8 intatti.
- Fallback: il nuovo background rimane utilizzabile senza il logo; F8 mostra il titolo legacy originale per confronto.
- Incrementata `SWOS_EXPANDED_VERSION` a `0.10.53`.
- [TEST LOCALE] Decoder PNG RGBA con confronto pixel-per-pixel contro Pillow e harness C/SDL stub per ordine di rendering e coordinate, F8/mask; [DA TESTARE] compilazione Windows e interazione reale. Disponibilità dei binari PNG su GitHub da verificare separatamente.


## 0.10.53_01 — Title Menu moderno a due livelli (2026-11-08)
- [CODICE, WINDOWS NON TESTATO] Overlay titolo SDL2 non usa più lo sfondo RAW né i pulsanti SWOS nella presentazione. Rimossi toggle F8 e mascheratura storica. Sfondo RGB e logo trasparente approvati rimangono separati.
- [CODICE, WINDOWS NON TESTATO] Menu di primo livello a cinque pulsanti, sprite PNG RGBA derivati dalla grafica blu scelta dall'utente, pressione animata via SDL2; sottomenu classico provvisorio senza sfondo illustrato e callback originali mantenute.
- [CONSEGNA ESTERNA] I cinque PNG sono forniti in uno ZIP da estrarre nella cartella del gioco. Non considerarli committati in GitHub finché non sono realmente pubblicati dall'utente.


## 0.10.53_02 — Correzioni layout/navigazione/focus Title Menu (2026-11-08)
- [UTENTE — TEST WINDOWS] Segnalati Title Menu troppo a sinistra, focus giallo rettangolare e direzione GIÙ invertita anche nel sottomenu.
- [CODICE, DA RITESTARE SU WINDOWS] Pulsanti traslati sul lato destro e centrati sotto il logo approvato; maschera di focus giallo generata su tutti i pixel alpha della silhouette PNG completa, inclusi bordi smussati e spessore, senza toccare i file dei pulsanti.
- [CODICE, DA RITESTARE SU WINDOWS] Mappatura joystick/tastiera corretta (0 su, 4 giù), valida in entrambi i livelli; pressione più sobria di 5 tick senza deformare il PNG. Nessuna variazione di gameplay, salvataggi, sfondo o logo.


## 0.10.53_03 — Title più compatto e pulizia asset obsoleti (2026-11-08)
- [CODICE — TEST WINDOWS PENDENTE] Dimensioni dei cinque pulsanti ridotte circa del 10%, con centro orizzontale invariato (x=1230) e 64 px di spazio sotto il logo approvato; spostate le hitbox in accordo con le texture SDL2, senza toccare le PNG.
- [CODICE — TEST WINDOWS PENDENTE] Maschera esterna del focus giallo generata a raggio 5 px anziché 4 nei pixel sorgenti, continuando a seguire la sagoma alpha completa e la pressione.
- [PULIZIA REPOSITORY] Eliminati solo 11 file temporanei documentati/test/anteprima e vecchi manifest già consumati; il RAW del Title storico non è stato rimosso perché ancora richiesto da `expanded_economy.c` e `BUILD_SWOS.sh`. Conservati gli asset approvati e la procedura di trasferimento alternativa. `.gitignore` tutela contro il reinserimento di copie residue. Nessun test Windows effettuato dall'assistente.


## 0.10.53_04 — CONTINUE CAREER indipendente e menu ulteriormente compatto (2026-11-08)
- [ASSET APPROVATO / ZIP] Nuovo PNG RGBA `assets/title/buttons/continue_career.png` senza contorno giallo, consegnato via ZIP; deve essere pubblicato successivamente su GitHub dall'utente.
- [CODICE, WINDOWS DA TESTARE] CAREER diventa CONTINUE CAREER quando è attiva una carriera, con sprite specifico e senza sovrascritta runtime. Mantiene selezione, animazione e callback; rispetta le proporzioni dell'asset (contain).
- [CODICE, WINDOWS DA TESTARE] Altra riduzione ~10% di tutti e cinque i pulsanti, maggior respiro fra logo e pulsanti (106px) e spazi inferiori più compatti. Focus dinamico giallo da 5 a 6 pixel sorgente nei cinque sprite precedenti (12 pixel nel CONTINUE ad alta risoluzione per uno spessore a schermo uniforme), mai baked-in.
- [REGOLA] Un asset base UI non comprende la selezione: la sagoma del focus è un layer del renderer. Nessuna alterazione delle immagini statiche dei pulsanti precedentemente approvati.

## 0.10.54 — Title fullscreen/windowed con icone dinamiche (8 novembre 2026)
- [CODICE, DA TESTARE WINDOWS] Nuovo controllo video in alto a sinistra, due sprite PNG approvati senza testo (mostra solo l'azione possibile), controllo tastiera/joypad/mouse e ritorno dal fullscreen massimizzato nel profilo development.
- [CODICE, DA TESTARE WINDOWS] I cinque pulsanti principali sono ridotti di circa 10% rispetto a 0.10.53_04 e spostati verso il basso; background/logo/caption approvati invariati. Contorno di focus dinamico separato e generato in modo lineare rispetto ai pixel.
- [ASSET APPROVATO, ZIP] Entrambe le icone e CONTINUE CAREER in un unico ZIP strutturato da estrarre accanto a `AVVIA_SWOS.bat`; verifica del push GitHub separata.

## 0.10.54_01 — Stile canonico pulsanti e spaziatura Title (2026-11-08)
- [REGOLA UTENTE, DOCUMENTATA] Nuovi pulsanti pixel art blu frontali, con spessore scuro a destra/sotto e lieve bouncing sulla pressione. PNG base senza selezione gialla incorporata; focus dinamico dalla silhouette completa.
- [CODICE — TEST WINDOWS PENDENTE] Icona fullscreen/windowed trasferita da (58,56) a (34,34), mantenendo 92×88 logici. Hitbox e posizione della texture restano accoppiate.
- [CODICE — TEST WINDOWS PENDENTE] Spazi verticali dei cinque pulsanti principali ridotti, tenendo fisso il fondo di EXIT GAME (y871). Focus del Title ispessito (+2px nei PNG principali, aumento proporzionale per asset più densi). Bounce leggero di 1px sul rilascio, senza scaling del PNG.

## Correzione della scritta WORLD OF nel logo Title (2026-11-08)
- [ASSET APPROVATO] Corretto WORLO OF → WORLD OF, con intervento limitato alla forma dell'ultima lettera. Lo sprite approvato resta 1200×440 RGBA, logo e sfondo separati.
- [GITHUB] Sostituito il blob del PNG sotto il nome canonico esistente, conservando gli altri pixel del repository invariati; nessun ridisegno, cambio di posizione o revisione di gameplay, menu, callback, script o build.
- [WINDOWS DA VERIFICARE] GitKraken pull/sovrascrittura del vecchio PNG locale, render corretto del titolo in gioco.

### 0.10.56 — NEW CAREER corrections (2026-11-09)
[Code edited; Windows test pending] Upright integer-pixel bitmap text instead of fractionally scaled traced font atlas; checkbox inside unemployment toggle; direct FAME cycling without picker; arrow artwork not independently clickable. UI retains approved 88.5% scale and original background. No career or save-format changes. GitHub/GitKraken workflow restored.

### 0.10.57 — NEW CAREER visual fidelity fix (2026-11-09)
[CODE COMMITTED; WINDOWS TEST PENDING] Replaced stretched 5x8 font with offline-generated bold upright PNG atlases loaded at runtime with JSON glyph metrics; disabled age text popup on confirm and clamped age to 18-70; rendered legacy editing cursor 0xff as vertical caret rather than '?'; used narrow yellow alpha-derived toggle/start focus and a single thin form-field outline; separated touching input columns visually. Background and approved base graphics retained without resize; bitmap assets supplied separately for extraction.

---

## SWOS_DOC_SYNC_2026_11_10 — Aggiornamento documentale, senza modifiche al motore

**Tipo:** registrazione della manutenzione della documentazione progettuale, non implementazione. Preparati `docs/GAME_DESIGN.md` e aggiornamenti coerenti a `AGENTS.md`, `PROJECT_STATE.md`, `docs/DEVELOPMENT_WORKFLOW.md` per le decisioni del 10 novembre 2026: tre schermate graficamente finali, grafica obsoleta degli altri menu, calendario ad eventi, WEEK quasi ovunque, NEXT EVENT, chairman, YOUTH/RES/TITOLARE, completamento prioritario con database attuale e futuro BAT di pulizia asset verificati orfani.

**Codice/assets/build:** nessuna modifica e nessun file rimosso in questa operazione. **Test Windows:** non eseguiti. **GitHub:** nessun commit né push effettuato da questo pacchetto. Gli interventi funzionali descritti nel GDD sono futuri finché non confermati dal codice e dai test.


## 0.13.01 — Prima API calendario mondiale (10 novembre 2026)

- [CODICE] Aggiunto modulo C autonomo `expanded_world_time.c/.h`: date gregoriane 1900–2200, inizializzazione con anno/mese/giorno arbitrari, conversioni, day index, settimana relativa alla stagione, avanzamento solo in avanti.
- [CODICE] `Expanded_FormatDate` non implementa più un calendario separato; passa tramite l'API comune. I menu occupato/disoccupato usano `World_FirstYear()` per il mondo attivo; settimana visualizzata nel disoccupato ricalcolata dal reale inizio stagione (1 agosto nell'interfaccia provvisoria).
- [TEST GCC LOCALE] Test isolato di date 1996/2026/2034, bisestili, validazione e avanzamento; scansione completa dei 109.938 giorni 1900-2200. Nessun test Windows/joypad eseguito.
- [LIMITI] Nessun nuovo scheduler, risultati simulati, gestione eventi, salvataggio del clock o modifiche a fisica/finanze. La versione `0.13.01` applica la nuova numerazione richiesta dall'utente; la convenzione precedente con _01 è storica.

## 0.13.02 — Debug osservabile e ledger cronologico (10/11/2026)

- [CODICE] F12 mostra/nasconde debug testuale in sovraimpressione; F9 salva un PNG della **schermata realmente presentata**, debug incluso; F11/PrintScreen restano disponibili per screenshot legacy. Nessun nuovo asset.
- [CODICE] Tasti 1/2/3 mentre F12 è aperto: forza WIN o LOSS alternativamente per i risultati ufficiali della squadra gestita, oppure arma l'esonero al successivo ingresso nel percorso partita. Il motore fisico non cambia; i toggle non sono persistiti nel JSON e sono OFF al boot. I risultati forzati possono essere differenti dai gol effettivamente giocati: funzionalità solo per test.
- [CODICE] `expanded_world_ledger.h/.c`: registro cronologico indipendente con risultati immutabili, classifiche riproducibili, ID di incontri, doppia modalità 2/3 punti per vittoria e callback di simulazione. **Ancora scollegato da OpenSWOS** e dal salvataggio reale.
- [CODICE] La data secondaria del calendario nativo usa `World_FirstYear()` per il mondo scelto, al posto dell'anno fisso 1996.
- [TEST GCC ISOLATI] `tests/run_test_expanded_phase12.sh` verifica controlli e registro; simulazione artificiale di 35 settimane/140 risultati e classifiche. Nessuna compilazione completa e nessun test Windows in questa consegna.


## 0.13.03 — Pilot risultati mondiali durante UNEMPLOYED

- [CODICE] Simulazione offscreen indipendente dal manager, basata sulle squadre realmente lette da cinque paesi già sicuri del database e dalle relative divisioni supportate; ID squadra effettivi, rating aggregato dalla rosa, calendario round-robin **provvisorio** e risultati deterministici. Usa il registro cronologico introdotto in 0.13.02, avanzato a ogni settimana della disoccupazione.
- [CODICE] La schermata VIEW LEAGUE dell'offerta mostra ora classifica provvisoria e posizione del club con partite giocate e punti, anche prima della firma. I dati non sono risultati del motore carriera nativo.
- [CODICE] Gestione turni di riposo per divisioni con club dispari; rifiuto di costruire gironi parziali oltre 26 club; supporto a due stagioni-pilota. Nessun nuovo asset e nessun intervento al pannello debug F12.
- [TEST ISOLATI] GCC -Wall -Wextra -Werror + ASan/UBSan: 20 club/350 gare dopo 35 settimane; girone dispari 5 club e settimana 10; turni e classifiche validi. Non è ancora un test completo Windows.
- [ANCORA DA FARE] Calendario regolamentare, mondo completo, coppe, record nel salvataggio, classifica sincronizzata con motore SWOS e assunzione a metà stagione senza reset. Il vecchio InitCareer conserva il proprio comportamento.


## 0.13.04 — Fix sintattico bloccante nell'adapter (10 novembre 2026)

- [CODICE] Corretto `\\n` letterale introdotto accidentalmente in `expanded_world_live_adapter.c:55`, diventato un vero a capo nella condizione di validità del puntatore alla competizione. Nessuna modifica alla logica.
- [VERSIONE] Build aggiornato a `0.13.04`. Compilazione completa Windows e prove runtime da ripetere a cura dell'utente.


## 0.13.05 — Inizio luglio e precampionato pilota (11 novembre 2026)

- [CODICE] Il calendario UNEMPLOYED e l'orologio del mondo pilota iniziano il **1° luglio**. Le date delle partite native continuano per ora con il riferimento agosto: questo commit non cambia le chiavi del calendario SWOS.
- [CODICE] Quattro date estive nel mondo pilota per ogni divisione simulata, con abbinamenti amichevoli fra squadre, turni di riposo nei gironi dispari e aggregati autonomi di risultato, morale, fatica, infortuni potenziali e saldo stimato. Valori in RAM **non persistiti, né trasferiti nel gioco originale**.
- [CODICE] Prima delle partite ufficiali, VIEW LEAGUE mostra quante amichevoli pilota ha giocato la squadra offerta. Conversione più coerente alla data civile luglio quando il manager torna disoccupato dopo un esonero.
- [DESIGN DOCUMENTATO] Aggiornato `GAME_DESIGN`: precampionato globale, amichevoli organizzabili, prezzi biglietti in CLUB con elasticità domanda/FANS LIKING, unificazione CLASSIFICA/SQUAD e carriera europea dal 1° luglio. Funzionalità di prezzo biglietti e assunzione nativa a stagione iniziata non ancora implementate.
- [TEST] Test di regressione C estesi e disponibili nel repository; compilazione ed esecuzione Windows da verificare sull'utente. F12, salvataggi e fisica partita invariati.


## 0.13.06 — Recupero nativo del calendario alla prima assunzione (11 novembre 2026)

- [CODICE SPERIMENTALE] `expanded_midseason_join.c/.h`: automatizza le azioni native NEXT MATCH / VIEW RESULT durante l'ingresso in carriera dopo settimane di disoccupazione; conversione luglio → agosto e stop alla prima giornata futura. Non modifica la fisica delle partite.
- [CODICE] Guardia per non attribuire al nuovo allenatore statistiche, morale relativo a risultati e esoneri di partite antecedenti; registrazione delle gare lasciata alla normale logica OpenSWOS. Durante l'avanzamento automatico non aprire offerte economiche o trasferimenti di BUSINESS.
- [CODICE] Callback da `MenuProc` e dai due percorsi di nuova assunzione; regole di abort con log in `SWOS.log`. La classifica nel menu CARRIERA resta quella nativa, con progresso dalle partite del suo calendario.
- [TEST] Runner isolato per calcolo cutoff incluso, ma compilazione Windows e test in-game ancora da eseguire. Non garantito confronto esatto tra classifica VIEW LEAGUE pilota e quella nativa.
- [DESIGN] Documentati: mondo selezionabile da OPTIONS (non ancora implementato), sponsor preesistenti/scadenza/rinnovo (approvato ma non implementato), forma individuale+training collettivo (solo discussione futura). Nessun nuovo asset, F12 invariato.


### 0.13.07 — filtro offerte da risultati disponibili (11 novembre 2026)
- [CODICE, NON TESTATO WINDOWS] Quando la classifica pilota di una divisione è disponibile e sono state disputate almeno 3 partite, la selezione di club per offerte di lavoro pesca solo nella metà inferiore, evitando offerte casuali da capolista. Se i dati non sono sufficienti conserva il comportamento precedente, senza simulare risultati inesistenti.
- [DOCUMENTAZIONE] Registrati nel Game Design i due incidenti ChatGPT riportati dall'utente (Connection interrupted, Unknown error), con stato e limiti del recupero.
- [NON COMPLETATO] Unificazione reale pilot/native, sistema di vacancy persistenti di allenatori, nuove metriche database F12. Non attribuire a questa revisione una soluzione al reset del calendario dopo ingaggio.
- [TEST] Revisione statica della modifica e riscontro file GitHub; non compilato, non eseguito in Windows/joypad.


### 0.13.08 — controlli integrità native midseason catch-up (11 novembre 2026)
- [CODICE] Rafforzato l'accesso al keycache 8192 byte durante la scansione di pre-ingaggio: ogni stride è validato contro limiti e ordine delle fasi, e si richiedono incontri sia prima sia dopo il momento dell'assunzione. Controllo di bounds anche sul record corrente in ogni tick.
- [NON RISOLTO] Il calendario pilota differisce ancora dal calendario OpenSWOS inizializzato alla firma; non dichiarare completa la continuità dei risultati/classifiche o il salvataggio del mondo.
- [TEST] Revisione statica; compilazione e test reali Windows non eseguiti in questo ambiente.


### 0.13.09 — cutoff di fase nativo e diagnostica mondo F12 (11/11/2026)
- [CODICE] Corretto il confronto tra settimana civile e fase nativa OpenSWOS: codifica mesi ×8 e rollover +0x60; test unitari aggiornati.
- [CODICE] F12 esteso con dati del pilota (club, leghe, slot giocatori, fixture, risultati, RAM) e situazione native cache/fase e catch-up; dati mancanti (manager/chairman globali, dimensione su disco) dichiarati non disponibili.
- [REGOLA] Commit solo per cambiamenti significativi autorizzati; controllo diagnostiche F12 in ogni build.
- [PENDENTE] Compilazione Windows e test reale dell'assunzione. Pilot/world e carriera nativa restano non unificati; non dichiarare risolto il bug principale.


### 0.13.10 — simulazione CPU delle gare pregresse e coppe (11/11/2026)
- [CODICE] Il recupero di un ingaggio durante una stagione dirige le partite del club già trascorse verso la simulazione nativa anche se il menu presenta PLAY MATCH. SetupPlayers disabilitato soltanto in catch-up e percorso coppe native gestito nel driver automatico.
- [DEBUG] Contatori F12 per azioni catch-up e simulazioni forzate; logging di menu non riconosciuti e riepilogo dei motivi di arresto.
- [VERIFICA PENDENTE] Compilazione Windows, ingressi con coppe e confronti delle classifiche. Non è stata dimostrata l'identità tra le due classifiche pilota e nativa: il calendario doppio rimane in attesa dell'unificazione.
- [ROLLBACK] Precedente commit `fab201b98ae60ab37f105ee48589c66b234a809c` (0.13.09), da ripristinare manualmente con GitKraken soltanto su decisione dell'utente.


### 0.13.11 — data/carriera sincronizzata e recupero invisibile (11 novembre 2026)
- [CODICE] Corretto `Expanded_FormatDate`: i valori di sortKey sono ottavi di mese nativi, non settimane da agosto. Il caso mostrato (28 AUG 1997 nella stagione 1996/97) nasceva da questa conversione errata.
- [CODICE] UI carriera/business usa la fase della prossima fixture (keycache) per mostrare una data aggiornata, lasciando il contatore finanziario separato. Date d'ingaggio arrotondate alla successiva fase utile per comprendere tutte le partite anteriori.
- [CODICE] Driver: NEXT MATCH prioritario su VIEW RESULT; blocco input dell'utente durante recupero; overlay temporaneo che copre le classifiche mentre vengono simulate. F12 mostra esito dell'ultima esecuzione.
- [TEST] Controllati indipendentemente 94 casi nel calcolo delle fasi 1996/2026; test unitari C aggiornati ma non compilati, build/test Windows ancora da effettuare.
- [PENDENTE] Resta la divergenza fra risultati preview/pilota e risultati nativi; nessuna fusione di calendari o serializzazione mondo implementata.


### 0.13.12 — avanzamento nativo dopo menu lega completato (11/11/2026)
- [CODICE] Aggiunta la transizione automatica `DIYLeagueTournamentExit` quando nella schermata lega non sono disponibili NEXT MATCH/MATCH/RESULT/VIEW RESULT. Il driver precedente abortiva -8 dopo la giornata per assenza di callback, lasciando data e stagione incomplete.
- [DEBUG] Contatore F12 `LEAGUE MENUS COMPLETED` e log del ritorno al calendario per distinguere progressi reali da cicli nel menu. Interrotto ancora con -8 se non e' visibile neppure EXIT.
- [LIMITI] Solo controllo statico degli handler attivi, nessuna build Windows/test runtime; la preview del Living World Engine rimane separata dalla competizione nativa. Data, formule e renderer della 0.13.11 non modificati.


### 0.13.13 — ponte controllato tra risultati pilota e handler nativo (11/11/2026)
- [CODICE] Le gare di campionato del recupero nativo provano a riutilizzare un punteggio registrato nel pilota per la stessa coppia casa/trasferta, entro un intervallo temporale verificato di circa una settimana, una sola volta; se non c'e' corrispondenza, motore OpenSWOS invariato.
- [DEBUG] F12 `NATIVE REUSED` e `FALLBACK`; la dicitura `PILOT/NATIVE REPLAY PARTIAL` conferma che esistono ancora due calendari distinti.
- [TEST] Aggiunti casi di test C per abbinamento, errore temporale, mancato riuso e identita' invertite. Esecuzione completa e build Windows non verificate nell'ambiente corrente.
- [DOCUMENTAZIONE] Registrata nuova segnalazione `Connection interrupted` dell'11 novembre in `docs/GAME_DESIGN.md`.
- [SCELTA UTENTE] Lasciata invariata la posizione del benvenuto e il momento di avvio del recupero; l'accelerazione e l'unificazione globale saranno affrontate nella fase successiva, senza nuovi reset.


### 0.13.14 — rendering disaccoppiato dal recupero, ledger matching esteso (11/11/2026)
- [CODICE] In `Platform_RenderFrame` il recupero precedente alla firma disegna uno status statico alla prima iterazione; tutte le successive iterazioni processano eventi ma non SDL upload/render/present. In `Platform_Present` bypass esclusivo del pacing normale a 50Hz durante il catch-up. Timer-render fallback escluso durante l'operazione. Nessuna modifica al pacing della partita.
- [CODICE] La ricerca pilota→nativo non richiede più che le date degli stessi club siano entro una settimana; usa un unico risultato gia' simulato per ciascuna coppia esatta casa/trasferta nella stessa stagione. Il vecchio vincolo temporale aveva dato 12 match riutilizzati su 372 nella prova utente 0.13.13.
- [DEBUG] HEADLESS frames/time nel F12 per confrontare l'attesa reale; NATIVE REUSED/FALLBACK mantengono il conteggio degli abbinamenti. 
- [TEST] Aggiornate asserzioni pure-C per stagione corrente, non riutilizzo, invertire casa/trasferta e divieto di attraversare stagioni. Test C compilazione Windows **non eseguiti nell'ambiente attuale**.
- [NON RISOLTO] Le tabelle non possono essere garantite uguali finché `ExpandedWorldLive` e OpenSWOS non utilizzano un unico calendario/ledger. Non cancellare la preview né correggere artificialmente i punti per ottenere l'identità visiva.


### 0.13.15 — OG fixture schedule / shared results in both directions (11/11/2026)
- [CODICE] Il generatore mondiale ricicla le tabelle originali `_fixtureSchedulePtrTable`, anziché creare nuovi incontri con l'algoritmo circle separato. `ExpandedWorldLive_SetLeagueSchedule` importa da ogni Competition i mesi di inizio/fine e produce le date per fasi native (otto per mese).
- [CODICE] `DIY_ShuffleLeagueTeamOrder` non effettua un secondo sorteggio solo quando trova i club esattamente presenti nel ledger della stessa lega; riproduce la medesima numerazione per il calendario OG. Altrimenti il comportamento SWOS originale è invariato.
- [CODICE] `ExpandedWorldLive_RecordNativeLeagueResult`: dopo la firma, risultati originali del manager scritti nel ledger mondiale, in caso di match preciso nella fase nativa. Gli altri incontri CPU avanzano nello stesso ledger; nessun cambiamento alla fisica partita.
- [DEBUG] `OG LEAGUE CALENDARS / ORDERS` controlla in quali divisioni il collegamento è realmente attivo. Rimangono conteggi F12 dei fallback e del recupero.
- [TEST] Tabella originale per 4,20,24 squadre controllata staticamente; harness C ora linka fixture_data originale e modulo dei round dispari. Test C e Windows non eseguiti in questa sessione.
- [LIMITI] Non ancora rimossi stato delle competizioni nativo distinto, catch-up dopo assunzione e world-save. Leghe speciali, coppe e riassunzione da testare. Versione per rollback: 0.13.14 `e2cee6d`.


### 0.13.16 — importazione diretta di campionato e provenance manual result
- [CODICE] \`DIY_WorldDirectImportLeague\` ricostruisce lo stato originale del campionato a due gironi da risultati mondiali già registrati, con progressione fixture SWOS originale e KeyCache player match marcata solo se completa e consistente. Salvataggio del buffer nativo solo dopo controllo totale; fallback al vecchio driver quando incompatibile.
- [COPPE] Le coppe non hanno ancora un ledger mondiale; vengono mantenute nel percorso OG. Per storia senza coppe precedenti e import valido si elimina il recupero dopo firma (JOIN DIRECT). Con coppe precedenti il driver lavora solo sui turni OG residui e salta la lega già importata.
- [DEBUG] F12 mostra \`DIRECT OG IMPORT\`, \`LAST OG\` con goal e punti, \`RAW FINAL\` con score match/mismatch per gli incontri manuali. Nessuna modifica ai punti assegnati dagli handler originali.
- [TEST] Test read-only \`PeekNativeLeagueResult\` aggiunti; build e test Windows PENDENTI. Fallback/rollback alla \`0.13.15_01\` confermata dall'utente.


### 0.13.17 — persistenza JSON del world ledger
- [CODICE] Snapshot Living World dentro il normale .CAR: calendari originali, club, leghe, risultati e stato della simulazione, con unica fonte di verità nel ledger.
- [ROBUSTEZZA] Lettura transazionale con controlli di range e appartenenza esclusiva dei club alle leghe; fixture elaborate in tempo lineare.
- [SAVE] Schema 4 per i nuovi file, schema 3 ancora leggibile. Un vecchio save non conserva la precedente simulazione mondiale. Nella versione Amiga il campo mondiale è null.
- [DEBUG] F12 indica world snapshot pronto oppure assente.
- [LIMITI] Nessun motore completo: coppe mondiali, cicli multistagionali, allenatori/presidenti, mercato AI, generazioni e salvataggio nativo integrale sono ancora da sviluppare.
- [TEST] Nessuna compilazione MinGW64 o esecuzione Windows svolta dall'assistente; test interattivi rinviati con il resto della convalida.


### 0.13.18 — validazione salvataggi pluriennali
- Corretto un vincolo troppo stretto nel parser .CAR schema 4: settimane/date successive al secondo anno rimangono caricabili entro il range del calendario mondiale. La produzione di nuove stagioni resta un intervento separato.
- Windows compile e test save/load ancora pendenti.


### 0.13.19 — riduzione dello spazio del world snapshot
- [CODICE] Compressi solo i fixture del ledger condiviso nel .CAR JSON tramite miniz (zlib + Base64); nessuna DLL, file secondario o simulatore duplicato.
- [SAVE] Nuova chiave fixtures_zlib e lunghezza decompressa verificata; snapshot v2 in scrittura, lettura v1/v2; .CAR schema 5 in scrittura, schemi 3/4 ancora leggibili.
- [SICUREZZA] La decompressione ha un limite noto, conserva il parser esistente e non importa snapshot invalidi. Se la compressione fallisce, il salvataggio viene annullato anziché perdere il mondo senza avviso.
- [DIMENSIONI] Su dati sintetici, 8.384 fixture 276 KB -> 96 KB; 40.000 fixture 1,32 MB -> 0,46 MB. Non sono misure della build Windows e non includono gli altri dati della carriera.
- [PIANO] Per il motore globale finale serviranno archivi/delta ed eventualmente pack dati statici, ma non è stato cambiato l'assetto architetturale in questa build.
- [TEST] Nessuna compilazione MinGW64 o esecuzione Windows completata dall'assistente, richiede pull/compilazione in GitKraken/MSYS2; strumenti statistici rinviati alla fine dell'engine.


### 0.13.20 — portable autosave
- [CODE] Slot `save/AUTOSAVE.CAR` automatico: firma, fine incontro,
  ritorno sicuro ai menu, catch-up e stagione; CONTINUE dopo riavvio.
- [IO] Salvataggio .CAR tramite .tmp + sostituzione atomica; vecchio
  checkpoint valido preservato se una scrittura fallisce.
- [WINDOWS] save, config e log nella cartella dell'EXE; script separato
  confermato per ripulire i vecchi save condivisi in Documents.
- [LIMITI] Match live 50 Hz non salvato/ripristinato: anti-save-scum
  intrapartita ancora incompleto; niente forza-esito a crash.
- [TEST] Source review; build MinGW64 e Windows runtime pendenti.


### 0.13.21 — corretto controllo date caricamento world
- [CAUSE] Il campionato reale paese 21 (maggio-settembre) crea 45 gare antecedenti al 1° luglio, che il parser della 0.13.20 considerava impropriamente invalide al load.
- [CODE] Leggere fixture mondiali dal 1° gennaio dell'anno iniziale, preservando calendario OG multi-paese, risultati e controlli anti-corruzione.
- [TEST] Test C di round-trip snapshot compresso aggiunto. Verifica Python sul .CAR utente: 8.384 fixture, compressione integra; build Windows e test C compilati non ancora eseguiti.


### 0.13.22 — mondo 70 stagioni, senza nuove formule sportive
- [CODE] Finestra cronologica di due stagioni, roll automatico al 1 gennaio, riuso senza collisioni degli ID a 16 bit solo dopo archiviazione, nuova generazione dal calendario OG anche per leghe maggio-settembre; controllo della continuità dei risultati residenti.
- [HISTORY] Archivio compresso per stagione/lega: campione, punti, incontri, digest; mantiene riepiloghi permanenti senza fixture storiche complete. I risultati vecchi di dettaglio NON sono più accessibili, non nasconderlo.
- [SAVE] Snapshot living_world v3, campo window_first_year, history. Lettore ancora compatibile con v1/v2, schema carriera 5 invariato e autosave in save/AUTOSAVE.CAR.
- [INTEGRATION] Sincronizzazione del clock mondiale a nuovo anno carriera in ContinueCareerNextSeason, lasciando intatti comandi/fisica OG. F12 mostra finestre, archivio e residenti.
- [TEST] Nuovo TEST_LIVING_WORLD.bat e harness C da 70 stagioni, log LIVING_WORLD_70_TEST.log. Validazione ambiente Windows, 70 stagioni e caricamento .CAR reale PENDENTI.
- [DISCUSSIONE FUTURA] Tuning dei valori simulativi dopo struttura engine; nessuna nuova formula sportiva impostata in questa revisione.


### 0.13.23 — correzione test GCC 16.2
- [REPRO] Log reale del test da 70 stagioni: stop al compilatore per 6 warning elevati a errori (2 mjson, 2 miniz, 2 expanded_world_live), non errore della simulazione.
- [TEST RUNNER] Compilazione separata di mjson/miniz con esclusioni mirate dei warning legacy solo in quelle unità; mantenuti `-Wall -Wextra -Werror` in tutto il codice nostro. Aggiornati entrambi gli script C di test Living World.
- [CODE] Cast `long` dei limiti dei parser; validazione numero risultati native OG rispetto a tutti i risultati residenti e archiviati in carriera, non soltanto la finestra di due stagioni.
- [LIMITI] Test compilato e benchmark 70 stagioni ancora non verificati in questa sessione; atteso risultato Windows/MSYS2 dopo pull.


### 0.13.24 — ripara il collegamento dell'harness e i primi match storici
- [REPRO] Log reale del test 70 anni 0.13.23: GCC compila gli oggetti ma il linker non trova `swos_io_realloc` da mjson.o. Nessuna stagione simulata.
- [FIX TEST] Con solo `mjson.o` nei due harness isolati, `MJSON_REALLOC` punta alla `realloc` standard tramite macro + include forzato del relativo header. Il gioco usa ancora l'allocatore SWOS. Miniz e gli altri sorgenti non alterati.
- [FIX ENGINE] Al primo avvio `ExpandedWorldLive_Seal` e in lettura snapshot preesistenti, risultati deterministicamente completati per fixture con data anteriore al 1° luglio d'inizio carriera, necessario per archivio prima stagione dei campionati maggio-settembre. Il tempo non avanza all'indietro.
- [REGRESSION] Test C base e test 70 stagioni verificano la presenza di risultati maggio/giugno prima di procedere. Test di compilazione e run su Windows ancora da verificare.


### 0.13.25 — un solo avvio mondo a START; debug NEW CAREER
- [CODE] Il mondo dei club/competizioni parte al pulsante START di NEW CAREER indipendentemente dal contratto. Eliminata la re-inizializzazione durante la prima ricerca di offerte. Cancel selezione o ricerca cancella soltanto il mondo provvisorio in RAM.
- [DEBUG] F12+N al Title apre il form NEW CAREER, anche con autosave presente; vecchio .CAR conservato fino al primo successivo writer atomico. Nessuna nuova voce grafica Title, nessun gameplay/valore sportivo cambiato.
- [DA COMPLETARE] Differenti cronologie OG/Expanded e bridge calendario nativo, eventi infrasettimanali, coppe persistenti e carriera mondo ancora da unificare e validare. Non chiamare la singola inizializzazione un motore unificato completato.
- [PARAMETRI] Forma individuale, chairman e coach AI e tutti i valori iniziali rinviati alla penultima fase progettuale approvata.
- [TEST] Verifiche statiche; build Windows e test manager reali pendenti.

### 0.13.26 — single civil-day advancement and world league event diagnostics (12 November 2026)
- One `ExpandedWorldLive_AdvanceToDate` advances league fixtures and preseason through the existing world ledger; weekly and OG season-boundary callers share it. July 1 does not round into a future week. No new calendar generator, .CAR fields, competition handlers, or 50 Hz match changes.
- F12 shows WORLD CLOCK, NEXT LEAGUE EVENT and pending/overdue fixture counters. A real-time status query checks result counts without altering state.
- 70-season stress harness adds a midweek advance, save/load round-trip, backwards-time rejection and calendar audit checkpoints. Windows/MSYS2 compilation and gameplay verification pending.
- The original OG cup/knockout events and native career fixture callbacks remain active. Full calendar/event unification is still open and must be proven, not assumed.

### 0.13.27 — global club-offer coverage and native knockout outcome bridge (12 November 2026)
- The unemployed manager may advance time with a pending job offer; the offer expires and a fresh one may arrive later. New offer candidates come from actual country team files represented by the imported world leagues, without any manager-nationality filter or artificial >=3-division gate.
- Expand live league capacity and import all supported world/career-country lists using the existing official OG fixture tables. Capacity is raised, while unsupported/missing native formats remain excluded and must be reported rather than fabricated.
- Hook original `DIY_PlayCupMatchAndAdvance` after a completed score and after a real champion is declared. Preserve every OG bracket callback and score; store native knockout match/replay/leg results and trophies in the canonical world save, with a chronological two-season resident window and archived totals. World snapshot internal format v4 remains backward-readable for v1-v3 careers.
- The 70-year C runner now exercises a synthetic knockout outcome journal through season rollover and roundtrip, as well as source-country discovery; `TEST_LIVING_WORLD.bat` fails if any required PASS marker is absent. These static source changes are NOT yet a tested Windows build, nor a full offscreen global cup simulator.

### 0.13.28 — diagnosi fallback OG, test suite unica, debug leggibile (12 November 2026)
- Unemployed offer no longer auto-selects VIEW CLUB OFFER; keeps FORWARD TIME selected for keyboard/controller. Debug mode adds DRAW (F12+4, 1-1 persisted final score), with standalone regression assertions.
- Native join exposes historical non-league event count and direct import reason. The existing OG callbacks remain necessary where world snapshots lack cup history or native fixtures do not match; avoid presenting the old artificial full-screen catch-up card, keep the prior frame while replay continues. This is NOT a fix for global duplication.
- Two-column, color-coded F12 with legible section spacing and real TEAM-file import coverage; correct living-world snapshot display v4. Windows games and legacy renderer are otherwise untouched.
- One BAT runs previous 70-year synthetic stress plus independent midseason, world-live, debug/ledger tests and an installed-data inventory, writing one report. Synthetic results are labelled; no false PASS for worldwide promotion/relegation or offscreen cups.
- Validation: static edits only; Windows compile and runtime tests pending.

### 0.13.29 — native automatic mobility, match-speed debug, suite fix (12 November 2026)
- Fixed isolated midseason test link with its own native OG financial-week global; game global untouched.
- World snapshot v5: stores original automatic promotion and relegation counts, playoff flags and counters, maintains read compatibility with v1-v4. Membership for older seasons recovered in true fixture-pair order, avoiding use of new rosters for old standings.
- New seasons for mobile countries are generated only after previous completed fixture results; automatic places exchanged between adjacent real divisions, while unresolved native playoffs remain visible as pending.
- Real manual career match: F12+5 toggles fourfold pacing, running the original game-frame logic rather than forcing or substituting final scores. Turn off after match ends.
- F12 reports OG/world date discrepancies and automatic movement. One BAT runs 70-season stress, separate four-level mobility stress, all earlier C regressions, and installed DB inventory.
- Validation: source inspection and static checks only. Windows/MSYS2 compile and in-game tests pending. No approved sprite/menu art or manual match game physics replaced.

### 0.13.30 — league result reconciliation and native cup history (12 Nov 2026)
- Original OpenSWOS score remains authoritative; reconcile oriented club ID pair and season instead of relying on the two differing calendar dates. Reject ambiguous pairings and log unmatched OG outcomes rather than silently accepting divergent table totals. No point formula changes.
- OpenSWOS retains its bracket/knockout engine. The world cup journal protects completed score and champion identities against conflicting repeats and persists cup-title IDs after two-year fixture rollover. New world JSON v6 reads v1-v5 saves, including incomplete historical trophy identities.
- Correct standalone 70y mobility harness sign comparison. New stress assertions cover all 70 indexed cup champions, save/load, duplicate result rejection and conflicting trophy rejection.
- F12 debug accepts chosen WIN / LOSE / DRAW and F12+6 to complete a played match through original ESC callback; ordinary match physics unchanged.
- Static source inspection only; native world C runner and Windows runtime tests pending. Real offscreen OG cup progression and removing old catch-up remain unfinished.


### 0.13.36 — current world season native career join and pointer safety (pending Windows verification)
- Added pure absolute-unemployment-weeks to current-OG-season/phase conversion with regression for 43 weeks and 5+ years; archived prior seasons are not replayed.
- Align native season counters when accepting first or subsequent employment; require expected native season before catch-up.
- Prevent career calendar rendering from dereferencing null/low-address competition pointers after save VERIFY reload; preserve existing score records.
- Debug overlay documents F12+7, 260 weeks in next FORWARD TIME.
- Does NOT certify global OG offscreen cup advancement or replace the independently simplified Living World score resolver. Those remain open implementation items, along with test Windows and late-career mobility alignment.


### 0.13.37 — current-season world membership into OG and end-of-season importer
- [CODE] Validated two-pass translation of current-season Living World domestic club rosters into original OG global-team-number division offset table, before OG initializes league/cup brackets.
- [CODE] Native final-standings import can succeed after the last league game only when the full table's club IDs, W/D/L, GF/GA and points exactly match world. Original OG cup/season-end callbacks remain the owner, no fake bracket.
- [CODE] Incomplete native roster/handoff never writes first-contract or rehire autosave as if the join succeeded; added source TEAM file inventory test.
- [PENDING] Complete worldwide OG offscreen cup brackets, original OG score resolver for all world fixtures, structural player transfer state, post-hire progression and Windows test; not included in this build.

### 0.13.38_02 (13 novembre 2026) — Southern-calendar OG/LWE handoff diagnostic
The July-contract regression revealed an unhandled OG calendar with native league fixtures before August. A guarded southern-league cutoff path now attempts a strict canonical-world-to-OG import and rejects mismatches rather than entering a zeroed competition. Existing northern-season handling remains unchanged. F12 exposes this verdict and OG cup-season skip counters. An isolated regression tests the real 30 July 2001 cutoff; an additional runtime-log auditor scans actual Windows gameplay logs for first failures without treating static code inspection as a test. Windows behavior remains unverified until the user's compile and screenshots.

### 0.13.38_05 (13 novembre 2026): Diagnostica formato competizioni e handoff
Implementato un controllo effettivo della fase finale del recupero OG (nessun REACHED anticipato), l'importazione dei risultati per occorrenza nelle leghe a 3/4 cicli con verifica classifica esatta, il reset del CLOCK TRACE su nuova carriera e i campionamenti dei soli cambi di evento. Aggiunta diagnostica permanente dei formati installati per leghe/coppe/tornei/qualificazioni e collegamenti, con CSV e distinzioni esplicite tra dati validi e gameplay non verificato. Aggiunto AVVIA_DIAGNOSTICA.bat che utilizza AVVIA_SWOS.bat, compila normalmente, registra automaticamente il testo aperto in F12/F10/F12+C ed esporta i log della sessione. I BAT non segnalano piu' PASS complessivo con runtime Windows ancora NOT_TESTED. Implementazione pubblicata in sorgenti; compilazione e prova Windows pendenti.

### 0.13.39 — Conversione canonica calendario / salto debug anniversario

- Fonte giorno civile: una sola data LWE, senza ricostruzioni da
  `settimane * 7` dopo salti interannuali.
- Le fasi OG rappresentano 8 fasce per mese: primo giorno canonico;
  nuove conversioni FLOOR (giorno nella fascia) e NEXT (inizio della prima
  fascia non precedente il giorno), con test di andata/ritorno a 30 anni.
- F12+7 ora richiede +5 anni civili, non +260 settimane (non equivalenti).
  La data attuale viene esposta separatamente dalla prossima partita OG.
- Registro delle scoperte: `docs/ENGINE_CODE_DISCOVERIES.md`.
- Corretto l'audit per le sentinelle native `teams=-1`; nuovo audit
  runtime C senza awk. I PASS Windows completi non sono ancora disponibili.

- Data del normale menu calendario in carriera: corretta la vecchia
  visualizzazione da `_lastFinancialWeekIndex` in `menu_system.c`;
  ora mostra il giorno civile LWE, mentre Classic Modes resta OG.
- Test C canonico effettivamente eseguito con GCC: 109.938 giorni e
  7.905 fasi, 11.323 giorni nel periodo 1996-2026; PASS matematico;
  compilazione/test Windows ancora necessari.
- Audit C runtime compilato e verificato su log 0.13.38_05: separa
  classifiche PASS da handoff FAIL e coppe FAIL. Nessun falso PASS.

### 0.13.39_01 — diagnostica calendario 30 anni (14 novembre 2026)

Aggiunto test C `test_calendar_integrity_30.c` con fixture native OG
e codice world live attivo: 30 anni civili esatti, confini gennaio/luglio,
bisestili, serializzazione .CAR con confronto byte-per-byte, nessun
duplicato su avanzamento allo stesso giorno. PASS subordinato ai test reali.
Audit Windows aggiunge monitor cronologico sul log: non certifica
automaticamente 30 anni. Corretto il lettore array dell'audit formati per
evitare falsi riferimenti e il PATH DLL per il compilatore diagnostico
nel BAT. Build e test Windows da eseguire dall'utente.


## [2026-11-14] 0.13.41 — debug WORLD e primo allineamento 2026/27

- [CODICE, DA COMPILARE/TESTARE SU WINDOWS] NEW CAREER include una voce temporanea "DEBUG WORLD" attivabile con mouse oppure con UP dalla prima riga e A/INVIO. Cambiare mondo crea un nuovo processo con `--world=9697` o `--world=2627`, chiudendo quello precedente **solo** se il nuovo processo parte. Non si cambiano tabelle OG a runtime; nessun salvataggio viene cancellato.
- [CODICE, TEST PENDENTI] Portati da OpenSWOS `main` i record generati `world_data.c` compatibili con il pacchetto `data_json/worlds/2627/WORLD.ini` (`efe1464b6d1d62b8`). Restano fuori le API mod aggiunte nell'upstream e non presenti in Expanded: la logica 96/97 mantiene il default nativo.
- [CODICE, TEST PENDENTI] Il World 2026/27 abilita `SWOS_DATA_JSON=1` anche se il launcher sviluppo forza 0; 96/97 mantiene la sua impostazione originale.
- **Compilazione locale, test integrato Windows e carriera 2026/27: NOT_TESTED.** Questa integrazione NON sostituisce ancora tutto il motore gestionale con OpenSWOS recente e NON certifica il funzionamento di START, coppe o classifiche. I campi SWOS 2020 rimangono archiviati, non attivati.


## [2026-11-14] 0.13.42 — sicurezza fingerprint World 2026/27

- [CODICE / WINDOWS NON TESTATO] Il controllo fingerprint dei dati 2627 ora impedisce di proseguire nel mondo moderno quando il WORLD.ini non corrisponde alle tabelle native: registra il mismatch e ripristina il mondo 9697 prima dell'inizializzazione. Il caso WORLD.ini assente mantiene il fallback gia' presente.
- [LIMITI] Il selettore DEBUG WORLD continua a richiedere il riavvio (come in 0.13.41); motore OG unico per carriera/calendario/coppe NON completato. Nessun compilatore Windows eseguito tramite collegamento GitHub. Segnata l'interruzione "Connection interrupted" e mantenuto lo stato delle verifiche pendenti.


## [2026-11-14] 0.13.43 — default World al bootstrap
- [CODICE] `main.c` seleziona 2026/27 se il mondo non è stato indicato; rimangono prioritari i valori espliciti e la validazione preventiva del pacchetto 2627. La selezione avviene prima che `frontend.c` installi le tabelle OG. Test Windows NON ESEGUITI, nessuna migrazione a caldo o unificazione LWE dichiarata.


## [2026-11-14] 0.13.43_01 — log completo identificatori club esclusi
- [CODICE] L'importatore registra su SWOS.log ogni riga TEAM esclusa con chiavi sorgente e motivo, al posto di perdere i dettagli dopo il campione F10. Non cambia fixture, classifiche, salvataggi o rosa. Test runtime Windows NON ESEGUITO.


## [2026-11-14] 0.13.44_02 — autosave indipendenti per i due mondi
- [CODICE] Scrittura checkpoint per mondo in `AUTO9697.CAR` o `AUTO2627.CAR`, senza sovrascrivere `AUTOSAVE.CAR` storico. Lettura legacy compatibile in fallback; CONTINUE mantiene controllo schema/kind/world. La rimozione a fine carriera riguarda solo il mondo attivo. Build e test Windows: PENDENTI.


## [2026-11-14] 0.13.44_03 — OG world / competitions / manager separati

- [CODICE, WINDOWS NOT_TESTED] Divisa inizializzazione competizioni OG da world e manager in `career_init.c`, preservando le originali routine/reset, senza cambiare fisica o salvataggi. Corretto il percorso OG cups del 1996/97 che saltava la lista dei paesi; indice stagione in codifica OG 96-based con wrap 99->0. Non tutte le coppe né il calendario mondiale OG sono ancora integrati end-to-end.


## [2026-11-14] 0.13.44_04 — OG native league callbacks; no autonomous LWE scores
- [CODICE, NON TESTATO] Nella carriera non si inizializzano più fixture sintetiche LWE e il suo avanzamento non risolve partite, amichevoli o promozioni. Il percorso disoccupato chiama anche le routine OpenSWOS native per divisione a fine stagione. Restano aperti avanzamento durante l'anno, subentro e offerte. Windows e suite integrata: NOT_TESTED.


## [2026-11-14] 0.13.44_05 — OG-only save reader
- [CODICE] Snapshot nuovo senza fixture LWE accettato dal reader se contatori risultati coerentemente nulli. Log escape corretto. Prova save/load Windows PENDENTE.


## [2026-11-14] 0.13.44_07 — offerte senza standings LWE
- [CODICE, NOT_TESTED] Disponibilità di club nelle ultime divisioni originali con almeno quattro livelli importati senza simulare LWE. Non inventa ranking; collegamento alle classifiche OG per realismo offerte ancora da fare.


## [2026-11-14] 0.13.44_08 — OG native catch-up without LWE fixture transplant
- [CODICE, NOT_TESTED] Salto del vecchio import LWE se il mondo non contiene fixture; subentro rimane gestito dalle azioni e callback native OG, senza punteggi sintetici esterni. Calendari meridionali e test Windows ancora aperti.


## [2026-11-14] 0.13.44_09 — original OpenSWOS competition authority gate
- [CODICE] Il gate OG-only blocca l'utilizzo di classifiche, sorteggi, replay e mobilità calcolati nel LWE anche nei vecchi salvataggi. I dati legacy vengono conservati ma non possono più sostituire punteggi, partite o ordinamenti nativi. Test Windows NOT_TESTED.


## [2026-11-14] 0.13.45 — read-only standings bridge for unemployed offers
- [CODICE] Copia temporanea delle classifiche OG quando effettivamente sortite dal motore originale (club IDs e W/D/L/GF/GA/PTS); chiavi per paese, divisione e stagione, invalidazione tra stagioni. Anteprima e selezione offerte usano solo tali valori se disponibili, senza riaccendere score LWE. Nessuna partita ulteriore viene simulata dal ponte. Windows NOT_TESTED.


## [2026-11-14] 0.13.45_01 — fix compilazione due errori OG offer
- [CODICE] Rimosso `rankedCount` duplicato in `choose_small_club`. Aggiunta include `diy_competition.h` (solo SDL) in `career_init.c` per dichiarare `DIY_OGStandings_SetSeason`. Correzione basata su `COMPILAZIONE_0140.log` fornito dall'utente; avvisi preesistenti su packed member non bloccanti. Windows build dopo fix PENDENTE.


## [2026-11-14] 0.13.45_02 — audit init OG e tracciamento del reingaggio
- [CODICE] Log diagnostici nei percorsi native `InitCareerClub` / `Career_AttachManagerToClub`, per osservare se la firma ricostruisce ancora le competizioni. Corretto attributo `externally_visible` su variabile `static` (warning MSYS2). Nessun reset/simulatore/calendario aggiuntivo; non dichiarare unificazione del motore raggiunta. Compilazione e test Windows PENDENTI.


## [2026-11-14] 0.13.45_03 — native season vs club lifecycle split
- [CODICE] Introdotta `Expanded_InitializeClubSeasonState` come variante specializzata per il club, chiamata all'assunzione senza incrementare/ridurre i contatori globali della stagione o reset mondo playoff. `InitializeNewSeason` mantiene l'avanzamento anno/gruppi/playoff ai soli normali cambi di stagione OG. L'attach OG completo richiede ancora contesti di competizione mondiali persistenti: non risolto dalla sola variante. Windows compile/game NOT_TESTED.


## [2026-11-14] 0.13.45_04 — og standings offer preview/zero-match season
- [CODICE] Rimossa la falsa esclusione `hasGroups` dalle acquisizioni single-group native OG, mantenute convalide identità e W+D+L=PL, ammessi snapshot PL=0 pre-campionato. In mancanza di classifica catturata, l'offerta mostra nomi dei club dal TEAM-file snapshot, esplicitamente senza inventare punti. Log delle catture reali `[og-offer-table] CAPTURE`. Nessun nuovo simulatore, nessun reset delle competizioni. Windows compile e test gioco DA FARE.


## [2026-11-14] 0.13.45_05 — primo contesto OG indipendente dal manager
- [CODICE] Resident OG league image + original team list/RNG/phase per divisione in RAM, avvio a START, avanzamento con FORWARD TIME e binder club che valida gli ID reali. Ridotto replay OG alla firma quando l'immagine esiste. Nessun risultato LWE. Si tratta di implementazione sperimentale e NON di completa migrazione worldwide (coppe, save, keycache storica ancora da integrare). Test Windows NON eseguiti.

### 0.13.45_06 — DEBUG WORLD consultivo (14/11/2026; test Windows pendente)
- `CAREER / VIEW WORLD` non esegue più le callback OG di auto-simulazione selezionando competizioni esterne; le leghe con contesto OG residente sono visualizzate tramite copia temporanea in sola consultazione.
- La schermata disoccupazione dispone del nuovo pulsante `DEBUG WORLD`; record, offerte, FORWARD TIME e navigazione restano disponibili. Le competizioni senza stato residente sono indicate come non disponibili.
- Consegna del sorgente per compilazione e test Windows da parte dell'utente; non certificato in runtime.

### 0.13.45_07 — allowlist menu disoccupato e viewer OG (Windows da testare)
- Ripristino sicuro del menu disoccupato dopo modal: onReturn=Expanded_UnemployedInit, callback SQUAD e altre azioni del club rimosse oltre a essere invisibili; filtro MenuProc rifiuta anche tentativi via input vecchi. DEBUG WORLD ha coordinate e tipo di disegno espliciti sopra RECORD.
- Vista OG valida i riferimenti delle classifiche prima di aprire il menu nativo e non ri-cattura la classifica di mercato/offer mentre consulta lo snapshot. Non introduce nuovi simulatori o risultati.
- Correzione motivata dai crash e dai tasti invisibili segnalati dall'utente nella 0.13.45_06; compilazione/test Windows spettano all'utente, nessun PASS runtime asserito.

### 0.13.45_08 — proposta codice: offerte collegate alle leghe OG residenti
- I valori nel pannello VIEW COMPETITIONS della proposta non dipendono più da una cache costruita aprendo i menu OG; la classifica del paese/divisione/anno viene letta dal contesto OG già avanzato in s_expOG. Stessa fonte per individuare club in difficoltà, senza un secondo simulatore.
- Compilazione/test Windows dell'utente pendenti; gli altri bug della 0.13.45_07 (crash a novembre, UI DEBUG WORLD, TOP GOAL SCORERS) ancora da indagare.

### 0.13.45_08 — status before Windows build (15/11/2026)
- CLUB OFFER and AI first offer candidates read identical resident OG league snapshots by country, division, year, not stale view-callback cache. Missing snapshot is reported rather than simulated.
- Unemployed menu custom SDL renderer finally includes `DEBUG WORLD` (ordinal 11) and paints its label; previous whitelist still excluded it after MenuEntry setup.
- Prevent OG WORLD from simulating a completed league a second time; validate identity indices before scoring, fail and emit diagnostics for invalid/self fixtures and offscreen infinite-loop conditions. OG algorithms remain authoritative and CLASSIC/DIY paths unchanged. Windows crash fix NOT VERIFIED.
- The user compiles/tests on Windows after GitKraken; no full-world date rollback or 2026/27 competition-date correction in this patch. Log and diagnostics remain outside the save.

### 0.13.45_08 — WORLD visible at START (15/11/2026, Windows NOT TESTED)
- START crea gli stati OG delle leghe e le porta immediatamente al 1 luglio dell'anno scelto, invece di aspettare il primo FORWARD TIME. Le leghe iniziate prima della data mostrano le gare reali già disputate quando l'utente apre DEBUG WORLD.
- Calendario iniziale letto dalla data civile esistente, nessun secondo clock; riuso dello stesso `Expanded_OGWorld_AdvanceToDate`, perciò nessun doppio risultato al primo FORWARD TIME. Se catch-up iniziale fallisce, si interrompe la nuova carriera.
- Nessuna coppa mondiale residente nuova, nessun trasferimento/finestre implementati, nessuna modifica gameplay CLASSIC.

### 0.13.46 — fix compilazione e adeguamento documentazione (15/11/2026)
- `career_status.c`: i due controlli OG contro incontri oltre la fine del campionato leggono il membro esistente di `DIYCompetitionEx`, `_diyCompetitionStart.minSubstitutes`, invece dell'inesistente `DIYCompetition.minSubstitutes`. Algoritmo risultati OG, scope del guard e CLASSIC/DIY invariati.
- Aggiornate le guide di progetto e i riferimenti dei documenti upstream ai due salvataggi portatili e alla sola autorita' OG.
- Windows 0.13.45_08: compilazione FALLITA nel log utente. 0.13.46: compilazione, gameplay, FORWARD TIME, fine campionato e save/load OG NON PROVATI.

### 0.13.47 — OG-only diagnostics / BAT / DEBUG WORLD BACK (15/11/2026)
- F12 usa esclusivamente letture dei contesti OpenSWOS residenti per conteggio leghe, paesi, partite, completamenti e stato del collegamento manager; F12+C rende esplicita la distinzione fra record di coppa OG nel database e `cup-resident=0`. F10 elenca definizioni native; rimossi dalla nuova schermata F12 indicatori LWE fuorvianti.
- `career_init.c` registra `[og-world-audit]` a START e TICK. `audit_windows_runtime_log.c` riconosce build corrente, progressione OG, errori fixture, bind, viewer, BACK multilivello, assenza cup resident. Il rapporto resta INCOMPLETE finché coppe OG datate e salvataggi non sono realmente provati.
- `AVVIA_DIAGNOSTICA.bat`, `ANALIZZA_SWOS_LOG.bat`, `TEST_OG_WORLD.bat` e wrapper `TEST_LIVING_WORLD.bat` adottano i nuovi marker. Il runner `run_test_engine_suite.sh` esegue SOLO test statici su dati OG/algoritmi fase e non usa più la simulazione sintetica LWE come test finale.
- `Expanded_UnemployedDebugWorld` usa un flag temporaneo di selettore per garantire BACK di un livello sui paesi; nessuna variazione a CLASSIC o ai menu del manager assunto.
- **VERIFICHE:** ispezione sorgenti, dipendenze e separazione fra prove: SI; compilazione Windows 0.13.47 e gameplay dopo modifica: NON PROVATO. Coppe residenti e TOP GOAL SCORERS: ancora aperti.

#### Estensione test nativo 70 anni (richiesta del 15/11/2026)
- `F12+8` / `Expanded_DebugArmSeventyYearOG` arma 70 anni reali in UNEMPLOYED; FORWARD TIME usa i contesti OG, mai l'antico simulatore di risultati LWE. Ogni stagione chiusa e riaperta emette controlli nativi nel log.
- `TEST_OG_70.bat` restituisce PASS limitato ai 70 passaggi OG e alle date esatte quando il parser li osserva realmente; il rapporto mondiale complessivo resta INCOMPLETE per coppe, cronologia completa e save/load. Non testati pensionamenti del manager e dei calciatori: predisposti come separata fase futura.
- F12+8 modifica la carriera in memoria: usare soltanto una nuova carriera di test. I callback storici di coppa a fine anno sono saltati nel salto diagnostico per non creare 70 anni di esiti retroattivi; F12+7 e il tempo normale restano invariati.

### 0.13.48 — F12+8 annuale, test OG before/after, BACK e offerta diretta (15/11/2026)
- 0.13.47 crash/interruzione su 70 anni: 24 stagioni OG attraversate e ultima OPEN 2050; causa non identificata. Il salto ora usa annual steps da MenuProc per evitare una singola operazione di 70 anni; 70 CLOSE/OPEN/STEP e data esatta necessari per PASS_TRANSITIONS_ONLY. Si interrompe se un anno fallisce.
- Ogni step registra anno/phase/country/division prima e dopo la chiamata OG. Aggiunto marker RESET per non confondere nuove carriere in un unico SWOS.log con un rewind.
- Checkpoint OG per coppe (definitions != resident) e marcatori (PlayerFile.goalStats[0] + standings GF), prima/dopo 70 anni e vista offerta. F12 mostra contatori dei marcatori; TEST_OG_CUPS_SCORERS.bat separa evidenza dati e UI.
- Pad Switch B viene gestito come BACK, non FIRE/SKIP, solo nel selettore DEBUG WORLD del disoccupato; a ogni pressione una sola pagina, CLASSIC invariato.
- VIEW COMPETITIONS da un'offerta apre direttamente la divisione OG residente del club e torna all'offerta; invarianti dei risultati controllati. Nessuna seconda simulazione.
- Test del parser con casi positivi/negativi isolati, senza simulare il mondo. Test Windows 0.13.48, coppe residenti, TOP GOAL SCORERS, save/load globale e pensionamenti ancora NON PROVATI o mancanti.

### 0.13.49 — avvio Windows: guard B/SKIP compatibile con DEBUG WORLD (15/11/2026)
- Fix del controllo *precompilazione* in `sorgenti/tools/patch_title_b_01063.sh`: la versione 0.13.48 protegge la selezione DEBUG WORLD con un terzo predicato `!Expanded_UnemployedWorldPickerActive()`, ma la patch Title-B cercava una vecchia riga monolitica e fermava `BUILD_SWOS.sh` con `patch controller incompleta`. Il controllo riconosce le due forme supportate (legacy e nuova forma contigua) ed evita di sostituire un guard già corretto.
- Esito test isolato delle condizioni: due guard validi accettati, uno errato rifiutato. Nessuna modifica al codice C del motore OG, al controller runtime, ai menu o agli asset. Build Windows 0.13.49 **non provata**; il precedente log dimostrava soltanto lo stop del tool di preparazione.

### 0.13.50 — resident OG domestic cups at START and FORWARD TIME (15/11/2026)
- Introduced per-country, per-table-slot, per-year native OG cup state, using OG selection, bracket initialization, phase-limited round advancement and match simulation (no second result engine).
- Stops the former end-year cup backfill when the resident OG world is running, preventing the repeated 2026 cup results that followed the successful league stress to 2096.
- DEBUG WORLD for cups reads a scratch resident context; native cup match callbacks are blocked while the viewer is active. Added audit for resident matches and unsupported states.
- Safety: country table sentinel bounds, original 64-team bracket bound, no match result invention. Employed club cups remain on their existing OG slot, explicitly excluded from independent resident simulation (MANAGER_SLOT_UNBOUND) until a full identity mapping is proven.
- Not yet complete: manager cup bind/unbind, international cups, cup final archival, persistent save states, and Windows tests. Static C checks do NOT certify no crash.

### 0.13.51 — collegamento marcatori OG, BACK, diagnostica unificata e coppe per-ID (15/11/2026)
- Reader read-only dei `TeamFileEx.goalStats` dei campionati residenti OG, usato da TOP GOAL SCORERS dopo il bind del manager; il normale inserimento/classifica OG non genera nuovi risultati. Risolve una possibile divergenza tra `_selectedTeams` locale dell'ingaggio e stato OG persistente, da verificare su Windows.
- DEBUG WORLD e VIEW COMPETITIONS dell'offerta riusano il selettore nativo OG; dall'offerta paese noto e sola divisione pertinente, con coppe residenti che includono il club. BACK di un solo livello (viewer -> selettore -> offerta), non callback FIRE; comandi CLASSIC/partita invariati.
- Audit puntuale all'avvio per le coppe type-1 nazionali: `country+slot+year`, compId, formato, partecipanti definiti vs selezionati e motivo dell'assenza; parser stampa `OG_CUP_COVERAGE` e `OG_CUP_DIVERGENCE`, senza falso PASS sulle 70 definizioni non residenti.
- `AVVIA_DIAGNOSTICA.bat` ora avvia build/gameplay Windows e, dopo la chiusura, tutte le suite e i rapporti OG in `RISULTATI_DIAGNOSTICI/DIAGNOSTICA_SESSIONE_N`; archivia anche vecchi log separandoli dai nuovi. Corretta l'invocazione Bash del test statico. Legacy `TEST_LIVING_WORLD` resta alias, non un secondo motore.
- Aggiornato `build_info.h` alla 0.13.51. **Solo modifiche al sorgente e script: la compilazione dell'EXE, UI e gameplay Windows vanno provati dall'utente.** Restano pendenti coppe internazionali, cup manager bind, save/reload integrale OG e pensionamenti.

### 0.13.52 — fix Windows syntax + tutti i test con ZIP automatico (15/11/2026)
- Diagnostica ZIP `DIAGNOSTICA_SESSIONE_2(1).zip` utente: build 0.13.51 bloccata in `src/teams.c:3622` (`Expanded_UnemployedWorldPickerActive` implicit declaration). Corretto prototipo SDL file-scope; non modificata logica OG e nessun risultato risimulato. Aggiornato `build_info.h` a 0.13.52.
- `AVVIA_DIAGNOSTICA.bat` archivia SWOS.log precedente prima del compilatore, evita che log 0.13.50 siano valutati come 0.13.52, mostra la prima diagnostica di errore anche quando l'EXE non parte, continua tutti i test indipendenti e genera ZIP sessione automaticamente, con test `tar -tf` e avviso esplicito di fallimento.
- Nuovo runner `sorgenti/tests/run_all_diagnostic_tests.sh`: scoperta automatica di tutti i run_test*.sh, incluso `run_test11_offer.sh` e format audit, un log per ogni script in `TEST_ESEGUITI` e riepilogo current vs legacy. Aggregatori e alias non vengono invocati due volte. Test11 usa cartella temporanea unica e ASAN opzionale, senza nuova toolchain.
- I runner OG_70/coppe riusano un unico SWOS_RUNTIME_AUDIT senza ricompilarne il parser per ogni controllo; l'esito di ogni sottotest e' tracciato. I test legacy sintetici possono restare FAIL/OBSOLETE e non diventano un secondo motore di gameplay.
- **Windows 0.13.52 NON COMPILATA NE TESTATA dall'assistente**; verificare con l'utente, compresi VIEW COMPETITIONS, BACK, goal scorers, cup coverage e salvataggi pendenti.

### 0.13.53 — validazione rigorosa: bye OG, nazionali/internazionali, SAVE, warning (15/11/2026)
- Strumentazione sola lettura `Expanded_OGWorld_CupSetupAudit` e `[og-cup-rule]`: confronta squadre reali, padding del tabellone, bye, regola originalissima `WorldCups_CupMade` e partite del primo turno predisposte, senza cambiare selezioni o risultati. `OG_CUP_FORMAT_RULES` non promuove a PASS le 70 coppe mancanti per convenienza.
- Audit separato dei tornei UEFA/continentali per club e Mondiali/Europei/altre competizioni delle nazionali: conta definizioni e sottosistemi esistenti senza confonderli con stati OG residenti, fixture datate e match effettivi. `OG_INTERNATIONAL_NATIONAL` resta NOT PROVED finche' tali evidenze non siano disponibili.
- Checkpoint post `Expanded_AutoCareer_Save/Load` sui contesti OG residenti e lettura dei log consecutivi: il rientro nel 2096 e' test dell'anno, NON prova della serializzazione di tutte le classifiche, fixture, RNG, rose e coppe.
- Suite unificata AVVIA_DIAGNOSTICA: test legacy LWE obsoleti marcati NOT_RUN_OBSOLETE (nessun secondo motore, sorgenti conservati), nuovi controlli `run_test_final_og_contract.sh`, warning GCC `COMPILER_WARNING_AUDIT.log`, file di continuita' save. ZIP automatico esistente invariato.
- Analisi del log reale GCC 0.13.52: 29 warning: 25 `-Waddress-of-packed-member` (non silenziati), 3 misleading-indentation e 1 missing-braces. Correzione locale non funzionale dei 3 `if` ambigui e dell'inizializzazione; accertare in Windows se i 4 scompaiono. Rischio alignment a 25 siti non risolto senza audit degli accessori, nessun refactor arbitrario.
- Sorgenti build 0.13.53, NON compilato/testato su Windows dall'assistente, nessun aggiornamento della simulazione OG.

### 0.13.54 — 70 coppe OG con bye, paese-per-paese da START (15/11/2026)
- Identificato dal log Windows 0.13.53 il bug strutturale `base.numTeams<=_numSelectedTeams` nel filtro residente: il tabellone OG e' pieno per definizione, ma gli iscritti reali possono essere meno per bye. I 70 casi problematici di 64 paesi su 101 definizioni/87 paesi rispecchiano le regole native.
- Corretto il percorso del gestore residente senza modificare OG: solo se `World_Cups()`, `ExpandedOGCup_NativeByeLayout`, primi turni e ID degli avversari confermano una disposizione legittima, la coppa originale con bye viene conservata. Non viene creata nessuna squadra, nessun calendario o risultato; errori restano UNSUPPORTED, loggati.
- A START log `[og-cup-country]` per tutti i paesi con coppe e `[og-cup-start]` aggregato; parser produce `OG_CUP_COUNTRY`/ `OG_CUP_COUNTRIES`/ `OG_CUP_START_COVERAGE`, includendo il totale di definizioni per prevenire PASS su paesi mancanti.
- Test isolato C di validazione OG nativa aggiunto e integrato nell'unica `AVVIA_DIAGNOSTICA.bat`; fixture parser positive/negative aggiornate. Vecchi test LWE esclusi come da 0.13.53. Versione `0.13.54`; Windows compilazione, tabelloni reali, avanzamento 70 anni e interazione GUI NON TESTATI dall'assistente.
- Limiti aperti: percorso nazionale/internazionale (audit separato) e save residente OG; `g_worldCups.seasons` zero dopo 70 anni nel log precedente deve essere investigato SENZA doppio tick; coppe del manager e vincitori storici da confermare.

### 0.13.55 - diagnostica Title, copie CAR, named external cups (15/11/2026)
Codice preparato, test Windows di questa versione NON PROVATO. Sessione 5 utente su 0.13.54: Windows build e stress 70 anni riusciti nello scope dei 134 campionati e 101 coppe nazionali residenti (87 paesi); due riavvii completi dell'EXE con salvataggi e coppe/marcatori riferiti corretti.
Il checkpoint del ritorno al TITLE esegue controllo read-only di compatibilita' del CAR su disco; la diagnostica unica copia i CAR nello ZIP con confronto binario e hash SHA256 opzionale. Il picker dell'offerta consente vedere le coppe OG residenti del paese anche senza iscrizione della squadra offerta, utilizzando gli stessi dati della vista DEBUG WORLD.
Audit per nome di Champions, Europa League, Conference, coppe europee storiche, Libertadores, altre confederazioni, Mondiali/Europei e coppe/qualificazioni nazionali. Differenza tra DEFINIZIONI, CONTATORI OG e FIXTURE PROVATE sempre esplicita; non modificati gameplay, RNG o partite. Save di disoccupato mensile e full world serialization non aggiunti.


### 0.13.56 — anteprima rosa OG, inventario Mod 4.1.2, base di avanzamento coppe, pulizia (16/11/2026)
- Offerta di assunzione: percorso `ShowSquad` specifico dell'offerta sostituito con `_squadMenu` OG/Expanded, usando copia temporanea di `TeamFileEx` e snapshot del contesto; training/scambi/azione gestione bloccati anche nel dispatch. Renderer delle stelle condiviso con la rosa dell'allenatore. Nessun callback OG classico rimosso. **Test di compilazione/gameplay Windows in attesa**.
- Refactor additivo non invasivo nel motore extra-UEFA `world_conti.c`: preparazione partita da `CT_ED` ora funzione distinta e API `ct_world_prepare`/`ct_world_advance_month` con cursore di mese, OG `ct_round` una volta per fase datata. Non collegate ancora al mondo residente poiché mancano i partecipanti/salvataggio/collegamento della gara del manager: non dichiarare tornei internazionali funzionanti.
- `career_init.c` registra i singoli 12 nomi reali extra-UEFA con scope di sola definizione; audit precedente mantenuto. Inventario 4.1.2 in `docs/` (264 voci tecniche, 10 Supercoppe), tabella di coverage e test `run_test_mod2627_coverage.sh` aggiunto alla suite diagnostica.
- Root ripulita dai documenti storici, spostati sotto `docs/archive/` e `docs/reference/`; build aggiornata affinché non rinomini più asset già superati; eliminati dal repository 10 asset con nome `_OBSOLETO` e lo script di rinomina non più chiamato. BAT `PULISCI_OBSOLETI_SWOS.bat` elenca i soli asset già obsoleti, anteprima + richiesta ELIMINA, senza stadi, raw OG o save.
- **NON** nuova build Windows, non gameplay testato dall'assistente. User compila con GitKraken/MSYS2, test e problemi eventuali da registrare alla prossima verifica.


### 0.13.65 (17/11/2026) — Reset annuale selettivo, calendario-base gara Libertadores e 47 ID verificati
Modificato `career_status.c` e `career_init.c`: un'edizione OG continentale con CT_ED attivo non viene azzerata durante il cambio delle leghe al 1 luglio; reset completo su nuova carriera resta. Helper in `world_conti.c` restituisce date-base reali per singolo fixture/leg 2027 (non registra né simula partite). Manifest dati dei 47 club moderni e guida dettagliata per 1997/2027. **NON IMPLEMENTATO** collegamento completo calendario per gara, sorteggi, risultati, viewer o save per fixture: `ct_round` attuale continua a simulare il turno intero e l'helper non è invocato dall'esecutore residente. Windows compile/gameplay NON PROVATI.


### Build 0.13.66 — 17/11/2026 — protezione crash e input picker
- Preflight OG storica per ogni divisione (lettura reale TEAM.NNN) impedisce panic 0x8885 quando formato campionato e squadre nel file divergono; lega incompatibile SKIPPED / NOT_RESIDENT anziché falsamente riuscita. Debug atteso/effettivo. Finlandia 12 vs 14 NON RISOLTA nel dato.
- DEBUG WORLD/OFFERTA: navigazione circolare su tutte le righe visibili e ridisegno del focus. VIEW SQUAD offerta: B chiude prima popup X, poi torna all'offerta con una nuova pressione.
- Non implementati hot switch 96/26, audit completo regole/ID per entrambi i mondi, e tutte le coppe non residenti. Stato esplicito e test in `docs/WORLD_PRIORITY_20261117_BUILD_01366.md`. Compilazione e gameplay Windows 0.13.66 NON PROVATI.


### Build 0.13.67 (17/11/2026) — NO RESTART cambio mondo 9697 ↔ 2627
- `world_switch.c`: banche statiche 9697/2627 preparate al LOADING e scambiate nel processo attivo senza lanciare un EXE; 290 aree native/table snapshot, registro compId separato, check compatibilità WORLD.ini, cache squadra invalidata.
- `expanded_new_career.c`: eliminata la dipendenza Windows CreateProcess/GetModuleFileName/exit e cambiata etichetta in NO RESTART; con dati 2627 assenti/incompatibili, mostra WORLD DATA UNAVAILABLE.
- `frontend.c`: integrato setup doppia banca nel bootstrap, amica/CLASSIC rimane nel percorso precedente.
- `competition_data.c/h`: funzione portabile di esportazione/ripristino registro dinamico, nessun intervento sul motore originale delle partite.
- Test Windows cross-world, save e startup: **NON PROVATO**. I due mondi NON diventano contemporaneamente attivi e non vengono giocati match in anticipo. Documento: `docs/WORLD_SWITCH_NO_RESTART_20261117.md`.
