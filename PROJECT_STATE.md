# SWOS EXPANDED — PROJECT_STATE.md

**Aggiornato:** 9 novembre 2026
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
- [APPROVATO, 2026-11-09] Codex lavora sui sorgenti e asset locali; modifiche, compilazioni e test ripetibili senza commit intermedi. PNG approvati nei percorsi definitivi. Commit e push soltanto su esplicita autorizzazione. Conservare BAT e asset necessari; evitare di scambiare controlli GCC/ASan per un test reale con joypad.
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

[TEST UTENTE — COMPILAZIONE FALLITA 0.10.51_05] Il compilatore MinGW64 GCC 16.2.0 ha segnalato `_currentMenu` non dichiarato in `Expanded_ClubOverviewReapplyLayout` (`expanded_economy.c`, riga 683); i warning `-Waddress-of-packed-member` non sono l'errore bloccante. [STORICO: CORREZIONE CODICE 0.10.51_06, allora non ancora ritestata] Spostata la dichiarazione di `_currentMenu` prima dell'uso (percorso SDL), e il fallback YEAR usa direttamente l'entry 14 senza riferimenti a `_currentMenu` su altri target. Il successivo test utente generale della Business Screen è riportato in sezione 2; non equivale a un collaudo completo della build corrente.


[IMPLEMENTATO — TEST UTENTE GENERALE 2026-11-07; dettagli da riverificare sulla build corrente] L'integrazione della schermata Business carica direttamente i BMP approvati in `assets/business/business_ui_approved_2026-11-07.bmp` (320×256) e `assets/ui/controller_abc_approved_2026-11-07.bmp` (70×14, posizione 9,6), senza convertirli. Renderer SWOS su framebuffer indicizzato con composizione alpha e vecchio sfondo come fallback se manca un asset. Testo, importi, data, stadio (5 livelli), Board/Fans e faccine rimangono dinamici e vengono ridisegnati ad ogni refresh. Facilities ordinate per colonne, prima sinistra FITNESS–MEDICAL–OFFICES–SCOUTS, poi destra TRAINING–TRAVEL–YOUTH; UPGRADE apre il sottomenu esistente dello STADIUM. Navigazione joypad/mouse/tastiera riallineata e selezione/hover dinamici. WEEK e YEAR sono ora tasti indipendenti con valori finanziari correnti/stagionali; confermati popup con X per WAGES, TRAVEL, MAINTENANCE; B chiude prima il popup e poi torna indietro. Salvataggi ed economia non vengono modificati nel loro formato o nelle formule. `SWOS_EXPANDED_VERSION` incrementata a `0.10.51_05`.

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
- **BUSINESS:** schermata esistente e integrata con Board/Fans, stadio, strutture, FINANCES, WEEK/YEAR e popup. La grafica attuale è da riprogettare in una futura sessione. Conservare asset e layout precedentemente approvati come riferimenti storici fino a nuova approvazione, compreso `assets/business/business_ui_approved_2026-11-07.bmp` e la legenda controller. Nessuna modifica ora a grafica, coordinate, codice, gameplay o asset.
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

## Workflow Git e numerazione storici (6 novembre 2026)

[SUPERATO COME WORKFLOW OPERATIVO dal 9 novembre 2026; esempi di versione conservati per cronologia.]

- Branch attivo: `dev/0.10`; conservare il vecchio branch finché GitKraken e la compilazione locale non sono verificati.
- Commit soltanto su richiesta esplicita dell'utente; diversi commit intermedi possono precedere una nuova build.
- Build base attuale: `0.10.51`; commit intermedi di codice: `0.10.51_01`, `0.10.51_02` ecc.; nuova build su richiesta: `0.10.52`.
- Il titolo mostra `Build <versione>` con il suffisso intermedio quando presente.
- Codex locale è un'ipotesi di evoluzione del workflow, non ancora installata né collaudata.

## 16. Aggiornamento e continuità

Aggiornare il documento quando una decisione è confermata, implementata, verificata o superata. Non cancellare la storia: distinguere decisioni attuali, ipotesi e soluzioni storiche. Per ogni feature implementata, registrare **versione, modulo, test e commit** quando disponibili. `AGENTS.md` rimane la fonte delle regole operative, `IMPROVEMENTS.md` il registro delle modifiche realizzate.


> **Note storiche delle build:** consegne ZIP, ASSETS_INBOX, trasferimenti GitHub e assenza di accesso al PC descritti sotto riguardano le sessioni precedenti. Dal 9 novembre Codex accede alla copia Windows locale; queste note non autorizzano trasferimenti o commit automatici.

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


## [2026-11-08] Title Menu 0.10.53_03 — dimensioni e pulizia file
- [RICHIESTA / SCREENSHOT WINDOWS 0.10.53_02] Layout quasi approvato; ridurre leggermente l'intera colonna e aumentare lo spazio libero sotto il logo, senza modificare illustrazione, logo, font o PNG originali. Ispessire l'alone giallo di **1 pixel nel sorgente** e conservare il profilo esatto dei cinque bottoni.
- [CODICE — WINDOWS DA RITESTARE] Nuove hitbox/render coordinate su canvas 1664×936, tutte centrate a x=1230: CAREER (1005,308,450,187), HALL OF FAME (1014,507,432,82), CLASSIC MODES (1014,601,432,82), OPTIONS (1014,695,432,82), EXIT GAME (1014,789,432,82). Spazio verticale sotto il logo (fino a y=244) → 64 px; ultimo pulsante termina a y=871, build separata a y=902.
- [CODICE — WINDOWS DA RITESTARE] Raggio maschera alpha del focus da 4 a 5 px nei pixel nativi dello sprite. Nessun nuovo asset binario o conversione; animazione e controlli della 0.10.53_02 invariati.
- [PULIZIA GIT] Rimossi gli script temporanei di test 0.10.52/0.10.53, relativi LEGGIMI e README dello ZIP ormai archiviato, screenshot di debug, anteprima del Title RAW superato, immagine test 2×2 per trasporto e tre manifest di trasferimento già conclusi. Tutte le copie approvate in `assets/` restano. `.gitignore` blocca l'aggiunta involontaria delle copie locali di tali file.
- [PRESERVATO DOPO CONTROLLO] `data/expanded/expanded_title_bg.raw` è ancora referenziato da `expanded_economy.c` (caricamento `ui_load`) e richiesto da `BUILD_SWOS.sh`. Non rimuoverlo senza cambiare e collaudare entrambe le dipendenze. Conservate anteprime/RAW dei menu ancora in uso, sorgenti e documenti storici.
- [DA TESTARE] Sincronizzazione GitKraken che recepisce le cancellazioni, Windows build/avvio, focus giallo sui cinque pulsanti e mouse/joypad, corretto spazio sotto il logo, sottomenu Classic Modes. Le prove del gioco sul PC dell'utente non sono sostituite da controlli statici.


## [2026-11-08] Title Menu 0.10.53_04 — CONTINUE CAREER e nuova riduzione
- [ASSET APPROVATO] `CONTINUE CAREER` blu, stesso stile dei precedenti, senza bordo giallo baked-in. File generato PNG RGBA **2065×761 px**, non interlacciato, byte conservati dal PNG dell'immagine approvata; SHA-256 `19e3858030e140ec5f55e9211816da59b9f12cb86b78a9c44cc87ddcfd474df0`.
- [CONSEGNA ASSET] ZIP `SWOS_Expanded_Continue_Career_0.10.53_04_ASSET.zip` con `assets/title/buttons/continue_career.png` da estrarre accanto ad `AVVIA_SWOS.bat`. NON dichiarare il nuovo asset salvato su GitHub fino a push verificato. I cinque sprite precedenti rimangono approvati e già presenti nel repository.
- [CODICE — DA TESTARE SU WINDOWS] Quando una carriera è attiva, il renderer seleziona `continue_career.png` come sprite completo di primo livello, preservando la callback di continuazione. Rimossa la scritta `CONTINUE` aggiunta via `pixel_text` sopra CAREER: nessun testo di sistema sovrapposto. In assenza PNG nuovo, fallback al pulsante CAREER originale con avviso nel log.
- [CODICE — DA TESTARE SU WINDOWS] Nuovo layout 1664×936, tutti con asse x=1230.5: CAREER/CONTINUE (1028,350,405,168); HALL OF FAME (1036,529,389,74); CLASSIC MODES (1036,611,389,74); OPTIONS (1036,693,389,74); EXIT GAME (1036,775,389,74). Circa 10% più piccoli dei rispettivi riquadri 0.10.53_03. Margine sotto logo 106px invece di 64; intervalli inferiori ridotti a 8px.
- [CODICE — DA TESTARE SU WINDOWS] Il PNG CONTINUE ha rapporto ~2.71 invece di ~2.41 del pulsante CAREER: renderer esegue **contain** centrato nella hitbox comune per evitare distorsioni. Il focus deriva da alpha: raggio 6 px nei cinque PNG originali (5+1) e raggio 12 px nel PNG CONTINUE a risoluzione quasi doppia, per lo stesso spessore visivo nel layout. Sempre separato dall'immagine base. Gli altri asset, partita, salvataggi, callback e sottomenu rimangono invariati.
- [REGOLA] Nessun pulsante UI base approvato deve includere la cornice gialla della selezione; separare artwork statico e feedback focus/hover dinamico.
- [TEST] Verifica PNG RGBA/8bit/non-interlacciato e SHA-256 locale e controlli statici sorgente; **build e test Windows ancora pendenti**.

## [2026-11-08] Nuova build 0.10.54 — icone fullscreen/windowed e menu più basso
- [APPROVATO] Coppia di icone quadrate blu pixel art senza testo, spessore 3D coerente con i piccoli pulsanti: frecce verso l'esterno per entrare in fullscreen (modalità attuale finestra), frecce verso l'interno per tornare in finestra (modalità attuale fullscreen).
- [ASSET APPROVATI, NON ANCORA VERIFICATI SU GITHUB] `assets/title/buttons/fullscreen_expand.png` RGBA 1044×1009, SHA-256 `6bc13d1df16ea7331dac769ff8cd02919248b4ea2f39be8162529f3f9c306559`; `assets/title/buttons/windowed_collapse.png` RGBA 838×802, SHA-256 `4808d10a02ed70a97bea7d76709c8239c96224f0dc131dc7970f3962e1fd2863`. Cropping puro dalle immagini approvate con rimozione del rumore alpha esterno; **nessun resize dei pixel**. PNG 8bit RGBA non interlacciati.
- [CONSEGNA ZIP] `SWOS_Expanded_0.10.54_ASSETS.zip` con entrambe le icone nuove **e** `continue_career.png` (ancora da sincronizzare sul remoto se non presente), nei percorsi `assets/title/buttons/`, da estrarre nella cartella contenente `AVVIA_SWOS.bat`. Lo ZIP è distinto dal commit codice GitHub.
- [CODICE, TEST WINDOWS PENDENTE] Primo livello Title con sei voci selezionabili: le cinque voci testuali e icona video `EXP_TITLE_VIDEO_INDEX=5`, visibile in alto a sinistra nel rettangolo (58,56,92,88) del canvas 1664×936. SU da CAREER → icona e GIÙ da EXIT → icona, con navigazione ciclica. Mouse usa la stessa hitbox. Callback icona richiama `Platform_ToggleFullscreen()`, già in uso per Alt+Invio. Il renderer interroga `Platform_GetFullscreenMode()` ogni frame, mostrando soltanto l'immagine della prossima azione.
- [CODICE, TEST WINDOWS PENDENTE] Ripristino della finestra massimizzata reale dopo uscita da fullscreen quando la finestra era massimizzata prima del toggle; la finestra ordinaria conserva la risoluzione/scala; profilo release non alterato. Resa più efficiente la generazione dell'alone alpha per le icone ad alta risoluzione (passate separabili O(w×h)).
- [LAYOUT PROPOSTO DA RITESTARE] Tutti i cinque pulsanti principali un altro ~10% più piccoli e più in basso, centro x=1230.5: CAREER (1048,405,365,151), HALL OF FAME (1055,570,350,67), CLASSIC MODES (1055,648,350,67), OPTIONS (1055,726,350,67), EXIT GAME (1055,804,350,67). Logo canonico finisce a y244, quindi 161px sopra CAREER. Build rimane in sovraimpressione y902.
- [NON MODIFICATI] Logo, sfondo, cinque PNG testuali e CONTINUE CAREER, callback SWOS, partita e salvataggi; netplay escluso. Non dichiarare compile/test Windows riusciti senza evidenza.

## [2026-11-08] Commit intermedio 0.10.54_01 — Title: navigazione e spaziatura
- [RICHIESTA / REGOLA] Lo **stile canonico dei nuovi pulsanti Expanded** è quello dei pulsanti Title approvati: frontale, bordo blu/ciano, profondità scura a **destra e sotto**, PNG base senza focus giallo generato e pressione con **bouncing discreto** gestito via SDL2. Regola in `AGENTS.md`; nessun artwork approvato rigenerato.
- [CODICE — TEST WINDOWS PENDENTE] Spostata l'icona video in alto a sinistra da (58,56,92×88) a **(34,34,92×88)** nel canvas 1664×936, con hitbox e rendering condivisi. Navigazione del primo livello, interazione mouse/pad/tastiera e callback fullscreen restano le stesse.
- [CODICE — TEST WINDOWS PENDENTE] Avvicinati ulteriormente i cinque pulsanti, conservandone dimensioni, centro orizzontale e **posizione inferiore**: CAREER (1048,426,365×151), HALL (1055,585,350×67), CLASSIC (1055,658,350×67), OPTIONS (1055,731,350×67), EXIT (1055,804,350×67). Il pulsante EXIT termina ancora a y871, il build stamp è sempre separato a y902; sopra CAREER ci sono 182px rispetto al fondo del logo a y244.
- [CODICE — TEST WINDOWS PENDENTE] Contorno giallo +2 pixel sorgente per i PNG principali (**6→8**), proporzionale a risoluzione per CONTINUE CAREER (**12→16**) e icone video (**22→30** espansione, **18→24** riduzione). Il renderer genera la selezione, non modifica i PNG approvati.
- [CODICE — TEST WINDOWS PENDENTE] Pressione unificata in 5 tick con traslazioni verticali **1,3,4,2,-1,0 px** (breve ritorno elastico di 1px) e nessuna trasformazione dei PNG; comune a pulsanti principali, icona video e secondo livello provvisorio. Resta invariata la frequenza partita 50Hz.
- [TEST] Verifiche statiche in GitHub; non è stato eseguito un test Windows. Da controllare focus sulle due icone, hover/joypad, animazione e variazione della finestra prima della convalida.

## [2026-11-08] Logo canonico Title corretto (solo asset)
- [APPROVATO] L'utente ha approvato la **correzione esclusiva della lettera finale in WORLD OF**: il vecchio PNG visualizzava WORLO OF. Conservare identici logo, colori, proporzioni, trasparenza e posizione; **nessuna rigenerazione dell'illustrazione**.
- [GITHUB, SOSTITUZIONE STESSO PERCORSO] `assets/title/swos_expanded_logo_approved_2026-11-08.png`, 1200×440 RGBA 8bit, non interlacciato. Blob precedente `e5335fa4507ccca3e101802c544c53a8fff37287` sostituito sul ramo `dev/0.10` da blob corretto `7358520ea0b72206988d3bbc16a9e70c0afe633c`: **1062 soli pixel della lettera modificati**, resto del PNG GitHub preservato pixel-per-pixel; codifica lossless PNG ricostruita. Vecchia versione non più nell'albero corrente; come qualunque commit Git, sopravvive nella cronologia storica.
- [POSIZIONE INVARIATA] `expanded_title_overlay.c` continua a caricare lo **stesso nome** e a disegnare il logo x=920 y=17, 620×227 sul canvas 1664×936. Sfondo e pulsanti non modificati, build visualizzata non incrementata perché commit solo asset/documentazione.
- [CONSEGNA PC] Il PNG preciso approvato in chat è disponibile anche nello ZIP `SWOS_Expanded_Logo_WORLD_corretto_0.10.54_01.zip`, col percorso identico per sovrascrittura. Per eliminare la vecchia copia dal PC usare GitKraken pull o sostituire con ZIP; l'assistente non accede direttamente al disco Windows dell'utente. Non mantenere duplicati/backup dell'asset precedente.
- [TEST] Formato e geometria verificati; render/test Windows tramite `AVVIA_SWOS.bat` ancora a carico dell'utente.


## [2026-11-08] 0.10.54_02 — finestra development e ritorno da fullscreen
- [CORREZIONE CODICE / WINDOWS DA TESTARE] In `Platform_Init`, la presenza di `SWOS_EXPANDED_DEV_MAXIMIZED=1` forza la modalità iniziale windowed anche se `SWOS.ini` contiene `fullscreen=desktop`, senza usare borderless e senza alterare l'opzione release.
- [CORREZIONE CODICE / WINDOWS DA TESTARE] Una sessione avviata direttamente fullscreen ora ricorda un ritorno alla finestra decorata massimizzata; prima non aveva uno stato finestra precedente e riduceva la finestra al fattore di scala SWOS. I passaggi fullscreen↔finestra con icona Title e Alt+Invio mantengono la massimizzazione in sviluppo.
- [NON MODIFICATI] Immagini/asset, coordinate UI, motore partita 50Hz, gameplay e salvataggi. Il ramo `release/0.20` non riceve questa modifica di sviluppo automaticamente.
- [TEST WINDOWS RICHIESTI] `AVVIA_SWOS.bat` da `dev/0.10` deve avviare finestra massimizzata con titolo/taskbar; icona Title e Alt+Invio devono alternare fullscreen desktop e finestra massimizzata senza finestre minuscole; test menu→partita→menu. Avvio EXE isolato senza variabile resta governato da impostazioni video.


## [2026-11-08] 0.10.54_03 — elimina flash bianco prima del loading (Windows da verificare)
- [MODIFICA CODICE] In `platform_sdl.c` SDL crea la finestra desktop nascosta; dopo il primo `SDL_RenderPresent` completo viene mostrata una sola volta. Per WASM resta il comportamento precedente, con canvas visibile.
- [MOTIVO] Evitare che il sistema operativo esponga la superficie SDL non ancora disegnata tra creazione finestra, massimizzazione e caricamento grafico.
- [NON MODIFICATI] Splash e progressi di caricamento, asset, Title, posizione finestra, gameplay, controlli e salvataggi; release/0.20 non modificata.
- [TEST DA FARE SU WINDOWS] Avvio senza frame bianco, finestra decorata già massimizzata e ciclo fullscreen tramite icona/Alt+Invio, menu→partita→menu, splash visibile e aggiornato. Nessun test Windows eseguito qui.


## [2026-11-08] 0.10.54_04 — primo frame visibile: schermata di caricamento Expanded
- [MODIFICA CODICE / WINDOWS DA TESTARE] La finestra SDL2 rimane nascosta durante eventuali frame neri del bootstrap. Il primo `SDL_ShowWindow` avviene solo dopo `SDL_RenderPresent` di un frame con `s_expandedSplashActive` e sfondo caricamento disponibile. La percentuale continua a rappresentare le fasi reali, partendo da 0%; nessun tempo artificiale aggiunto.
- [FALLBACK] Se il BMP dello splash manca o la texture non può essere creata, `Platform_ExpandedSplashSet` mostra il primo frame legacy anziché lasciare la finestra invisibile. È ammessa anche la comparsa diretta al Title nelle modalità che saltano lo splash.
- [NON MODIFICATI] Asset e layout del loading, Title PNG, renderer partita/50 Hz, input e salvataggi. `release/0.20` non modificata.
- [TEST WINDOWS] Avvio con splash visibile subito, niente flash nero/bianco, avanzamento progressivo, finestra development massimizzata e icona fullscreen/Alt+Invio. Verificare anche il fallback con asset caricamento mancante in un ambiente di test, senza cancellare l'asset approvato.


## [2026-11-08] 0.10.54_05 — presentazione del primo frame di caricamento
- [CODICE] `platform_sdl.c`: la finestra rimane nascosta durante i frame neri iniziali, ma `SDL_ShowWindow` si esegue **prima** di `SDL_RenderPresent` del primo frame già composto con lo splash Expanded. Questo evita di rendere visibile la finestra dopo una presentazione effettuata da nascosta, che può causare un breve nero sul backend Windows.
- [FALLBACK PRESERVATO] Se lo splash non è disponibile o viene saltato direttamente verso il Title, la finestra diventa comunque visibile; la modifica non cambia asset, progressi, gameplay, salvataggi, né `release/0.20`.
- [TEST WINDOWS DA ESEGUIRE] Avvio da BAT, durata del passaggio nero, rendering dello splash e avanzamento percentuale, fullscreen/Alt+Invio e ritorno finestra massimizzata. Nessun test Windows rivendicato.


## [2026-11-08] 0.10.54_06 — splash anticipato e CONTINUE CAREER luminoso
- [CODICE / WINDOWS DA TESTARE] In `Platform_Init`, lo splash Expanded al progresso reale 0% viene caricato e presentato **appena renderer, frame textures, mutex, input/audio e timer sono pronti**, prima della decodifica dei grandi PNG Title e dei contorni alpha. `mainC` conserva tutte le fasi e le percentuali di caricamento; se manca lo splash rimane il percorso fallback. Misura nel log il tempo di preparazione Title dopo lo splash.
- [CODICE / WINDOWS DA TESTARE] `CONTINUE CAREER` quando esiste una carriera usa una texture aggiuntiva cyan RGBA generata una volta dall'alpha approvato (raggio sorgente 42, alpha 135), più un passaggio di illuminazione additiva delicata, ripristinando subito blend/color/alpha della texture. Focus giallo e bounce rimangono separati. Nessun PNG modificato.
- [LIMITAZIONE] Il tempo per la creazione nativa della finestra/renderer SDL resta prima dello splash. Il nero residuo può dipendere anche da Windows/D3D; serve il test reale, senza promettere eliminazione totale.
- [INVARIATI] Gameplay 50Hz, salvataggi, asset, altri pulsanti, layout, release/0.20. Test richiesti: avvio nero/splash, progressi, continuità menu↔partita, illum. CAREER vs CONTINUE, focus e fullscreen.


## [2026-11-08] 0.10.54_07 — CONTINUE CAREER senza glow, tonalità viola
- [CODICE / WINDOWS DA TESTARE] Rimosso completamente il contorno cyan generato dall'alpha e il passaggio additivo di illuminazione. Il PNG approvato `continue_career.png` rimane immutato: quando la carriera è attiva, il renderer SDL2 applica una modulazione colore `RGB(255,120,215)` al solo pulsante, ottenendo una tonalità viola più riconoscibile rispetto agli altri bottoni blu.
- [INVARIATI] Geometria, scala, focus giallo, bounce, altri pulsanti, gameplay, splash anticipato della build precedente. Effetto visivo da verificare sul PC Windows; colore ulteriormente regolabile su richiesta.


## [2026-11-08] 0.10.54_08 — CONTINUE CAREER rosso, testo bianco
- [CODICE / TEST WINDOWS PENDENTE] La tonalità viola precedente applicata a tutto il PNG è stata rimossa. All'atto del caricamento SDL2 del solo `continue_career.png`, una copia temporanea dei pixel RGB blu/indaco viene convertita in rossi con gradazioni coerenti, preservando alpha, ombre e profondità. I pixel neutri, la scritta bianca e il bordo nero restano invariati. Il PNG su disco NON viene sovrascritto.
- [INVARIATI] Layout, focus giallo da alpha, bouncing e altri pulsanti. Avvio/loading e partita immutati. Verificare su Windows che tutte le parti blu del pulsante siano rosse e il testo rimanga bianco; valori di colore ritoccabili.


## [2026-11-08] 0.10.54_09 — CONTINUE CAREER verde
- [CODICE / TEST WINDOWS PENDENTE] Sostituita la conversione selettiva rosso con verde nel renderer SDL2, sul solo `continue_career.png`. Cambiano esclusivamente i pixel delle superfici blu/indaco in una copia decodificata in RAM; la scritta bianca, contorni neri, ombre, alpha e geometria sono conservati. PNG canonico intatto. Focus giallo e bouncing invariati.
- [TEST] Verificare tonalità verde e scritta bianca sul PC Windows dopo Pull; nessuna compilazione o verifica grafica Windows eseguita qui.

## [2026-11-09] Workflow locale e manutenzione, senza nuova build

- Base verificata: `dev/0.10`, HEAD `4290e8c7cb2cc3864a26cbbf9cf7b6c98604a662`, versione `0.10.54_09`.
- Codex modifica sorgenti e asset direttamente nel repository Windows. PNG approvati nei percorsi definitivi; iterazioni, compilazioni e test locali senza commit intermedi. Commit e push solo su esplicita autorizzazione.
- Abolito il workflow ordinario ZIP/ASSETS_INBOX/trasferimenti GitHub e revocate le autorizzazioni automatiche ai commit asset. Rimossa soltanto ASSETS_INBOX/.gitkeep e la cartella vuota; documentazione storica conservata, job di ricomposizione disabilitato.
- BUSINESS esiste ed è integrata; grafica da riprogettare in una futura sessione. Asset e layout precedenti restano riferimenti storici protetti fino a nuova approvazione. Nessun codice o asset BUSINESS modificato.
- Messaggi operativi degli script resi neutri; nomi legacy del log e marker conservati per compatibilità. Nessun incremento di versione.
- BUILD_SWOS.sh verifica GCC MinGW64, SDL2, pkg-config e Make senza installazioni o aggiornamenti automatici. Nessuna compilazione o esecuzione del gioco svolta durante questa manutenzione.

## [2026-11-09] Configurazione definitiva del workflow Codex

- [APPROVATO] Regole fondamentali e sequenza di lavoro consolidate all'inizio di `AGENTS.md`; modalità di applicazione in `docs/DEVELOPMENT_WORKFLOW.md`. Le alternative tecniche sono da ricercare e proporre spontaneamente, ma nessuna proposta autonoma è autorizzata all'implementazione senza approvazione esplicita.
- [DECISIONE DEFINITIVA] CLASSIC MODES rimane una voce principale del Title Menu; non è una questione aperta.
- [STATO LOCALE] Ramo `dev/0.10`, HEAD `4290e8c7cb2cc3864a26cbbf9cf7b6c98604a662`; modifiche locali delle sessioni precedenti conservate, inclusa la data generata in `sorgenti/include/swos_build.h`. Versione invariata `0.10.54_09`. Questo intervento aggiorna soltanto i tre documenti.
- [VERIFICHE PRECEDENTI, NON RIPETUTE IN QUESTO INTERVENTO] Prima compilazione Windows incrementale riuscita in 7,64 s con 20 warning `-Waddress-of-packed-member`; eseguibile verificato. Primo avvio raggiunto, log development massimizzato e controller riconosciuto; chiusura volontaria confermata dall'utente.
- [TEST PENDENTI] Conferma visiva del CONTINUE CAREER verde e delle transizioni fullscreen/windowed, collaudo completo della navigazione e dei flussi. L'analisi statica del Career Menu non equivale a un test runtime né autorizza automaticamente la proposta di renderer HD.

### 0.10.56 — NEW CAREER (2026-11-09)
[CODICE MODIFICATO, TEST WINDOWS PENDENTE] UI 88,5% autorizzata per lasciare visibili tavolo e penna, sfondo senza resize. Testi bitmap diritti a pixel interi; checkbox nel pulsante START UNEMPLOYED; FAME cicla senza popup; frecce decorative. Logica carriera e formato salvataggi invariati. Workflow ChatGPT/GitHub/GitKraken attivo, Codex storico.

### NEW CAREER 0.10.57 (2026-11-09)
[IMPLEMENTAZIONE CODICE, TEST WINDOWS PENDENTE] Font in atlas PNG lossless formato/form/button/title, coerente in forma con il mockup di riferimento e non soggetto a shrink 0,885; focus sottile dei pulsanti basato sulla sagoma, focus singolo dei campi; AGE 18-70 non apre InputText; cursore '|' per NAME/SURNAME; separazione visiva di 4px tra colonne. Asset nuovi consegnati via ZIP nei percorsi runtime. Non modificati sfondo e UI sorgenti approvati, gameplay o salvataggi.

---

## SWOS_DOC_SYNC_2026_11_10 — Stato e decisioni aggiornati (10/11/2026)

**Riferimento esaminato:** branch `dev/0.10`, HEAD `f33041df7204936da22e6a066977b0dc382093b4`, versione sorgente `0.10.75`. **Non è stata generata né testata una nuova build** in questo aggiornamento. `docs/GAME_DESIGN.md` diventa la fonte tematica delle scelte e idee di gioco, da consultare insieme al codice.

### Decisioni confermate

- **Design grafico:** SOLO TITLE, NEW CAREER, LOADING hanno grafica considerata finale. Le vecchie approvazioni grafiche di BUSINESS/SQUAD/MATCH STATS/TRANSFER 1 e degli altri menu sono **superate** per il *layout e gli asset*; dati e callback restano da salvaguardare. Tutte le altre schermate saranno ridisegnate con approvazione singola.
- **Obsolescenza:** tutti gli asset grafici della precedente fase di menu diversi dalle tre schermate finali sono **OBSOLETI DA SOSTITUIRE**, non necessariamente **orfani eliminabili**. Il prossimo BAT di pulizia deve eliminarli **solo quando non vi sono più dipendenze attive**; preservare fallback necessari, sprite condivisi e asset di gameplay. Nessuna cancellazione effettuata ora.
- **Base dati prima di tutto:** completare la carriera con squadre/competizioni già disponibili. L'espansione fino a molte divisioni/paesi (obiettivo discusso: fino a 8 livelli per paese quando pertinente) e relativi database è posticipata all'ultima fase.
- **Mondo unico:** tempo **a date/eventi**, non un incremento soltanto settimanale; i paesi possono avere calendari differenti e coppe infrasettimanali. Simulare risultati, classifiche, mercato, ritiri e progressione a ogni evento; elaborare periodicamente economia e allenamenti. Maggior dettaglio solo dove serve, senza partite simulate fisicamente per ogni club.
- **Settimana in UI:** numero della settimana visibile nella maggioranza delle schermate della carriera e una sola origine temporale. Convenzione ISO vs stagione ancora da definire, quindi NON hardcodare una formula nuova senza discuterla.
- **Nuova azione:** ADVANCE TO NEXT EVENT in CALENDAR e UNEMPLOYED; deve avanzare attraverso tutti gli eventi senza ignorarli o prendere decisioni autonome.
- **Chairman:** nome/età nel contratto e WELCOME; ritiro e successore automatico nello stesso club (soglie ancora da definire).
- **Giocatori:** YOUTH/RES/TITOLARE come status di rosa, sviluppo/ritiro, giovani stagionali e potenziale conosciuto per i giovani della propria squadra, esterni da osservare; niente prestiti TRIAL automatici. Semantica originale dei campi SWOS da verificare prima di cambiare codice.

### Codice esaminato e limiti da correggere

- `sorgenti/src/expanded_career.c`: il disoccupato usa `s_waitWeeks`; `Expanded_UnemployedWait()` incrementa settimane e genera offerte; `Expanded_WaitForRehire()` nota che il reingaggio avvia una nuova stagione anziché entrare nella stagione reale in corso. **Da correggere nel Living World Engine**, non considerare già risolto.
- `sorgenti/src/expanded_economy.c`: CALENDAR e UNEMPLOYED condividono in parte il rendering delle fixture e dei pulsanti. I layout sono **da riprogettare**; non cancellarne prima le risorse RAW ancora caricate.
- `sorgenti/include/build_info.h`: `SWOS_EXPANDED_VERSION` vale `0.10.75` alla lettura. È distinto da vecchi screenshot/riassunti 0.10.54; verificare di nuovo il prossimo HEAD.
- Alcuni file `assets/title/buttons/*_OBSOLETO.png` e `data/expanded/*_OBSOLETO.*` sono già marcati come obsoleti nel nome. **Non eliminare** i file del nuovo TITLE/LOADING/NEW CAREER solo per errore di percorso o per somiglianza.

### Ordine operativo corrente

1. Audit del tempo e del calendario; definire transizione ad eventi cronologici persistenti con il database attuale, test regressione.
2. Offerte che rispettano stagione reale, chairman, contratto; grafica OFFER demo prima della versione finale.
3. NEXT EVENT + WEEK visibile quasi ovunque, senza fissare il vecchio layout.
4. Giovani, scouting, mercato AI, ritiri e finanze/allenamento.
5. Verifiche Windows/joypad/save di lunga durata; grafica dei menu rimanenti; database allargato solo alla fine.

### Test e consegne

**Solo documentazione predisposta.** Nessuna compilazione, nessun test Windows, nessuna modifica al codice di gioco, nessun commit e nessun push eseguiti per questa raccolta. Aggiornare questo stato dopo i test reali; dettagli e stati [APPROVATO]/[PROPOSTA]/[DA DEFINIRE] sono in `docs/GAME_DESIGN.md`.
