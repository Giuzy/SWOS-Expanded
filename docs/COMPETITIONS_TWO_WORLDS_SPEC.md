# SWOS Expanded — Copertura universale per epoca, club/nazionali, menu e prove
_Data decisionale del 16 novembre 2026. Preparato per 0.13.59; non è una certificazione di gameplay._

## Regola definitiva approvata

**Tutte** le competizioni realmente esistenti nella stagione storica pertinente devono avere un'unica implementazione OG nella carriera Expanded, indipendentemente dal fatto che un SWOS/mod le mostrasse in PRESET o CAREER. Non importare limiti artificiali della mod. L'interfaccia è unificata; sono i dati storici a scegliere nome, numero di posti, formula, stagione effettiva, qualificazioni e ricorrenza. Non creare due Champions, due Libertadores o due Intercontinentali per lo stesso evento. Distinguere sempre coppe per CLUB da coppe per NAZIONALI e tornei diversi pur con nomi simili.

Un solo mondo attivo per carriera, due database 1996/97 e 2026/27 caricabili senza riavvio (funzionalità futura, **NON** implementata da questo intervento). CONTINUE carica l'ultima carriera; LOAD CAREER espone i due salvataggi AUTO9697.CAR/AUTO2627.CAR; l'avvio di una nuova carriera non cancella automaticamente l'altra. Il caricamento cambia mondo senza reinizializzare le competizioni in corso, non mantiene *due orologi che avanzano insieme*: ciascun salvataggio ha il proprio mondo OG unico. La regola di portabilità e preservazione dei save prevale.

## Tassonomia obbligatoria

- **CLUB**: leghe, coppe nazionali, coppe di lega, coppe regionali/statali, supercoppe nazionali, confederali e interconfederali; il FIFA Club World Cup è diverso dalla FIFA Intercontinental Cup moderna.
- **NAZIONALI**: World Cup, Europei, Copa América, Gold Cup, Asian Cup, Africa Cup of Nations, OFC Nations Cup e le qualificazioni pertinenti. La NORTH AMERICAN CUP di SWOS è un torneo **per nazionali** (Gold Cup), non la CONCACAF Champions Cup per club.
- **STORICA/ATTUALE**: stessa identità competitiva con cambio di nome/formato = una sola competizione per mondo; competizioni cessate solo nel mondo in cui esistevano; qualunque torneo si gioca negli anni/mesi reali in cui era previsto, non annualmente a prescindere.

## Inventario continentale: club

| Regione | 1996/97 — esistenti, da coprire | 2026/27 — esistenti, da coprire | Identità / non duplicazione |
|---|---|---|---|
| UEFA | Champions League (allora già UEFA CL), Coppa delle Coppe, Coppa UEFA, Supercoppa UEFA | Champions League, Europa League, Conference League, Supercoppa UEFA | European Champions Cup SWOS = identità Champions; UEFA Cup -> Europa League. Nessuna Conference nel 1996/97; Coppa delle Coppe assente nel moderno |
| CONMEBOL | Libertadores, Supercopa Libertadores, Copa CONMEBOL, Recopa Sudamericana; Copa Interamericana inter-confederale | Libertadores, Sudamericana, Recopa Sudamericana; altre coppe regionali reali da censire | Una Libertadores per mondo; non confondere Supercopa Libertadores storica con le odierne supercoppe nazionali |
| CONCACAF | CONCACAF Champions' Cup; Copa Interamericana con CONMEBOL (quando effettivamente disputata) | CONCACAF Champions Cup, Central American Cup, Caribbean Cup, Leagues Cup | Gold Cup è **NAZIONALI**, non un duplicato della Champions Cup |
| AFC | Asian Club Championship, Asian Cup Winners' Cup, Asian Super Cup | AFC Champions League Elite, AFC Champions League Two, AFC Challenge League | Continuità del torneo principale, tornei secondari distinti per l'epoca |
| CAF | CAF Champions League/CAF African Cup of Champions Clubs secondo l'edizione, CAF Cup, CAF Cup Winners' Cup, CAF Super Cup | CAF Champions League, Confederation Cup, CAF Super Cup | La Confederation Cup nasce dalla confluenza dei due tornei secondari storici |
| OFC | Nessuna edizione della competizione continentale per club nel 1996/97; confrontare cronologia | OFC Champions League | Non inventare una OFC Champions League nel 1996/97 |
| FIFA/intercontinentale | Coppa Intercontinentale/Toyota Cup UEFA–CONMEBOL (e Copa Interamericana, diversa) | FIFA Intercontinental Cup annuale, 6 confederazioni; FIFA Club World Cup 32 club nel 2029 | Intercontinentale storica/attuale: una competizione per epoca; Club World Cup è torneo DISTINTO e non annuale |

La presenza in questa tabella è **obiettivo di design**, NON prova di una coppa residente in Expanded. Verificare denominazioni storiche e data effettiva per ogni stagione, per esempio l'intitolazione CAF dal 1997.

## Inventario continentale: nazionali

| Regione | Competizioni per nazionali (1996/97, quando programmate) | Competizioni per nazionali (2026/27, quando programmate) |
|---|---|---|
| Mondo | FIFA World Cup (prossima 1998) e qualificazioni | FIFA World Cup 2026 (se alla data del mondo), qualificazioni e successivi cicli |
| Europa | UEFA European Championships (Euro 1996/2000) e qualificazioni | UEFA EURO (2028) e qualificazioni |
| Sud America | Copa América e qualificazioni al Mondiale | Copa América e qualificazioni al Mondiale |
| Nord/Centro America | CONCACAF Gold Cup / NORTH AMERICAN CUP, qualificazioni WC | CONCACAF Gold Cup, qualificazioni WC |
| Asia | AFC Asian Cup, qualificazioni WC | AFC Asian Cup (2027), qualificazioni WC |
| Africa | Africa Cup of Nations e qualificazioni, WC qualifiers | Africa Cup of Nations e qualificazioni, WC qualifiers |
| Oceania | OFC Nations Cup e qualificazioni WC | OFC Nations Cup e qualificazioni WC |

**European Championships ≠ European Champions Cup**: il primo è Euro per nazionali; il secondo è Champions League per club. Usare cicli pluriennali e verificare le date reali: la definizione di torneo non equivale a calendario annuale.

## Coppe domestiche e supercoppe

La copertura domestica comprende tutte le 91 nazioni censite per 2026/27 (134 leghe/gironi, 101 coppe nazionali/lega nel file di riferimento), i 13 campionati statali brasiliani e la Coppa Italia Serie D; nel 1996/97, usare i tornei esistenti allora (es. Coppa Italia Dilettanti e non la futura Coppa Serie D autonoma).

Riferimento 2026/27 confermato dalla Mod 4.1.2: **Italian Super Cup, Spanish Super Cup, Turkish Super Cup, FA Community Shield, German Super Cup, Trophée des Champions, Saudi Super Cup, Campeón de Campeones, Brazilian Super Cup, UEFA Super Cup**. Queste **10** voci sono un inventario minimo, non un tetto. Censire anche le supercoppe nazionali realmente esistenti in ciascun paese del database, soprattutto in Asia e Africa, nonché le supercoppe continentali CAF e le eventuali AFC/CONCACAF/CONMEBOL/OFC storiche/attuali. Per ciascuna: istituzione, sospensioni/assenze in date specifiche, partecipanti, regole e data; **NON aggiungere una supercoppa automatica per ogni paese senza riscontro reale**.

Esempi di rischio di duplicato: UEFA Super Cup è **continentale** e non una supercoppa nazionale; CAF Super Cup non deve essere duplicata nello stesso mondo; COPA INTERAMERICANA è distinta dalla FIFA Intercontinental Cup; il Campeón de Campeones non equivale alla CONCACAF Champions Cup.

## UI: riferimento provvisorio approvato

Schermate PRESET (prime 6 immagini) e CAREER successive nella chat del 16 novembre; archivio esterno alla repo: `SWOS_EXPANDED_RIFERIMENTI_COMPETIZIONI_UI_SAVE_2026-11-16.zip` e testo `SWOS_EXPANDED_GUIDA_INTEGRAZIONE_MOD_E_MENU_2026-11-16.md`. Le immagini sono RIFERIMENTI, non asset approvati da copiare 1:1. Identica navigazione provvisoria: continente → paese → competizione → dettaglio OG residente; stessa distinzione club/nazionali e stesse competizioni della carriera; focus/hover e BACK per un livello. OPTIONS, MONDO, CONTINUE e LOAD restano voci distinte, CLASSIC MODES resta al primo livello.

VIEW COMPETITIONS dall'OFFERTA: eliminare la galleria testuale fittizia di nomi inserita nella 0.13.57; aprire direttamente il selettore OG in sola lettura. A START, una coppa OG residente è consultabile anche senza gare disputate e **anche se il club offerente non partecipa**. Il viewer non avanza orologio, RNG, turno o classifica; BACK dal dettaglio torna all'elenco del paese e poi all'offerta.

**Nuova scoperta dalla Mod 4.1.2 effettiva (16/11):** in `tools/conti.c` (sorgenti ZIP locali) il parser legge un blocco aggiuntivo `CT_S1` con firma `S1`; `ct_new1(mgr)` costruisce la **prima edizione** da quei partecipanti e dal suo seed senza attendere `CT_IN` dell'anno precedente. E' un ampliamento della Mod 4.0.6/4.1.1 ASSENTE dal port `world_conti.c` di Expanded, che legge `s1h` (solo i vincitori seed) ma non `s1e` e non espone `ct_new1`. `world_data.c` espone `kWorld2627ContiSpec[4536]`, NON un blocco `CT_S1` dedicato. Questo è il **blocco specifico** per la Libertadores a START. Per portarlo correttamente servono i dati binari `CT_S1` della Mod 4.1.2 (potenzialmente estraibili da `ENGLISH.EXE` installato) e audit formato/versione, prima di collegare OG/manager e salvataggio. Il codice attuale non deve inventare iscritti o dichiarare la Libertadores residente.

## Stato tecnico fotografato alla 0.13.58 e tentativo 0.13.59

- Osservato nei log Windows precedenti: 134 contesti OG per leghe e 101 contesti per coppe domestiche iniziali in 2026/27. Sono evidenze d'avvio, non prova della completa correttezza di tutti i 91 paesi né dei save.
- Le coppe UEFA moderne hanno tre record type-2 OG con **36 ingressi reali per la prima stagione** (`world_data.c`: `World_LeaguePhaseCups`). La 0.13.59 tenta di renderle residenti in `career_status.c` con il normale `InitDIYLeagueFromRecord`, `Career_AutoAdvanceCompetitionSlots`, snapshot OG/RNG/roster e avanzamento per fase chiamato da `expanded_og_world_run_year`. **Nessun nuovo motore di punteggi**.
- La Mod ha già `ct_world_prepare` e `ct_world_advance_month` (importati nella 0.13.56, persistono nella 0.13.58): solo il primo è preparazione senza giochi; il secondo usa le simulazioni originali della Mod e richiede `CT_IN` correttamente popolata. **Non richiamarlo senza qualificati reali**, né sovrascrivere un evento OG già giocato.
- **Libertadores 2026/27 NON ANCORA RESIDENTE:** `CT_SPEC` è disponibile, ma mancano i partecipanti dell'edizione iniziale restituiti dallo stato OG/pre-stagione precedente e la sincronizzazione fra percorso umano e risultati OG. Non riusare i 20 iscritti del 1996/97 né inventare risultati. Un log diagnostico deve dirlo.
- Ingresso club allenato nelle coppe UEFA e relativi slot OG, qualificati della stagione 2027/28, conservazione integrale degli stati in CAR, calendario per-fixture, vincitori e continuità multiannuale: **NON PROVATI**.
- Rollback OFFERTA: rimosso il passaggio dalla pagina sintetica `s_previewMode==3`; il resto del picker resta OG di sola lettura; i test Windows controlleranno la navigazione con B e la visibilità di club NON partecipante.
- I tre stati UEFA sono **solo una prima integrazione da testare**, NON dichiarare “tre coppe funzionanti” prima di un log Windows che mostri fixture, avanzamento, risultati e restore senza doppi incontri.

## Verifica Windows per questa e le prossime build

L'utente sincronizza GitKraken e compila personalmente Windows. Usare `AVVIA_DIAGNOSTICA.bat` e i quattro BAT ripristinati (`ANALIZZA_SWOS_LOG`, `TEST_OG_WORLD`, `TEST_OG_70`, `TEST_OG_CUPS_SCORERS`); il runner NON usa più i test LWE storici come autorità. Test statici ≠ risultati OG. Verificare OFFERTA prima/dopo ingaggio, DEBUG WORLD, F12, due WORLD in **sessioni separate** e SAVE/RESTORE; test unico con cambio mondo senza riavvio sarà aggiunto **solo dopo** aver implementato il caricamento simultaneo.

Checklist per ogni coppa: record → iscritti reali → presenza a START → prima data → fixture OG → risultato/arcore/qualificazione → visita read-only invariata → salvataggio → ripresa → nuovo anno → partecipazione manager, distinguendo SI / NO / NON PROVATO. Il test 70 anni NON sostituisce un test manager Windows.

## Riferimenti tecnici e interruzioni

- Baseline 0.13.58, `dev/0.10`, HEAD `0875b8315c6b152ed5bd816efb2f9e03d42d753c` (verificare prima del commit).
- 0.13.56 commit `3873a630b4c92f155bd5b70f4024d17234ce74a3`: separa preparazione e avanzamento `ct_world_*`.
- Interruzione di connessione/chat il 16/11/2026: è stata persa una parte della conversazione, NON il codice delle `ct_world_*` già nel repository. Non reimplementarle.
- Fonti primarie: `docs/WORLD_2627_MOD_4_1_2_INVENTORY.md`, `docs/reference/mod2627_4_1_2_competitions.tsv`, `docs/reference/mod2627_4_1_2_supercups.tsv`, `sorgenti/src/world_conti.c`, `sorgenti/src/world_data.c`, `sorgenti/src/career_status.c`, `sorgenti/src/expanded_career.c` e sorgenti Mod 4.1.2 (MIT) inclusi nell'archivio della conversazione.


### Aggiornamento 16/11/2026 — formati e CT_S1

OpenSWOS 1996/97 e OpenSWOS 2026/27 hanno **formati diversi**: preservare Cup Campioni/Coppa Coppe/UEFA storiche nel primo; Champions/Europa/Conference tipo-2 a 36 club (8/8/6 giornate e tabella unica) nel secondo. `LeaguePhase_InstallCups` sostituisce i template europei solo nel mondo moderno. La Mod 4.1.2 fornisce il blocco prima-edizione `CT_S1` per 12 coppe extraeuropee, letto da copia locale per evitare distribuzione di dati non MIT. **Il solo catalogo visibile non dimostra residenti o salvataggi funzionanti**. Analisi, offset, SHA, stato e regressione 96/97 in [COPPE_OG_CT_S1_OPEN_SWOS_2026-11-16.md](COPPE_OG_CT_S1_OPEN_SWOS_2026-11-16.md).
