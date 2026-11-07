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
- [APPROVATO] Joypad, mouse e tastiera; tutti i pulsanti devono essere raggiungibili. Le regole operative correnti del joypad e degli hover sono in `AGENTS.md`.
- [APPROVATO] GitHub come archivio centrale, GitKraken per aggiornare la copia locale, compilazione/test Windows sul PC dell'utente. Conservare BAT e asset necessari; evitare di scambiare controlli GCC/ASan per un test reale con joypad.
- [DA VERIFICARE] Contenuti e funzionamento complessivi del ramo `release/0.10.40`, differenze da `main`, compatibilità salvataggi, stato dei launcher e degli script.

## 3. Mondo calcistico e competizioni

[APPROVATO] Campionati, più divisioni, coppe nazionali, tornei continentali, promozioni/retrocessioni e in prospettiva nazionali/Mondiale. Ogni paese avrà la propria struttura calcistica appropriata.

[DA DEFINIRE] Copertura iniziale, stagioni sfalsate, regole concrete di playoff e qualificazioni, numero di divisioni e database. Le idee di struttura dati per competizioni e federazioni restano proposte tecniche, non un sistema già implementato.

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
