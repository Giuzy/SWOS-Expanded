## AGGIORNAMENTO 20/11/2026 — REGOLA FUTURO 2627 E GUIDA IMPLEMENTAZIONE
Il presente documento conserva l'audit **storico 217** delle date/competizioni reali al 01/07/2026. Da decisione successiva, nel mondo **2627** il **formato e i giorni/mesi fissati per il 2026** si ripetono nelle successive edizioni di **gioco** finché un aggiornamento del progetto non cambia la definizione: non aspettare date ufficiali future impossibili da conoscere, ma distinguere `REAL_2026` da `2026_GAME_TEMPLATE`, aggiornare partecipanti/qualificati via OG, controllare tutte le collisioni. **1996/97:** preferenza di seguire le vere riforme storiche negli anni corretti, modalità da deliberare. Per istruzioni su catalogo, validatori, residenti type0/1/2, fixture, KO, CAR, viewer, regressioni, incidenti reali 217–225 e checklist vedi **[COMPETITION_ENGINE_IMPLEMENTATION_GUIDE.md](COMPETITION_ENGINE_IMPLEMENTATION_GUIDE.md)**. Niente delle note di audit 217 qui sotto equivale a una futura gara CONMEBOL già giocabile.

# SWOS Expanded — audit calendari OG, fotografia al 1 luglio 2026, cicli autonomi delle competizioni

**Data audit:** 20 novembre 2026. **Ramo di lavoro:** `dev/0.10`. **Base verificata:** `7974000cc0a4a8c53250b88b4139b0bf296c1503`, `00.13.216`. **Patch parziale:** `00.13.217`. **Stato:** ricerca e protezioni reali limitate, **NON Libertadores 2027 giocabile**. L'utente compila e testa su Windows.

## Decisione approvata dall'utente

1. Lo stesso mondo OG genera risultati, classifiche, gare e titoli. Non introdurre LWE simulatore parallelo; il coordinatore Expanded può gestire calendario civile, dipendenze e CAR.
2. Il manager cambia anno amministrativo al 1 luglio, ma **NESSUNA competizione si resetta per questo solo fatto**. Ogni edizione ha identità mondo+competizione+stagione/anno, data di apertura e conclusione OG autonomi. Gare, tabelloni, qualificati, RNG e dati salvati sopravvivono a qualunque 1 luglio fino alla vera conclusione.
3. I partecipanti alla Libertadores 2027 dipendono dai veri risultati nazionali 2026 (campionati, coppe, classifiche aggregate, titoli e riallocazioni). Un trofeo concluso a novembre/dicembre 2026 manda alla **Libertadores 2027**, non a quella 2026, anche se il calendario di carriera ha iniziato l'anno 2026/27.
4. La carriera moderna inizia **01/07/2026**. Ogni evento già svolto va importato dal **risultato storico verificabile al cutoff**, con club/score/fase reali; non può venire simulato dopo START né ripetuto. Dove manca lo storico preciso, segnalare **HISTORIC_SNAPSHOT_MISSING** e NON inventare risultato, vincitore o sorteggio. Tutti i match successivi al cutoff saranno OG, incluse le partite del manager.
5. Prima edizione: **Belgrano Córdoba** è l'unico titolo argentino Apertura 2026 già confermato come dato fisso qui: finale 24/05/2026 Belgrano–River Plate 3–2, ID DB `43:21` e `43:3`. Per la **tabla anual** servono i dati verificati del **girone regolare dei trenta club**, NON la tabella cumulativa con i playoff (RSSSF riporta 20 gare di Belgrano e soltanto 16 nel girone). I punti della prima fase non vanno inventati né assegnati in assenza della mappatura completa. Clausura e coppe non concluse al 01/07 sono future OG.
6. Questo vale anche per Paraguay e per tutte le competizioni che attraversano stagioni. Non rimuovere i passaggi OG di promozione/retrocessione o i selettori delle coppe; separarli dal rinnovo della stagione amministrativa.

## Matrice CONMEBOL delle 11 coppe nei dieci paesi (non soltanto sei)

**Il dato della colonna OG prima della patch è agosto 2026–maggio 2027 (`beginMonth=56, endMonth=32`), tranne Copa de la Liga cilena `56,16`.** Ogni mese ha otto fasi OG: gennaio=0, luglio=48, agosto=56, novembre=80, dicembre=88; una fase `87` corrisponde all'ultima fascia di novembre. Questi byte indicano una **finestra**, non automaticamente la data precisa della finale.

| Paese | Coppa | Stato reale al 01/07/2026 | Termine 2026 verificato/programmatico | Posto Libertadores 2027 | Stato integrazione |
|---|---|---|---|---|---|
| Argentina (43) | Copa Argentina | Gia' iniziata prima di luglio | Dicembre (programma aggiornato) | Campione con riallocazioni | **Storico match/fixture non importato** |
| Bolivia (45) | Copa Bolivia | **Non iniziata**, parte 19/08 | **25/11**, finale andata/ritorno | Campione (Bolivia 3; riallocazione se necessario) | **Modificata solo fascia OG 56→87**, fixture/runtime Windows da verificare |
| Brasile (46) | Copa do Brasil | Gia' iniziata nel primo semestre | **06/12**, finale unica | Campione e finalista: controllare le regole di posti | **Storico non importato; formato reale molto piu' ampio** |
| Cile (48) | Copa Chile | Gia' iniziata **30/01** | Data finale da verificare | Regola coppa + eventuale spareggio | **Storico e formato non importati** |
| Cile (48) | Copa de la Liga | Gia' iniziata **20/03** | **14/11** | Vincitore: Chile 3 | **Storico pre-luglio non importato** |
| Colombia (49) | Copa Colombia | Gia' iniziata da maggio | Calendario finale in definizione | **Non assegna direttamente** un posto Libertadores | **Storico non importato** |
| Ecuador (50) | Copa Ecuador | Gia' iniziata in aprile | Finale dicembre, giorno non ufficializzato al 20/11 | Campione con riallocazioni | **Storico non importato** |
| Paraguay (64) | Copa Paraguay | Gia' iniziata **02/06** | **06/12** | Campione, turno preliminare | **Storico e accoppiamenti non importati** |
| Peru' (65) | Copa de la Liga | Gia' iniziata **11/06** | **15/11** | **Nessuno**: 2 punti classifica 2027 al vincitore, 1 al finalista | **Storico non importato** |
| Uruguay (71) | Copa AUF Uruguay | **Non iniziata**, parte 04/08 | Finale da verificare | Uruguay 4 solo con requisiti di prima divisione | **OG 32 club, formato reale 40**; non indovinare finale |
| Venezuela (77) | Copa Venezuela | Gia' iniziata 24/06, poi ripresa da agosto | Finale dicembre, giorno da verificare | Regolamento quote ancora da verificare | **Storico pre-luglio non importato** |

**Avvertenza:** nel codice `kWorld2627_world*Cup` molte coppe hanno un numero di club e un formato OG inferiori alla vera competizione (per esempio la Copa AUF 2026 ne ha 40; Copa Caliente peruana 34). Allungare/spostare i soli `beginMonth/endMonth` **non importerebbe** i club mancanti, i gruppi e gli incontri reali. Non fare una falsa simulazione della parte gennaio-giugno in luglio-agosto per riempire quelle lacune.

**Registro machine-readable audit**: `data_json/worlds/2627/competitions/conmebol_2026_cutoff_audit.json`. È **solo audit**, non consumato dal runtime, non validato come calendario OG. L'unica modifica di definizione sicura già preparata è la fascia per **Copa Bolivia 2026**, che comincia **dopo** il cutoff e termina in novembre. Nessuna squadra nazionale o vincitore futuro è stato fissato.

### Fonti pubbliche delle date/regole (audit, non database di risultati)

- AFA/RSSSF, Apertura 2026: https://www.rsssf.org/tablesa/arg2026.html ; Copa Argentina: https://www.tycsports.com/copa-argentina/copa-argentina-2026-cronograma-de-partidos-y-todos-los-resultados-id706751.html
- FBF Copa Bolivia e accessi CONMEBOL: https://fbf.com.bo/temporada-2026-de-la-division-profesional/ ; calendario 19/08–25/11: https://erbol.com.bo/deporte/la-copa-bolivia-ya-tiene-hoja-de-ruta e https://diez.bo/futbol/la-copa-bolivia-ya-tiene-calendario-para-definir-a-su-campeon-2026_1791576050
- CBF, finale 06/12: https://www.cbf.com.br/futebol-brasileiro/noticias/copa-brasil/copa-brasil-masculino/a/brasilia-recebe-final-da-copa-do-brasil-2026
- Cile: https://www.cooperativa.cl/noticias/site/artic/20260122/pags-amp/20260122173507.html ; https://www.campeonatochileno.cl/competition/copa-de-la-liga/
- Colombia: https://caracol.com.co/2026/04/13/copa-colombia-2026-formato-del-campeonato-y-fechas-en-las-que-iniciaria/
- Ecuador: https://www.primicias.ec/deportes/futbol/cupos-copa-libertadores-equipos-ecuador-clubes-clasificacion-136417/
- Paraguay APF: https://www.apf.org.py/noticias/esta-es-la-hoja-de-ruta-de-la-copa-de-todos-2026 ; https://www.apf.org.py/noticias/el-estadio-la-huerta-recibira-la-final-2026-de-la-copa-paraguay
- Peru', Copa Caliente de la Liga (NON Copa Peru' dilettanti): https://andina.pe/agencia/noticia-copa-de-liga-2026-premio-recibira-campeon-esta-novedosa-competicion-1075293.aspx
- Uruguay AUF: https://auf.org.uy/copa-auf-uruguay-2026/
- Venezuela FVF: https://www.fvf.com.ve/articulos/vuelve-la-copa-venezuela-la-fvf-anuncia-el-inicio-de-la-edicion-2026

## Architettura futura: reset per competizione OG, NON per compleanno del manager

**Decisione approvata, codice ancora da implementare:**

- Registry residente chiave `(world, competition identity, edition year)`, con fase/partita OG e data civile. Per il campionato argentino conservare identità Apertura/Clausura, tabla anual e relativa classificazione, senza seconda lega artificiale; Paraguay analogo secondo regolamento.
- `COMPETITION_FINISHED` deve provenire dalla transizione originale corretta per il tipo: nelle coppe `competitionFinished`, nelle leghe `minSubstitutes`/progressione e fasi native, per continentali anche il vero tabellone finale. Terminare **una volta sola**; chiamare a quel punto gli originali algoritmi OG per promozioni, posti, trofei e record storici. I nuovi draw iniziano alle proprie date da quote già certificate.
- Il 30/06→01/07 cambia solo il manager year e lo stato amministrativo. Gli stati OG vivi restano nella tabella anche se la loro chiave di edizione precedente differisce dal nuovo anno del manager. Il dispatcher deve avanzare gli eventi della competizione rimasta viva col corretto anno/fase, senza crearne un duplicato con il nuovo anno.
- Persistenza CAR: salvare tutti i contesti OG vivi, calendario future fixture, squadre, RNG, fasi, timestamp, identità, classifiche, vincitori, quote e anti-replay; compatibilità salvataggi vecchi; proteggere il CAR precedente al primo fault. In archivio storico chiuso restano dettagli minimi fino a esaurimento delle dipendenze di qualificazione.
- Modifica di date a singola fase/andata/ritorno solo a fixture future non giocate e con validazione di ordinamento, sorteggio, dipendenze e SAVE; mai ricalcolare retroattivamente le date dei risultati già ottenuti. Non spostare la data di chiusura del campionato semplicemente per comodità di febbraio.
- **Regressioni:** NEW CAREER con/senza club, FORWARD TIME, FINISH SEASON, cambio club, esonero, CLOSE/CONTINUE, due mondi, tre transizioni luglio; tutti i manager qualificati devono poter giocare, i non qualificati avere viewer read-only, non duplicare fixture.

## Verifiche mirate preparate 00.13.217

1. **OG viewer:** `career_status.c` calcola fingerprint FNV prima/dopo apertura dei viewer LEAGUE/CUP/TYPE2, oltre al contatore match. Eventuale byte del residente cambiato: log `[og-viewer-check] ... unchanged=0`, ritorno fallito. Il controllo misura mutazioni del **contesto residente**, NON costituisce prova che la navigazione Windows funzioni.
2. **OG 0x5555:** `diy_competition.c` verifica la lunghezza/offset prima di leggere il segmento di scratch; blocca tramite `Expanded_YearEndReportFault` nel percorso CAREER year-end invece di scrivere fuori buffer. I log includono la somma positiva/negativa e gli ID globali che muovono promozioni/retrocessioni. **Non vengono inventate promozioni per azzerare il checksum**. La causa esatta del precedente `OG_MOBILITY_BALANCE_5555` resta da riprodurre su Windows, quindi NON dichiarare risolto.
3. **Calendario Bolivia:** `world_data.c` cambia SOLO `kWorld2627_worldBolCup` da endMonth 32 (maggio 2027) a 87 (fine novembre 2026); start 56 (agosto) intatto. Non cambiare il mondo classic o altri cup record senza storico reale al cutoff.
4. **Libertadores 2027:** `ct_world_first_ready()=0`, `ct_round` non risolve fixture giorno per giorno, Q1/Q2 mancano assegnazioni e sorteggi approvati. Nessun falso PASS, nessun CLUB MANAGER ammesso dal nulla. Risolvere queste dipendenze prima di attivare il torneo residente e il viewer.

## Audit generale date — obbligatorio per TUTTE le competizioni (lavoro successivo)

Inventariare leghe, coppe domestiche **anche quelle con preliminari**, coppe internazionali, qualificazioni nazionali/continentali e tornei speciali per **entrambi** i mondi. Per ogni `world,competizione,edizione,fase` registrare fonte/regolamento e versioni, data civile precisa o intervallo OG, calendario di gare, origine dei partecipanti, stato storico al cutoff, rischio 1 luglio, spostamenti approvati, verifica field OG reale (id squadra, sottopartita, turno, non due volte), e prova SAVE→CONTINUE. Le sole fasce native non bastano a qualificare un calendario come 'reale'; i manifest non equivalgono a fixture residenti. Tenere una coda per gare preliminari delle coppe nazionali e per le altre coppe CONMEBOL/UEFA/CONCACAF/AFC/CAF/OFC; ogni differenza porta a un TODO o a una decisione utente motivata, NON a un risultato inventato.

## Checklist di consegna

| Test | Stato |
|---|---|
| HEAD/versione e guide principali lette | SI |
| Dieci federazioni, undici coppe mappate per cutoff | SI (audit metadati; NON storico gare) |
| Banda Copa Bolivia corretta nel codice OG moderno | SI (statica) |
| Guard range 5555 e log signed delta | SI (statica) |
| Fingerprint stato residente aprendo viewer LEAGUE/CUP/TYPE2 | SI (statica) |
| Apertura argentina realmente residente con tabella a 30 club e titolo | **NO**, titolo noto solo nel catalogo storico |
| Coppe in corso al 01/07 con fixture/risultati autentici | **NO** |
| Qualificazioni Libertadores 2027 complete a 47 club | **NO** |
| Libertadores moderna 2027 giocabile con OG e continuazione oltre luglio | **NO** |
| Reset automatico a competizione conclusa, non al 1 luglio | **NO** |
| 5555 causa determinata/rimossa | **NO** |
| Nuovi test isolati compilati/eseguiti | **NON PROVATO** |
| Windows compilazione, gameplay, SAVE/CONTINUE | **NON PROVATO** |

**Non dichiarare completata la richiesta di una Libertadores funzionante sulla sola base di questo commit**. La documentazione registra anche le attività da riprendere prima di una release, non equivale a un test o a un algoritmo implementato.


## 20/11/2026 — definizioni corrette, motore da collegare

Per i 4 gruppi boliviani e le date reali 2026 dei campionati argentino/paraguayano vedere [COPA_BOLIVIA_ARG_PAR_2026_DATA_AUDIT.md](COPA_BOLIVIA_ARG_PAR_2026_DATA_AUDIT.md). Nuovi JSON sotto `data_json/worlds/2627/competitions/` (Bolivia: 16 ID, 4×4; Argentina e Paraguay: Apertura/Clausura distinti). **NON CONSUMATI DAL RUNTIME OG**: non sono un fix automatico del viewer, calendario del manager, qualificazioni o CAR. Non fingere che il cambio di mesi basti: le competizioni richiedono edizioni residenti vere. La data finale paraguayana aggiornata da APF è 30/11/2026.
