# SWOS EXPANDED — PROJECT_STATE.md

**Aggiornato:** 6 novembre 2026  
**Repository:** `Giuzy/SWOS-Expanded`  
**Ramo di riferimento al momento della stesura:** `dev/0.10` (il vecchio `release/0.10.40` resta temporaneamente come backup)  
**Base:** Sensible World of Soccer / OpenSWOS · C / SDL2 · Windows x64  
**Stato:** sviluppo attivo; la presenza nel repository non equivale a collaudo Windows.

## 0. Scopo, fonti e legenda

Questo file conserva le decisioni di gameplay e gestione, lo stato delle funzionalità, i layout approvati, le idee future e i problemi aperti. Le regole di lavoro dell'assistente sono in `AGENTS.md`; il registro delle modifiche effettive rispetto a SWOS originale è destinato a `IMPROVEMENTS.md`.

**Fonti ricostruite:** conversazioni del progetto recuperabili, precedente `SWOS_EXPANDED_PROJECT_STATE.md` (4 novembre 2026), documentazione storica `PROJECT_HISTORY.md` presente su `main` (README delle versioni 0.10.3–0.10.38, con lacune), decisioni più recenti e note sui rami GitHub. Non è una trascrizione completa di ogni chat né una verifica funzionale dell'ultima build.

**Stati:** [APPROVATO] decisione confermata; [DOCUMENTATO] descritto in README/build; [TESTATO] provato dall'utente; [DA VERIFICARE] da controllare nel codice/Windows; [PIANIFICATO] richiesto ma incompleto; [IDEA] non confermata nei dettagli; [SUPERATO] sostituito da decisioni successive. Le etichette non sono intercambiabili.

## 1. Visione

[APPROVATO] Espandere la carriera manageriale, il mondo calcistico, l'economia e le competizioni preservando il motore e il feeling delle partite SWOS. Obiettivo di lungo periodo: paesi e divisioni di tutto il mondo, comprese le categorie minori, con campionati, coppe, promozioni, retrocessioni, tornei continentali e internazionali.

La base è OpenSWOS, derivata dal reverse engineering del gioco originale. Il progetto è offline, senza netplay. Non sostituire il motore partita con una simulazione nuova.

## 2. Tecnica, distribuzione e verifica

- Windows 10/11 x64, C, SDL2, MSYS2/MinGW; monitor di riferimento 3840×2160 a 144 Hz, gioco a 50 Hz.
- [APPROVATO] I menu Expanded in fullscreen usano il contenuto 320×256 anziché esporre i margini del framebuffer originale 336×272; stretch 16:9, pixel art nitida, rendering della partita separato.
- [TEST UTENTE, 2026-11-07] La Business Screen integrata nella serie 0.10.51 funziona, da rifinire graficamente in una sessione successiva; conservare il layout approvato e gli asset BMP.
- [SEGNALATO DALL'UTENTE] Entrando o uscendo dalla partita, la finestra development massimizzata si rimpiccioliva alla scala nativa. [CORREZIONE CODICE 0.10.51_07 — DA TESTARE WINDOWS] In `Platform_ViewChanged` si salta il ridimensionamento della finestra quando SDL segnala `SDL_WINDOW_MAXIMIZED`; cambiano soltanto dimensione logica/scala del renderer. Finestra ordinaria, fullscreen release, gioco e salvataggi invariati. Necessario test reale entrando e uscendo dalla partita.
- [APPROVATO] Joypad, mouse e tastiera; tutti i pulsanti devono essere raggiungibili. Le regole operative correnti del joypad e degli hover sono in `AGENTS.md`.
- [APPROVATO] GitHub come archivio centrale, GitKraken per aggiornare la copia locale, compilazione/test Windows sul PC dell'utente. Conservare BAT e asset necessari; evitare di scambiare controlli GCC/ASan per un test reale con joypad.
- [DA VERIFICARE] Contenuti e funzionamento complessivi del ramo `release/0.10.40`, differenze da `main`, compatibilità salvataggi, stato dei launcher e degli script.

## 3. Mondo calcistico e competizioni

[APPROVATO] Campionati, più divisioni, coppe nazionali, tornei continentali, promozioni/retrocessioni e in prospettiva nazionali/Mondiale. Ogni paese avrà la propria struttura calcistica appropriata.

[DA DEFINIRE] Copertura iniziale, stagioni sfalsate, regole concrete di playoff e qualificazioni, numero di divisioni e database. Le idee di struttura dati per competizioni e federazioni restano proposte tecniche, non un sistema già implementato.

### Fonte esterna per database e competizioni — SWOS 96/97 Mod, World 2026-27 (7 novembre 2026)

[DOCUMENTATO — NON INTEGRATO AUTOMATICAMENTE] `World of SWOS / SWOS 96/97 Mod 4.0.5` di Davide Lorigliola (`https://worldofswos.com/en/`, `https://github.com/aminta/swos-9697-mod`) descrive 91 nazioni, 2.385 club e 260 competizioni nella versione aggiornata; i comunicati 4.0 originali riportavano 264 competizioni, quindi **non mescolare i conteggi tra versioni**. Include rose stagione 2026-27, gruppi paralleli, play-off/play-out, Apertura/Clausura, coppe continentali, stadi dei tornei, e rose allargate (fino a 30 gestiti in alcuni rilasci); distinguere il limite del club allenato da quello dei club AI e controllare la versione del dato.

[DOCUMENTATO NEL CODICE — TEST DA FARE NEL NOSTRO GIOCO] OpenSWOS upstream ha già adottato World 2026-27 Mod 4.0.5 e le sue regole, via OpenSWOS Setup (comunicato `https://baker76.itch.io/openswos/devlog/1748322/openswos-on-android-and-iphone-and-world-2026-27-in-the-options`). Nel nostro ramo sono già presenti `sorgenti/src/world.c`, `world_data.c`, moduli di competizione e `sorgenti/docs/WORLD_2026_27.md`, che descrive il porting. **Non dedurre che il pacchetto di team e giocatori sia già installato**, né dichiarare funzionante la carriera 2026-27 senza test specifici.

[VINCOLO LICENZE] Il file `LICENSE` di `aminta/swos-9697-mod` dà licenza **MIT al codice**, ma esclude esplicitamente i **dati calcistici** (giocatori, statistiche, rose, maglie e partecipanti alle competizioni), per i quali chiede contatto col manutentore prima del riuso. Il progetto OpenSWOS è GPL-3.0. Non copiare/distribuire dati del mondo 2026-27 senza verificare autorizzazioni e diritti; preferire il percorso ufficiale di installazione dei dati per l'utente, se compatibile.

[DA VALUTARE — NESSUN CAMBIO GAMEPLAY APPROVATO] 1) usare il catalogo world e gli importer OpenSWOS esistenti, verificando limiti dei file, ID, salvataggi e squadre da 16/30 elementi; 2) valutare ampliamento delle rose dei club AI e schermate gestione come intervento separato; 3) widescreen/4K esistono già in SDL2 (`sorgenti/docs/WIDESCREEN.md`), quindi non importarli dalla mod DOS; 4) **60 FPS non equivale a simulazione 60 Hz**: il gameplay Amiga è a 50 Hz fisso (`sorgenti/docs/GAMEPLAY.md`) e va preservato. L'eventuale rendering a refresh maggiore richiede disaccoppiamento presentazione/tick e test di fluidità, input e sincronizzazione, non aumento dei tick del motore.

### Business Screen — codice 0.10.51_05; correzione compilazione 0.10.51_06 (7 novembre 2026)

[TEST UTENTE — COMPILAZIONE FALLITA 0.10.51_05] Il compilatore MinGW64 GCC 16.2.0 ha segnalato `_currentMenu` non dichiarato in `Expanded_ClubOverviewReapplyLayout` (`expanded_economy.c`, riga 683); i warning `-Waddress-of-packed-member` non sono l'errore bloccante. [CORREZIONE CODICE 0.10.51_06 — NON ANCORA RITESTATA] Spostata la dichiarazione di `_currentMenu` prima dell'uso (percorso SDL), e il fallback YEAR usa direttamente l'entry 14 senza riferimenti a `_currentMenu` su altri target. Da verificare una nuova compilazione Windows e il test reale in gioco.


[IMPLEMENTATO NEL CODICE — NON ANCORA TESTATO SU WINDOWS] L'integrazione della schermata Business carica direttamente i BMP approvati in `assets/business/business_ui_approved_2026-11-07.bmp` (320×256) e `assets/ui/controller_abc_approved_2026-11-07.bmp` (70×14, posizione 9,6), senza convertirli. Renderer SWOS su framebuffer indicizzato con composizione alpha e vecchio sfondo come fallback se manca un asset. Testo, importi, data, stadio (5 livelli), Board/Fans e faccine rimangono dinamici e vengono ridisegnati ad ogni refresh. Facilities ordinate per colonne, prima sinistra FITNESS–MEDICAL–OFFICES–SCOUTS, poi destra TRAINING–TRAVEL–YOUTH; UPGRADE apre il sottomenu esistente dello STADIUM. Navigazione joypad/mouse/tastiera riallineata e selezione/hover dinamici. WEEK e YEAR sono ora tasti indipendenti con valori finanziari correnti/stagionali; confermati popup con X per WAGES, TRAVEL, MAINTENANCE; B chiude prima il popup e poi torna indietro. Salvataggi ed economia non vengono modificati nel loro formato o nelle formule. `SWOS_EXPANDED_VERSION` incrementata a `0.10.51_05`.

[DA TESTARE] Sintassi/build Windows MinGW64 SDL2, resa grafica in 3840×2160, corrispondenza coordinate e mouse, focus di tutti i pulsanti da joypad, popup X immediati, selezione WEEK/YEAR e prezzi con valori lunghi, corretto fallback, apertura dei sottomenu di tutte le facilities e navigazione di ritorno. I due BMP approvati non sono stati alterati.

## 4. Living World Engine

[APPROVATO — OBIETTIVO CENTRALE] Il mondo continua durante l'occupazione e la disoccupazione del manager. Avanzano date, partite, risultati, classifiche e stagioni. Offerte a metà stagione: il manager entra nella situazione realmente raggiunta, senza ricreare il campionato. Risultati già determinati non devono cambiare aprendo una schermata.

[DOCUMENTATO] Il calendario da disoccupato avanza di settimana e può generare offerte. [PIANIFICATO] Risultati autentici di tutte le leghe durante l'attesa e assunzione a stagione già avviata: la documentazione storica li indicava esplicitamente come non completi.

[IDEA] Simulazione dettagliata/rapida/differita, seed deterministici, eventi storici, evoluzione dei club e allenatori AI. [DA DEFINIRE] Granularità temporale, prestazioni, persistenza e stagioni in mesi diversi.

## 5. Carriera manageriale

[APPROVATO] Manager inizialmente anche UNKNOWN / UNEMPLOYED; nome, cognome, nazionalità, età iniziale 18–70, fama e valuta EUR/USD/GBP. Carriera senza limite fisso di 20 stagioni; ritiro previsto a 85 anni.

**Fama:** UNKNOWN (~20), AMATEUR (~40), RISING STAR (~60), ESTABLISHED (~80), LEGEND (~100). I valori sono riferimenti indicativi per il gradimento iniziale e l'accesso alle offerte.

**Offerte:** paese/regione, club, divisione, stipendio **annuale**, trattativa, VIEW SQUAD, VIEW COMPETITIONS, ACCEPT, DECLINE, BACK. La fama influisce sulla qualità delle offerte e sulla negoziazione. Le precedenti build documentano fino a cinque rifiuti di controfferte, da verificare nel codice corrente.

**Board e Fans:** gradimenti distinti 0–100, collegati a risultati, reputazione, finanze e negoziazione.

**Esonero:** [APPROVATO] quando dirigenza e tifosi sono molto scontenti, con schermata SACKED e ritorno alla disoccupazione senza azzerare il mondo. [DA VERIFICARE] Soglia finale; storicamente furono provati Board+Fans <40 e test di esonero forzato, da non lasciare attivi per errore.

**Management Record / Hall of Fame:** club, paesi, stagioni, contratti, guadagni, trofei, esoneri e record. [DA VERIFICARE] completezza e comportamento a 85 anni.

## 6. Club, strutture ed economia

[APPROVATO] Distinguere **blasone storico**, **prestigio attuale** e **forza della rosa**. Il club può crescere o decadere. Stadio in cinque livelli e infrastrutture FITNESS, TRAINING, SCOUTING, TRAVEL, MEDICAL, OFFICES, YOUTH.

**Entrate:** sponsor, TV, incassi stadio, premi. **Costi:** WAGES giocatori, STAFF personale/strutture, TRAVEL trasferte effettive (zero in casa; mai doppio addebito), stipendio MANAGER contrattuale incluso nella contabilità, ADMINISTRATION spese amministrative. [APPROVATO] ADMINISTRATION = 45.000 annui per un club piccolo; scala per altri club da definire. Nel popup STAFF le etichette e i totali devono essere coerenti, senza righe duplicate.

**FINANCES:** WEEK e YEAR; distinguere il consuntivo stagionale dalle proiezioni MONTH/YEAR (×4 / ×52 dei costi correnti). [DOCUMENTATO] Costi storici TRAVEL per livello: 100/180/320/560/950 a trasferta; rivedere se necessario.

**Debiti:** [APPROVATO] niente prestiti bancari, indebitamento o fallimento. Saldo negativo del club gestito comporta esonero; club AI devono evitare deficit. [DA VERIFICARE] Implementazione reale.

## 7. Giocatori, POTENTIAL, forma e morale

Conservare le abilità originali SWOS (Passing, Velocity, Heading, Tackling, Control, Speed, Finishing) e i campi necessari al motore.

[APPROVATO] **POTENTIAL** è il massimo livello di abilità raggiungibile da ciascun giocatore; non coincide con la forza attuale. [DA DEFINIRE] Formula di sviluppo, età, strutture giovanili, crescita e declino.

[APPROVATO] **TEAM FORM** e **TEAM MORALE** sono due valori di squadra da 0 a 100. La nuova condizione individuale Expanded 0–100 introdotta storicamente è [SUPERATA]; non confonderla con i campi originali SWOS.

**Faccine:** cinque espressioni distinte ogni 20 punti, con **colori per tre fasce** (regola più recente):
| Valori | Espressione | Colore |
|---|---|---|
| 0–19 | Molto negativo | Rosso |
| 20–39 | Negativo | Rosso |
| 40–59 | Neutro | Giallo |
| 60–79 | Positivo | Verde |
| 80–100 | Molto positivo | Verde |

Per evitare sovrapposizioni: 0–39 rosso, 40–59 giallo, 60–100 verde. Stessa convenzione per Board/Fans e Form/Morale. [DOCUMENTATO] Questo schema risulta già descritto nelle note 0.10.27–0.10.28.

**Modificatore squadra:** media M=(Form+Morale)/2; M=65 neutro, M≤20 malus massimo −20%, M=100 bonus massimo +20%, interpolazione nei tratti intermedi. [DOCUMENTATO] La 0.10.25 descrive l'applicazione temporanea alle abilità in partita senza modificare i valori memorizzati. [DA VERIFICARE] Implementazione e bilanciamento attuali.

## 8. Training

[PIANIFICATO] Nuova schermata e meccaniche da definire con l'utente. Temi: schemi di allenamento, forma/morale di squadra, recupero, crescita dei giocatori, effetti di TRAINING/FITNESS/MEDICAL/YOUTH. Non reintrodurre automaticamente il vecchio sistema di condizione individuale.

[DA DEFINIRE] Frequenza, efficacia, costi, crescita permanente, infortuni e interazione con le partite originali.

## 9. Trasferimenti e scouting

[APPROVATO] **TRANSFER 1** è il riferimento del menu principale: sfondo nero, barre separate titolo/stato, pulsanti BUY PLAYERS, SELL PLAYERS, OFFERS, SCOUTING, EXIT, nessuna cornice esterna.

[PIANIFICATO] Mercato mondiale, acquisti, cessioni, scouting e offerte. Prestiti, rinnovi, svincoli, clausole e contratti avanzati restano da definire.

**Scouting geografico:** modello storico discusso per i livelli: 1 paese del club, 2 paesi vicini, 3 regione/continente, 4 quasi tutto il mondo, 5 mondo intero. Parametri da confermare.

**Visibilità parziale / ??? — [DA DEFINIRE]:**
- Quali giocatori appaiono già nei risultati di ricerca e quali restano sconosciuti in base a livello di scouting, area geografica e osservazioni effettuate.
- Quali statistiche di un giocatore non osservato vengono mostrate come `???` (singole abilità, POTENTIAL, valore, stipendio o altre informazioni).
- Se le statistiche vengono rivelate progressivamente, con quale precisione e dopo quanto scouting.
- Se i giocatori fuori copertura non appaiono affatto oppure appaiono con informazioni oscurate.

**Nessuna delle alternative precedenti è già approvata come regola definitiva.** Non inventare quali statistiche siano nascoste.

## 10. Schermate e design approvati

- **TITLE:** logo originale + Expanded, menu compatto, Hall of Fame, sfondo pixel art approvato.
- **NEW CAREER:** campi nome/cognome/nazionalità/età/fama/valuta, font SWOS, focus e input.
- **CALENDAR / UNEMPLOYED:** data, righe partite/coppe e casa/trasferta, frecce interne, calendario condiviso, FORWARD TIME e WAIT FOR A CLUB OFFER.
- **BUSINESS:** layout di gioco approvato con Board/Fans, stadio, strutture, FINANCES, WEEK/YEAR, popup STAFF, joypad. **2026-11-07: approvato soltanto come asset grafico** il BMP pulito `assets/business/business_ui_approved_2026-11-07.bmp` (320×256, 32 bit); non è ancora integrato. Facilities in ordine alfabetico: sinistra FITNESS, MEDICAL, OFFICES, SCOUTS; destra TRAINING, TRAVEL, YOUTH, con UPGRADE distinto e icona stadio. Nessun testo/valore/stadio/faccina/hover/selezione statico. L'implementazione richiederà riassociazione delle hitbox e controllo del loader BMP alpha; preservare tutti gli altri menu.
- **SQUAD:** layout approvato, paese in piccolo box leggibile, TEAM FORM/MORALE, popup statistiche e controlli.
- **CLUB OFFER:** contratto sul tavolo, dati dinamici, stipendio annuo e anteprime.
- **WELCOME / SACKED:** scene pixel art dedicate.
- **MANAGEMENT RECORD:** storico carriera con schermata dedicata; verificare stato.
- **TRANSFER 1:** riferimento approvato, UI su nero senza pannello esterno.
- **MATCH STATS / EXIT MATCH:** nelle 0.10.36–0.10.38 nuovi pannelli in partita; ultima revisione documentata: MATCH STATS compatto, dati dinamici senza testi doppi; EXIT MATCH senza freccia, cornice gialla sulla scelta e BACK preselezionato. Verificare su Windows.

### Loading screen — [APPROVATO]

**Il design della schermata di caricamento è approvato**, non soltanto una direzione artistica. Scena stadio dall'alto, calciatore in rovesciata, cartelli/crediti, logo e barra come asset/overlay separati; fondale completo. La percentuale deve seguire fasi reali del bootstrap. Le note 0.10.25 e 0.10.31–0.10.32 documentano integrazioni e asset approvati, compresa la separazione dei layer.

[DA VERIFICARE] Quale asset e quale durata minima siano effettivamente attivi nell'ultima build: alcune versioni prevedevano pause temporanee di 5 o 20 secondi per screenshot; non trattarle come requisito permanente.

### Pulsanti animati — [APPROVATO — PROPOSTA DI DESIGN; NON IMPLEMENTATO]

- Obiettivo: pulsanti in stile SWOS che reagiscano visivamente alla selezione e alla pressione, senza animazioni vistose o moderne.
- Tre stati principali: **NORMAL** (blu, fermo); **SELECTED** (colore modificato e bordo giallo, selezione da joypad e mouse); **PRESSED** (abbassamento visivo di circa 2–3 pixel per circa 100 ms, poi esecuzione dell'azione).
- Soluzione tecnica proposta: sprite per l'aspetto dei pulsanti, con eventuali animazioni leggere gestite dal rendering SDL2; mantenere la logica runtime esistente per focus, navigazione, input, hitbox e azioni. Non è necessario trasformare l'intero menu in un video o in un'immagine unica.
- Conservare testi dinamici e font SWOS nel rendering runtime quando opportuno. Evitare di modificare i layout già approvati.
- **Prima sperimentazione prevista:** TRANSFER 1, mantenendo il design approvato e verificando selezione/pressione con mouse, tastiera e joypad. Estendere alle altre schermate soltanto dopo prova e approvazione.
- **Stato:** decisione grafica approvata; nessun codice, sprite animato o test Windows realizzato da questa nota.

## 11. Cronologia delle build — principali cambiamenti documentati

La tabella riporta **tutte le versioni identificabili nelle fonti disponibili**, non inventa release mancanti. La colonna indica quanto risulta da note storiche, non garantisce test Windows.

| Build | Principali modifiche documentate |
|---|---|
| 0.1–0.7 | Prime sperimentazioni su port OpenSWOS, avvio, stabilità e interfacce; dettagli per singola versione non recuperati con sufficiente certezza. |
| 0.8 | Titolo/Hall of Fame, primo Club Business e carriera da UNKNOWN/UNEMPLOYED secondo riepiloghi storici. |
| 0.9 | Correzioni nazionalità, calendario del disoccupato, offerte e anteprime club secondo riepiloghi storici. |
| 0.10 / 0.10.1 | Prime correzioni di stabilità e gestione carriera/offerte; cronologia puntuale incompleta. |
| 0.10.2 | Correzioni menu e stabilità, nazionalità e flussi precedenti (documentazione storica). |
| 0.10.3 | Calendario disoccupato riutilizzato, data dinamica, BACK, Club Business e navigazione. |
| 0.10.4 | Nuovo Club Business pixel art, cinque stadi e sette strutture selezionabili. |
| 0.10.5 | Quattro menu pixel art; dettagli aggiuntivi nel README storico. |
| 0.10.6 | Titolo, carriera, calendario e Business allineati ai menu originali; callback preservati. |
| 0.10.7 | Menu compatti, font SWOS originale, redraw e calendario rifinito. |
| 0.10.8 | Correzioni titolo/input carriera, calendario e disoccupazione. |
| 0.10.9 | Board/Fans con percentuali e faccine, numero tifosi, pulizia BUSINESS e calendario. |
| 0.10.10 | Calendario compatto senza sovrapposizioni, D. THREE, frecce interne; UPGRADE. |
| 0.10.11 | Allineamenti grafici e COSTS sopra lo stadio; niente cambio di logica. |
| 0.10.12 | Conferma NAME/SURNAME senza riapertura input; calendario con pannello adattivo. |
| 0.10.13 | Calendario fino a 14 partite visibili, frecce e clipping corretti. |
| 0.10.14 | CLUB OFFER: illustrazione, trattativa e anteprime; prima versione della retribuzione da correggere. |
| 0.10.15 | CLUB OFFER con prima illustrazione approvata, stipendio annuale e pulsanti reali. |
| 0.10.16 | WELCOME/SACKED, Fans/Board e prime regole di esonero. |
| 0.10.17 | Management Record e test del flusso di esonero forzato. |
| 0.10.18 | Rosa interattiva e correzioni del flusso di carriera. |
| 0.10.19 | SQUAD/Business, sponsor e TV, infrastrutture e loro effetti. |
| 0.10.20 | Forma individuale Expanded e rifiniture SQUAD/Business; sistema poi superato. |
| 0.10.21 | WAGES/STAFF/TRAVEL, costi per trasferta, proiezioni e popup statistiche SQUAD. |
| 0.10.22 | YEAR/WEEK, navigazione joypad e hover STAFF, FORM in SQUAD. |
| 0.10.23 | Popup STAFF compatto, TRAVEL, frecce calendario e fullscreen stretch 4K. |
| 0.10.24 | Font originale WEEK/YEAR e fullscreen 320×256 senza residui di mappa/scudo. |
| 0.10.25 | Splash stadio e caricamento a fasi reali; TEAM FORM/MORALE e modificatore ±20%. |
| 0.10.26 | Base dichiarata della 0.10.27; incorpora correzioni BUSINESS/SQUAD, dettagli autonomi non recuperati. |
| 0.10.27 | Etichette MATCH STATS, input START/SELECT, ADMINISTRATION/manager, faccine rosso-giallo-verde. |
| 0.10.28 | Popup STAFF a colonne, faccine e allineamenti SQUAD; splash con pausa temporanea. |
| 0.10.29 | Nessun README specifico recuperato; non attribuire cambiamenti senza prova. |
| 0.10.30 | Loading e cartelli, STAFF più compatto, SQUAD/BANK, bonus/malus indicativi nel tooltip. |
| 0.10.31 | Sorgenti grafici approvati SQUAD/LOADING separati dalla UI; dialogo pausa sperimentale. |
| 0.10.32 | Layer separati LOADING, ADMIN e SQUAD; nota sul problema START/ESC. |
| 0.10.32b | Ripristino START alla pausa originale, rimozione del popup sperimentale. |
| 0.10.33 | Popup START di conferma uscita, tooltip STAFF e SQUAD più compatti. |
| 0.10.34 | Correzioni input START/ESC e bonus/malus del tooltip coerenti con forma/morale. |
| 0.10.35 | Popup START correttamente posizionato, SELECT statistiche, STAFF più leggibile. |
| 0.10.36 | Nuovi pannelli MATCH STATS ed EXIT MATCH sovrapposti al campo congelato, dati dinamici. |
| 0.10.37 | MATCH STATS ripulito, EXIT MATCH con freccia temporanea; bordi corretti. |
| 0.10.38 | MATCH STATS senza testi prestampati; EXIT MATCH con cornice gialla, senza freccia. |
| 0.10.39 | Nessuna documentazione specifica verificata; non inventare modifiche. |
| 0.10.40 | Ramo di lavoro noto; differenze specifiche rispetto alla 0.10.38 ancora da verificare nel codice/commit. |

**Nota:** 0.10.25–0.10.38 sono note di pacchetti e revisioni, non prove che ogni correzione funzioni sull'ultimo eseguibile. Le build 0.10.29, 0.10.39 e i dettagli mancanti vanno completati soltanto da fonti verificabili. `PROJECT_HISTORY.md` su `main` conserva molti README e report; il registro `IMPROVEMENTS.md` dovrà mantenere un changelog verificato.

## 12. Stato attuale e problemi aperti

**Documentato storicamente:** avvio/menu, carriera disoccupato, offerte, calendario, BUSINESS, SQUAD, Board/Fans, finanze, fullscreen, loading e pannelli di partita. Non equiparare «documentato» a «testato sull'ultima build».

**Da completare o verificare:** Living World Engine, ingresso a stagione iniziata, database mondiale, AI club e trasferimenti, POTENTIAL, TRAINING, scouting con `???`, salvataggi, esoneri, ritiro/Hall of Fame, costi e bilanci, joypad reale, MATCH STATS/EXIT MATCH, durata loading e compilazione Windows corrente.

## 13. Prossimi sviluppi

1. Verificare la base corrente e distinguere `main` da `release/0.10.40`.
2. Conservare e testare i layout già approvati, inclusi MATCH STATS, EXIT MATCH, TRANSFER 1 e loading.
3. Definire scouting `???` e quali statistiche vengono oscurate.
4. Completare il flusso trasferimenti e definire TRAINING.
5. Sviluppare la simulazione mondiale persistente e le offerte a stagione iniziata.
6. Integrare POTENTIAL, sviluppo giocatori, AI e competizioni mondiali.
7. Completare ritiro, esoneri, record e test dei salvataggi.

L'ordine deve riflettere le richieste più recenti, non le priorità obsolete dei vecchi README.

## 14. Idee future

Nazionali allenabili; giovani generati; evoluzione del blasone; mercato AI avanzato; contratti con clausole; storico mondiale; database espandibile; strumenti di editing e modding; salvataggi versionati; simulazione differita delle leghe lontane.

Queste sono idee da valutare, non funzioni automaticamente approvate.

## 15. Questioni aperte

Algoritmo POTENTIAL; invecchiamento; frequenza e impatto del TRAINING; scouting (giocatori visibili e campi `???`); bilanciamento FORM/MORALE; mercato AI; calendari sfalsati; soglie esonero; ritiro a 85 anni; copertura iniziale del database; compatibilità save e differenze reali tra rami.

## Workflow Git e numerazione correnti (6 novembre 2026)

- Branch attivo: `dev/0.10`; conservare il vecchio branch finché GitKraken e la compilazione locale non sono verificati.
- Commit soltanto su richiesta esplicita dell'utente; diversi commit intermedi possono precedere una nuova build.
- Build base attuale: `0.10.51`; commit intermedi di codice: `0.10.51_01`, `0.10.51_02` ecc.; nuova build su richiesta: `0.10.52`.
- Il titolo mostra `Build <versione>` con il suffisso intermedio quando presente.
- Codex locale è un'ipotesi di evoluzione del workflow, non ancora installata né collaudata.

## 16. Aggiornamento e continuità

Aggiornare il documento quando una decisione è confermata, implementata, verificata o superata. Non cancellare la storia: distinguere decisioni attuali, ipotesi e soluzioni storiche. Per ogni feature implementata, registrare **versione, modulo, test e commit** quando disponibili. `AGENTS.md` rimane la fonte delle regole operative, `IMPROVEMENTS.md` il registro delle modifiche realizzate.


## [2026-11-08] Frontend Title HD — build 0.10.52
- [APPROVATO] Soluzione B: SDL2 RGB/RGBA frontend moderno separato dal motore partita, che resta invariato. Sfondi PNG senza UI/logo, logo canonico separato; sprite e pannelli UI PNG distinti, testi e valori dinamici del motore.
- [APPROVATO] Standard generazione sfondi: 1672×941 nativi; crop senza resize a 1664×936 (16:9). Avvisare esplicitamente se una o entrambe le risoluzioni differiscono. Frontend logico 1920×1080, uscita di riferimento 3840×2160, futura opzione risoluzione non implementata.
- [ASSET APPROVATO] Prima immagine senza tabellone (allenatore con giocatori blu, bandiere tra il pubblico, uccelli e porta). Originale 1670×941, crop 1664×936 PNG lossless, SHA-256 locale 34ab54c7548f546ee2cc531be54a5ad366ec4bf35dba4fab2e2010c1e6c4d9b5; non confondere con ulteriori varianti.
- [CODICE PREPARATO / TEST WINDOWS DA FARE] `expanded_png.c` (RGB/RGBA 8-bit non interlacciato, miniz già incluso) e `expanded_title_overlay.c`: title PNG 1664×936 con menu legacy ancora sovrapposto, F8 attiva/disattiva il PNG per confrontarlo con l'originale, fallback automatico se asset assente. Non tocca la partita.
- [TEST LOCALE] Decoder PNG: pixel RGBA identici al file di riferimento; test isolato alpha-mask/f8 riuscito con SDL stub. Non è una compilazione né un test del gioco su Windows.
- [PENDENTE] Verificare trasferimento binario PNG tramite `asset-transfer/README.md` (blob a blocchi massimo 1 MiB, manifest SHA-256, ricomposizione Action) e test Windows di scaling, pulsanti e uscita. Il logo legacy è incorporato nel RAW e non è ancora un asset HD separato.


## [2026-11-08] Title Menu — logo canonico posizione C, build 0.10.53
- [APPROVATO — SCELTA UTENTE] Logo ispirato al vecchio titolo SWOS, scritte Sensible / WORLD OF / SOCCER / EXPANDED, senza ®. Un unico sprite riutilizzabile, **PNG RGBA trasparente 1200×440**.
- [APPROVATO] Posizione **C, alto a destra** nell'anteprima realizzata sullo sfondo realmente approvato 1664×936: **x=920, y=17, w=620, h=227**; piccolo abbastanza da non toccare gli spalti. Lo sfondo rimane invariato.
- [VERIFICATO ASSET LOCALE] PNG 1200×440 RGBA, SHA-256 `a194160cdbc9a4d3315b926b8dbee75df2dea9b7aac1d92433929279639f6b96`; PNG originale generato 2071×759 e ridotto a 1200×440. Non dichiarare risoluzione 1200×440 nativa.
- [IMPLEMENTATO NEL CODICE — WINDOWS DA PROVARE] `expanded_title_overlay.c` carica il logo una sola volta, lo disegna con alpha in posizione C dopo i vecchi pulsanti e testi, mantiene F8 PNG+logo/originale, lo sfondo PNG e i menu del motore invariati; nessun cambiamento al match engine. Manca l'asset remoto finché il PNG non viene trasferito byte per byte via blob/manifest o ASSETS_INBOX.
- [TEST LOCALE] Decoder RGBA pixel-perfect, test C isolato con SDL stub per allocazione texture, ordine di disegno, posizione relativa, mask del RAW e F8. **Non equivale a compilazione Windows reale.**


## [2026-11-08] Title Menu a due livelli — codice 0.10.53_01, test Windows pendente
- [RICHIESTA UTENTE — IMPLEMENTAZIONE PREPARATA] Sostituzione visiva del Title Menu con cinque pulsanti PNG dalla **ultima immagine statica blu** scelta dall'utente, spessore scuro sui bordi destro/inferiore, senza asset GIF. CAREER è più grande; seguono HALL OF FAME, CLASSIC MODES, OPTIONS ed EXIT GAME.
- [CODICE — DA TESTARE WINDOWS] Rendering SDL2 a texture native e pressione di 6 tick; background RGB e logo RGBA restano gli asset approvati in posizione C. Niente vecchi pulsanti o RAW del Title in output SDL, niente F8. Nessuna modifica al motore partita o ai dati di salvataggio.
- [CODICE — DA TESTARE] Sottomenu provvisorio CLASSIC MODES senza illustrazione, con callback originali e righe CONTINUE/REPLAY/SAVE quando una competizione classica è attiva; CAREER attiva espone CONTINUE senza modificare il PNG canonico.
- [CONSEGNA ASSET] ZIP `SWOS_Expanded_TitleMenu_0.10.53_01_ASSETS.zip` con cinque PNG nei rispettivi percorsi `assets/title/buttons/` da estrarre accanto a `AVVIA_SWOS.bat`. Le immagini restano separate e si considerano su GitHub solo dopo push verificato.
- [DA PROVARE] Compilazione Windows MinGW64, selezione mouse/tastiera/joypad, uscita con B/ESC, allineamento e qualità 4K, ritorno da opzioni/carreer e persistenza CONTINUE/REPLAY/SAVE; release fullscreen e finestra development massimizzata.


## [2026-11-08] Title Menu — correzioni basate sul test Windows (0.10.53_02)
- [EVIDENZA UTENTE] Screenshot Windows della build 0.10.53_01: sfondo/logo/pulsanti PNG caricati; menu troppo a sinistra sopra l'allenatore, rettangolo giallo di selezione non segue lo spessore e movimento GIÙ invertito anche nel sottomenu.
- [MODIFICHE CODICE, WINDOWS DA RITESTARE] Spostata la colonna a destra, **centrata sull'asse x=1230 del logo** approvato: CAREER (982,261,496,206), HALL OF FAME (990,479,480,91), CLASSIC MODES (990,581,480,91), OPTIONS (990,683,480,91), EXIT GAME (990,785,480,91), coordinate tela 1664×936. Logo, sfondo e PNG originali invariati; ingombro fino a y=876, build separata a y=902.
- [REGOLA — DA RITESTARE WINDOWS] Il focus giallo è una texture di alone calcolata sull'**alpha effettivo del singolo PNG**, incluso lo spessore scuro a destra e sotto. Non usare rettangolo di selezione per i pulsanti sagomati. Il sottomenu provvisorio resta senza sprite artistici e con evidenziazione rettangolare temporanea.
- [MODIFICHE CODICE, WINDOWS DA RITESTARE] Compass del motore verificato in `swos_globals.c`: 0=su,2=destra,4=giù,6=sinistra. Corretto in entrambe le schermate. Pressione ridotta da 6 a 5 tick a 50 Hz (circa 100 ms), spostamento solo verticale 2–4 px, nessuna deformazione/overshoot.
- [ANCORA DA VERIFICARE] Test su Windows 10/11 con mouse, pad e tastiera, vista in HD/4K, focus su tutti e cinque i pulsanti e nel sottomenu, transizioni ed eventuali regressioni. Nessun nuovo asset: restano validi i PNG già consegnati via ZIP.

- [BINARI GITHUB VERIFICATI DOPO NUOVO PUSH UTENTE] Commit asset `2c39dd3` sul ramo `dev/0.10`: tutti e cinque i PNG in `assets/title/buttons/` sono presenti, con Git blob SHA-1 identici a quelli dello ZIP originali. Sfondo e logo risultano anch'essi presenti nei loro percorsi GitHub. Non sovrascrivere gli asset.
