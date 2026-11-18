# SWOS Expanded — development procedures and rationale

## 0.13.87 — verifica Windows TRAINING nomi A/B e avviso lampeggiante (PENDENTE)
- Sincronizza GitKraken su `dev/0.10`, compila personalmente Windows 0.13.87, conservando copie esterne non distruttive di `AUTO9697.CAR` e `AUTO2627.CAR`.
- Apri TRAINING di un club con abbastanza giocatori: prima schermata **nome club A**, pulsante `VIEW B TEAM`; dopo averlo premuto, **nome club B**, pulsante `VIEW A TEAM`. Ripeti dopo X A->B, X B->A, BACK e variazione della formazione. Nomi e rose devono riferirsi al lato visualizzato realmente.
- `VIEW B TEAM` puo' lampeggiare quando il controllo OG delle riserve B non e' soddisfatto: verifica se cambia dopo aver sistemato gli 11 titolari e le riserve. Non segnare `flicker grafico` solo per il lampeggio della scritta, ma segnala qualsiasi pannello/sprite che salti o si sposti.
- Per X: primo evento = `SOURCE ... WAITING_TARGET`, secondo sul lato opposto = `VERIFIED ... status=OK`. Confronta *nominativi* sulle medesime righe e formazione ufficiale dopo TRAINING; salva, chiudi, CONTINUE e verifica anche il CAR. Se compare `FAILED_INVARIANT`, `B_COPY_NOT_FOUND`, `NOT_IN_SAVED_LINEUP` o un artefatto, allega la nuova sessione di `AVVIA_DIAGNOSTICA` con screenshot.
- Non considerare superati i test 1996/97, 2026/27, gioco reale, pad, save/restore e 0.13.86 solo grazie a revisione dei sorgenti. Build Windows e gameplay di 0.13.87: NON PROVATI.


## 0.13.86 — test manuale Windows X e grafica del TRAINING (PENDENTE)
- Sincronizza `dev/0.10` e compila 0.13.86 su Windows. Conserva backup non distruttivi `AUTO9697.CAR` e `AUTO2627.CAR`, senza cancellare il vecchio save. Carica il Newport County (o un club con almeno 17 giocatori) e apri TRAINING.
- A->B: evidenzia un titolare A con nome annotato, X (deve aprirsi B), poi X su una RIGA B con nome annotato. Ora il giocatore A deve occupare **la stessa riga B selezionata**; passando ad A, il giocatore B deve essere nella riga A originale. Nel log cerca `[training-cross] VERIFIED`, due file ID scambiati e `B-copies=1` (o 2 se entrambi i posti appartenevano a B).
- B->A: ripeti partendo dalla B. Premere X molte volte e cambiare schermata: entry 12/29/30, pulsanti e campo non devono traslare, diventare artefatti o scomparire. Controlla anche le selezioni OG ordinarie con A e la navigazione B/BACK, X INFO della VIEW SQUAD normale.
- Esci da TRAINING senza giocare, verifica in SQUAD/partita successiva che il giocatore promosso sia presente: `official-match=UPDATED` significa che il codice ha aggiornato lo snapshot, NON prova da solo la persistenza nel CAR. Salva, riavvia, CONTINUE e verifica prima/dopo in entrambe le epoche. Invia log e screenshot di eventuali anomalie.
- Altri `error` compilazione, `FAILED_INVARIANT`, `B_COPY_NOT_FOUND` o `official-match=NOT_IN_SAVED_LINEUP` vanno segnalati separatamente; non segnare automaticamente PASS perche' compare `VERIFIED` sullo swap canonico.


## 0.13.85 — riprova compilazione Windows (PENDENTE)
- Sincronizzare GitKraken `dev/0.10`, poi compilare con i BAT ufficiali; controllare versione 0.13.85.
- Leggere `diagnostica/log/COMPILAZIONE_0140.log`. Non deve ricomparire `career_init.c:952:47: error: '_gameType' undeclared`; se emergono altri `error:`, inviare i log e non dichiarare compilazione PASS.
- Se la build funziona: avviare il gioco, provare TITLE 96/26 dopo BACK, TRAINING A->B/B->A con X nei mondi 96 e 26, quindi save/CONTINUE. Non cancellare ne' sostituire CAR esistenti.
- I 13 test OG della diagnostica allegata erano solo isolati. Compilazione, gameplay e persistenza Windows restano NON PROVATI fino al nuovo test.


## 0.13.84 — prove Windows: X scambio TRAINING A/B (PENDENTI)
- Backup esterno non distruttivo dei CAR, sincronizzare GitKraken dev/0.10, compilare 0.13.84 su Windows.
- Carriera Gela 26 con 27 giocatori, SQUAD -> TRAINING prima della partita. A -> X sul titolare -> passaggio automatico a B -> X su giocatore B: log `[training-cross] SOURCE side=A`, poi `SWAPPED`. Tornare A e verificare che il B scelto sia al posto dell'A, mentre l'altro risulti nella B, senza doppioni. Ripetere partendo da B -> X -> A -> X.
- Provare normale A/SELECT due volte all'interno di A e poi B (funzione preesistente), X su riga non giocatore, X sullo stesso giocatore, tastiera X, Switch Pro X, focus, B/ESC e popup X INFO nel normale VIEW SQUAD.
- Uscire da TRAINING, verificare la formazione nella carriera, salvare, riavviare, CONTINUE e confrontare giocatori e posizioni. Ripetere almeno in 1996/97, inclusi giocatori indisponibili. Inviare log `[training-cross]` e anomalie: nessun PASS Windows e' attestato da questo commit.


## 0.13.83 — prova Windows: TITLE WORLD 96/26 (PENDENTE)
- Backup dei tre salvataggi CAR in posizione esterna, sincronizzare GitKraken e compilare 0.13.83.
- Da 26 -> NEW CAREER -> START -> CHOOSE TEAM -> BACK: tornare al TITLE, cercare `CANCELLED_SETUP_RESET reason=team-selection-back game-type=0` e cambiare 96/26 senza riavvio.
- Da 26 -> NEW CAREER -> START UNEMPLOYED -> BACK: tornare al TITLE, cercare `reason=unemployed-back game-type=0`, cambiare 96/26; ripetere nei due mondi.
- Da carriera Gela 26 con contratto -> TITLE: richiedere `[autosave-title] status=COMPATIBLE_ON_DISK`, poi `[title-world] DETACH_CAREER` e `SELECTED 9697`; tornare a 26 e CONTINUE, verificando data, nomi EXTRA e competizioni OG. Ripetere da 96 a 26. Provare mouse e pad A.
- In caso di `WRITE_FAILED`, `CHECK_FAILED`, `SKIPPED` o `checkpoint=NOT_VERIFIED` di una carriera attiva, il cambio resta bloccato intenzionalmente: non bypassare il CAR, inviare log diagnostico nuovo.


## 0.13.82 — prova Windows correzione autosave con EXTRA (PENDENTE)
- Sincronizzare GitKraken su `dev/0.10`; eseguire backup dei CAR in cartella esterna, **non eliminare** quelli preesistenti. Compilare su Windows con lo strumento gia' in uso.
- Nuova carriera 2026/27 di prova con Gela, oppure club con 11 EXTRA: VIEW SQUAD deve indicare 27 giocatori. Firmare, avanzare, uscire al TITLE. Nel log controllare `[autosave-stage] ... SERIALIZED_COMPLETE`, `READY_FOR_ATOMIC_SAVE`, `[autosave-title] status=COMPATIBLE_ON_DISK`; assenza di `MISSING_CIVIL_CLOCK` e `PREWRITE_REJECTED`. Verificare dimensione/data del CAR e checkpoint **prima** di considerare valido il test.
- Chiudere il processo, riavviare e CONTINUE 2026/27: confrontare data esatta, rosa 27 e nomi EXTRA, campionati/coppe residenti, risultati e stato manager; fare un altro save e reload. Provare vendita di un EXTRA + cambio club e controllare che non ricompaia alla squadra originaria. Questi test non sono coperti dall'analisi statica.
- Ripetere una carriera 1996/97 di prova con save separato e senza accesso agli EXTRA moderni. Confermare che AUTO9697.CAR e il vecchio AUTOSAVE.CAR non subiscano modifiche impreviste.
- Il 26/27 ha ancora 26 competizioni segnate NON RESIDENTI; coppe continentali e 5 conflitti vincitori 1996/97 rimangono aperti e non devono risultare PASS solo grazie al salvataggio corretto.


## 0.13.81 — test rose reali 2026/27 (Windows utente, NON PROVATO)
- Backup non distruttivo di `save/AUTO2627.CAR` e `save/AUTO9697.CAR` in una cartella fuori dal gioco.
- Estrarre il pacchetto additivo con `diagnostica/INSTALLA_ROSE_2627.bat` e aprire una nuova carriera 2026/27 con un club del pacchetto. VIEW SQUAD deve vedere i nomi del 17º..30º e devono sparire le 10 riserve inventate quando sono presenti extra reali. Non dichiarare 30 per ogni club: alcuni ne hanno meno.
- Provare il mercato nazionale e un acquisto di extra, poi cambio club, salvataggio, chiusura e CONTINUE; confrontare nomi, dati e la mancata ricomparsa presso il club d'origine. Provare anche un club senza extra (fallback 10 inventati).
- Ripetere nuova carriera 1996/97: solo i 16 OG + 10 generati; nessun accesso ai dati 2026. Verificare salvataggi separati, non sovrascrivere i CAR reali.
- B-team, CPU autonomous replacements e pagina 2 manuale dei club esterni: NON IMPLEMENTATI, non dichiarare PASS. I test Windows devono essere provati con screenshot/log e il codice può richiedere correzioni prima del rilascio.


## 17/11/2026 — prova Windows 0.13.80: WORLD 96/26 sul TITLE
- GitKraken su dev/0.10, compilazione personale Windows 0.13.80. Prima eseguire backup esterno non distruttivo di AUTO9697.CAR e AUTO2627.CAR. START 96/97 o 26/27 con CAR di PROVA, avanzare un evento, tornare al TITLE: il log deve riportare `[autosave-title] status=COMPATIBLE_ON_DISK`; premere 96/26 con mouse e Switch Pro A. Attesi `[title-world] DETACH_CAREER`, `SELECTED`, preferenza SWOS.ini aggiornata, colore/focus normale e stato del pulsante coerente col nuovo mondo.
- Ritornare all'altro mondo dal TITLE e usare CONTINUE: verificare che data, calendario OG, risultati e CAR iniziale siano effettivamente ripristinati, non soltanto che la voce 96/26 cambi visivamente. Ripetere per entrambi i CAR; verifica la conservazione dei file via timestamp/checksum esterno, senza cancellare file reali.
- Caso negativo: carriera con `menu-exit` WRITE_FAILED o nessun CAR valido: click 96/26 deve registrare `checkpoint=NOT_VERIFIED`, non distruggere lo stato vivo e non commutare bank. Il blocco e' intenzionale; non considerare il salvataggio vecchio sufficiente. La sicurezza delle transizioni, il controller/mouse, il gameplay e il load Windows sono PENDENTI fino alla prova dell'utente. Non confondere con DEBUG NEW CAREER, che ha callback diversa.


## 17/11/2026 — checklist Windows 0.13.79 (DEBUG 30 JUNE)
- GitKraken dev/0.10 e compilazione personale Windows 0.13.79; usare solo AUTO9697/AUTO2627 di **prova**, senza sovrascrivere i CAR importanti. CALENDAR con club: assenza del DEBUG +1 MONTH sopra TRANSFERS e layout/joypad precedente intatti.
- START UNEMPLOYED 96 e 26: verificare che DEBUG +1 MONTH sia ancora presente e che DEBUG 30 JUNE sia sotto di esso, senza hotspot fantasma o focus bloccati. A/SELECT, mouse e B/BACK. Da luglio/novembre saltare una volta: esattamente 30 giugno seguente, eventi/coppe OG fino al cutoff, F12 data/settimana, log `[debug-june30] OG_ADVANCE_BEGIN` poi `OG_DATE_COMMITTED`. Un secondo click non cambia la data e scrive `STOP_AT_SEASON_BOUNDARY`.
- Eseguire FORWARD TIME standard dal 30 giugno e verificare separatamente il rollover una sola volta; confrontare risultati/stagione e SAVE/CONTINUE nei due mondi. In caso di FAIL del tick OG, non riprovare o salvare lo stato parziale: inviare ZIP AVVIA_DIAGNOSTICA. Build Windows, gameplay, input, competizioni globali, snapshot CAR NON PROVATI dall'assistente.


## 17/11/2026 — guida all'uso della matrice comparativa (solo documenti)
- Aprire `docs/reference/SWOS_CROSS_VERSION_COMPETITIONS_20261117.tsv` con editor TSV, filtrare `mondo=9697/2627`, `tipo`, `paese`, `esito`; per ogni record consultare `docs/SWOS_CROSS_VERSION_COMPETITION_AUDIT_20261117.md`. Righe `NV` = non verificato dal materiale indipendente, NON assenza; `PRESENTE MA INCOMPLETA` = record/manifest senza catena mondiale OG pienamente collaudata. Non sommare le 494 righe (duplicazioni fra fonti/epoche) come numero tornei.
- Verificare, su copie non distruttive delle distribuzioni originali, per SWOS2020, DOS/GOG 96, OpenSWOS versione precisa, Mod 1997 e Mod 4.1.2: (1) hash/versione dataset, (2) presenza nel MENU CAREER, (3) presenza nel PRESET, (4) numero squadre e IDs, (5) fasi e calendario, (6) promozioni/retrocessioni, (7) qualificazioni e rollover, (8) save/restore. Non inserire EXE o dati calcistici terzi nel repo per creare dipendenze. Se dati assenti scrivere NV; usare un esito runtime solo con screenshot/log Windows.
- Anomalie principali da triagiare DOPO decisione utente: differenza delle due fasi Libertadores OpenSWOS vs Expanded 96; 8 valori numerici promozioni e 13 retrocessioni moderni, possibili corrispondenze playoff/play-out, non bug automaticamente. Documentare la fonte di ogni comparazione prima di modificare engine o records. **Solo commit documentale**, `build_info.h` invariato 0.13.78, Windows compilazione/test gameplay non pertinenti.


## 17/11/2026 — prova 0.13.78 Windows: DEBUG +1 MONTH direttamente via OG
- Prima GitKraken su `dev/0.10`, poi compilazione personale Windows 0.13.78; backup di `save/AUTO9697.CAR` e `save/AUTO2627.CAR`. Usare carriera DI PROVA nuova nel 96 e nel 26, club con contesto OG bound: nel CALENDAR premere DEBUG +1 MONTH. Atteso un solo `[debug-month-career] ENGINE_BEGIN` e `ENGINE_COMMITTED`, crescita partita OG via F12/debug world, data civile esatta del mese successivo, nessuna schermata PLAY/VIEW RESULT e nessun cambio dei controlli squadra.
- Provare ottobre->novembre, 31 gennaio->28/29 febbraio, maggio->30 giugno, poi clic da 30 giugno: `STOP_BEFORE_SEASON_CHANGE` senza 1 luglio. Controllare che il normale `FINISH SEASON` sia manualmente raggiungibile e cambi la stagione secondo OG. Verificare con mouse, Switch Pro A/B e senza mouse move, eventuali blocchi da `[debug-month-career] BLOCKED`, save -> riavvio -> CONTINUE, classifica del club, contatori partite prima/dopo, niente duplicazioni. Ripetere se manager gia' disputato un match: se il locale non coincide col residente, il salto deve essere BLOCCATO anziche' cancellare la partita.
- Non segnare PASS per tutti i risultati del calendario manager: lo storico per fixture nel keycache NON e' ancora mappato dal mondo OG, ne' sono riprodotti i movimenti economici/status del manager. In caso di `FAILED ... do-not-save-partial-world`, NON ripetere lo stesso salto o salvare: diagnosticare la carriera di prova con AVVIA_DIAGNOSTICA e inviare ZIP/log. L'isolato C copre solo date/fasi, non build Windows o avanzamento reale del motore.


## 17/11/2026 — test Windows 0.13.77, ultimo mondo scelto
- Copiare senza cancellare `SWOS.ini`, `save/AUTO9697.CAR`, `save/AUTO2627.CAR` e legacy `AUTOSAVE.CAR`; GitKraken `dev/0.10` e build `0.13.77`. Da TITLE fresco premere il piccolo 96/26 in alto a destra (mouse e Switch Pro A), verificare cambio immediato e `[world-preference] STORED world=9697/2627`; cambiare anche in NEW CAREER, tornare a TITLE, verificare l'ultimo valore.
- Chiudere completamente, riaprire: senza schermate aggiuntive il primo TITLE deve presentare esattamente l'ultimo mondo scelto, e `SWOS.ini [general] world` deve concordare. Ripetere 96->26->96. Provare anche B/ESC, focus circolare verso il controllo indice 7 e clic solo nella hitbox 187x64. Da un TITLE con carriera/CLASSIC effettivamente attiva la pressione deve rifiutare la commutazione con `[title-world] REFUSED`: non perdere stato o cambiare l'anno della carriera.
- Con entrambi i CAR disponibili verificare che CONTINUE legga solo quello del mondo visualizzato (non cancellare o sovrascrivere quello dell'altro); dopo interruzione/riavvio il mondo deve restare selezionato. Se `[world-preference] WRITE_UNCONFIRMED`, allegare log, permessi di SWOS.ini e contenuto della sola riga world. Non segnare PASS per la scrittura Windows, GL/SDL o il reload senza osservazione reale.


## 17/11/2026 — prova Windows 0.13.76 dopo rollover
- Backup dei `save/AUTO9697.CAR`, `save/AUTO2627.CAR` e `save/AUTOSAVE.CAR` prima di test; non cancellare o sovrascrivere file importanti. Sincronizzare GitKraken ramo `dev/0.10`, compilare 0.13.76. Carriera di prova disoccupato 9697: vedere risultati Champions, CWC, UEFA, Libertadores nel primo anno; a giugno annotare club/gironi/finali/gare, eseguire regolare FORWARD TIME fino al 1 luglio 1997, riaprire le coppe. Atteso: si vede L'EDIZIONE PRECEDENTE con gli stessi risultati, `[og-season-retain]` e `LAST_COMPLETED_EDITION`; non dire che la nuova 1997/98 e' attiva.
- Controllare che campionati e coppe nazionali nel 1997/98 abbiano nuovi contesti (gare zero al primo luglio) e avanzino correttamente, senza duplicati; F12 segnalera' giustamente UEFA nuova edizione NON RESIDENT. Fare SAVE, chiudere processo, CONTINUE, confrontare risultati vecchie coppe, anno reale del torneo e fingerprint `[og-state-fp]`. Ripetere in 2627 per CL/Europa/Conference e 1 luglio 2027. Testare due rollover consecutivi, con e senza club (manager-in-cup escluso fino a binding).
- In caso di `OG WORLD: ... NOT RESIDENT` per una COPPA della vecchia edizione, `checksum` diverso, save rifiutato o match vecchi scomparsi: FAIL, inviare ZIP AVVIA_DIAGNOSTICA. La nuova annata delle coppe continentali NON e' simulata da questa patch: test nuovi incontri = NON IMPLEMENTATO, non segnare PASS. I dati dei campionati/coppe nazionali dei vecchi anni restano senza archivio fixture completo.


## 17/11/2026 — verifica Windows 0.13.75, Copa Libertadores OG 1996/97
- Prima di provare archiviare separatamente il CAR 96/97 e non cancellare/sovrascrivere altri salvataggi; per la prova di avanzamento temporale iniziare una carriera **nuova di prova**, disoccupato, mondo 1996/97. Il CAR della 0.13.74 puo' gia' contenere partite Copa disputate troppo presto e NON viene riavvolto.
- Al 1 luglio 1996 in DEBUG WORLD -> SUD AMERICA -> Copa Libertadores 1996 verificare contesto residente, gironi iniziali, gare ancora zero, `[og-libertadores-96] world-phase=48 fixture-phase=0`. Fare FORWARD TIME fino a febbraio 1997: nessun risultato Copa anticipato. Fra marzo e giugno 1997, nuove gare devono apparire progressivamente; nei log confrontare `world-phase`, `fixture-phase`, `OG-matches=a->b`, `stage=x->y`. Controllare realmente gironi, ottavi, quarti, semifinali e finale; se resta fermo segnalarlo FAIL.
- Provare anche al confine giugno/luglio, senza doppia simulazione, e CONTINUE dopo salvataggio; confrontare date, partite già giocate, tabelloni e nessuna perdita. Ripetere con manager assunto in club NON partecipante e verificare che il torneo resti consultabile/avanzabile. Se il manager e' una delle 20 squadre, il percorso e' ancora `PENDING_MANAGER_BIND`: non segnare PASS, richiede integrazione con il suo slot OG.
- Verificare regressioni 96/97 Champions/Coppa Coppe/Coppa UEFA e leghe; 2026/27 Champions/Europa/Conference e Libertadores moderna non devono cambiare. Test Windows, save nuovo, multianno, controller: NON PROVATI dall'assistente; allegare ZIP AVVIA_DIAGNOSTICA e screenshot prima/dopo se un controllo fallisce.


## 17/11/2026 — checklist Windows 0.13.74, Champions 96/97
- GitKraken su `dev/0.10`, verificare build `0.13.74`, backup separato dei CAR preesistenti. START 1996/97 disoccupato: VIEW WORLD -> Europa -> Champions, annotare anno/girone/gare; accettare un'offerta di club che NON partecipa e aprire la stessa Champions da VIEW WORLD: deve restare consultabile, senza gara retroattiva. Controllare `[og-champions-view] ROUTE_RESIDENT clubSlot=0` e `RETURN resident=1`.
- Ripetere da club effettivamente iscritto nel primo anno: aprire anche la Champions dal calendario carriera, controllare B/BACK, stesso girone e risultati; se presente verificare `clubSlot=1`. Dopo avanzamento, SAVE e CONTINUE verificare che le gare OG non diminuiscano o si duplichino. Testare 2026/27 Champions/Europa/Conference per esclusione del routing storico.
- I log `[og-champions-view] RETURN resident=0`, crash, mancata voce menu o perdita gare sono FAIL/da analizzare, non PASS. Il test C isolato verifica solo il selettore di ramo; Windows/build/incontri reali NON PROVATI.


## 17/11/2026 — test Windows 0.13.72 e recupero file mancanti
- Prima di compilare usare copie esterne in luogo sicuro di save/AUTO9697.CAR, save/AUTO2627.CAR, save/AUTOSAVE.CAR: NON cancellare ne' sovrascrivere i file reali. Sincronizzare GitKraken, compilare. Aprire 9697 se snapshot valido, fare un passo OG, autosave e riaprire in processo nuovo verificando data, club, partite e F12.
- Ripetere su 2627 con un CAR valido di backup o con carriera DI PROVA separata, testare START->BACK senza firma: atteso WRITE_BLOCKED, non nuovo CAR incompleto. Sotto diagnostica log READ_ONLY_DISK_OK, no piu' vecchio [json-verify-career] N field(s)/byte(s) mismatch, PREWRITE_REJECTED e' errore e deve preservare il vecchio slot.
- Il AUTO2627.CAR incluso nel ZIP _22 contiene living_world:null e potrebbe essere gia' stato scritto dopo un test fallito: NON attestare ripristino fedele; preservarlo per futura analisi, senza fingere di conoscere la data. I commit precedenti _22/_23 restano nella storia; nuova numerazione di build .71 e .72.


## 17/11/2026 — test Windows 0.13.71
- Salvare copie separate di AUTO9697.CAR, AUTO2627.CAR e AUTOSAVE.CAR prima di fare prove; non sovrascriverle manualmente. Avviare il 96 con un save valido, controllare CONTINUE. Ripetere 26 con un nuovo save valido. Tentare START e poi BACK senza ingaggio; verificare nel log WRITE_BLOCKED e nessuna sostituzione di AUTO2627.CAR.
- Con il CAR 2627 dello ZIP, atteso INCOMPLETE_CAR reason=MISSING_CIVIL_CLOCK, NON 'schema aggiornato'; esso non puo' essere dichiarato integro. Nessuna verifica gameplay Windows o ripristino della data storica fatta.


## 17/11/2026 — test Windows 0.13.70_23
- Carriera prova Crewe 2026/27 con 2 indisponibili: verificare che si presentino 11 titolari e una panchina di sani, che i due rossi rimangano nella rosa extra e che PLAY MATCH sia selezionabile. Ruotare il piccolo controllo extra e verificare se si raggiungono entrambi. Nessun calciatore deve essere guarito o duplicato.
- AVVIA_DIAGNOSTICA: cercare career-lineup con roster, available, unavailable, old-bench, active-bench, swaps, extra e status; in presenza di NOT_REPAIRED non dichiarare il caso risolto. Verificare salvataggio, successivo turno, e 1996/97; niente prova gameplay Linux sostitutiva. Coppe con team copia e oltre 16 giocatori sono fuori perimetro.


## 17/11/2026 — checklist Windows 0.13.70_22
- In 96 e 26, F12 da START il 1 luglio (fase attesa 48) poi FORWARD TIME: leggere FASE, rapporto IN FASE, gare OG che aumentano quando previste, ANOM campionati, e coppe assenti senza falsi verdi.
- In club da settembre/dicembre, DEBUG +1 MONTH: cercare BEGIN e COMPLETED o FAILED nel log, verificare data +1 mese, risultati, nessun doppione, save/continue. A giugno blocco prima del cambio stagione; cambio stagione manuale. Usare soltanto una carriera di prova.
- Il softlock dei giocatori non e' stato modificato. Build Windows, manager/cup, controller e save ripreso: NON PROVATI.


## 17/11/2026 — test START CAREER 0.13.70_21 (Windows)
- Con build nuova, riprodurre 2627: START UNEMPLOYED, trascorrere 15 settimane, accettare offerta (esempio precedente ENNA). Nel log verificare `BEFORE_OG_RESIDENT` seguito da `SERIALIZED_COMPLETE` e `READY_FOR_ATOMIC_SAVE`, quindi calendario accessibile; controllare file CAR e riavvio CONTINUE. Ripetere con START club immediato e 9697. Non usare salvataggi importanti: controllare data, risultati OG, coppe e assenza duplicati.
- Compilazione Windows e gameplay di questa build NON PROVATI; se un altro blocco compare allegare ZIP della sessione.


0.13.70_19 — Compilare su Windows e testare DEBUG +1 MONTH nella carriera 96 e 26. Confrontare giornate, risultati della propria squadra, F12, date, autosave e CONTINUE, fermando al 30 giugno. In caso di FAILED in SWOS.log usare solo salvataggi di prova. NON PROVATO.


## 17/11/2026 — diagnostica mese UNEMPLOYED e verifica cambio stagione
- Build di codice 0.13.70_18: dopo START UNEMPLOYED (96 e 26), aprire F12 e vedere data/SETTIMANA/stagione. Usare il nuovo bottone `DEBUG +1 MONTH` (lato sinistro del calendario) fino al 30 giugno: verificare che ogni click conservi il giorno dove possibile, non superi il 30 giugno, e che il successivo click sia un NO-OP con `[debug-month] STOP_BEFORE_SEASON_CHANGE`. Poi premere `FORWARD TIME` normale per verificare il cambio stagione OG, confrontare F12 e stati delle competizioni.
- Ripetere dopo salvataggio/ripresa .CAR dei due mondi, verificando nessun reset di competizioni, nessun doppio risultato. Shortcut F12+7/8 rimangono separati; il nuovo pulsante rifiuta di sovrapporsi agli stress test armati.
- Questo pulsante non e' nel calendario del manager impiegato: esso usa PLAY/NEXT MATCH ed e' escluso per evitare match saltati. Si richiede audit distinto per il comportamento con club. Windows test NON ESEGUITO in questa consegna.


## 17/11/2026 — 0.13.70_9, pulizia root (Windows NON PROVATO)
- Unici BAT di avvio root: AVVIA_DIAGNOSTICA.bat e AVVIA_SWOS.bat. COMPILA_SWOS.bat e PREPARA_SWOS.bat sono in diagnostica/; tutti i riferimenti di avvio e compilazione sono aggiornati. BUILD_SWOS.sh scrive fuori dalle sessioni in diagnostica/log/COMPILAZIONE_0140.log; main.c su Windows redirige stderr in diagnostica/log/SWOS.log anche con avvio diretto. La diagnostica cattura solo il log della sessione corrente, archivia quelli precedenti e lascia tutti i .CAR invariati. Marcatori .SWOS_*_COMPILATO e backup SWOS.ini sono spostati in diagnostica/ (i preesistenti sono migrati dal launcher soltanto senza collisioni). La copia ASSETS.ini root viene eliminata dal repo: quella effettivamente letta e' assets/ASSETS.ini.
- SDL2.dll e SWOS.ini rimangono accanto all'EXE: il loader Windows e la funzione swos_ini_path() li richiedono ancora nella posizione originale; nessun cambiamento ai salvataggi in save/. I file locali non tracciati non possono essere spostati dalla sola operazione Git: la migrazione avviene alla successiva esecuzione del launcher, quando non esiste un file di destinazione omonimo. Verifiche Windows e compilazione PENDENTI.

## 16/11/2026 — prossimo test e diagnostica 0.13.59

`AVVIA_DIAGNOSTICA.bat` invoca quattro BAT che devono esistere nella root (`ANALIZZA_SWOS_LOG.bat`, `TEST_OG_WORLD.bat`, `TEST_OG_70.bat`, `TEST_OG_CUPS_SCORERS.bat`); wrappers MSYS2 preesistente, non richiedere una nuova toolchain né far finta che i test statici provino Windows. Il parser legge **solo SWOS.log della sessione**, controllando build; stress 70 anni = osservazione del F12 realmente completato, non simulatore alternativo. Gli script LWE storici sono esclusi dal runner attivo. I test d'integrazione dei due mondi nella STESSA sessione inizieranno solo dopo aver tolto il restart per cambiare mondo; intanto prove Windows separate e senza modificare i save esistenti.

Checklist prima del «funziona»: compilazione Windows SI/NO, START 1996/97 SI/NO, START 2026/27 SI/NO, tre UEFA type-2 residenti al START SI/NO, prima partita OG datata SI/NO, OFFERTA 0 partite e club non partecipante SI/NO, selettore B un livello SI/NO, Libertadores CT_IN preparato SI/NO, SAVE/RESTORE residenti SI/NO, 70 anni SI/NO, cambio mondo senza restart NON IMPLEMENTATO. Rapporti e screenshot verranno dall'utente Windows dopo GitKraken. Vedere [COMPETITIONS_TWO_WORLDS_SPEC.md](COMPETITIONS_TWO_WORLDS_SPEC.md); annotare le interruzioni del 16 novembre.


## Regole prevalenti — 15 novembre 2026
Il vecchio simulatore autonomo LWE, i risultati/calendari LWE e il singolo `AUTOSAVE.CAR` sono descrizioni storiche. Per i due mondi la sola autorita' e' OG OpenSWOS; LWE non genera fixture o risultati paralleli. Su Windows `save/AUTO9697.CAR` e `save/AUTO2627.CAR` sono gli slot scrivibili accanto a `SWOS_Expanded.exe`, mentre `AUTOSAVE.CAR` e' solo fallback legacy in lettura. L'utente esegue la compilazione e i test Windows dopo GitKraken usando MSYS2 gia' installato (`C:\\msys64`). Un commit o la revisione statica non provano una build funzionante.

Status: approved working rules. Primary integration branch: `dev/0.10`.

## 1. Workflow ChatGPT/GitHub/GitKraken (ripristinato 2026-11-09)

Il metodo attivo usa la chat normale e il repository GitHub, sincronizzato dall’utente con GitKraken. Il precedente metodo Codex locale e` storico. Prima di modificare verificare ramo, HEAD, stato locale, documentazione e dipendenze. Preservare modifiche non committate anche tra sessioni; non ricreare automaticamente la copia di lavoro dal remoto.

Si possono ripetere modifiche, compilazioni e test locali senza commit intermedi. Commit e push richiedono ciascuno autorizzazione esplicita dell'utente. Registrare risultati reali e lavoro non committato nel passaggio di sessione. Rispettare i limiti della richiesta corrente: installazioni/aggiornamenti e avvio del gioco richiedono autorizzazione.

### Applicazione della procedura permanente

La sequenza obbligatoria e le autorizzazioni sono definite in [AGENTS.md](../AGENTS.md), sezione 0; non duplicare qui l'elenco delle regole.

- **Preparazione:** leggere il diff esistente e identificare i file necessari. Usare controlli Git in sola lettura; non sincronizzare, ripristinare o cambiare ramo automaticamente. GitKraken resta lo strumento di sincronizzazione scelto dall'utente.
- **Contraddizioni:** riportare le fonti e le opzioni concrete, attendere la decisione sulla parte controversa e proseguire soltanto sulle parti indipendenti già autorizzate.
- **Alternative:** presentare un confronto proporzionato alla richiesta, distinguendo fatti verificati e ipotesi. Indicare benefici, impatto, compatibilità Windows, costi, tempi e rischi della soluzione raccomandata; attenderne l'approvazione esplicita prima di adottarla.
- **UI:** raccogliere misure e mappatura input/callback dal codice reale secondo la sezione 8; mostrare il PNG con dati riproducibili prima dell'asset pulito. Distinguere approvazione visiva, autorizzazione all'integrazione e approvazione di eventuali cambi tecnici.
- **Sviluppo e verifica:** eseguire le iterazioni locali autorizzate senza commit intermedi; scegliere controlli pertinenti e verificare che la toolchain sia già disponibile. Nessuna installazione o aggiornamento automatico. Un limite specifico della richiesta, come sola lettura o divieto di avvio, prevale sulle verifiche normalmente previste.
- **Consegna:** elencare file modificati/rimossi, verifiche svolte e relativo esito, prove ancora necessarie e problemi aperti. Non dichiarare collaudi non eseguiti; lasciare le modifiche locali in attesa di eventuali richieste esplicite di commit e push.

## 2. Asset approvati nei percorsi definitivi

Salvare direttamente i nuovi PNG approvati nelle cartelle definitive locali e verificarne formato, dimensioni, trasparenza e contenuto. Conservare gli asset approvati esistenti fino a richiesta di sostituzione.

ZIP, ASSETS_INBOX e trasferimenti GitHub non sono più il workflow ordinario. Sono revocate le autorizzazioni automatiche a committare gli asset. ASSETS_INBOX, verificata contenente soltanto `.gitkeep`, viene rimossa. La documentazione di trasferimento resta solo per memoria storica; il relativo job GitHub è disabilitato.

**Motivo:** lavorare sulla copia Windows reale consente iterazioni e verifiche locali senza trasferimenti né commit intermedi obbligatori.

## 3. Asset approval versus implementation

**Procedure:** Approval of artwork authorizes storing that asset. Modify code or replace an existing in-game asset only when the user requests implementation or replacement. Keep previously approved UI elements unchanged unless specifically requested.

**Why:** Approval of an image is not automatically approval of unrelated code or layout changes; separating the steps prevents regressions and preserves agreed visual designs.

## 4. Fixes, verification and commits

**Procedure:** For a requested bug fix, identify the actual active source files and input mappings, make a targeted change, inspect the diff, run available syntax/build checks and relevant tests, leave the changes local until the user explicitly authorizes a commit; push requires explicit authorization too. Report precisely what was tested, what could not be tested, and any uncommitted changes; report a commit identifier only if a commit was explicitly authorized and created. Do not claim a fix or successful build merely because a commit exists.

**Why:** A repository edit does not prove runtime behavior; focused changes and explicit checks reduce regressions and make troubleshooting reproducible.

**Current pending issue:** In the Business screen, selecting an expense item with the joypad and pressing X should open its details popup immediately, without requiring cursor movement or an additional keypress. Preserve the approved popup artwork. This remains unverified and must not be reported as fixed until the code and behavior have been checked.

## 5. Conservazione e pulizia

Conservare asset approvati e file necessari al progetto. Le vecchie procedure di trasferimento sono documentazione storica, non istruzioni per caricare, committare o cancellare automaticamente file.

## 6. Versioni e continuità

Verificare la versione in `sorgenti/include/build_info.h`. Modifiche locali e test ripetuti non richiedono commit intermedi né incrementi automatici della versione. Applicare le convenzioni di numerazione di AGENTS.md quando viene autorizzato un commit di codice o richiesta una nuova build. Questa manutenzione documentale e degli script conserva `0.10.54_09`.

I nomi `COMPILAZIONE_0140.log` e `.SWOS_0140_COMPILATO` sono mantenuti per compatibilità; non indicano la versione corrente. BUILD_SWOS.sh verifica la toolchain già installata senza installare o aggiornare pacchetti. PREPARA_SWOS.bat conserva il percorso di installazione iniziale MSYS2 con conferma esplicita: non autorizzarlo se la richiesta vieta installazioni.

## 7. Modalità video development e release

**Procedura development:** il launcher `AVVIA_SWOS.bat` imposta `SWOS_EXPANDED_DEV_MAXIMIZED=1`, usa `--fullscreen=off` e applica `CONFIGURA_4K.ps1` senza `-Release`. SDL2 massimizza una finestra Windows **con bordi e barra del titolo**, mantenendo taskbar/Alt+Tab. Non impostare una finestra borderless fullscreen: non è la stessa cosa.

**Persistenza della massimizzazione durante le partite:** il cambio di vista DOS/menu tramite `MatchViewEnter`/`MatchViewLeave` chiama `Platform_ViewChanged`, che aggiorna la scala logica ma non deve chiamare `SDL_SetWindowSize` se `SDL_WINDOW_MAXIMIZED` è attivo. Le finestre normali continuano a ridimensionarsi come prima; le release fullscreen non sono interessate. Test Windows: avviare via BAT, entrare e uscire da una partita, verificare dimensioni/bordi/taskbar e Alt+Tab sia all'ingresso che all'uscita.

**Procedura release (obbligatoria):** preparare il profilo release con `CONFIGURA_4K.ps1 -Release`, non impostare `SWOS_EXPANDED_DEV_MAXIMIZED` e avviare il gioco con `--fullscreen=desktop` (o impostazione equivalente nel pacchetto release). Verificare che ogni launcher/release distribuita parta effettivamente a schermo intero, anche se una precedente sessione development aveva impostato `fullscreen=off`. Conservare il rendering 4K e Alt+Invio.

**Perché:** la finestra massimizzata rende più rapido passare dal gioco agli strumenti di sviluppo senza nascondere l'interfaccia Windows. La release deve invece conservare l'esperienza fullscreen approvata; separare le configurazioni evita che il debug cambi accidentalmente il comportamento finale.

## 8. Progettazione e approvazione delle nuove UI PNG

La regola del BMP nativo obbligatorio per i nuovi asset frontend è **annullata** dal commit 0.10.52. Conservare intatti i BMP, RAW e PNG storici già approvati.

1. Leggere il menu attivo, `sorgenti/docs/FONTS.md`, i font e le funzioni effettivi, incluse spaziature, coordinate, dimensioni e hitbox; misurare le stringhe tramite `GetTextWidth` / varianti e dati massimi.
2. Prima creare il mockup **PNG lossless** con dati dimostrativi riproducibili nel motore per approvazione; poi creare il PNG UI pulito RGBA trasparente, con gli elementi dinamici gestiti dal renderer.
3. Conservare ogni artwork illustrato separato da logo, UI e dati. Per gli sfondi 1672×941 generazione e crop 1664×936 senza resize salvo richiesta.
4. Il nuovo renderer SDL2 carica 8-bit PNG RGB/RGBA non interlacciati con miniz; il titolo è modernizzato prima degli altri menu, senza modificare partite e salvataggi.

**Workflow corrente (9 novembre):** salvare i PNG approvati direttamente nei percorsi definitivi locali; commit e push solo su esplicita autorizzazione.

> **Cronologia tecnica delle build (sezioni 9–23):** consegne ZIP, trasferimenti, push e limitazioni di accesso locale descritti sotto sono storici, superati dal workflow del 9 novembre. Non costituiscono autorizzazioni operative; conservare i dettagli tecnici pertinenti e distinguere test storici e correnti.

## 9. Frontend moderno PNG e prova Title Menu (build 0.10.52)

I nuovi sfondi sono PNG lossless nativi; per gli sfondi a schermo intero riferimento 1672×941 di generazione e 1664×936 dopo il solo crop 16:9. Se una delle due dimensioni differisce occorre avvertire l'utente prima dell'integrazione. Le componenti UI PNG sono separate dallo sfondo; il logo canonico sarà un asset condiviso, i testi dinamici restano del renderer.

Il Title Menu in 0.10.52 carica `assets/title/title_background_approved_2026-11-08.png` tramite il decoder PNG in `sorgenti/src/expanded_png.c` (miniz esistente, nessuna nuova DLL), estrae i soli pixel non identici allo sfondo RAW legacy e lascia le reali callback/tasti/focus nello stesso menu 8-bit. **F8** alterna PNG/originale esclusivamente sul Title Menu. In assenza dell'asset, o con PNG non conforme a 1664×936, usa senza crash il vecchio sfondo. Non modificare la partita.

Dopo sincronizzazione GitKraken, eseguire `AVVIA_SWOS.bat` per compilare/avviare come già previsto. Verificare con joypad, tastiera e mouse i pulsanti, il ciclo TITLE→sottomenu→TITLE, F8, build number, windowed maximized e fullscreen desktop, ed eventuali residui del vecchio sfondo. La compilazione/test Windows può essere confermata solo dall'utente. Per il PNG usare il trasferimento a blocchi documentato in `asset-transfer/README.md` con verifica SHA-256 del file ricostruito; non dichiarare asset su GitHub finché l'Action non ha scritto e verificato il binario.


## 10. Logo Title Menu condiviso — build 0.10.53
Il file `assets/title/swos_expanded_logo_approved_2026-11-08.png` è il logo canonico, non deve essere incorporato negli sfondi. Dim. originale 1200×440 RGBA trasparente; in Title Menu la posizione approvata C corrisponde a (920,17) con dimensioni 620×227 sullo sfondo di coordinate 1664×936. Lo scaling segue le dimensioni del renderer senza toccare gli hitbox; i dati dinamici e i controlli rimangono originali. F8 passa dall'insieme PNG+logo alla schermata storica, e viceversa.

Per test Windows: sincronizzare `dev/0.10`, copiare i due PNG esatti sotto `assets/title/` (non sostituire il RAW), avviare `AVVIA_SWOS.bat`, verificare logo, F8 e ogni pulsante con tastiera/joypad/mouse. Non considerare i nuovi PNG su GitHub finché il relativo trasferimento binario, a blocchi di massimo 1 MiB e SHA-256, non risulti verificato. Conservare asset storici e salvataggi invariati.


## 11. Title Menu 0.10.53_01: due livelli e pressione SDL2
Il Title non disegna più grafica dei pulsanti SWOS 8-bit e non offre più F8. Mantiene background e logo approvati e carica cinque PNG RGBA indipendenti sotto `assets/title/buttons/`. Il primo livello presenta CAREER, HALL OF FAME, CLASSIC MODES, OPTIONS, EXIT GAME. Il secondo livello temporaneo, senza immagine di sfondo, presenta le modalità originali e CONTINUE/REPLAY/SAVE condizionali. La pressione è un movimento di circa sei tick via renderer SDL2, senza GIF.

Asset in ZIP da estrarre accanto a `AVVIA_SWOS.bat`; compilazione e test Windows ancora necessari. Non affermare che gli asset sono su GitHub prima di un push verificato.


## 12. Pulizia launcher/anteprime superati (0.10.53_03)
Per il gioco corrente il launcher unico è `AVVIA_SWOS.bat`; `COMPILA_SWOS.bat`, `PREPARA_SWOS.bat` e `BUILD_SWOS.sh` restano script attivi. Le note più sopra su **F8** e sui vecchi BAT di test riguardano esclusivamente le build storiche 0.10.52 e 0.10.53; NON applicarle al Title moderno.

I file temporanei di quei test e il PNG di anteprima del Title RAW sono stati rimossi dal repository. Sono stati eliminati anche tre manifest di asset già ricomposti e verificati, senza rimuovere gli asset definitivi. Il vecchio sistema Action/manifest è disponibile soltanto come metodo secondario quando lo ZIP o il deposito `ASSETS_INBOX/` non bastano.

**Windows:** dopo pull con GitKraken, i file tracciati cancellati vengono normalmente rimossi dal working tree; i file personali o altre copie non tracciate possono restare. `.gitignore` previene l'aggiunta accidentale degli specifici file di test superati. Se occorre cancellare copie residue, farlo solo per i percorsi specifici presenti nel registro `PROJECT_STATE.md`, mai con una cancellazione generica di cartelle di gioco. Conservare tutte le risorse RAW/BMP approvate ancora richieste dal codice e i salvataggi locali.


## 13. Pulsanti base PNG vs stato selezionato — Title 0.10.53_04
Per i pulsanti UI del nuovo frontend conservare file PNG RGBA **privi di outline gialla**. Il bordo illustrato proprio del bottone (azzurro/3D) rimane. Il layer SDL2 del focus giallo deriva dinamicamente dall'alpha completo dello sprite e non deve essere disegnato nel PNG di partenza. Se due PNG di uno stesso pulsante hanno risoluzioni sorgenti differenti, compensare il raggio del focus per mantenere lo stesso spessore visivo (CAREER raggio 6 px, CONTINUE CAREER raggio 12 px).

Per la carriera attiva il Title usa uno sprite separato `assets/title/buttons/continue_career.png` con testo già presente nel file, non una scritta sovrapposta via renderer. Lo stato seleziona lo sprite; usare contenimento proporzionale in presenza di rapporti geometrici differenti e condividere le hitbox con CAREER.

Consegna approvata in ZIP con percorso finale, estrazione accanto ad `AVVIA_SWOS.bat`, successivo push GitKraken e verifica GitHub. Non indicare l'asset come remoto finché non è verificato.

## 14. Icona video Title e build 0.10.54
La regola di assenza di contorno giallo si applica **al momento della generazione di ogni nuovo PNG base della UI**: nessun focus baked-in; non ritoccare retroattivamente gli asset approvati. Il focus dinamico SDL2 è un effetto separato che segue la silhouette alpha.

Il Title usa un sesto punto selezionabile in alto a sinistra, con immagini fullscreen_expand/windowed_collapse a stati mutuamente esclusivi e navigazione ciclica: SU da CAREER o GIÙ da EXIT seleziona il controllo. Premendo il tasto si richiama lo stesso Platform_ToggleFullscreen() usato da Alt+Invio. La finestra development massimizzata resta una finestra decorata anche dopo il ritorno da fullscreen.

Il nuovo codice si trova nel branch `dev/0.10`; due PNG dell'icona e CONTINUE CAREER sono consegnati nel ZIP versione 0.10.54. Verificare questi file dopo il push GitKraken, poi testare su Windows tastiera/joypad/mouse, stato icona in entrambi i versi, Alt+Invio, finestra massimizzata durante menu→partita→menu, release fullscreen desktop e scaling del Title.

## 15. Canonical button style — 0.10.54_01 (8 novembre 2026)
Quando si **genera** un nuovo pulsante PNG del frontend Expanded, rispettare lo stile della serie Title approvata: pixel art blu/ciano vista frontalmente, con profondità scura a destra e sotto, alpha trasparente attorno alla sagoma e nessun bordo giallo di focus/hover nel PNG base. Il renderer produce la selezione separatamente dalla silhouette e anima la pressione mediante traslazione (breve bounce sul rilascio) senza riprocessare i pixel artistici. Non applicare modifiche automatiche agli asset storici approvati.

Per il Title 0.10.54_01: icona video (34,34,92×88); CAREER (1048,426,365×151), HALL (1055,585,350×67), CLASSIC (1055,658,350×67), OPTIONS (1055,731,350×67), EXIT (1055,804,350×67) nel canvas 1664×936. La grafica originale non è stata rigenerata. Testare su Windows focus, bounce, giù/su da EXIT/CAREER e transizione fullscreen↔finestra massimizzata.


## 16. Ripristino finestra dalla modalità fullscreen — 0.10.54_02
Nel ramo development `AVVIA_SWOS.bat` imposta `SWOS_EXPANDED_DEV_MAXIMIZED=1`: `Platform_Init` forza l'avvio in finestra SDL2 decorata anche se un profilo precedente ha memorizzato fullscreen in `SWOS.ini`. Il flag non va impostato nei launcher release.
All'uscita dal fullscreen, una sessione inizialmente fullscreen usa per impostazione di ritorno la finestra decorata massimizzata, invece della vecchia geometria piccola `_screen_width × s_win_scale_current`. Se la sessione proveniva da una finestra normale non massimizzata, conserva le regole di ripristino della dimensione utente. Verificare su Windows icona Title, Alt+Invio, taskbar/titolo e menu→partita→menu.


## 17. Primo frame SDL desktop — 0.10.54_03
Il renderer desktop crea la finestra con `SDL_WINDOW_HIDDEN` e chiama `SDL_ShowWindow` una volta dopo il primo `SDL_RenderPresent` completo; nessun cambiamento al canvas WASM. Scopo: non mostrare il client bianco non inizializzato prima dello splash. Testare su Windows la visibilità dello splash, la massimizzazione vera e le transizioni menu/partita e fullscreen; l'esito non è garantito finché non è verificato su Windows.


## 18. Visibilità iniziale dello splash — 0.10.54_04
La prima chiamata a `SDL_RenderPresent` può precedere `Platform_ExpandedSplashSet(0)`, creando un fotogramma nero anche con la finestra inizialmente nascosta. Mostrare la finestra solo dopo aver composto e presentato il primo frame dello splash Expanded; se l'asset è indisponibile mostrare un fallback, e consentire l'apertura diretta del Title. Non modificare i layer approvati né inventare percentuali di caricamento. Controllare realmente su Windows il percorso normale e il percorso senza splash.


## 19. Primo present dello splash (0.10.54_05)
Per il frontend desktop, `SDL_ShowWindow` viene chiamato subito **prima** del primo `SDL_RenderPresent` contenente lo splash Expanded: i frame di bootstrap privi di splash restano nascosti. Si preservano i percorsi fallback, la versione WASM e la finestra massimizzata development. Il risultato contro il breve flash nero resta da verificare su Windows.


## 20. Title glow e splash anticipato — 0.10.54_06
Il caricamento dei PNG Title ad alta risoluzione, incluso il calcolo degli outline alpha, è spostato dopo la prima presentazione dello splash Expanded 0%, ora eseguita sul renderer SDL già inizializzato in `Platform_Init`. `mainC` mantiene la progressione del caricamento reale. La luminosità di `CONTINUE CAREER` è una sovrapposizione cyan SDL2 separata e permanente soltanto con carriera attiva, senza modificare asset, contorno giallo o bounce. Verificare il nero prima dello splash, il messaggio di tempo `[boot] Title PNG preparation after splash`, e qualità del focus su Windows.


## 21. Colore CONTINUE CAREER — 0.10.54_07
Tolto il precedente glow azzurro esterno e la sovrapposizione additiva. Si usa una semplice modulazione SDL2 `RGB(255,120,215)` sul PNG canonico mentre `CONTINUE CAREER` è presente; immediato ripristino dello stato colore dopo il disegno. Il focus giallo continua indipendente e lo sprite PNG non è stato editato.


## 22. CONTINUE CAREER rosso — 0.10.54_08
Per colori di pulsanti con iscrizioni bianche, NON applicare SDL_TextureColorMod all'intera texture: altera anche le lettere. La tinta rossa del solo `CONTINUE CAREER` è ottenuta a runtime tramite conversione selettiva dei pixel blu nel buffer RGBA decodificato, lasciando invariati bianchi, nero, alpha e profondità. Nessuna modifica al PNG canonico su disco. Verificare l'effetto sul rendering Windows; il focus dinamico resta separato.


## 23. CONTINUE CAREER verde — 0.10.54_09
Il colore rosso della build precedente è sostituito dal verde mediante riconversione selettiva degli stessi pixel blu/indaco del PNG canonico già decodificato. Non applicare tint globale: il testo resta bianco. Non cambiare asset né geometria; mantenere focus e animazione precedenti.

### Consegna attuale
La chat normale puo' creare commit GitHub su richiesta dell'utente, sul ramo verificato. GitKraken sincronizza e l'utente compila/testa su Windows. Gli ZIP con asset/file e le istruzioni in chat sono **separati**. Le istruzioni precedenti sul workflow locale Codex non sono piu' prescrittive.

---

## SWOS_DOC_SYNC_2026_11_10 — Documentazione e pulizia grafica (10/11/2026)

- Prima di una modifica leggere `AGENTS.md`, `PROJECT_STATE.md`, `docs/GAME_DESIGN.md` e i documenti tecnici pertinenti. `GAME_DESIGN` specifica funzionalità approvate e idee; `PROJECT_STATE` descrive cosa è nel gioco; `IMPROVEMENTS` registra modifiche effettivamente introdotte. Quando cambia una decisione progettuale, allineare le guide nel **prossimo commit pertinente autorizzato**, non in commit documentali autonomi.
- **Grafica finale:** TITLE, NEW CAREER e LOADING. Gli altri menu e asset pre-switch sono **visivamente obsoleti**, ma rimangono funzionanti e presenti finché non saranno riprogettati; il nome storico `approved` non autorizza a riusarli come layout definitivo.
- **Prossimo BAT di pulizia:** prima creare un inventario dei caricamenti e riferimenti in C, JSON, script, menu e fallback, classificandoli `IN_USE`, `REPLACED`, `ORPHAN_CANDIDATE`. Produrre un elenco esatto dei file per revisione; solo gli orfani verificati possono essere cancellati dal BAT. Nessuna pulizia per pattern generale `*.raw`, `*_approved*`, directory intere o per confronto di data. Preservare `.gitkeep` e asset/shared runtime. Controllare la compilazione e il ciclo completo di menu; ricordare l'utente al prossimo BAT di pulizia programmato. Distinguere gli asset obsoleti nel **design** dagli asset inutilizzati nel **runtime**.
- **Tempo:** uno scheduler di date/eventi, con risultati e salvataggi coerenti. Test d'integrazione con i club attuali, prima di allargare il database. WEEK visibile nella maggioranza dei menu e NEXT EVENT in CALENDAR/UNEMPLOYED; posizione grafica ancora da concordare; nessuna modifica all'orologio partita a 50 Hz.
- **Consegna ZIP:** contenere esclusivamente file nuovi o realmente modificati nei relativi percorsi. L'utente estrae nella cartella contenente `AVVIA_SWOS.bat`, sincronizza su GitHub via GitKraken quando decide. Uno ZIP non equivale a commit o push; verificare HEAD/branch alla sessione seguente. Script di aggiornamento documentale e BAT di rimozione asset sono **operazioni diverse**.


## Workflow vigente dal 10 novembre 2026 — sostituisce le indicazioni ZIP precedenti

- **Codice C, script, BAT, JSON e documenti:** lavorare sul repository GitHub `Giuzy/SWOS-Expanded`, sul branch verificato, con **commit/push solo su richiesta esplicita «committa»**. L'utente sincronizza via GitKraken. Non rinominare automaticamente il ramo `dev/0.10` quando cambia la versione.
- **ZIP soltanto per nuovi asset grafici** da estrarre con i percorsi definitivi. Non predisporre pacchetti ZIP per semplici modifiche di codice/documentazione.
- **Versionamento:** primo commit serie nuova `0.13.01`; ogni commit successivo di codice incrementa .01 senza suffissi, nuovo milestone +0.1 soltanto su richiesta. Se il formato esatto della successiva milestone non è chiaro, confrontarsi con l'utente prima di cambiarlo.
- **Fase 1:** testare `bash sorgenti/tests/run_test_expanded_world_time.sh` (o aprire la shell nella directory `sorgenti` e chiamare `bash tests/run_test_expanded_world_time.sh`). La compilazione completa e il test gameplay su Windows rimangono distinti dai test isolati.

## Test manuali 0.13.02 — debug F12, F9 e comandi

1. Sincronizzare `dev/0.10` con GitKraken, lanciare `AVVIA_SWOS.bat` e verificare **Build 0.13.02**.
2. Premere **F12** in UNEMPLOYED e controllare le settimane di attesa reali, il WORLD 9697/2627, la data **DATE EST**, e i tre stati **NOT ACTIVE**. Non interpretare i risultati del ledger isolato come partite già simulate nel gioco.
3. Premere **F9**: nel percorso di avvio del gioco deve apparire `swos_debugNNN.png`, con testo leggibile. Premere ancora F12 per chiudere il testo.
4. Entrare in un club, aprire F12, premere **1** (WIN ALWAYS ON), giocare la successiva partita con **VIEW RESULT**, verificare vittoria ufficiale, classifica e statistiche; premere **2** e verificare la sconfitta nella partita seguente. Le forzature sono esclusive. **Non usare PLAY MATCH per controllare la corrispondenza tra gol in campo e risultati ufficiali**, perché questo debug modifica solo la fase dei risultati.
5. Con F12 visibile premere **3** (SACK NEXT ARMED), poi aprire la prossima azione PLAY MATCH / NEXT MATCH; controllare la procedura di esonero e di nuovo impiego. Se si sta provando un salvataggio importante, usare una copia: i risultati di prova, a differenza dei toggle, possono essere salvati.
6. Da shell MSYS2, se necessario: `bash sorgenti/tests/run_test_expanded_phase12.sh`. Nessuna nuova dipendenza.

**Stato della consegna:** solo GCC su moduli isolati provato; build completa/test Windows ancora dell'utente. Nessuna asserzione di eventi realmente simulati durante disoccupazione.


## Test della 0.13.03 — mondo pilota, prima dell'integrazione della carriera

- Sincronizzare `dev/0.10` con GitKraken e compilare normalmente via `AVVIA_SWOS.bat`, osservando **Build 0.13.03**.
- Avviare **UNEMPLOYED**, far trascorrere almeno 5–10 settimane, aprire l'offerta e scegliere **VIEW LEAGUE**: la classifica pilota dovrà mostrare partite `PL > 0`; se ci sono 35 settimane di attesa, dovrà riflettere le giornate maturate. La posizione del club offerto deve essere identificabile.
- Se la simulazione non parte, consultare `SWOS.log` (righe `[living-world]`) e condividere screenshot/errori; non presupporre che un'assenza di partite in una divisione non supportata sia un successo.
- Accettare una squadra e verificare che la partita reale e il calendario classico restino funzionanti. **Il calendario nativo può ancora partire da settimana 1**: l'innesto senza reset è dichiaratamente NON ancora implementato, non va considerato un errore nuovo introdotto dalla patch.
- Test isolato `bash sorgenti/tests/run_test_expanded_world_live.sh`. Questi controlli non sostituiscono compilazione/test Windows su dati reali. F12/F9 continuano come in 0.13.02; nessuna nuova grafica o ZIP.


## Verifica build 0.13.05 — inizio luglio e amichevoli pilota

1. Sincronizzare `dev/0.10` su Windows e ricompilare con `AVVIA_SWOS.bat`: versione **0.13.05**. Accertare che non vi siano errori GCC prima di testare.
2. NEW CAREER → UNEMPLOYED: la prima schermata deve mostrare il **1° luglio** (1996 o 2026, secondo database) e **WEEK 01**. Dopo quattro avanzamenti: luglio concluso, nessuna giornata di campionato simulata, ma i contatori del precampionato delle squadre AI già aggiornati.
3. Quando arriva un'offerta prima della prima partita ufficiale, VIEW LEAGUE deve mostrare **PRESEASON: N FRIENDLIES (PILOT)**, con `PL=0`; dalla prima giornata di agosto la classifica comincia a riportare PL e punti.
4. Dopo 20–35 settimane verificare classifiche e offerte senza duplicazioni. F12/F9 restano come prima e indicano ancora i limiti del mondo nativo.
5. Test isolati disponibili: `bash sorgenti/tests/run_test_expanded_world_live.sh`; campionati dispari con BYE, continuità luglio/agosto e passaggio alla stagione successiva. Prova in-game con screenshot preferita.
6. **ATTENZIONE**: CLUB prezzo biglietti, amichevoli del manager, persistenza delle conseguenze e ingresso nativo a stagione iniziata sono ancora da implementare. **Non usare una carriera importante per prove di questa fase pilota**.


## Test obbligatori 0.13.06 — assunzione a stagione inoltrata

1. **Compila 0.13.06** da `dev/0.10` con `AVVIA_SWOS.bat`. Non eseguire prove su salvataggi importanti. Se il compilatore si ferma, condividi il log; nessuna compilazione Windows è stata eseguita nella chat.
2. **NEW CAREER → UNEMPLOYED → 8–12 settimane:** apri VIEW LEAGUE e prendi uno screenshot **prima** di firmare. Accetta e osserva il recupero automatico di NEXT MATCH/VIEW RESULT. In `SWOS.log` cerca `[midseason] begin` e `[midseason] native next-fixture calendar reached`; un messaggio `catch-up aborted` è un errore/limitazione da comunicare.
3. **CARRIERA → classifica nativa:** controlla che non sia a 0 gare e che `NEXT MATCH` sia una partita successiva alla data di firma. Confronta partite giocate e punti con il preview: **le due classifiche possono differire perché i calendari non sono ancora unificati**. Non dichiarare il test di corrispondenza superato senza riscontro.
4. **Ripeti con 20 e 35 settimane** se il primo caso è stabile; controlla ingresso al club, match simulato, partite non rigiocate, tavola nativa, numero di vittorie/sconfitte personali inizialmente 0 e nessun esonero immediato per partite precedenti.
5. **Dopo la firma, gioca una partita reale**; verifica menu → match → menu, F12/F9 come 0.13.05, joypad e mouse. Salva una **carriera di prova**, ricaricala e verifica stato nativo della lega e prossima partita. Prova una coppa soltanto dopo che il caso campionato funziona.
6. **Limiti:** oltre 52 settimane dall'inizio del primo anno il catch-up non parte; world pilot non è persistito e il confronto pre-/post-firma è ancora solo diagnostico. Se il log non mostra lo stop, fermati e segnala l'ultima schermata.
7. Test isolato per cutoff: `bash sorgenti/tests/run_test_expanded_midseason_join.sh`; non sostituisce test dell'engine nativo. Nessuna modifica ad asset finali o debug F12.


### Smoke test 0.13.20 — non sostituisce test Windows
1. Pull e compila dev/0.10. Controlla Build 0.13.20.
2. Firma un contratto, cerca `save/AUTOSAVE.CAR` accanto all'EXE.
3. Simula/gioca una gara, chiudi e riavvia DIRETTAMENTE l'EXE.
   CONTINUE CAREER deve comparire e caricare settimana, club,
   risultati e classifica esatti. Salva nuovamente dopo altro match.
4. Controlla `SWOS.ini` e `SWOS.log` accanto all'EXE.
   Non aspettarti di riprendere un match interrotto a metà.
5. Solo dopo test positivo, esegui volontariamente
   `PULISCI_VECCHI_SAVE_DOCUMENTI.bat` per i vecchi file; non
   cancellarli prima di aver verificato il caricamento del nuovo slot.


### 0.13.22 — test automatico 70 stagioni
1. Dopo pull di `dev/0.10`, avvia `TEST_LIVING_WORLD.bat` dalla cartella `SWOS_Expanded.exe`. Richiede solo MSYS2/GCC già impiegati dal normale `PREPARA_SWOS.bat`. Non avvia il gioco e **NON** modifica `save/AUTOSAVE.CAR`.
2. Il BAT scrive nel gioco `LIVING_WORLD_70_TEST.log`; cercare `STRESS_RESULT=PASS` alla fine. Su FAIL incollare/inviare il log **completo**, anche se il BAT si chiude dopo un tasto. Non affermare un successo se non verificato.
3. Il test usa 216 club/11 leghe sintetici e la stessa sorgente `expanded_world_live.c`, calendari OG e serializer v3. Completa 70 anni archiviati, controlla campioni, assenza di fixtures duplicate, risultati cumulativi, memoria e 15 salvataggi/caricamenti esatti. Non sostituisce le prove UI/manager.
4. Per integrazione reale Windows, eseguire `AVVIA_SWOS.bat` e poi l'EXE; verificare `Build 0.13.22`, CONTINUE CAREER dal vecchio `AUTOSAVE.CAR` v2 e almeno una simulazione e una partita manuale con F12. Aspettarsi `WORLD WINDOW`, `ARCHIVED LEAGUE-YEARS`, `ARCHIVED RESULTS`.
5. Se una build/test fallisce, non cancellare il save esistente. `SWOS.log` e `LIVING_WORLD_70_TEST.log` sono i due file diagnostici richiesti.


### 0.13.23 — GCC 16.2 / test runner Windows
- Il `LIVING_WORLD_70_TEST.log` precedente si fermava in compilazione per warning `-Werror` di mjson/miniz (librerie vendorizzate) e `sign-compare` in Expanded. Il log NON misura ancora la simulazione.
- Gli script `run_test_living_world_70.sh` e `run_test_expanded_world_live.sh` ora compilano mjson e miniz in due oggetti separati con flag di compatibilità mirati; tutto il codice nostro resta `-Werror`.
- Procedura: pull `dev/0.10`, doppio click `TEST_LIVING_WORLD.bat`; sul log verificare `[run]` e `STRESS_RESULT=PASS`. Se fallisce, inviare il file completo. Il BAT non cancella né sovrascrive `save/AUTOSAVE.CAR`.
- Poi avviare il gioco con `AVVIA_SWOS.bat`, controllare Build 0.13.23 e il precedente `CONTINUE CAREER`. Compilazione Windows e verifiche effettive rimangono esterne.


### 0.13.24 — passo successivo dopo l'errore di linker `swos_io_realloc`
1. Esegui pull `dev/0.10`, poi avvia `TEST_LIVING_WORLD.bat`. Il runner ora risolve `MJSON_REALLOC` con `realloc` **solo** dentro mjson.o del test standalone, evitando collegamenti al sottosistema I/O del gioco. Non installare librerie né modificare configurazioni del PC.
2. Verifica la presenza di `[run]` dopo `[build]`; `STRESS_RESULT=PASS` significa successo effettivo delle 70 stagioni, non soltanto del linker. In caso di fail invia il log locale `LIVING_WORLD_70_TEST.log`.
3. Il codice world e i test integrano un controllo per le gare di maggio/giugno 1996, precedenti al 1° luglio del manager. Nessuna azione manuale necessaria e nessun file save toccato dal BAT.
4. Se il test passa, prova `AVVIA_SWOS.bat` e CONTINUE CAREER dal .CAR esistente: F12/partita manuale/simulazione e save/restart. Test Windows live non ancora effettuati dall'assistente.


### 0.13.25 — regressione career world / Title debug
1. Pull e compila la nuova versione. Avvia NEW CAREER senza save, premi START e scegli subito il club: F12 WORLD deve già riportare club/leghe/incontri. Il mondo NON deve rinizializzarsi con un'accettazione di offerta.
2. Inizia NEW CAREER come disoccupato: F12 deve mostrare lo stesso mondo dal primo giorno; avanza settimane, osserva la stessa classifica prima e dopo l'accettazione del contratto. Riavvio/CONTINUE deve riprendere.
3. Da Title con autosave già presente, apri F12, premi N: appare il form NEW CAREER. Premi BACK prima di START e assicurati che CONTINUE ricarichi la vecchia carriera. Ripeti creando una nuova carriera: il nuovo AUTOSAVE.CAR deve sostituire il precedente soltanto dopo un primo salvataggio riuscito.
4. F12+N non deve avere effetto durante una partita o un menu diverso dal Title. I pulsanti Title, controller e scelta CLASSIC MODES restano invariati.
5. Controllo prestazioni manuali: confrontare F12 disattivo, stessa risoluzione, stesso VSync e medesima squadra, osservando se il rallentamento è in campo o soltanto subito dopo la fine. Invio SWOS.log in caso di problema. Nessun test Windows eseguito qui.

## 0.13.26 — verifica cronologia eventi (Windows)
1. Dopo pull e compilazione eseguire `TEST_LIVING_WORLD.bat`: nel `LIVING_WORLD_70_TEST.log` cercare `MIDWEEK_EVENT_SAVE_RELOAD=PASS`, `WORLD_EVENT_CURSOR=PASS` e `STRESS_RESULT=PASS`. In assenza di uno dei tre, riportare il log e non considerare il test superato.
2. In NEW CAREER -> UNEMPLOYED attivare F12: leggere WORLD CLOCK (data), NEXT LEAGUE EVENT (data/numero partite) e PENDING/OVERDUE. FORWARD TIME deve avanzare la data senza tornare indietro; OVERDUE deve restare 0.
3. Firmare un'offerta, salvare, tornare al Title, ricaricare. Con F12 verificare che la data mondiale e PENDING/OVERDUE persistano. Controllare la classifica prima/dopo e giocare un incontro manuale; la fisica a 50 Hz e il calendario OG delle coppe devono restare invariati.
4. Il debug controlla *solo gli eventi di lega del ledger*: non equivale a certificare cronologia condivisa per coppe e competizioni. Non dichiarare completa l'unificazione globale finche' non c'e' un test end-to-end specifico.

## Test Windows 0.13.27 — calendario, offerte globali, coppe OG
1. Dopo Pull da `dev/0.10`, compilare usando il solito `COMPILA_SWOS.bat`. Aprire `TEST_LIVING_WORLD.bat` nella stessa cartella di `AVVIA_SWOS.bat`. Il BAT deve terminare con exit code 0 e riportare *tutti* i marker: `MIDWEEK_EVENT_SAVE_RELOAD=PASS`, `WORLD_EVENT_CURSOR=PASS`, `GLOBAL_OFFERS_DISCOVERY=PASS`, `CUP_NATIVE_JOURNAL=PASS`, `CUP_ARCHIVE_70=PASS`, `STRESS_RESULT=PASS`. In caso contrario inviare `LIVING_WORLD_70_TEST.log`.
2. NEW CAREER disoccupato con nazionalita' manager qualsiasi. Quando arriva un'offerta, scegliere FORWARD TIME invece di VIEW CLUB OFFER: settimana+1, proposta sparita, partite mondiali avanzate; aspettare un'altra offerta. Verificare almeno una proposta NON inglese anche con manager inglese; se non arriva entro 30 settimane annotare paesi/club visibili in F12 e `SWOS.log`.
3. Accettare una squadra e aprire F12: gli eventi del mondo devono avere `OVERDUE 0`. Giocare o simulare una partita di coppa OG, poi controllare `OG CUPS MATCHES archived+resident`. Chiudere/riavviare, CONTINUE CAREER, confrontare il conteggio. Giocare un altro turno o una doppia sfida e verificare che il vincitore/avanzamento nel tabellone OG non cambi.
4. Il runner C verifica la PERSISTENZA del registro di coppa e un tabellone sintetico ogni stagione; non garantisce coppe offscreen durante disoccupazione. Separare ogni test automatico dal vero collaudo manageriale su Windows.

## 0.13.28 — un solo BAT e diagnosi ingresso nella carriera
1. Esegui Pull di dev/0.10, controlla BUILD 0.13.28, compila con COMPILA_SWOS.bat e avvia TEST_LIVING_WORLD.bat. Il runner conserva save/AUTOSAVE.CAR e scrive soltanto LIVING_WORLD_70_TEST.log. Richiede tutti i precedenti sei marker + DB_INVENTORY, ENGINE_UNIT_REGRESSIONS e ENGINE_SUITE con =PASS. Non assumere PASS se la compilazione del gioco fallisce.
2. Il file distingue test sintetici 70 stagioni da inventario data/TEAM.* e data_json/teams. I numeri REAL COUNTRIES e REAL TEAM FILES in F12/SWOS.log sono osservazioni sul loader autentico del mondo, NON la dimostrazione che il world scheduler faccia promozioni, coppe e carriere mondiali complete.
3. NEW CAREER -> START UNEMPLOYED: F12 diviso in due colonne. Anche con una nuova offerta, FORWARD TIME deve mantenere focus iniziale; VIEW CLUB OFFER raggiungibile col controller e col mouse. Con F12 aperto, tasti 1 WIN, 2 LOSE, 3 SACK, 4 DRAW; tasto 4 forza il risultato finale pari a 1-1 senza velocizzare una partita manuale. Verificare disattivazione con nuova pressione.
4. Dopo accettazione offerta osservare in F12 DIRECT IMPORT / PAST OG CUP EVENTS, CATCHUP HEADLESS e NATIVE REUSED/FALLBACK. Il vecchio pannello SIMULATING SEASON non deve apparire; se invece il gioco effettua ancora un replay di CPU callback, questo deve restare registrato nei contatori/log. Inviare screenshot e SWOS.log se il passaggio e' lento o il risultato/classe cambia.
5. Per una partita manuale vera, usare PLAY MATCH normalmente. Non confondere WIN/DRAW/LOSE DEBUG con un collaudo dei 50 Hz o input partita. Coppe OG: se il club partecipa, giocare/simulare e confrontare il counter prima/dopo il reload; se non partecipa segnare NO / non verificabile, non PASS.
6. Il test "promotion/relegation full pyramid" e la simulazione coppe mondiali offscreen sono in programma: il BAT li dichiara NOT_IMPLEMENTED. Nessun falso allarme rosso se questi moduli non esistono ancora, ma non dichiarare engine completo.

## 0.13.29 — Verifica Windows in un BAT
1. Pull dev/0.10, compila con COMPILA_SWOS.bat, verifica build 0.13.29.
2. Avvia TEST_LIVING_WORLD.bat: il nuovo runner deve mostrare PYRAMID_MOVEMENT_NATIVE_RULES=PASS, PYRAMID_FULL_UP_DOWN=PASS, PYRAMID_IDENTITY_SAVE_LOAD=PASS, PYRAMID_BOUNDARIES=PASS, PYRAMID_STRESS_RESULT=PASS. Il test real DB è inventario, non ancora 70 stagioni di tutte le squadre.
3. NEW CAREER START UNEMPLOYED: avanza e accetta una proposta; F12 deve confrontare WORLD DATE / OG DATE. Registra CLOCK MISMATCH e FALLBACK; ingresso NO se date/classifica/calendario sono incoerenti.
4. Avvia una vera partita manuale di carriera, F12 poi 5 per attivare FAST X4, richiudi F12. Confronta tempi, input, fisica e risultati dopo partita. FAST deve spegnersi al ritorno al calendario. WIN/LOSE/DRAW debug non sono un test del gameplay manuale.
5. Salva/ricarica dopo cambio stagione: nessun club deve sparire o comparire in due divisioni. Spareggi, coppe e paesi non testati = NON PROVATO.
6. Consegna con checklist SÌ / NO / NON PROVATO e pulsante INVIA RAPPORTO, e log se qualcosa fallisce. Non eliminare salvataggi preesistenti.

### Test Windows 0.13.30 — BAT unico
1. Pull dev/0.10 in GitKraken, verificare build 0.13.30 e compilare con COMPILA_SWOS.bat. Proteggere il save .CAR esistente.
2. TEST_LIVING_WORLD.bat: cercare STRESS_RESULT=PASS, CUP_HISTORY_70=PASS, CUP_RESULT_IDEMPOTENCE=PASS, PYRAMID_STRESS_RESULT=PASS, ENGINE_SUITE=PASS. In caso di FAIL inviare il log completo LIVING_WORLD_70_TEST.log. Le coppe dello stress sono sintetiche: testano il journal, non OG end-to-end.
3. Carriera da disoccupato: confronto classifiche mondo/OG squadra per squadra, stesso numero di partite, vittorie, pareggi, sconfitte e punti. Qualsiasi discrepanza = NO. F12 mostra eventuali errori di calendario o risultati OG non riconciliati.
4. Debug manuale in partita reale: F12+1 vittoria, F12+2 sconfitta o F12+4 pareggio, quindi F12+6 per fine immediata attraverso ESC originale. Controllare risultato finale e classifica. F12+5 x4 e un test separato.
5. Coppa OG vera: simulare e se possibile giocare; verificare risultati, turno successivo, unico incremento del journal, campione, autosave/ripresa. Se due infortunati bloccano la partita senza sostituti, annotare NO con screenshot e non aggirare la regola originale.
6. Report checklist SI/NO/NON PROVATO; inviare log e screenshot dei NO. Non promuovere a PASS una funzione non testata.

### Flusso vincolante dalle analisi 11–13 novembre 2026

Prima di aprire i sorgenti, leggere `docs/ENGINE_CODE_DISCOVERIES.md`:
il registro contiene le invarianti gia' scoperte e i bug riprodotti.
Aggiornarlo ad ogni commit tecnico pertinente. Ordine operativo:
CALENDARIO -> CAMPIONATI -> COPPE -> stress Windows integrato.
Ogni gate richiede test C con codice reale e conferma Windows dove
sono coinvolti menu/OG/save; uno stato NOT_TESTED non e' PASS.
F12+7 da calendario disoccupato deve arrivare all'anniversario di
5 anni esatto: 1 luglio 1996 -> 1 luglio 2001, non +260 settimane.
Il giorno corrente e' il civile LWE; un sortKey OG e' il primo
giorno canonico di una fascia o la prossima fixture, non un orologio
indipendente. Non inventare date/risultati per pareggiare i contatori.

### Verifica calendario 30 anni — procedura 0.13.39_01 (14 novembre 2026)

- `TEST_LIVING_WORLD.bat` esegue il nuovo stage
  `run_test_calendar_integrity_30.sh` con sorgenti del motore LWE reale:
  01/07/1996 -> 01/07/2026, Jan/Jul di ogni anno, bisestili,
  stagioni nord/sud con fixture OG, snapshot .CAR ogni confine e confronto
  identico prima/dopo ReadJson. Quattro marcatori
  `CALENDAR_30Y_*` obbligatori: assenza/mismatch = FAIL.
- Passare anche per `AVVIA_DIAGNOSTICA.bat` nell'uso reale Windows;
  F12 registra le date viste e l'analizzatore segnala un ritorno
  all'indietro con `FIRST_CLOCK_FAILURE`. 
  `RUNTIME_30_YEAR_CALENDAR_SAVE_LOAD=NOT_TESTED_IN_WINDOWS` e'
  intenzionale finche' mancano 30 anni di carriera integrata.
- Non confondere `CALENDAR_30Y_E2E=PASS` dello stage isolato C con
  successo OG/SDL Windows. Ogni FAIL va esaminato prima di procedere ai
  campionati e coppe. Non eliminare i file di salvataggio dell'utente.


### Test mirato 0.13.45_04 — prima giornata vs classifica dell'offerta
- Caso A: NEW CAREER/START UNEMPLOYED prima dell'inizio del campionato, VIEW CLUB OFFER -> VIEW COMPETITIONS. Deve mostrare un'eventuale classifica OG PL=0 quando già catturata; altrimenti nomi TEAM realmente importati e avviso `OG TABLE NOT YET CACHED`, senza punti inventati.
- Caso B: FORWARD TIME fino a quando VIEW WORLD mostra una lega con PL>0; aprire un'offerta dello stesso paese/divisione e la preview. Cercare `[og-offer-table] CAPTURE country=... division=... year=...` nel log. Se resta la lista squadre senza statistiche, confrontare l'ID del paese, la divisione e l'anno: la cache non è stata popolata dalla callback OG oppure la chiave non coincide. NON segnare PASS in base a una lega diversa.
- Caso C: salvare/ricaricare e verificare che l'offerta non mostri dati di una stagione diversa. La cache è solo RAM e non è la fonte di verità del save OG.
- Evidenza del 14/11/2026: El Salvador Tercera Division Orient PL=0 nello screenshot iniziale, PL=6 nello screenshot successivo. Prova di quel campionato, non copertura globale OG.

### Prossimo test Windows — 0.13.45_06 (DEBUG WORLD OG)
1. L'utente sincronizza `dev/0.10` con GitKraken e **compila personalmente** `SWOS_Expanded.exe` con i BAT ufficiali. La chat non deve dichiarare compilazione Windows riuscita senza il log dell'utente.
2. NEW CAREER -> START UNEMPLOYED: senza attendere offerte, selezionare DEBUG WORLD (mouse e controller). Scegliere una lega OG presente: controllare la classifica; uscire e riaprire la medesima lega **senza FORWARD TIME**. Stessi risultati, punti, partite e data; nessun evento nel log di avanzamento causato da VIEW WORLD.
3. Premere FORWARD TIME: data +7 giorni e solo gli incontri realmente previsti OG; riaprire WORLD e verificare continuità. Provare anche mondo 1996/97 e 2026/27, leghe con ID condivisi/multipli; un menu privo di snapshot non deve produrre risultati.
4. Accettare offerta dopo diverse settimane: la classifica della stessa divisione rimane identica e nel calendario club le partite anteriori devono mantenere il loro risultato (questa seconda capacità è ancora PENDENTE). Testare partita manuale e simulata senza cambi ai 50 Hz.
5. Verificare coppe: se manca un contesto residente, messaggio «OG WORLD: COMPETITION NOT RESIDENT» è corretto, ma **NON** un PASS di integrazione coppe. Save/reload, stagioni future e storico risultati sono PENDENTI.
6. Inviare `SWOS.log`, screenshot e lista SI/NO/NON PROVATO. Distinguere compile, test isolati e test Windows; evitare di cancellare i salvataggi.

### Test mirato 0.13.45_07 — menu disoccupato senza hotspot fantasma
1. Fai pull del ramo `dev/0.10` e compila tu con i BAT esistenti; al TITLE verifica BUILD 0.13.45_07, conserva salvataggi.
2. NEW CAREER -> START UNEMPLOYED: deve esserci un riquadro DEBUG WORLD nella colonna destra subito SOPRA RECORD. Azioni presenti: FORWARD TIME, BACK, RECORD, DEBUG WORLD; VIEW CLUB OFFER solo con proposta reale.
3. Prima di premere WORLD, fai click su tutte le zone che nella carriera OG corrispondono a TRANSFERS, CLUB BUSINESS, SQUAD, NEXT MATCH, VIEW COMPETITIONS e frecce. NON devono aprirsi menu né provocare crash. Ripeti con joypad e tastiera.
4. Apri DEBUG WORLD: scegli una lega residente e torna indietro. Ripeti 3 volte, alternando BACK, RECORD, WORLD. Ripeti test delle zone fantasma: deve restare impossibile aprire la SQUAD senza club.
5. Il 01 luglio PUO' mostrare P=0: premi FORWARD TIME parecchie settimane e osserva `[og-world-view] year= phase= native-matches=`, confronta partite e classifica. Apri la stessa competizione due volte SENZA avanzare: PTS, PL, date, numero di partite, RNG e risultati NON devono cambiare.
6. Prova un'offerta, rifiuta e torna al calendario; i controlli devono restare bloccati. Coppe senza contesto residente: messaggio di indisponibilita', non auto-simulazione.
7. Per ogni voce rispondi SI / NO / NON PROVATO e invia SWOS.log e screenshot del primo NO. Crash o disallineamenti sono FAIL, non dichiarare test Windows PASS perche' il codice e' stato committato.

### Prossima verifica Windows — offer OG world 0.13.45_08
1. Pull e compilazione Windows a cura dell'utente, controllando Build 0.13.45_08.
2. START UNEMPLOYED; FORWARD TIME fino a una proposta con campionato avviato. CLUB OFFER -> VIEW COMPETITIONS deve mostrare la classifica OG residente dello STESSO paese, divisione e anno (non "OG TABLE NOT YET CACHED").
3. Aprire la stessa divisione da DEBUG WORLD: PL/PTS/GF/GA e ordine devono coincidere club per club con la preview dell'offerta e con quello dopo l'assunzione; re-opening senza FORWARD TIME non può cambiare nulla.
4. All'inizio campionato una classifica corretta può avere PL=0; se non esiste snapshot la preview deve dire OG WORLD TABLE UNAVAILABLE, mai generare risultati o usare statistiche di altre divisioni.
5. Bug indipendenti ancora DA INDAGARE: crash FORWARD TIME dopo 04/11/2026, pulsante DEBUG WORLD non visibile e TOP GOAL SCORERS vuoto post-assunzione. Inviare SWOS.log/screenshot per questi.

### Windows checklist 0.13.45_08 — WORLD, season end, offers
1. GitKraken pull on `dev/0.10`; compile `SWOS_Expanded.exe` personally using the existing BAT. Check TITLE build 0.13.45_08; do not delete or overwrite old saves automatically.
2. NEW CAREER -> START UNEMPLOYED: DEBUG WORLD button visible above RECORD and functional by mouse/controller; invisible SQUAD, TRANSFERS, CLUB BUSINESS, NEXT MATCH must remain inaccessible after modal returns.
3. FORWARD TIME repeatedly beyond 04 Nov 2026; report first failure. Watch `[og-world-fixture] FINISHED` for Brazil country 46 and Sweden country 36, and absence of `INVALID`/`LOOP LIMIT`. A stopped date with `WORLD TICK FAILED` is a safe failure, **not** a functional PASS.
4. For the selected offer's country/division, compare CLUB OFFER -> VIEW COMPETITIONS against DEBUG WORLD / native OG standings: same played/points/goals by actual team ID, same season, no extra simulated matches on opening.
5. Confirm two consecutive VIEW WORLD opens and a career hire do not change the date, standings, RNG or fixtures. TOP GOAL SCORERS under the employed Career still open bug.
6. Copy `SWOS.log` lines for `[og-world-fixture]`, `[og-world]` and screenshot of the first mismatch; send SI/NO/NON PROVATO for visible button, no self-match, no crash, correct date, correct offer standings.

### Prova aggiuntiva a START — 0.13.45_08
- NEW CAREER -> START UNEMPLOYED, prima di premere FORWARD TIME: controllare il log `[og-world] START READY: date=YYYY-07-01 native-phase=... resident-leagues=...` e aprire DEBUG WORLD per Brasile/Svezia o altre leghe gennaio-giugno. Confrontare le gare OG già disputate con il calendario originale del paese, e confermare le leghe agosto-maggio ancora a zero.
- Aprire due volte la stessa lega senza FORWARD TIME: punti/gol/partite identici. Poi FORWARD TIME una settimana: nessuna seconda simulazione delle gare precedenti (e nessuna `[og-world-fixture] INVALID`).
- Se START scrive `[og-world] START FAILED`, non chiamare riuscita la creazione del mondo: inoltrare il log. Le coppe non residenti restano correttamente «non disponibili» ma da implementare.

### Checklist Windows 0.13.46 (dopo fallimento compilazione 0.13.45_08)
1. Pull `dev/0.10` con GitKraken, compilazione con BAT/MSYS2 esistenti; verificare `Build 0.13.46`. In caso di errore inviare `COMPILAZIONE_0140.log`, senza dichiarare testata la carriera.
2. NEW CAREER -> START UNEMPLOYED: log `[og-world] START READY`, inizializzazione alla data 1 luglio. In DEBUG WORLD le leghe gia' iniziate devono avere partite precedenti; quelle che iniziano dopo possono essere a zero. Re-opening senza FORWARD TIME non deve modificare dati.
3. FORWARD TIME oltre il 4 novembre: nessuna fixture duplicata/self-match dopo la fine OG, nessun `INVALID`, `LOOP LIMIT`, `WORLD TICK FAILED`; in caso contrario segnare NO e inviare log con paese/divisione/data.
4. CLUB OFFER -> VIEW COMPETITIONS: confrontare i medesimi paese/divisione/anno con DEBUG WORLD prima e dopo accettazione; controllare PL/PTS/GF/GA, storico calendario e TOP GOAL SCORERS.
5. Eseguire prove distinte 1996/97 e 2026/27, salvataggi `AUTO9697.CAR`/`AUTO2627.CAR`, senza alterare `AUTOSAVE.CAR`. Coppe residenti e persistenza completa OG ancora NON PROVATO.
6. Per ogni punto annotare SI / NO / NON PROVATO e fornire `SWOS.log` sui NO; screenshot per problemi visivi. Le prove statiche o di simulatore isolato NON equivalgono a test del gameplay Windows.

### Procedura OG-only 0.13.47 — Windows dopo GitKraken
1. Sincronizzare `dev/0.10`, compilare dal BAT ufficiale (MSYS2 già installato) e verificare `Build 0.13.47`. Lo sviluppo sorgente non equivale a build Windows.
2. Avviare **`AVVIA_DIAGNOSTICA.bat`**. Il BAT esegue AVVIA_SWOS, raccoglie `SWOS.log`, stampa rapporto `SWOS_RUNTIME_AUDIT.log` e produce `DIAGNOSTICA_SESSIONE_N.zip` quando `tar.exe` è disponibile. In carriera premere F12 (leghe residenti OG), F12+C (coppe OG residenti mancanti) e F10 (definizioni OG).
3. Prima di un FORWARD TIME, verificare 01/07/2026 Svezia >0 gare, aprire due volte la stessa divisione senza incrementi; usare BACK da un paese in DEBUG WORLD per tornare al selettore precedente, poi dalla radice a UNEMPLOYED. Audit `OG_PICKER_BACK=PASS` solo se log mostra BACK annidato seguito da POP.
4. FORWARD TIME oltre novembre/gennaio: `OG_START=PASS`, `OG_RESIDENT_LEAGUES=PASS`, osservazioni `OG_FORWARD_TIME`; nessun `[og-world-fixture] INVALID`, `LOOP LIMIT`, `WORLD TICK FAILED`. Un PASS delle date su C isolato non certifica il runtime Windows.
5. Confrontare classifica offerta, DEBUG WORLD esterno e VIEW COMPETITIONS dopo assunzione su stesso paese/divisione/anno; TOP GOAL SCORERS resta da correggere.
6. Eseguire **`TEST_OG_WORLD.bat`**, oppure il vecchio nome `TEST_LIVING_WORLD.bat` (ora wrapper, stessa suite). In `OG_WORLD_TEST.log` i test math/dati sono STATICI. In `SWOS_RUNTIME_AUDIT.log` il parser C legge soltanto il gameplay del SWOS.log corrente e la versione del sorgente. Valori EXIT 1=FAIL, 2=INCOMPLETE, 0=PASS completo quando possibile.
7. `OG_CUP_RESIDENT=MISSING_IMPLEMENTATION` è un limite reale della 0.13.47, non simulare alcuna coppa aprendo viewer e non dichiarare PASS mondiale. Salvataggio OG globale, coppe datate/qualificazioni internazionali, testa su anni multipli e TOP GOAL SCORERS restano NON PROVATI. Inviare ZIP diagnostico e `OG_WORLD_TEST.log`.

### Stress 70 stagioni OG — test Windows della 0.13.47
1. Sincronizzare e ricompilare `0.13.47`. **Proteggere i save** e iniziare una carriera di prova da UNEMPLOYED; il comando NON e' in sola lettura e avanza davvero la data di 70 anni.
2. Lanciare `AVVIA_DIAGNOSTICA.bat`, aprire F12, premere **8** (F12+8), richiudere F12 e premere una sola volta **FORWARD TIME**. L'invocazione usa il normale motore OG, non esegue un test sintetico esterno. Non interpretare un freeze come successo: occorre l'esito nel log.
3. Attendere il completamento dell'azione, chiudere normalmente il gioco, lanciare `TEST_OG_70.bat` che legge il `SWOS.log` della sessione e stampa `OG_70_NATIVE_SEASONS=PASS_TRANSITIONS_ONLY` soltanto se osserva **70 CLOSE, 70 OPEN, nessun errore OG e data civile esatta**. Inviare `DIAGNOSTICA_SESSIONE_N.zip` e `OG_70_RUNTIME_DRIVER.log` se l'esito e' diverso.
4. `PASS_TRANSITIONS_ONLY` certifica solo la prova dei passaggi delle leghe OG: coppe residenti (assenti), salvataggio completo e **pensionamenti** restano NON PROVATO. I callback di coppa legacy sono intenzionalmente saltati in questo test, non nelle carriere normali. Nessun test puo' sostituire il gameplay del manager impiegato o coprire 70 anni di mercato/retirement non ancora implementati.

### Test Windows 0.13.48 — verifica dopo GitKraken
1. Sincronizzare dev/0.10 e usare AVVIA_DIAGNOSTICA.bat; verificare build 0.13.48 e compilazione con la toolchain esistente. Non usare salvataggi importanti per lo stress.
2. UNEMPLOYED -> DEBUG WORLD: entrare in continente/paese/divisione, B (Switch Pro) deve risalire UNA pagina per pressione; ESC/BACKSPACE uguale; solo BACK dalla radice torna a UNEMPLOYED. Se fallisce cercare og-world-picker BACK/POP.
3. Con un'offerta, VIEW COMPETITIONS apre direttamente la divisione OG della squadra offerta, senza selettore paesi. Deve restituire all'offerta e preservare matchesPlayedCount, punti e gol; cercare og-offer-view RESIDENT unchanged=1 e checkpoint OFFER_BEFORE/AFTER.
4. Nuova carriera di prova disoccupato, 1 luglio, F12+8, chiudere F12, FORWARD TIME UNA volta. Il salto riparte a video un anno OG per ciclo menu; 70 STEP e 70 CLOSE/OPEN più FINISHED exact=1 sono necessari. In caso di interruzione individuare og-world-step BEGIN senza END.
5. Dopo chiusura eseguire TEST_OG_70.bat e TEST_OG_CUPS_SCORERS.bat; inviare ZIP diagnostico, SWOS_RUNTIME_AUDIT.log e OG_CUPS_SCORERS_DRIVER.log. Controlli before/after leggono solo dati OG; se cup-resident=0 => NOT_IMPLEMENTED, non PASS. Se goalStats[0] non corrisponde alla UI, indagare separatamente TOP GOAL SCORERS.
6. TEST_OG_WORLD.bat avvia anche run_test_og_runtime_parser.sh per validare il parser su fixture artificiali di PARSER. Queste non sono test di gameplay o del motore mondiale: il test reale è sul SWOS.log Windows.
7. Un PASS_TRANSITIONS_ONLY non prova coppe residenti, salvataggi di ogni contesto, pensionamenti o più stagioni con manager assunto. Il fix del salto può ridurre i blocchi ma non garantisce assenza di crash senza Windows.

### Recupero da AVVIA_DIAGNOSTICA 0.13.48 bloccato — verifica 0.13.49
- Sincronizzare `dev/0.10` con GitKraken e avviare il consueto `AVVIA_DIAGNOSTICA.bat`; questo richiede che `patch_title_b_01063.sh` riconosca sia il guard legacy sia il B/SKIP moderno con `!Expanded_UnemployedWorldPickerActive()`. Non sostituire `input_sdl.c` con la condizione vecchia per far passare il test: ripristinerebbe il bug del BACK in DEBUG WORLD.
- Se la compilazione non parte, allegare `DIAGNOSTICA_SESSIONE_N/AVVIO_COMPILAZIONE.log` e `COMPILAZIONE_0140.log`. Il log precedente fermava la procedura prima di GCC/Make: non equivale a crash eseguibile.
- Solo dopo che l'EXE 0.13.49 si avvia, verificare B un livello alla volta in DEBUG WORLD, viewer OG dall'offerta, F12+8 con 70 STEP e `TEST_OG_70.bat`/`TEST_OG_CUPS_SCORERS.bat`. I risultati del precedente 0.13.47 (crash al 2050) restano da riverificare; non dichiarare il test superato prima delle prove Windows.

### Windows verification 0.13.50 — native resident cups
1. Sync dev/0.10 in GitKraken and compile with standard scripts. Expected build 0.13.50; do not overwrite important saves.
2. NEW CAREER START unemployed; inspect F12+C and DEBUG WORLD national cups. Require cup-resident > 0 and read-only cup viewer. A definition alone is NOT a playable cup.
3. Advance into January; OG cup matches/rounds should progress with dates; opening the cup viewer must not create new matches. If the log shows og-cup-resident UNSUPPORTED SETUP, report country/slot/teams/selected.
4. On a NEW disposable test career use F12+8 once and inspect 70 annual transitions using TEST_OG_70.bat and TEST_OG_CUPS_SCORERS.bat. The latter now prints OG_CUP_NATIVE_MATCHES with before/after played counts, unsupported and fault counts. Do not consider a 70-league PASS to certify cups or entire save.
5. On accepting an offer, expect OG manager cup slots to remain authoritative; a managed cup may be logged MANAGER_SLOT_UNBOUND and absent from the resident viewer this season. This safeguards against double results but does NOT prove join/sack continuity.
6. Send DIAGNOSTICA_SESSIONE ZIP and SWOS_RUNTIME_AUDIT.log for any crash, missing cup, log mismatch, failing callback or viewer error. International cups, full cup history/trophies, save/reload and pensionments remain separate pending features.

### Test Windows 0.13.51 — unico ingresso diagnostico (sostituisce i passaggi manuali 0.13.50)
1. Sincronizzare `dev/0.10` con GitKraken e avviare **solo `AVVIA_DIAGNOSTICA.bat`** per la sessione. Compila/avvia tramite le BAT ufficiali, poi alla chiusura esegue analisi runtime, suite statica/dati, test OG 70 anni e coppe-marcatori. I test C non equivalgono a gameplay. Non serve richiamare i vecchi BAT separatamente.
2. I risultati vanno in `RISULTATI_DIAGNOSTICI/DIAGNOSTICA_SESSIONE_N/` (compilazione, AVVIO_COMPILAZIONE, SWOS.log, SWOS_RUNTIME_AUDIT, OG_WORLD_TEST, drivers/test report) e ZIP nella cartella `RISULTATI_DIAGNOSTICI/`. `LOG_PRECEDENTI` contiene eventuali log storici root: NON sono test della build corrente. Nessun save viene cancellato o spostato dal raccoglitore.
3. Da UNEMPLOYED: DEBUG WORLD, aprire nazione/competizione, premere B o ESC/BACKSPACE: una pagina per pressione, poi menu disoccupato. Anche viewer OG: prima BACK torna al selettore. Non toccare CLASSIC/input partita.
4. Offerta: VIEW COMPETITIONS apre direttamente l'elenco OG delle competizioni della squadra, paese gia' noto (nessuna scelta paese); selezionare campionato e, quando presente, coppa; BACK: viewer -> elenco -> offerta. Verificare che punti, gare, contatori goalStats, cup-resident NON cambino aprendo/chiudendo i menu. Poi accettare ed effettuare confronto.
5. TOP GOAL SCORERS: controllare la schermata prima dell'ingaggio e dopo (nomi, club, gol), poi dopo una partita del club; `[og-scorers-view]` deve indicare sorgente `RESIDENT_OG` con giocatori positivi. Il log non puo' certificare da solo che il menu sia visivamente corretto.
6. Per ciascuna coppa nazionale OG type-1, `[og-cup-coverage]` produce `expected` vs `selected` e causa; `OG_CUP_DIVERGENCE` elenca stato NON RESIDENTE, selettore incompleto, formato invalido, ecc. L'audit ha scope **solo domestic type-1 table slots 2/3**. Coppe internazionali/type-0, manager-cup bind e save completo restano NON PROVATO.
7. Stress su nuova carriera disoccupata SACRIFICABILE: F12+8, FORWARD TIME una volta, verificare 70 STEP/CLOSE/OPEN senza duplicati. Questo AVANZA realmente la carriera; il BAT non esegue 70 anni automaticamente e non altera save da solo.
8. Inviare lo ZIP della nuova sessione, screenshot marcatori prima/dopo e primo BACK/CUP difettoso. Checklist SI/NO/NON PROVATO per: compilazione, START, coppe, league-before/after-bind, offerte, marcatori, BACK, 70 anni, save/reload. Compilazione e gameplay Windows sono svolti dal tester, non dedotti da grep e test C.

**Test storici isolati:** `TEST_OG_WORLD.bat` esegue anche `run_test_legacy_suite.sh` e salva `OG_LEGACY_SUITE.log` nella sessione; `run_test11_offer.sh` con path temporaneo fisso/ASAN opzionale resta dichiarato NON ESEGUITO. Questi runner LWE sintetici possono fallire per vecchi `-Werror=unused-function`: non sono un secondo motore del gioco e **non** certificano o invalidano automaticamente il gameplay OG. Se falliscono, il log lo segnala senza falsi PASS. Il test OG ufficiale non modifica gli stati residenti durante l'esecuzione di questi binari isolati.

### 15/11/2026 — Test Windows 0.13.52, avvio unico e ZIP completo
**Dopo GitKraken, aprire soltanto `AVVIA_DIAGNOSTICA.bat`**: non serve lanciare manualmente compilazione, `TEST_OG_WORLD.bat`, `TEST_LIVING_WORLD.bat`, `TEST_OG_70.bat`, `TEST_OG_CUPS_SCORERS.bat` o le vecchie shell. Il BAT compila/avvia tramite i comandi ufficiali, poi lancia i controlli indipendenti anche se l'avvio fallisce; non modifica salvataggi.
- `RISULTATI_DIAGNOSTICI/DIAGNOSTICA_SESSIONE_N/` contiene `LEGGIMI_RISULTATI.txt`, `AVVIO_COMPILAZIONE.log`, `COMPILAZIONE_0140.log` quando GCC viene raggiunto, `ANALISI_RUNTIME_DRIVER.log`, `SWOS_RUNTIME_AUDIT.log`, `TEST_OG_WORLD_DRIVER.log`, `TEST_OG_70_DRIVER.log`, `TEST_OG_CUPS_SCORERS_DRIVER.log`, `TEST_ESEGUITI/ELENCO_TEST.txt`, `TEST_ESEGUITI/RIEPILOGO_TEST.txt` e un log per ciascun `run_test*.sh`, oltre al format audit e `LOG_PRECEDENTI/` con log della cartella radice.
- Lo `SWOS.log` preesistente viene conservato come `LOG_PRECEDENTI/SWOS_PRE_SESSIONE.log` prima della compilazione, per non far passare un precedente gameplay 0.13.50 come test della 0.13.52. Solo uno SWOS.log realmente prodotto nel nuovo avvio e copiato nella sessione conta per il parser. La diagnostica non altera, sovrascrive o cancella `save/*.CAR`.
- Script scoperti automaticamente: tutti `run_test*.sh` (anche vecchi 70 stagioni LWE, mobilita', calendario, import, test11 offerte) e `run_audit_competition_formats.sh`. `run_test_engine_suite.sh` e `run_test_legacy_suite.sh` sono aggregatori gia' espansi, `TEST_LIVING_WORLD.bat` e `audit_windows_runtime_log.sh` sono alias degli stessi test: elencati come ALIAS per evitare doppie esecuzioni. ASAN di test11 e' opzionale, stessa toolchain.
- Il parser C legge il log corrente **una volta sola**; `TEST_OG_70.bat` e `TEST_OG_CUPS_SCORERS.bat` condividono il rapporto quando eseguiti dalla diagnostica. Per il vero salto F12+8/70 anni serve comunque il test MANUALE nel gioco su nuova carriera senza club, perche' il BAT non simula il mondo.
- Al termine viene creato `RISULTATI_DIAGNOSTICI/DIAGNOSTICA_SESSIONE_N.zip` usando il `tar.exe` Windows preinstallato. Il contenuto viene elencato e lo ZIP finale controllato prima del messaggio `[ZIP OK]`; se non funziona, la cartella resta integra e viene mostrato `[ZIP FAIL]`. Non aggiungere ZIP di asset o save. Invia lo ZIP insieme a screenshot se marcatori/BACK sono ancora difettosi.
- Checklist: compilazione=SI soltanto dopo `Compilazione completata...`; avvio=SI soltanto dopo vero log e finestra Windows; scenari gameplay=SI soltanto dopo prova dell'utente; headless/legacy=PASS_ISOLATED_ONLY; prova completa OG per coppe internazionali, manager e save=NON PROVATO.

### 15/11/2026 — 0.13.53: unica diagnostica, check per bye, internazionali, SAVE/LOAD e warning
Usare **SOLO `AVVIA_DIAGNOSTICA.bat`** dopo sincronizzazione GitKraken (ramo dev/0.10), per compile/avvio e tutti i test OG attuali, log e ZIP nella stessa `RISULTATI_DIAGNOSTICI/DIAGNOSTICA_SESSIONE_N`. Il runner ora NON esegue test obsoleti sintetici LWE: li elenca `NOT_RUN_OBSOLETE`, mantenendo sorgenti e storico. `run_test_final_og_contract.sh` verifica wiring statico, NON risultati match. `COMPILER_WARNING_AUDIT.log` conta e categorizza i warning reali e segnala i packed-member come REVIEW_REQUIRED.
Dopo il gioco, leggere in `SWOS_RUNTIME_AUDIT.log`: `OG_CUP_FORMAT_RULES` e righe `OG_CUP_DIVERGENCE` (numeri del primo turno + regole OG; bye possibile non e' PASS), `OG_INTERNATIONAL_NATIONAL` (distinguere definizioni da fixture residenti), `OG_RESIDENT_SAVE_RELOAD` e `OG_RESIDENT_SAVE_CHECKPOINTS` (dati residenti prima/dopo; mai certificazione dal solo anno). `OG_SAVE_CONTINUITY_AUDIT.log` confronta presenza di snapshot da due sessioni, sempre senza dichiarare equivalenza integrale senza id/fixture/RNG.
**Prova di salvataggio:** con una carriera di prova nel mondo scelto, annotare data, competizioni, stato delle gare, marcatori e squadra; uscire dal gioco e terminare la diagnostica. Avviare una **nuova sessione dello stesso BAT**, scegliere CONTINUE nel medesimo mondo, confrontare gli stessi dati e avanzare un solo evento; caricare l'ultimo ZIP. Il BAT NON azzera mai save, NON esegue una seconda partita automatica e NON forza reset. Rifare con **l'altro mondo**, evitando sovrascritture. Per il salto F12+8 usare soltanto carriera sacrificabile (cambia realmente 70 anni).
**Coppe:** dalla UI dell'offerta o DEBUG WORLD verificare una coppa con primi turni/bye e poi gare realmente avanzate, compresa se possibile una coppa del club. Nazionali e internazionali hanno inventari dedicati ma la loro esistenza NON e' prova delle partite. Obiettivo: date, calendario, incontri, classifiche e marcatori coerenti, nessun duplicato, stesse identita' dopo cambio club e reload.

### 15/11/2026 — 0.13.54: validazione di TUTTI i paesi alla partenza
Dopo GitKraken su `dev/0.10`, usare **SOLO `AVVIA_DIAGNOSTICA.bat`**: compila/avvia l'EXE tramite toolchain ufficiale, esegue test isolati e parser, crea `RISULTATI_DIAGNOSTICI/DIAGNOSTICA_SESSIONE_N.zip`, senza cancellare o sovrascrivere save preesistenti da parte del BAT.
- In una carriera test NUOVA nel mondo 2026/27, usare START UNEMPLOYED e prima di FORWARD TIME aprire DEBUG WORLD in qualche paese che prima mostrava indisponibile (es. Albania ID0, Austria ID1, Belgio ID2; paesi africani e sudamericani nello stesso selettore). La coppa deve mostrare il tabellone native **anche con zero partite**, senza inventare score. Ritornare due volte nel viewer e verificare che dati/data non cambino. Se offre un club partecipante, aprire VIEW COMPETITIONS e confrontare con la successiva carriera.
- Fare anche START nel mondo 1996/97 con una nuova carriera di test, per verificare la compatibilita' del percorso OG storico; non sovrascrivere il save dell'altro mondo.
- Il nuovo rapporto `SWOS_RUNTIME_AUDIT.log` include `OG_CUP_START_COVERAGE` (stati a START), `OG_CUP_COUNTRY year=... country=... definitions=... resident=... unavailable=...` per ogni paese, `OG_CUP_COUNTRIES` (conteggio paesi completo), `OG_CUP_COVERAGE` (totale coppe), `OG_CUP_NATIVE_BYE_EVENTS` (validazioni native esplicite) e `OG_CUP_DIVERGENCE` (eventuali casi residui). `TEST_OG_CUPS_SCORERS_DRIVER.log` mostra gli aggregati. Il vecchio `SELECTOR_UNDERFILLED` non conta piu' automaticamente come errore **solo** quando `NATIVE_BYES_VERIFIED` dopo confronto di tutte le identita' e del primo turno OG. PASS statico degli helper e' distinto dal gameplay.
- Dopo le verifiche START, una carriera test disoccupata sacrificabile puo' eseguire F12+8 e FORWARD TIME una volta per testare 70 anni. Il test dei 70 anni deve mostrare continuita' di coppe in piu' stagioni, senza due partite uguali o cup manager duplicate; controllare `worldcup-season-counter` e riferire se resta zero (limite gia' noto, NON corretto in questa build). L'utente compila/testa; gli script non devono simulare un'altra carriera.
- Restano NON PROVATE la persistenza intera dello stato OG via SAVE/LOAD, le coppe nazionali delle nazionali e le competizioni internazionali residenti. Non usare gli esiti delle coppe nazionali per dichiararle completate.

### 15/11/2026 - unico test Windows 0.13.55, ritorno TITLE e coppe internazionali
Dopo sincronizzazione GitKraken dev/0.10 eseguire soltanto AVVIA_DIAGNOSTICA.bat. Lasciare che compili e si apra SWOS_Expanded.exe, entrare in carriera, attendere/autosalvare e tornare al Title. Il parser riporta OG_TITLE_AUTOSAVE=ON_DISK_PREFLIGHT_OBSERVED quando il file CAR e' leggibile e ha schema/kind/world compatibili, FAIL per un controllo fallito, NOT_TESTED se mai tornati al Title con una carriera salvabile. Questo NON equivale a un vero restore.
Al termine il BAT copia i CAR esistenti AUTO9697.CAR, AUTO2627.CAR e, se presente, AUTOSAVE.CAR legacy nella sottocartella SAVE_COPIE del nuovo ZIP, li confronta con fc /B e include SHA256 se disponibile con certutil. Originali immutati. Inviare ZIP privatamente: possibili nome e dati del manager.
Dall'offerta aprire VIEW COMPETITIONS in un paese con coppa residente, confrontare lo stesso tabellone in DEBUG WORLD; provare club non iscritto e tabellone con zero partite, BACK senza accettare il contratto. I dati OG non devono cambiare. Ripetere brevemente nei due mondi.
Nel rapporto OG_EXTERNAL_COMPETITION ogni torneo appare per nome: Champions/Europa/Conference, vecchie coppe UEFA, Libertadores, altri continenti, Mondiali, Europei, America/Africa/Nord America/Asia/Oceania, qualificazioni. DEFINITION_ONLY_NO_RESIDENT_FIXTURE_EVIDENCE e COUNTER_ACTIVITY_ONLY_DATED_FIXTURES_NOT_VERIFIED indicano NON PROVATO, non PASS. F12+8 prova soltanto le transizioni/competizioni tracciate, non automaticamente le coppe esterne.
Per LOAD reale chiudere completamente l'EXE e riavviare un'altra sessione dello stesso BAT, fare CONTINUE CAREER nel mondo corretto e confrontare data, risultati di campionato, coppe nazionali, marcatori e (se esistono) coppe internazionali. RESTORE_AFTER va verificato nel nuovo SWOS.log. Test manuale positivo precedente riferito dall'utente; serializzazione integrale dei contesti OG/fixture/RNG rimane NON PROVATA solo dai counters. Nessun SAVE originale deve essere cancellato.


### 16/11/2026 — prova 0.13.56 e pulizia sicura
Dopo GitKraken eseguire `AVVIA_DIAGNOSTICA.bat` per compilare con il toolchain Windows già installato; il test `run_test_mod2627_coverage.sh` controlla il manifesto Mod 4.1.2 e segnala una definizione nazionale o internazionale mancante. `MOD2627_COVERAGE=PASS_STATIC_ONLY` significa soltanto dati, NON fixture UEFA/Conti, save globale o nazionali.
In NEW CAREER -> START UNEMPLOYED verificare VIEW SQUAD durante un'offerta: deve aprire lo stesso layout della rosa assunta, con stelle, A/B/X, EXIT e menu fuori portata; provare click sui pulsanti nascosti e scambi di giocatori (devono essere bloccati), uscire e verificare che offerta/rosa/manager non cambino. Ripetere dopo ingaggio: le funzioni gestionali devono funzionare normalmente.
Prima o dopo la build, opzionalmente lanciare `PULISCI_OBSOLETI_SWOS.bat` dalla directory del repository. Prima anteprima, poi scrivere esattamente `ELIMINA` per i 10 soli asset esplicitamente obsoleti; se sono stati già rimossi da GitKraken, il BAT stampa ASSENTE. Nessuna wildcard e nessuna rimozione di stadi, salvataggi, log o RAW originali. Nessun ZIP asset richiesto.
Le routine OG extra-UEFA per avanzamento sono un'API interna preparatoria **non ancora attivata** in START/FORWARD TIME, poiché prima servono selezione reale prima stagione, fixture datate, stato salvo e handoff manager. Non scambiare la lista di 12 nomi per coppe giocabili: usare i log `[og-external-cup-inventory]` e i test runtime per diagnosticare il lavoro mancante. 

### Collaudo Windows 0.13.63 dopo sync GitKraken
Avviare solo AVVIA_DIAGNOSTICA.bat sul ramo dev/0.10. Controllare versione 0.13.63, che la compilazione raggiunga GCC/Make e che SWOS.log sia della nuova sessione. Nuova carriera di prova: B Switch Pro da DEBUG WORLD, OFFER VIEW COMPETITIONS e VIEW SQUAD, un livello per pressione senza flicker o FIRE; Champions 2026/27 START e FORWARD TIME con fase/partite/finale anno corretto; 1996/97 START sia disoccupato che occupato (ultimo marker [og-9697-start]/[og-world-step] se crash); dodici extra-UEFA distinguendo definizioni da fixture residenti. Restituire DIAGNOSTICA_SESSIONE_N.zip e screenshot primo NO. Non cancellare o sovrascrivere CAR. Audit sorgenti non sono test runtime.

### Test Windows dopo GitKraken, build 0.13.64
1. Avviare SOLO AVVIA_DIAGNOSTICA.bat e controllare che COMPILAZIONE_0140.log arrivi al compilatore GCC/Make e la versione stampata sia 0.13.64; un audit sorgente non prova il runtime.
2. Avviare una nuova carriera 1996/97 disoccupato e, separatamente, occupato: verificare il log `[world-team-source] world=9697 file=12 source=OG_BINARY`, poi vedere se [og-world-step] supera country=12 div=0 senza panic 0x8885; se emergono altri paesi riportare ultimo marker + expected/actual, senza ritoccare team count.
3. In DEBUG WORLD e VIEW COMPETITIONS dell'offerta, Switch Pro UP dalla prima voce ritorna all'ultima **visibile** della medesima lista/colonna; DOWN dall'ultima ritorna alla prima. Confermare [og-picker-wrap], tasto B un livello senza lampeggio, e CLASSIC originale invariato.
4. World 2026/27: verificare che mantenga rose e dimensioni moderne, le tre competizioni UEFA, campionati e coppe. Cercare `[world-team-source] WARNING ... LEGACY_FALLBACK`: un warning e' una possibile commistione da correggere, non un PASS.
5. Libertadores 2026/27: `[og-conti-first] entrants=NOT_MAPPED fixtures=NOT_RESIDENT` segnala onestamente lo stato ancora non collegato. Non chiedere ENGLISH.EXE, non simulare risultati retroattivi per i preliminari. Verificare lo script `run_test_mod2627_coverage.sh`: il suo eventuale PASS e' solo inventario statico. Controllare save distinti e non cancellare CAR preesistenti; consegnare il prossimo DIAGNOSTICA_SESSIONE_N.zip.


### 17/11/2026 — Decisioni e verifica Libertadores per entrambi i mondi (0.13.65, PARZIALE)
**Regola approvata:** date reali per tutte le competizioni di **entrambi** i mondi 1996/97 e 2026/27. La carriera cambia stagione il 1 luglio, ma un'edizione OG annuale può proseguire fino ad agosto o novembre. Formato storico Libertadores: 20 club, 5x4; formato 2027: 47 iscritti, 28 dirette, 19 preliminari, quattro posti gironi liberi fino a Q3. Fra i 32 club dello screenshot sono spostati a preliminari Alianza Lima, The Strongest, Juventud, Deportivo Táchira; altre quindici squadre dai rispettivi paesi completano la lista. **Nomi, source nazionale e ID OG di tutti i 47, date ufficiali 2027 e limiti** sono in `docs/LIBERTADORES_1997_2027_DECISIONS.md` e nel manifest `data_json/worlds/2627/competitions/libertadores_2027_first.json`.
**Codice 0.13.65:** reset della stagione della lega distinto dal reset totale, conservazione dello stato CT_ED attivo; lettore read-only delle date-base per singolo leg 2027. **NON ANCORA IMPLEMENTATO:** fixture reali per squadra con risultato/andata-ritorno persistente, esecutore OG giornaliero, sorteggi e save/load integrale, e collegamento completo delle due Libertadores. I dati NON sono partite giocate. Windows compilazione e gameplay NON PROVATI.



### Verifica Windows 0.13.66 dopo commit / GitKraken
Leggere prima `docs/WORLD_PRIORITY_20261117_BUILD_01366.md`. Non confondere **supporto a partire senza panic** con **campionato Finlandia funzionante**: la preflight 96/97 deve loggare `[og-native-league-preflight] UNSUPPORTED country=12 division=0 expected=12 actual=14` e NON generare incontri; verificare eventuali altri file con mismatch e report di scope. Le 0.13.65 sessioni 21/22 NON collaudano la 0.13.66. Picker DEBUG WORLD/OFFERTA: scorrere con UP/DOWN ogni voce visibile comprese UEFA e Libertadores, verificare focus e B, senza attribuire fixture a una competizione NOT_RESIDENT. VIEW SQUAD in OFFERTA: X popup, B chiude solo popup, nuova pressione B torna al menu offerta. No restart ancora NON implementato: non considerare testato. Chiedere un nuovo DIAGNOSTICA_SESSIONE_N.zip dopo test sul PC Windows; non cancellare CAR o usare altra toolchain.


### 17 novembre 2026 — 0.13.67: NO RESTART e doppio static namespace preparato al loading (Windows NON PROVATO)
Decisione già approvata e ora implementata nel codice del selettore NEW CAREER: preparare i registri statici di **entrambi** i mondi durante il LOADING, ma mantenere **un solo insieme di puntatori OG attivo**; nessuna partita simulata durante la selezione. `world_switch.c` salva 290 aree dei record/tabelle native prima della sovrascrittura e dopo l'installazione completa dei dati moderni; include offsets squadra, paese, competizioni, playoff, regioni e `_careerContests`. `competition_data.c` aggiunge restore del registro compId per non mescolare i template dinamici moderni con i 1996/97. Il switch UI usa `WorldSwitch_SelectBeforeCareer`, invalida la cache TEAM, non chiama più CreateProcess/exit e mantiene salvataggi di mondi distinti. Modello confrontato con MOD 4.1.2 (`careerworld.py::w_ns_set`, `swe.c::reg_enter/reg_leave`); Expanded deve salvare più registri dei soli tre della MOD. Guida/audit dettagliati in `docs/WORLD_SWITCH_NO_RESTART_20261117.md`. **Compilazione e gameplay Windows NON PROVATI; non equivale a completamento mondiale OG, né a risoluzione della Finlandia 96/97.**
