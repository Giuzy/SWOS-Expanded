# SWOS Expanded — Piano integrale di sviluppo verso il rilascio

**Bozza di pianificazione da concordare — 23 novembre 2026**  
**Base verificata:** repository `Giuzy/SWOS-Expanded`, ramo `dev/0.10`, HEAD `b9f65566b00f5fca49da32f73be005e008da3fc3`, versione `00.13.292`.  
**Natura:** guida di prodotto e piano operativo; **non** certificazione di funzioni, non commit, non modifica del repository.

## 0. Decisioni nuove e precedenze

1. L'intera roadmap e le funzionalità approvate in `GAME_DESIGN.md` e nell'audit comparativo delle altre mod fanno parte dell'obiettivo di prodotto, **salvo le esclusioni esplicite**: soltanto mondi 1996/97 e 2026/27; nessuna nuova epoca storica; niente riprogettazione arbitraria dell'originale OG.
2. **UI definitiva soltanto dopo avere definito le meccaniche** e i campi realmente necessari. Nel frattempo si possono studiare flussi e inventariare menu, senza rifarli anticipatamente. Le tre schermate TITLE / NEW CAREER / LOADING conservano le approvazioni già date. Le schermate OG originali richiedono approvazione specifica prima di qualsiasi modifica, anche minima.
3. La **forma individuale è una proposta di stato dinamico della carriera**, non un nuovo attributo nei dati statici del giocatore. Verificare prima la semantica di `PlayerFile.form`; decidere solo dopo se riutilizzarlo, proiettarlo in un overlay per-carriera o evitare il nuovo campo. **Non è ancora scelta definitiva.**
4. Le nuove informazioni permanenti riguardano principalmente i **club** (finanze, struttura, sponsor, staff, presidente, stato societario); non duplicare i dati base dei giocatori. Al giocatore aggiungere il minimo necessario per stato di carriera, ove non già rappresentato da OG: ad esempio condizione/forma, aspetto pelle-capelli, età/ritiro/contratto soltanto se non già recuperabili dai dati esistenti. Audit prima di introdurre campi.
5. Per la persistenza **non si approva una lista infinita di cronologie**. I campi proposti sotto sono un tetto di *categorie da verificare*, non autorizzazione ad aggiungerli tutti: conservare solo i valori indispensabili per riprendere esattamente la carriera e gli eventi futuri pendenti. Snapshot corrente + archivio strettamente necessario; niente storico settimanale per ogni giocatore o club.
6. La progettazione dei numeri inizia **prima** del codice che li usa; il bilanciamento finale si fa con simulazioni realistiche. Claude è riservato principalmente a blocchi di architettura e integrazione; ChatGPT + utente preparano decisioni, schede dati, regole, audit, prototipi e verifiche semplici. Non promettere che il consumo Claude finisca esattamente con la settimana.
7. Prima di cambiare UI o comportamento OG, presentare differenze e conseguenze e ottenere approvazione esplicita. Conservare fisica e controlli della partita a 50 Hz, replay, CLASSIC/DIY/SEASON, editor e giochi originali. QUICK RESULT OG originale per CPU/VIEW RESULT; 50 Hz per partita manuale, mai un simulatore concorrente.
8. Le guide precedenti contengono note storiche contrastanti: prevalgono decisioni recenti e approvate; etichettare *approvato / in progettazione / implementato / testato Windows* separatamente. Non promuovere un test isolato a prova gameplay.

## 1. Obiettivo finale e criteri di prodotto

Un gioco Windows x64 utilizzabile con tastiera e Nintendo Switch Pro Controller, con due mondi di carriera persistente, ampiezza mondiale da definire in base ai dati verificabili, 70+ anni di evoluzione, tutte le competizioni per cui esistono definizioni supportate effettivamente giocabili da CPU e manager, contratti/mercato/allenamento/economia coerenti, menu definitivi e partita originale migliorata senza perdita di feeling. Il mondo continua durante la disoccupazione, l'esonero e il cambio club. L'intera carriera è ripristinabile con `AUTO9697.CAR`/`AUTO2627.CAR`, senza ripetizioni.

**Definizione di “finito”:** tutti i percorsi di gioco previsti sono funzionali, le due epoche rispettano i dati approvati, il risultato di ogni fixture ha unica autorità, i salvataggi resistono a EXIT/CONTINUE, il menu consente di usarli, 70 stagioni e grande scala hanno test appropriati, le differenze dalla fisica OG sono soltanto quelle approvate e collaudate.

## 2. Ripartizione del lavoro

| Attore | Compito primario | Cosa NON delegare inutilmente |
|---|---|---|
| **Claude Code** | Motore C generale, dispatcher e lifecycle edizioni, adattatori OG, sistemi economici/giocatori, serializzazione/compatibilità, test automatizzati complessi | Discussioni ripetute di design, ricerca manuale di dati e disegno preventivo dei menu |
| **Utente + ChatGPT** | Decisioni di gameplay, regolamenti, fonti dati, cataloghi e importazione guidata, modelli economici, mockup quando maturi, piccole correzioni e analisi log | Modificare senza autorizzazione le funzioni OG, accettare dati inventati o dichiarare build testate senza Windows |
| **Utente su Windows** | GitKraken, build e diagnostic BAT, test partite e controller, valutazione UI, approvazione del risultato | Test automatici isolati spacciati per gameplay completo |

**Economia di token Claude:** una richiesta per blocco generale e interfacce ben definite, non dieci micro-task; offrire un fascicolo di decisioni e dati già completato; test isolati automatizzati; inviare log solo sui fallimenti; checkpoint Issue #1 dopo ogni preparato; niente commit/push senza ordine esplicito. Questa settimana preferire CE, edizioni e magari classifiche; non aprire ora economie, sprite o redesign.

## 3. Sequenza di lavoro: fasi e porte d'uscita

### FASE 0 — Congelare i contratti architetturali (breve; prima del codice nuovo)

**Responsabili:** ChatGPT + utente, Claude solo per dubbi di codice. **Input:** `AGENTS.md`, guide obbligatorie, storico RG, `SAVE.md`, codice HEAD.

1. Identificare per ciascun sistema **autorità/writer**, reader, identità, eventi, possesso dello stato, scadenze e obbligo CAR.
2. Confermare la separazione: CE crea/possiede fixture, date, classifiche, edizioni e qualificazioni; OG produce il risultato QUICK o gioco 50 Hz; LWE non dispone di secondo orologio o punteggio.
3. Confermare le nozioni di competizione `type0/type1/type2`, paese/slot/edizione/fase/fixture, obbligo niente due gare dello stesso club nello stesso giorno e rinvii tracciati.
4. Confermare la regola delle edizioni: non si chiudono al 30 giugno per convenzione; scadenza paese/edizione configurabile e chiusura solo dopo gare e recuperi conclusi.
5. Proteggere in continuità CAR legacy, due world namespace, CLASSIC/DIY/SEASON, OG 50Hz, replay e UI originale.

**Gate:** tabella degli invarianti e test RG per ogni workstream; nessun conflitto fra fonti approvate lasciato implicito.

### FASE 1 — Chiudere il nucleo CE: singola gara manager/CPU

**Responsabile codice:** Claude. **Priorità massima.**

1. Tracciare `SCHEDULED → CLAIMED → OG QUICK/50Hz → COMPLETED`, con un solo ID fixture e transazione CAR obbligatoria quando il manager entra/conclude/annulla.
2. PLAY MATCH mostra il tabellone originale appropriato e una sola gara, COACH/MATCH entra nel 50 Hz; VIEW RESULT usa QUICK originale e aggiorna ledger e classifica; l'annullamento ripristina lo stato corretto senza risultato inventato.
3. FORWARD da disoccupato esegue CPU senza aprire una gara interattiva; se manager assunto non rigioca il suo fixture come CPU.
4. Club switch, firma, esonero e offerte cambiano solo manager/binding, senza reset degli eventi già eseguiti.
5. VIEW COMPETITIONS/WORLD/CALENDAR sono reader della medesima classifica e non writer; evitare fallback grafici OG non approvati.
6. Coprire match pari, rigori, witness home/away, ritorno da menu, clock, replay e sincronizzazione writer CAR.

**Test minimi:** RG-T03/04/05/14/15/16, prova Windows con una QUICK manager, una 50Hz terminata, una partita annullata, uscita, CONTINUE e assunzione a stagione in corso. **Gate:** nessuna gara duplicata/perduta, classifica identica da tutte le viste, CAR conserva score/seed/fixture.

### FASE 2 — Regole e lifecycle universale delle competizioni

**Responsabile codice:** Claude. **Responsabili dati:** ChatGPT + utente.

1. Per **campionati type0**, completare punti per vittoria, criteri di classifica, parità, gironi, calendario regolare, retrocessioni/promozioni, quote e finale di lega dalle regole della singola competizione.
2. Per **coppe type1**, realizzare sorteggio/seed verificati, byes, andata/ritorno, supplementari/rigori dove previsti, finale, vincitore e credenziali della qualificazione.
3. Per **type2**, gironi, gruppi, split, playoff/out, classifica fase, KO successivi, finale, titolo e qualificati. Riutilizzare le strutture OG di presentazione senza alterarne UI/ABI.
4. Rendere automatiche apertura nuova edizione, derivazione partecipanti dalle standings e vincitori **verificati**, custodia delle edizioni a cavallo d'anno e aggiornamento ricorrente della scadenza.
5. Usare il modulo archivio già presente in CAR soltanto al vero `CloseEdition`, con stato completo e regole anti-replay: non generare champion da stringhe viewer o semplici `16/16`.
6. Date per-gara, collisioni internazionali e rinvii registrati (data originale, motivo, nuova data); quando dati reali non verificati, etichette corrette senza inventare ufficialità.
7. Identità club persistenti in caso di promozione/retrocessione/nuova edizione; stesso sistema per tutto il mondo, Bolivia solo test di regressione.

**Test:** RG-T06/07/08/09/10/12/13/16/24; completare lega, coppa nazionale type1 e continentale type2 multifase per almeno due edizioni e un torneo a cavallo anno; manager/disoccupato; CAR prima/dopo rollover. **Gate:** per ogni tipo esiste un ciclo end-to-end Windows e una matrice formati globale, non patch paese-specifiche.

### FASE 3 — Copertura delle competizioni OG e nazionali esistenti

**Responsabile architettura:** Claude. **Dati:** noi.

1. Inventariare tutte le competizioni reali presenti in `9697` e `2627`, incluse divisioni minori, coppe nazionali, UEFA, CONMEBOL, CONCACAF, AFC, CAF, OFC, Coppe del mondo e tornei nazionali per cui esistono dati, Supercoppe/Recopa/Intercontinentali pertinenti.
2. Per ogni definizione compilare una *scheda dati* con ente/paese, world, tipo, partecipanti, fonti, criteri accesso, date storiche, fasi, spareggi, quote, premi e classifica finale.
3. Distinguere prima edizione con partecipanti storici verificati e successive edizioni determinate dal mondo **attuale**. Il template 2026 per 2027+ è di GAME, non ufficialità storica futura. In 9697 non inventare il calendario dei cambi storici di formato.
4. Allargare gli adapter dati senza aggiungere un secondo motore o un massimo globale arbitrario di residenti.
5. Aggiungere convocazioni e gestione nazionali/CT **al ciclo competizioni** come obiettivo confermato; l'esatta UI del ruolo CT e i roster specifici si integreranno quando il modello giocatori è pronto.
6. Certificare inizi e pause da disoccupato, offerte e subentro a stagione avanzata, comprese fixture precedenti START importate come stato storico reale o `UNAVAILABLE` se non verificato.

**Gate:** matrice paese × competizione × world × completezza dati × CPU/manager/viewer/qualificazioni/CAR; nessuna definizione dichiarata “funzionante” perché compare solo in un JSON o viewer.

### FASE 4 — Stress test di scala realistico, prima del modello manageriale

**Responsabili:** Claude prepara harness; utente testa Windows; ChatGPT interpreta dati.

1. Test 70+ anni con club e fixture realistici, assunzione/disoccupazione/esoneri, chiusure molteplici, coppe e tornei sovrapposti.
2. Scale progressive: dati correnti → stima media → scenario limite **fino a ~1,2 milioni fixture annue** se il catalogo finale lo conferma. Evitare deduzioni da sezione CE sintetica come prova CAR intero.
3. Misurare picco RAM, dimensione CAR, tempo START, FORWARD, QUICK OG, worker async, CONTINUE, snapshot e archiviazione.
4. Verificare nessun doppio risultato, fixture collision, retrocessione incoerente, titolo duplicato, ranking falso, schema CAR che cresce indefinitamente.
5. Carico con 20k/50k/70k club come **scenari di test**, non quantità finali approvate. Rilevare se cache 64 MiB limita il guadagno e se il formato necessita evolvere.

**Gate:** envelope di prestazioni documentato e percorsi di miglioramento localizzati; non ottimizzare sulla base di numeri immaginari.

### FASE 5 — Definire schema minimale delle simulazioni società/manager

**Responsabili principali:** ChatGPT + utente. **Claude:** valida ABI e interfacce, poi implementa.

1. Definire quali dati statici appartengono al club e quali **stati dinamici per-carriera** sono indispensabili (saldo, sponsor attivo, facilities, presidente, staff). Evitare scritture duplicate in sistemi OG/LWE/CE.
2. Definire eventi economici una tantum/settimanali/stagionali con data e ID: salario, trasferta, gate receipts, diritti TV, sponsor, premi posizione/fase, facilities, manutenzione, amministrazione. Ogni transazione contabilizzata una sola volta.
3. Presidente/chairman: nome, età, ritiro e successore; obblighi di contratto, trattativa, Board/Fans e carriera manager (18–70 a START, fino a 85 anni, HOF).
4. Prezzo biglietti e curva dell'affluenza, stadio e capienza, impatto reputazione e tifosi; costi staff FITNESS/MEDICAL/TRAINING/SCOUTS/YOUTH/OFFICE, maintenance e travel.
5. Fair Play Finanziario approvato come **obiettivo**, ma periodi, soglie, premi e penalità da approvare prima del codice; nessun sistema di bancarotta giocabile, esonero in caso di crisi insostenibile secondo criteri concordati.
6. Sponsor preesistenti, scadenze e rinnovi indipendenti dall'allenatore; nessun contratto magicamente creato per la firma.
7. Documentare contratti di un nuovo componente: writer singolo, lettori, giornalizzazione minima, ID transazione, invarianti e migrazione CAR.

**Gate:** tabelle piccola/media/grande società, unità monetarie, timing dei pagamenti e casi limite approvati; schema CAR minimale e versionato.

### FASE 6 — Implementare economia, presidente, offerte e IA societaria

**Claude:** implementazione generica event-driven; **noi:** tarature iniziali e test. Implementazione simultanea club umani e IA in modo che società non gestite evolvano senza ridurre il mondo a una sola squadra. Niente AI economica completa a ogni frame: eventi discreti e transazioni verificabili. Collegare esoneri, rinnovi, sponsor, staff e prezzi a evento unico; offerte visibili senza creare copie della carriera. Solo dopo stabilizzazione delle meccaniche fissare i campi dei menu.

**Gate:** conti riconciliati per 3 stagioni; club conserva presidente/contratti/denaro dopo assunzione ed esonero; nessun premio/ricavo doppio; SAVE/CONTINUE identico; IA in limiti realistici.

### FASE 7 — Calciatori: audit prima di estendere il formato

**Claude:** audit OG e codice; **noi:** scelte gameplay, rose e parametri.

1. Mappare senza modifiche tutti i campi OG già presenti: identità, età, abilità, condizione e `PlayerFile.form`, contratti, valori, look e stato match.
2. **Forma per-carriera (proposta, non definitiva):** distinguere attributo base e condizione temporanea; provare prima riutilizzo del campo OG esistente, poi valutare overlay dinamico salvato nel CAR, **non duplicazione nel file sorgente TEAM**.
3. Aspetto pelle/capelli: usare tabelle e LUT OG disponibili se bastano; introdurre pochi dati mancanti solo dove serve e con identità stabile, senza alterare inutilmente ogni giocatore del database.
4. Età/progressione, picco/declino, ritiri, contratti, necessità rosa; preferire i dati già presenti e un minimo di stato corrente persistente.
5. YOUTH/RES/TITOLARE come **categorie di assegnazione**, non valutazione tecnica; vera squadra B/U23 separata dal TRAINING A/B; richiami e numeri di maglia, nessun prestito TRIAL automatico.
6. Potenziale noto per i giovani propri, esterno mascherato e stimato da scouting; cinque livelli scout; generazione ragazzi per turnover demografico; nessun bisogno di salvare centinaia di osservazioni di routine se ricostruibili.
7. Forma e morale collettivi già discussi (neutro 65, range 0–100 e impatto massimo ±20% **da validare**, non fissare universalmente). Allenamenti collettivi, eventuali eccezioni individuali solo dopo design.

**Gate:** schema personale minimo approvato e benchmark incremento CAR su 50k e 200k giocatori; nessun potenziale nascosto rivelato e nessuna corruzione dei record OG.

### FASE 8 — Mercato, IA giocatori, allenamento, amichevoli

1. Conservare mercato originale utile e rendere i trasferimenti reali in entrambe le rose; club IA acquistano, vendono e sostituiscono ritirati.
2. Contratti firmabili fuori finestra, **trasferimento registrazione soltanto alla finestra applicabile**; fino allora calciatore nell'origine. Svincolati ingaggiabili tutto l'anno. Finestre, bonus, durata contratti e prestiti **da concordare**, non inventare.
3. Scouting geografico 1–5, stime confidenziali, rapporto fra scout e negoziazione; player potential visibile correttamente solo dove noto.
4. Training, sviluppo, recupero, forma, morale e fatica; eventi giornalieri/settimanali ma niente tick separato; amichevoli e preparazione per tutti i club, con rischi e benefici coerenti.
5. Modellare infortuni/squalifiche e portieri senza doppio update da OG QUICK/50Hz; dati propri e condizioni di roster dopo una partita.
6. Aggiunte extra pratiche approvate come obiettivo da dettagliare: training/practice, demo, rigori standalone e challenge/speedrun; conservarle fuori dal percorso critico del CE.

**Gate:** 70 anni simulabili senza squadre senza giocatori; mercato IA non produce duplicati; operazioni pendenti persistite; test fine finestra e CONTINUE.

### FASE 9 — Bilanciamento con numeri misurati

**Responsabili decisioni:** noi. **Claude:** implementazione delle formule dopo approvazione.

1. Definire distribuzioni iniziali: capacità club, tasso giovani/ritiri, salari, ricavi, crescita facilities, mercato, turnover manager, infortuni e scelte IA.
2. Separare effetti sul **QUICK OG** e sulla partita **50Hz**: prima audit dove si iniettano statistiche, evitare scorer parallelo; preservare imparzialità CPU/manager.
3. Definire forma/morale/stanchezza, casa/trasferta, forma individuale (se approvata), allenamento, staff e tattiche; distinguere influenza realistica da modifica illegittima della fisica.
4. Esperimenti A/B deterministici su migliaia di partite e campionati, con seeds controllati e fonte OG; controllare distribuzione gol, punti, promozioni, budget, età e infortuni.
5. Tarare in più passaggi fino a risultati credibili per club piccoli, medi e grandi; non fissare numeri definitivi prima di queste misurazioni.

**Gate:** schede formule con valori/limiti e test, regressione 50Hz, risultati CPU/manager coerenti, nessuna iniezione statistica fuori contesto.

### FASE 10 — Miglioramenti partita OG (dopo definizione meccaniche)

**Claude:** integrazione OG circoscritta e test. **Noi:** design dettagli e grafica.

1. Recupero realistico al termine dei tempi: prima audit timer originale, poi algoritmi e indicatori approvati.
2. Parità CPU/manager per rossi, infortuni normali e portieri; salvare e ripristinare gli stati realmente necessari.
3. Panchina IN-MATCH a due file, info live, consultazione rosa avversaria readonly con un tasto da definire; regole sostituzioni per competizione.
4. Nomi completi stadi e dati reali, Ravanelli capelli bianchi, varianti capelli/pelle e divise con contrasto; verificare LUT e sprite OG, non introdurre palette fittizie.
5. Replay avanzati (speed, rewind, highlights ecc.) **solo valutazione opzionale se costo e rischio bassi**, originali sempre intatti; non prerequisito.
6. Test CLASSIC/DIY/SEASON e due mondi, 50Hz, infortuni, risultati, controller, CAR, replay, shader/palette e audio.

**Gate:** gameplay originale preservato, ogni novità indipendentemente disattivabile o isolabile dove opportuno, input e rendering approvati.

### FASE 11 — Design definitivo UI, *solo dopo* meccaniche complete

**Responsabili:** utente + ChatGPT progettano, Claude integra quando approvato.

1. Inventario schermi, dati realmente disponibili, callback, duplicazioni e azioni usate/obsolete; raggruppare funzioni senza distruggere le vie CLASSIC/DIY/SEASON.
2. Definire nuova navigazione: titoli e sotto-menu CAREER, CALENDAR, NEXT EVENT, VIEW WORLD/COMPETITIONS, CLUB/FINANCE, STAFF/TRAINING, SQUAD/B-U23, SCOUTING/TRANSFERS, OFFER/WELCOME/SACKED, RECORD/HOF, OPTIONS e MATCH STATS.
3. Documentare per schermo *fonti dati*, campi visibili/occulti (potenziale), aggiornamenti, azioni, focus, gamepad/tastiera, BACK e scenari senza informazioni; nessun `PAGE` OG non approvato.
4. TITLE/NEW CAREER/LOADING conservano design approvato; altri menu riprogettati solo dopo meccaniche. Per OG tabelloni/menu mantenere UI intatta salvo consenso specifico.
5. Per ogni nuova schermata creare prima PNG-demo con font, dati reali e misure secondo `sorgenti/docs/FONTS.md`, poi PNG pulito separato dai testi dinamici.
6. Standard: sfondi RGB8, overlay RGBA8, pixel art nitida, canvas/crop approvati, pulsanti blu 3D e focus giallo, BACK/B, A, X coerenti; mouse solo nelle nuove UI autorizzate, **non nella partita originale**. Confermare conflitti fra vecchie decisioni input prima di cambiare.
7. Testare 3840×2160 e modalità desktop Windows, Switch Pro, tastiera, scala, focus, scroll e dati estremi (nomi lunghi, molto denaro, numerose fasi).

**Gate:** tutte le funzioni accessibili, niente callback orfani, nessun layout approvato prima delle meccaniche da mostrare.

### FASE 12 — Ampliare definitivamente i dati mondiali

**Responsabili:** ChatGPT + utente con fonti e licenze; Claude solo import/validator/bug generali.

1. Determinare cardinalità realistiche per ciascun mondo: paesi, divisioni, club, stadi, nazionali, rose/giocatori, competizioni e formati; **fino a otto livelli solo dove esistono**, non obbligo uniforme.
2. Convalidare identità club e giocatori, cartelle TEAM, mapping mondiale, collisioni, duplicati, asset, permessi di redistribuzione.
3. Catalogare regolamenti reali specifici per campionato e epoca, incluse quote continentali, promozioni/split/playoff e cambi storici verificati.
4. Per 2627, distinguere esiti veri prima di START (quando disponibili) da simulazioni future e da GAME_TEMPLATE_2026; per 9697 definire cambi formato con fonti e approvazione.
5. Conservare dati e versioni immutabili necessari alla ripresa delle carriere; nessun re-import distruttivo di roster durante CONTINUE.
6. Testare la completezza usando la matrice d'inventario, non una cifra totale che nasconde coppe assenti.

**Gate:** import completo e validazione dati di entrambi i mondi, prestazioni su cardinalità finali, copertura dichiarata onestamente.

### FASE 13 — Extra e multiplayer, senza bloccare il single-player

**Obiettivo futuro approvato come intenzione:** più manager su club differenti nello stesso mondo, **locale e online**. È un progetto architetturale significativo: autorità host, input multiutente, clock condiviso, gare fra due manager, scontri in contemporanea, salvataggio e recovery, desync, sessioni, esoneri/offerte. **Non iniziare codice né scegliere protocolli finché non si apre una progettazione specifica.** Il netplay non è prerequisito della prima release completa single-player salvo nuova decisione di ambito. Escludere extra mondi storici. Alcuni extra arcade, opzioni e replay restano candidati a costo contenuto.

### FASE 14 — QA finale e release

1. Windows x64 MSYS2/SDL2 con BAT esistenti; niente nuova toolchain.
2. Matrice: 9697/2627, assunto/disoccupato/esonerato, club/CT dove disponibile, campionato/cup/type2, annullamento/PLAY/COACH/QUICK, 70 anni, CAR legacy e nuovo, SAVE/EXIT/CONTINUE e world switching consentito.
3. Negativi: file troncato/hash sbagliato, disco pieno/rename fallito, cancel, corruzione registry, fixture collision, due club nello stesso giorno, repeat event, durata edizione inusuale, club con rosa incompleta.
4. Performance: tempi FORWARD, START, CONTINUE, memoria e dimensione CAR su database finale; nuovo run prima di modificare cache/roster-delta.
5. UI, input, scena partita, controller, audio, fullscreen/windowed, localizzazione/crediti e diritti dati/asset.
6. Release candidate e report di test **SI/NO/NON PROVATO**; mai chiamare “gioco finito” una build che compila soltanto.

## 4. Politica SAVE/CAR per evitare crescita esagerata

**Principio:** stato corrente indispensabile + eventi futuri pendenti + sintesi storica non ricostruibile. **Non salvare ogni movimento nel tempo.** Non imporre oggi un campo per ogni categoria: decidere per sistema solo dopo audit OG e benchmark.

| Dominio | Da conservare se indispensabile | Da evitare |
|---|---|---|
| CE | Fixture attive, stato, ledger e RNG; archivio sintetico dell'edizione chiusa | Dettagli di tutte le gare per 70 anni |
| Club | Identità stabile, saldo e condizioni correnti, strutture, eventuali accordi attivi | Duplicazione della base del catalogo; migliaia di rendiconti storici |
| Sponsor | Contratti ancora in vigore, importo/scadenza | Cronologia di ogni pagamento già consolidato |
| Transazioni | Eventi futuri pendenti e anti-doppio pagamento dove necessario | Ledger per ogni settimana di tutte le stagioni |
| Giocatori | **Preferibilmente OG base**; soltanto delta effettivi di carriera ove indispensabili | Duplicazione continua delle rose statiche e record per ogni allenamento |
| Forma | **Proposta:** stato volatile *per-carriera* e solo se OG non rappresenta già il dato necessario | Nuovo attributo base statico TEAM senza giustificazione |
| Look | Dati esistenti OG, piccoli override colore pelle/capelli se realmente necessari | Riprogettare ogni record giocatore per il solo aspetto |
| Mercato | Accordi non ancora eseguiti, effetti correnti su roster/contratto | Copie multiple dello stesso trasferimento in tre sottosistemi |
| Manager | Stato carriera + storico sintetico incarichi/titoli/esoneri | Log di ogni decisione settimanale |

**Regole di approvazione prima di ogni nuova variabile persistente:** (a) perché non è derivabile; (b) quanti record al massimo; (c) byte grezzi e compressi attesi; (d) chi scrive e chi legge; (e) quando si aggiorna e si elimina; (f) migrazione CAR/legacy; (g) RG-T. Finché questi sette punti non sono chiari, campo *proposto*, non implementato.

**Stato v292:** CAR `SWOSZC4A` con blocchi compressi, cache e worker async verificati nel percorso CPU dell'ultima diagnostica analizzata; lavoro futuro su scala e crash/manager. Roster a differenze differiti: nessun nuovo loader OG senza una decisione esplicita. Il formato potrà evolvere mantenendo lettori/versioni e test.

## 5. Inventario delle funzionalità da altre mod: cosa entra e cosa no

| Famiglia | Decisione del progetto | Collocazione |
|---|---|---|
| SWOS Mod: competizioni e formati | **Approvato**, con dati verificati | Fasi 2–3, 12 |
| UEFA, Libertadores, confederazioni, Supercoppe/Recopa/Intercontinentali | **Approvato** ove pertinente ai due mondi | Fasi 2–3, 12 |
| Nazionali, qualificazioni, CT e convocazioni | **Approvato come obiettivo**; dettagli da stabilire | Fasi 3, 7–8 |
| Fisica OG 50Hz | **Da preservare** | Tutte |
| Recupero partita, CPU disciplina e infortuni GK | **Approvato come obiettivo**; algoritmi da approvare | Fase 10 |
| Panchina 2 righe live, strip e rosa avversaria | **Approvato**; tasti da decidere | Fase 10 |
| Squadra B/U23, richiamo, maglie 1–99 | **Approvato come obiettivo**; record OG da analizzare | Fasi 7–8 |
| Fair Play Finanziario e premi per piazzamento/fase | **Approvato come obiettivo**; numeri da concordare | Fasi 5–6 |
| Esercizi/practice, demo, rigori, challenge/speedrun | **Approvato come direzione**, specifica separata | Fasi 8, 13 |
| Nome stadio, look pelle/capelli, Ravanelli, contrasto divise, opzioni A/V | **Approvato come obiettivo**; prima verifica risorse esistenti | Fasi 7, 10–11 |
| Replay avanzati extra | **Facoltativi se semplici**, replay OG intatto | Fase 10/13 |
| Altri mondi storici 1984/85 ecc. | **Esclusi** | Fuori scope |
| Nuova suite di piccoli toggle/calibratori/modi tastiera proposta da altre mod | **Non approvata come pacchetto** | Fuori scope salvo scelta specifica |
| Multi-manager locale e online su club diversi nello stesso mondo | **Obiettivo futuro di design**, non autorizzazione al codice | Fase 13 |

Non importare codice, musica, sprite o database di altre mod senza verificare licenze e attribuzioni. Una funzione presente in altra mod non prova che la stessa manchi nell'OG/Expanded: prima controllare sorgente e comportamento.

## 6. Pianificazione pratica della settimana in corso

**Claude — concentrazione su un solo workstream:**

- **Pacchetto C1:** audit mirato RG/HEAD del percorso manager, mappa writer-reader, implementazione unificata QUICK/50Hz/annullamento/CONTINUE per fixture CE, più test C e un percorso Windows misurabile.
- **Pacchetto C2:** con C1 sufficientemente stabile, ciclo completo type0/type1/type2 e chiusura edizioni generica → archivio CAR. Prima definire gate e dati necessari, poi implementare per interfacce; se quota insufficiente, fermarsi con stato recuperabile invece di metà refactor.
- **Riserva:** bug bloccanti reali dalla diagnostica di C1/C2. Non aprire ora UI, economia, roster-delta, API nuovi o ottimizzazioni speculative.

**Noi in parallelo:**

- Inventario competizioni esistenti nei due mondi, classificato per tipo e fonte. Tabelle di quote, date, regolamenti e conflitti.
- Scheda di stato minimale per un club, senza scegliere ancora 40 variabili CAR. Ricognizione delle informazioni OG esistenti per giocatore e dei modelli economici da discutere.
- Preparare matrice di test Windows per le gare manager e le edizioni, con script BAT già esistenti.
- Raccogliere i requisiti dei menu senza produrre ancora il layout finale.

**Criterio di stop settimanale:** tutto ciò che Claude modifica deve avere un punto di recupero, report e test; il budget di tempo risparmiato non vale un cambiamento non certificabile.

## 7. Schema dei report per ogni fase

`Ramo | HEAD | versione | ambito | file interessati | writer/reader | tipo0/1/2 | cause V/I/? | precheck RG | test ISO | build Windows | gameplay Windows | CAR legacy/nuovo | performance | limiti | prossimo passo | Issue #1 checkpoint`.

Le scoperte si documentano appena emergono; prima di ogni modifica rileggere `docs/REGRESSION_HISTORY_AND_DEPENDENCIES.md`, casi pertinenti e mappa dipendenze. Dopo ogni preparato aggiornare Issue #1 con percorso e SHA, se l'accesso è disponibile; se non lo è, dichiararlo esplicitamente e non fingere il deposito. Nessun commit o push senza richiesta.

## 8. Fonti di progetto da mantenere come riferimento

- `AGENTS.md`, `docs/AGENTS.md`
- `docs/PROJECT_STATE.md`
- `docs/GAME_DESIGN.md`
- `docs/IMPROVEMENTS.md`
- `docs/ENGINE_CODE_DISCOVERIES.md`
- `docs/DEVELOPMENT_WORKFLOW.md`
- `docs/COMPETITION_ENGINE_IMPLEMENTATION_GUIDE.md`
- `docs/REGRESSION_HISTORY_AND_DEPENDENCIES.md`
- `sorgenti/docs/SAVE.md`, `sorgenti/docs/FONTS.md`
- Issue #1: https://github.com/Giuzy/SWOS-Expanded/issues/1

**Questa guida è una proposta consolidata; non sostituisce le guide autoritative finché l'utente non approva la sua integrazione nel repository.**
