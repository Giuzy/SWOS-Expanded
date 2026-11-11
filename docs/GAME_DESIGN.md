# SWOS Expanded — Game Design Document

**Edizione consolidata:** 10 novembre 2026  
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

**[APPROVATO IN DISCUSSIONE 09/11/2026]** Ogni club ha un **chairman/presidente con nome ed età**. L'età cambia con le stagioni e consente il pensionamento/ritiro. Quando un chairman si ritira, il motore genera o assegna **automaticamente un sostituto nello stesso club**. Età limite e distribuzione delle età restano da scegliere; non introdurre altri campi anagrafici non richiesti come prerequisito.

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
