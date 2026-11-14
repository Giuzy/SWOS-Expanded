# SWOS Expanded — Registro delle scoperte sul codice

Ultima analisi: 13 novembre 2026. Fonti: codice attivo del ramo dev/0.10,
AGENTS.md, PROJECT_STATE.md, IMPROVEMENTS.md, test e log Windows dell'11-13
novembre, incluse le chat del progetto. Questo documento registra **fatti,
cause confermate e ipotesi da verificare**. Non e' un registro dei PASS.

## Regola di consultazione (vincolante)

Prima di analizzare o modificare codice SWOS Expanded, leggere AGENTS.md,
PROJECT_STATE.md e **questo registro**, oltre ai file e alle guide pertinenti.
Prima di un commit di codice pertinente annotare qui ogni nuova scoperta
(riproduzione, file/funzione/struttura, prova o limite, stato) e indicare
esplicitamente se si tratta di analisi, patch implementata, test C o verifica
Windows. Non attribuire PASS a implementazioni non testate. Cronologia e
decisioni vecchie non equivalgono a comportamento attivo.

## Priorita' e gate di completamento — decisione del 13 novembre 2026

1. CALENDARIO: data esatta della carriera e conversione delle fasi OG,
   avanzamento settimanale, anniversario di 5 anni, cambi stagione,
   rehire, persist/restore, regressioni fino a 30 anni.
2. CAMPIONATI: un solo risultato per incontro, formati OG 2/3/4 cicli,
   classifiche OG/LWE identiche club per club, promozioni, assunzione.
3. COPPE: OG nazionali, internazionali per club e nazionali, qualificazioni,
   turni e campioni, eventi offscreen, autosave e journal.
4. STRESS INTEGRATO Windows: piu' stagioni con cambio squadra/save/rehire.

Una feature si considera COMPLETATA solo dopo test sul codice attivo
e, quando coinvolge OG/menu/SDL/save, prova Windows reale. Se fallisce
si rimane sulla stessa feature, senza aggiungere altre funzioni.
HEADLESS PASS non implica Windows PASS. NOT_TESTED resta NOT_TESTED.

## Calendario — codice e fatti osservati

- `sorgenti/src/expanded_world_time.c`: `ExpandedWorld_DateToIndex` /
  `ExpandedWorld_IndexToDate` conservano il giorno gregoriano (1900-2200).
  `ExpandedWorld_DateAfterWeeks` avanza di 7 giorni per settimana;
  `ExpandedWorldLive_AdvanceToDate` conserva gia' un giorno esatto nel
  ledger della singola istanza mondo.
- I `sortKey` nativi OG sono ottavi di mese: 0=1 gennaio,
  48=1 luglio, 56=1 agosto, 96=1 gennaio dell'anno successivo.
  `ExpandedWorld_NativePhaseToDate` (codice preesistente) restituisce
  il **primo giorno canonico** della fase: giorno
  `1+floor(eighth * days_in_month / 8)`.
  Ogni mese ha 8 fasce disgiunte, la prima comincia l'1, l'ultima
  finisce l'ultimo giorno. Esempio luglio 31 giorni:
  1-3, 4-7, 8-11, 12-15, 16-19, 20-23, 24-27, 28-31.
  Febbraio rispetta 28/29 giorni. La chiave 255 e' sentinella.
- **Problema concettuale individuato:** una data civile arbitraria
  (es. 2 luglio) non puo' essere recuperata dalla sola fase (48),
  perche' la fase non conserva quale giorno della fascia si intende.
  Per date canoniche, invece, `phase -> primo giorno -> phase`
  puo' e deve risultare identico. La data civile non si arrotonda.
  Due operatori distinti: `DateToNativePhaseFloor` (fascia contenente)
  e `DateToNativePhaseNext` (prima fascia che comincia ON/AFTER il giorno).
  Non usare `NEXT` come orologio civile.
- `expanded_midseason_join.c::native_cutoff_at` prima ripeteva
  autonomamente l'arrotondamento alla fase NEXT; ora deve riutilizzare
  la stessa funzione canonica. Per le leghe che giocano in luglio,
  il caso speciale `SouthernCutoff` non deve applicare la presunzione
  generale di preseason agosto.
- `Expanded_Career_NativeCalendarKey` restituisce il sortKey
  della **prossima fixture OG**, da `_teamSelectionKeyCache`, o il
  fallback `_lastFinancialWeekIndex` (contatore FINANZIARIO).
  Non e' una seconda data civile.
  Nell'overlay precedente `OG DATE EST` veniva confrontata con
  `WORLD DATE` con soglia arbitraria +/-31 giorni; questo
  confondeva evento futuro e giorno attuale. Un evento OG futuro
  di 2 giorni **non e'** una divergenza di due orologi.
  Occorre verificare a parte gli eventi OG scaduti e l'assenza
  di fixture pregresse non giocate, senza nascondere errori reali.
- Nel codice precedente F12+7 aumentava `s_waitWeeks` di 260:
  `260 * 7 = 1820` giorni NON sono esattamente cinque anni.
  Da 1 luglio 1996 a 1 luglio 2001 passano 1826 giorni.
  Da 1 luglio 1996 a 1 luglio 2026 passano 10.957 giorni,
  mentre `30*52*7 = 10.920`: deriva di 37 giorni.
  La data deve avanzare per anniversario gregoriano, con policy
  Feb29->Feb28 se anno di destinazione non bisestile.
- `expanded_career.c::unemployed_date_at` ricostruiva il giorno
  esclusivamente da settimane * 7; `Expanded_WaitForRehire`
  arrotondava in avanti alla prossima settimana partendo dalle
  settimane finanziarie OG. Entrambi potevano introdurre giorni
  fittizi dopo un salto civile esatto. Riferirsi al giorno residente
  `ExpandedWorldLive_CalendarState().today`, usare le settimane
  soltanto come contatore di avanzamenti/offerte.
- Una nuova carriera azzera CLOCK TRACE; le offerte successive nella
  stessa carriera non devono cancellare il primo errore.
  La diagnostica F12 deve distinguere `CIVIL TODAY` e `NEXT OG EVENT`.
  Il PASS matematico di conversione non certifica il calendario OG.

## Campionati / handoff — risultati dalle prove Windows

- Un solo ledger di risultati LWE; il motore OG resta l'autorita'
  per le competizioni/carriera e, quando possibile, le partite.
  Richiesta esplicita: nessun doppio motore di campionati parallelo.
- Test Australia (log 0.13.38_01): dopo oltre cinque anni LWE aveva
  14 incontri per squadra, OG 0. Il cutoff 0 prima di agosto non
  considerava leghe native con incontri gennaio-luglio.
  51 record coppe trovati non equivalevano a partite/titoli eseguiti.
- Log reale 0.13.38_05 dopo 261 settimane: caso Australia country=44,
  division=3, `[midseason-southern] PASS strict-world-table`
  e `[join-standings] PASS` su 12 club, zero differenze.
  Questo prova quel singolo confronto, non l'intero calendario
  e non le coppe.
- Stessa prova, nuova carriera a 23 settimane:
  `[midseason] FAIL EARLY_HANDOFF target=91 actual=50`;
  l'originale finisce la carriera OG troppo presto. La guardia
  rileva la violazione ma non risolve la causa, da investigare
  nella fase CAMPIONATI dopo il calendario.
- Hartlepool, log precedente: fase obiettivo143, OG terminava96/97,
  scarto -172 giorni. Classifiche di una divisione con 24 club
  coincidevano nel confronto isolato anche se la stagione si
  interrompeva troppo presto. Evidenza che classifica e tempo
  vanno collaudati separatamente.
- Scozia/Ross County: importazione diretta rifiutata con
  `OG NON-DOUBLE ROUND`; rollback OG e replay storico non
  riconciliavano i punteggi LWE. I formati 3/4 cicli
  ripetono coppie HOME/AWAY: occorre disambiguare per occorrenza
  immutabile della fixture e confrontare la classifica intera.
  Codice tentato in 0.13.38_05; Windows non ancora certificato.
- Promozioni automatiche in 70 stagioni dei test headless erano
  verificate; playoff/split OG completi restano non provati.
  `Career_AdvanceUnemployedOGCups` non ha dimostrato avanzamento
  completo delle coppe durante la disoccupazione.

## Coppe, database e test — limiti confermati

- Il catalogo `data_json/competitions` include leghe, coppe,
  tornei con gruppi, playoff e qualificazioni; non sono
  tutti normali campionati round-robin.
  In `cups.json`, `_tour` e `_friendly` definiscono
  `teams=-1`, sentinella nativa con `comp.type=0`.
  L'audit introdotto in 0.13.38_05 li etichettava erroneamente
  come dati INVALIDI: va accettato il valore speciale solo in
  quel formato. Non allentare i controlli per altri dati errati.
- Audit mondo domestico: 65 JSON TEAM, 1430 club caricati,
  1425 schedulati e 5 singleton, 114 leghe, 70 stagioni.
  Non equivale a test nazionale/internazionale, coppe e playoff.
- 70 stagioni headless e journal coppe sintetico PASS non
  costituiscono prova delle callback OG offscreen, delle nazionali,
  delle coppe reali o del save/reload di tutti i tabelloni.
  `GLOBAL_CUPS_OG_OFFSCREEN=NOT_TESTED` e
  `OG_CUP_WINDOWS_OFFSCREEN=NOT_TESTED` devono restare espliciti.
- Nei log di Windows F12/F10/F12+C vengono acquisiti con
  `SWOS_DIAGNOSTIC_CAPTURE=1` tramite
  `AVVIA_DIAGNOSTICA.bat` che richiama AVVIA_SWOS.bat
  (compilazione e gioco). Non serve fare screenshot per informazioni
  gia' nei log; screenshot per grafica.
- Il BAT 0.13.38_05 falliva nell'analisi runtime con
  `awk: command not found` anche se il gioco funzionava.
  Soluzione coerente: tool C compilato col GCC MinGW gia'
  utilizzato dal gioco, senza Python/PowerShell/awk.
  Un parser mai eseguito o un log di un'altra build deve restituire
  INCOMPLETE e non PASS.
- Crash Windows 0xC0000005 dopo recupero/coppe e reload:
  osservato in un test precedente, causa non dimostrata;
  non attribuirlo automaticamente a un puntatore/fixture senza
  riproduzione e prova specifica.

## Stato e prove richieste prima di avanzare

CALENDARIO = **IN CORSO / WINDOWS NON CERTIFICATO**.
La matematica deve verificare su ogni giorno del 1996-2026 e
ogni fase canonica utilizzabile, mesi/anni bisestili, 5 e 30 anni,
salvataggio e ricarica e F12; il test Windows deve dimostrare
assenza di salti non autorizzati e disallineamenti nel tempo.
Solo dopo iniziare nuovi interventi sui CAMPIONATI, poi COPPE.
Ogni nuovo bug va aggiunto qui con commit, log, funzione responsabile,
test di regressione e stato.

## Ulteriori verifiche del 13 novembre — runtime UI e parser

- `sorgenti/src/menu_system.c::DrawMenu` disegnava ancora nel calendario
  carriera `Expanded_FormatDate(...,_lastFinancialWeekIndex)`: usava
  il contatore FINANZIARIO al posto del giorno civile LWE. Corretto solo
  nel percorso SDL carriera, preservando il fallback OG per Classic e
  il comportamento Amiga. Nuovo testo usa `Expanded_FormatCivilDate`
  sul `ExpandedWorldLive_CalendarState.today`; se data mancante, non
  viene fabbricata una data. Windows NON TESTATO.
- In prova C locale con GCC, convertitore canonico compilato da codice
  corrispondente al blob in commit, con sola intestazione minimale;
  test completo C della suite eseguito con controlli runtime: 109.938
  giorni gregoriani 1900-2200, 7.905 fasi e 11.323 giorni 1996-2026,
  cinque e trenta anni esatti: PASS. Non e' compilazione intera Windows.
- Analizzatore C `audit_windows_runtime_log.c` compilato senza warning
  con GCC -Werror ed eseguito sul vecchio SWOS.log della diagnostica:
  rileva `RUNTIME_OG_LWE_STANDINGS=PASS` e
  `RUNTIME_OG_HANDOFF=FAIL` (target 91 actual 50), in separata sede
  `RUNTIME_OG_CUPS=FAIL` dove 51 coppe registrate ma 0 callback dopo
  261 settimane. Lo stesso FAIL non va chiamato PASS perche' il database
  si carica. Questo e' il log della vecchia 0.13.38_05, NON una
  verifica della nuova build Windows.

## 14 novembre 2026 — calendario: nuovi gate a 30 anni (0.13.39_01)

- **Nuovo test, NON un risultato PASS dichiarato:** 
  `sorgenti/tests/test_calendar_integrity_30.c` usa 
  `ExpandedWorldLive_Begin/SetLeagueSchedule/Seal/AdvanceToDate`, i tavoli
  OG di `FixtureRounds_Install` e il serializzatore `.CAR` esistenti.
  Simula dal 01/07/1996 al 01/07/2026 con due leghe da 20 e 12 club
  (stagioni agosto-maggio e marzo-ottobre); controlla l'anno civile e il
  cursore native in gennaio/luglio, febbraio bisestile, incontri scaduti,
  confronto archivi/fixture/resident, doppio avanzamento allo stesso giorno.
  Ogni gennaio e luglio, e negli anni bisestili, effettua vera serializzazione,
  End + ReadJson e confronto byte-per-byte dello snapshot; primo problema
  `FIRST_CALENDAR_FAILURE`. I marcatori `CALENDAR_30Y_*` sono richiesti
  nel BAT e restano FAIL se non compaiono. Il test C non valida i menu OG
  Windows, ma copre un buco diverso dai 70 anni a settimane.
- Il nuovo audit runtime in `audit_windows_runtime_log.c` verifica l'ordine
  cronologico delle date civili registrate da CLOCK TRACE, resettando il
  confronto alle nuove carriere e alle ricariche `.CAR`. Stato
  `OBSERVED_MONOTONIC_NOT_PROOF` se avanza senza arretramenti;
  `RUNTIME_30_YEAR_CALENDAR_SAVE_LOAD=NOT_TESTED_IN_WINDOWS` fino a
  una prova integrata. Non confondere la prossima fixture OG con il giorno
  della carriera.
- **BUG test competizioni, non database:** l'audit iterava
  `$.countries[i].cups[n]` oltre la fine degli array, e in alcuni casi il
  parser mjson ritornava stringhe dalle `leagues[]` adiacenti. I 16 
  `wrong category` erano riferimenti a leghe esistenti in `leagues.json`.
  `audit_competition_formats.c` ora cerca il token array concreto e legge
  soltanto al suo interno, senza disattivare verifiche categoria o riferimenti.
  Nuova verifica da eseguire: NESSUN falso FAIL, ma veri riferimenti errati
  devono ancora fallire.
- **BUG BAT audit Windows del 14/11:** il GCC assoluto per l'audit 
  caricava `cc1.exe` senza trovare `libgmp-10.dll`; la compilazione
  del gioco con l'ambiente MSYS2 era invece completa. 
  `ANALIZZA_SWOS_LOG.bat` ora antepone `%MSYS_ROOT%\\mingw64\\bin`
  (oppure ucrt64) e `%MSYS_ROOT%\\usr\\bin` a PATH per risolvere le
  DLL, senza installer o compilatore nuovo. Verifica Windows pendente.
- **Stato calendario**: la formula e il salto F12+7 hanno passato i test
  gia' documentati; gli eventi OG da recuperare e i 30 anni di carriera
  reale Windows restano NON PROVATI. Per decisione dell'utente si resta
  su CALENDARIO finche' la nuova suite + prova Windows non bastano.

## 14 novembre 2026 — identificatori squadre, 80 TMD e bug import 3/4 cicli (0.13.40)

**Fonti reali:** archivio ZIP utente (76 `data_json/teams/*.json` e `data/TEAM.NNN`), `sorgenti/src/teams.c`, `expanded_world_live_adapter.c`, `expanded_world_live.c`, `diy_competition.c`, `career_season.c`, `career_eurocups.c`, `team_pool.c`, JSON competizioni. Il censimento dei file non è un test integrato Windows.

- Nei **76 JSON squadre** ci sono **1.700 record**: **1.430 club domestici** (65 file), **80 rose TMD europee** (EUROCUP 16, EUROCWC 32, UEFACUP 32), **142 nazionali** (6 file continentali), **48 CUSTOM**; POOLPLYR contiene giocatori, non ulteriori club.
- Il numero **1.700** NON è il numero dei club distinti: `(country_number, team_ordinal)` ha **1.620 valori distinti** sull'intero corpus; le 80 TMD riutilizzano 80 chiavi già presenti nei club domestici, **talora assegnandole a nomi diversi**. Tutti i **1.430** club domestici hanno invece una coppia distintiva nel loro insieme.
- `global_team_number` **NON è un ID canonico**: ha appena **1.449 valori distinti** su 1.700 record (solo **1.293** distinti sui 1.430 club domestici). `teams.c::SetTeamGlobalNumbers` lo ricalcola da `World_GlobalBase(file)+teamOrdinal` come indice OG; non usarlo come identità globale in competizioni/salvataggi.
- Identità approvata per Expanded: il packed 16 bit **`(country_number<<8)|team_ordinal`**, già usato dal LWE e da varie callback OG. Conservare `team_ordinal`; NON introdurre un catalogo parallelo né cambiare ora il formato dei save. I club in coppe devono riferirsi alla stessa squadra domestica, non essere contati come club supplementari.
- Le **80 TMD vanno CONSERVATE** come rose/seed della **prima stagione 1996/97**. **Non sono posti liberi né qualificate fisse per sempre**: `career_season.c::Career_AdvanceToNextSeason_UpdateCompetitions`, `career_eurocups.c::Career_ProcessCompetitionForEuropeanCupQualification` e `Career_DrawEuropeanCupTeamsAndCopyGrids` compongono le partecipanti delle stagioni successive. Il loro funzionamento integrato offscreen durante disoccupazione è ANCORA NON PROVATO.
- Audit nomi delle 80 TMD contro i 1.430 club domestici, vincolando il paese: **68 corrispondenze esatte normalizzate**, **12 non esatte** (11 possibili denominazioni alternative e **CERCLE BRUGES** senza corrispondente club BELGIUM.json). Non forzare la conversione delle 12 senza prova nominativa; il caso assente richiede decisione su come aggiungere/gestire il club. Nessun TMD è stato cancellato, rinominato o sovrascritto.
- Mismatch provati: **MYPA** Finlandia ordinal **10** ma TMD EUROCWC ordinal **8** (nella Finlandia **KUPS**=8); **HAMBURGER SV** Germania ordinal **16** ma TMD UEFACUP `HAMBURG` ordinal **17** (Germania **HANNOVER 96**=17); **VIT. GUIMARAES** Portogallo ordinal **23** ma TMD UEFACUP ordinal **24** (**VITORIA SETUBAL**=24). Non riscrivere TMD o seed in isolamento: servono resolver e save compatibili.
- Le **127 squadre segnalate escluse dall'import LWE** NON vanno confuse con le 80 TMD o le nazionali: l'import reale scarta club fuori divisioni supportate secondo `Team_IsOutsideLeague`, e possono esistere singleton senza fixture. Inventario degli scarti e motivazioni per club resta da validare nella suite Windows. Non trattare “non schedulato” come identità duplicata.
- Competizioni JSON inventariate: **66** definizioni leghe, **76** coppe, **14** tornei, **7** playoff. Gli array iniziali delle competizioni europee contengono **80 seed** (16+32+32); gli altri tornei comprendono nazionali/CONMEBOL. Numero di club ≠ numero di competizioni ≠ numero di partite. 2/3/4 cicli sono formati nativi da rispettare, senza inventare calendari.
- **BUG concreto import OG/LWE multi-ciclo:** in `diy_competition.c::DIY_WorldDirectImportLeagueChecked` la verifica delle partite pregresse e la scrittura del keycache cercavano la *prima* partita con lo stesso `sortKey`. Con fasi native in ottavi di mese e 3/4 cicli più incontri possono condividere la fase; il secondo match poteva riusare il primo o far fallire l'import. La correzione usa `expanded_native_history.h::ExpandedNativeHistory_ClaimNext`, abbinamento per **occorrenza cronologica univoca**, con rifiuto esplicito del primo mismatch e mappa prevalidata prima di mutare i dati OG. Non sono modificati risultati, classifiche o calendario originale.
- **Test C isolato eseguito localmente** per l'helper condiviso (GCC `-Werror`): fasi ripetute PASS; mismatch respinti PASS; 92 incontri a quattro cicli PASS. Il test è aggiunto a `run_test_engine_suite.sh` e `TEST_LIVING_WORLD.bat`. **Non è** test OG Windows di tutte le leghe.
- **Calendario 30 anni**: `test_calendar_integrity_30.c` e i marker `CALENDAR_30Y_*` restano gate obbligatorio della suite esistente; in questa sessione NON è stata eseguita una compilazione completa Windows o il runtime OG. Mantenerli `NOT_TESTED` finché arrivano log della build aggiornata.
- **Stato**: bug di riconciliazione per stessa fase = PATCH DI CODICE + TEST C ISOLATO; copertura completa 2/3/4 cicli = WINDOWS PENDENTE; identità canoniche delle 80 TMD nel gioco = AUDIT PARZIALE, RIMAPPATURA NON IMPLEMENTATA; 127 esclusi = AUDIT PENDENTE; coppe offscreen = NON TESTATE. Nessuna dichiarazione di integrazione totale o PASS Windows.


## 14 novembre 2026 — World 2026/27, compatibilita' dei dati e debug NEW CAREER

- [VERIFICATO NEL CODICE] `data_json/worlds/2627/WORLD.ini` sul ramo `dev/0.10` usa `efe1464b6d1d62b8`; la precedente `world_data.c` integrava ancora `bf50ef15a18f05ae`. I dati e le tabelle native delle competizioni sono legati alla stessa generazione: **non basta cambiare la stringa fingerprint**.
- [CODICE 0.13.41 — WINDOWS NON TESTATO] Primo allineamento di `sorgenti/src/world_data.c` ai record generati da OpenSWOS ufficiale; mantenuti i fallback del ramo Expanded quando API mod-only upstream non esistono. Aggiunto debug switch con riavvio processo anziche' `World_Set` a runtime; JSON 2627 abilitato al boot. Valutare compilazione e test Windows prima di considerare utilizzabile il mondo.
- [PENDENTE] `InitCareerClub` continua a richiamare `InitializeNewSeason`; il LWE non e' ancora pienamente sostituito dal coordinatore unico approvato. Non attribuire un eventuale avvio riuscito alla piena integrazione del motore.

## 14 novembre 2026 — 0.13.42: fail-closed world fingerprint (Windows NOT_TESTED)

- [CODICE] `sorgenti/src/main.c::main`: dopo confronto del fingerprint in `data_json/worlds/2627/WORLD.ini`, una discrepanza prima veniva solo registrata; il processo continuava con le tabelle native 2627 e file JSON non corrispondenti. Ora conserva il diagnostic `AssetsCheck_SetWorld(2,...)` e torna al World 9697 prima del caricamento delle tabelle e del selettore JSON.
- [LIMITI] Non e' la migrazione completa OG, non cambia in hot swap il mondo selezionato, non corregge le coppe o il recupero OG; manca prova con database errato/assente su Windows, e tutti i gate del registro rimangono aperti. Compilazione Windows, test 96/97 e 2627: NOT_TESTED.
- [CONTINUITA'] Segnalata interruzione chat "Connection interrupted" del 14/11/2026; richiesta esplicita di riprendere da GitHub, preparare sorgenti e commit. Nessun test precedente trasformato in PASS.

## 14 novembre 2026 — 0.13.43: default World al bootstrap

- [CODICE] `main.c::main` sceglie `2627` quando argomenti, variabile SWOS_WORLD e configurazione non specificano un mondo. `World_InstallBaked` rimane in `frontend.c::FrontEndPreLoopSetup` e riceve la selezione validata; `WORLD.ini` assente o fingerprint incompatibile conserva il fallback a 9697. Il 1996/97 selezionato esplicitamente resta disponibile.
- [LIMITI / WINDOWS NOT_TESTED] La selezione non è ancora senza riavvio; `World_InstallBaked` sovrascrive i dati OG e manca una procedura verificata per ripristinare integralmente le tabelle e cache storiche a caldo. Non dichiarare OG/LWE unificati, 127 club risolti o 80 TMD rimappate. Richiesta di migrazione totale ancora aperta.

## 14 novembre 2026 — 0.13.43_01: inventario esclusioni per ID, Windows NON TESTATO

- [CODICE] `expanded_world_live_adapter.c::track_rejection` scrive una riga di log per OGNI squadra esclusa, con mondo, file TEAM, countryNumber, teamOrdinal, packed raw-team-id, divisione, motivo e nome. Le precedenti 12 voci campione per categoria restano per F10, ma non costituiscono il censimento completo.
- [LIMITI] I 127 esclusi storici vanno confrontati col nuovo log Windows reale; non sono stati cancellati, inseriti artificialmente in divisioni non supportate o contabilizzati come club che partecipano al campionato. Questa è diagnostica runtime, NON correzione di tutte le identità/80 TMD né certificazione del motore OG.

## 14 novembre 2026 — 0.13.44, fix nat stagione e preflight save (Windows NOT_TESTED)

- [CODICE] `expanded_midseason_join.c::Expanded_MidseasonJoin_AlignSeason`: `_seasonPlaying` ora usa `(96+offset)%100` anziche' offset puro. L'anno 2026/27 iniziale non deve risultare 2030/31 per questa conversione.
- [CODICE] `SaveJson_CareerCompatible`: lettura non distruttiva di schema (3/4/5), kind=career e world corrispondente prima di attivare CONTINUE verde. Il controllo non certifica integrita' dei dati successivi; il loader mantiene i propri gate. Salvataggi incompatibili non vengono cancellati.
- [CODICE/UI] DEBUG WORLD: etichetta corrente/obiettivo e avviso che lo switch riavvia il gioco, area focus e mouse allineate. Asset PNG invariati.
- [CONFLITTO] AGENTS.md dice slot unico AUTOSAVE.CAR, ultimo piano chat suggerisce salvataggi per mondo: non modificati percorsi/formato; decisione da rendere coerente nel prossimo intervento autorizzato.
- [PENDENTE] Selezione del mondo a START senza restart, ripristino completo delle tabelle OG e separazione lifecycle world/competitions/manager/club; Windows build e test NOT_TESTED.

## 14 novembre 2026 — due slot per mondo (decisione utente e codice 0.13.44_02)

- [DECISIONE] Sostituita la precedente regola del singolo AUTOSAVE.CAR: due salvataggi nativi indipendenti `AUTO9697.CAR` e `AUTO2627.CAR` sotto `save/`. `AUTOSAVE.CAR` preesistente è solo fallback in lettura se schema/kind/world compatibili, non sovrascritto o cancellato dal nuovo sistema.
- [CODICE] `menus.c` seleziona il nome dal `World_Current()`, salva esclusivamente nello slot di quel mondo, rilegge automaticamente dopo cambio mondo, non considera verde CONTINUE quando lo slot (o il legacy) non supera `SaveJson_CareerCompatible`. Alla pensione cancella soltanto il checkpoint da cui è stata caricata/salvata quella carriera, mai quello dell'altro mondo.
- [LIMITI] Il vecchio `AUTOSAVE.CAR` rimane anche dopo il primo nuovo checkpoint; se il nuovo slot viene cancellato, il fallback legacy può ricomparire se compatibile. La pulizia legacy richiede azione esplicita distinta. Il selettore mondo con riavvio resta provvisorio; test Windows e compile NOT_TESTED.

## 14 novembre 2026 — 0.13.44_03: init OG distinti e coppe disoccupato 96/97

- [VERIFICATO NEL CODICE] `World_SeasonCountries()` restituisce NULL nel mondo 9697 (in `world_data.c`); prima `Career_AdvanceUnemployedOGCups` faceva `if(!countries)return`. Risultato: nessuna callback OG delle coppe domestiche del 1996/97 veniva eseguita dal percorso di disoccupazione.
- [PATCH] `career_init.c`: fallback alla tabella originale `_careerCompetitionIdRemapTable` già utilizzata dall'importatore, senza introdurre squadre/paesi inventati; `_seasonPlaying=(96+season)%100` come numerazione originale OG. Il loop resta limitato alle posizioni cup 2/3 e NON certifica tutte le coppe.
- [PATCH] `InitCareerWorld` imposta lo stato world e team pool una volta; nuova `InitCareerCompetitions` contiene gli stessi reset native OG cup, qualificazioni, playoff e copie dei template; `Career_BeginNewCareer` chiama world -> competitions -> manager, `InitCareer` ripara l'ordine per i percorsi legacy. Club/contratto successivo non deve resettare OG world o cups. Non modifica l'installazione globale baked/JSON nel frontend.
- [PENDENTE] La data e lo storico OG devono tornare autoritativi, mentre il LWE corrente mantiene ancora il proprio avanzamento e risultati offscreen. Sostituzione completa non implementata: NON confondere questa separazione con unificazione. Prove Windows e compilazione: NOT_TESTED.
- [CONTINUITA] Richiesta utente ripresa dopo "Connection interrupted": OpenSWOS OG è autorità per data, calendario, carriera, competizioni, risultati e archivio storico originale nei due mondi; il LWE deve essere estensione, non alternativa. Nessun cambio a data o archivio in questa patch.

## 14 novembre 2026 — 0.13.44_04: disattivazione simulatore autonomo LWE

- [CODICE] `ExpandedWorldLive_Seal` importa l'anagrafica club/leghe ma non chiama più `make_fixtures`; `AdvanceToDate` non invoca più `ExpandedWorldLedger_Advance(resolve_game)`, mobilità LWE o amichevoli pilot. `live_complete_prestart` non produce più risultati retroattivi. Codice simulatore precedente racchiuso in rami inattivi `#if 0` per revisione; NON usato in produzione.
- [CODICE] `Career_AdvanceUnemployedOGCups` ora invoca anche `Career_AutoAdvanceInternationalCompetitionFixtures` per le divisioni dei campionati OG nazionali a fine stagione, oltre ai callback OG di coppe già presenti; file TEAM, calciatori, manager, fisica e due autosave invariati.
- [LIMITI CRITICI] `InitCareerCompetitions` a START copia template originali ma non dimostra che tutti gli slot OG siano inizializzati per un'offscreen completo. Non c'è ancora avanzamento OG giornaliero/fase di tutte le leghe; il catch-up dei campionati ora è solo alla fine della stagione. Le offerte basate sulle classifiche LWE non hanno risultati reali e possono NON comparire; il midseason rehire basato su replay LWE è non verificato. I test stress LWE storici non attestano più il percorso corrente.
- [TEST] NESSUNA COMPILAZIONE Windows/MSYS2 o esecuzione del gioco. Lo stato globale OG=autorità è direzione approvata, NON conclusione testata dell'intero engine. Registrata continuità dopo "Connection interrupted".

## 14 novembre 2026 — 0.13.44_05: serializzazione snapshot OG-only vuoto

- [CODICE] Il lettore `ExpandedWorldLive_ReadJson` prima rifiutava sempre `fixture_count==0`; ora accetta solo un ledger LWE vuoto e coerente (nessuna partita giocata, nativa o archiviata), preservando il controllo sulle partite storiche. Necessario per permettere a una nuova carriera OG-only di salvare e ricaricare la propria anagrafica senza creare fixture finte.
- [LIMITI] Non equivale a una migrazione automatica degli storici LWE già presenti nei save vecchi: restano leggibili ma senza ulteriori risultati sintetici. Non collaudato in Windows; le routine OG offscreen e il subentro sono ancora da verificare.

## 14 novembre 2026 — 0.13.44_06: native result does not require LWE mirror

- [CODICE] `diy_competition.c` non tenta `ExpandedWorldLive_RecordNativeLeagueResult` quando il mondo OG-only non ha fixture LWE; la partita resta regolata e salvata nelle strutture native OG. Evita il falso messaggio UNMATCHED per ogni risultato OG valido. Le sessioni con fixture LWE di vecchi save mantengono il controllo di coerenza diagnostico.
- [PENDENTE] Recupero dei risultati OG durante il subentro (il vecchio replay LWE non può più valere come dato autorevole); offerte dalla classifica OG e test Windows non completati.

## 14 novembre 2026 — 0.13.44_07: prima offerta indipendente dalle classifiche LWE

- [CODICE] `choose_small_club` conta le divisioni popolate importate dagli originali CompetitionLeague OG (`ExpandedWorldLive_DivisionStatus` è ora usata solo per metadati paese/divisione/club), scegliendo soltanto l'ultima divisione di paesi con almeno quattro livelli; non usa punteggi o risultati simulati LWE. Nessun risultato o classifica inventato.
- [LIMITE] La classifica nativa OG non è ancora collegata all'algoritmo di offerte: questo fallback seleziona club di categoria inferiore, **non certifica che stiano perdendo**. Il subentro a metà stagione, le coppe internazionali offscreen e il calendario OG completo non sono collaudati.
- [CODICE] I mondi OG-only senza match LWE non presentano una classifica pilota a punti zero come classifica reale; anteprima segnala OG risultati non ancora disponibili. Test Windows NOT_TESTED.

## 14 novembre 2026 — 0.13.44_08: subentro originale OG senza importazione LWE

- [VERIFICATO IN CODICE] `Expanded_MidseasonJoin_Begin` dopo l'import diretto fallito possiede già un ramo di recupero con callback OG (`ContestMenuInit`, `NextMatch`, `DIYLeagueTournamentNextMatch`, `DIYLeagueTournamentViewResult`, `DIYLeagueTournamentResult`, callback OG delle coppe). `diy_competition.c` se non trova un risultato LWE usa già `getCachedOrCalcViewResult` nativo.
- [CODICE] Con `ExpandedWorldLive_Fixtures()==0` (OG-only), `InitCareerClub` non importa più mobilità fittizia LWE; `Expanded_MidseasonJoin_AlignSeason` non richiede più quel prerequisito; `Expanded_MidseasonJoin_Begin` salta l'importazione da classifica LWE e lascia lavorare le callback OG, una volta sola, fino alla prima partita del contratto.
- [LIMITI] Il recupero non è stato eseguito su Windows: i tornei del calendario meridionale hanno ancora controlli legacy che usano tabelle LWE per stabilire la data di luglio. Stagione conclusa, più anni di disoccupazione e premi/offerte basate sulle classifiche OG ancora da verificare. Il cursore giorno LWE per il menu disoccupato è ancora codice di compatibilità, non la data nativa OG definitiva.
- [TEST] Solo lettura/patch GitHub; nessun eseguibile Windows compilato qui. `RUNTIME_30_YEAR_CALENDAR_SAVE_LOAD` e suite OG end-to-end restano NOT_TESTED.

## 14 novembre 2026 — 0.13.44_09: hard gate OG-only anche per vecchi save

- [CODICE] Nuova costante `EXPANDED_WORLD_OG_ONLY=1` in `expanded_world_live.h`: `ExpandedWorldLive_Table` non produce piu' classifiche da snapshot LWE, `ExpandedWorldLive_NativeOrder` non forza shuffle da roster LWE, `diy_competition.c` non consuma risultati LWE nei replay o duplica il risultato OG nel ledger di partite. `career_init.c` e `expanded_midseason_join.c` saltano staging/import LWE anche se un vecchio save contiene ancora fixture. Le callback OG precedenti rimangono attive.
- [LIMITI] L'archivio LWE storico nei vecchi .CAR rimane leggibile ma NON autorevole. Il catch-up OG è predisposto, ancora senza test Windows; alcuni calendari di gennaio e i contratti a stagione conclusa erano dipendenti da tabelle LWE e richiedono un adeguamento. Le offerte in base al RANK OG non sono ancora collegate; non inventare punti o posizioni.

## 14 novembre 2026 — 0.13.45: OG standings read bridge (non un secondo motore)

- [CODICE] Hook read-only dopo `DIY_SortLeagueTableByStandings`: valida n (2..26), campionato senza gruppi, identità dei team, stessa nazione/divisione, partite reali e W+D+L=PL; salva solo W/D/L/GF/GA/PTS prodotti da OG. Cache RAM massima 512 divisioni, invalidata a nuova stagione. `DIY_OGStandings_Read` controlla mondo corrente indirettamente via reset a START, paese, divisione e anno; se manca la tabella la preview non inventa una classifica a zero. L'offerta usa i posti bassi solo con una classifica valida attuale.
- [LIMITE] Il punto di callback dopo il sort OG non prova che tutte le competizioni siano inizializzate/avanzate a START. `InitCareerCompetitions` copia solo template; `InitializeNewSeason` impone team e club. Per una simulazione OG mondiale effettiva occorre isolare l'init nativa delle competizioni dagli slot del manager, preservare gli stati/callback per tutte le nazioni e avanzare per evento durante disoccupazione. Il ponte non completa questa migrazione. Non certificare disponibilità 100% classifiche: solo quelle veramente prodotte.
- [TEST] Codice remoto, compilazione e test Windows NON ESEGUITI. Un unico commit per la richiesta; annotata regola in AGENTS.md.


## 14 novembre 2026 — 0.13.45_02: mappatura Init e caricamento in LOADING/START

1. `main.c` fissa il mondo prima di `FrontEndPreLoopSetup`; `frontend.c` chiama `World_InstallBaked`, `DataJson_InstallCompetitionGraph`, `WorldCups_Install`. Queste operazioni modificano registri/puntatori OG globali: sono un'INSTALLAZIONE, non un innocuo preload simultaneo di due mondi. Servono asset statici condivisi separati dai registri attivi; non fare `World_Set` a caldo con grafi mutati.
2. `menus.c::InitCareerVariables` resetta game type, `_selTeamNumber`, `_currentSeasonShowing=0`, `_seasonPlaying=96`, RNG, cache; `InitCareerManager` lo richiama una sola volta dopo START.
3. `career_init.c::InitCareerWorld` resetta scratch divisioni e `TeamPool_Reset`; `InitCareerCompetitions` resetta cup/UEFA/playoff e copia template; entrambi già protetti da guard ma NON inizializzano ogni competizione di tutte le nazioni per tutti gli eventi futuri.
4. `InitCareerClub` resetta slot-state A/B/C e chiama `InitializeNewSeason`; questo incrementa anno, azzera `_careerLastMatchIndexByCompetitionSlot` e i cinque slot OG, configura club, ricrea il key cache, seleziona competizioni e resetta alcune progressioni. La logica globale va estratta e chiamata a START/cambio stagione, quella club-specifica solo all'assunzione, e va collegata ai progressi OG preesistenti: non spostare la funzione intera a START e non richiamarla al reingaggio.
5. `InitNewSeason` è un altro initializer di slot con `WorldPlayoff_Reset`; usato anche nel percorso CLASSIC/SEASON e va preservato isolato. `Career_AdvanceToNextSeason_UpdateCompetitions` avanza la stagione usando club umano e iterando paesi OG; va reso indipendente dal club senza alterare la logica sorteggi/risultati native.
6. `Career_AutoAdvanceInternationalCompetitionFixtures` e `Career_AutoAdvanceLeagueFixtures` usano `_diyCompetitionStart` condiviso, RNG e risultati originali, ma le chiamate esterne attuali non preservano un contesto nativo duraturo per ciascuna lega. Per FORWARD TIME progressivo serve stato comp (OG) per mondo/paese/divisione/stagione, checkpoint coerente nel salvataggio e avanzamento sino alla prossima fase nativa, mai simulazione LWE parallela. `DIY_OGStandings_Read` è solo cache di osservazione, non archiviazione dello stato completo.
7. [STATO] Questa revisione registra le responsabilità e diagnostica il reingaggio; NON implementa ancora le nuove fasi worldwide/club. Non attribuire alla build corrente classifiche OG complete durante disoccupazione. Nessun test Windows.


## 14 novembre 2026 — 0.13.45_03: separata inizializzazione anno dal club
- [VERIFICATO IN CODICE] Prima della patch `InitCareerClub` chiamava `InitializeNewSeason`; `InitCareer` e `Career_AttachManagerToClub` anticipavano un decremento compensativo di `_seasonPlaying`/`_currentSeasonShowing`. `InitializeNewSeason` incrementava anno e stagioni e ricostruiva slot in un'unica funzione.
- [CODICE] Spostato l'incremento del contatore e la rotazione LeagueGroups/WorldPlayoff nel wrapper solo nuova stagione; la routine Expanded per il club conserva contatori. `WorldPlayoff_Reset` avviene a START e alla vera transizione stagionale, non alla firma. `InitCareerClub` richiama la variante locale, non `InitializeNewSeason`.
- [LIMITI] Gli slot di club vengono tuttora ricreati dal vecchio corpo di `InitializeNewSeason` e non sono importati da una competizione worldwide persistente. Manca il mondo OG season-by-season a START e lo scheduler degli eventi OG offscreen; non affermare che un'offerta mostri classifiche native aggiornate. Il salvataggio dei nuovi stati globali non e' implementato. Test MSYS2/Windows/gioco PENDENTI.


## 14 novembre 2026 — 0.13.45_04: screenshot reale dopo 6 giornate; revisione diagnosi START
- [TEST WINDOWS UTENTE] Screenshot della Tercera División Orient (El Salvador) dopo l'avanzamento: tutte le dieci squadre hanno PL=6 con vittorie/pareggi/sconfitte e punti diversi; la stessa competizione prima dell'inizio aveva PL=0. I risultati OG sono dunque VISIBILI per questa specifica divisione; il precedente PL=0 non provava un mancato avvio di tutte le competizioni. Non estendere questa prova a ogni lega/coppa o alla disponibilità dello stato prima dell'assunzione.
- [CODICE] La cache OG per la preview delle offerte rifiutava `hasGroups != 0`; tuttavia `Career_AutoAdvanceCompetitionSlots` imposta `hasGroups=1/2` anche per l'avanzamento di uno slot di tipo lega. Rimosso il rifiuto basato su tale flag, mantenuti controllo `competitionType==0`, tabella a gruppo unico, validazione team/identità e statistica W+D+L=PL. Accettati anche snapshot nativi PL=0 (preseason) senza aggiungere partite. Diagnostica `[og-offer-table] CAPTURE` per ogni cattura effettiva.
- [CODICE] Quando ancora non è stata catturata una classifica OG della stessa divisione/stagione, VIEW COMPETITIONS mostra la lista delle squadre già memorizzate dall'offerta (sottoinsieme dal relativo file TEAM, massimo 10 visibili), con avviso esplicito che NON sono risultati simulati. Nessuna chiamata a `InitCareerClub`, nessuna partita AI aggiuntiva, nessun LWE scoring.
- [LIMITI] La cache viene alimentata solo quando OG richiama `DIY_SortLeagueTableByStandings`; se nessuna callback ha ancora elaborato la divisione, la preview non può fornire una classifica completa e non deve inventarla. L'intero scheduler mondiale a START e la persistenza per competizione restano da verificare/implementare. Windows compile & gameplay del fix non testati.


### 0.13.45_05: collegamento calendario OG alla firma (sperimentale)
- Dopo l'aggancio alla tabella OG residente, la nuova `Expanded_OGWorld_PositionClubCalendar` valida tutti gli stride del `TeamSelectionKeyCache` originale e posiziona indice e offset sulla prima fase non precedente alla data mondiale; non esegue alcuna partita. Eventi storici delle coppe e risultati individuali del club richiedono un archivio di key-cache: non è ancora completo il loro ripristino.
- I mondi supportati sono istanziati quando l'utente preme START. Il test per un club dopo FORWARD TIME deve verificare che la sua prima partita non sia una gara già svolta, che gli incontri OG e i punti della lega non vengano risimulati e che il calendario civile non salti avanti. Snapshot e salvataggio sono ancora in verifica.
- REGRESSIONI DA TESTARE: corretto ripristino degli eventi di coppa e delle fixture multi-ciclo, overlay manager, riassunzioni, cambio stagione, carriera con squadra iniziale, 2026/27, squadre con ID multipli, save/reload. La compilazione MSYS2 non è stata eseguita qui.

## 14 novembre 2026 — 0.13.45_06: viewer OG senza side effect e audit mercato
- [ANALISI VERIFICATA] `menus.c::ViewWorldMenu` seleziona una competizione e chiama `career_init.c::Career_ViewOrEditCompetitionSlot`. PRIMA: se non apparteneva agli slot attivi, il ramo `Not in any active slot` chiamava `Career_AutoAdvanceCupCompetitionFromRecord`, `Career_AutoAdvanceLeagueCompetitionFromRecord` o `Career_AutoAdvanceInternationalCompetitionFixtures`. Queste routine inizializzano da definizioni OG e possono simulare partite via `Career_AutoAdvanceCupMatches` -> `DIY_PlayCupMatchAndAdvance(0,0)` e `Career_AutoAdvanceLeagueFixtures` -> `DIY_PlayOrSimulateLeagueFixture(0,0)`. NON dimostrato che ogni apertura cambiasse un risultato, ma il percorso poteva mutare lo stato.
- [CODICE MODIFICATO] Il ramo esterno SDL non chiama più auto-advance; `Expanded_OGWorld_ViewLeague` copia il contesto nativo residente (`s_expOG`) nello scratch OG e apre il menu lega in `viewOnly`, ripristinando scratch, team e RNG. Se nessuna competizione residente corrisponde in modo univoco (anche per ID multipli), mostra stato non disponibile, senza inventare risultati. `DEBUG WORLD` si trova nell'entry 11 della schermata UNEMPLOYED; entry 12 RECORD e focus FORWARD TIME rimangono. Coppe non residenti NON sono visualizzate come se già giocate.
- [OG WORLD ESISTENTE] `career_init.c::Expanded_OGWorld_Start/AdvanceToDate` attiva contesti di lega a START e li porta per fase alla data del disoccupato. `career_status.c::Expanded_OGWorld_AdvanceLeague` conserva `DIYCompetitionEx`, `DIYTournamentState`, squadre, RNG e `lastPhase`; `Expanded_OGWorld_BindClub` copia classifica OG ma NON l'intera key-cache storica delle gare del club. Coppe internazionali/nazionali per data, archivi risultati individuali e save completo rimangono PENDENTI.
- [RNG/RECORD] `Career_AutoAdvanceCupCompetitionFromRecord` riceve il record DI DEFINIZIONE della competizione, inizializza `Randomize2` da byte record e stagione e chiama l'algoritmo OG di simulazione: non è una funzione di sola lettura dei risultati. Seed uguale non garantisce identità se rose o condizioni sono mutate; la visualizzazione non deve risimulare.
- [INFORTUNI] `match_sim.c::CalculateViewResult` chiama `ApplyRandomPlayerStatusAfterSim` su entrambe le squadre, applicando i campi OG `cardsInjuries` per le squadre eleggibili; controllare che tali stati si conservino a livello mondiale. `career_status.c` contiene recupero infortuni e squalifiche.
- [MERCATO] `transfer_market.c` include `UpdateHomeMarketPlayers`, `UpdateForeignMarketPlayers`, `GenerateRandomTransferOffers`, `Career_AcceptOffer_BuyPlayer`, `Career_AcceptOffer_SellPlayer`: gestione mercato e contratti OG esiste, ma NON prova trasferimenti autonomi fra TUTTI i club IA. Analisi richiesta prima di implementare finestre o eventi IA. Nuova regola: accordi possibili a mercato chiuso con efficacia alla finestra seguente; svincolati immediati, date e finanze da decidere.
- [TEST] Connettore GitHub e controlli testuali, NON compilazione MSYS2/Windows o verifica gameplay. Test obbligatorio: START UNEMPLOYED -> DEBUG WORLD -> lega con PL > 0 -> aprire due volte senza incremento risultati/data, FORWARD TIME -> avanzamento OG, firma -> stessi punti e punteggi; coppe senza contesto = indisponibili; save/reload e ID multipli NON ancora provati.

## 15 novembre 2026 — 0.13.45_07: modal-return in UNEMPLOYED e root cause dei pulsanti fantasma
- [CODICE VERIFICATO] `sorgenti/src/menu_system.c::RestorePreviousMenu` richiama `UnpackMenu(oldPackedMenu,_currentMenu)`, ripristina il selectedEntry e chiama `menu->afterDraw` (il callback onReturn dei menu packed), SENZA rieseguire `menu->onInit`. `sorgenti/src/menu_data.c::Expanded_UnemployedCalendarMenu` in 0.13.45_06 aveva `hdr.onReturnPtr=GlobalRetn`. Pertanto, dopo ogni `ShowMenu` nidificato, il clone perdeva il blocco di SQUAD/TRANSFERS/altre entry operato solo in `Expanded_UnemployedInit`.
- [CONSEGUENZA] Gli hotspot invisibili del Contest OG potevano tornare selezionabili o invocabili col mouse: sintomo riferito dall'utente, apertura involontaria di SQUAD senza club e crash. La relazione precisa col crash richiede log Windows, ma la riattivazione di callback era determinata dal codice.
- [FIX] onReturn=Expanded_UnemployedInit; la nuova Init usa una whitelist di cinque azioni reali (7 FORWARD TIME, 9 BACK, 10 VIEW CLUB OFFER condizionale, 11 DEBUG WORLD, 12 RECORD) e revoca `onSelect`, `beforeDraw`, `afterDrawCb` su 44 entry prima di abilitare i soli pulsanti autentici. Un gate nel MenuProc rifiuta ogni ordinal diverso o disabled/invisible, compresi click/joypad inattesi. Non altera il packed `_contestMenu` originale, solo il clone.
- [RENDER] DEBUG WORLD nella colonna destra, sopra RECORD: `x=entry12.x, y=entry12.y-entry12.dwHeight-1`, 100x12 come RECORD; tipi `ENTRY_FRAME_AND_BACK_COLOR` + `ENTRY_STRING`, stringa DEBUG WORLD e callback dedicata.
- [GUARD OG] `Expanded_OGWorld_ViewLeague` accetta solo contesti di lega residenti, con 2..32 squadre, mapping `teamSelOffset` e offsets della classifica validi prima di aprire il viewer OG. Quando il viewer ordina lo scratch, `DIY_SortLeagueTableByStandings` non aggiorna la cache delle classifiche delle offerte. Nessun auto-advance eseguito da VIEW WORLD.
- [ZERO PUNTI] 01/07/2026, fase iniziale = possibile zero partite disputate per ogni squadra; non e' prova di errore. Confrontare la data WORLD, la fase e il numero `native-matches` del log dopo FORWARD TIME. Non costruire risultati artificiali per popolare la vista.
- [VERIFICA] Controllo statico e GitHub, NO compilazione Windows eseguita dall'assistente. Test di sicurezza input, modali, viewer ripetuto, dati avanzati e save ancora da effettuare.
