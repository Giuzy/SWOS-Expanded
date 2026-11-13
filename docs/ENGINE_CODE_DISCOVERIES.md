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
