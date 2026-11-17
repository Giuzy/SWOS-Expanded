# SWOS EXPANDED — PROJECT_STATE.md

## 17/11/2026 — 0.13.70_9, pulizia root (Windows NON PROVATO)
- Unici BAT di avvio root: AVVIA_DIAGNOSTICA.bat e AVVIA_SWOS.bat. COMPILA_SWOS.bat e PREPARA_SWOS.bat sono in diagnostica/; tutti i riferimenti di avvio e compilazione sono aggiornati. BUILD_SWOS.sh scrive fuori dalle sessioni in diagnostica/log/COMPILAZIONE_0140.log; main.c su Windows redirige stderr in diagnostica/log/SWOS.log anche con avvio diretto. La diagnostica cattura solo il log della sessione corrente, archivia quelli precedenti e lascia tutti i .CAR invariati. Marcatori .SWOS_*_COMPILATO e backup SWOS.ini sono spostati in diagnostica/ (i preesistenti sono migrati dal launcher soltanto senza collisioni). La copia ASSETS.ini root viene eliminata dal repo: quella effettivamente letta e' assets/ASSETS.ini.
- SDL2.dll e SWOS.ini rimangono accanto all'EXE: il loader Windows e la funzione swos_ini_path() li richiedono ancora nella posizione originale; nessun cambiamento ai salvataggi in save/. I file locali non tracciati non possono essere spostati dalla sola operazione Git: la migrazione avviene alla successiva esecuzione del launcher, quando non esiste un file di destinazione omonimo. Verifiche Windows e compilazione PENDENTI.

## 17/11/2026 — 0.13.70_8, snapshot OG nella carriera — test Windows PENDENTE
- Il `.CAR` porta ora `og_resident` (lega OG/UEFA/coppe nazionali effettivamente presenti, comp/torneo/squadre/RNG, ID e fasi). Il decoder precontrolla interi contesti prima della modifica; i puntatori vengono ricostruiti dopo LOAD e l'anno OG viene riallineato **senza replay**. Salvati separatamente AUTO9697 e AUTO2627 con sostituzione atomica.
- Formato tecnico v1 (blob esadecimali di stato nativo ABI-checked dentro JSON), non ancora schema campo-per-campo multipiattaforma; non contiene i tornei mai inizializzati come Libertadores. Un vecchio .CAR non contiene i residenti OG e viene rifiutato dal nuovo restore anziche' creare dati simulati. File legacy preservato.
- Compilazione Windows, SAVE/RESTORE, FORWARD TIME, stagioni multiple, join/leave club, prove per entrambe le epoche: **NON PROVATI**. Non dichiarare crash risolto prima del test.


## 17/11/2026 — 0.13.70_5, confronto fingerprint SAVE/RESTORE OG tra due processi
- `career_status.c` produce fingerprint READ ONLY per ogni lega OG, coppa nazionale e UEFA residente su SAVE_AFTER e RESTORE_AFTER: identifica mondo/paese/slot/stagione, hash pointer-free di tabelloni, classifiche, round/fixture scratch, rose giocatori e RNG, senza includere puntatori soggetti ad ASLR. `run_save_continuity_audit.sh` compara soltanto lo stesso slot e mondo in due sessioni AVVIA_DIAGNOSTICA, confronta l'ultimo checkpoint, evidenzia contesti assenti o cambiati come FAIL. Uguale fingerprint non equivale ad affermare completo il CAR: risultato INCOMPLETE, fino ad audit di tutti i campi e assenza di gare duplicate dopo un FORWARD TIME. Test Windows e verifica file CAR NON PROVATI.


## 17/11/2026 — 0.13.70_4, Libertadores 1997: 21 club e ID auditati
- Fonte originale Mod 1997 (`aminta/swos-9697-mod/tools/sacups.py`): 20 club in 5 gironi + River Plate campione 1996 agli ottavi (totale **21 iscritti**, non 20); `tools/lib97.py` gestisce il bypass. Confrontate 21/21 identità con i JSON storici Expanded: 6 ID incompatibili, conversione registrata in `docs/reference/libertadores_1997_source_id_map.json`. Questo e' un MANIFEST, non una connessione giocabile, senza simulatori, fixture o punteggi nuovi. Windows NON PROVATO.


## 17/11/2026 — 0.13.70_3, audit multi-fonte in corso
- Documento: `docs/COMPETITION_AUDIT_SOURCE_MATRIX_20261117.md` (confronto per 48 formati/divisioni cambiati, 26 record aggiunti moderni, 1 record CUSTOM solo storico; catalogo mancanze storico/moderni/nazionali). Dati GOG e SWOS 2020 non presenti come inventari indipendenti nel repository, quindi confronto integrale a tre NON certificabile. Aggiunta dichiarazione locale `extern` per la nuova diagnostica OG NO DATA compilabile; resto motore invariato. Versione `0.13.70_3`, test Windows NON PROVATO.


## 17/11/2026 — 0.13.70_2
- OG NO DATA per definizioni e contesti da leggere in diagnostica (START, SAVE, RESTORE). L'utente conferma che statali brasiliani e COPPA ITALIA SERIE D NON compaiono nel calendario mondiale OG: MANCANTI, non solo incerti. Manca ancora integrazione effettiva, calendario e salvataggio residenti; la sola definizione Mod non vale come progresso del motore. Per nazionali la diagnostica evita falsi allarmi su anni senza gare previste. Test Windows NON PROVATO.


## 17/11/2026 — 0.13.70_1 (primo checkpoint a commit per funzione)
- OFFERTA -> VIEW COMPETITIONS ora chiama lo stesso OG picker direttamente sul paese della squadra offerente per WORLD_9697 e WORLD_2627. Europa/altre internazionali rimangono in WORLD e nel menu del club qualificato: posizione/layout non modificati. BACK e viewer restano originari.
- Audit 96/26, Libertadores 1997, Champions nel calendario post-ingaggio, save/restore OG e OG NO DATA ancora da completare. Compilazione/test Windows NON ESEGUITI dall'assistente; l'utente sincronizza con GitKraken e prova tramite BAT ufficiali dopo il ciclo concordato.


## 17/11/2026 — prossima versione 0.13.69, test Windows NON ESEGUITI
- Source update solo WORLD_9697 in CAREER: quattro RNC OpenSWOS originali in data/9697/TEAM.012/.013/.038/.073, campionati OG previsti FI 12 / FR 20+22 / TR 18 / USA 10. I file legacy data/TEAM.NNN e JSON 2026/27 NON sono sovrascritti; old saves NON cancellati. L'eventuale incompatibilita' dei vecchi salvataggi 96 richiede collaudo esplicito.
- OFFER 9697 apre il picker mondiale OG, rendendo raggiungibile la Champions originale nel ramo EUROPE; OFFER 2627 conserva selezione del paese. Fix B/ESC in picker (latch fino al rilascio fisico), senza alterare il gameplay.
- DEBUG WORLD: ABORT nativo #88 non e' piu' visibile, non possiede callback ed e' scollegato dalla navigazione in quel menu. TITLE: piccolo DEBUG NEW CAREER. NEW CAREER: piccolo 96/26 in alto a destra in sostituzione del banner temporaneo; asset approvati non modificati.
- Non e' un risultato Windows: verificare START/FORWARD TIME e lega 96 dei quattro paesi, Champions OFFER+DEBUG, back con joypad, persistenza OG 9697/2627, blocco calendario 96, multi-stagione. La sessione 26 certifica la precedente 0.13.68, non questo codice.


## Aggiornamento 17/11/2026 — 0.13.68 candidata (menu)
La diagnostica Windows della **0.13.67** conferma che START 1996/97 non crasha piu' e che il cambio statico 9697/2627 funziona nella stessa sessione. Quattro leghe 96/97 rimangono escluse per mismatch numero di squadre (country/division 12/0, 13/0, 38/0, 73/0), senza generare incontri; non dichiarare mondo completo. L'utente conferma che DEBUG WORLD e la rosa dell'offerta non navigano correttamente. La 0.13.68 modifica solo la navigazione dei due menu readonly: wrap della lista intera e guardia sul sentinel -1 delle voci invisibili. **Modifica di codice, compilazione/test Windows 0.13.68 NON PROVATI; save e gameplay non modificati.** Vedere `ENGINE_CODE_DISCOVERIES.md` per la riproduzione esatta.

**Edizione originaria:** 9 novembre 2026 · **Sintesi aggiornata:** 15 novembre 2026
**Repository:** `Giuzy/SWOS-Expanded`  
**Ramo di riferimento al momento della stesura:** `dev/0.10` (il vecchio `release/0.10.40` resta temporaneamente come backup)  
**Base:** Sensible World of Soccer / OpenSWOS · C / SDL2 · Windows x64  
**Stato:** sviluppo attivo; la presenza nel repository non equivale a collaudo Windows.

## Stato vigente — 15 novembre 2026
**OG e' l'unica autorita' sportiva** di 1996/97 e 2026/27. Il simulatore di risultati autonomo LWE e' disattivato; tutte le relative procedure precedenti sono storia, non piu' architettura da usare. La 0.13.45_08 contiene leghe OG residenti con allineamento al 1 luglio a START, viewer DEBUG WORLD read-only e offerte che consultano gli stati OG. Il log Windows 0.13.45_08 ha pero' evidenziato due errori in `career_status.c` e il gioco NON si e' avviato. Nella 0.13.46 i due accessi sono stati corretti senza cambiare la condizione originale di fine campionato; compilazione e runtime Windows della nuova versione **NON PROVATI**.

Su Windows gli slot attivi sono `save/AUTO9697.CAR` e `save/AUTO2627.CAR` accanto all'EXE. `AUTOSAVE.CAR` e' legacy in sola lettura, `Documents\\SWOS` e' percorso storico upstream e non destinazione Expanded. Restano da completare coppe OG residenti, cronologia partite nel calendario post-ingaggio, save/reload degli stati OG, test piu' stagioni e ID multipli; crash FORWARD TIME e TOP GOAL SCORERS sono ancora da verificare. Test LWE isolati di 70 stagioni non provano la nuova integrazione.

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


## 0.13.01 — Fase 1: calendario civile configurabile (10/11/2026)

**Nuova regola versionamento:** serie `0.13.01`, `0.13.02`... un incremento 0.01 per commit autorizzato di codice, niente suffisso; il milestone avanza di 0.1 soltanto su decisione esplicita. **Nuovo workflow:** ZIP solo per asset; codice e documenti su GitHub con commit esplicito, nessun push automatico in assenza di permesso.

- [CODICE PREPARATO / DA TESTARE WINDOWS] Nuovi `sorgenti/include/expanded_world_time.h` e `sorgenti/src/expanded_world_time.c`. Data iniziale configurabile per giorno/mese/anno (non bloccata al 1996), calendario gregoriano, settimana di stagione, avanzamento monotono.
- [CODICE PREPARATO / DA TESTARE WINDOWS] `Expanded_FormatDate` passa alle nuove API; le schermate con calendario usano `World_FirstYear()` per 1996/97 oppure 2026/27. Il contatore visuale di settimana del disoccupato è quello della stagione (NON ISO); il vecchio contatore interno resta tale e quale.
- [TEST LOCALE] Due test C isolati: conversioni, bisestili, settimane, 109.938 giorni consecutivi; non una prova di avvio Windows.
- [DA FARE] Gli eventi datati reali, la simulazione del mondo durante la disoccupazione, le offerte a metà stagione, NEXT EVENT, il salvataggio del nuovo orologio, le date specifiche dei pacchetti dati oltre al loro anno e la presentazione WEEK in tutti i menu **non sono ancora implementati**.
- [NESSUNA VARIAZIONE] Fisica partita, game tick 50Hz, attuali partite, rose, grafica finale TITLE/NEW CAREER/LOADING, dimensioni database e formato save.

## 0.13.02 — Diagnostica F12 e primo registro risultati del mondo

**Ramo:** dev/0.10; **base del commit:** 3ee53ba9 (0.13.01). Numero codice aggiornato a **0.13.02**. Nessuna modifica alla fisica della partita o ai tre frontend grafici finali.

- [IMPLEMENTATO NEL CODICE, TEST WINDOWS PENDENTE] F12 attiva/disattiva sovraimpressione testuale con versione, mondo corrente, **data UI approssimativa**, settimane disoccupato, contatori partita, e stato effettivo delle funzioni mondiali (**NOT ACTIVE**). F9 cattura il **render finale**, incluso il testo debug, in `swos_debugNNN.png`. F11/PrintScreen continuano come screenshot classici.
- [IMPLEMENTATO NEL CODICE, TEST WINDOWS PENDENTE] Tasti 1/2/3 disponibili solo con F12 aperto: forza vittoria / forza sconfitta / esonero alla prossima azione partita. Le forzature 1/2 passano attraverso il percorso originale `getCachedOrCalcViewResult` nei risultati del club dell'utente, prima della contabilizzazione, non toccano la fisica. L'esonero riusa `Expanded_HandleSackingAfterMatch`. Disabilitati al boot e non inclusi nel save. Durante PLAY MATCH il risultato ufficiale potrebbe quindi differire dai gol visti sul campo: **usare soprattutto VIEW RESULT**.
- [MODULO INDIPENDENTE, NON INTEGRATO] `expanded_world_ledger.c/.h` ordina incontri per data e ID, conserva i risultati e calcola classifiche, con 2 o 3 punti/vittoria; può ricevere risultati del giocatore o da callback. Non carica ancora incontri reali, non aggiorna UI classifiche, non salva il mondo e non impedisce a `InitCareer` di ricominciare la stagione. Non equivale a simulazione mondiale attiva.
- [TEST GCC ISOLATI SUPERATI] Forzatura win/lose/sack; 35 settimane/140 risultati artificiali; no duplicati, no replay, classifiche e 2/3 punti. Windows MinGW, avvio, menu/partita/menu, F12/F9 screenshot, esonero e salvataggi restano **da provare**.
- [PROSSIME DIPENDENZE] Mappare squadre e competizioni a ID stabili e date, associare i resolver OpenSWOS, persistere gli eventi nella .CAR in modo retrocompatibile o versionato; poi preservare stato di lega/cassa/roster durante nuove assunzioni, evitando `InitCareer` distruttivo. Non inventare partite giocate durante la disoccupazione.


## 0.13.03 — World pilot con squadre reali, anteprima offerte (10/11/2026)

**Codice su `dev/0.10`:** il modulo `expanded_world_live.c/.h` costruisce in RAM incontri andata/ritorno da club realmente caricati dal database SWOS, in una coda cronologica deterministica. `expanded_world_live_adapter.c` acquisisce le cinque nazioni già ammesse dal generatore offerte (8/13/14/20/21, inclusi secondi file quando esistono) **prima** di aprire il menu disoccupato. Il modulo legge il valore/forza della rosa e produce risultati offscreen, classifica e date stabili, senza alterare la fisica delle partite. Le divisioni fino a 26 club sono supportate, anche dispari con BYE; altre divisioni NON vengono simulate a metà.

`Expanded_UnemployedWait` chiama l'avanzamento dei risultati a ogni settimana; `Expanded_OfferPreviewDraw` mostra la **classifica pilota** del campionato del club che offre il contratto, già prima dell'assunzione, evidenziando il club. Il preview mostra esplicitamente che **non è ancora sincronizzato con la carriera nativa**. Le fixture create usano un calendario round-robin **provvisorio**, non il calendario reale delle singole leghe/coppе.

**NON IMPLEMENTATO e DA NON CONFONDERE CON QUESTA PROVA:** caricamento/simulazione di tutte le nazioni, coppe nazionali/internazionali, date specifiche per paese, salvataggio della simulazione mondiale, prosecuzione della simulazione coerente con tutte le partite giocate dell'utente e soprattutto trasferimento della classifica del pilot nello stato di `InitCareer`. Dopo l'assunzione, le competizioni native di OpenSWOS possono ancora ripartire dall'inizio: questo è il prossimo punto strutturale, non un test superato.

**Test:** unit test isolati GCC/ASan+UBSan sulla schedulazione di 20 club (23/35 settimane, 230/350 risultati), avanzamento stagionale e girone dispari con riposo (5 club/20 incontri) superati su piattaforma locale; integrazione Windows, installazione dati 96/97 e 2026/27, prova dopo 35 settimane, preview e save **pendenti**. Debug F12 immutato. Nessun asset nuovo.


## 0.13.04 — Correzione compilazione Windows (10 novembre 2026)

- [CORRETTO NEL CODICE] `sorgenti/src/expanded_world_live_adapter.c` conteneva un carattere di escape letterale `\\n` nel test di puntatore della tabella delle competizioni alla riga 55; la sintassi C non era valida. Sostituito con un vero a capo, senza cambiare la condizione o il comportamento.
- [VERSIONE] `SWOS_EXPANDED_VERSION` avanzata da `0.13.03` a `0.13.04` secondo la regola un commit = +0.01.
- [EVIDENZA] Il log di compilazione Windows della 0.13.03 termina con `stray '\\' in program` in `expanded_world_live_adapter.c:55`, prima dell'avvio del gioco. Altri avvisi presenti nel log non sono la causa del blocco.
- [DA VERIFICARE] Nuova compilazione e avvio su Windows, prova UNEMPLOYED/VIEW LEAGUE; nessun test Windows di questa revisione dichiarato. Tutte le limitazioni funzionali della 0.13.03 rimangono: non è ancora disponibile l'ingresso nativo a stagione iniziata e il pilota non viene salvato nel .CAR. F12 invariato.


## 0.13.05 — 11 novembre 2026: 1° luglio e primi eventi precampionato (Windows da provare)

- [DESIGN APPROVATO] Sostituito **1° giugno** con **1° luglio** come inizio predefinito della carriera europea; giugno rimane operativo nel mondo; calendari per altri paesi configurabili. Approvati inoltre precampionato per **tutte** le squadre, scelta amichevoli da parte del manager, prezzi biglietti configurabili in **CLUB** con curva domanda/umore dei tifosi, e unificazione dei futuri menu CLASSIFICA/SQUAD tra offerte e carriera. Specifiche e vincoli nel `docs/GAME_DESIGN.md` §19.
- [CODICE PILOTA] `expanded_world_live.c` crea l'orologio mondiale provvisorio dal 1° luglio, conserva le fixture ufficiali da agosto e genera quattro date di **amichevoli simulate per squadre AI** il 7,14,21,28 luglio (BYE nelle leghe dispari). Tiene aggregati di partite, reti, morale, fatica, possibili eventi infortunio e saldo economico **soltanto nel PILOTA in RAM**. Non è l'economia/infortunio nativa.
- [CODICE] L'UNEMPLOYED mostra date e WEEK dal 1° luglio con `Expanded_FormatCivilDate` (distinto dall'ancora provvisorio formatter di date native basato su agosto). Il passaggio alla disoccupazione dopo esonero converte la data nativa a una settimana del calendario luglio arrotondata in avanti. In VIEW LEAGUE prima della prima partita ufficiale compare il numero di amichevoli pilota.
- [TEST AGGIORNATI, ESECUZIONE WINDOWS PENDENTE] `sorgenti/tests/test_expanded_world_live.c`: pre-campionato luglio con 0 risultati di lega alla settimana 4, 40 amichevoli per girone di 20 club, giornata da agosto; settimana 23 e 35 senza duplicati; avvio seconda stagione e BYE per leghe dispari. Eseguire `bash sorgenti/tests/run_test_expanded_world_live.sh` e normale compilazione Windows.
- [DA FARE] Il pilot copre soltanto i paesi precedentemente caricati (8,13,14,20,21) e i gironi supportati; le sue partite e conseguenze non vengono salvate né travasate in `InitCareer`. Dopo la firma il calendario nativo può ancora ripartire dalla prima giornata; la data mostrata da BUSINESS usa ancora il vecchio formatter agosto. Prezzo biglietti e CLUB nuovi NON ancora implementati. F12, motore partita 50Hz, salvataggi, asset TITLE/NEW CAREER/LOADING e classiche modalità invariati.


## Osservazioni di test 0.13.05 e selettore mondo approvato (11 novembre 2026)

- [TEST WINDOWS UTENTE] In `UNEMPLOYED → VIEW LEAGUE`, uno screenshot mostra `BARNET — ENGLAND`, DIVISION 4 1996/97, classifica a zero partite giocate e `PRESEASON: 4 FRIENDLIES (PILOT)`; un altro mostra `HARTLEPOOL UTD — ENGLAND` con tre gare di campionato conteggiate. Confermata visivamente la differenza fra luglio di preparazione e giornate ufficiali del pilota.
- [NON ANCORA VERIFICATO] Queste prove non dimostrano effetti persistenti su forma, infortuni o bilancio; restano separati dai dati nativi. L'assunzione a stagione in corso e salvataggi mondo rimangono aperti.
- [DESIGN APPROVATO, NON IMPLEMENTATO] In OPTIONS aggiungere **FOOTBALL WORLD: 1996/97 | 2026/27**, prima di iniziare una nuova carriera, controllando l'installazione e la compatibilità dei dati. Mantenere il world tag del salvataggio e impedire modifiche incompatibili alla carriera attiva. Dettagli nel `docs/GAME_DESIGN.md` §20. Fino all'implementazione rimane il selettore di avvio tramite `SWOS.ini`, `SWOS_WORLD` o `--world`.


## 0.13.06 — Primo ingresso a calendario nativo già avviato (11 novembre 2026)

- [BASE] `dev/0.10`, commit precedente `40045ed8`, Build `0.13.05`. Versione sorgenti aggiornata a `0.13.06`.
- [CODICE PROTOTIPO, WINDOWS PENDENTE] `expanded_midseason_join.c/.h`: durante la prima assunzione, converte le settimane di disoccupazione dal 1° luglio al `sortKey` della settimana nativa (base 1° agosto), avvia un *catch-up* che usa `NEXT MATCH`/`VIEW RESULT` e le stesse callback di simulazione del test `career_autopilot.c`. Si interrompe quando la prossima voce è successiva/uguale alla data di assunzione; abortisce con ripristino del controllo quando raggiunge i guardrail. Log `[midseason]` in `SWOS.log`.
- [CODICE] Sospesi solo durante il catch-up i contatori personali del nuovo manager e le sanzioni per risultati antecedenti alla firma; il risultato della gara continua a entrare nel record e nella classifica **nativi**. Inibiti popup CLUB/BUSINESS e richieste di mercato nell'avanzamento automatico. F12 e gameplay partita invariati. La chiamata iniziale a `InitCareer()` resta valida per i dati del club e le competizioni; l'avanzamento avviene successivamente, non è un trapianto di buffer.
- [LIMITE FONDAMENTALE] La classifica pilota **prima** della firma è generata da un calendario differente, perciò non si può ancora garantire corrispondenza esatta con quella nativa **dopo** la firma. L'unificazione della fonte mondiale e la persistenza del pilota nel `.CAR` NON sono implementate. Anche le riassunzioni oltre il primo anno continuano con le limitazioni di `InitCareer`. Non confondere l'esperimento con il completamento delle Fasi 1–2.
- [SICUREZZA] Limite 6000 passaggi menu, controlli su indice, stride e fine stagione, abilitazione soltanto con data luglio entro le prime 52 settimane. Funzioni di catch-up OFF per default e salvataggi originali non alterati nella struttura. Non testare con una carriera importante.
- [TEST] Test C unitari per la conversione della data disponibili in `tests/test_expanded_midseason_join.c` e runner; **compilazione completa Windows e test runtime ancora pendenti**, così come 1996/97 e 2026/27, coppe, firma dopo 20–35 settimane e salvataggio/ricarica.
- [DESIGN REGISTRATO] Inclusi §20 (selettore mondo in OPTIONS) e §21 (sponsor preesistenti con scadenze e rinnovo, anche zero per club in crisi) e §22 (forma individuale/allenamento collettivo **solo proposta**). Il selettore mondo e gli sponsor non sono stati implementati in codice in questo commit.


### 0.13.10 — Recupero storico senza input (11 novembre 2026)
- [CODICE] La sequenza di recupero dopo l'ingaggio pilota anche le schermate delle coppe native (DIYCupInit), selezionando callback di risultato/avanzamento disponibili.
- [CODICE] Se il menu mostra soltanto PLAY MATCH per una partita che precede l'ingaggio, i callback originali di lega/coppa usano la modalità simulazione CPU; SetupPlayers non viene aperto durante il recupero. Dopo il recupero il gameplay normale è invariato.
- [DEBUG] F12: conteggio azioni automatiche e callback di partita reindirizzati alla simulazione, visibile durante il recupero; stderr: diagnostica menu non riconosciuti, abort e fase raggiunta.
- [DA TESTARE WINDOWS] Recupero con club di alta e bassa classifica, campionati e coppe, integrità delle classifiche prima e dopo l'ingaggio, nessuna apertura partita durante recupero. Build Windows non eseguita in questo ambiente.
- [LIMITAZIONE] Il pilota pre-assunzione usa ancora un calendario differente da quello nativo; la completa unificazione del Living World Engine, inclusi save/reload, non è raggiunta da questa build.


### 0.13.11 — Recupero della data nativa, UI congelata durante catch-up (11 novembre 2026)
- [CODICE] L'orologio **visualizzato** legge la fase del prossimo incontro nativo (keycache) e non il contatore dell'ultima fase finanziaria. La conversione della fase e' mese ×8 + ottavi del mese, con rollover +0x60: sortKey 56 = 1 agosto 1996 per il mondo 1996/97, non 28 agosto 1997.
- [CODICE] Il recupero seleziona la prima fase NON precedente alla data della firma (arrotondamento verso l'alto), in modo da simulare anche le partite gia' trascorse nella stessa frazione di mese. L'ordine dei callback per le leghe e' allineato al golden harness: NEXT MATCH prima di VIEW RESULT, per non rileggere all'infinito il risultato precedente; per le coppe avanzamento della fase prima dei risultati.
- [CODICE UI] Durante il recupero, presentazione oscurata da testo di avanzamento temporaneo, senza nuovi PNG; input utente mascherato nel driver per impedire che la classifica venga aperta mentre cambia. F12 rimane operativo e ora espone anche stato REACHED/ABORTED e ultima chiave per diagnosticare un arresto precoce.
- [VERIFICA] 94 casi di controllo indipendente della corrispondenza tra data di ingaggio e fase di arresto per mondi 1996/2026; aggiunte asserzioni pure-C sulle conversioni. Compilazione dei test C e test Windows dell'intera carriera **non eseguiti qui**.
- [LIMITAZIONE] Il calendario di preview `ExpandedWorldLive` e quello nativo rimangono due sorgenti diverse; questo commit corregge il recupero nativo e la data mostrata, ma non certifica un Living World Engine persistente unificato.


### 0.13.12 — Recupero storico: uscita automatica dalle schermate lega concluse (11 novembre 2026)
- [ERRORE RIPRODOTTO DA SCREENSHOT] F12 nella 0.13.11 mostrava `JOIN STATUS ABORTED FINAL -8 TARGET 87` con pochissime giornate simulate. `-8` corrisponde al ramo `DIYLeagueTournamentInit` senza callback tra NEXT MATCH, VIEW RESULT, RESULT e MATCH: quando la giornata termina, queste azioni diventano invisibili e il menu mostra soltanto EXIT.
- [CODICE] Nel catch-up storico, dopo aver cercato gli incontri da simulare, selezionare il vero callback `DIYLeagueTournamentExit` quando e' l'unica azione rimasta. Ritorna alla carriera per continuare con la prossima voce del keycache. Non cambia fisica, calendario, punti, standings, budget, menu standard o salvataggi.
- [DEBUG F12] Aggiunto `LEAGUE MENUS COMPLETED` per contare le uscite automatiche dalle giornate concluse, incluso dopo il recupero; log stderr include indice e fase al momento dell'uscita. Codice abort -8 resta per menu senza azioni neppure EXIT (possibile corruzione/altro stato).
- [TEST] Verificato staticamente che il menu originale `_diyLeagueTournamentMenu` espone EXIT con callback `DIYLeagueTournamentExit`, e che `career_autopilot.c` ricorre all'uscita dal menu quando non ci sono piu' azioni. Compilazione Windows/test carriera da effettuare, nessun risultato simulato inventato.
- [ROLLBACK] `0523315224f38e4f26fc6653eebee3c11493b59a` = 0.13.11. La duplicazione dei calendari pilota/native resta una questione aperta.


### 0.13.13 — riuso parziale dei risultati pilota nel recupero OpenSWOS (11 novembre 2026)
- [CODICE] Il registro risultati pilot `ExpandedWorldLive` espone una ricerca non distruttiva delle gare gia' giocate; ogni record puo' essere riusato una sola volta nell'avanzamento nativo, soltanto se coincidono i due club nell'ordine casa/trasferta e la finestra temporale (massimo una settimana di scostamento dalla fase nativa, non oltre la fine della stessa fase).
- [CODICE] Durante il recupero precedente alla firma il normale handler dei risultati di campionato `ProcessDIYLeagueMatchResultAndCareerUpdates` tenta prima di usare il punteggio pilota compatibile; se manca usa `CalculateViewResult` originale. Restano invariati il calcolo dei punti, i dati dei giocatori, le gare normali dopo la firma, il motore fisico, le coppe e le tabelle dei campionati; NON e' stato costruito un singolo calendario.
- [F12] Aggiunti `NATIVE REUSED` e `FALLBACK` e la dicitura `PILOT/NATIVE REPLAY PARTIAL`. Entrambi i sistemi restano distinguibili; il debug NON rappresenta una completa unificazione.
- [TEST] Aggiunte asserzioni su due partite pilota: phase window, inversione casa/trasferta, divieto di riusare il risultato, risultati pilota invariati. I file dei test C non sono stati compilati/eseguiti in questa sessione; test Windows necessari.
- [UX] Su richiesta, nessun cambiamento al flusso ACCEPT / WELCOME; il recupero rimane al punto attuale e puo' richiedere tempo/flicker finche' non esiste un solo stato mondiale.
- [REGOLE] Commit autorizzato solo per cambi significativi; rollback alla 0.13.12 `a0e01ee5f40cbffb1676c2b57dfb86a0a1d5d766` mediante GitKraken, su scelta dell'utente.


### 0.13.14 — recupero nativo senza rendering continuo e riuso dei risultati per stagione (11 novembre 2026)
- [TEST UTENTE 0.13.13] `JOIN STATUS REACHED FINAL 105 TARGET 105`, `LEAGUE MENUS COMPLETED 31`, `NATIVE REUSED 12 FALLBACK 360`; la data del calendario e' corretta, ma l'anteprima disoccupato e la classifica nativa restano diverse. Nessuna nuova screenshot diagnostica necessaria finche' non si prova il nuovo recupero.
- [CODICE] `platform_sdl.c`: durante `Expanded_MidseasonJoin_Active()`, presenta una sola schermata statica, poi aggiorna esclusivamente SDL events/audio senza upload framebuffer/present e senza pacing a 50 Hz; conservati callback originali, controlli di uscita, motore partita e tutti i tempi di gioco al termine. La timer-thread di fallback rispetta lo stesso blocco. In WASM rimane il normale yield al browser.
- [DEBUG F12] `CATCHUP HEADLESS [N] FRAMES [MS] MS` mostra i present evitati e il tempo reale del recupero; i contatori NATIVE REUSED/FALLBACK rimangono validi.
- [CODICE] `ExpandedWorldLive_TakeNativeLeagueResult` abbina una gara del pilota gia' giocata alla stessa squadra di casa e di trasferta **nella stessa stagione**, anche quando il round e' in una data differente del calendario originale. Ogni risultato e' consumabile una sola volta; fallback OpenSWOS per le altre gare. Nessun risultato di coppa o successivo alla firma puo' essere importato.
- [LIMITAZIONE] I due calendari non sono ancora stati unificati; il riuso dei punteggi puo' crescere, ma non significa che le classifiche prima e dopo la firma siano matematicamente identiche. Il prossimo intervento strutturale dovra' far avanzare la stessa agenda nativa durante l'attesa disoccupato anziche' recuperarla dopo la firma.
- [VERIFICA] Test C su abbinamento/anno/riuso aggiornati ma non eseguiti, build Windows/test manuali pendenti; nessuna modifica a schermate di carriera approvate o asset. Rollback da GitKraken alla 0.13.13 `21750831d8c1c22728495369fd854dbb10511a49`.


### 0.13.15 — calendario OG SWOS condiviso, tentativo di unificazione incisivo (11 novembre 2026)
- [DECISIONE UTENTE] «Spavalderia»: consentire nuovi moduli e modifiche architetturali sostanziali già autorizzate, mantenendo commit precedente per rollback. Il sistema originale di calendari OpenSWOS deve RESTARE la base; non sostituirlo con un nuovo motore parallelo.
- [CODICE] Il Living World usa `_fixtureSchedulePtrTable` originali OpenSWOS per coppie, BYE e ritorni, con fase mensile da `Competition.beginMonth/endMonth`. In leghe esattamente riconosciute, l'inizializzazione nativa adopera lo stesso ordine club del mondo anziché una permutazione casuale distinta. Resta la routine OG originale per leghe non riconosciute.
- [CODICE] I risultati prima della firma continuano ad essere estratti dal registro del mondo e applicati alle classifiche originali senza ricalcolarli. Dopo la firma i risultati nativi del manager sono riscritti nel medesimo registro mondiale, per esatta coppia e fase, prima di avanzare le altre gare CPU.
- [F12] `OG LEAGUE CALENDARS X/Y ORDERS Z` misura leghe con tabella OG e inizializzazioni native allineate. I contatori `NATIVE REUSED/FALLBACK` restano per mismatch.
- [ATTENZIONE] Il motore nativo conserva ancora il suo stato delle classifiche e necessita di applicare i risultati nel recupero headless dopo l'ingaggio: non è eliminato ogni doppio stato. Coppe, leghe speciali e save globale non ancora unificati; usare F12 e test prima di dichiarare COMPLETO.
- [TEST] Controllo statico delle tabelle originali 4/20/24 club: tutte le coppie uniche, nessun indice invalido. Test C puri aggiornati ma NON eseguiti; build/test Windows della 0.13.15 da effettuare. Rollback manuale tramite GitKraken: `e2cee6d2d965b965034b15246b0d178143ea5d61` (0.13.14).


### 0.13.16 — OG direct-league import e debug manual match
- [CODICE] Importazione del campionato standard già giocato nel ledger mondiale direttamente nel buffer OG OpenSWOS, con cursor di fixture nativo, punteggi e KeyCache verificata prima del commit in memoria. Nessuna risimulazione delle gare importate.
- [FALLBACK] Se leghe speciali o calendari non corrispondono: riprende recupero headless 0.13.15_01, senza mutare la lega. Se ci sono vecchie coppe, solo i turni di coppa restano elaborati col driver OG; le vecchie giornate di lega importate vengono avanzate tramite cursore OG.
- [JOIN] Nessuna vecchia coppa + import OK: la carriera salta direttamente al primo incontro futuro, senza driver di catch-up e senza rendering. F12 \`JOIN STATUS DIRECT\`, \`DIRECT OG IMPORT\`.
- [AUDIT] Ultima partita OG nel F12: punteggio letto e applicato, differenza punti dei due club, punteggio raw finale per partite manuali e \`SCORE MISMATCH\` quando diverge. La routine OG assegna +1/+1 ai pareggi: l'impressione dell'utente su un +3 va verificata con questo audit.
- [NON COMPLETO] World save, coppe persistenti, riassunzioni, special leagues ancora da unificare. Non eliminare il vecchio recupero fino ai test Windows integrati.
- [TEST] Aggiunte asserzioni read-only ledger a test C; NON eseguite né compilata build Windows. Rollback \`600141d\` (0.13.15_01).


### 0.13.17 — snapshot persistente del Living World nel salvataggio .CAR (11 novembre 2026)

**Priorità decisa:** completare la versione principale del Living World Engine prima di tornare ai menu. Gli strumenti di bilanciamento statistico e i test di scenario estesi saranno sviluppati alla fine del ciclo del motore. Segnalazione chat: "Error in message stream", causa sconosciuta, non imputata al gioco.

**Codice:** l'unico ledger mondiale ora può essere serializzato e ricaricato nel normale save .CAR JSON. La chiave living_world contiene club e loro rating/identità, leghe e associazioni, fixture, risultati, calendario, settimane, preseason e contatori dell'integrazione nativa. Il parser costruisce uno snapshot temporaneo e verifica tipi, limiti e coerenza delle relazioni club/leghe prima di sostituire lo stato attivo. I fixture sono letti in una scansione lineare, evitando iterazioni quadratiche sull'array esteso.

**Compatibilità:** nuovo schema JSON 4, lettura dei salvataggi schema 3 mantenuta. I vecchi .CAR non contengono il ledger mondiale e quindi vengono caricati con il pilot vuoto: risultati storici globali non possono essere ricostruiti dal vecchio file. Gli altri formati JSON rimangono compatibili con schema 3. La build Amiga archivia living_world come null, poiché il pilot è SDL-only. Nessun sidecar, DLL o nuovo database parallelo.

**Limitazioni:** la persistenza del pilot non completa ancora l'engine. Rimangono il limite delle due stagioni pre-generate, calendario ciclico multistagionale, coppe in ledger, manager/chairman persistenti, simulazione del mercato/ritiri/nuovi giovani, calendario globale anche dopo ingaggio e riassunzioni, e connessione dei dati finanziari e calcistici canonici. Il rating club non equivale a un database persistente di calciatori.

**Verifica:** esaminati sorgenti e dipendenze su GitHub; non eseguiti compilazione MSYS2/Windows o test runtime in questa sessione. Dopo il pull, verificare che AVVIA_SWOS.bat compili; salvare e caricare una carriera schema 4, comparare F12 prima/dopo (club/leghe/fixtures/playback), poi verificare classifica e partita manuale. Provare anche a caricare un vecchio .CAR schema 3. Non dichiarare build Windows testata finché l'utente non riporta esito.


### 0.13.18 — correzione durata carriera nel world snapshot
- Corretto il controllo di caricamento delle settimane: il pilot genera fixture solo per due stagioni ma l'orologio della carriera puo' avanzare oltre. Un .CAR valido non viene piu' respinto soltanto perche' la data e' successiva al secondo campionato generato.
- Il limite del mondo resta l'intervallo di date supportato dal motore (fino al 2200). Questo intervento NON genera stagioni nuove dopo il secondo anno: la progressione perpetua e' ancora da implementare.
- Non eseguita compilazione Windows; test completi rinviati al ciclo conclusivo.


### 0.13.19 — snapshot dei risultati compresso e strategia di dimensionamento mondiale

**Segnalazione comunicazione:** "Connection interrupted. Waiting for the complete answer" (11/11/2026). Interruzione del flusso di risposta; nessuna correlazione dimostrata con errori di SWOS. Non sostituisce log/test del gioco.

**[CODICE]** Nel medesimo .CAR, il blocco living_world usa la versione interna 2. L'elenco fixture/risultati precedentemente scritto come array JSON viene serializzato in una sequenza JSON compatta, compresso con zlib/miniz già incluso nel progetto e salvato come stringa Base64 (fixtures_zlib + fixture_json_bytes). L'identità dei club, delle leghe e gli aggregati conservano campi JSON leggibili; tutto resta in un unico file .CAR, senza secondo database e senza nuovi asset o DLL. In caso di errore/OOM della compressione, il writer abortisce il salvataggio invece di generare una carriera apparentemente valida ma senza risultati. La lettura v2 decomprime in memoria con dimensione massima verificata e utilizza lo STESSO parser/controlli sui fixture della v1.

**[COMPATIBILITÀ]** Schema .CAR JSON 5 in scrittura; lettura schema 3 e 4 mantenuta. Il parser mondiale riconosce il vecchio living_world v1 con array normali, e v2 con blob compresso. I vecchi .CAR schema 3 restano senza snapshot mondiale. Gli altri tipi di salvataggio JSON possono leggere anche i precedenti schemi 3/4. Il codice non modifica OpenSWOS in partita né gli algoritmi di calcolo classifica.

**[DIMENSIONI]** Stima illustrativa eseguita su righe di partite sintetiche, NON su una carriera reale: 8.384 fixture circa 276 KB in JSON, 96 KB dopo compressione zlib e Base64; 40.000 fixture circa 1,32 MB contro 0,46 MB. Risultati reali possono variare; club, giocatori e resto del .CAR non sono inclusi. Il limite attuale resta 40.000 fixture, 1.500 club, 128 leghe e 2 stagioni generate: la compressione non li estende.

**[ARCHITETTURA FUTURA — PROPOSTA, NON APPROVATA/IMPLEMENTATA]** Per 8 leghe per paese, coppe e decine di anni, i risultati di ogni singola gara storica e i calciatori di tutto il pianeta non possono crescere senza controllo. Conservare dati statici in un world-pack versionato, con identità stabili; memorizzare nei salvataggi solo delta persistenti/eventi essenziali, archivi stagionali compatti e stato completo delle aree rilevanti. Generazione deterministica per il mondo distante quando non cambia fatti già registrati. Rotazione dei calendari delle stagioni passate e limiti di memoria espliciti, senza perdere titoli, risultati aggregati, storico manager e transazioni. Ogni migrazione/archivio va progettato e approvato separatamente: non introdotto in questa build.

**[BUILD E TEST]** GitHub aggiornabile con commit autorizzato, ma non è stato possibile eseguire la toolchain MSYS2/MinGW64, né compilare SWOS_Expanded.exe su questo ambiente: il repository non è montato localmente e la rete della shell non raggiunge GitHub. Verifiche statiche sui nomi dei simboli/collegamenti e simulazione della compressione con dati sintetici eseguite; test effettivi di serializzazione C, compilatione e caricamento su Windows ancora pendenti. Test statistici estesi rimandati alla fine del ciclo dell'engine su richiesta dell'utente.


### 0.13.20 — autosave e struttura portabile Windows
**CODICE:** `save/AUTOSAVE.CAR` diventa unico checkpoint di carriera
nella cartella dell'EXE, tramite writer .CAR/schema 5 e file temporaneo
sostituito atomicamente dopo flush riuscito. Il Title verifica il file
su disco e CONTINUE CAREER lo carica dopo un vero riavvio. Checkpoint
alla prima firma, dopo incontri/simulazioni e uscite sicure da menu,
a fine catch-up e al cambio stagione; finale carriera rimuove lo slot.
`SWOS.ini`/`SWOS.log` su Windows nella cartella del gioco; nessun
nuovo file in Documents. Pulizia legacy come script manuale solo dopo
test di lettura; EXE non cancella file condivisi con OpenSWOS originale.

**LIMITI:** niente save in partita 50 Hz, ripresa esatta da crash/
anti-save-scum di match NON implementata. Comando dedicato ABANDON
CAREER con conferma non ancora presente, EXIT GAME conserva save.
Lo schema mondiale delta-only rimane progettazione futura: la 0.13.20
usa ancora ledger pilot compresso, senza duplicare database nuovi.

**TEST:** controlli statici codice e riferimenti di GitHub. Compilazione
MSYS2/MinGW64, test EXE Windows e round-trip save/load su PC utente
non eseguiti dall'assistente. Non dichiarare build Windows collaudata.


### 0.13.21 — fix caricamento autosave: calendario mondiale precedente a luglio
- [BUG REALE] Test Windows 0.13.20: `AUTOSAVE.CAR` esiste, schema 5 e Base64/zlib validi, ma `CONTINUE CAREER` mostra il messaggio generico "FILE SAVED BY DIFFERENT S.W.O.S. VERSION".
- [DIAGNOSI CON SAVE REALE] Il salvataggio fornito contiene world version 2, 216 club, 11 leghe, 8.384 fixture e 1.520 risultati. Un campionato con paese 21, divisione 0, calendario OG maggio-settembre, ha 45 fixture datate maggio/giugno 1996, precedenti al 1° luglio 1996 del manager. In `ExpandedWorldLive_ReadJson` la verifica `fixture.day < initial` le respingeva senza ragione, malgrado fossero già generate dall'engine stesso.
- [FIX] Il validatore ora accetta date dal **1° gennaio del primo anno della carriera**; mantiene tutti gli altri controlli di club/competizioni, fixture, ordine, zlib, fase e compatibilità schema 3/4/5. Nessuna modifica al formato del .CAR; il file di prova non richiede conversione o cancellazione.
- [REGRESSION] Aggiunto test C per campionato OG maggio-settembre, con serializzazione compressa e lettura dello snapshot, aggiornando lo script di compilazione isolato a collegare i moduli mjson/miniz. Controllo esterno Python del .CAR reale: 45 righe respinte dalla vecchia verifica, zero violazioni riscontrate degli altri vincoli numerici/relazionali.
- [TEST] Verifica del contenuto del file reale effettuata, test C e build Windows/MSYS2 ancora DA ESEGUIRE. Con nuovo EXE, provare direttamente CONTINUE CAREER sul medesimo `AUTOSAVE.CAR` senza eliminarlo. Se ancora fallisce consultare `SWOS.log`; non dichiarare funzionamento in-game prima della verifica Windows.


### 0.13.22 — stagioni mobili, archiviazione compatta e test da 70 stagioni

**[CODICE]** Il Living World usa una sola finestra residente di **due stagioni** generate con gli indici di accoppiamento originali OpenSWOS; ogni 1° gennaio due anni dopo la più vecchia archivia quella stagione completa e genera la nuova, prima dell'avvio dei campionati che iniziano a maggio. Lo storico contiene per lega/anno campione, punti del campione, numero incontri e digest dei risultati; le gare storiche individuali non rimangono tutte consultabili: per uno storico match-by-match andrà sviluppato un archivio a delta apposito. Le coppe e la gestione completa degli individui non sono ancora parte del ledger.
- Gli ID della competizione riutilizzano solo uno slot **già rimosso** dall'orizzonte resident (2 stagioni) per rispettare il vecchio formato `uint16_t` e non collidere durante 70 stagioni; `s_first_year` originale e il seed procedurale rimangono stabili. Gli indici di consumo del risultato OG seguono la partita anche quando le fixture vengono riordinate.
- Il clock mondiale conserva anni/settimane dall'inizio effettivo e i risultati totali includono storico + 2 stagioni residenti. In `ContinueCareerNextSeason`, dopo il cambio stagione OG, il mondo viene sincronizzato al luglio del nuovo anno: fisica e progressione partita originale non vengono alterate. Il sync settimanale per ogni evento del manager rimane un lavoro successivo di integrazione.
- Il blocco `living_world` dentro `AUTOSAVE.CAR` passa alla **versione interna 3** (`window_first_year` e `history`); lo schema .CAR esterno resta **5**, e sono ancora leggibili gli snapshot mondiali v1/v2 della 0.13.21. Il log F12 distingue window, partite in memoria, risultati archiviati e anni/leghe storiche.
- `TEST_LIVING_WORLD.bat` compila ed esegue la simulazione C senza rendering: **70 stagioni completate**, 216 club/11 leghe sintetici di dimensione simile al pilot reale, campionati maggio-settembre e agosto-maggio, 2/3 punti, 15 round-trip serializzazione/deserializzazione ogni 5 anni circa, validazione campioni/partite duplicate/archivio/finestra di memoria con report portabile `LIVING_WORLD_70_TEST.log`. Usa il calendario OG e il codice live reale, ma **non è** una carriera manageriale originale end-to-end, né carica in autonomia tutti i file reali. `save/AUTOSAVE.CAR` dell'utente non viene toccato.
- [TEST EFFETTIVI] Revisione statica delle sorgenti e dipendenze; build GCC del test isolato, compilazione Windows/MSYS2 e runtime in-game **da eseguire** sul PC utente. Il codice predisposto non equivale ad affermare il risultato PASS. È richiesto un pull+compile e un controllo dell'autosave v2 precedente, poi esecuzione del BAT di stress e upload del log. Niente conclusione su 70 anni finché il report non conferma.
- [FUORI SCOPE] Eventi globali davvero giornalieri (infrasettimanali/cup), promozioni/retrocessioni, mercato/giovani, chairman/manager persistenti e bilanciamento numerico completo. Dopo consolidamento engine discuteremo con l'utente i **VALORI DELLA SIMULAZIONE**; placeholder sportivi correnti non modificati.


### 0.13.23 — GCC 16.2: test 70 stagioni non compilato (11 novembre 2026)

**[ESITO UTENTE]** Il primo `TEST_LIVING_WORLD.bat` della 0.13.22 si è fermato nella fase **[build]** e NON ha simulato alcuna stagione. `LIVING_WORLD_70_TEST.log` mostra 6 errori `-Werror`: due `unused-but-set-variable` in `src/mjson.c`, due `type-limits` in `third_party/miniz/miniz.c` e due `sign-compare` in `expanded_world_live.c`. Nessuna asserzione dello stress test è stata raggiunta.
**[CODICE]** Nei due script dei test C di Living World le unità vendor `mjson.c` e `miniz_impl.c` vengono compilate separatamente con due **sole** soppressioni di warning circoscritte ai file (rispettivamente `-Wno-unused-but-set-variable` e `-Wno-type-limits`). Su tutto il codice Expanded e test C resta `-Wall -Wextra -Werror`. Corretto direttamente `expanded_world_live.c`: confronti `long` vs macro `unsigned` con cast a `long` per limiti contenuti in 32 bit; validatore di vecchi risultati OG conta **archiviati + residenti**, non solo le due stagioni residenti, per non rifiutare futuri .CAR pluridecennali. Nessuna formula sportiva o fisica partita modificata.
**[TEST]** Revisione statica dei warning e delle istruzioni dei runner; compilazione MSYS2/GCC e test 70 anni ancora da eseguire sul PC dell'utente. Non attribuire PASS alle 70 stagioni fino al log `STRESS_RESULT=PASS`. Se `BUILD_SWOS.sh` fallisce, inviare `COMPILAZIONE_0140.log`; se lo stress harness si ferma, inviare `LIVING_WORLD_70_TEST.log`.


### 0.13.24 — test 70 stagioni: fix linker e preparazione prima stagione maggio (11/11/2026)

**[TEST UTENTE]** Il test .23 supera il compilatore `-Werror`, ma l'operazione di collegamento fallisce con `undefined reference to swos_io_realloc` proveniente da `mjson.o`; **nessuna simulazione è ancora stata eseguita**. Il log è `LIVING_WORLD_70_TEST(1).log`, 4 righe. Questa dipendenza proviene dal default `MJSON_REALLOC` in `include/mjson.h`, che fa riferimento al wrapper di allocazione del gioco, intenzionalmente non linkato nel test autonomo.
**[TEST RUNNER]** Nei soli due script standalone del Living World compilare `mjson.c` con `-DMJSON_REALLOC=realloc -include stdlib.h`, usando l'allocatore della libc locale in quel solo oggetto (non nel gioco). Nessun nuovo linker input, nessuna modifica a `mjson.c`/miniz/produzione. Rimangono le esclusioni strettamente mirate dei warning vendor, mentre il codice Expanded viene compilato con `-Werror`.
**[BUG PREVENUTO]** La finestra mobile avrebbe potuto bloccarsi quando archivia il primo anno perché i campionati OG maggio-settembre iniziano prima del 1° luglio della carriera europea: i risultati già storici non erano mai processati al Seal. Il World ora completa deterministicamente soltanto le fixture con `f.day < s_initial_day` dopo la generazione iniziale, senza riavvolgere l'orologio. Esegue lo stesso recupero sui vecchi .CAR v1/v2/v3 caricati che hanno incontri iniziali non giocati; non modifica le partite manuali/50 Hz, né i coefficienti sportivi. Nuove assert nei test C verificano che i risultati precedenti alla carriera esistano al giorno 1.
**[VERIFICA]** Runner/link e condizioni di backfill analizzati staticamente. **Non** eseguiti compilazione integrale MinGW64/Windows, test 70 stagioni, né test regressivi C in questa sessione: l'utente dovrà ritestare `TEST_LIVING_WORLD.bat` e inviare `LIVING_WORLD_70_TEST.log`, in seguito prova del vero CONTINUE CAREER. Un commit non attesta che siano trascorse 70 stagioni.


### 0.13.25 — start del mondo e nuova carriera debug (12/11/2026)

**[CODICE PREPARATO]** L'unico `ExpandedWorldLive` nasce alla pressione di START di NEW CAREER, a monte sia di SELECT TEAM sia dell'attesa disoccupata. `Expanded_WaitForFirstOffer` non effettua più `ExpandedWorldLive_LoadOfferCountries`, evitando di ricreare il calendario quando arriva o si accetta un'offerta. Alla cancellazione della selezione club o dell'attesa iniziale, chiudere il solo mondo provvisorio in memoria; `save/AUTOSAVE.CAR` non viene cancellato.
**[DEBUG]** F12+N sul Title (con overlay F12 visibile) accoda una richiesta di aprire NEW CAREER nel **thread menu**, non dentro SDL KEYDOWN. Svuota lo stato di carriera attivo **solo in RAM** usando `InitCareerVariables(GAME_TYPE_NO_GAME)`, mantenendo l'autosave esistente. Al primo nuovo salvataggio riuscito sarà il writer atomico esistente a sostituire l'autosave. Annullare il setup non tocca il file. TITLE PNG e gerarchia dei pulsanti invariati.
**[DIAGNOSI PRESTAZIONI]** Confronto sorgenti 0.13.21→0.13.24: `platform_sdl.c` aggiunge solo testo/debug F12; il loop a 50 Hz e le funzioni del match non sono stati modificati in quei commit. Un rallentamento durante i 90 minuti NON è escluso senza misurazioni Windows in condizioni equivalenti (F12 OFF, renderer/vsync, risoluzione). Autosave si trova dopo la risoluzione della partita, non a ogni frame.
**[UNIFICAZIONE ANCORA APERTA]** Il ledger Expanded è un solo modello di risultati, ma l'orologio/calendario OpenSWOS mantiene ancora callback e fixture di competizioni originali, coppe e recuperi. `ExpandedWorldLive_TakeNativeLeagueResult` contiene un adattamento per le date OG non identiche: non cancellare quel bridge o il percorso di fallback senza sostituirli con una vera fonte cronologica comune e copertura dei tornei. La prima iniziativa sposta solo il punto di inizializzazione, non completa da sola lo scheduler.
**[DATI]** L'utente ha rinviato l'approvazione di tutti i dati (inclusa forma individuale, allenatori e chairman) al **penultimo passo**, prima del collaudo finale e della prevista 0.14.00. Nessuna statistica sportiva modificata in questa fase.
**[TEST]** Revisione statica e confronto funzioni; compilazione Windows/MSYS2, nuova carriera da Title F12+N, rollback su annullamento, prova manuale di classifica e save/load sono DA ESEGUIRE. Il PASS del test 70 stagioni riguarda la 0.13.24 isolata, non la carriera manageriale completa.

### 0.13.26 — cursor civile unico per gli eventi del mondo (12/11/2026)
- [CODICE] Le chiamate del Living World per settimane e cambio stagione condividono ora l'avanzamento `ExpandedWorldLive_AdvanceToDate` basato sulla data civile esatta; il luglio annuale non viene arrotondato alla settimana successiva. Resta unico il ledger di fixture/risultati di lega. Non cambiano la fisica di partita, i callback OG delle coppe, il Title o il formato .CAR v3.
- [DEBUG F12] WORLD CLOCK con data civile e WEEK; NEXT LEAGUE EVENT con data e numero di partite; PENDING e OVERDUE. OVERDUE=0 segnala che non ci sono fixture rimaste insolute prima dell'orologio mondiale. Non implica da solo che tutte le competizioni siano nel ledger.
- [TEST DA ESEGUIRE] Runner 70 stagioni aggiornato con avanzamento infrasettimanale di tre giorni, .CAR round-trip a data non settimanale, monotonicita', audit cronologico ai checkpoint; log `MIDWEEK_EVENT_SAVE_RELOAD=PASS`, `WORLD_EVENT_CURSOR=PASS`, `STRESS_RESULT=PASS`. Le asserzioni sono predisposte nel codice, senza prova di compilazione Windows qui.
- [LIMITE TECNICO] Il calendario cup/knockout originale e il bridge di inserimento native rimangono attivi, quindi non e' ancora corretto dichiarare completata l'unificazione di TUTTI gli eventi/competizioni.
- [DECISIONE PRESIDENTI RICONFERMATA] Nome, eta', ritiro e sostituzione automatica nel medesimo club, nome/eta' in CLUB OFFER e WELCOME. `THE PRESIDENT` e' solo il testo OG corrente. Dati degli allenatori AI e valori numerici dei presidenti da decidere nel penultimo passo.

### 0.13.27 — offerte globali + ponte risultati coppe OG + BAT vincolante (12/11/2026)
**TEST REALI 0.13.26 UTENTE:** `LIVING_WORLD_70_TEST(2).log` conferma `MIDWEEK_EVENT_SAVE_RELOAD=PASS`, `WORLD_EVENT_CURSOR=PASS`, `STRESS_RESULT=PASS`: 70 stagioni e 282800 risultati archiviati, 15 save/load, 0.77s CPU nell'harness. F12 in carriera mostra `OVERDUE 0`, 216 club/11 leghe. Non equivale alla prova completa di ogni competizione.
**CODICE 0.13.27:** `Expanded_UnemployedWait` ora fa scadere l'offerta non accettata quando il manager sceglie FORWARD TIME, senza bloccare le settimane. `choose_small_club` usa i file/club idonei del Living World mondiale e non filtra mai in base alla nazionalita' del manager; rimosso l'obbligo artificiale di almeno tre divisioni. `ExpandedWorldLive_LoadOfferCountries` legge la lista paesi del mondo attivo o OG e importa le leghe con le regole/pairings OpenSWOS, espandendo i limiti del registro.
**COPPE OG:** il punto centrale `DIY_PlayCupMatchAndAdvance` registra dopo il risultato reale il giorno, identita', turno, fixture, eventuale replay, risultato e i vincitori effettivamente determinati dal tabellone. Il Living World conserva un registro temporale nello stesso snapshot .CAR (schema interno v4, leggibili v1-v3); i due anni residenti sono compattati in conteggi archiviati. Non viene creato un secondo simulatore o modificata la fisica partita.
**CHECK BAT:** `TEST_LIVING_WORLD.bat` richiede esplicitamente i marker `MIDWEEK_EVENT_SAVE_RELOAD`, `WORLD_EVENT_CURSOR`, `GLOBAL_OFFERS_DISCOVERY`, `CUP_NATIVE_JOURNAL`, `CUP_ARCHIVE_70`, `STRESS_RESULT`, tutti `=PASS`; controlli assenti/falliti danno exit code nonzero. Il test coppa genera appositamente una piccola coppa sintetica/anno: prova journaling/archiviazione e roundtrip, NON i tabelloni giocati nel gioco.
**PENDENTE:** compilazione/test Windows/MSYS2 0.13.27; prova in-game offerte internazionali, rifiuto implicito via FORWARD, coppa manuale e CPU (andata/ritorno/replay), cambiamenti di fase e caricamento del save precedente. Il ponte osserva solo competizioni realmente processate dal motore OG; competizioni interamente offscreen durante disoccupazione, fixture di coppe future, tornei di tutti i paesi e simulazione integrata completa restano da completare. Non dichiarare l'engine finito.

### 0.13.28 — diagnosi entrata carriera e suite integrata (12 novembre 2026)
**INPUT TEST UTENTE 0.13.27:** screenshot Win x64 confermano REAL WORLD 1303 club / 88 leghe, OVERDUE 0, 18 risultati nativi riutilizzati, 84 fallback OG, 789 frame del recupero storico / 333 ms in quel percorso, direct OG import 0. L'automazione 70 stagioni / coppe sintetiche / save-load e' PASS nel log inviato dall'utente.
**CAUSE DISTINTE:** il mondo nazionale e il calendario di carriera originale non condividono ancora tutti gli eventi cup e, quando il direct import non trova tutti i risultati o il key-cache e' incompatibile, Expanded_MidseasonJoin_Begin lascia attivi i callback OG. Il join puo' quindi visualizzare l'antico pannello di sincronizzazione. Fallback non significa automaticamente classifica errata, ma nemmeno prova unificazione.
**MODIFICHE CODICE 0.13.28:** durante il replay breve l'SDL mantiene il frame precedente senza presentare il vecchio full-screen di attesa; continua a misurare frame/ms e a eseguire i callback OG. F12 usa sezioni in due colonne colorate, distingue inventario installato, numero di eventi OG non-league storici e ragione dell'import diretto fallito. Non e' stata rimossa la duplicazione architetturale del catch-up. Il focus UNEMPLOYED resta su FORWARD TIME anche con offerta; tasto F12+4 attiva/disattiva risultato DRAW 1-1 (solo risultato registrato, non simulazione fisica abbreviata); WIN/LOSE/SACK invariati.
**DATI REALI:** il loader TEAM.* conta paesi, file, club e leghe realmente importate all'avvio e li riporta in SWOS.log/F12. Il nuovo inventory test legge i file installati data/TEAM.* e i count del catalogo JSON senza alterare i dati; non equivale a simulare per 70 stagioni tutte le squadre. I contatori del loader non sono persistiti nel .CAR, quindi dopo un riavvio possono essere assenti.
**BAT UNICO:** TEST_LIVING_WORLD.bat lancia ora la suite 70 stagioni, midseason calendario, world-live, debug/ledger e inventory reale, con un solo log LIVING_WORLD_70_TEST.log. Mobiltà interdivisionale, coppe interamente offscreen e handoff a piu' stagioni restano NOT IMPLEMENTED, non PASS.
**TEST:** revisioni statiche e riscontro screenshot/log utente 0.13.27; GCC 16.2/MSYS2 test runner e build completa Windows 0.13.28 DA ESEGUIRE. Nessun .CAR cancellato; nessuna modifica alle regole sportive.

### 0.13.29 — test mobilità, debug match rapido, bug BAT (12 novembre 2026)
- INPUT 0.13.28 WINDOWS: compilazione, F12, focus FORWARD TIME, DRAW e save/load SÌ; ingresso carriera NO; suite test NO per undefined reference a _lastFinancialWeekIndex nel runner isolato; stress 70 stagioni e coppe sintetiche PASS. Coppa manuale non provata.
- BAT: il test isolato midseason definisce la variabile OG, senza alterare la build di produzione. Suite comune aggiunge 4 divisioni / 70 stagioni con tabelle native, salvataggi, controlli identificativi, risultati e percorsi estremi in entrambe le direzioni. Inventario TEAM/JSON reale distinto dai test sintetici.
- MOBILITA: usa promoted, relegated, promotedPo e relegatedPo originali dei descriptor OpenSWOS. Movimenti automatici scambiano club solo tra categorie adiacenti esistenti, dopo risultati della stagione precedente; ordine dei team ricostruito dagli indici delle tabelle OG; calendario rigenerato solo dopo la classifica finale; stato salvato in versione interna 5, lettura v1-v4 ancora disponibile. Spareggi e regolamenti speciali OG restano pendenti.
- MATCH MANUALE: F12+5 dentro una vera partita carriera attiva/disattiva x4 di pacing. Ogni frame originale di GameLoop_Frame si esegue, con presentazioni video intermedie saltate. Torna a x1 alla fine. Necessari test Windows input/audio/render.
- INGRESSO: F12 distingue data OG, WORLD DATE e CLOCK MISMATCH; join OG legacy e callback coppe non eliminati. Non considerare risolto un problema di sincronizzazione soltanto perché non appare il vecchio caricamento.
- LIMITI: promozioni automatiche nella struttura presente, non spareggi; catalogo reale ancora non collaudato end-to-end per 70 stagioni; coppe mondiali offscreen, dati allenatori/chairman/calciatori e simulazione globale completa non terminati. Forma e dati numerici da decidere insieme al penultimo passo; audit generale del codice ultimo intervento strutturale prima di 0.14.00.
- TEST ASSISTENTE: analisi GitHub + verifiche statiche e simulazione parallela JS sulle regole di ranking. Non compilato GCC/MSYS2 né Windows: da testare a cura dell'utente. Non cancellare AUTOSAVE.CAR.
- PATCH ESTERNA: richiesto confronto DOPO commit, possibile fonte per regole moderne, divisioni e numero squadre/giocatori; nome/link non ancora individuato.

### 0.13.30 — raccordo nativo, storia coppe, regressioni (12/11/2026)
- Input Windows 0.13.29: stress 70 stagioni PASS, suite unica NO per signedness del confronto nel nuovo test di mobilità; non confondere warning di compilazione con prova di classifiche errate. Entrata carriera NO: dopo stesso numero di gare il Living World e OG avevano punti differenti. Formula punti OG approvata e non modificata.
- Raccordo gare di lega per coppia ORIENTATA di ID e stagione: risultato unico, data nativa riallineata preservando i token di consumo; se più partite indistinguibili, non attribuire un risultato arbitrario. Risultato della partita manuale OpenSWOS autorevole. Le gare OG non riconciliate ora vengono segnalate come potenziale divergenza.
- Coppe: callback OG originali intatti. Registro mondiale rifiuta risultati/titoli contrastanti e serializza anche i campioni storici oltre le due stagioni in memoria, schema world v6 con lettura v1-v5. I .CAR v4-v5 archiviano numero titoli ma non tutti i nomi; salvarli/ricaricarli senza inventare campioni.
- Debug gioco reale: in F12 selezionare 1 WIN / 2 LOSE / 4 DRAW e poi 6 per uscita nativa veloce. Percorso da testare su Windows anche in coppa, conservando il gioco normale. Il vecchio 5 x4 resta solo debug.
- Test: fix signed/unsigned al runner 4 divisioni; stress 70 anni verifica anche continuità dei vincitori, idempotenza, conflitti e save/load. Source review e verifica di parentesi, ma nessuna compilazione MSYS2/Windows eseguita dall'assistente.
- Ancora PENDENTI: tutte le coppe reali offscreen durante disoccupazione, eliminazione sicura del catchup OG, qualificazioni/spareggi speciali, test punto-per-punto della classifica col database reale. Poi decisioni dati allenatori/chairman/giocatori e audit/refactoring finale prima 0.14.00.


### 0.13.32 — raccordo corretto tra date SWOS e Living World (12/11/2026)
- [CODICE] Il valore `uint8_t phase` e' una fase nativa in ottavi di mese (non una settimana); `ExpandedWorldDate` e' la data civile e `int32_t day_index` il numero di giorni dal 1900. I tipi non vengono confusi: la conversione `ExpandedWorld_NativePhaseToDate` resta unica. Il motore usa lo stesso clock `ExpandedWorldLive_AdvanceToDate` quando importa un risultato finale OG in una data futura.
- [BUG FIX] La ricerca del risultato nativo non applica piu' una finestra agosto-agosto che escludeva i campionati maggio-settembre: utilizza la coppia ordinata ID squadra e lo slot originale della stagione/competizione. Anche fasi OG maggio-giugno dello stesso anno iniziale mantengono la corretta stagione.
- [OFFERTE] Prima offerta da disoccupato: solo paesi con >=4 divisioni realmente schedulate nel Living World (anche su due TEAM files), sempre nella divisione piu' bassa presente; necessita classifica disponibile e almeno tre gare disputate. In futuro con otto divisioni usa automaticamente l'ottava. Questa regola sostituisce la precedente offerta globale da qualsiasi divisione per il solo primo impiego.
- [SUITE] TEST_LIVING_WORLD.bat contiene anche stress 70 anni con ID/nomi/leghe delle squadre dei cataloghi JSON reali; non equivale ancora a simulare tutte le regole OG, playoff/coppe o i giocatori. Conteggi effettivi e file falliti sono visibili; test non superati restano FAIL, il runner continua le altre fasi e fornisce riepilogo.
- [TEST] Ispezione statica di codice/branch, non compilazione Windows o esecuzione C MSYS2 dell'assistente; verifica Windows e confronto della classifica OG dopo la prima offerta rimangono PENDENTI.


### 0.13.33 — coppe native CPU e cataloghi domestici, regressioni (12/11/2026)
- [CODICE] I risultati e i vincitori delle coppe OG passano al registro Living World anche durante il percorso nativo di autoavanzamento internazionale (flag OG), senza introdurre un secondo tabellone. Fallimenti di registrazione del vincitore non restano piu' silenziosi. Non copre automaticamente tutte le coppe offscreen durante disoccupazione: questo requisito rimane aperto.
- [DATE] La fase nativa di una partita di campionato e' verificata contro l'intervallo in ottavi di mese importato dalla sua CompetitionLeague OG: fase 0 non e' valida per una lega agosto-maggio; il campionato maggio-settembre conserva le sue fasi originali. Nessun cambiamento alla formula punti.
- [STRESS COPPE] Test sintetico su 70 stagioni con due tornei simultanei, risultati di andata/ritorno/replay, identita' e idempotenza, salvataggi ed archivio dei titoli. NON e' simulazione del sorteggio OG o completamento di tutte le coppe del mondo.
- [DATI REALI] TEST_LIVING_WORLD.bat usa il medesimo runner per cataloghi JSON reali dei campionati per 70 stagioni; nazionali, pool giocatori e cataloghi di iscritti alle coppe vengono inventariati separatamente come NON applicabili a un campionato. Totali effettivamente processati visibili. Marcatori distinti da coppe OG ancora non collaudate in-game.
- [BAT] Riepilogo confronta le righe intere lette da FOR/F e non fa piu' apparire un DB_INVENTORY=PASS come FAIL per il problema findstr /X in Windows. Rimane FAIL autentico se mancano marker o gli assert non passano.
- [VERIFICHE] Lettura codice, test di coerenza statici e ricognizione dati JSON su GitHub. Non eseguiti MSYS2/GCC e build Windows dall'assistente; collaudo Windows richiesto all'utente.

### 12/11/2026 — isolamento competizioni DIY / carriera
- [REGOLA] Le funzioni DIY OpenSWOS sono condivise fra competizioni liberamente create e carriera: preservare le prime integre e fuori da ledger, salvataggi e simulazione Living World. Non separare riscrivendo il motore OG: isolare solamente gli hook Expanded con il tipo partita GAME_TYPE_CAREER (4).
- [FIX 0.13.33_01] I due hook di registrazione risultati/vincitori in DIY_PlayCupMatchAndAdvance sono guardati da _gameType==4 e dalla presenza di club Living World; il solo _internationalAutoAdvanceFlag non puo' attivarli fuori carriera.
- [LIMITI] Verifica statica della separazione; test Windows di coppa carriera e DIY autonomo ancora pendenti.


### 0.13.34 — World Cup Inspector, dati JSON e diagnosi ingresso (13/11/2026)
- [IMPLEMENTATO] F12+C alterna riepilogo Living World e ispettore di sola lettura delle ultime 24 registrazioni coppa provenienti da callback OG. Date, hash identificativo (il nome non e' persistito nel registro), slot, round, leg, home/away, risultato o vincitore. Nessuna modifica a DIY/CLASSIC MODES. L'assenza di eventi e' esplicita: NO OG CUP EVENTS RECORDED. Titoli e partite archiviate conservano solo i rispettivi contatori; non ricostruire dati inventati.
- [DA COMPLETARE] Coppe offscreen di tutti i paesi durante disoccupazione NON simulate/validate end-to-end: un registro che aumenta quando gioca OG non dimostra che le competizioni lontane progrediscano autonomamente. Qualificazioni, nazionali, playoff e passaggio AI al di fuori del club rimangono aperti. Prima di dichiarare le coppe complete servono i relativi eventi concreti e test.
- [TEST REALE] Il collaudo cataloghi JSON verifica per ogni divisione se il calendario OG e' stato effettivamente generato; gruppi con una sola squadra sono indicati singolarmente UNSCHEDULED SINGLETON_NO_FIXTURE, non conteggiati come 70 campionati, e non ricevono risultati fittizi. Qualsiasi divisione con 2+ club senza calendario resta FAIL. I campionati continuano a essere simulati per singolo catalogo, non in un mondo globale unico.
- [DIAGNOSI INGRESSO] Il percorso DIY_WorldDirectImportLeague della sola carriera ora identifica la precisa condizione OG incompatibile in F12 (gironi, numero squadre, single-leg, accoppiamenti, stato non iniziale, ecc.), invece del generico UNSUPPORTED. Non bypassa il replay originale e non altera i punti. Differenza Australia offerta/classifica OG ancora da validare e risolvere su Windows.
- [TEST] Assert sintetico aggiunto sulla nuova API recent-events; solo ispezione statica e riscontro dei cataloghi reali effettuati dall'assistente. GCC/MSYS2, Windows, checksum dei salvataggi e confronto punto per punto dopo assunzione PENDENTI. Non dichiarare ENGINE_SUITE=PASS senza nuovo log utente.


### 0.13.35 — sicurezza recupero OG e test 5 anni (13/11/2026)
- [CODICE / WINDOWS NON PROVATO] Corretto l'autosalvataggio anticipato nei due ingressi in carriera (first-contract e new-contract). Quando il recupero OG di coppe/campionati e' attivo, il salvataggio viene rinviato a native-join-complete; il salvataggio immediato resta solo quando il recupero non e' attivo. Le build VERIFY ricaricano i blocchi di competizioni dopo ogni salvataggio: il reload nel mezzo del recupero OG era rischioso. Causa del crash utente NON ancora dimostrata.
- [NUOVA FUNZIONE DEBUG] F12+7 dal calendario UNEMPLOYED arma il salto di 260 settimane usando il calendario Living World alla successiva selezione di FORWARD TIME. Non modifica risultati, tabelloni OG o controlli. Il test di assunzione dopo cinque anni e' attualmente PENDENTE e il midseason join multi-stagione resta NON SUPPORTATO; NON etichettare come completo il flusso.
- [CRITICO APERTO] Coppe offscreen: il codice OG aggiorna il journal soltanto mentre vengono eseguite le sue callback. Manca l'isolamento per competizione dei globali e contesti OG per aggiornare e salvare i veri tabelloni di tutti i paesi, coppe domestiche e internazionali. Non creare risultati fittizi o un secondo motore. Richiede una vera integrazione, con fixture, promozioni e salvataggi coerenti; non certificato nella 0.13.35.
- [VALIDAZIONE] Commit sorgenti GitHub e controllo testuale dei cambi non equivalgono a compilazione o test Windows. ENGINE_SUITE 0.13.34 era PASS solo sulla copertura domestica; i test complessivi della 0.13.35 restano PENDENTI.


### 0.13.36 — OG season alignment after prolonged unemployment (13/11/2026)
- [CODE, WINDOWS BUILD/TEST PENDING] Native midseason cutoff is now calculated from the actual world season, not from a fixed 1996 initial-year window. InitCareer/rehire align _currentSeasonShowing and _seasonPlaying before native cup catch-up. New pure regression cases for 43, 260, 261, 273 weeks, preserving legacy 52-week helper semantics. F12+7 is documented in the debug HUD (+260 weeks on next FORWARD TIME).
- [CODE, CRASH UNPROVEN] The career fixture renderer no longer dereferences null/invalid low-address top-level competition pointers after save/load verification; it preserves the native result row and uses the stored competition slot name. Crash 0xc0000005 RVA 0x25920 requires Windows retest and addr2line for verification.
- [NOT IMPLEMENTED] The offscreen OG cup scheduler/state isolation for all countries is still absent. Existing OG callback replay after a contract and the observed-result journal are not equivalent to world cup simulation while unemployed. Do not claim complete cups, playoffs or multi-season seasonal roster mobility based on these changes.
- [DIRECTION CONFIRMED] The final Living World must NOT calculate football scores itself, including expanded leagues. Original OG match simulation should be adapted as a score provider for all simulated fixtures; Living World orchestrates and archives. Current score_world_result() remains a simplified independent resolver and must be replaced in a separately verified integration, not silently in this commit.
- [TESTS PENDING] Native Windows compilation, 43-week join, continue career, 5-year join, world/OG current-season standings and offscreen cups must be verified before calling this build stable.


### 0.13.37 — world→OG native current-season membership + late-season handoff (13/11/2026)
- [CODE / Windows build + gameplay UNTESTED] On InitCareer, after resetting OG's division-offset scratch and BEFORE InitializeNewSeason, ExpandedWorldLive_StageNativeMobility enumerates the real scheduled CURRENT-season world rosters. It validates every native TEAM globalTeamNumber, applies current-season league offsets only after two-pass validation, and leaves the original OG league/cup fixture construction/callbacks intact. It does not use the mutable next-season LiveClub.division.
- [CODE / Windows UNTESTED] Midseason align now requires a prepared native roster. A contract with inconsistent OG member mapping is rejected without overwriting AUTOSAVE.CAR (no new empty-point career misreported as success).
- [CODE / Windows UNTESTED] Completed domestic league handoff: direct OG score import now permits the original division's ALL fixtures to have finished before the offer. It audits exact club IDs/W-D-L/GF-GA/points against the current Living World full season before committing the original OG slot. When the OG career calendar has no future fixture, the native headless callback driver may continue through the authentic cup/season-end flow, with completed domestic import REQUIRED; if this cannot be done, fail closed, do not mark or save false sync.
- [LIMITATION CRITICAL] This is a bridge for the SELECTED CLUB'S native OG calendar and cups after hiring. It is NOT global OG offscreen cup simulation: while unemployed, all other countries' knockout brackets are still not updated/saved as full OG state. The OG scoring engine is NOT YET the sole score producer for every world league; expanded_world_live.c resolve_game still uses its simplified scorer. No claim of full-engine completion.
- [TEST PENDING] The isolated world unit test checks all source TEAM file enumeration. End-to-end Windows tests must cover five years unemployed and mid-season offer, exact original league standings, national cups with native callback replay, end-of-season offer after last league fixture, successful save/load, and former 0xc0000005 crash reproduction. If any differs, record FAIL, not PASS.


### 0.13.37_01 — test di copertura globale 70 anni (13/11/2026)
- [CODICE COMMITTATO, NON TESTATO WINDOWS] Il precedente \`test_real_json_world_70.c\` avanzava 70 stagioni separatamente per ogni catalogo nazionale, senza provare simultaneamente identità, limiti e calendario dell'intero mondo. Dopo i controlli individuali ora carica **tutti i cataloghi domestici in un unico stato Living World**, esegue 70 stagioni, verifica club importati, gruppi schedulati, singleton non simulabili, conteggio dei campionati archiviati e risultati effettivamente archiviati. I nomi e gli ID sono quelli dei cataloghi installati.
- [BAT] \`TEST_LIVING_WORLD.bat\` richiede i nuovi marker \`GLOBAL_DOMESTIC_CLUB_COVERAGE=PASS\` e \`REAL_JSON_GLOBAL_70_SEASONS=PASS\`; in caso di errore l'intera suite deve uscire FAIL. In parallelo rimane il test sintetico da 216 squadre per le regressioni rapide. La copertura delle squadre fuori campionato è riportata separatamente, non finta come campionato.
- [LIMITAZIONE CRITICA] Questo commit NON implementa le coppe mondiali offscreen, NON sostituisce il calcolo dei risultati semplificato con OG e NON prova tornei nazionali/continentali o spareggi originali. Il registro coppe osserva soltanto eventi effettivamente processati dalle callback OG. Non classificare la suite dei campionati come prova del motore completo.
- [SEGNALAZIONE UTENTE 13/11/2026] Dopo 5 anni disoccupato, classifica Living World/OG della divisione australiana mostrata coincide (Brisbane Lions 7ª, 30 punti in 22 gare). Le coppe invece non avanzano visibilmente durante la disoccupazione e vengono elaborate soltanto dopo l'assunzione: FAIL aperto.
- [TEST PENDING] Compilazione MSYS2 e \`TEST_LIVING_WORLD.bat\` sul PC dell'utente; test di assunzione dopo 5 anni, coppe durante disoccupazione, salvataggio/caricamento e allineamento OG. Nessun PASS globale dichiarato in assenza di log.

- [AUDIT OG 13/11] `Career_ViewOrEditCompetitionSlot` in `sorgenti/src/career_init.c` usa `Career_AutoAdvanceCupCompetitionFromRecord`, `Career_AutoAdvanceLeagueCompetitionFromRecord` o `Career_AutoAdvanceInternationalCompetitionFixtures` per le competizioni NON presenti negli slot della carriera selezionata. È un percorso di VIEW/consultazione, NON prova dell'esecuzione automatica durante `Expanded_UnemployedWait`. `InitializeNewSeason` inizializza i 5 slot a partire dalla squadra scelta; mancano per ora uno stato OG offscreen indipendente e un test OG prima dell'assunzione. Non chiamare le routine di VIEW su stato OG non inizializzato.


### 2026-11-13 — Separazione inizializzazione carriera (0.13.37_02)

- `InitCareerWorld()` inizializza template e strutture OG a START CAREER; `InitCareerManager()` inizializza lo storico personale una sola volta; `InitCareerClub()` prepara gli slot della squadra; `InitCareer()` resta il punto di ingresso compatibile.
- Il percorso di riassunzione chiama `Career_AttachManagerToClub()` senza ripetere i reset mondiali o `Expanded_StartCareer()`. Rimangono da validare sul gioco le ricostruzioni degli slot OG e i salvataggi.
- Il test JSON aggiunge un inventario di **ogni record** in tutte le categorie (club, pool, coppe, nazionali), distinguendolo dalla simulazione di 70 stagioni dei soli campionati. Non esiste ancora un test end-to-end delle coppe OG per tutte le nazioni e delle nazionali: status NOT_TESTED, non PASS.
- Codice modificato e commit remoto: verifiche di compilazione e test Windows ancora pendenti.


### 13/11/2026 — 0.13.37_03: inizializzazione carriera / F12 / database

- Prototipi dichiarati per `InitCareerWorld`, `InitCareerManager`, `InitCareerClub`, `Career_AttachManagerToClub`; `Career_BeginNewCareer` separa il reset esplicito del mondo dai contratti successivi. Il percorso di riassunzione non richiama più `InitCareer(newTeam)`.
- Hook `Career_AdvanceUnemployedOGCups`: chiama le callback OG delle coppe domestiche dopo ogni anno completo di disoccupazione. **Parziale**, non sostituisce un calendario OG completo; coppe UEFA, nazionali, risoluzione tabelloni in tempo reale, save/load e riassunzione devono essere testati/implementati ulteriormente. Non dichiarare completamento.
- F12: distingue i file e le squadre effettivamente caricati dal loader OG almeno una volta dai club e slot giocatore importati in LWE; attenzione: i file OG visti non sono contemporaneamente residenti.
- Test JSON: aggiunta convalida dell'intero catalogo 1996/97 prima dello stress domestico 70 anni; test statico regressione lifecycle. Build e test Windows **non eseguiti** in questa sessione.
- Conteggio repository **76 cataloghi JSON, 1.620 record squadra, 25.920 slot rosa + 900 POOLPLYR = 26.820 slot giocatore, 80 partecipanti TMD copiati (non nuove squadre)**. Dettagli e metodologia in `docs/DATABASE_1996_97_COVERAGE.md`.


### 13/11/2026 — 0.13.37_04: confronto automatico classifiche / F10 import / coppe
- `DIY_WorldAuditStandingsAtMenu` confronta in lettura classifica OG e LWE per ID, W/D/L, gol e punti quando si apre il menu OG; log `[join-standings] PASS/FAIL/SKIP`. Non modifica casualità né stato della competizione.
- Nuovo ispettore **F10 nei menu Career**, preservando F10 Arcade durante le partite: file TEAM realmente aperti, club importati, record scartati per motivo con esempi di nome/ID/file/divisione, paesi senza record validi, coppe OG registrate separate dalle competizioni realmente giocate. Dettagli in `docs/DATABASE_1996_97_COVERAGE.md`.
- Durante la disoccupazione l'hook OG coppe considera `g_worldCups.seasons` per evitare riavanzamenti evidenti al ripristino e traccia le differenze del registro OG di risultati e vincitori. Non è ancora la simulazione integrale dei tabelloni, né prova le qualificazioni e le nazionali.
- Suite: controllo statico del wiring e migliorata stampa marcatori BAT; build e test runtime Windows ancora pendenti. I pass statici NON certificano le classifiche o le coppe nel gioco.

### 13 novembre 2026 — Build 0.13.38: diagnostica runtime (test Windows pendenti)
- Aggiunto in F12 un audit del caricamento TEAM: tentativi, stato, righe contabilizzate, accettate/escluse, file falliti e ultimo paese/file. Le righe devono essere contabilizzate integralmente o l'importazione viene segnalata ERROR.
- Aggiunti conteggi coppe OG al momento dell'importazione: record di coppa registrati distinti dai risultati e dai titoli prima/dopo; storico e residente rimangono separati. Nessun record registrato è spacciato per coppa simulata.
- I conflitti OG delle coppe stampano ora data/slot/identità e risultati o vincitori precedenti e nuovi. I conflitti volutamente provocati dallo stress test sono identificati come EXPECTED_REJECTION senza eliminare le asserzioni di rifiuto.
- Audit cronologico: F12 riporta cambi dello scarto OG/LWE e input della conversione; il percorso midseason traccia ALIGN_PRE, ALIGN_APPLIED, JOIN_BEGIN e JOIN_FINISH con stagione, fase e cursore. Sono osservazioni runtime, non auto-correzioni del calendario.
- Rimosse dalla suite del motore le due prove statiche, che restano fuori dalla certificazione funzionale. PASS dei test C isolati non dimostra l'integrazione runtime Windows né le coppe offscreen.
- Test Windows, compilazione e verifica del flusso OG/LWE ancora da eseguire dall'utente. Non sono stati modificati risultati, simulazione, salvataggi o rendering del gameplay.

### 13 novembre 2026 — 0.13.38_01: verdetti runtime e protocollo screenshot
- F12: OG/LWE STANDINGS PASS solo dopo un confronto OG effettivo, tramite ID, senza differenze; FAIL dopo una o piu' differenze; NOT_TESTED prima di eseguirlo. Mostra numero di confronti ed errori. PASS si riferisce SOLO ai campionati confrontati.
- F12: OG CUP TITLE INDEX PASS solo se esistono titoli archiviati tutti indicizzati, FAIL se titoli archiviatI superano gli indici, NOT_TESTED senza titoli archiviati. NON verifica simulazione offscreen o coppe internazionali.
- Il messaggio F12 sugli orologi vicini e' stato rinominato CLOCK SAMPLE NEAR (NOT JOIN PROOF): non deve essere interpretato come test di corretto handoff.
- TEST_LIVING_WORLD.bat elenca esplicitamente il controllo runtime CUP_NEGATIVE_REJECTIONS_70, mantenendo le relative asserzioni negative.
- Dopo ogni build, chiedere screenshot mirati in base alle parti modificate e dichiarare cosa dimostrano, distinguendo schermate normali e debug. Protocollo consigliato per il prossimo test Windows: (1) dopo 5 anni disoccupato F12; (2) F10 import/coppe; (3) F12+C registro coppe; (4) subito dopo l'offerta F12; (5) classifica OG e riepilogo della stessa divisione senza avanzare; (6) dopo un solo FORWARD TIME F12. Non rifare i cinque anni se il salvataggio di test esiste. Richiedere SWOS.log per identificare il primo evento [clock-audit], [join-standings], [og-cups-audit] pertinente.
- Compilazione e verifiche Windows rimangono responsabilita' dell'utente: mai descriverle come eseguite da remoto.

### 13 novembre 2026 — 0.13.38_02: verifiche della divergenza meridionale
- Il log Windows 0.13.38_01 ha dimostrato Australia country=44 division=3, contratto al 30 luglio 2001: OG 0 incontri, LWE 14 per squadra; `cutoff=0` saltava il recupero di partite gia' disputate in gennaio-luglio.
- In `expanded_midseason_join.c` il percorso eccezionale per una competizione con prime partite native prima di agosto calcola il cutoff effettivo di luglio dalla data, non dalla preseason generica. Le competizioni agosto-maggio mantengono il vecchio comportamento.
- Il recupero del campionato si appoggia al ledger LWE esistente e alle strutture native OG: per il caso meridionale e' obbligatorio il confronto esatto di tutti i club, partite, gol, punti; in caso di rifiuto restituisce FAIL e non simula arbitrariamente risultati alternativi.
- F12 presenta SOUTHERN HANDOFF PASS/FAIL/NOT_TESTED (PASS solo dopo confronto esatto), e i contatori di tentativi di avanzamento delle coppe OG; i log indicano esplicitamente quando la stagione coppe viene saltata.
- Nuovo analizzatore `sorgenti/tests/audit_windows_runtime_log.sh` per eventi osservati dal gameplay: evidenzia la prima anomalia OG/LWE e distingue log di altre build. `TEST_LIVING_WORLD.bat` lo esegue separatamente dallo stress C, conserva il proprio SWOS_RUNTIME_AUDIT.log e non dichiara testati gli scenari senza SWOS.log.
- Una regressione C specifica riproduce week=265, anno 2001, mese di luglio, prima fase OG anteriore ad agosto; non certifica il handoff completo del gioco.
- Codice e logica della SWOS 96/97 Mod verificati: i port `fixture_rounds.c`, `league_groups.c` usano gia' dati/tabelle del Mod; non si aggiungono calendario, motore o dati duplicati.
- Compilazione Windows e controllo del comportamento del caso reale PENDENTI all'utente. Nessun PASS Windows dichiarato in assenza di log runtime.

### 13 novembre 2026 — 0.13.38_03: ambito corretto dell'audit orologi
L'analizzatore Windows classifica FAIL l'orologio soltanto se una lettura F12 sullo stato effettivo contiene uno scarto anomalo (`source=F12_CALENDAR_SAMPLE`). La differenza iniziale `ALIGN_PRE`, diagnostica del recupero ancora da eseguire, non e' un FAIL del calendario recuperato. Verifica dell'analizzatore sul log reale SWOS(3).log: 3 confronti classifica FAIL, scarti F12 -151/-128 giorni FAIL, coppe non registrate dopo 264 settimane FAIL, caricamento TEAM PASS. Questa e' una verifica del parser sul log precedente, non una build/test Windows 0.13.38_03.

### 13 novembre 2026 — 0.13.38_04 CLOCK TRACE runtime
- Nuovo tracciamento read-only degli orologi con eventi UNEMPLOYED_BEFORE_ADVANCE / AFTER_ADVANCE, ALIGN_APPLIED, OG_CATCHUP_TICK, DIRECT_IMPORT_HANDOFF, JOIN_FINISH e OG_LEAGUE_RESULT.
- Per cambi effettivi di stato registra i giorni OG/LWE, indici temporali, delta, avanzamenti per evento, stagione e fase; conserva il primo errore dal momento in cui l'handoff e' applicato. Nessun giudizio di successo in disoccupazione/recupero OG; NEAR indica soltanto osservazione, non PASS funzionale.
- F12 espone lo stato CLOCK TRACE, numero di campioni/cambi, ultimo evento e primo errore. Il BAT di analisi runtime distingue il primo clock-trace FAIL dalle osservazioni pre-handoff.
- Tracciamento non modifica gioco, calendario, scoreboard, salvataggi o 50 Hz. Compilation/test Windows ancora da svolgere dall'utente.

### 13 novembre 2026 — 0.13.38_05: diagnostica permanente competizioni / recupero OG (Windows DA VERIFICARE)

- L'handoff della carriera ora controlla l'effettiva fase OG anche quando la carriera raggiunge un menu di fine stagione. Se actual<target: `[midseason] FAIL EARLY_HANDOFF`, LastResult=2 e nessun salvataggio di avvenuto recupero. Regressione C sul caso 143 contro 96/97; il test NON certifica il recupero Windows.
- Importazione diretta OG: ora tenta 2, 3 e 4 cicli sfruttando gli incontri REALI gia' giocati del singolo LWE. Per orientamenti HOME/AWAY duplicati distingue l'occorrenza del match nel calendario nativo (id); per 3/4 cicli obbliga il confronto esatto della classifica club-by-club anche prima della fine stagione e fallisce chiuso su mismatch. Gruppi/split, non gestiti, rimangono OG e NON diventano implicitamente supportati.
- CLOCK TRACE azzerato con Career_BeginNewCareer, campionato evento per evento sulla fase/cursore anziche' a ogni SDL frame; non cancella i primi errori quando avviene un'altra offerta nella stessa carriera.
- DATA FORMAT AUDIT: `run_audit_competition_formats.sh` esegue un programma C con il parser mjson effettivo sulle definizioni in `data_json/competitions` (leghe, coppe nazionali e internazionali, nazionali, tornei, qualificazioni e playoff) e genera `COMPETITION_FORMAT_AUDIT.csv` con stato dati VALIDATED_DATA, gioco Windows NOT_TESTED. Convalida ID, tipi, cicli 1–4, fasi, divisioni, regole round e riferimenti alle competizioni. Niente numeri fissi per i conteggi: riconosce automaticamente il catalogo espanso.
- F12 (il solo testo debug esistente) visualizza l'ultimo numero di cicli e presenza gruppi osservati dall'importatore; `SWOS_DIAGNOSTIC_CAPTURE=1` abilita il salvataggio nel log del contenuto testuale F12/F10/F12+C quando l'utente apre la pagina, limitato a un cambiamento ogni 5 secondi.
- `AVVIA_DIAGNOSTICA.bat` chiama il normale `AVVIA_SWOS.bat` (compila con `PREPARA_SWOS.bat`) e attende la chiusura tramite una modalita' SOLO diagnostica del BAT originale. Alla chiusura crea `DIAGNOSTICA_SESSIONE_N/` con log compilazione/gioco/audit, e ZIP se `tar.exe` di Windows e' disponibile. Nessuna modifica a save e nessuna seconda esecuzione dell'eseguibile.
- `ANALIZZA_SWOS_LOG.bat` ora invoca la shell MSYS2 dalla cwd del progetto senza login `-lc`: elimina il precedente errore `No such file or directory`. `TEST_LIVING_WORLD.bat` distingue la suite C dai FAIL/INCOMPLETE di Windows: exit 2/OVERALL=INCOMPLETE quando la prova end-to-end manca, non un PASS totale.
- I test C e l'audit sul nuovo catalogo sono da compilare e provare localmente. La vera build Windows e i test menu/partite/coppe restano da fare dall'utente. Nessun risultato Windows dato per PASS.

### 13 novembre 2026 — nuova priorita' calendario / registro tecnico

- Creato `docs/ENGINE_CODE_DISCOVERIES.md` con analisi del codice e
  dei test reali delle chat 11–13 novembre (date OG, ottavi mese, LWE,
  salto F12+7, Hartlepool/Australia/Scozia, coppe offscreen, audit BAT).
  Consultarlo prima di nuove analisi; aggiornarlo con ogni scoperta.
- **Decisione**: non iniziare la fase CAMPIONATI finche' CALENDARIO non e'
  completamente verificato con test su codice attivo + Windows;
  successivamente COPPE. Non si considera risolta una funzione per
  un PASS isolato.
- 0.13.39: convertitore canonico OG bidirezionale (floor/next distinti),
  stessa fase in roundtrip dai primi giorni canonici, LWE unico giorno
  civile esatto; salto 5 anni mediante anniversari gregoriani anziche'
  260 settimane; uso LWE anche durante disoccupazione/rehire; F12
  distingue oggi dalla prossima fixture OG. Test C su fasi e giorni
  1996-2026 e invarianti a 30 anni: da eseguire e poi verificare Windows.
- Stato dei problemi non risolti: recovery anticipato OG `91->50`;
  avanzamento coppe offscreen non dimostrato; merge squadre/competizioni
  non completo; Windows test e save/reload dopo cambi stagione pendenti.
- Correzioni diagnostiche aggiunte solo per rendere testabile la fase:
  audit `teams=-1` dei due formati nativi speciali
  `_tour`/`_friendly`; analizzatore Windows C al posto di `awk`.
  Queste correzioni non costituiscono implementazioni di coppe.

#### Test C effettivi e calibrazione diagnostica della fase calendario

- Test su convertitore C compilato localmente con GCC -Wall -Wextra -Werror:
  109.938 giorni 1900-2200; OG 7.905 fasi canoniche e 11.323 giorni
  1996-2026; +5 anni = 1 luglio 2001; +30 = 1 luglio 2026: PASS.
  Non e' l'exe Windows e non prova l'integrazione OG/LWE.
- `menu_system.c`: l'overlay data del calendario carriera e' passato
  dalla fase finanziaria OG alla data civile reale LWE; Classic/Amiga
  invariati. Windows da verificare.
- Parser Windows C compilato localmente ed eseguito sul precedente
  SWOS.log: separa `STANDINGS PASS`, `HANDOFF FAIL` (91->50) e
  `OG CUPS FAIL` offscreen. Nessuna attestazione di correzione
  del recupero o delle coppe.

### 14 novembre 2026 — calendario: gate trentennale aggiunto, NON ancora certificato

- Intervento 0.13.39_01 **limitato a test e diagnostica**: nuovo test C di
  30 anni su motore LWE effettivo, con date civili gennaio/luglio,
  stagioni nord/sud, .CAR save/reload esatto e rilevamento di eventuali
  doppie fixture. Integrato nel BAT principale, marcatori richiesti.
- Corretto l'audit formati: 16 riferimenti validi a leghe di paesi con
  array diversi venivano letti come voci coppa per uscita dal range JSON.
- Corretto PATH GCC nel BAT analizzatore: `cc1.exe` non trovava libgmp-10.dll.
- Test C e Windows su questo commit: PENDENTI. 
  `RUNTIME_30_YEAR_CALENDAR_SAVE_LOAD` non e' certificato Windows.
  Nessuna modifica al motore partita, calendari OG o coppe.


## Decisioni del 14 novembre 2026 — World moderni, campi, audio e migrazione

- [APPROVATO] Nuova base futura: OpenSWOS ufficiale recente; conservare `dev/0.10` e asset/menu Expanded come riferimento, migrazione controllata su ramo dedicato (non ancora eseguita).
- [APPROVATO] Un solo motore gestionale per 1996/97 e 2026/27 con regole dati distinte; niente duplicazione OG/LWE nella versione finale.
- [CODICE DI PROVA 0.13.41, NON COLLAUDATO WINDOWS] Selettore DEBUG WORLD temporaneo in NEW CAREER con riavvio per non cambiare a caldo le tabelle native; cataloghi squadre 2026/27 presenti nel repository, compatibilita' applicativa da verificare.
- [ARCHIVIATO LOCAMENTE, NON INTEGRATO NEL RENDERER] Raccolta dei DLC SWOS 2020 conservata intera, inclusi file accessori. In futuro classificare per condizioni invernali, stadi piccoli, tornei internazionali ecc. e scegliere dinamicamente secondo la competizione, con criteri da approvare.
- [PIANIFICATO, NON ORA] Telecronaca durante le partite; selezione risorse, compatibilita', licenze e interfaccia rinviate a una fase successiva.


## 0.13.42 — checkpoint GitHub del 14 novembre 2026

- [CODICE] Protezione fingerprint 2627: ritorno a 9697 se i dati installati appartengono a generazione incompatibile; non cambia il salvataggio e non installa file nuovi.
- [DA FARE] Migrazione approvata all'unica autorita' OG per competizioni, calendario, cronologia e carriera: NON COMPLETATA; verificare il piano per il ramo dedicato prima di interventi strutturali. Il selettore DEBUG WORLD 0.13.41 riavvia ancora il processo, contrario all'obiettivo finale di selezione senza riavvio.
- [TEST] Build Windows / menu / 2026/27 / fallback fingerprint: NOT_TESTED; eseguibile Windows non generato dall'assistente. Chat "Connection interrupted" segnalata il 14/11/2026.


## 0.13.43 — World 2026/27 predefinito all'avvio (14/11/2026)

- [CODICE] Se non viene indicato il mondo con impostazione/configurazione/CLI, SWOS Expanded tenta 2627 al bootstrap e verifica WORLD.ini prima di installare le tabelle native. Una scelta esplicita 9697 è rispettata. Avvio Windows NOT_TESTED.
- [NON COMPLETATO] Cambio in NEW CAREER senza riavvio, una sola autorità OG per simulazione completa, migrazione delle competizioni e dismissione LWE, audit 80/127 e stress runtime. Non confondere 0.13.43 con la build finale della richiesta.


## 0.13.44_02 — due autosave separati (decisione 14/11/2026)

- [CODICE] `save/AUTO9697.CAR` e `save/AUTO2627.CAR`; vecchio `AUTOSAVE.CAR` disponibile solo come fonte read-only con world/schema compatibili. Nessuna conversione/cancellazione automatica; il checkpoint del mondo attivo non può sovrascrivere quello dell'altro mondo. CONTINUE verde solo per lo slot compatibile del mondo attivo.
- [PENDENTE] Verificare primo caricamento del legacy, save successivo, switch dei mondi, avvio disoccupato e pensione su Windows. Migrazione world bootstrap a START e separazione init ancora NON IMPLEMENTATE.


## 0.13.44_03 — checkpoint codice native OG (14/11/2026)

- [APPROVATO] OpenSWOS è proprietario della progressione di carriera, data, calendari, partite, risultati e archivi storici OG tanto in 1996/97 quanto in 2026/27. LWE aggiunge sistemi nuovi, non una seconda simulazione autorevole. Niente formule di date, archivi o calendari nuovi in questa fase.
- [CODICE] Lifecycle a START separato fra reset del world OG, reset competizioni OG, manager OG; club all'accettazione. Ripristinato l'elenco nazionale storico per callback OG delle coppe durante disoccupazione. Questo NON significa che tutta la disoccupazione sia gia' simulata da OG: il percorso LWE per i match lontani resta attivo ed è ancora da sostituire.
- [TEST] Windows/MSYS2/suite complete non eseguiti da qui. Conservare entrambi AUTO9697.CAR e AUTO2627.CAR e legacy AUTOSAVE.CAR.


## 0.13.44_04 — LWE simulator disattivato (14/11/2026)

- [CODICE] Dismessa in carriera la generazione LWE di fixture, risultati, amichevoli, mobilità e backfill retroattivo. Richiamate routine OG delle divisioni a fine anno durante disoccupazione. LWE conserva dati dei club e lettura vecchie snapshot senza generare risultati. Data LWE residua è un cursore UI/compatibilità, NON la nuova autorità sportiva.
- [NON RISOLTO] OG offscreen fase per fase, classifiche/offer AI collegate a OG, subentro a stagione in corso, verifica 1996/97 e 2026/27, compatibilità vecchi salvataggi LWE. Test Windows pendenti. Non dichiarare “disoccupazione funziona” da sola esistenza del callback.


## 0.13.44_05 — compatibilità save OG-only
- [CODICE] Snapshot di sola anagrafica LWE senza partite ora deserializzabile (zero fixture e zero risultati dichiarati). Mantiene due autosave distinti e file legacy. Test Windows NOT_TESTED.


## 0.13.44_07 — offerte OG-only basate su divisioni reali
- [CODICE] Il primo incarico da disoccupato può trovare squadre nelle divisioni inferiori usando il catalogo dei campionati OG senza LWE scored matches. Risultati/posizione in classifica OG non sono ancora importati: filtro performance team disattivo, dichiarato nel log. Test Windows pendente.


## 0.13.44_08 — subentro attraverso callback OG
- [CODICE] Con LWE simulator spento, il subentro non richiede più LWE mobile roster né direct table import; usa il fallback nativo OG già esistente per completare gli incontri precedenti. Le competizioni speciali e il handoff stagione finita richiedono prova reale. Test Windows NON ESEGUITI.


## 0.13.44_09 — OG authority non riattiva LWE da save legacy
- [CODICE] Gate centrale OG-only applicato a replay risultati, shuffle, standings e stage mobilità anche quando la carriera precedente conteneva fixture LWE. File di salvataggio invariati, compatibilità runtime Windows non provata. Ripristino OG gestione di tutte le competizioni ancora da testare e completare nei casi speciali.


## 0.13.45 — ponte read-only classifiche OG nelle offerte (14/11/2026)
- [CODICE] `DIY_SortLeagueTableByStandings` acquisisce le righe reali OpenSWOS dopo sort, con validazione identità/partite, senza simulazione LWE; la vista offerte legge per paese/divisione/stagione dalla cache OG, altrimenti mostra indisponibilità. La selezione delle offerte restringe ai club in seconda metà soltanto quando una classifica OG corrente è disponibile; prima usa il fallback delle ultime divisioni e non inventa risultati.
- [LIMITE FONDAMENTALE] A START `InitCareerCompetitions` prepara ancora i template; `InitializeNewSeason` necessita club e viene chiamata in `InitCareerClub`. Mancano scheduler OG universale per turni durante disoccupazione e attivazione completa di tutte le competizioni a START: requisito utente NON COMPLETATO. Cache OG in RAM non sostituisce salvataggio nativo di classifiche. Windows/compile NOT_TESTED.


## 0.13.45_01 — errori di compilazione segnalati dall'utente
- [RISCONTRATO DA LOG WINDOWS] `expanded_career.c:435` duplicava `rankedCount` e `career_init.c:880` invocava `DIY_OGStandings_SetSeason` senza dichiarazione visibile. Entrambi corretti nel codice. Compilazione Windows e prove delle offerte da ripetere; nessun PASS attribuito senza nuovo log.


## 0.13.45_02 — audit obbligatorio Init/START/club, struttura futura OG
- [CODICE] Inseriti log `[career-init-audit]` in `InitCareerClub` e `Career_AttachManagerToClub`, e rimosso attributo GCC `externally_visible` applicato erroneamente a una variabile statica. Questi cambi SONO DIAGNOSTICA, non la migrazione mondiale richiesta.
- [SCOPERTA VERIFICATA] Il loading installa già SOLO il mondo scelto via `World_InstallBaked`/JSON e muta `_competitionsTable`; selezionare il secondo senza ricostruzione sicura è rischioso. Allo START `InitCareerCompetitions` copia template, non avvia giornate. `InitCareerManager` invoca `InitCareerVariables(4)`; `InitCareerClub` richiama `InitializeNewSeason`, che incrementa anno, azzera competition slot pointer table, key cache, e copia i template del club. `InitNewSeason` resetta ulteriori slot. `Career_AutoAdvanceInternationalCompetitionFixtures` carica uno stato temporaneo e non mantiene ogni campionato persistente.
- [OBIETTIVO CONFERMATO, NON IMPLEMENTATO] Preparare 9697/2627 al loading senza installarli contemporaneamente; START seleziona il mondo e avvia tutte le competizioni OG; FORWARD TIME le fa avanzare giornata dopo giornata indipendentemente dal manager; assume = attach club senza reset; licenziamento = detach; salvataggio conserva stagione, competizioni e classifiche OG. Necessaria rifattorizzazione delle routine native di competizione + serializzazione.
- [TEST] Windows compile/test interamente a carico utente. Non segnare build eseguita né engine completo.


## 0.13.45_03 — separazione native season boundary vs attach club (14/11/2026)
- [CODICE] `InitializeNewSeason` resta chiamata per un VERO passaggio di stagione: incrementa `_currentSeasonShowing` e `_seasonPlaying`, sposta i gruppi, resetta i playoff di stagione. Nuova `Expanded_InitializeClubSeasonState` prepara il club SENZA incrementare l'anno e SENZA `WorldPlayoff_Reset`; `InitCareerClub` la richiama invece di `InitializeNewSeason`. Anche `InitCareer` e `Career_AttachManagerToClub` non fanno piu' il vecchio decremento artificiale. Regola delle varianti OG registrata in AGENTS.
- [LIMITE CRITICO] La variante club conserva i contatori globali, ma ricostruisce tuttora slot locali delle competizioni perché non esiste ancora stato OG persistente per ogni paese/divisione. Il percorso START continua a inizializzare solo template e registri; FORWARD TIME non aggiorna ancora tutti i campionati ad ogni evento. NON dichiarare mondo/competizioni operative a START. Save/Windows compatibility NOT_TESTED.


## 0.13.45_04 — lettura anteprima offerte / correzione interpretazione screenshot
- [SCREENSHOT WINDOWS] La lega OG di El Salvador con PL=0 era prima del campionato; screenshot successivo della stessa lega con PL=6 conferma il gioco di risultati native OG almeno in quel percorso. Non affermare che tutte le competizioni OG siano ferme: distinzione tra dati inizializzati e snapshot catturato mancante.
- [CODICE] `DIY_OGStandings_Read` continua read-only; rimossa esclusione errata `hasGroups` dal suo hook di acquisizione, perché 1/2 descrivono anche la progressione OG; ora accetta PL=0 OG reali. Nel menu offerta, senza snapshot nativo, si vedono i nomi già presenti nello snapshot roster dell'offerta e un avviso, non statistiche artificiali. Registrato audit `[og-offer-table] CAPTURE`.
- [STATO] Non ancora confermata la lettura per tutti i paesi, i risultati prima della firma, il calendario mondiale e le coppe. Compilazione Windows/MSYS2 e test postpatch pendenti.


## 0.13.45_05 — test iniziale del mondo OG residente (Windows PENDING)
- [CODICE] Nuovo archivio RAM OG per divisione con immagine `DIYCompetitionEx`, selezione squadre, RNG e fase; routine `Expanded_OGWorld_Start` a START dopo import dati; `Expanded_OGWorld_AdvanceToDate` prima di aggiornare la data della disoccupazione; routine `Expanded_OGWorld_BindClub` che sostituisce la lega locale appena allocata con l'immagine OG residente e rimappa le identità. Se binding riesce, evita `Expanded_MidseasonJoin_Begin` alla firma; altrimenti resta fallback precedente e logging. NON è ancora un refactor completo di tutti gli slot dell'allenatore.
- [LIMITI] La vecchia preparazione degli slot/menu del club è ancora necessaria per keycache e contratti; la routine di binding sostituisce il contenuto della lega primaria, ma non ricostruisce risultati storici del calendario del manager. Coppe/contesti internazionali, saved-state della RAM mondiale, stagioni multiple e sincronizzazione del calendario dopo firma NON certificati. Test Windows/MSYS2, stress test e salvataggio/ripristino PENDENTI. Nessuna prova di PASS.

## 0.13.45_06 — VIEW WORLD senza simulazione all'apertura (14/11/2026)
- [CODICE] Rimossa la chiamata agli auto-advance OG dal ramo di consultazione di competizioni esterne `Career_ViewOrEditCompetitionSlot`. Aggiunto `DEBUG WORLD` nel menu UNEMPLOYED (entry 11) con scelta della competizione e visualizzazione **solo delle leghe OG residenti**, mediante copia temporanea del relativo contesto; messaggio esplicito se assente. Focus, RECORD, FORWARD TIME e offerte conservati.
- [VINCOLO] Simulazioni OG si effettuano solo nel ciclo cronologico: START/FORWARD TIME e normali percorsi match/season, non durante la mera consultazione. Nessun risultato LWE generato dal viewer. Il codice OG originale di simulazione resta disponibile per il mondo e la carriera.
- [LIMITI] Coppe non residenti non visualizzabili in WORLD; salvataggio dello stato OG mondiale, storici risultati per match e ricollegamento del club da testare/completare. Mercato: confermato solo design accordo fuori finestra con trasferimento alla prossima apertura, svincolati tutto l'anno; non implementato.
- [VERIFICHE] Lettura codice su GitHub e validazioni testuali prima del commit. Compilazione e test Windows demandati all'utente (no PASS non verificato). Istruzioni di progetto da aggiornare con «compila l'utente».

## 0.13.45_07 — Correzione sicurezza UNEMPLOYED / VIEW WORLD (15 novembre 2026)
- [BUG WINDOWS RIPORTATO] Nella 0.13.45_06 DEBUG WORLD non visibile, un pulsante invisibile nel centro apriva SQUAD e provocava crash, la prima vista del mondo mostrava classifiche a zero e una seconda apertura andava in crash. L'utente conferma l'eseguibile 0.13.45_06 dallo screenshot. Sono osservazioni Windows, non un test automatico.
- [CAUSA CODICE CERTA] RestorePreviousMenu() in menu_system.c ri-esegue UnpackMenu() ma solo onReturn, non onInit. Il clone disoccupato aveva onReturn=GlobalRetn: al rientro da ChooseCompetitionMenu o viewer tornavano callback, hitbox e link ORIGINALI del Contest senza club (SQUAD ecc.). Non e' ancora dimostrato dal crash log che questo sia l'unica causa del crash, ma il percorso e' reale.
- [FIX CODICE] onReturn del clone ora richiama Expanded_UnemployedInit; essa disabilita, nasconde e azzera tutti i callback ereditati e prima/dopo-draw (44 entry), poi autorizza soltanto 7 FORWARD TIME, 9 BACK, 10 OFFER se presente, 11 DEBUG WORLD, 12 RECORD; attiva il pannello 18. Geometry/renderer di DEBUG WORLD espliciti nel riquadro destro sopra RECORD. MenuProc applica in aggiunta filtro anti-hitbox ereditata per input mouse/tastiera/joypad.
- [VIEW OG] Prima di mostrare la lega residente, valida numero squadre e ogni ID/indice di classifica; niente dereferenziazione se non valida. Il viewer sospende la cattura collaterale delle classifiche per offerte mentre ordina lo scratch locale OG, e ripristina stato e RNG all'uscita.
- [DA COMPILARE E PROVARE IN WINDOWS DALL'UTENTE] Confermare visibilita' e focus, impossibilita' di accesso a SQUAD/trasferimenti dai punti invisibili prima E dopo VIEW WORLD, stabilita' dopo aperture ripetute. Zero partite alla data iniziale 1 luglio e' prevedibile: solo FORWARD TIME puo' far avanzare fasi OG; dopo piu' settimane verificare partite/classifiche. Coppe e salvataggio globale OG restano pendenti.

### 0.13.45_08 — prepare-only OG resident offer table (non compilata Windows)
- Correzione bug: CLUB OFFER -> VIEW COMPETITIONS usava cache OG non popolata invece dello stato mondiale OG `s_expOG`, pur con tabella reale a 18 partite per club.
- Nuova API read-only in career_status, collegata a `expanded_career.c` sia per la selezione club sia per la preview; data world/paese/divisione usati come chiave, ID file+ordinal, ordinamento su copia locale. Non produce né modifica risultati.
- Prove Windows, cronologia, WORLD invisibile, crash FORWARD TIME dopo 04/11/2026 e TOP GOAL SCORERS post-firma ancora pendenti; screenshot riportati dall'utente il 15/11.

### 15/11/2026 — build source 0.13.45_08: offer / visible DEBUG WORLD / OG season-end guard
- [SOURCE] The resident league snapshot is now the source for club-offer standings; UI renderer includes DEBUG WORLD entry 11 and explicit visible label; OG-world callback prevents re-entering a concluded league `minSubstitutes==0`, rejects invalid/self pairings before their results and adds detailed log fields. Only original OG scoring is used, no new simulator. `TOP GOAL SCORERS` after hiring remains uncorrected.
- [WINDOWS 0.13.45_07 EVIDENCE] User reported FORWARD TIME crash after Nov 4, and invisible DEBUG WORLD. Log `SWOS(6).log` shows self-match attempts after full-round-robin counts for Brazil (country 46, 380) and Sweden (country 36, 240). Not every match belongs to an August-May league: country-specific OG JSON phases differ.
- [UNTESTED] Windows compile and runtime 0.13.45_08 (user); exact crash root and whether end guard fully resolves it; time-to-date accuracy of Brazilian competition; atomic all-league rollback if one fixture fails; save/reload and cups. July START currently phase 0 instead of actual date phase and needs separate fix.

### 0.13.45_08 — START già allineato a luglio (aggiornamento del 15/11)
- La nascita del mondo alla pressione NEW CAREER -> START importa e inizializza le leghe OG, quindi avanza i loro incontri originali alla data iniziale **1 luglio**, ancora prima della scelta squadra / disoccupazione. Gli incontri precedenti non vengono rigiocati al primo FORWARD TIME.
- Invariati: solo OG produce risultati; DEBUG WORLD è read-only; coppe residenti, storico fixture club, persistenza save mondiale completi ancora da terminare. Log di successo `[og-world] START READY`, errore `[og-world] START FAILED`. Sorgenti non compilati né provati su Windows.

### 15/11/2026 — prove Windows 0.13.46 e preparazione 0.13.47 (OG TEST DIAGNOSTICS)
- **TEST WINDOWS UTENTE 0.13.46 SI (percorso provato):** 01/07/2026 Svezia Allsvenskan OG residente già a 15 match/squadra senza FORWARD TIME; FORWARD TIME fino al 20/01/2027 senza riprodurre il crash di novembre; classifica dell'offerta e vista OG esterna/interna per Vigor Lamezia coerenti (11°, 20 partite, 25 punti) senza reset osservato. Questi test NON certificano tutte le leghe, tutti gli anni né save/reload.
- **NO/DA FARE:** coppe non residenti in DEBUG WORLD e dopo ingaggio, TOP GOAL SCORERS vuoto; coppe datate residenti, cronologia gare OG dopo firma e serializzazione globale ancora mancanti.
- **0.13.47 SORGENTE (DA COMPILARE SU WINDOWS):** F12 sostituisce i vecchi contatori LWE come indicatori di salute con lettura diretta degli stati residenti OG tramite `Expanded_OGWorld_Audit`, dichiarando separatamente `cup-definitions` e `cup-resident`. Log `[og-world-audit] stage=START/TICK` emesso da inizializzazione e avanzamento, senza creare nuove fixture.
- **Test .BAT:** `TEST_OG_WORLD.bat` e il vecchio collegamento `TEST_LIVING_WORLD.bat` eseguono soltanto controllo formule di date/fasi, identità fixture e definizioni OG; `ANALIZZA_SWOS_LOG.bat` verifica il reale SWOS.log della build corrente. `AVVIA_DIAGNOSTICA.bat` acquisisce log e F12/F12+C/F10; coppe non residenti significano INCOMPLETE, non PASS sintetico.
- **BACK DEBUG WORLD DISOCCUPATO:** nello stack originale il criterio `_topOfStackMark` può chiudere l'intero selettore; la 0.13.47 prepara override limitato alla chiamata di DEBUG WORLD: a profondità >0, BACK esegue un `PopContinent` singolo, al livello radice torna a UNEMPLOYED; log `[og-world-picker] BACK/POP`. Windows gameplay di questa modifica NON PROVATO.

### 15/11/2026 — prova nativa 70 stagioni progettata nella 0.13.47
- F12+8 da UNEMPLOYED prepara **70 anni**, un click reale FORWARD TIME percorre ogni chiusura `phase=56` e la successiva inizializzazione `phase=0` delle **leghe OG residenti**. `[og-season-transition] CLOSE/OPEN` registra numero delle leghe, partite OG giocate e contesti invalidi. L'audit C cerca 70 CLOSE, 70 OPEN e 70 anni civili esatti per `OG_70_NATIVE_SEASONS=PASS_TRANSITIONS_ONLY`.
- Per il salto diagnostico il callback di fine-anno delle coppe NON RESIDENTI non viene invocato a posteriori 70 volte: test coppe = NON PROVATO, non PASS. La regola non si applica ai FORWARD TIME normali; stato del mondo dopo un test di stress NON va considerato una carriera gestionale reale.
- `TEST_OG_70.bat` valuta il vero `SWOS.log`, senza simulazione propria. Stress anni multipli su Windows, prestazioni, eventuali crash e correttezza retrocessioni/promozioni: **NON PROVATI fino al log dell'utente**.
- Futuro: controlli dei pensionamenti giocatori/allenatore e sostituzioni/giovani, distinguendo gli eventi OG da estensioni LWE autorizzate. L'attuale salto disoccupato non li esercita.

### 15/11/2026 — test Windows 0.13.47 e intervento 0.13.48 (Windows non ancora provato)
- L'utente ha riportato SWOS.log 0.13.47: F12+8 iniziato il 01/07/2026, 24 chiusure e 24 riaperture OG, poi crash/interruzione durante l'elaborazione 2050 senza il marker FINISHED. I 24 controlli precedenti non provano il 70-year test, né identificano la causa nativa del crash.
- Falso OG_DATE_REWIND: nel log ci sono più nuove carriere; il nuovo START produce stage=TICK prima di START READY. L'analizzatore azzerava il cursore troppo tardi. Aggiunto marker og-career RESET all'inizio, con contatori per ultima carriera.
- 0.13.48: salto F12+8 spezzato in 70 step annuali OG, un step per iterazione MenuProc; confronto annuale col solo orologio civile, marcatori og-stress STEP e og-world-step BEGIN/END per paese/divisione; se fallisce si interrompe. Non è provato che ciò elimini un eventuale crash deterministico del motore.
- Nuovi checkpoint og-feature-audit prima/dopo stress e stagione: definizioni coppe contro contesti residenti, e contatori goalStats[0] dei calciatori OG confrontati ai GF. Test dedicato TEST_OG_CUPS_SCORERS.bat; coppe residenti assenti e menu TOP GOAL SCORERS vuoto restano da correggere.
- DEBUG WORLD disoccupato: la 0.13.47 tratta B come FIRE e non emetteva og-world-picker BACK. Nuovo routing B a BACK solo dentro il selettore, ESC/BACKSPACE equivalenti, pop di un livello alla volta.
- Schermata offerta: VIEW COMPETITIONS collega direttamente al paese/divisione/anno OG residente; controllo read-only di matchesPlayedCount e di contatori cup/scorer prima e dopo. Non accetta ingaggio né ricalcola competizioni.
- Windows compilation, 70 step, UI e controller della 0.13.48: NON PROVATI. Save/load del mondo, coppe residenti, marcatori UI e pensionamenti: NON COMPLETATI.

### 15/11/2026 — 0.13.49: blocco del BAT diagnostico prima della compilazione
- **Prova Windows utente 0.13.48 (NON COMPILATA):** `AVVIA_DIAGNOSTICA.bat` avvia `AVVIA_SWOS.bat` -> `PREPARA_SWOS.bat` -> `BUILD_SWOS.sh`; quest'ultimo si ferma in `sorgenti/tools/patch_title_b_01063.sh` per l'ancora testuale obsoleta `if (skip && !input_on_title_first_level() && SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_B))`. La condizione in `input_sdl.c` ora include su tre righe `!Expanded_UnemployedWorldPickerActive() &&` per distinguere B=BACK dal FIRE nel selettore. Il vecchio `grep -Fq` fallisce e stampa `patch controller incompleta`, *prima della compilazione C*. Questo log non costituisce crash di SWOS_Expanded.exe.
- **Correzione 0.13.49:** `patch_title_b_01063.sh` riconosce sia il vecchio guard monoriga, sia il nuovo guard contiguo su tre righe, senza riscrivere il B del DEBUG WORLD. Se non trova nessuna variante verificata, fallisce esplicitamente. Le altre patch richiamate dal builder sono state controllate nei rispettivi marker di attivazione.
- **Verifiche effettive:** script/parsing del guard provati su tre esempi separati (moderno: accettato; storico: accettato; guard incompleto: rifiutato). Non è una compilazione Windows; compilazione, avvio, gameplay, BACK pad e stress OG della 0.13.49 restano NON PROVATO. Coppe residenti e TOP GOAL SCORERS ancora mancanti.

### 15/11/2026 — 0.13.50 source implementation, Windows NOT TESTED
- Windows 0.13.49: native league stress completed 70/70 annual transitions to 01/07/2096 but crash followed upon opening menus. The stale unemployed end-year cup callback then attempted to replay cup seasons starting in 2026; cup journal capacity reached and cup-conflict messages occurred. The 70 leagues succeeded; the entire career is not certified.
- The new 0.13.50 resident OG national cup patch uses the EXISTING original functions InitDIYCupFromRecord, Career_InitCompetitionTeamSelectionFromTableHeaderSpec and Career_AutoAdvanceCupMatches. Each national type-1 cup is started at NEW CAREER START and moved only to the native phase reached by FORWARD TIME; other cup types/international formats remain pending.
- Scratch bracket, OG teams, tournament and RNG are preserved per country/slot/season. At season transition the prior resident cup state is discarded alongside leagues (full historic match records/save NOT implemented). All match outcomes use the native OG simulation, not LWE.
- The old Career_AdvanceUnemployedOGCups is suppressed whenever the resident OG world is active, preventing replay of old years after F12+8. The old bounded career cup journal is not called for resident cup results, so cumulative cup history and save are still incomplete.
- Data safety: stop at sentinel -1 on variable-length country tables; max 64 native bracket identities, unsupported definitions are explicit; resident cup viewer uses a scratch copy and blocks simulation.
- Managed club safety: cannot yet map resident cup identities into OG manager slot; that cup is tagged MANAGER_SLOT_UNBOUND and excluded for the season to prevent an extra independent fixture. This is deliberately INCOMPLETE for hire/fire continuity and must be addressed before declaring a single resident cup world.
- New OG_CUP_NATIVE_MATCHES test output reports contexts, matches, unsupported and faults. Compilation, gameplay on Windows, 70-year cups, cup read-only viewer, manager cups: NOT PROVATO. Other competitions unaffected by design, NOT PROVATO on Windows.

### 15/11/2026 — 0.13.50 verificata dall'utente; preparazione codice 0.13.51
- **0.13.50 Windows utente (dati forniti):** START OG 2026-07-01 PASS, 134 contesti nazionali/91 paesi, 70 CLOSE+OPEN fino al 2096 con 0 errori nativi nel report; le coppe sono visibili dopo 70 anni da disoccupato e da occupato, ma non e' stata provata la coppa del club allenato. Campionato prima/dopo ingaggio invariato visivamente. Non dimostrato save/reload completo; suite automatica precedente bloccata anche da errore Bash e Werror su unit inutilizzate.
- **Problemi riferiti:** BACK non affidabile; TOP GOAL SCORERS mostra marcatori prima dell'ingaggio e non dopo; offerta VIEW COMPETITIONS apriva la divisione direttamente anziche' la lista delle competizioni del club; diagnostica frammentata; 101 definizioni nazionali type-1 e 31 residenti, con scarti della selezione OG da spiegare puntualmente.
- **0.13.51 codice preparato:** riuso degli originali dati goalStats dei TeamFileEx della lega residente in TOP GOAL SCORERS e relative schermate; BACK del picker e viewer un livello alla volta; offer VIEW COMPETITIONS apre selettore OG gia' posizionato sul paese del club, filtrato su divisione e coppe confermate, poi viewer read-only e ritorno all'elenco; nuovo log per ciascuna coppa nazionale type-1; runner AVVIA_DIAGNOSTICA include compilazione, runtime, statici, 70 stagioni e coppe-marcatori in RISULTATI_DIAGNOSTICI/DIAGNOSTICA_SESSIONE_N.
- **Vincolo:** START crea/avanza mondo OG; accettare/cambiare club NON e' autorizzazione a rifare le partite o i `goalStats`; il menu puo' usare scratch locale ma non deve costituire una seconda autorita'. Nessuna simulazione autonoma LWE aggiunta.
- **Limiti:** diagnosi coppe attualmente solo type-1 nazionali slot 2/3; internazionale/type-0 non testato, manager-cup bind non risolto, storico gare/coppe non persistito, save/reload OG completo e pensionamenti non verificati. Il reader dei marcatori e la navigazione richiedono Windows, incluso controllo dopo partite del manager. NON dichiarare il motore terminato.

- **Raccolta storica isolata (0.13.51):** i precedenti test world LWE di archivio restano disponibili in `OG_LEGACY_SUITE.log` eseguiti come binari C separati, ma NON sono il motore di gioco e i loro esiti non certificano la carriera OG. Le compilazioni legacy sotto `-Werror` possono ancora fallire per `live_roll_window`/`live_advance_with_mobility` non usate; non sopprimere arbitrariamente l'errore né chiamare la partita fallita per questa ragione. `run_test11_offer.sh` e' esplicitamente NOT_RUN per sanitizer opzionale e percorso /tmp fisso. Non e' stato introdotto un nuovo ambiente di compilazione.

### 15/11/2026 — 0.13.52 correzione build e runner ALL TESTS (Windows da provare)
- Nel test Windows della **0.13.51**, `DIAGNOSTICA_SESSIONE_2(1).zip` mostra compilazione fermata in `teams.c:3622`: prototipo file-scope mancante per `Expanded_UnemployedWorldPickerActive`. Nessun EXE 0.13.51 compilato/giocato in quella sessione. Primo problema reale diverso dai vecchi errori LWE sotto `-Werror`.
- Corretto prototipo con guard SDL; versione aggiornata a 0.13.52. Nessuna modifica a risultati/fixture/engine OG o ai salvataggi. Build 0.13.52 Windows NON PROVATA.
- Diagnostica ora archivia sempre il vecchio SWOS.log nella sessione PRECEDENTE e legge SOLO il log generato dal nuovo avvio. Se il compile abortisce, non mostra falsi risultati runtime di una versione precedente.
- Un unico avvio `AVVIA_DIAGNOSTICA.bat` effettua compilation+avvio quando possibile, esegue TUTTI gli script `run_test*.sh` (compresi i vecchi e test11 offerte), il format audit, log individuali, rapporti 70 anni/coppe-marcatori e ZIP automatico verificato; i wrapper alias non sono test diversi. I PASS dei vecchi harness sintetici non costituiscono prova del mondo OG. Vecchi FAIL restano leggibili e non falsificano esito OG.
- Stato motore invariato rispetto a 0.13.51: marcatori dopo bind, navigazione BACK e offerta competizioni da verificare in Windows; coppe internazionali e del manager, save residente completo NON PROVATI.

### 15/11/2026 — preparazione 0.13.53: VALIDAZIONE OG, non correzione engine
- Test utente 0.13.52 su Windows: compilazione/avvio, 70 OPEN/CLOSE fino al 2096, marcatori dopo ingresso, BACK a livelli, VIEW COMPETITIONS dichiarati funzionanti. Dopo save/load l'utente riferisce di essere tornato nel **2096**: data confermata dall'osservazione, ma serializzazione integrale di `s_expOG` e `s_expOGCups` NON PROVATA.
- Nuovo check OG dei bye: selezionati veri vs posti del tabellone, entranti/turni da `World_Cups()`, riscontro di `totalMatchesInRound` originale; nessun pass conferito a una definizione soltanto perche' ha bye possibili. Coppe con `SELECTOR_UNDERFILLED` ancora da distinguere caso per caso.
- Nuovo inventario read-only competizioni delle nazionali e internazionali: definizioni/qualificazioni/finali e stati UEFA/continentali/nazionali, marcati NON PROVATI come tornei residenti, datati e completi. Non istanziate competizioni sostitutive.
- Nuovi checkpoint log OG dopo SAVE e RESTORE: verificano se i contesti residenti sono presenti, senza prendere il solo anno corrente o vecchi .CAR per equivalenza dei risultati. Non si toccano i salvataggi preesistenti.
- In suite attiva eliminati i test obsoleti del LWE (solo esclusi dal runner, fonti conservate in Git); warning del compilatore contati in `COMPILER_WARNING_AUDIT.log`. Dei 29 warning precedenti: chiariti 3 indentazione e 1 parentesi di inizializzazione; i 25 `packed-member` sono da valutare singolarmente, non silenziati.
- **Stato:** codice e diagnostica 0.13.53 preparati; compilazione e gameplay Windows della nuova versione NON PROVATI. Coppe 70 mancanti, prime fasi e qualificazioni OG, competizioni internazionali/nazionali e save integrale richiedono controlli sul gioco. Non dichiarare il motore chiuso sulla base del report delle definizioni.

### 15/11/2026 — 0.13.54: fix coppe OG con bye, risultati Windows da provare
- Test utente 0.13.53 (`DIAGNOSTICA_SESSIONE_4(2).zip`): 101 definizioni type1/domestic/slots2-3 in 87 paesi, 31 cup context resident e 70 cup context rifiutati in 64 paesi. L'audit REAL OG aveva gia' dimostrato che tutti e 70 i casi rispettavano esattamente i conti nativi di bye, slot e primi turni. I 70 check numerici C isolati sono tutti PASS, MA non equivalgono a 70 coppe giocate.
- Causa: `base.numTeams <= _numSelectedTeams` nel validatore residente rifiutava i bye OG legittimi. Introduzione di `ExpandedOGCup_NativeByeLayout` e verifica stretta anche di ID squadra, coppie reali e bye dalla struttura OG e dalle regole `World_Cups()`. Se validi, conservare la stessa coppa OG residente, disponibile subito a START senza simulare gare non giocate. Nessuna modifica a fisica/fixture/sorteggi/risultati.
- Audit `OG_CUP_START_COVERAGE`, `OG_CUP_COUNTRY`, `OG_CUP_COUNTRIES` e log per ogni paese; il numero di definizioni e residenti deve corrispondere senza abbassare i criteri di PASS. Test C/helper e fixture parser nuovi sotto AVVIA_DIAGNOSTICA unica, ZIP sempre automatico.
- Stato prova: sorgenti 0.13.54 preparati; **Windows compilazione/partita/F12/70 anni NON PROVATI** per 0.13.54. Le 101 coppe residenti non sono ancora dimostrate, neppure i risultati delle gare successive. Altri punti pendenti indipendenti: coppe internazionali/nazionali, cup manager bind, save/resume stato residente, vecchio counter `g_worldCups.seasons` rimasto 0 dopo 70 anni nel precedente log; no reset o doppia evoluzione.
- Offerta NAZIONALE come PRIMA offerta di test richiesta dall'utente: NON inclusa in 0.13.54. L'attuale `Expanded_WaitForFirstOffer` in `expanded_career.c` verifica `offer_is_valid()` e un club caricato da TEAM file, poi `Career_AttachManagerToClub` chiama `InitCareerClub`. Un ID di nazionale in quel percorso non e' compatibile. La funzione OG `Career_MaybeGenerateJobOffer` in `job_offers.c` offre gia' nazionali attraverso un'altra catena. Richiede variante test esplicita del percorso nazionale OG, senza convertire un'offerta club e senza rischio per gli autosave; da affrontare a parte.

### 15/11/2026 - esito Windows 0.13.54 e intervento 0.13.55 (test Windows pendenti)
- DIAGNOSTICA_SESSIONE_5.zip, Windows 0.13.54, mondo 2026/27: 70 CLOSE/OPEN fino al 2096, 134 leghe residenti in 91 paesi, 101 coppe domestiche type1 in 87 paesi (70 casi native bye, zero divergenti o cup faults nello scope), contatori marcatori OG osservati. L'utente riferisce due riavvii completi dell'EXE con save e dati visivamente invariati: prova manuale positiva. Il vecchio report RESTORE_AFTER=0 vale solo per quel log, non smentisce il test svolto dall'utente.
- Nuovo codice 0.13.55: al ritorno al TITLE, dopo autosave, log di preflight dello stesso CAR su disco (solo schema, kind, mondo). BAT unico include copie di AUTO9697.CAR, AUTO2627.CAR ed eventuale AUTOSAVE.CAR con manifest, confronto binario ed eventuale hash SHA256. Il contenuto del CAR potra' contenere dati del manager e va condiviso soltanto in privato.
- Corretto il filtro VIEW COMPETITIONS dell'offerta: le coppe domestiche OG residenti nel paese sono consultabili anche se la squadra del contratto non e' entrante; stato iscrizione OG invariato. Usa la stessa vista residente DEBUG WORLD; BACK invariato.
- Inventario per nome delle tre coppe UEFA moderne o storiche, Copa Libertadores, altre confederazioni, Mondiali, Europei e tornei/qualificazioni delle nazionali. Nel log 0.13.54 UEFA clubs=0, conti season=0, worldcup season counter=0 durante lo stress: l'esistenza di definizioni NON equivale a calendario e match, e i nuovi marker NON creano partite.
- Ancora da verificare prima di chiamare COMPLETO l'intero motore: incontri residenti datati di internazionali e nazionali, save/ripresa integrale dei vettori OG, fixture IDs, RNG, manager handoff di coppa, test Windows 1996/97. Autosave mensile da disoccupato e' una proposta non implementata nella 0.13.55. Utente compila/testa Windows, nessun EXE costruito dall'assistente.


### 16/11/2026 — perimetro build 0.13.56 (Windows ancora da compilare/testare)
- Fonte più recente controllata: Mod 2026/27 **v4.1.2**, 91 paesi, 134 divisioni, 101 coppe domestiche, 13 statali brasiliani, Coppa Italia Serie D, 15 coppe continentali per club e 10 Supercoppe. Il manifesto tecnico conta 264 voci più 10 Supercoppe; gli autori pubblicizzano 260 competizioni, discrepanza aperta. Nomi e confronto vanno nelle guide `docs/WORLD_2627_MOD_4_1_2_INVENTORY.md` e `docs/WORLD_2627_EXPANDED_COVERAGE.md` con riferimenti TSV in `docs/reference/`.
- Codice 0.13.56 proposto: anteprima dell'offerta ora in `_squadMenu` (stesso renderer Expanded con stelle) su working copy read-only, interdizione di azioni gestionali; BAT `PULISCI_OBSOLETI_SWOS.bat` esplicito e sicuro; test statico delle competizioni in `AVVIA_DIAGNOSTICA.bat`. I vecchi VIEW SQUAD OG sono mantenuti perché condivisi con CLASSIC/DIY/SEASON.
- L'audit START dei 12 tornei extra-UEFA stampa i loro nomi OG individuali, ma **il nome NON è uno stato residente né un calendario**. `world_conti.c` separa tramite API additiva `ct_world_prepare` e `ct_world_advance_month` l'accesso/sorteggio OG dal singolo avanzamento mensile OG, mantenendo `ct_season` legacy invariato. **Il nuovo adattatore del mondo, i partecipanti 2026, i risultati per fixture, il viewer, il collegamento manager e il save/load NON sono collegati**: non dichiarare risolte le coppe internazionali. Prima di usarlo serve adattamento ai dati iniziali e al calendario OG.
- **APPROVATO, NON IMPLEMENTATO** `INIZIA COME CT`, nazionali direttamente a START mantenendo OG convocazioni/qualificazioni/offerte/evoluzione. Mod 4.1.2 non contiene ancora le nazionali 2026/27 (roadmap 5.0); nel mondo 1996/97 il percorso OG nazionale esiste. Ogni altra versione SWOS in sviluppo richiederà manifest e compatibilità dedicati.
- Nessun eseguibile Windows compilato o gameplay verificato dall'assistente in questa consegna. Dopo GitKraken eseguire solo `AVVIA_DIAGNOSTICA.bat`, poi controllare salvataggi e log. Non cancellare save, stadi o asset ancora referenziati.


## STATO 16/11/2026 — correzione 0.13.62 in attesa collaudo Windows

Ramo `dev/0.10`, base 0.13.61 commit `b2dc8404e51fef32f4b091709ac12bf7564d4ca3`. La diagnostica Windows SESSIONE 15 ha **COMPILAZIONE FALLITA** (inclusione accidentale di `competition_data.h` in due moduli con `extern uint8_t[]` incompatibili); nessun gameplay della 0.13.61 testato in quella sessione. La versione 0.13.62 corregge staticamente quelle dichiarazioni, ripara BACK persistente nei viewer OG annidati e impedisce di saltare in finale UEFA senza completare le giornate native. Aggiunge diagnostica 96/97 START, ma **il crash riferito NON E' ancora risolto**, né sono implementati calendario/viewer residenti completi delle 12 continentali o qualificazioni UEFA negli anni successivi. La build e la verifica Windows spettano al proprietario dopo sync GitKraken. Un commit unico autorizzato; nessun save modificato/cancellato.

### 16/11/2026 — 0.13.63, sessione diagnostica 16
Base dev/0.10 0.13.62 0b89f0f. Sessione 16 = preflight stop B/SKIP prima di GCC, NON crash dell'EXE. Corretto riconoscimento B-held e priorita' del viewer/squad sul picker, corretto CRLF nell'audit 2627 e due ancore obsolete nell'audit UEFA. Nessun cambiamento a fisica, partita, fixture OG, save o asset. Versione 0.13.63, compilazione Windows e gameplay NON PROVATI. START 1996/97, UEFA fasi/anni e dodici continentali extra-UEFA restano pendenti.

### 16/11/2026 — 0.13.64 prepared from Windows session 18 and original Mod 2026 screenshots
Branch dev/0.10, parent 0.13.63 e4a15412. The 1996/97 TEAM loader and TeamDb identity source now use original native TEAM.NNN regardless of global JSON setting; 2026/27 remains on modern JSON and logs any legacy fallback for audit. Original record sizes are not modified. Expanded-only country/competition picker list navigation wraps UP/DOWN at both ends; pad B unchanged, CLASSIC unaffected. Removed the entire runtime dependency on the Mod's ENGLISH.EXE: no bundled EXE or S1 football-data blob. First-edition CONMEBOL club-to-ID mapping and dated OG cups remain NOT_RESIDENT, not silently simulated. Repaired the duplicate/malformed 2026/27 coverage shell script. Compiling SWOS_Expanded.exe, Windows start 1996, viewer/play, and full Mod 2026 competition formats are NOT TESTED in this environment. Save files and title/menu assets untouched.


### 17/11/2026 — Decisioni e verifica Libertadores per entrambi i mondi (0.13.65, PARZIALE)
**Regola approvata:** date reali per tutte le competizioni di **entrambi** i mondi 1996/97 e 2026/27. La carriera cambia stagione il 1 luglio, ma un'edizione OG annuale può proseguire fino ad agosto o novembre. Formato storico Libertadores: 20 club, 5x4; formato 2027: 47 iscritti, 28 dirette, 19 preliminari, quattro posti gironi liberi fino a Q3. Fra i 32 club dello screenshot sono spostati a preliminari Alianza Lima, The Strongest, Juventud, Deportivo Táchira; altre quindici squadre dai rispettivi paesi completano la lista. **Nomi, source nazionale e ID OG di tutti i 47, date ufficiali 2027 e limiti** sono in `docs/LIBERTADORES_1997_2027_DECISIONS.md` e nel manifest `data_json/worlds/2627/competitions/libertadores_2027_first.json`.
**Codice 0.13.65:** reset della stagione della lega distinto dal reset totale, conservazione dello stato CT_ED attivo; lettore read-only delle date-base per singolo leg 2027. **NON ANCORA IMPLEMENTATO:** fixture reali per squadra con risultato/andata-ritorno persistente, esecutore OG giornaliero, sorteggi e save/load integrale, e collegamento completo delle due Libertadores. I dati NON sono partite giocate. Windows compilazione e gameplay NON PROVATI.



### 17/11/2026 — 0.13.66, priorità riprese dopo sessioni Windows 21/22
La 0.13.64 aveva già isolato la lettura 96/97 nei binari OG, ma la sessione 22 della 0.13.65 prova un mismatch **nel binario stesso** della Finlandia TEAM.012: file 16, league0 14, formato nativo 12, panic 0x8885. Nuova preflight rileva il mismatch PRIMA del selettore nativo e marca la lega NON_RESIDENT invece di generare un risultato inventato; non risolve quali due club escludere. Picker DEBUG WORLD/OFFERTA naviga tra le entry visibili senza gap e ridisegna il focus. B nell'offerta VIEW SQUAD chiude prima popup X. Tutte le dieci priorità, compresi NO RESTART, differenze regolamenti, diagnostica e Windows TEST NON PROVATO, sono inventariate in `docs/WORLD_PRIORITY_20261117_BUILD_01366.md`. Niente commit a main/release, nessun save/asset modificato.


### 17 novembre 2026 — 0.13.67: NO RESTART e doppio static namespace preparato al loading (Windows NON PROVATO)
Decisione già approvata e ora implementata nel codice del selettore NEW CAREER: preparare i registri statici di **entrambi** i mondi durante il LOADING, ma mantenere **un solo insieme di puntatori OG attivo**; nessuna partita simulata durante la selezione. `world_switch.c` salva 290 aree dei record/tabelle native prima della sovrascrittura e dopo l'installazione completa dei dati moderni; include offsets squadra, paese, competizioni, playoff, regioni e `_careerContests`. `competition_data.c` aggiunge restore del registro compId per non mescolare i template dinamici moderni con i 1996/97. Il switch UI usa `WorldSwitch_SelectBeforeCareer`, invalida la cache TEAM, non chiama più CreateProcess/exit e mantiene salvataggi di mondi distinti. Modello confrontato con MOD 4.1.2 (`careerworld.py::w_ns_set`, `swe.c::reg_enter/reg_leave`); Expanded deve salvare più registri dei soli tre della MOD. Guida/audit dettagliati in `docs/WORLD_SWITCH_NO_RESTART_20261117.md`. **Compilazione e gameplay Windows NON PROVATI; non equivale a completamento mondiale OG, né a risoluzione della Finlandia 96/97.**
