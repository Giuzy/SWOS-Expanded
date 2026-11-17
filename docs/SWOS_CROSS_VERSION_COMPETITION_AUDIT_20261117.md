# Audit per competizione fra SWOS Expanded e le versioni di riferimento — 17 novembre 2026

**Snapshot:** `Giuzy/SWOS-Expanded`, ramo `dev/0.10`, codice 0.13.78 (HEAD `6392b98a67c372dd2feb743cd42048679f447d1b`); OpenSWOS upstream `main` **`1582e90505bf83c5f34c726812f2667be60a189a`**; Mod fonte **tag `v4.1.2`** (ref tag `6d8908f62f74377b68f06818e72b304f13cf8a05`). **Intervento esclusivamente documentale:** non modifica versioni, codice, calendari o salvataggi. **Non è un collaudo gameplay Windows.**

**Matrice completa e filtrabile (494 voci): [`reference/SWOS_CROSS_VERSION_COMPETITIONS_20261117.tsv`](reference/SWOS_CROSS_VERSION_COMPETITIONS_20261117.tsv)**. È l'elemento da consultare per **ciascuna** competizione, non una lista campione. Il file contiene 264 voci Mod moderne, 91 divisioni + 76 coppe + 14 tornei OG 1996/97, 17 estensioni storiche richieste, 10 supercoppe moderne, 4 ulteriori competizioni moderne da censire e 9 famiglie nazionali *per ciascun mondo*: **494 righe**. I record OG originali e quelli della Mod sono intenzionalmente separati e possono sovrapporsi come identità; **494 non è il numero di competizioni distinte** e **264 non significa 264 residenti funzionanti**.

## Interpretare correttamente tutte le colonne

- **Mondo, tipo, paese, competizione, record/id**: identità nominale ed epoca; una stessa nazione può avere più divisioni parallele, ed una competizione storica cambiata di nome non è un doppione da simulare.
- **Squadre, gironi/fasi, formula, promozioni e retrocessioni Expanded**: campi di `data_json/competitions/leagues.json` (96), `data_json/worlds/2627/competitions/leagues.json` (26), `data_json/competitions/cups.json`, `tournaments.json`; per cup/league cup moderna il record è in `world_data.c` / `world_cups.c`, la regola testuale è riportata dalla Mod ma **non è stata ricostruita la struttura interna di ogni record di coppa**. Il valore `promoted` / `relegated` è **il campo numerico grezzo OG**: non equivale automaticamente a *promozioni dirette* dichiarate nel testo di un regolamento. Un confronto numerico diverso non è prova di un bug senza controllare playoff e playout.
- **Formato e valori Mod**: tutte le **264 voci del manifest** hanno ora un campo di formula testuale ricondotto a fonti pubblicate: 234 associazioni paese/titolo dal documento ufficiale `world-2026-27-competitions.md`, una divisione Taiwan risolta nello stesso documento, e altre 29 voci (13 statali, 1 Coppa Serie D, 3 UEFA e 12 continentali) dal documento ufficiale `notes-world-2026-27.md` e dal modello delle 3 UEFA. Le squadre indicate come `(formula)` sono calcolate da gironi espliciti (es. 2×6=12) e **non** da una lettura di squadre del motore residente. `NV` rimane per i dati che la fonte non dà (soprattutto singole partecipanti/preliminari, dati delle altre versioni e contenuti PRESET). I 12 tornei extra-UEFA hanno dunque formule Mod per riga, ma non prova di residenza Expanded. Le statistiche NUMERICHE per le leghe estratte senza interpretazioni automatiche di playoff sono: **70** dimensioni squadre comparabili, nessuna discordanza; **26** valori di promozione comparabili, 8 divergenze numeriche; **30** retrocessioni comparabili, 13 divergenze numeriche. Gli altri non sono automaticamente «uguali».
- **Base 96 e differenze 96→26**: confronti solo sullo stesso ID-paese e sulla posizione della divisione nel record, **non garanzia che l'ordinalità identifichi lo stesso campionato storico**. Le 26 nazioni/record presenti solo nella banca moderna non provano un'assenza nel calcio del 1997 o nelle estensioni della Mod storica.
- **SWOS 2020 CAREER/PRESET, OpenSWOS CAREER/PRESET, Mod CAREER/PRESET**: sono **sei colonne distinte**. `NV` = prova per quella voce non disponibile, **non** «assente». `EREDITATA` = record derivato dal gioco OG, non conferma visiva/menu; `DOC_CARRIERA` = competizione elencata in un mondo-carriera del produttore, non esecuzione Windows; `Mondo_4.0.5_compatibile` = OpenSWOS dichiara integrazione della Mod **4.0.5**, non prova della stessa precisa versione **4.1.2** in tutte le voci. **Non sono stati attribuiti «SI in PRESET» non sostenuti da evidenza per singola competizione.**
- **Expanded 1996/97 DB, Expanded 2026/27 DB, OG/world e esito**: «presente» significa **record/codice**, non necessariamente competizione residente con match e save. Per le voci già definite ma senza la catena completa il giudizio richiesto è **PRESENTE MA INCOMPLETA**; per le aggiunte non censite è **DA CENSIRE/DA INTEGRARE**, senza chiamarle falsamente presenti.

## Confronto tecnico dei due mondi Expanded

| Categoria | Expanded 1996/97, base | Expanded 2026/27 / Mod 4.1.2 | Conseguenza |
|---|---|---|---|
| Record campionati | 66, incluso CUSTOM; 91 divisioni, incluse 3 CUSTOM | 91 record per 91 paesi; 134 divisioni | 26 record solo nella banca moderna; NON assumere stesso numero di club e formule nei due mondi |
| Coppe domestiche/di lega | 76 record `cups.json` **complessivi**: comprendono coppe internazionali, qualificazioni, friendly e altri, NON 76 coppe domestiche | 87 coppe nazionali + 14 coppe di lega = 101 | Non confrontare il 76 con il 101 come se fossero categorie omogenee |
| Altre voci moderne | Non trapiantare dal 2026 | 13 stati brasiliani + Coppa Serie D + 3 UEFA + 12 internazionali = 29 | Esistono specifiche e talune routine; 14 gareggiamenti statali/Serie D non residenti nel mondo OG |
| Supercoppe | Individuare edizioni realmente presenti nel 1996/97 | 10 censite fuori dai 264 | Nessuna residenza completa per ogni annualità certificata |
| Nazionali | Record OG storici di qualificazione/coppie | Mod 4.1 dichiara nazionali 2026/27 **non disponibili** | NON riciclare rose 1996 né inventare cicli 2026/27 |

### 21 divergenze numeriche da verificare, NON 21 bug accertati

Le celle evidenziate nella colonna `verifica_Expanded26_vs_Mod412` della matrice individuano:
- **Promozioni (8):** Championship inglese 3 nel campo OG contro 2 «up» diretti nel testo Mod; League One 3/2; League Two 4/3; Serie B 3/2; Segunda spagnola 3/2; TFF 1. Lig 3/2; Brasile Serie B 4/2; Giappone J2 3/2. **Possibile spiegazione da verificare:** il record OG rappresenta anche uno o due posti decisi ai playoff.
- **Retrocessioni (13):** Serie C italiana gironi A/B/C 3/1 (verificare posti diretti + playout); Serie D italiana gironi A-I 0/2 (verificare uscita verso Eccellenza e universo delle squadre); Liga Portugal 2 0/2. **Questi scarti non vanno «corretti» senza controllare la logica OG realmente usata e la regola pertinente al 2026/27.**

**Dati dimostrati dalla relazione record ↔ Mod:** i 91 paesi e i 134 slot-leghe combaciano *esattamente* fra il manifest Mod e le `divisions[]` Expanded (nessuna discordanza di quantità di divisioni o associazione ID-paese in questo audit). Questo NON prova ordine reale dei gruppi dopo promozioni, replay, calendario, coefficienti né retrocessioni nel gameplay.

## Verifica indipendente sui file di OpenSWOS upstream (nuovo controllo diretto)

Ho letto i file della **versione pubblica attuale** `benbaker76/OpenSWOS` sul branch `main` e li ho confrontati con `dev/0.10` di Expanded, non soltanto con le descrizioni.

| Fonte 96/97 | OpenSWOS upstream vs Expanded | Evidenza |
|---|---|---|
| `bin/data_json/competitions/leagues.json` ↔ `data_json/competitions/leagues.json` | **IDENTICI byte per byte** | Git blob SHA `dd3290f6f10e34a68d75a7086b9136552be7a5d7` |
| `cups.json` | **IDENTICI byte per byte** | `976b804fb06cbbe2353c24a84e7e8686965aa2f8` |
| `country_competitions.json` | **IDENTICI byte per byte** | `360e7b31f44a23c4fd42f7f99348cc7b1d6bd693` |
| `career.json` | **IDENTICI byte per byte** | `80216d5e3455fdb226d25c7c7b896f3e5b44d4e8` |
| `qualifying.json` | **IDENTICI byte per byte** | `f5a875cbb5eb5d53bae3d66ec897729acd765993` |
| `tournaments.json` | **DUE CAMPI DIFFERENTI** in un solo record, `_copaLibertadores`; tutti gli altri campi JSON equivalenti | OpenSWOS `comp.begin_month=112`, `comp.end_month=-120` (byte 136); Expanded `begin_month=16`, `end_month=40` |

**Conseguenza della divergenza Copa:** nel sorgente pubblico OpenSWOS la finestra OG della Libertadores cade **marzo–giugno 1997** (chiavi 112→136). Nel record Expanded è **marzo–giugno della prima annualità** (16→40) e la versione 0.13.75 la traduce nel driver del calendario per marzo–giugno 1997. Sono due realizzazioni *diverse* di una finestra desiderata simile. È necessario un test reale sulle fasi, soprattutto dopo SAVE e dopo cambio anno, prima di decidere se riutilizzare la codifica OG upstream. **Nessuna modifica di data è autorizzata/effettuata in questo audit.**

**Confronto dei sorgenti moderni:** `sorgenti/src/states.c` è byte-identico a `OpenSWOS/src/states.c` (SHA `615bc5be75...`), mentre `world_data.c`, `world_conti.c`, `league_phase.c`, `world_cupd.c`, `competition_data.c` **NON** sono byte-identici fra i due repository: non si può dedurre parità di formato/regole moderni solo dalla parentela del codice. Inoltre OpenSWOS documenta l'integrazione della Mod 4.0.5, mentre la matrice attuale usa la Mod 4.1.2. Questi scarti sono motivo per confrontare gli snapshot **della medesima versione** e le singole routine prima di classificare `CAREER/PRESET=SI`.

La matrice ora contiene la colonna **`verifica_dato_OpenSWOS_UPSTREAM`**, per dare il risultato di questo confronto su **ogni riga**, distinta dai sei campi CAREER/PRESET e dall'eventuale esecuzione effettiva nei menu. **Identità dei JSON non equivale a menù o gameplay collaudato.**

## 1996/97 — differenze rispetto alle versioni estese

- **Champions**: il database 96 ha record OG type-2 da 16 squadre in quattro gruppi iniziali; `_cupWinnersCup` e `_uefaCup` sono type-1, 32 iscritti iniziali ciascuna. Expanded ha residenti della prima edizione, ma input/calendario del manager, nuove qualificazioni e continuità interannuale restano da provare; «presente in PRESET» non prova «residente in carriera».
- **Libertadores**: record `_copaLibertadores` type-2 (20 squadre, **5 gruppi da 4**) presente nel dato 96. La prima edizione nella carriera Expanded ha ricevuto il collegamento residente e il riallineamento marzo–giugno, ma non ha il percorso completo di qualificazione/successione. **Non confondere** le 20 squadre del tabellone di gironi col formato storico della Mod 1997 **20 + River Plate detentore direttamente agli ottavi**: queste 21 identità richiedono verifica dei sei ID discordanti e dei dati/seed specifici dell'edizione successiva. Nessuna data aggiustata a posteriori.
- **Estensioni storiche Mod**: Serie C1/C2 italiane, Supercopa Libertadores, Copa CONMEBOL, Intercontinentale e altre coppe CONMEBOL/AFC/CAF/CONCACAF sono nella matrice come identità storiche separate, con `NV` dove non esistono record/ID binari indipendenti della Mod 1997 da controllare. README della Mod documenta esplicitamente l'integrazione in CAREER di Serie C1/C2, Libertadores, CONMEBOL, Supercopa e Intercontinentale. Non attribuire automaticamente lo stesso grado di conferma alle altre identità elencate nei requisiti.
- **Dimensioni e regolamenti**: il precedente audit aveva già identificato 48 record comuni con dimensioni differenti fra epoche e quattro file squadre problematici (Finlandia, Francia, Turchia, USA) corretti nella 0.13.69. Qui la matrice amplia il confronto ai **campi promozioni/retrocessioni/cicli/finestre** di ogni `divisions[]`, ma non dispone ancora della tabella binaria indipendente di SWOS2020 e della Mod storica per dichiarare identiche tutte le regole del 1997.

## 2026/27 — lacune di integrazione da non confondere con dati mancanti

| Competizioni | Evidenza del dato | Stato del mondo OG |
|---|---|---|
| 134 divisioni | Record divisi in 91 paesi, numero club e parametri OG per divisione | Prima stagione residente nelle nazioni supportate, verifiche per singolo club, girone, calendario e save pluriennale NON complete |
| 87 coppe nazionali + 14 di lega | Inventario Mod e record domestici Expanded | Prima edizione nominalmente residente; nessuna certificazione end-to-end di tutti i 101 casi |
| Paulista, Carioca, Mineiro, Gaúcho, Paranaense, Catarinense, Goiano, Paraense, Baiano, Cearense, Pernambucano, Alagoano, Mato-Grossense | `World_States` / Mod | **13 NON RESIDENTI** nel calendario mondiale; la presenza in altri menu non equivale a un torneo datato OG |
| Coppa Italia Serie D | `World_CupD` / Mod | **NON RESIDENTE** nel calendario mondiale, nonostante la gestione locale del club |
| Champions / Europa / Conference | Type-2 moderni con 36 club, 8/8/6 gare iniziali | Solo prima edizione residente; nuove qualificazioni, campi della stagione successiva e tabelloni nel save non certificati completamente |
| Libertadores, Sudamericana, Recopa, CONCACAF, AFC ×3, CAF ×3, OFC, Intercontinental | `World_ContiCups` / `CT_S1`, formati e numeri Mod v4.1.2 ora in tutte le 12 righe della matrice | Le 12 voci sono **catalogate / con routine**, NON 12 competizioni con prova completa START→fixture→risultato→qualificazione→save→nuova stagione; **Libertadores: 47 posti nella Mod vs 45 nella schermata iniziale da riconciliare**, senza dedurre falsi iscritti |
| CONCACAF Central American Cup, Caribbean Cup, Leagues Cup; FIFA Club World Cup | Identità ulteriori fuori dalle 264 voci | Da censire, confrontare e integrare nell'anno corretto; World Cup per club NON è l'Intercontinental annuale |
| 10 supercoppe censite + altre eventuali | Inventario indipendente; Mod spiega anche il primo anno e i casi di doppietta | Servono qualificati, annate, calendario, fixture, save; NON trasformare in coppa nazionale generica |

## CAREER e PRESET delle versioni di confronto: che cosa risulta davvero

| Versione/sorgente | 1996/97 CAREER | 1996/97 PRESET | 2026/27 CAREER | 2026/27 PRESET | Limite |
|---|---|---|---|---|---|
| SWOS 96/97 DOS/GOG originale | Funzione storica documentata, non replay per ogni riga | Funzione storica documentata, non replay per ogni riga | Non applicabile al database originale | Non applicabile | Mancano dump binari e sessioni di prova indipendenti |
| SWOS 2020/2021 | Disponibilità generale del modo CAREER; per competizione `NV` | Modo PRESET generale; per competizione `NV` | Versioni update 2016/17/2024/25 NON equiparabili al dataset Mod 2026/27 | `NV` | Nel repository ci sono asset SWOS2020, **non** il database competizioni installato da confrontare |
| OpenSWOS (`benbaker76`) | Base OG in codice; per riga `EREDITATA/NV` | Picker OG presente; selezione per singola riga `NV` | Supporta mondo 26, changelog dichiara dati e regole Mod **4.0.5** | Picker e mondo disponibili, catalogo esatto 4.1.2 `NV` | Occorre correlare commit OpenSWOS e versioni pacchetto della Mod |
| SWOS 96/97 Mod v4.1.2 | Documentate in CAREER anche estensioni del 1997 | Mod storico aveva estensioni PRESET; non testate per riga nella 4.1.2 | 264 voci elencate come mondo CAREER + 10 supercoppe | CLASSIC/PRESET disponibili come interfacce, ma `NV` su ogni singola voce 2026 | Fonte primaria Mod, non una prova automatica di ogni partita |
| SWOS Expanded 0.13.78 | Record + residenti OG parziali | Menu CLASSIC esistente; disponibilità e formula per ogni singola voce non provate Windows | Record + residenti OG parziali | Menu CLASSIC esistente; disponibilità e formula per ogni singola voce non provate Windows | **PRESENTE MA INCOMPLETA** vale per record definiti, non per nuove identità solo proposte |

## Altre versioni/edizioni esistenti: catalogo verificabile, NON confondere con i due mondi Expanded

La richiesta «tutte le altre versioni» è aperta: senza archivio di ciascun eseguibile, disco e database modificato non è possibile attestare ogni patch fan-made. Fonti storiche identificano le principali linee **SWOS 1994/95**, aggiornamento **SWOS 1.1**, **SWOS 95/96 / European Championship Edition**, **SWOS 96/97 originale** (Amiga e DOS/GOG), **SWOS 97/98 World Cup Update**. I nomi e l'esistenza delle edizioni NON forniscono i relativi roster e record competizioni, quindi le loro righe CAREER/PRESET rimangono `NV` in attesa di fonte verificabile. Non equiparare *Sensible Soccer 98* 3D a un'edizione del medesimo motore SWOS.

Le varianti documentate che **non sono 1996/97 né 2026/27** sono importanti ma vanno tenute in un inventario di mondi autonomo:
- **Mod Europe 1984/85 CAREER**: 32 paesi europei, 949 club; coppe nazionali in 15 paesi; Champions' Cup 32, Coppa delle Coppe 32, UEFA Cup 64, nazionali 143, qualificazioni Mondiali 1986/Euro 1988.
- **Mod CLASSIC SEASONS**: DDR 1988/89 (14 club, FDGB-Pokal); Serie A 1947/48 (21 squadre, 2 punti a vittoria, quattro retrocessioni).
- **Mod CLASSIC PRESET**: World Cup 1934 (16 nazionali); World Cup 1982 (seconda fase: quattro gironi da tre), Mitropa Cup 1934 e FA Cup 1871/72. Questi **NON** possono essere classificati come campionati 96 o 26 solo perché la Mod li mostra nel menu storico.
- **SWOS 2020 DLC, SWOS++, patch SWOS 2016/17 / 2024/25, editor Amiga 96/97**: versioni/pacchetti complementari documentati; non equivalgono a cataloghi CAREER/PRESET indipendenti verificati in tutte le stagioni. In particolare un patcher che modifica il numero di posti europei non implica una nuova definizione canonica dei tornei mondiali.

## Prove mancanti per dichiarare davvero «completato»

1. **Dump indipendenti e hash** di SWOS 2020/2021 (gioco + database e patch attiva), SWOS 96/97 DOS/GOG, OpenSWOS ai commit/versioni esatte, Mod storica 1997 e Mod 4.1.2: **solo per audit offline**; nessuna dipendenza runtime dall'EXE, nessuna copia di dati non licenziati nel progetto.
2. **CAREER e PRESET per ciascuna riga**: snapshot o lettura dei veri selettori di quelle versioni (incluso quando la voce è nascosta perché l'anno non è ancora attivo), con mondi 96/26 separati.
3. **Parità regolamenti**: confronto del parametro OG grezzo con playoff, playout, split dei 22 campionati moderni, qualificazione, coefficienti, bye, calendari domestici, stato di mercato e ID degli ingressi; le 21 differenze numeriche sono l'elenco iniziale, non il limite.
4. **Prova Windows con progressione e permanenza** per ciascuna competizione: START anche disoccupato → prima gara (data OG) → risultati/classifiche e eventuali gironi → qualificate → anno successivo → SAVE/CLOSE/CONTINUE → stessa identità di partite, senza risimulazioni o reset alla firma/esonero. La vista PRESET non supplisce alle gare residenti.
5. **Nazionali pluriennali** e supercoppe solo alle date reali della rispettiva stagione, comprese qualificazioni cross-year; non un nuovo torneo ogni luglio.

**Esito generale dell'audit documentale: PRESENTE MA INCOMPLETA.** Per le voci 2627 con record: `PRESENTE MA INCOMPLETA` significa «dati tecnici presenti, verifica integrata mancante». Le competizioni effettivamente non residenti sono specificate, mentre le nuove voci e le comparazioni senza fonte sono marcate `DA INTEGRARE`/`NV`. Non dichiarare «tutte le versioni confrontate completamente» finché non si acquisiscono le prove sopra.

## Fonti verificabili (versione e percorso)

**Interne Expanded:** `data_json/competitions/{leagues,cups,tournaments,country_competitions,career}.json`, `data_json/worlds/2627/competitions/leagues.json`; `docs/reference/mod2627_4_1_2_{competitions,countries,supercups}.tsv`; `docs/reference/COMPETITION_FORMAT_AUDIT.csv`; `docs/COMPETITION_AUDIT_SOURCE_MATRIX_20261117.md`; `docs/COMPETITIONS_TWO_WORLDS_SPEC.md`; `sorgenti/src/{world_data,world_cups,world_conti,world_cupd,states,league_phase,career_status,career_init}.c`.

**Esterni versione-specifici:**
- [Mod v4.1.2: descrizione completa dei campionati e coppe per paese](https://github.com/aminta/swos-9697-mod/blob/v4.1.2/release/world-2026-27-competitions.md); [note complete Mod mondo 2026/27](https://github.com/aminta/swos-9697-mod/blob/v4.1.2/release/notes-world-2026-27.md); [README Mod (estensioni 1997)](https://github.com/aminta/swos-9697-mod/blob/v4.1.2/README.md).
- [Mod 4.0.5: regole/prima stagione delle supercoppe](https://worldofswos.com/en/news/4.0.5/); [Mod 4.1: mondi e preset storici](https://worldofswos.com/en/news/4.1/); [altri mondi/classic](https://worldofswos.com/en/classic/).
- [OpenSWOS changelog: integrazione Mod 4.0.5](https://openswos.net/changelog.html); [sorgente OpenSWOS](https://github.com/benbaker76/OpenSWOS).
- [SWOS 2020/2021: patch/edizioni con engine diversi](https://swos2020.com/patches); [versioni storiche documentate dalla comunità](https://www.sensiblesoccer.de/forum/swos-related/2247-re-version-history-of-sensible-soccer-games); [manuale SWOS 1.1](https://files.swos.eu/manual/SWOS1.1/SWOS1.1manual.pdf).

*Valido al 17/11/2026, esclusivamente audit di codice/dati e fonti pubbliche; nessun Windows PASS aggiunto.*
