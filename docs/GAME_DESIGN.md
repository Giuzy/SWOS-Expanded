## 19/11/2026 — 00.13.194: FINISH SEASON OG invisibile senza saltare ordinamento e mobilita'
- Diagnostica Windows 00.13.193 (sessione 1 20261119-091533): UNEMPLOYED apre il secondo anno OG con 134 leghe (2627) e 88 (9697) senza invalidi; 9697 ha ancora qualificazioni `FIELD_INCOMPLETE`, SAVE/CONTINUE non provato. EMPLOYED fallisce 2627 per `country=44 division=3`, 12 squadre richieste/11 reali, e 9697 per `PANIC 0x5555` su bilancio dei trasferimenti di divisione.
- Correzione alla 00.13.193: `ShowDiyLeaguesTournamentMenu` ordina la classifica con `DIY_SortLeagueTableByStandings` PRIMA che qualificazioni/promozioni leggano gli indici. Non saltare MAI quel callback OG per nascondere la UI: ripristinato in `career_season.c` e `career_status.c`.
- Soltanto Expanded CAREER OG-only: `MenuProc` elimina l'attesa artificiale di 50 frame per OG menu e SDL non presenta i frame intermedi quando `_seasonUpdateInProgress` (continua eventi/input/audio); CLASSIC/DIY/SEASON e altre piattaforme invariate. Log finale `[og-season-ui] HEADLESS_NATIVE_MENUS`.
- Preflight OG nuovo residente: `INITIAL_TEAM_SELECTION_MISMATCH` rileva 12/11 immediatamente senza inserire squadre inventate o salvare stati incoerenti; `FILE_BALANCE_MISMATCH` aggiunge dettagli del bilancio 0x5555 senza nascondere il PANIC.
- Diagnosi tecnica e checklist in [OG_SEASON_MANUAL_HEADLESS_FIX_0013194_20261119.md](OG_SEASON_MANUAL_HEADLESS_FIX_0013194_20261119.md). Nuovo codice **NON compilato e NON testato su Windows** da assistente, nessuna garanzia che il cambio manuale sia gia' riparato. L'utente compila/testa dopo GitKraken; warning nuovi da correggere subito. Unico commit autorizzato.

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

# SWOS Expanded — Game Design Document

## 18/11/2026 — punto2: entità e fixture senza secondo motore
- La versione 0.13.88 aggiunge ClubKey/CompetitionKey/EditionKey/FixtureKey tipizzati e un possibile claim esclusivo di fixture, ma non attiva nuovo scheduler né risultato. ID dei club persistenti da world-pack, NON UUID/checksum di TeamDb o compId a 8 bit.
- La cache dati squadre viene segregata per 96/26 e CAREER/CLASSIC. Per le coppe live la presa in carico deve diventare un'unica transazione atomica con OG e CAR prima di migrare una competizione. Decisione B storica rinviata. [Report](COMPETITION_ENGINE_POINT2_20261118.md).


## 18/11/2026 — scelta B FUTURA, prime competizioni BASILARI adesso
- Utente approva **B**: mondo che nasce nel 1996/97 con competizioni che evolvono storicamente fra edizioni (Champions 16→24 nel 1997/98 come esempio), tramite ruleset/data pack verificati. **Ora la priorità è far funzionare engine unico e coppe basilari**, non implementare subito i cambi di formato. Il pilot OG a 16 ripetuto è SOLO test di API, non tre autentiche edizioni storiche; CAREER live non può presentare finte edizioni 1997/98.
- Distinguere ClubKey persistente, TeamDb UUID di dati fissi, compId nativo non globalmente unico; rispettare file TEAM storici usati dalla CAREER e non dalla cache generica. Fonte: [COMPETITION_ENGINE_PHASE1_CLOSURE_20261118.md](COMPETITION_ENGINE_PHASE1_CLOSURE_20261118.md). Questo commit non cambia gameplay.


## 18/11/2026 — precisazione progettuale emersa dall'audit OG
- Non esiste un unico simulatore OG universale: diverse famiglie (partita normale, continentali moderne, playoff/Serie D, qualificazioni) usano algoritmi e RNG differenti. Il Competition Engine definisce un contratto UNICO DI EVENTO/RISULTATO e seleziona il rispettivo adapter OG con comportamento verificato, senza imporre la stessa funzione di score a tutte le competizioni.
- Prima coppa pilota scelta come candidato tecnico: **Champions storica 1996/97, type-2 con 16 squadre/quattro gironi/KO**, tre edizioni consecutive con veri qualificati. Il formato negli anni successivi nel mondo storico NON si assume invariato o aggiornato senza decisione e dati; data civile, risultato unico e manager bind devono funzionare. Secondo formato proposto: Coppa delle Coppe 1996/97.
- `CAR` OG esistente e' base da preservare, non da sostituire; processo intero di season-end OG NON puo' essere richiamato come sola qualificazione. Vedi [COMPETITION_ENGINE_OG_AUDIT_20261118.md](COMPETITION_ENGINE_OG_AUDIT_20261118.md). Nessuna modifica al gameplay/decoder/motore e nessun nuovo sistema in uso.


## APPROVATO 18/11/2026 — gestione Expanded delle competizioni, riuso OG
- Superata SOLO la parte della regola 14–15/11 che attribuiva obbligatoriamente a OG anche la titolarita' esclusiva dei calendari e del ciclo completo dei tornei. Expanded diventera' **UNICO** gestore di data/fixture/edizioni/qualificazioni/storico/CAR, riutilizzando algoritmi OG (e preservando partita originale 50 Hz). Vietati due mondi, due simulatori concorrenti, doppi risultati, replay di gare precedenti e reset al cambio club. Resta invariato il mondo 96/97 vs 26/27 con regole specifiche; CLASSIC/DIY/SEASON intatti.
- Obiettivo: definizioni di tornei tramite pack versionati validati (OG, altri SWOS, creazioni dell'utente), per aggiungere molte competizioni senza nuovo C per ogni torneo esprimibile con primitive comuni. Primo traguardo reale: una COPPA OG gia' esistente, preferibilmente continentale, con TRE edizioni consecutive, qualificazioni, fixture/ID, save e CONTINUE; poi formato differente e import batch. Sette attivita' in TRE traguardi NON sette versioni.
- Possibile data scelta in NEW CAREER successiva all'origine verificabile del mondo e pre-simulazione di tutti gli eventi; DEFAULT 1 luglio non cambiato ora. Storia per-gara a finestra o totale pluridecennale da decidere dopo misure; regolamenti e date source da verificare.
- Specifica completa, TODO e gate: [COMPETITION_ENGINE_ARCHITECTURE_TODO_20261118.md](COMPETITION_ENGINE_ARCHITECTURE_TODO_20261118.md). Questo e' DESIGN, non codice gia' implementato.


## Aggiornamento vincolante 16/11/2026 — competizioni di entrambe le epoche

Vedere [COMPETITIONS_TWO_WORLDS_SPEC.md](COMPETITIONS_TWO_WORLDS_SPEC.md): ogni torneo reale pertinente a 1996/97 o 2026/27 va nella carriera OG, senza distinguere la provenienza PRESET/CAREER. Una sola identita' per torneo/epoca, ma CLUB e NAZIONALI separati. Esempi: NORTH AMERICAN CUP = Gold Cup nazionali, CONCACAF Champions Cup = club; EUROPEAN CHAMPIONSHIPS = Europei nazionali, EUROPEAN CHAMPIONS CUP = Champions club; FIFA Intercontinental Cup moderna distinta da FIFA Club World Cup quadriennale. Includere le supercoppe nazionali di TUTTI i paesi che le disputarono realmente (comprese Africa e Asia), e quelle continentali realmente esistenti nell'epoca: il riferimento 4.1.2 con 10 supercoppe NON e' esaustivo.

START prepara tutti i tornei esistenti nel mondo; il player deve poterli consultare prima della prima partita e indipendentemente dal club/contratto. OFFERTA usa il menu OG read-only, senza catalogo testuale surrogato. Salvataggi dei due mondi separati, CONTINUE ultimo e LOAD selezione, senza cancellazioni o riavvio: il cambio senza restart e' ANCORA DA IMPLEMENTARE. La 0.13.59 tenta la residenza OG tipo-2 delle 3 UEFA iniziali; Libertadores e annualita' successive restano incompleti: vedere tabella stato nella guida, nessun PASS Windows dichiarato.


## Decisione vigente del 14-15 novembre 2026
Per i mondi 1996/97 e 2026/27 **OpenSWOS OG e' l'unica autorita'** per stagione/data, carriera, calendari, partite, risultati, classifiche, coppe, replay, qualificazioni, spareggi e trofei. Le precedenti proposte di un calendario, un simulatore di risultati e un ledger autorevole autonomi LWE sono [STORICO/SUPERATO], mantenute solo per tracciabilita'. LWE puo' aggiungere soltanto dati ed eventi che OG non gestisce gia'; nuovi valori e formule da approvare con l'utente. Le leghe residenti alla 0.13.45_08 non completano ancora le coppe, il save OG globale, lo storico delle fixture e i test pluriennali.

Salvataggi Expanded Windows: `save/AUTO9697.CAR` e `save/AUTO2627.CAR` accanto all'EXE; `AUTOSAVE.CAR` precedente e' solo fallback in lettura, da non cancellare/sovrascrivere. Compilazione e prove Windows restano compito dell'utente.

**Edizione originaria:** 10 novembre 2026 · **Regole aggiornate:** 15 novembre 2026  
**Progetto:** SWOS Expanded, basato su OpenSWOS — Windows 10/11 x64, C/SDL2  
**Ramo consultato:** `dev/0.10` · **HEAD consultato:** `f33041df7204936da22e6a066977b0dc382093b4` · **Versione nel codice:** `0.10.75`  
**Natura:** specifiche di gioco, decisioni, proposte, questioni aperte. **Non è un rapporto di implementazione.**

> Questo documento consolida le decisioni delle conversazioni di progetto recuperabili e i documenti di repository/Library consultabili. Le singole conversazioni non sempre sono recuperabili integralmente: i dettagli non accertati sono esplicitamente marcati come proposte, storici o da verificare, anziché inventati. Il codice verificato e `PROJECT_STATE.md` definiscono lo stato implementativo; `AGENTS.md` regola lo sviluppo; `IMPROVEMENTS.md` registra soltanto cambiamenti effettivi.

## Legenda di stato

- **[APPROVATO]** Decisione vigente dell'utente; costituisce il design da realizzare, non una prova che sia già implementato.
- **[DIREZIONE CONFERMATA]** Obiettivo esplicitato, ma alcuni dettagli richiedono progettazione o verifica tecnica.
- **[PROPOSTA]** Possibilità discussa, non autorizzazione a cambiare gameplay/architettura.
- **[DA DEFINIRE]** Questione aperta da decidere con l'utente.
- **[STORICO / SUPERATO]** Decisione o asset precedente sostituito da indicazioni successive.
- **[CODICE DA VERIFICARE]** Funzione riscontrata in sorgente o documentata storicamente, non necessariamente testata in Windows nella versione corrente.

## 1. Visione, priorità e ambito

**[APPROVATO]** Preservare integralmente le partite di Sensible World of Soccer: fisica, ritmi, controlli, logica a 50 Hz, schermata partita e feeling OpenSWOS. Espandere invece la **carriera manageriale**, l'economia, la vita dei club, il mercato, lo scouting, i giocatori, le coppe e il mondo persistente. Non introdurre netplay senza una nuova richiesta. Non sostituire il motore della partita.

**[APPROVATO — priorità 10/11/2026]** Realizzare una carriera completa e funzionante con **le squadre e i dati già disponibili**. L'ampliamento a moltissimi paesi, livelli e giocatori viene **alla fine** del progetto, non è un prerequisito per il nuovo motore. L'obiettivo è un gioco giocabile, consistente e salvabile prima della crescita del database.

**[DIREZIONE CONFERMATA — futuro]** Mondo globale ampio; richiesta emersa di **fino a otto livelli/leghe per paese** dove esistono, più coppe nazionali, continentali, internazionali e trofei realmente pertinenti. Va valutato paese per paese: le piramidi reali variano; non promettere otto identiche divisioni in ogni nazione. Le regole effettive, licenze e quantità di dati restano da affrontare nella fase finale.

## 2. Living World Engine: tempo e simulazione

### 2.1 Mondo unico persistente

**[APPROVATO]** La carriera si svolge in **un solo mondo condiviso**. Competizioni e squadre continuano a esistere sia mentre l'utente allena, sia durante settimane o mesi di disoccupazione. Una nuova assunzione **non deve riavviare** la stagione del club assunto: risultati, classifica, rosa, calendario, cassa e condizioni rimangono quelli raggiunti alla data di ingaggio. Risultati già calcolati non cambiano aprendo o chiudendo una schermata.

### 2.2 Unità temporale ed eventi

**[APPROVATO — 10/11/2026]** Il tempo mondiale ha un **calendario comune con date**, non una sola settimana come unità indivisibile. I diversi paesi hanno giornate e stagioni non coincidenti; le coppe possono disputarsi infrasettimanalmente. Il motore processa gli eventi **in ordine cronologico**, per esempio:

- Partite di campionato, coppa e recuperi **alla propria data**; classifiche, disponibilità, forma e squalifiche dopo il risultato.
- Offerte di lavoro, mercato e scadenze alle rispettive date e finestre.
- Economia, stipendi, allenamenti e progressione dove appropriato con cadenza **settimanale** o altro intervallo approvato.
- Premi, qualificazioni, promozioni, retrocessioni, fine stagione e ricambio delle generazioni quando si verificano.

**[APPROVATO]** Simulare **le conseguenze rilevanti**, non ogni azione di ogni giocatore. Le altre partite possono essere risolte statisticamente, senza lanciare il motore grafico/fisico; non occorre simulare tutte le strutture e le attività in tempo reale. La squadra gestita riceve il dettaglio completo richiesto dal giocatore. Il mondo deve rimanere consistente e riproducibile, non diventare più leggero a prezzo di risultati modificabili retroattivamente.

**[PROPOSTA TECNICA RACCOMANDATA]** Coda di eventi con data, identificatore, priorità e RNG riproducibile indipendente dalle visite ai menu; elaborazione incrementale all'avanzamento; risultati persistiti e simulazione più sintetica per club lontani. Per incontri allo stesso orario, gli esiti non devono dipendere artificiosamente dall'ordine di esecuzione. Conservare stabilità dei salvataggi e gameplay partita. Determinare i costi di CPU/memoria e la compatibilità con l'attuale codice prima di adottare questa architettura.

### 2.3 Numero della settimana e data in UI

**[APPROVATO — 10/11/2026]** Mostrare il **numero della settimana quasi dappertutto** nelle schermate della carriera, insieme alla data quando utile: calendario, disoccupazione, BUSINESS, mercato, allenamento, rosa, offerte e altri menu manageriali. Non sovrapporlo allo sfondo o al layout: posizione grafica da stabilire nelle singole riprogettazioni.

**[APPROVATO — 10/11/2026]** Il numero WEEK è quello della **stagione calcistica**, non la settimana ISO dell'anno; l'API di calendario conta settimane da una data iniziale configurabile. Anni e stagioni con 53 settimane non devono provocare un reset arbitrario. La fonte del numero deve essere **unica** e derivata dall'orologio mondiale; nessun contatore indipendente per schermata. Il tempo della **partita in corso** non va modificato da questa scelta.

### 2.4 ADVANCE TO NEXT EVENT

**[APPROVATO — 10/11/2026]** Inserire **ADVANCE TO NEXT EVENT** sia in **CALENDAR** sia in **UNEMPLOYED**. L'azione salta il tempo vuoto, ma processa cronologicamente tutti gli eventi intermedi; si ferma prima del successivo evento che richiede una decisione del giocatore (prossima partita, nuova offerta, scadenza importante, decisione di mercato ecc.). Non accetta proposte e non scarta opportunità automaticamente. Conservare un avanzamento manuale per settimana/data quando utile. La dicitura breve `NEXT EVENT` è soltanto una **proposta di etichetta**, non un layout approvato.

**[CODICE DA VERIFICARE]** Nel ramo consultato `Expanded_UnemployedWait()` usa il contatore separato `s_waitWeeks`, e `Expanded_WaitForRehire()` documenta un riavvio di stagione dopo un nuovo incarico. Il nuovo motore deve sostituire con cautela queste limitazioni; non bastano due pulsanti fittizi.

### 2.5 Prove obbligatorie per il tempo

**[APPROVATO COME CRITERIO DI ACCETTAZIONE]** Avanzare un anno intero da disoccupato verificando risultati persistenti; ricevere un'offerta a stagione iniziata e ritrovare la classifica reale; accettare, giocare, salvare e ricaricare senza perdere né alterare gli eventi già elaborati. Testare giorni con più partite, anticipi, infrasettimanali, finestre di mercato e cambio stagione. Il gold test dell'OpenSWOS originale è un riferimento utile per evitare regressioni, non un sostituto del test Windows.

## 3. Carriera e manager

**[APPROVATO]** Profilo con nome, cognome, nazionalità, età iniziale **18–70**, fama iniziale e valuta visualizzata EUR/USD/GBP. Carriera non fermata arbitrariamente dopo vent'anni; ritiro manager previsto a **85 anni**. Selezione del club o partenza da **disoccupato**. La fama del manager influenza accesso alle offerte e gradimento iniziale dei tifosi; livelli discussi: UNKNOWN, AMATEUR, RISING STAR, ESTABLISHED, LEGEND (percentuali indicative in `PROJECT_STATE.md`). Lo stipendio manageriale è annuale.

**[APPROVATO]** Una squadra può ingaggiare e licenziare il manager. BOARD LIKING e FANS LIKING sono distinti e dinamici; risultati, fama, obiettivi, trattative salariali e finanze hanno conseguenze. Nessun sistema gestionale di debito/fallimento del club: una situazione finanziaria insostenibile può condurre all'**esonero**, secondo soglie e tempistiche da calibrare, non a bancarotta giocabile. Dopo esonero, nuova disoccupazione nel **medesimo mondo**, non reset della carriera.

**[APPROVATO]** HALL OF FAME e MANAGEMENT RECORD conservano trofei, club allenati, risultati, denaro guadagnato, storia ed esoneri; conclusione a 85 anni. Definire come persistere e mostrare decenni di storia senza record duplicati.

## 4. Club, chairman e offerte di lavoro

### 4.1 Presidenti

**[APPROVATO 09/11/2026; RICONFERMATO 12/11/2026]** Ogni club ha un **chairman/presidente con nome ed età**. L'età cambia con le stagioni e consente il pensionamento/ritiro. Quando un chairman si ritira, il motore genera o assegna **automaticamente un sostituto nello stesso club**. Età limite e distribuzione delle età restano da scegliere; non introdurre altri campi anagrafici non richiesti come prerequisito.

**[APPROVATO]** Il nome e l'età del chairman devono apparire nella schermata di **trattativa/CLUB OFFER** e in **WELCOME**: l'utente tratta con lui e viene accolto da lui. Il cambio presidente può avere effetti su aspettative/giudizio del Board, ma le formule non sono ancora approvate.

### 4.2 Offerte e negoziazione

**[APPROVATO]** Dati dinamici: nome squadra, nazione/regione, divisione, chairman e sua età, stipendio **annuale**, eventuale capitale disponibile e informazioni utili sul club. Azioni: VIEW SQUAD, VIEW COMPETITIONS, variazione/negoziazione dello stipendio, ACCEPT, DECLINE, BACK. Il livello di fama cambia probabilità e qualità delle offerte. Le anteprime devono mostrare la **rosa e le competizioni attuali**, non una copia immutabile della stagione iniziale.

**[CODICE DA VERIFICARE]** Esiste gestione di richieste salariali e rifiuti successivi in `expanded_career.c`; verificare il comportamento effettivo prima di alterare formule o limiti. Alcune offerte storiche erano ristrette a un piccolo elenco di paesi e squadre; il motore di carriera deve poter operare sul **database esistente completo**, senza hardcoding di cinque paesi come regola finale.

### 4.3 Welcome, Sacked, Record

**[APPROVATO]** WELCOME mostra l'arrivo al club, presidente, Board/Fans e contesto corrente. SACKED è una schermata di esonero chiara e ritorna al mondo persistente. RECORD registra il percorso senza perdere le stagioni precedenti. **[VISUAL OBSOLETE]** Le vecchie immagini di questi menu vanno riprogettate; le funzioni restano da preservare.

## 5. Club, economia e infrastrutture

**[APPROVATO]** La schermata BUSINESS distingue fatturato/spese settimana/anno, incassi da partite, sponsor, diritti TV, personale, manutenzione, trasferte, stipendi giocatori e stipendio manager; mostra BOARD/FANS con percentuali e faccine e il numero dei tifosi. Il salario annuo concordato va contabilizzato **una volta sola**, ripartito coerentemente per le settimane. Proiezioni e consuntivi devono essere distinti.

**[DECISIONI DI DETTAGLIO PRECEDENTI, DA PRESERVARE FUNZIONALMENTE]** I costi di viaggio sono solo quando c'è una trasferta, non una voce duplicata in STAFF; COSTS/MAINTENANCE includono ADMINISTRATION e STADIUM, con STAFF e salari coerenti. Ordine di dettaglio STAFF discusso: colonna sinistra FITNESS, MANAGER, MEDICAL, OFFICE; destra TRAINING, SCOUTS, YOUTH; STAFF, MAINT. e TRAVEL devono riconciliarsi con i totali principali. Il valore annuo di Administration da 45.000 era una base provvisoria, non una formula realistica definitiva. Piccoli stadi previsti da 2.000 posti. Strutture, sponsor e TV reagiscono allo sviluppo del club; i dettagli richiedono bilanciamento.

**[APPROVATO]** Forma e morale di squadra da 0 a 100, faccine canoniche comuni fra i menu; bande rosso 0–39, giallo 40–59, verde 60–100. Valore neutro 65 e modificatore di prestazione massimo ±20% discussi nel progetto; verificare il percorso reale delle formule, soprattutto quello che modifica i parametri della partita senza compromettere il gameplay SWOS. Gol segnati/incassati e cartellini rossi influiscono sul morale; cartellini gialli non lo modificano secondo decisioni precedenti. Recupero/affaticamento della forma tramite riposo/allenamento/gara.

**[GRAFICA SUPERATA]** Il vecchio layout e gli asset BUSINESS devono essere ridisegnati, senza considerare definitiva la loro posizione; conservare per ora dati, callback e comportamenti funzionanti.

## 6. Calciatori, rose, YOUTH / RES / TITOLARE

**[DIREZIONE CONFERMATA 09–10/11/2026]** Evolvere l'uso originale di `TRIAL` verso tre categorie di rosa: **YOUTH** (giovani del club), **RES** (riserve e seconde linee) e **TITOLARE** (prima squadra). Sono **ruoli nella rosa**, non livelli di abilità o potenziale: un veterano può essere RES; un giovane può essere subito titolare. Promozioni e retrocessioni di status sono decisioni/effetti dinamici.

**[DIREZIONE CONFERMATA]** Ogni inizio stagione il settore giovanile può generare ragazzi nella rosa del club; numero e qualità dipendono da struttura/fortuna. Anche club piccoli possono occasionalmente scoprire un campione. I nuovi giovani possono riempire buchi quando la squadra ha pochi giocatori; quando qualcuno si ritira o parte, il club cerca sostituti via YOUTH/RES/mercato.

**[APPROVATO]** Per i ragazzi cresciuti nel club che il manager allena, il **potenziale vero è noto** al manager. Per i calciatori esterni è ignoto finché osservati dagli scout; l'osservazione produce informazioni stimate secondo le capacità dello scouting. Conoscere il potenziale non significa raggiungerlo automaticamente: disponibilità, età, forma, allenamento, infortuni e utilizzo possono influire. Proteggere il potenziale dall'esposizione impropria tramite UI e dati di mercato.

**[APPROVATO COME VINCOLO]** Non assegnare automaticamente giocatori **TRIAL in prestito** alla squadra. Il comportamento originale TRIAL va esaminato nel codice prima di sostituirlo; può rimanere **solo un fallback tecnico d'emergenza** quando la rosa non permette di giocare, evitando corruzione dei vecchi salvataggi. Dove sensato, i giocatori non sotto contratto devono stare nella lista **svincolati**. La compatibilità con i veri marcatori SWOS (`trialist`/`reserve`) richiede verifica specifica: l'utente non ha autorizzato una sostituzione di bit alla cieca.

**[DIREZIONE CONFERMATA]** Ogni giocatore ha età, abilità originali SWOS, costo/valore, contratto e progressione nel tempo. Sviluppo delineato: 16–20 rapido, 21–23 regolare, 24–25 più lento, 26 picco indicativo, 27–30 mantenimento, 31+ declino. Le età sono **curve di design da calibrare**, non soglie rigide universali. Ritiri e nuovi ingressi devono far sopravvivere il mondo per molte stagioni.

**[APPROVATO]** Preservare le abilità e il motore originale; distinguere `PlayerFile.form` originale da eventuali campi Expanded. Il popup giocatore presenta otto statistiche su due colonne, barre e stelle; la UI futura verrà ridisegnata. Le formule per portieri e difensori vanno verificate contro le abilità disponibili.

## 7. Scouting

**[DIREZIONE CONFERMATA]** Scouting a **cinque livelli** con accesso geografico e profondità progressivi: livello 1 nazionale, 3 continentale, 5 mondiale, livelli 2/4 intermedi. Al livello 1 soltanto informazioni molto limitate (es. stelle); al livello 3 alcune statistiche; al livello 5 anche stime sul potenziale. Gli sconosciuti mostrano `???` dove i dati non sono stati scoperti. L'esatta lista di campi e il tempo necessario per osservarli richiedono approvazione sul menu definitivo. **Il potenziale dei propri giovani resta noto** e non è soggetto a scouting.

**[PROPOSTA]** Ricerche non globali a ogni frame: generare osservazioni al termine di periodi/eventi, memorizzando l'informazione acquisita e il grado di confidenza. Nessuna visualizzazione illegittima dei valori nascosti tramite il mercato.

## 8. Trasferimenti e contratti dei giocatori

**[APPROVATO 14/11/2026 — DA IMPLEMENTARE]** Anche a finestra chiusa si possono negoziare, acquistare e firmare accordi per giocatori sotto contratto, ma l'arrivo effettivo e l'iscrizione nella nuova rosa avvengono **solo all'inizio della prossima finestra applicabile**. Prima di tale data il calciatore resta nel club d'origine e può giocare per esso. Gli svincolati possono essere ingaggiati e schierati durante tutto l'anno. Identico principio per club IA e giocatore. Finestre paese/competizione, pagamenti, salari e annullamenti sono **DA DEFINIRE**, senza date arbitrarie. **PRIORITÀ:** finire il mondo OG e VIEW WORLD in sola lettura; nessun nuovo sistema di trasferimenti implementato da questa decisione.


**[APPROVATO]** Conservare le funzioni trasferimenti SWOS esistenti, comprese lista trasferimenti, offerte, mercato, vendite e scambi utili, ma espandere la carriera in modo coerente. Un giocatore in lista è più facile da acquistare, fuori lista più difficile; il manager può mettere/togliere dalla lista i propri calciatori. I trasferimenti cambiano realmente le rose delle squadre AI, non solo la squadra gestita. Gli svincolati entrano in un bacino comune.

**[DIREZIONE CONFERMATA]** Una conferma di trasferimento deve rendere leggibili **BANK BEFORE/AFTER**, stipendio annuale e impatto previsto sul Board; valore di mercato distinto dal prezzo effettivamente richiesto/pagato. Budget e scelte dei club AI devono essere coerenti, senza simulazione finanziaria completa per tutti i club.

**[DA DEFINIRE]** Prestiti volontari, durata contratti, clausole, bonus, calendari reali dei mercati, contrattazione AI, nazionali/limiti rosa, punteggi di scouting. Nessun sistema di prestiti TRIAL automatici. Lo schermo TRANSFERS (riferimento storico `TRANSFER 1`) **non ha più layout finale** e sarà ridisegnato.

## 9. Training, squad, match stats

**[APPROVATO]** TRAINING deve avere conseguenze su forma, recupero e sviluppo coerenti con l'età e le strutture; il design esatto degli esercizi non è ancora approvato. I cambi tattici, gli scambi di squadra e la formazione non devono interrompere la navigazione SWOS né alterare la fisica della partita.

**[APPROVATO]** SQUAD usa joypad/mouse/tastiera; popup +INFO con X (anche clic destro nel contesto previsto), apertura/chiusura immediata, B/ESC prima chiudono il popup; non aprirlo automaticamente quando cambia il focus. Preservare la navigabilità di BACK/EXIT e le statistiche funzionanti. MATCH STATS rappresenta squadre, punteggio, FORM/MORALE con le faccine canoniche; STOP/EXIT MATCH deve avere conferma funzionante. I layout grafici storici non sono più approvazioni finali.

## 10. Competizioni, nazioni e regolamenti

**[APPROVATO]** Simulare coerentemente campionati, coppe nazionali e internazionali, qualificazioni, promozioni, retrocessioni, risultati e trofei nelle competizioni già presenti. Non interrompere campionati non direttamente giocati dal manager. Quando entreremo nella fase database mondiale, replicare **quanto possibile i sistemi reali paese per paese**: multipli gironi, play-off/out, split, fasi stagionali diverse, quote continentali, leghe con periodi differenti. Il desiderio di otto divisioni è un **obiettivo massimo discusso**, non obbligo che ogni nazione abbia otto leghe identiche.

**[CODICE DA VERIFICARE]** L'OpenSWOS di base contiene infrastrutture per world pack e competizioni; `sorgenti/docs/WORLD_2026_27.md` descrive dati, limiti e vincoli di licensing, ma non autorizza a importare o distribuire nuovi database. In questa fase: **non allargare**, mettere in funzione ciò che c'è.

**[PROPOSTA]** Generare risultati delle altre squadre tramite forza relativa, rosa disponibile, forma, casa/trasferta e randomizzazione deterministica; evitare discrepanze fra classifiche mostrate, qualificazioni e storico. Indipendenza dell'ordine quando partite condividono la stessa data. Livelli di simulazione dettagliata per area vicina e sintetica per area lontana sono un'ottimizzazione, non un cambio d'importanza dei risultati.

## 11. Frontend, interazione e direzione grafica

### 11.1 Decisione grafica definitiva del 10/11/2026

**[APPROVATO, PREVALE SUI VECCHI VINCOLI GRAFICI]** Soltanto **TITLE**, **NEW CAREER**, **LOADING** hanno una grafica *attualmente finale*. Per **tutti gli altri menu e sfondi**, le vecchie approvazioni sono **[STORICO / SUPERATO]** e il design deve essere rifatto quando si affronta la schermata. Questo non cancella automaticamente gameplay, callback o informazioni già approvate.

**[APPROVATO]** Contrassegnare gli asset grafici dei menu pre-switch come **OBSOLETI — da sostituire**. Il **prossimo BAT di pulizia** potrà eliminarli **solo quando i riferimenti attivi nel codice/build e i fallback saranno stati rimossi o sostituiti e verificati**, altrimenti produrrebbe menu vuoti. Nessuna cancellazione di massa senza controllo delle dipendenze. Non rinominare asset caricati da sorgenti solo per cambiarne lo status. Conservare `.gitkeep` e gli asset condivisi ancora necessari. Per il momento la dichiarazione è **di stato grafico**, non una cancellazione dal repository.

### 11.2 Standard grafico dei nuovi menu

**[APPROVATO]** Pixel art illustrata Expanded, non copia dei menu originali SWOS. PNG lossless **RGB 8-bit** per sfondi e **RGBA 8-bit trasparenti** per sprite/loghi/pulsanti/pannelli; preservare BMP/RAW/PNG storici durante la migrazione. Sfondi nuovi 1672×941 in fase generazione e **solo crop a 1664×936**, senza ridimensionamento se non autorizzato; frontend logico 1920×1080 previsto (non interamente realizzato); uscita di riferimento 3840×2160. Mai aggiungere sfondi illustrati autonomamente ai menu se non richiesti.

**[APPROVATO]** Tenere **sfondo, logo canonico, pannelli statici, testo dinamico, focus/hover** in livelli separati. Prima del PNG definitivo produrre un PNG **dimostrativo** con valori plausibili, scritte con font e larghezze realmente usabili dal renderer e coordinate misurate. Dopo approvazione, produrre il PNG pulito identico senza valori dinamici. Consultare `sorgenti/docs/FONTS.md`: misurare tramite funzioni reali (`GetTextWidth`, `DrawMenuText` ecc.); font SWOS small 6 px, big 8 px, margini liberi simmetrici e due bordi da 1px: righe minime rispettivamente 10/12px. Etichette e valori massimi non devono toccarsi né essere troncati.

**[APPROVATO]** Nuovi pulsanti blu frontali, spessore 3D sotto/destra, focus giallo calcolato dalla silhouette alpha e non incorporato nel PNG; animazione pressione con leggero bounce **senza resize**. Le azioni devono essere accessibili con joypad, tastiera e mouse; BACK/EXIT sempre raggiungibili; X per +INFO solo dove l'azione esiste, B per BACK, A per SELECT. Hover/pop-up immediati e senza obbligo di muovere il mouse. Il testo di versione è un overlay dinamico e non modifica l'immagine dello sfondo.

**[APPROVATO]** Development Windows: vera finestra SDL2 **massimizzata e decorata**, taskbar/Alt+Tab, anche nel ciclo menu→partita→menu, non borderless. Release: fullscreen desktop reale. Alt+Invio mantenuto; `SWOS_Expanded.exe` e BAT/SH coerenti. Non imporre nuovi toolchain o riscritture SDL3/Godot senza approvazione.

### 11.3 Le tre schermate finali

- **TITLE [FINALE GRAFICO]** Sfondo `assets/title/title_background_approved_2026-11-08.png` (1664×936), logo trasparente canonico sopra (riferimento di posizione x920,y17,620×227, da verificare sul codice attuale); pulsanti primo livello CAREER/CONTINUE CAREER, HALL OF FAME, CLASSIC MODES, OPTIONS, EXIT GAME, più FULLSCREEN/WINDOWED separato. CLASSIC MODES **resta nel primo livello** e apre il sottomenu con CREDITS, LOAD OLD COMPETITION, EDIT TACTICS, EDIT CUSTOM TEAM, HIGHLIGHTS, FRIENDLY, DIY COMPETITION, PRESET COMPETITION, SEASON, BACK. Non perdere CONTINUE, REPLAY, SAVE delle competizioni originali. Sprite continue cambia a verde **a runtime** con testo bianco. Le correzioni minime al pixel e la barra LOADING restano da verificare nella build corrente; non significa rifare la grafica.
- **NEW CAREER [FINALE GRAFICO]** Illustrazione e layout approvati a scala 88,5%, font bitmap diritto a pixel interi; flags vicino alla nazionalità, FAME diretto, età 18–70, titolo/ruolo, checkbox disoccupato e pulsanti START/BACK/altre azioni attive. I bug testati successivamente sono correzioni, non riprogettazione autorizzata.
- **LOADING [FINALE GRAFICO]** Illustrazione approvata e progress bar basata su progresso reale, piena al 100% senza spazio vuoto finale; separare sfondo, barra e testi. Evitare pause artificiali introdotte solo per screenshot.

### 11.4 Menu da riprogettare

**[APPROVATO COME STATO]** CALENDAR, UNEMPLOYED, CLUB OFFER, WELCOME, SACKED, MANAGEMENT RECORD, BUSINESS, SQUAD, MATCH STATS, TRANSFERS, TRAINING e gli altri sottomenu non inclusi nelle tre eccezioni sono **graficamente provvisori/obsoleti**. Il nuovo layout si decide di volta in volta; **non** trasferire automaticamente il layout vecchio come vincolo. Le funzioni, i dati e i controlli esistenti restano invece riferimenti da conservare o migliorare.

## 12. Architettura, salvataggi e verifiche

**[APPROVATO]** L'implementazione parte dalle API e dai sistemi OpenSWOS reali. Nuove strutture devono avere ID stabili per club/giocatori/eventi/presidenti; l'orologio non deve creare campionati duplicati. La persistenza deve includere data globale, eventi già elaborati, risultati, rose e movimenti, chairman attivi, sviluppi e decisioni; prevedere versione schema e migrazione compatibile con i salvataggi esistenti, quando tecnicamente possibile. Verificare recupero dopo rientro da match e caricamento save.

**[PROPOSTA, NON APPROVATA COME MIGRAZIONE]** SQLite, una riscrittura del renderer, file database esterni, AI avanzata, plugin o dati commerciali non sono prerequisiti. Preferire modifiche mirate al C attuale, piccoli test golden/deterministici e test Windows/joypad del gioco completo. Non confondere test isolati SDL/C, asset approvato, commit GitHub, build Windows e prova in Windows.

## 13. Piano di lavoro per avere un gioco finito con il database attuale

1. **Tempo mondiale**: inventario degli hook data/season/match, fonte unica della data; registrare event ledger e aggiornare tutte le competizioni già disponibili; test di un anno da disoccupato.
2. **Carriera coerente**: ingresso a stagione in corso senza reset, trasferimento di calendario/risultati, salvataggi compatibili, offerte non duplicate.
3. **Chairman/offerte**: identità, età, pensionamento/successione, nome/età in CLUB OFFER e WELCOME; rivedere negoziazione. Progettare i PNG demo prima della grafica pulita.
4. **NEXT EVENT** in CALENDAR e UNEMPLOYED, con avanzamento normale alternativo e settimana/data coerente quasi ovunque. Il pulsante si collega a eventi veri, non a un contatore fittizio.
5. **Giocatori/mercato**: YOUTH–RES–TITOLARE, potential e scouting, ritiri, svincolati, trasferimenti AI; fallback sicurezza quando mancano giocatori.
6. **Finanze, training e club**: test contabilità, costo staff/stadio, facilities, forma/morale e sviluppo.
7. **Collaudi e UI progressive**: test salvataggi su stagioni, controller, menu/partita/menu e riprogettazione concordata delle schermate non finali.
8. **Solo alla fine**: estendere database, nazioni, campionati/divisioni, coppe e regole specifiche.

L'ordine è una **roadmap proposta** coerente con l'ultima priorità; ogni cambio strutturale di motore richiede comunque approvazione della specifica tecnica prima del codice.

## 14. Questioni aperte, da non risolvere da soli

- Convenzione WEEK **risolta: settimana della stagione**. Rimangono da definire le date di inizio/chiusura delle competizioni nei vari paesi e gli eventuali campionati a cavallo di anni differenti.
- Identità persistente dei presidenti; età iniziali, età massima, frequenza/criteri di successione; impatti di un nuovo chairman.
- Specifiche esatte di TRAINING, aumento permanentemente delle abilità e modello di infortuni.
- Equilibrio e durata di contratti/prestiti; parametri AI e scouting che mascherano valori.
- Comportamento degli slot `TRIAL` e `RES` nelle strutture SWOS esistenti e compatibilità dei salvataggi.
- Quanto mostrare negli screen non ancora rifatti, e a quale posizione ancorare WEEK/date.
- Divisioni e trofei per ciascun paese quando in futuro si amplia il database; fonti/licenze dei dati.
- Implementazione degli eventi infrasettimanali con lo scheduler SWOS esistente e test di regole reali.

## 15. Tracciabilità e documenti correlati

- `AGENTS.md`: obblighi, workflow, gerarchia approvazioni, asset e Git.
- `PROJECT_STATE.md`: stato del codice e verifiche pendenti; non duplicare l'intero GDD come se tutto fosse implementato.
- `IMPROVEMENTS.md`: soltanto funzionalità aggiunte, bug risolti e test realmente effettuati.
- `docs/DEVELOPMENT_WORKFLOW.md`: procedura Windows, asset, pacchetti e sincronizzazione via GitKraken.
- `docs/OBSOLETE_UI_INVENTORY.md`: classificazione dei vecchi asset grafici e condizioni per il prossimo BAT di pulizia.
- `sorgenti/docs/FONTS.md`: font/verifica degli ingombri UI.
- `sorgenti/docs/WORLD_2026_27.md`: storico e limiti della tecnologia world OpenSWOS, **non** mandato a importare automaticamente un database.
- Storico conversazioni di progetto e documento Library `SWOS_EXPANDED_PROJECT_STATE.md`: fonti di decisioni precedenti. Se emerge un conflitto con un'altra fonte attiva, discuterlo con l'utente prima di cambiare la parte controversa; registrare la scelta nel commit successivo autorizzato.

**Stato di questa consegna:** documentazione; nessuna modifica a codice, asset, build o repository. Nessun test Windows eseguito. I riferimenti di commit/versione fotografano lo stato consultato, non una build prodotta in questa consegna.


## 16. Prima realizzazione tecnica del tempo — 0.13.01

- [CODICE, DA TESTARE WINDOWS] Nuova API `expanded_world_time` per date gregoriane 1900–2200, indice del giorno, avanzamento monotono e settimane a partire da una **data completa fornita dal chiamante**. Non fissa l'anno 1996: funziona anche con 2026/27 e futuri aggiornamenti.
- [CODICE, DA TESTARE WINDOWS] La data visualizzata negli attuali menu manageriali usa `World_FirstYear()` per rispettare il pacchetto di squadre scelto, senza alterare le chiavi settimanali delle partite o il conto economico. Nell'attuale UNEMPLOYED la settimana visualizzata riparte dalla stagione di riferimento; questo **non** significa che le partite del mondo progrediscano durante la disoccupazione.
- [NON ANCORA IMPLEMENTATO] Lo scheduler degli eventi, il tempo globale persistito, l'ingresso a stagione iniziata e ADVANCE TO NEXT EVENT restano per i passi seguenti.
- [TEST ISOLATO] Roundtrip di 109.938 date, casi 1996/2026/2034, bisestili, settimane e avanzamenti negativi vietati, eseguiti con GCC locale; **nessun test Windows completo**.

## 17. Progresso 0.13.02 e metodo di diagnosi

**[APPROVATO]** Per la vera simulazione, **NON creare una squadra fittizia del manager** al principio della carriera. Il mondo vive indipendentemente dall'allenatore: competizioni/fixture devono avanzare anche in stato UNEMPLOYED, e le loro classifiche devono essere visibili in anteprima prima della firma.

**[CODICE DI SUPPORTO, NON ANCORA MOTORE MONDIALE]** Il registro `ExpandedWorldLedger` conserva incontri datati, ID, risultato, controllo anti-duplicazione, classifiche con punti/vittoria configurabili e percorso comune per risultati automatici e umani. Nei test la callback produce gol *artificiali*: la vera risoluzione deve chiamare le funzioni SWOS originali e sincronizzare lo stato delle competizioni.

**[IMPLEMENTATO NEL CODICE / WINDOWS DA VERIFICARE]** F12: testo fotografabile contenente world tag, build, data **indicativa**, stato carriera/disoccupato e gli espliciti indicatori **WORLD SIM NOT ACTIVE**, **MIDSEASON JOIN NOT ACTIVE**, **WORLD SAVE NOT ACTIVE**. F9: PNG finale con overlay. Tasti 1/2/3: debug WIN/LOSE/SACK, disabilitati al riavvio e non persistiti. Non introdurre nuovi asset o layout UI per la diagnostica. Modalità win/lose modificano il risultato *registrato* dalla logica SWOS, non la fisica della partita.

**[DA IMPLEMENTARE]** Generazione degli eventi dalle competizioni vere (date specifiche per paese), simulazione durante UNEMPLOYED, serializzazione del mondo, anteprima classifica delle offerte e riassunzione a stagione in corso senza `InitCareer` che resetta tutto. Sono criteri di accettazione della Fase 1–2, non risultati ottenuti nel commit.


## 18. Prova del mondo attivo 0.13.03

**[CODICE PILOTA, NON FASE 1–2 COMPLETA]** Il mondo non dipende più concettualmente dal club dell'allenatore: una prima coda di risultati offscreen con ID reali delle squadre e un modello di forza della rosa fa avanzare campionati mentre si è disoccupati. Ambito temporaneo di test: i cinque paesi già caricabili con sicurezza dalle offerte di lavoro, massimo 26 squadre per divisione, due stagioni, fixture round-robin provvisorie. In `VIEW LEAGUE` prima della firma è visibile la classifica del campionato-pilota.

**[DA INTEGRARE]** La classifica pilota non ha ancora aggiornato gli slot nativi `Career_*Competition*` e la chiamata `InitCareer()` al momento della firma ricomincia tuttora il calendario OpenSWOS. Non utilizzare l'esito della prova per dichiarare pronta l'assunzione durante la stagione. La sincronizzazione degli incontri, il calcolo coerente delle coppe e la persistenza nei save sono il prossimo lavoro con test di regressione.

**[CONFERMATO]** Nessun cambiamento a UI di debug F12, titolo, New Career, Loading, grafica e fisica partita. Lo scheduling provvisorio non è l'approvazione dei calendari reali.


## 19. Avvio 1° luglio, precampionato mondiale e prezzo dei biglietti (APPROVATO, 11/11/2026)

### 19.1 Anno calcistico e inizio carriera

- **[APPROVATO, sostituisce la proposta «1° giugno»]** Per le carriere europee, **1° luglio** è la data iniziale predefinita della stagione manageriale, inclusa la disoccupazione. Il mese di **giugno continua a esistere nel calendario del mondo**: finali, contratti in scadenza, licenziamenti, preparazione del mercato e attività societarie possono cadervi senza richiedere l'avvio del giocatore.
- **[APPROVATO]** Campionati con stagioni a cavallo di date diverse (es. calendari annuali) usano **inizio stagione e prima giornata configurabili per competizione/paese**, non una data mondiale fissa per tutte le leghe. Distinguiamo **inizio carriera/contratti**, **inizio precampionato** e **prima giornata di competizione**; la data 1° agosto del pilot attuale è **provvisoria** per il calcio europeo, non una regola universale.
- **[APPROVATO]** Le settimane mostrate in UI sono settimane **della stagione** e partono dalla data di avvio del calendario pertinente. Il tempo deve rimanere monotono, anche nei passaggi UNEMPLOYED → CLUB e CLUB → UNEMPLOYED, senza resettare risultati e campionati.

### 19.2 Precampionato per TUTTE le squadre

- **[APPROVATO]** Il precampionato esiste per **tutte le squadre**, non solo per quella allenata dal giocatore. Le squadre controllate dall'AI ricevono preparazione e partite amichevoli gestite a livello di eventi leggeri, con conseguenze su forma, fatica, infortuni, morale, finanze e disponibilità dei giocatori. **Non simulare frame per frame** le amichevoli AI; risolverle per data, avversario e condizioni usando risultati deterministici/riproducibili.
- **[APPROVATO]** Il giocatore può **organizzare le amichevoli**: proporre data e squadra avversaria, casa/trasferta, costo o compenso, calendario e disponibilità dell'altra squadra. Potrà disputarle o simularle. L'importanza, la forza avversaria, la distanza, il prezzo dei biglietti e la reputazione influiscono su richiesta, spese, pubblico e incassi.
- **[APPROVATO]** Amichevoli e allenamento comportano **benefici e rischi**: preparazione fisica, forma, morale e intesa migliorano secondo intensità e risultati; eccessivo carico causa fatica e probabilità di infortunio. Infortuni e costi non devono essere falsi o applicati due volte; il modello AI e quello della squadra umana devono condividere lo stesso saldo mondiale persistente.
- **[REGOLA DI INTEGRAZIONE]** Nessuna amichevole deve occupare indebitamente uno slot di campionato o alterare la fisica SWOS; il calendario degli eventi tiene separati amichevoli, campionati e coppe. Quando il mondo passa a campionato in corso, ritroviamo lo stesso storico.

### 19.3 Prezzo biglietti in CLUB

- **[APPROVATO]** Nella futura schermata **CLUB** il manager può impostare il prezzo dei biglietti delle partite casalinghe (conferma esplicita, joypad/tastiera/mouse). Il prezzo è un dato **persistente del club** e può essere differenziato per categorie di evento quando avremo il relativo modello.
- **[APPROVATO]** Domanda decrescente al crescere del prezzo: un biglietto più caro aumenta il ricavo per spettatore ma **riduce l'affluenza potenziale**, soprattutto se il morale dei tifosi è basso. Supporters/FANS LIKING, reputazione del club, importanza e qualità dell'avversario, risultati recenti, competizione e capacità dello stadio modificano la curva di domanda. I tifosi entusiasti sono più disposti a pagare; prezzi palesemente eccessivi possono anche penalizzare la soddisfazione dei tifosi.
- **[PROPOSTA DI BILANCIAMENTO, non definitiva]** Prezzo consigliato iniziale `P0`, modificabile entro una fascia proporzionale, e fattore d'affluenza del tipo `max(minimo, 1 - sensibilita_tifosi * (prezzo/P0 - 1))`, con sensibilità più alta a FANS LIKING basso. `spettatori = min(capienza, tifosi_disponibili * domanda)`; `incasso_lordo = spettatori * prezzo`; `incasso_netto = incasso_lordo - spese_organizzazione` (salvo diversa ripartizione accordata nelle amichevoli). Soglie, valuta e intervallo devono essere misurati e approvati durante il bilanciamento.
- **[REQUISITO]** CLUB mostra prezzo attuale, prezzo di riferimento, **stima spettatori e incasso** per la prossima partita casalinga; l'incasso effettivo deriva **da un unico conteggio** e confluisce in BUSINESS/finanze, senza duplicare `_gateReceipts` originali. Per le partite fuori casa, nessun incasso biglietti a favore del club salvo accordo separato. Definire quando il prezzo viene fissato per ogni evento, evitando variazioni dell'ultimo istante che sfruttano una previsione già calcolata.
- **[APPROVATO]** Anche i club AI applicano una politica prezzi coerente nel mondo persistente; il comportamento non richiede un menu per l'AI.

### 19.4 Unificazione CLUB / SQUAD e classifiche

- **[APPROVATO]** Un solo modello dati mondiale per **classifica, calendario e rosa**: l'offerta di lavoro e la carriera impiegata devono leggere la stessa fonte, con **lo stesso componente UI** (solo la visibilità delle azioni cambia). La consultazione della rosa prima della firma resta *read-only*.
- **[VINCOLO]** Non fare una copia soltanto visiva della classifica pilota nelle competizioni native; la stessa partita deve essere consumata **una sola volta** dal calendario nativo e da quello del mondo, con salvataggio coerente. Il motore OpenSWOS deve conservare ogni fixture, coppa, risultato e stato squadra.

### 19.5 Stato della realizzazione 0.13.05

- **[CODICE PILOTA, da testare Windows]** L'orologio del disoccupato e del pilota mondiale parte dal **1° luglio**; quattro date di amichevoli AI in luglio (7/14/21/28), eventi aggregati e risultati di campionato provvisori da agosto. I risultati e gli effetti del precampionato AI **non sono ancora trasferiti** a rose, infortuni e finanze delle partite native: non scambiare i loro contatori per veri stipendi, salvataggi o indisponibilità.
- **[NON ANCORA IMPLEMENTATO]** L'ingresso nativo effettivo a metà campionato, il salvataggio del mondo, gli incontri amichevoli giocabili per il manager, il prezzo biglietti regolabile in CLUB e la fusione di CLASSIFICA/SQUAD in una sola UI: **restano punti di lavoro**, non funzioni consegnate da questa build.


## 20. Selettore del mondo calcistico in OPTIONS (APPROVATO — 11/11/2026)

- **[APPROVATO — UI DA IMPLEMENTARE]** Aggiungere in **OPTIONS** la voce **FOOTBALL WORLD**, con scelta fra **1996/97** (`9697`) e **2026/27** (`2627`). La scelta deve essere disponibile **prima di iniziare una nuova carriera**, senza richiedere la modifica manuale di `SWOS.ini`.
- **[REQUISITO]** Utilizzare `World_Set` / `World_Current` e le fonti di dati già previste dal codice; non inventare un mondo 2026/27 limitandosi a cambiare le date. Verificare che il pacchetto necessario sia installato e compatibile: in caso contrario disabilitare la scelta o mostrare chiaramente perché non è disponibile. Rispettare le limitazioni di licenza e distribuzione dei dati.
- **[REQUISITO]** Una carriera salvata mantiene il mondo e la versione dei dati con cui è stata creata. Non cambiare al volo il mondo di un salvataggio già avviato; evitare la perdita di stato, le incompatibilità tra squadre e i reset silenziosi. Se il cambio necessita riavvio o ricaricamento dati, comunicarlo nella UI prima di applicarlo.
- **[REQUISITO UI]** Navigazione completa mouse, tastiera e joypad, focus dinamico coerente con la nuova UI Expanded. Solo TITLE, NEW CAREER e LOADING sono graficamente finali: il layout definitivo di OPTIONS richiede una proposta e approvazione prima di creare nuovi asset.
- **[STATO]** Non ancora implementato in OPTIONS. Attualmente il motore legge `--world`, `SWOS_WORLD` oppure `[general] world` in `Documenti/SWOS/SWOS.ini` (default `9697`).

### Evidenza dalla prova 0.13.05

- **[TEST WINDOWS DELL'UTENTE — screenshots 11/11/2026]** Prima delle giornate ufficiali, VIEW LEAGUE mostra **4 FRIENDLIES (PILOT)** e `PL = 0` per Barnet. Dopo l'avanzamento, mostra una classifica con `PL = 3` per Hartlepool United. Il calendario pilota e l'anteprima delle offerte sono quindi osservabili in gioco; non dimostra ancora l'integrazione con `InitCareer()`, la persistenza in `.CAR` o gli effetti delle amichevoli sul club nativo.


## 21. Sponsor preesistenti e continuità dei contratti (APPROVATO — 11/11/2026)

- **[APPROVATO]** La squadra esiste economicamente **prima dell'ingresso dell'allenatore**. Una squadra normalmente ha già uno o più sponsor attivi, con compenso e **scadenza contrattuale preesistenti**. Un club in gravissima difficoltà può averne zero. Non inventare automaticamente un nuovo contratto soltanto perché il manager viene assunto.
- **[APPROVATO]** Ogni contratto deve continuare nel tempo anche mentre il manager è disoccupato, e restare valido dopo un licenziamento o cambio allenatore. Alla scadenza il manager può **rinnovare** o cercare un accordo sostitutivo. Gli introiti devono essere contabilizzati una sola volta nella stessa contabilità CLUB/BUSINESS.
- **[APPROVATO]** Non estendere ulteriormente per ora il modello di sponsorizzazioni (no sotto-categorie, pubblicità dinamiche, trattative multiple obbligatorie o altri sistemi non richiesti).
- **[NON IMPLEMENTATO]** Il pilota mondiale attuale non conserva ancora sponsor e contratti propri di ogni club, né le scadenze in un salvataggio mondiale condiviso.

## 22. Forma individuale e allenamento collettivo (PROPOSTA DA DISCUTERE, non approvata)

- **[IDEA APERTA DELL'UTENTE]** Valutare la **forma per ogni giocatore** senza obbligare il manager a impostare manualmente una routine settimanale individuale per tutta la rosa.
- **[PROPOSTA]** Separare **forma individuale** (stato atletico e prestazionale di ciascun calciatore, un piccolo valore per giocatore) dalla **forma collettiva** (andamento recente della squadra), mantenendo **allenamenti di gruppo** come interfaccia normale. Eventuale lavoro individuale sarà opzionale e limitato alle eccezioni.
- **[VINCOLO]** Prima di decidere controllare la semantica del campo `PlayerFile.form` già usato dalle competizioni originali; evitare duplicazione e salvataggi incompatibili. Nessuna implementazione finché la proposta non viene approfondita e approvata.

## 23. Esperimento 0.13.06 — ingresso in corsa nel calendario nativo (CODICE PROTOTIPO)

- **[PROTOTIPO IMPLEMENTATO, TEST WINDOWS PENDENTE]** Dopo l'accettazione di un'offerta nel **primo anno**, il calendario della carriera viene inizializzato normalmente, poi l'automazione usa le azioni native `NEXT MATCH`/`VIEW RESULT` per far disputare le settimane precedenti all'assunzione, fino al primo `sortKey` futuro rispetto alla data di firma. I risultati e la classifica dentro CARRIERA dipendono **dallo stesso motore OpenSWOS** che gestisce gli incontri giocati dopo l'assunzione: non sono punti copincollati da una schermata.
- **[VINCOLO ESPLICITO]** Il mondo pilota mostra ancora un calendario **round-robin provvisorio**, diverso da quello nativo per date, ordine e possibili risultati: **la classifica pre-firma può differire da quella all'ingresso**. La piena unificazione della fonte dati (classifiche, rosa, fixture, coppe e salvataggi) NON è implementata in questa prova. Non nascondere la discrepanza né considerarla approvata.
- **[TUTELA]** Le partite antecedenti alla firma non accreditano risultati, statistiche allenatore o esoneri al manager appena assunto; gli effetti societari nativi seguono la loro logica. L'avanzamento ha guardie di sicurezza e non procede implicitamente oltre la fine stagione. F12 e fisica partita invariati.
- **[LIMITI]** Solo per una data di firma entro le **52 settimane** dall'avvio del primo anno; ritiri/riassunzioni dopo più anni e stati globali delle coppe di paesi lontani non sono ancora unificati. Il salvataggio mondiale del pilota non è disponibile. Debug e stato del prototipo vanno verificati prima di dichiararlo funzionante.


## 24. Registro affidabilità strumenti ChatGPT (11/11/2026)

Questo registro documenta incidenti **segnalati dall'utente** durante lo sviluppo, separandoli dai bug di SWOS Expanded. Non attribuisce cause tecniche senza evidenza.

| Data (locale) | Errore riportato | Conseguenza | Recupero/verifica |
|---|---|---|---|
| 11/11/2026 (ora esatta non verificata) | `Connection interrupted` | Conversazione tecnica interrotta mentre si lavorava al Living World Engine | Recuperati requisiti e stato del codice dal repository; eventuali modifiche non committate nella sessione precedente non verificabili da GitHub |
| 11/11/2026 (ora esatta non verificata) | `Unknown error` | Nuova interruzione della stessa sessione, con rischio di lavoro non trasferito | Ripresa da HEAD GitHub dopo il push dell'utente; nessuna diagnosi della piattaforma confermata |
| 11/11/2026 (ora esatta non verificata) | `Connection interrupted` (nuova segnalazione dell'utente) | Interruzione mentre si preparava il collegamento dei risultati pilota alle partite native e i relativi test | Ripresa tramite commit HEAD `a0e01ee` su `dev/0.10`, controllo delle fonti e continuazione delle modifiche; causa tecnica e durata non note |
| 11/11/2026 (ora esatta non verificata) | `error in message stream` (ulteriore segnalazione dell'utente) | Messaggio interrotto durante la pianificazione dell'unificazione Living World Engine | Recuperata la conversazione e ripreso il lavoro da `0.13.14`; causa, durata e dati persi non verificabili |

**Metodo per le prossime segnalazioni:** registrare data/ora se note, messaggio esatto, attività in corso, commit/branch, modifiche potenzialmente perse, recupero e tempo perso solo se misurabile. Nessuna frequenza o durata inventata.

### Avanzamento successivo al push del 11/11/2026

**0.13.07 — correzione limitata (codice, non collaudato Windows):** in presenza di una classifica pilota valida con almeno tre giornate disputate, le offerte casuali della divisione più bassa selezionata sono limitate alla metà inferiore della classifica. Prima di tre partite o in assenza di dati validi resta la selezione precedente. Questa modifica **non** unifica risultati/campionato pilota e calendario nativo né rende già affidabile l'assunzione a metà stagione.

**Ancora da implementare e verificare:** unico Living World Engine persistente, eliminazione dei calendari duplicati solo dopo unificazione reale, offerte collegate agli esoneri registrati degli allenatori AI, conteggi effettivi del database (players/managers/chairmen/entries e MB) in F12, regressioni Windows e salvataggi. Le note 0.13.06 restano valide per i limiti del prototipo.


### 0.13.08 — verifica integrità del calendario nativo prima dell'ingresso

**[CODICE DI SICUREZZA, DA TESTARE WINDOWS]** Il recupero delle partite precedenti alla firma verifica l'intero keycache nativo (lunghezza entro 8192 byte, stride valido, ordine dei sortKey e presenza di fixture sia antecedenti sia successive al cutover). Prima era sufficiente trovare una fixture futura per attivare il recupero: una memoria incoerente poteva essere letta oltre il buffer o simulata in modo errato. Le condizioni di arresto riportano diagnostica esplicita a stderr.

**[ATTENZIONE — OBIETTIVO PRIMARIO ANCORA APERTO]** Questa è una protezione per la prototipazione, non una soluzione del bug segnalato: la classifica pilota pre-firma e la competizione nativa post-firma restano due origini diverse. Non è stato verificato l'ingresso riuscito su Windows con calendario, risultati, coppe, finanze e salvataggi invariati. Il requisito definitivo è costruire una sola simulazione persistente e riutilizzare i suoi stati nelle schermate prima/dopo il contratto, senza ri-simulare o ricopiare punti.


## 25. Midseason native month-phase correction (0.13.09 — 11/11/2026)

**[CODICE, TEST WINDOWS PENDENTE]** Il vecchio `Expanded_MidseasonJoin_Cutoff`
confrontava settimane trascorse da agosto (valori 0–51) con `sortKey`
dei calendari OpenSWOS, che rappresentano **ottavi di mese**
(`beginMonth`/ `endMonth`, gennaio=0, marzo=16, agosto=56,
+0x60 dopo il nuovo anno). Non erano unità compatibili.
Ora la data civile calcola la fase nativa, con casi di
regressione per settembre, marzo, maggio e giugno. Non è
una riscrittura delle fixture; il raggiungimento esatto dei
round richiede ancora un test reale sulla timeline.

**[F12 ESTESO]** La schermata diagnostica espone database world tag,
club, divisioni, 16 slot giocatori per club importato dal pilota,
fixture totali e disputate, memoria RAM del pilota, cache e fase
native e stato dell'autocatchup. I giocatori indicati sono record
importati, **non necessariamente tutti i giocatori nel mondo
installato**. Allenatori e chairman non hanno ancora un modello
mondiale persistente: non esistono conteggi validi di quelle entità.
La dimensione su disco dei database non è indicizzata, e non deve
essere confusa con quella RAM.

**[PRIORITÀ MASSIMA ANCORA APERTA]** La preview pilota genera
fixture e risultati separatamente dalla carriera OpenSWOS.
Sebbene il join ora impieghi la codifica di fase giusta,
questo non unifica le competizioni e non garantisce risultati
o classifiche identici prima e dopo la firma. Va costruita
una sola sorgente di risultati persistenti con integrazione
nativa, coppe e salvataggi: nessuna doppia simulazione.


## 26. 0.13.10 — Continuazione automatica degli incontri storici (11/11/2026)

**[CODICE / WINDOWS DA TESTARE]** L'utente ha osservato che, nel recupero delle
partite prima della firma, il simulatore nativo progrediva attraverso le
squadre CPU, poi restava in attesa di input quando arrivava a una partita
del club gestito. La causa plausibile è la distinzione nativa fra
`teamControls` umano e CPU, che apre `SetupPlayers` quando usa la
modalità `VIEW RESULT` oppure offre soltanto `PLAY MATCH`.

La nuova build reindirizza **solo mentre `Expanded_MidseasonJoin_Active()`**
le azioni PLAY MATCH di lega e coppa al loro percorso CPU nativo
(selector/mode 0), bypassa `SetupPlayers` soltanto in quel periodo,
e fa percorrere esplicitamente le schermate coppa `DIYCupInit`.
Sono inclusi i callback alternativi in `DIYLeagueTournamentInit` quando
il bottone VIEW RESULT non è visibile. Tutte le operazioni usano ancora
le funzioni originali che aggiornano risultati/classifiche/bracket;
al termine del recupero l'interfaccia umana resta normale. Un guard
continua a interrompere il recupero se compare `PlayMatchMenuInit`:
la protezione contro l'avvio di fisica storica non viene rimossa.

In F12, durante il recupero, vengono mostrati numero di azioni e
numero di simulazioni reindirizzate. Nei log si identificano abort,
menu fallback e chiave nativa finale.

**[LIMITI / PRIORITÀ SUCCESSIVA]** Questo è un avanzamento funzionale
del recupero *nativo*, non è ancora una unificazione completa:
`ExpandedWorldLive` calcola prima dell'assunzione risultati pilota
indipendenti. Servono ancora il mondo canonico, le fixture mondiali
condivise prima/dopo la firma e la serializzazione. Non proclamare
coerenza delle classifiche prima/dopo né salvataggio mondiale prima
dei test Windows.

**[VERIFICA]** Revisione del codice e controllo degli instradamenti;
non è stata eseguita una build completa Windows né è stato fatto un
test manuale di carriera in questo ambiente. Versione precedente
per rollback: `fab201b98ae60ab37f105ee48589c66b234a809c` (0.13.09).


## 27. 0.13.11 — Date native e avanzamento non visibile durante l'ingaggio

**[OSSERVAZIONE UTENTE, 11 novembre 2026]** La prima versione del catch-up
mostrava i risultati della classifica cambiare riga per riga, a fronte di
una sola gara visibile giocata e una data che rimaneva ferma: schermata
`THU 28 AUG 1997` nella stagione 1996/97.

**[ERRORE VERIFICATO NEL CODICE]** `Expanded_FormatDate` usava
`ExpandedWorld_DateAfterWeeks(august_1, native_key)`: `native_key=56`
era interpretato come **56 settimane**, anziché `56/8=7` mesi dopo
gennaio 1996, quindi **1 agosto 1996**. Introduzione di
`ExpandedWorld_NativePhaseToDate` e lettura del `sortKey` della prossima
fixture dal calendario nativo, mantenendo distinta l'ultima fase
contabilizzata (`_lastFinancialWeekIndex`).

**[CODICE]** La fase di arresto ora e' la prima fase **non precedente**
alla data della firma, arrotondata in alto, per non escludere incontri
anteriori appartenenti alla stessa frazione di mese. L'ordine di priorita'
del driver usa NEXT MATCH prima delle voci di visualizzazione risultato,
come nel golden career harness. L'input umano viene scartato soltanto
durante il catch-up, e il renderer nasconde temporaneamente le tabelle
native che cambiano dietro una semplice schermata di avanzamento, con
F12 diagnostico sopra di essa.

**[VERIFICA E LIMITI]** Eseguiti 94 controlli indipendenti di aritmetica
civile/fase nel runner JavaScript (47 settimane per ciascun mondo 1996 e
2026), **non** la compilazione/test C o il runtime Windows.
I test C dei due moduli contengono nuove asserzioni da eseguire.
La preview pilota e la carriera nativa **non** sono ancora un unico
registro risultati persistente; verifica Windows obbligatoria prima di
considerare l'ingaggio funzionante. Versione per rollback:
`f58818c29e607017e62e390c1804548d5c299ce6` (0.13.10).


## 28. 0.13.12 — Risolto il blocco da menu lega chiuso (proposta di verifica Windows)

**[BUG OSSERVATO, 11/11/2026]** Il recupero del calendario nativo nella 0.13.11
si arrestava dopo una sola giornata simulata; F12 indicava `ABORTED FINAL -8`
con target 87. La data e' ora coerente e non necessita altre modifiche.

**[CAUSA INDIVIDUATA NEL CODICE]** Quando una giornata del campionato e'
completata, `DIYLeagueTournament_UpdateMenuEntryVisibilityAndFocus`
rende invisibili `NEXT MATCH`, `MATCH`, `RESULT`, `VIEW RESULT`,
selezionando `EXIT` (callback `DIYLeagueTournamentExit`). Il recupero
storico non tentava `EXIT` e chiamava immediatamente `finish(0,-8)`.
Il golden career harness, invece, esegue l'uscita standard e ritorna al
calendario, dove puo' raggiungere la giornata successiva.

**[MODIFICA]** Dopo aver privilegiato le azioni native di partita/risultato,
il catch-up sceglie il callback originale `DIYLeagueTournamentExit`
quando non trova una partita ancora eseguibile. Il callback originale
ripristina il flusso verso `Career_NextMatch`, senza patch manuali dei
cursori, dei punti o delle classifiche. `-8` resta un arresto diagnostico
solo se non e' disponibile nemmeno EXIT. F12 include il numero di
`LEAGUE MENUS COMPLETED`; log stderr con indice, fase e target.

**[STATO]** Cambio funzionale implementato nel codice, non collaudato Windows.
Da verificare: la simulazione progressiva fino alla prima partita non
precedente alla firma, coerenza classifica e input normale una volta
terminato il catch-up. Pilot e competizioni native sono ancora due
fonti indipendenti e non sono state unificate.


## 29. 0.13.13 — primo collegamento dei risultati tra pilota e campionato nativo (11/11/2026)

**[CODICE / PRIMA INTEGRAZIONE, NON UNIFICAZIONE COMPLETA]** Nella 0.13.12
il recupero storico arriva effettivamente alla data dell'ingaggio:
screenshot utente `JOIN STATUS REACHED, FINAL 91, TARGET 91`,
`LEAGUE MENUS COMPLETED 22`, circa un minuto per recuperare
le partite, con rendering delle schermate originali ancora visibile
ai lati dell'overlay temporaneo. Il fatto che il recupero funzioni
non significa che i risultati pilota e nativi siano identici.

Nella 0.13.13, `ProcessDIYLeagueMatchResultAndCareerUpdates` interroga il
registro `ExpandedWorldLive` prima di chiamare il generatore di un
nuovo risultato durante il solo recupero storico. Il risultato pilota
puo' essere riutilizzato una sola volta se la gara e' gia' disputata
nel registro, i **club di casa e trasferta coincidono esattamente** e
la data della gara cade nella finestra della fase nativa (fino a sette giorni prima, e non oltre l'inizio della fase successiva). Non
si copiano classifiche; la partita aggiorna normalmente punteggi,
statistiche e calendario nelle routine OpenSWOS. Se la gara nativa
non ha un risultato compatibile disponibile (calendari differenti,
altra nazione, altro ordine, coppe), resta attivo il resolver OpenSWOS.
Le coppe e tutte le partite dopo la firma sono invariate.

F12 espone `NATIVE REUSED` (risultati comuni realmente riutilizzati)
e `FALLBACK` (incontri per cui si e' usato il resolver nativo).
La dicitura `PILOT/NATIVE REPLAY PARTIAL` segnala esplicitamente
che **non esiste ancora una sola fonte globale di competizioni**.

**[NON ANCORA IMPLEMENTATO]** Sostituire il calendario pilota con
il calendario nativo condiviso per tutte le nazioni, effettuare la
simulazione nativa mentre si e' disoccupati senza ricostruire
`InitCareer()` alla firma, garantire persistenza di tutte le
competizioni e salvataggi mondiali; poi eliminare il recupero
storico e la seconda simulazione. Questa build non promette
di eliminare l'attesa o il flicker, soltanto di iniziare a condividere
risultati dove la partita corrisponde realmente.

**[PERSONE DEL MONDO]** Nel database OpenSWOS `TeamFile.coachName[23]`
contiene il nome dell'allenatore associato alla squadra; manca
una tabella di entita' globali allenatore con età, contratti e
carriera. I messaggi del presidente non dimostrano l'esistenza di
identita' chairman persistenti; non e' stata trovata un'anagrafica
globale di presidenti. Non inventare tali conteggi nel F12.

**[VERIFICHE]** Aggiunto test della corrispondenza home/away e della
fase cronologica, incluso controllo di riuso singolo e di assenza
di mutazioni dei risultati pilota; build Windows e test end-to-end
obbligatori prima di dichiarare affidabile la continuita'.

**[SCELTA UX]** Su richiesta esplicita, il recupero non e' stato spostato dal passaggio tra contratto e benvenuto: la sequenza ATTUALE resta invariata. Il miglioramento dell'avvio e dell'attesa non e' parte di questo commit.


## 30. 0.13.14 — decoupling renderer e riuso stagionale dei punteggi

**[TEST UTENTE]** Nella 0.13.13 il calendario della carriera mostrava la data giusta
e il recupero terminava (`REACHED`), ma la classifica vista durante la
disoccupazione era diversa da quella nativa. F12 ha certificato soltanto
12 incontri riutilizzati e 360 fallback nativi. Le classifiche NON sono
ancora unificate.

**[MODIFICA CODICE]** Il percorso provvisorio di recupero delle partite non
necessita un render SDL per ciascuna partita: `render_frame_impl` mostra
**una sola volta** una schermata statica di simulazione e continua a
pompare eventi e audio, ma sopprime gli upload delle texture, il
`SDL_RenderPresent` e gli effetti CRT durante il recupero. Il renderer
di backup eseguito dal timer rispetta lo stesso controllo. `Platform_Present`
non applica il pacing a 50 Hz durante l'automazione storica **su Windows/
native**; WASM conserva il yield. Terminato l'ingaggio, rendering e pacing
originali riprendono. Nessun cambiamento alla fisica partita, ai callback
di competizione o al layout del menu calendario. F12 mostra numero di
frame soppressi e durata del recupero in millisecondi.

**[MODIFICA CODICE, COLLEGAMENTO ANCORA PARZIALE]** Due calendari differenti
possono collocare la stessa gara in settimane diverse. Il nuovo
`ExpandedWorldLive_TakeNativeLeagueResult` cerca il risultato gia'
registrato nello stesso **anno sportivo** per la stessa coppia ordinata
(home/away), senza imporre la stessa data di incontro. Il risultato
puo' essere usato soltanto una volta e soltanto se il ledger pilota lo
ha effettivamente gia' disputato prima della firma. L'esito attraversa
comunque i callback di classifica originali OpenSWOS. Se non c'e'
corrispondenza, rimane la simulazione nativa. Questa scelta privilegia
l'identita' della partita alla coincidenza artificiale delle date,
**ma l'ordine delle fixture resta diverso** e le tabelle possono ancora
divergere. Non si copiano punti né si riscrivono risultati gia' registrati.

**[COLLAUDO PENDENTE]** Test C delle finestre stagionali e dell'unicita'
aggiornati; non e' stata compilata la build completa Windows in chat.
Confrontare a test Windows (a) tempo reale del recupero, (b) assenza di
flicker, (c) stato `REACHED`, (d) F12 `NATIVE REUSED`/`FALLBACK`,
(e) integrita' di calendario/classifica dopo la firma e input normale.
Il requisito finale rimane il **Living World Engine unico**: avviare e
far progredire le competizioni native una sola volta durante la
disoccupazione, eliminando il catch-up quando esiste uno stato condiviso.


## 31. 0.13.15 — calendario OpenSWOS condiviso dal mondo e dalla carriera

**[DECISIONE UTENTE]** Non sostituire le competizioni originali SWOS con
un nuovo scheduler parallelo. Mantenere le tabelle native
`_fixtureSchedulePtrTable` (inclusi i calendari Berger con BYE per
campionati dispari) e i campi `beginMonth`/`endMonth` ufficiali,
estendendoli con un unico registro mondiale dei risultati.

**[CODICE, BUILD WINDOWS DA TESTARE]** Il Living World Engine genera
le gare del campionato usando le coppie della tabella originale SWOS
anziché il proprio algoritmo a rotazione. Per ogni lega importata
legge le fasi dei mesi dal record `Competition` usato anche dalla
carriera; distribuisce le giornate nelle stesse fasi originali tramite
l'interpolazione `phaseStartMonths`. Le gare di ritorno invertono
casa/trasferta. Per le leghe del mondo importate e completamente
riconoscibili, `DIY_ShuffleLeagueTeamOrder` usa l'ordine dei club già
contenuto nel mondo anziché un nuovo sorteggio: la coppia OG
casa/trasferta rimane uguale prima e dopo la firma.
Le leghe non riconosciute usano ancora la routine originale di
shuffle senza alcun override.

**[CODICE, POST FIRMA]** `ProcessDIYLeagueMatchResultAndCareerUpdates`,
dopo aver elaborato un vero risultato nativo post-ingaggio, registra
lo stesso punteggio nel ledger mondiale soltanto se trova la coppia
esatta nella fase temporale nativa corretta. Quindi avanza il mondo
fino alla giornata, simulando le gare CPU rimanenti, senza
ricalcolare la gara giocata. Il gioco non altera la fisica SWOS.

**[DEBUG F12]** `OG LEAGUE CALENDARS X / Y ORDERS Z`:
X=leghe del mondo per cui è stata costruita una tabella nativa OG,
Y=leghe importate, Z=numero di inizializzazioni di leghe native con
ordine allineato al mondo. Rimangono `NATIVE REUSED`/`FALLBACK`
per valutare i casi di mismatch dopo la firma.

**[LIMITI DA NON NASCONDERE]** La carriera originale conserva il proprio
stato delle classifiche/cursor e all'ingaggio ha ancora bisogno
di applicare lo storico del ledger attraverso il recupero headless;
questo commit **non elimina** il doppio contenitore di stato, il
recupero dopo firma, i casi particolari di campionati a fasi/split,
le competizioni a coppa, i trasferimenti e il salvataggio del mondo.
Il primo obiettivo operativo è che, per le normali leghe coperte,
i risultati vengano calcolati una sola volta e le classifiche
prima/dopo coincidano. Non proclamare unificazione globale completa
senza un test integrato che includa esonero, riassunzione e salvataggio.

**[VERIFICA]** Test C aggiornati per: tabella originale 20 squadre,
giornate dispari a 5 squadre con BYE, fasi originali, mappatura
deterministica squadre native, risultato pilota riusato una sola volta,
e scrittura dei risultati reali nativi nel mondo. I test C e la
compilazione Windows non sono stati eseguiti qui; test manuale e
compilazione sul PC dell'utente obbligatori.

Rollback: `0.13.14`, commit
`e2cee6d2d965b965034b15246b0d178143ea5d61`.


## 32. 0.13.15_01 — hotfix dopo il primo pull (11/11/2026)

**[INCIDENTE UTENTE]** Segnalato il messaggio esatto `Connection interrupted. Waiting for the complete answer` al termine dell'unificazione sperimentale 0.13.15. L'utente conferma di aver **gia' effettuato il Pull** da `dev/0.10`. Non riscrivere il commit pubblicato: fornire il fix come commit successivo, mantenendo la cronologia GitKraken.

**[ERRORE CODICE CONFERMATO]** `ExpandedWorldLive_LoadOfferCountries` leggeva `league->rounds`, ma il record consolidato `CompetitionLeague` contiene il numero di gironi in `league->header.rounds` (`sorgenti/include/game_types.h`, `CompLeagueHeader`). Corretto l'accesso per evitare errore di compilazione della 0.13.15. Simulazione e gameplay invariati.

**[BUILD]** Suffisso di commit intermedio `0.13.15_01`, senza nuova BUILD di release. Compilazione e collaudo Windows non eseguiti in chat. Basta un ulteriore Pull: nessun reset, force-push o rollback automatico.


### 0.13.16 — importazione diretta dei campionati OG / audit punteggio manuale (11/11/2026)

**[CODICE, SVILUPPO SPERIMENTALE]** Nuova funzione
\`DIY_WorldDirectImportLeague(cutoff)\`: il mondo è già stato simulato
durante la disoccupazione. Dopo \`InitCareer()\` la lega nativa OG
importa il prefisso di incontri già conclusi dal **medesimo ledger**
mondiale, senza risimulare risultati e senza aprire i menu.
Per aggiornare lo stato OG usa \`DIY_GetCurrentFixtureTeamPtrs\` e
\`DIY_AdvanceLeagueProgressAfterMatch\`, che gestiscono partita,
mezza giornata, fase e girone; aggiorna anche statistiche, classifica,
gol e punti. Per ogni partita storica del club, aggiorna e marca
il relativo elemento della keycache originale soltanto dopo aver
verificato la completa corrispondenza delle fasi e degli incontri.
Se manca un risultato, se il calendario non coincide o se è una lega
non standard, l'importazione NON viene salvata e torna automaticamente
al recupero nativo già collaudato della 0.13.15_01.

**[COPPE]** Il ledger mondiale non possiede ancora i tabelloni delle
coppe. Per non creare automaticamente risultati falsi, se esistono
turni di coppa precedenti all'assunzione, questi continuano
nell'originale driver OpenSWOS. Le giornate del campionato già
importate sono saltate attraverso il cursore originale
\`Career_AdvanceToNextTeamSelectionKeyCacheEntry\`, conservando
i suoi aggiornamenti finanziari e gli hook delle competizioni.
Quando non esistono coppe/eventi precedenti all'assunzione e
l'importazione riesce, si raggiunge **direttamente** il primo
incontro futuro e si evita tutto il recupero e il rendering headless.
Il risultato F12 è \`JOIN STATUS DIRECT\` in quel caso.

**[DEBUG F12]** \`DIRECT OG IMPORT N MATCHES M ROUNDS\` misura
gli incontri OG importati senza ricalcolare risultati. Le nuove righe
\`LAST OG [gol] PTS +[...]\` e \`RAW FINAL ... MATCH/SCORE MISMATCH\`
confrontano risultato dell'ultimo incontro manuale, risultato
effettivamente applicato dalla cache e incremento di punti della
classifica OG. Nel codice della lega originale il pareggio aggiunge
1 punto a ciascuna squadra, la vittoria \`pointsForAWin\`; un pareggio
che apparentemente aggiunge 3 punti richiede un test manuale con F12,
non modificare le regole OG sulla base di un'impressione.

**[TEST E RISCHI]** Nuove asserzioni nel test puro C per lettura
non consumante dal ledger mondiale e riutilizzo originale. Test statici
sulle sorgenti; nessuna compilazione eseguibile Windows o esecuzione
completa di partite manuali in questo ambiente. Questo percorso
importa solo leghe nazionali standard a due gironi, numero pari di
club; NON significa che coppe, campionati speciali, carriera dopo
esonero, salvataggi mondiali, fixture in tutte le nazioni siano
unificati. Il vecchio recupero, già verificato su Windows dall'utente,
rimane fallback. Confrontare data, F12, classifica e primo incontro
giocabile e verificare che le coppe non siano state saltate.

**[NOTA STORICA]** Lo screenshot \`MEMO FROM THE CLUB CHAIRMAN\`
mostrato dall'utente è il promemoria originale OpenSWOS del presidente,
non il menu calendario (una risposta precedente lo aveva interpretato
erroneamente). Il campo \`coachName\` OG è un nome, non ancora
un'entità manager persistente; chairman non ha ancora un'anagrafica
globale.

**[INTERRUZIONE]** 11/11/2026: segnalato testualmente
\`Message delivery timed out. Please try again.\` durante le analisi
per la 0.13.16; ripresa senza perdere le decisioni del progetto.
Causa/durata non note. La successiva osservazione dell'utente sul
possibile risultato manuale riletto dalla cache viene trattata
con l'audit in F12.

**[ROLLBACK]** Commit funzionante verificato sul PC dall'utente:
\`600141da7d82ae88f753f36dffebd917d9fbf382\` (0.13.15_01).
Nessun rollback o force-push automatico.


### Living World — milestone di persistenza 70 stagioni (11/11/2026)
**[CODICE PREPARATO 0.13.22; TEST WIN PENDENTE]** Due stagioni residenti; archivio leggero per campionato/anno con campione, punti, match e digest, in `living_world` version 3 dentro lo stesso `.CAR`. Generazione delle stagioni con tabelle OG e slot 16-bit riutilizzati soltanto dopo archiviazione. Il riavvio della carriera (snapshot v1/v2 legacy compatibili) resta necessario per verificare l'integrazione. Le coppe, i singoli risultati di match storici, la progressione delle rose, i manager/chairman, il mercato e gli eventi giornalieri non sono coperti dal nuovo archivio: il loro design deve restare coerente con la futura persistenza minima dei fatti irripetibili.
**[CRITERIO DI TEST]** Il test C da 70 stagioni, con almeno 14 checkpoint save/reload, è integrato nel repository; PASS sul PC utente è da verificare, non presunto. È distinto dal test end-to-end della carriera OG.
**[DECISIONE FUTURA]** Discutere e approvare insieme **tutti i valori della simulazione e il bilanciamento** (probabilità reti, dispersioni, infortuni, crescita, trasferimenti, forma/morale, economia, esoneri) dopo aver concluso la struttura dell'engine. Gli attuali coefficienti del pilot sono placeholder, non valori finali approvati.


### Conferma 12/11/2026 — penultimo step dati e formula di simulazione
L'utente ha stabilito che **tutti** i dati da aggiungere o calibrare nel database/mondo (compresa forma individuale dei calciatori, dati degli allenatori e dei chairman, età, stipendi, rating, comportamenti e formule) saranno concordati insieme **come penultimo passo prima di chiudere il Living World Engine**, seguito da collaudo/integrazione e dalla build target 0.14.00. Le strutture potranno essere preparate senza fissare in autonomia valori arbitrari. Il titolo grafico resta rinviato; unicamente tasto DEBUG F12+N per NEW CAREER nella prossima build, senza cancellare o sovrascrivere .CAR finché non esiste il nuovo primo salvataggio valido.

## Integrazione del mondo calcistico e mod esterne — aggiornamento 12/11/2026
**[DECISIONE CONFERMATA]** OpenSWOS conserva il ruolo di motore autorevole per ogni gestione esistente di calendario, gironi, coppe, sorteggi, spareggi, rigori, replay, classifiche e partite. Living World registra in un solo archivio cronologico gli ID dei club, gli esiti e i titoli, senza ripetere i callback OG. Ogni incontro deve essere finalizzato una volta sola.
**[CODICE PARZIALE 0.13.30 — TEST WINDOWS DA FARE]** Persistono anche le identità dei campioni di coppa dopo la rotazione delle stagioni. Nel nuovo JSON mondiale v6 sono leggibili salvataggi v1-v5. Alcuni vecchi save conservano solo il numero dei titoli: non inventare nomi mancanti. Un doppio evento coppa con risultato diverso deve essere rifiutato/loggato, non applicato.
**[PENDENTE]** Far svolgere tramite funzioni OG, non un simulatore parallelo, tutte le coppe nazionali e internazionali anche durante più anni di disoccupazione e dopo un'assunzione in stagione; sostituire il recupero OG solo quando equivalenza cronologica e save/load siano verificati su competizioni reali. I test OG cup journal sintetici non sono prova di coppe reali end-to-end.

### Mod e progetti esistenti — idee/codice, non nuovi dati approvati
**[RICERCA / PROPOSTA]** SWOS 96/97 Mod / World of SWOS di Davide Lorigliola, versione 4.1.1 (https://github.com/aminta/swos-9697-mod ; https://worldofswos.com/en/): formati speciali di campionato, gironi paralleli, playoff/playout, coppe continentali, rose fino a 30 giocatori e supporto squadre B. Valutare/riutilizzare SOLO codice o funzioni compatibili e autorizzate; il database 96/97 resta intatto finché non lo decideremo. Dati calcistici e codice possono avere licenze differenti. Prima verificare ciò che OpenSWOS già incorpora, evitando duplicati. Limite OG 24 squadre per girone round-robin sufficiente; tornei più grandi modellati come gruppi/conference.
Altre fonti da studiare all'occorrenza: OpenSWOS upstream (netcode e motore), SWOS 2020 (editor e strumenti), SensiSquad (fonti dati da autorizzare).

### Possibile patch multiplayer 1.1 — NON ora
**[DIREZIONE DI DESIGN, NON IMPLEMENTATA]** Università calcistica con allenatori umani indipendenti, ognuno nel proprio club e campionato, sfide dirette fra due utenti tramite netcode OpenSWOS se compatibile. Obiettivi di carico da PROVARE: 64, 128 e 256 allenatori; 64 consentono al massimo 32 partite fra due utenti simultanee, ma non provano che il server regga il carico.
**[PROPOSTA]** Un solo server autorevole per tempo degli eventi, mercato, trasferimenti, rose, risultati e salvataggi. Avanzamento asincrono finché le azioni non coinvolgono un altro manager; appuntamenti e watermark temporali per incroci, qualificazioni e date divergenti, senza mai cambiare retroattivamente il passato di un altro club. Definire politiche di assenza e modalità locali/online separatamente. Principali limiti: coerenza multi-utente, sincronizzazione, cheating, disconnessioni, trasferimenti concorrenti, archivio mondiale.
**[TEST FUTURI]** Su un singolo PC: server localhost + due client, poi 16/64/128/256 client simulati senza grafica con latenza/conflitti/riconnessioni; test effettivo Windows e partita reale prima di promettere capacità.
Nessuna modifica di rete o architettura corrente autorizzata: il multiplayer rimane dopo il completamento del gioco.

### 15/11/2026 — proposta autorizzata per VIEW COMPETITIONS da offerta
Quando esiste un'offerta, il manager conosce già il paese (convertito dal file squadra), la divisione e l'anno. VIEW COMPETITIONS deve aprire DIRETTAMENTE il viewer della divisione OG residente, non un secondo selettore di paesi. Uscendo torna all'offerta senza accettarla e senza cambiare punti, risultati, goalStats o stato coppe. Il selettore DEBUG WORLD disoccupato risale un livello per B/ESC/BACKSPACE. Implementazione sorgente 0.13.48, verifica Windows PENDENTE.

### 15/11/2026 — Decisione definitiva: VIEW COMPETITIONS dall'offerta e navigazione BACK
**[CONFERMATO, sostituisce il paragrafo 15/11 sulla vista diretta della divisione]** Dall'offerta il club, paese e divisione sono gia' noti: VIEW COMPETITIONS apre la **stessa lista OG di DEBUG WORLD** gia' posizionata nel paese corretto, SENZA chiedere nuovamente il paese. Elencare il campionato del club e le coppe a cui i dati OG residenti confermano che partecipa; segnalare le coppe non residenti/da verificare senza inventare iscrizioni o partite. La scelta di una voce apre il suo viewer OG in sola lettura; BACK dal viewer torna alla lista, poi BACK torna all'offerta; ogni pressione risale **un solo menu**. ACCEPT non e' implicito.
**[INVARIANTE ENGINE]** Unico motore OG per tutto il calcio in entrambi i mondi. Ingresso in carriera e firma del contratto devono collegare il manager ai dati OG residenti, mai ricalcolare risultati, marcatori, rose o competizioni. La UI puo' ricostruire soltanto scratch/menu non autoritativi. Se i dati OG mancano, mostrare NON DISPONIBILE e registrare il motivo.
**[STATO]** Preparazione codice 0.13.51, Windows e gameplay non ancora verificati. La verifica include TOP GOAL SCORERS pre/post ingaggio, coppa del club realmente partecipante, BACK, 70 anni e save/reload. Il controllo delle 101 coppe type-1 nazionali NON certifica le altre competizioni nazionali/internazionali.

### 15/11/2026 — criteri finali di validazione OG (decisione utente)
Non far venire PASS al controllo forzando criteri o cambiando le coppe. Una coppa puo' avere bye nativi e meno iscritti dei posti del tabellone, ma solo il confronto con i turni OG e il reale avanzamento dimostra il suo funzionamento. Nazionali (Mondiali/Europei/altre coppe + qualificazioni) e internazionali per club (UEFA/continentali) vanno valutate **separatamente** dalle nazionali di club. Solo OG genera le loro partite; nessun LWE separato.
Non confondere rientro nella carriera al 2096 con prova che `s_expOG`, `s_expOGCups`, RNG, fixtures, risultati e rose siano tutti stati ricaricati. Richiedere confronto dopo SAVE/LOAD reale; non risimulare i 70 anni per ripristinare un salvataggio. Dopo prove nei due mondi 1996/97 e 2026/27 si puo' congelare il motore e tornare alle espansioni, con bug reali corretti separatamente.

### 15/11/2026 — visibilita' integrale delle coppe a START (decisione utente)
Ogni paese incluso nel mondo scelto deve mostrare OG campionati e coppe fin da NEW CAREER -> START, **anche prima di avere giocato una sola partita**: struttura, partecipanti e abbinamenti nativi; non inventare risultati, non aprire gare due volte, niente seconda simulazione LWE. Se uno stato manca, indicare la competizione precisa e il motivo, non fabbricare PASS o fixture.
Una coppa OG con bye ha `base.numTeams` pari agli slot del tabellone (potenza di due), `_numSelectedTeams` pari ai club REALI, e solo `WorldCup.ties` gare effettive nel primo turno. `WorldCups_Padded` non e' l'ingresso di squadre vere; `WorldCups_CupMade` marca i bye. La correzione accetta solo i formati verificati anche negli ID dei reali accoppiamenti, senza cambiare alcuna regola OG.
Le competizioni nazionali per club, internazionali per club e competizioni per nazionali con qualificazioni hanno audit distinti; superare il test delle 101 definizioni type1 non chiude gli altri ambiti. Dopo entrambe le prove Windows e save/reload completo si puo' valutare di chiudere il motore, non prima.

### 15/11/2026 - regola definitiva di visibilita' coppe da OFFERTA
La consultazione di una competizione OG residente del paese dell'offerta e' indipendente dall'effettiva iscrizione del club che offre il contratto. VIEW COMPETITIONS deve mostrare le stesse coppe residenti leggibili da DEBUG WORLD (anche con zero incontri e se il club non partecipa). Lo stato del club nella coppa NON viene cambiato; nessuna partita nuova, punteggio o tabellone inventato. Questa decisione di visibilita' sostituisce il precedente filtro UI delle sole coppe del club, senza modificarne la partecipazione sportiva.
Le coppe UEFA moderne e storiche, Libertadores e altri continenti, Mondiali/Europei/altre nazionali e qualificazioni sono ambiti separati dalle 101 coppe nazionali domestiche type1. Definizioni e contatori OG non sono prova di fixture datate, match o persistenza. Autosave mensile da disoccupato discusso e proposto dall'utente: NON IMPLEMENTATO in 0.13.55, da realizzare solo con persistenza coerente del manager disoccupato e di tutto il mondo OG.


### 16/11/2026 — nazionali, tutte le competizioni Mod e viste squadra
- **APPROVATO: INIZIA COME CT** nel menu NEW CAREER: la nazionale si sceglie all'inizio, con carriera nazionale OG completa (convocazioni, formazione e rosa della nazionale, amichevoli, qualificazioni, coppe, contratti/offerte successive, esonero). E' un percorso Expanded aggiunto alle offerte originali da CT che possono arrivare durante la carriera di club. Non deve far partire un secondo mondo, cambiare la data OG o saltare gare. Non ancora implementato.
- **Mod 2026/27**: implementare tutto il contenuto della versione corrente 4.1.2 e progettare un inventario/version manifest per integrarne le evoluzioni future. Roadmap degli autori 5.0: nazionali, convocazioni, Nations League, Europei 2028, Mondiali 2030 e offerte da CT; non ancora presenti nella 4.1.2, quindi niente calendari o punteggi immaginari.
- **Coppe**: partecipanti/sorteggi all'inizializzazione del mondo, risultati **soltanto** nei loro eventi datati OG. UEFA, Libertadores, altre 11 extra-UEFA, Supercoppe, statali brasiliani e Coppa Serie D vanno trattati ciascuno in forma nativa; mai far girare `ct_season` completo a START.
- **OFFERTA club**: esattamente la stessa schermata rosa Expanded/OG con stelle e informazioni giocatori; azioni impossibili prima della firma nascoste, fuori dall'hitbox e protette dalla callback. Il vecchio percorso OG può restare per CLASSIC e altre modalità originali per non perdere funzionalità.


### 17/11/2026 — Decisioni e verifica Libertadores per entrambi i mondi (0.13.65, PARZIALE)
**Regola approvata:** date reali per tutte le competizioni di **entrambi** i mondi 1996/97 e 2026/27. La carriera cambia stagione il 1 luglio, ma un'edizione OG annuale può proseguire fino ad agosto o novembre. Formato storico Libertadores: 20 club, 5x4; formato 2027: 47 iscritti, 28 dirette, 19 preliminari, quattro posti gironi liberi fino a Q3. Fra i 32 club dello screenshot sono spostati a preliminari Alianza Lima, The Strongest, Juventud, Deportivo Táchira; altre quindici squadre dai rispettivi paesi completano la lista. **Nomi, source nazionale e ID OG di tutti i 47, date ufficiali 2027 e limiti** sono in `docs/LIBERTADORES_1997_2027_DECISIONS.md` e nel manifest `data_json/worlds/2627/competitions/libertadores_2027_first.json`.
**Codice 0.13.65:** reset della stagione della lega distinto dal reset totale, conservazione dello stato CT_ED attivo; lettore read-only delle date-base per singolo leg 2027. **NON ANCORA IMPLEMENTATO:** fixture reali per squadra con risultato/andata-ritorno persistente, esecutore OG giornaliero, sorteggi e save/load integrale, e collegamento completo delle due Libertadores. I dati NON sono partite giocate. Windows compilazione e gameplay NON PROVATI.



### 17 novembre 2026 — 0.13.67: NO RESTART e doppio static namespace preparato al loading (Windows NON PROVATO)
Decisione già approvata e ora implementata nel codice del selettore NEW CAREER: preparare i registri statici di **entrambi** i mondi durante il LOADING, ma mantenere **un solo insieme di puntatori OG attivo**; nessuna partita simulata durante la selezione. `world_switch.c` salva 290 aree dei record/tabelle native prima della sovrascrittura e dopo l'installazione completa dei dati moderni; include offsets squadra, paese, competizioni, playoff, regioni e `_careerContests`. `competition_data.c` aggiunge restore del registro compId per non mescolare i template dinamici moderni con i 1996/97. Il switch UI usa `WorldSwitch_SelectBeforeCareer`, invalida la cache TEAM, non chiama più CreateProcess/exit e mantiene salvataggi di mondi distinti. Modello confrontato con MOD 4.1.2 (`careerworld.py::w_ns_set`, `swe.c::reg_enter/reg_leave`); Expanded deve salvare più registri dei soli tre della MOD. Guida/audit dettagliati in `docs/WORLD_SWITCH_NO_RESTART_20261117.md`. **Compilazione e gameplay Windows NON PROVATI; non equivale a completamento mondiale OG, né a risoluzione della Finlandia 96/97.**
