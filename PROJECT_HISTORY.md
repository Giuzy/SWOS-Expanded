# Storico documentale SWOS Expanded 0.10.38

Archivio dei testi delle versioni precedenti, conservato per riferimento.
Non sostituisce il codice attuale.


---

## LEGGIMI.txt

```text
SWOS EXPANDED 0.10.25 - SPLASH / MORALE / BUSINESS
=================================================
1. Estrai TUTTO in una nuova cartella.
2. Doppio clic su AVVIA_SWOS.bat (compila con MSYS2 e avvia).
3. La prima compilazione richiede qualche minuto.
4. Leggi LEGGIMI_0125_IT.txt per modifiche e test.
5. Se la compilazione fallisce, invia COMPILAZIONE_0125.log.

```

---

## LEGGIMI_0103_IT.txt

```text
SWOS EXPANDED 0.10.3 - CORREZIONI CALENDARIO, CLUB BUSINESS E BACK
================================================================

INSTALLAZIONE / AVVIO
1) Estrai lo ZIP in una NUOVA cartella, senza sovrascrivere la 0.10.2.
2) Apri SWOS_Expanded_0_10_3 e fai doppio clic su AVVIA_SWOS.bat.
3) Il BAT compila automaticamente con MSYS2 MINGW64 al primo avvio.
4) Ai successivi avvii riusa SWOS.exe. Nessuna patch manuale.

MODIFICHE
- Calendario normale: data spostata a y=171, nella striscia libera fra
  l'ultima partita (anche di coppa) e i pulsanti. Stesso formato data.
- Disoccupazione: riusa la stessa schermata calendario originale (clone
  del Menu_Contest, nessuna seconda interfaccia). Tre riquadri superiori,
  area centrale e griglia 3x3 dei pulsanti sono preservati.
- I pulsanti relativi a club/rosa/competizioni restano visibili, ma
  diventano grigi e non selezionabili finche' non hai un club.
- PLAY MATCH diventa FORWARD TIME; l'area partite mostra le settimane
  con la scritta WAIT FOR A CLUB OFFER, senza inventare partite.
- Quando arriva un'offerta, VIEW CLUB OFFER diventa selezionabile.
- BACK dal calendario del disoccupato torna alla configurazione carriera.
  Corretto il blocco input causato dalla protezione anti-doppio-clic
  quando FIRE restava premuto dopo la chiusura di un menu.
- Club Business: BACK e' ora a destra della data, sopra i conti, non
  sovrapposto a MONTHLY/ANNUAL. Tutte le otto strutture possono
  raggiungerlo direttamente con la navigazione direzionale.
- Nazionalita': conservata la correzione funzionante della 0.10.2.
- View Squad e View League: conservate le correzioni precedenti.

VERIFICHE ESEGUITE SUI SORGENTI
- GCC controllo sintattico: menu_data.c, menu_system.c,
  expanded_career.c, expanded_economy.c, career_init.c, blitter.c.
- test11_offer (AddressSanitizer/UBSan): PASS.
- test12_ui_layout: PASS.
- test08_club: PASS.
- test12_rect_clip: PASS (build SDL).

LIMITI ATTUALI
- Qui non e' stato compilato ne' provato un SWOS.exe Windows.
- Il calendario disoccupato fa avanzare le settimane e genera offerte,
  ma NON simula ancora le vere classifiche e i risultati di tutti i
  campionati prima dell'assunzione. Questo richiede un intervento
  ulteriore sul motore carriera e NON e' dichiarato come completato.
- Il tempo trascorso prima dell'assunzione non viene ancora applicato
  alla giornata del campionato nel motore carriera: e' una modifica
  separata necessaria per entrare a stagione gia' iniziata.

PROBLEMI
- Compilazione: invia COMPILAZIONE_0103.log dalla cartella del gioco.
- Crash o blocco: invia SWOS.log da Documenti\SWOS.
- Segnala schermata e pulsante esatti, preferibilmente con screenshot.

```

---

## LEGGIMI_0104_IT.txt

```text
SWOS EXPANDED 0.10.4 - CLUB BUSINESS PIXEL ART
=============================================

AVVIO SU WINDOWS x64
1. Estrai lo ZIP in una cartella NUOVA, senza sovrascrivere le versioni precedenti.
2. Apri SWOS_Expanded_0_10_4.
3. Fai doppio clic su AVVIA_SWOS.bat.
4. La prima volta il BAT compila SWOS.exe usando MSYS2 MINGW64 (gia' installato
   se hai usato le versioni 0.10.2/0.10.3); le volte successive lo avvia.
5. Non devi applicare patch, copiare file o installare Python.

NOVITA'
- Nuovo CLUB BUSINESS in pixel art, ispirato all'anteprima approvata.
- Il disegno dello stadio cambia davvero su CINQUE livelli (5.000-90.000 posti).
- Rimossi i comandi a freccia per cambiare stadio: si vede solo quello del club.
- I sette riquadri FITNESS, TRAINING, SCOUTING, TRAVEL, MEDICAL, OFFICE,
  YOUTH sono veri tasti selezionabili, con mouse e tastiera/joypad.
- Il pulsante UPGRADE STADIUM apre la scheda dello stadio; il comando UPGRADE
  nella scheda conferma l'acquisto e usa il saldo effettivo.
- BACK e' in basso a sinistra, selezionabile e torna al calendario precedente.
- Data settimanale vicino a FINANCES; niente fascia data dedicata in basso.
- Pannelli allargati per Board, tifosi, finanze, saldo e livelli infrastrutture.
- Tutto usa i dati gia' presenti in SWOS Expanded 0.10.3, senza inventare
  risultati o modificare il motore delle partite.

VERIFICHE EFFETTUATE
- GCC: controllo sintattico su src/menu_data.c e src/expanded_economy.c.
- Test 0.10.4: 9 pulsanti cliccabili, posizione, navigazione, cinque immagini
  stadio distinte, test framebuffer e acquisto stadio con ASan/UBSan: PASS.
- Test offerte 0.10.1 e date/esonero 0.10: PASS.

IMPORTANTE
- Non e' stato compilato ne' provato qui un EXE Windows x64. Il BAT compila
  i sorgenti sul tuo PC e crea SWOS.exe al primo avvio.
- La simulazione completa dei campionati mentre sei disoccupato NON e' ancora
  implementata. Ci occuperemo della simulazione dopo la grafica dei menu.
- Le grafiche sono in data/expanded e devono restare in quella cartella.

IN CASO DI PROBLEMI
- Se la compilazione fallisce: invia COMPILAZIONE_0104.log dalla cartella gioco.
- Se il gioco crasha: invia SWOS.log da Documenti\\SWOS.
- Se un tasto non risponde: indica se mouse o tastiera e allega uno screenshot.

```

---

## LEGGIMI_0106_IT.txt

```text
SWOS EXPANDED 0.10.6 - PIXEL ART, MENU ORIGINALI
================================================

AVVIO WINDOWS x64
1. Estrai tutto lo ZIP in una cartella nuova (non sovrascrivere la 0.10.5).
2. Apri SWOS_Expanded_0_10_6.
3. Fai doppio clic su AVVIA_SWOS.bat.
4. Il primo avvio compila SWOS.exe con MSYS2 MINGW64 (gia' installato sul tuo PC).
   I successivi avvii usano lo stesso EXE, senza ricompilazione.
5. Non devi applicare patch o spostare i file grafici.

SCHERMATE 0.10.6
- TITOLO: allenatore e squadra in pixel art, logo completo senza riquadro,
  niente testa duplicata. Pulsanti compatti in una colonna a destra.
  Tutte le 16 voci/callback originali sono mantenute nel menu sorgente.
  La Hall of Fame appare una sola volta; il suo pulsante reale e' la voce 5.
  Continue/Replay e Save Game compaiono soltanto con una competizione attiva.
  Multiplayer compare soltanto nelle build con NETPLAY abilitato (NETPLAY=0
  in questo BAT, come nella 0.10.5). Non si inventano pulsanti alternativi.
- NUOVA CARRIERA: frame spostato 11 pixel a sinistra, bordi interamente
  visibili. Restano sesso/titolo, nome, cognome, nazionalita', ruolo, eta',
  fama, scelta squadra/disoccupazione, Back e l'azione Load Liverpool.
- CALENDARIO: pannelli centrati, partita sullo sfondo, data nella striscia
  dedicata sopra le partite. Rimangono il calendario nativo con scorrimento,
  i comandi reali e il calendario del disoccupato con Forward Time.
- CLUB BUSINESS: INVARIATO rispetto alla 0.10.5 approvata, con Back in alto,
  stadio dinamico a cinque livelli, sette strutture selezionabili, finanze,
  board, tifosi e miglioramenti.

VERIFICHE
- Controllo sintattico GCC dei file modificati: superato.
- Test del framebuffer con ASan/UBSan: superato.
- Test dei 16 callback originali e visibilita' contestuale: superato.
- Test delle offerte e navigazione della versione precedente: superato.
- Menu originali src/menu_data.c e src/menus.c identici alla 0.10.5.
- Il nuovo eseguibile Windows NON e' precompilato: viene creato dal BAT.
  Il test interattivo completo su Windows non e' stato effettuato qui.

IN CASO DI PROBLEMI
- Se il BAT non compila, invia COMPILAZIONE_0106.log.
- Se il gioco si blocca, invia SWOS.log dalla cartella Documenti\SWOS.
- Se un pulsante non risponde, indica quale e se usavi mouse o tastiera.

NOTA SULLA SIMULAZIONE
Questa release aggiorna interfaccia e hitbox. Non introduce ancora la
simulazione completa delle competizioni durante la disoccupazione o il nuovo
motore economico, che implementeremo successivamente.

```

---

## LEGGIMI_0107_IT.txt

```text
SWOS EXPANDED 0.10.7 - MENU COMPATTI E FONT ORIGINALE
=======================================================

NOVITA' RISPETTO ALLA 0.10.6
- TITOLO: stessi 16 slot/callback del menu originale, Hall of Fame dedicato.
  Il duplicato Hall resta nascosto. Pulsanti 84x9 pixel, sempre allineati
  al bordo inferiore, senza righe vuote dovute ai tasti condizionali.
- FONT: il testo di titolo, carriera e calendario usa le funzioni originali
  GetTextWidth / DrawMenuText* e il charset originale di SWOS.
- CARRIERA: form circa dimezzato, a destra, con AGE (18-70). La scena
  dell'allenatore e' ritagliata mantenendo le proporzioni, non schiacciata.
- CALENDARIO: riquadri piu' centrati; eliminati i due grossi pannelli
  originali sovrapposti alla grafica. Data dentro il textbox superiore.
  La lista delle partite, la navigazione e i callback originali restano.
- DISOCCUPATO: stesso calendario, data derivata dalle settimane di attesa;
  Forward Time e il testo Wait for a Club Offer restano funzionanti.
- CLUB BUSINESS: IDENTICO alla versione approvata (Back in alto,
  infrastrutture, finanze, cinque immagini dello stadio).
- REDRAW: aggiornamento dei valori e della selezione anche quando il gioco
  ridisegna un solo campo (nazionalita', fama, eta', ecc.).

COME AVVIARE (WINDOWS x64)
1. Estrai tutto in una cartella NUOVA, senza mescolare file di vecchie build.
2. Apri la cartella SWOS_Expanded_0_10_7.
3. Doppio clic su AVVIA_SWOS.bat.
4. La prima volta il BAT compila SWOS.exe con MSYS2; ai successivi avvii
   usa il file compilato. Non servono patch manuali.

IMPORTANTE
Il pacchetto contiene i sorgenti, gli asset e i dati del gioco, piu' il BAT
che compila e avvia. NON contiene un nuovo SWOS.exe gia' compilato: la
compilazione Windows completa e la prova interattiva avvengono sul tuo PC.
In caso di errore invia COMPILAZIONE_0107.log, o SWOS.log in caso di crash.

VERIFICHE ESEGUITE
- Controllo sintattico GCC sulle sorgenti dei menu modificati: PASS
- Smoke test del framebuffer con AddressSanitizer/UBSan: PASS
- Test delle 16 azioni originali e della disposizione dei menu: PASS
- Test offerte / disoccupazione: PASS
- Integrita' degli asset Club Business e 5 stadi: PASS
- Build Windows x64 end-to-end: NON VERIFICATA in questo ambiente.

```

---

## LEGGIMI_0108_IT.txt

```text
SWOS EXPANDED 0.10.8 - CORREZIONI INTERFACCIA
===========================================

CAMBIAMENTI
- TITOLO: 15 opzioni originali condizionali (Hall of Fame compreso),
  tasti piu larghi/alti (99x11 pixel) ancorati al bordo inferiore,
  scritte centrate sul font SWOS originale, senza ombra del vecchio menu.
- NUOVA CARRIERA: immagine dell'allenatore estesa a tutto lo sfondo
  senza deformazioni; tutti i campi in sequenza verticale (TITLE,
  NAME, SURNAME, NATION, ROLE, AGE 18-70, FAME). Le funzioni e i buffer
  originali restano. Nome e cognome: conferma con INVIO; ESC annulla.
  Il click che apre l'editor non lo chiude subito. Durante la digitazione
  viene mostrato il testo in corso, non il valore precedente.
- CALENDARIO: nomi squadra adattati alla larghezza del font originale,
  colonna avversario spostata a sinistra e bordo destro ripristinato
  sopra eventuale testo lungo. Dati reali delle squadre invariati.
- CLUB BUSINESS: stessi riquadri, tasti e 5 immagini di stadio,
  font sostituito con quello originale di SWOS per maggiore leggibilita.
- DISOCCUPAZIONE: eliminati i pulsanti grigi inutilizzati, pannello
  centrale piu piccolo, 5 righe settimanali piu distanziate.
  FORWARD TIME, BACK e VIEW CLUB OFFER (solo quando c'e un'offerta)
  mantengono i callback reali. Nessuna simulazione di lega aggiunta.

INSTALLAZIONE WINDOWS X64
1. Estrai lo ZIP in una cartella NUOVA.
2. Apri SWOS_Expanded_0_10_8 e fai doppio clic su AVVIA_SWOS.bat.
3. Il BAT usa MSYS2 per compilare SWOS.exe al primo avvio.
4. Per problemi di compilazione invia COMPILAZIONE_0108.log;
   per crash o bug di gioco invia SWOS.log e uno screenshot.

VERIFICHE
- Test sintattici GCC: superati per i moduli modificati.
- Test C con ASan/UBSan: framebuffer, tasti, focus e stadio: superati.
- Test automatico offerte/disoccupazione: superato.
- Compilazione Windows completa e test interattivo: NON eseguiti qui.
- Non e incluso un EXE precompilato; lo genera il BAT su Windows.

LICENZE
OpenSWOS e distribuito sotto GPL; SDL2 e dati originali conservano
le rispettive licenze e i diritti dei rispettivi titolari.

```

---

## LEGGIMI_0109_IT.txt

```text
SWOS EXPANDED 0.10.9 - PIXEL ART: CALENDARIO, BUSINESS E CARRIERA
=================================================================

MODIFICHE

1. TITOLO
   - Il menu del titolo rimane quello approvato nella 0.10.8.
   - 15 opzioni e relativi callback originali, Hall of Fame incluso.

2. CALENDARIO
   - Corretta la barra blu delle partite: termina dentro il pannello.
   - Sistemati i veri campi dei nomi delle squadre (39 e 43), senza
     toccare il contenuto dei nomi reali o i risultati delle partite.
   - I nomi lunghi sono accorciati SOLO a video con il font SWOS.
   - Il tasto VIEW COMPETITIONS ora mostra COMPETITIONS per non tagliare
     il testo, ma richiama la stessa identica funzione originale.
   - TRANSFERS e JOB OFFERS conservano gli stati e i callback originali:
     NO JOB OFFERS resta non selezionabile quando non esistono offerte;
     TRANSFERS resta non selezionabile quando le regole SWOS lo vietano.
     I tasti disabilitati ora hanno comunque testo leggibile.

3. CLUB BUSINESS
   - Eliminati EUR e GBP dalla grafica delle cifre: restano numeri,
     segni e separatori. La valuta reale e il saldo non cambiano.
   - BOARD: percentuale reale (0-100%) e faccina pixel art variabile.
   - FANS: percentuale reale (0-100%), faccina e numero tifosi.
   - Rimossi vecchi ritratti e coriandoli per evitare sovrapposizioni.
   - Aggiunto il numero settimana accanto a FINANCES / WEEK.
   - Stadi, upgrade, costi reali, selezione e BACK invariati.

4. NUOVA CARRIERA
   - Rimossa la striscia d'ombra traslucida dietro al menu; il bordo
     si appoggia direttamente sull'illustrazione senza stacco.
   - Input nome/cognome, navigazione, font e funzioni invariati.

5. DISOCCUPAZIONE
   - Stesso aspetto e funzioni della 0.10.8, senza pulsanti morti.

AVVIO WINDOWS X64

1. Estrai lo ZIP in una cartella NUOVA (non sovrascrivere la 0.10.8).
2. Apri SWOS_Expanded_0_10_9 e fai doppio clic su AVVIA_SWOS.bat.
3. Il BAT installa/usa MSYS2 e compila SWOS.exe al primo avvio.
4. Se la compilazione fallisce, invia COMPILAZIONE_0109.log.

TEST E LIMITI

- Verificati i sorgenti modificati con GCC -fsyntax-only.
- Superati i test ASan/UBSan su framebuffer, faccine, settimana,
  menu carriera, tasti disabilitati e clipping calendario.
- Superati i test di navigazione, offerte, stadi e callback originali.
- La compilazione completa Windows e la prova manuale su Windows non
  sono state eseguite in questo ambiente.
- Il pacchetto contiene sorgenti, dati e BAT: NON un EXE precompilato.

LICENZE

Codice OpenSWOS secondo GPL; SDL2 e dati SWOS mantengono le proprie
licenze e i rispettivi diritti.

```

---

## LEGGIMI_010_IT.md

```text
> **Versione corrente: 0.10.13** — per l’avvio usa `AVVIA_SWOS.bat`.
> Le istruzioni storiche riportate sotto si riferiscono alla prima 0.10.
> Leggi `LEGGIMI_0113_IT.txt` per le modifiche più recenti.

# SWOS Expanded 0.10 — test stabilita Windows x64

## INSTALLAZIONE
Estrai lo ZIP in una cartella NUOVA e avvia `AVVIA_EXPANDED_010.bat`.
Non sovrascrivere 0.9, 0.6 o i salvataggi. SDL2.dll e i dati sono inclusi.

## CORREZIONI IMPLEMENTATE
- CLUB OVERVIEW: gli 8 riquadri sono ora hitbox trasparenti; il motore
  ripristina la trasparenza a ogni redraw e dopo il ritorno da una facility.
  Eliminati i riquadri blu opachi sopra la grafica e le finanze.
- CARRIERA: tolta la seconda scritta UNKNOWN in fondo sopra EXIT e nascosta
  la vecchia frase FINANCES/FANS. La nota rimane sotto il titolo CAREER.
- HALL OF FAME: eliminato il codice che nascondeva per errore LOAD OLD
  COMPETITION e altre voci del menu principale al ritorno.
- DISOCCUPAZIONE: la schermata ora clona il VERO menu contest/calendario
  originale di SWOS, con le stesse posizioni dei pulsanti e lo stesso
  pannello; sostituisce soltanto il testo della partita e l'azione centrale.
  Le azioni relative al club sono invisibili e non selezionabili.
- OFFERTA: VIEW SQUAD mostra i 16 calciatori del file originale del club;
  VIEW LEAGUE mostra i club della stessa divisione; BACK ritorna all'offerta.
  Prima di accettare viene validato l'ID della squadra nel file TEAM.
- DATA: formato giorno/settimana/mese/anno, partendo dal 1 agosto 1996;
  il calendario in carriera usa l'indice settimanale finanziario SWOS,
  quello da disoccupato usa il contatore FORWARD TIME.
- CLUB: data uguale a quella del calendario, stime settimanali, mensili e
  annuali delle entrate/uscite. Sono stime, non addebiti ripetuti.
- ESONERO: due parametri persistenti BOARD e FANS. Se entrambi molto bassi
  (board <=12 e fans <=20, oppure board <=5 e fans <=35) per tre partite
  valutate consecutive, scatta il flag di esonero del motore originale.
  Il contatore si azzera quando la situazione migliora.
- Testi nuovi del gioco in inglese, grafica originale pixel art SWOS.

## LIMITI IMPORTANTI (NON IMPLEMENTATI IN QUESTA VERSIONE)
- Le competizioni NON vengono ancora simulate dal motore originale durante
  la disoccupazione. Accettando un club, il motore della carriera parte ancora
  dall'inizio della stagione: NON e' un vero ingresso a campionato iniziato.
  Questo richiede una sincronizzazione piu' profonda dei calendari e dei
  risultati di tutti i campionati, da realizzare senza compromettere le partite.
- La data in carriera usa la settimana finanziaria disponibile; non e' ancora
  il calendario giornaliero ufficiale delle singole leghe.
- La nuova logica di esonero usa il flag SWOS; la schermata di comunicazione
  del board e' quella originale e richiede verifica sul PC Windows.
- Le statistiche della schermata VIEW LEAGUE sono per ora l'elenco dei club,
  non una classifica simulata.

## VERIFICHE
- Cross-compilazione Windows PE32+ x86-64: OK.
- Test nativo C per calendario gregoriano e doppia soglia di esonero: OK.
- ZIP verificato CRC e contenuti: OK.
- NON e' stato possibile eseguire interattivamente su Windows; non e'
  corretto dichiarare risolto il crash fino al test dell'utente.
  Se l'offerta continua a mandare in crash, invia SWOS.log e la squadra scelta.

## PROVENIENZA
OpenSWOS (Ben Baker e contributori, GPL), asset SWOS originali con diritti
ai rispettivi titolari, SDL2 secondo la sua licenza.

```

---

## LEGGIMI_0110_IT.txt

```text
SWOS EXPANDED 0.10.10 - CALENDARIO COMPATTO / PIXEL ART
========================================================

NOVITA

1. TITOLO
   Nessuna modifica. Rimangono tutte le voci e i callback originali
   (Hall of Fame, competizioni, editor, opzioni, carriera, ecc.).

2. CLUB BUSINESS
   Unica modifica: "UPGRADE COST" diventa "UPGRADE" nel pannello dello
   stadio. Nessuna modifica a costi, finanze, faccine, stadi o navigazione.

3. CALENDARIO
   - Le partite sono ora disegnate UNA VOLTA per riga, usando i dati
     originali SWOS: non si sovrappongono piu nomi, V, (H)/(A) e barre blu.
   - "DIVISION THREE" diventa "D. THREE" (analogamente alle altre divisioni).
   - Il nome della competizione e il turno di coppa stanno su una riga,
     per esempio "L. CUP R2 L1".
   - (H)/(A) e vicino al separatore V, con colonne squadra distinte.
   - I nomi lunghi vengono abbreviati solo a video; i nomi nei dati
     della carriera NON vengono modificati.
   - Frecce rosse spostate dentro il riquadro, con i callback di scroll
     originali. L'area di ridisegno copre tutte le righe, anche in basso.
   - Rimossa la striscia grigia a destra; sfondo e pannello arrivano
     fino al bordo.
   - TRANSFERS, JOB OFFERS, PLAY MATCH, NEXT MATCH e tutti gli altri
     tasti conservano esattamente gli stati e i callback originali.

4. NUOVA CARRIERA
   - Rimossa la barra bianca a sinistra dell'immagine.
   - Rimossa la barra grigia decorativa sotto BACK.
   - Formulario leggermente abbassato, con BACK a filo del bordo
     inferiore e tutti i campi editabili invariati.

5. DISOCCUPAZIONE
   - Lo sfondo riempie anche il margine destro, senza barra grigia.
   - FORWARD TIME, VIEW CLUB OFFER e BACK conservano i callback esistenti.

AVVIO WINDOWS X64

1. Estrai lo ZIP in una cartella NUOVA.
2. Apri SWOS_Expanded_0_10_10.
3. Doppio clic su AVVIA_SWOS.bat.
4. Il BAT usa MSYS2 e compila il gioco al primo avvio.
   Non devi applicare patch.

VERIFICHE
- GCC -fsyntax-only sui sorgenti modificati (per menus.c e stato usato
  uno shim SDL minimo, dato che qui non sono installati gli header SDL2).
- Test UBSan: club business, calendario, faccine, nomi e coppe, pulsanti,
  framebuffer, navigazione, offerte.
- Test statici: menu originali e asset del titolo/stadi identici alla 0.10.9.
- NON e stato possibile compilare o eseguire l'EXE completo Windows in
  questo ambiente. Il pacchetto contiene sorgenti, dati e BAT, NON un EXE
  gia compilato.

Se il BAT fallisce invia COMPILAZIONE_0110.log.
Se il gioco si avvia ma qualcosa si blocca invia SWOS.log e screenshot.

```

---

## LEGGIMI_0111_IT.txt

```text
SWOS EXPANDED 0.10.11 - MENU PIXEL ART
======================================

1. Estrai lo ZIP in una NUOVA cartella.
2. Avvia AVVIA_SWOS.bat (doppio clic).
3. Il BAT usa MSYS2 e compila SWOS.exe al primo avvio.

NOVITA
- Titolo: invariato.
- Nuova carriera: frame spostato a destra e a filo bordo inferiore,
  titolo NEW CAREER verde pieno, testi dei campi centrati meglio.
- Calendario: eliminata la banda grigia a destra nell'immagine;
  pannello centrale a x=5..315, centrato nello schermo da 320px.
- Disoccupazione: eliminata la banda grigia, pannello x=19..301
  centrato; tasti FORWARD TIME e BACK e logica delle offerte invariati.
- Club Business: soltanto il titolo COSTS al posto di UPGRADE
  sopra il costo dello stadio. Il tasto UPGRADE resta invariato.

Tutte le azioni, callback, nomi, data, offerte e upgrade restano
collegati al motore precedente; questa release modifica solo UI.

LIMITI
Il BAT compila l'EXE sul PC Windows. La compilazione Windows e il
comportamento interattivo non sono verificati in questo ambiente.
Se la compilazione fallisce, invia COMPILAZIONE_0111.log.

```

---

## LEGGIMI_0112_IT.txt

```text
SWOS EXPANDED 0.10.12 - EDITOR E CALENDARIO
==============================================

1. Estrai lo ZIP in una NUOVA cartella.
2. Fai doppio clic su AVVIA_SWOS.bat.
3. MSYS2 compila SWOS.exe al primo avvio.

NOVITA
- Nuova carriera: conferma NAME e SURNAME con INVIO.
  Il tasto INVIO funge anche da FIRE in SWOS; ora l'input
  confermato non si riapre a causa del tasto ancora premuto.
  ESC continua ad annullare l'inserimento.
- Calendario: colonna prima squadra avvicinata alla divisione.
  I nomi lunghi sono abbreviati con puntini.
- Il pannello delle partite si ridimensiona automaticamente
  secondo il numero di incontri realmente disponibili (1-14).
  Le frecce di scorrimento restano all'interno del pannello.
- Titolo, Club Business, disoccupazione e azioni delle partite:
  invariati rispetto alla 0.10.11.

VERIFICA
I test automatici controllano la geometria e la conferma di
NAME/SURNAME. L'esecuzione su Windows va verificata sul PC.
Se la compilazione si ferma invia COMPILAZIONE_0112.log.

```

---

## LEGGIMI_0113_IT.txt

```text
SWOS EXPANDED 0.10.13 - CALENDARIO DEFINITIVO
===========================================

AVVIO (Windows x64)
1. Estrai questo ZIP in una nuova cartella.
2. Apri la cartella SWOS_Expanded_0_10_13.
3. Doppio clic su AVVIA_SWOS.bat.
   Al primo avvio il BAT usa MSYS2 per compilare SWOS.exe.
   Ai successivi avvii parte direttamente.

CORREZIONE 0.10.13
- Calendario: il vecchio limite logico di altezza bloccava la
  visualizzazione dopo circa 8 righe nonostante le partite fossero
  disponibili. Ora possono essere mostrate fino a 14 partite per pagina.
- Quando le partite sono meno di 14, il pannello si accorcia per
  terminare poco sotto l'ultima riga. Nessun grande spazio vuoto.
- Frecce rosse e zona cliccabile si adattano all'altezza del pannello.
- Resta invariato tutto il resto: Title, New Career, Club Business,
  Unemployed, testo e logica delle partite, trasferimenti e salvataggi.

NOTA
La compilazione Windows e la verifica manuale sul PC restano necessarie.
Se non parte, invia COMPILAZIONE_0113.log.

```

---

## LEGGIMI_0114_IT.txt

```text
SWOS EXPANDED 0.10.14 - SCHERMATA OFFERTA
=======================================

INSTALLAZIONE E AVVIO (Windows x64)
1. Estrai lo ZIP in una cartella nuova.
2. Apri SWOS_Expanded_0_10_14.
3. Avvia AVVIA_SWOS.bat.
   Il BAT ricompila SWOS.exe da sorgente con MSYS2 al primo avvio.
   Se la compilazione non parte, invia COMPILAZIONE_0114.log.

NOVITA
- CLUB OFFER: nuovo sfondo pixel art centrato (manager e presidente al
  tavolo, contratto da firmare), palette del gioco, nessuna banda laterale.
- Club, paese, continente e divisione ben visibili.
- Offerta di stipendio SETTIMANALE basata su valore medio dei giocatori,
  livello del campionato e fama del manager.
- +/- cambiano la richiesta a scatti, fra offerta base e 2 volte la base.
- ACCEPT a stipendio base accetta. Se chiedi di piu, la societa puo
  rifiutare: dopo cinque rifiuti l'offerta decade e devi aspettare.
  UNKNOWN ha meno potere contrattuale di un allenatore famoso.
- Lo stipendio concordato viene convertito in annuale e mantenuto dopo
  l'inizializzazione della carriera, senza confonderlo con i fondi trasferimenti.
- VIEW SQUAD apre la schermata SWOS di consultazione della rosa usando
  una copia dei dati della squadra, senza caricare file durante il menu.
- VIEW COMPETITIONS sostituisce VIEW LEAGUE: anteprima pre-carriera
  di campionato e coppe. Il motore delle competizioni completo si avvia
  solo dopo la firma: risultati e classifiche non sono ancora disponibili.
- Bottoni nello stile del titolo, con navigazione da tastiera/controller.
- Calendario: copia l'intero frame durante lo scroll, compresi i 6 pixel
  inferiori che causavano clipping intermittente.
- Accettazione del lavoro: non ridisegna per un frame la schermata
  disoccupato/career prima dell'apertura della carriera.
- Title, Career, Club Business e logica delle partite invariati.

TEST
- Sintassi GCC dei moduli modificati: OK.
- Test test11_offer.c: 5 rifiuti, base salary, +/- e limite richiesta: OK.
- Test test19_ui.c: disegno offerta, frame, tasti e guardie memoria: OK.
- Test Python per layout, callbacks e calendario: OK.
- Compilazione Windows x64 e test interattivo completo: NON eseguiti qui.

Se c'e un errore, invia il log COMPILAZIONE_0114.log e una schermata.

```

---

## LEGGIMI_0115_IT.txt

```text
SWOS EXPANDED 0.10.15 - SCHERMATA CLUB OFFER (PRIMA ANTEPRIMA)
=============================================================

NOVITA' 0.10.15
- Schermata OFFERTA: ritorna la PRIMA illustrazione approvata, con
  allenatore e presidente al tavolo, contratto sul tavolo, finestra,
  stadio, trofeo e decorazioni. L'immagine occupa TUTTO lo schermo,
  inclusa la zona dietro i pulsanti: nessun grande pannello blu vuoto.
- Layout dei pulsanti come nella prima anteprima: VIEW SQUAD,
  VIEW COMPETITIONS, ACCEPT, DECLINE, BACK, con + e - dello stipendio.
- I pulsanti sono vere voci di menu, selezionabili via mouse/tastiera.
  I rettangoli cliccabili corrispondono esattamente a quelli disegnati.
- Offerta economica e richiesta mostrate PER ANNO, con separatori delle
  migliaia e valuta configurata. Il calcolo interno della trattativa
  rimane invariato e il contratto salva la retribuzione annua.
- Restano le regole esistenti: massimo 5 controfferte rifiutate,
  probabilita' legata alla fama, anteprime squadra e competizioni.
- Il piccolo stemma in alto a destra e' neutro SWOS, non di una squadra
  fissa, dato che l'offerta puo' provenire da club diversi.
- Title Screen, New Career, Calendario, Unemployed e Club Business
  NON sono stati modificati in questa versione.

INSTALLAZIONE / AVVIO
1. Estrai tutto lo ZIP in una cartella nuova.
2. Apri la cartella SWOS_Expanded_0_10_15.
3. Avvia AVVIA_SWOS.bat con doppio clic.
4. Il BAT compila SWOS.exe al primo avvio con MSYS2 MINGW64 e poi lo avvia.
   La compilazione Windows non e' stata verificata in questo ambiente.

PER RIGENERARE LO SFONDO
La prima illustrazione e' conservata in:
  sorgenti/tools/ref_offer_first_0115.png
Script: sorgenti/tools/generate_0115_offer_art.py
Output: data/expanded/expanded_offer_bg.raw (320 x 256 byte)

NOTE
Le anteprime di rosa e competizioni pre-firma sono di sola lettura.
La pagina competizioni completa resta dipendente dall'avvio della carriera.
Il file SWOS.exe viene generato sul tuo Windows, non e' gia' compilato.

ANTEPRIMA
ANTEPRIMA_0115_LAYOUT_OFFERTA_ESEMPIO.png mostra un esempio
illustrativo con NOTTS COUNTY e 65.000 annui: il gioco disegna i valori
veri con il font originale SWOS e li aggiorna a ogni offerta.

```

---

## LEGGIMI_0116_IT.txt

```text
SWOS EXPANDED 0.10.16 - WELCOME / SACKED / FANS & BOARD
=======================================================

1. WELCOME TO THE CLUB
- E' la PRIMA immagine approvata: allenatore di spalle, tifosi blu,
  presidente che lo accoglie, finestre e messaggio centrato.
- Restano nelle medesime posizioni il riquadro centrale, le due
  statistiche FANS LIKING e BOARD LIKING e CONTINUE.
- Nome dell'allenatore, club e percentuali sono dinamici, con font SWOS.
- Gli stessi valori sono mostrati in CLUB BUSINESS.

2. SACKED!
- Popup centrale di circa meta' schermo, con la SECONDA scritta
  rossa approvata, allenatore triste e tifosi sullo sfondo.
- Un solo tasto selezionabile: RETURN UNEMPLOYED.
- Quando FANS + BOARD scende sotto 40 dopo una partita, si apre il
  popup e quindi il calendario da disoccupato alla settimana raggiunta.

3. SISTEMA DI GRADIMENTO
- Fama iniziale UNKNOWN / AMATEUR / RISING STAR / ESTABLISHED /
  LEGEND = fans 20 / 40 / 60 / 80 / 100.
- Board normalmente vicino a 70; peggiora con la percentuale di
  aumento contrattato, i successivi aumenti ottenuti e i conti in rosso.
- Vittorie, pareggi, sconfitte e finanze modificano il gradimento.
- Le variabili sono salvate nel salvataggio JSON della carriera.
- Una controfferta del 10% resta possibile anche per UNKNOWN: la
  probabilita' diminuisce con richiesta e rifiuti (massimo cinque).

4. CLUB OFFER
- La prima immagine, il layout e tutti i pulsanti restano quelli
  della 0.10.15; le due intestazioni ora usano font nativo nitido.
- Stipendio annuale e contrattazione invariati.

5. LIMITAZIONE ATTUALE
- Il ritorno al calendario UNEMPLOYED conserva la settimana della
  squalifica/licenziamento. Se si accetta un successivo impiego, la
  ricostruzione sicura della stagione tramite InitCareer riparte
  dall'inizio di una nuova stagione. Un vero trasferimento a meta'
  campionato richiedera' una futura integrazione dei calendari.
- Non e' stato possibile provare l'eseguibile Windows in questo
  ambiente Linux. Il BAT compila il gioco tramite MSYS2 MINGW64.

INSTALLAZIONE
- Estrai lo ZIP in una cartella nuova (non sovrascrivere vecchie build).
- Doppio clic su AVVIA_SWOS.bat; al primo avvio il BAT compila i sorgenti.
- Il progetto e' basato su OpenSWOS e include sorgenti e licenze.

TEST ESEGUITI
- Compilazione sintattica delle sorgenti modificate con GCC Linux.
- Test del modello di fama, gradimento, finanze e controfferte.
- Verifica dei nuovi RAW 8-bit e della presenza degli asset nel pacchetto.

```

---

## LEGGIMI_0117_IT.txt

```text
SWOS EXPANDED 0.10.17 - MANAGEMENT RECORD / TEST ESONERO
=======================================================

NOVITA
- MANAGEMENT RECORD: nuovo sfondo approvato con allenatore a bordo campo,
  tifosi e partita. Il titolo e le statistiche sono disegnati dal font SWOS
  originale, nitido. Restano funzionanti le frecce PREV/NEXT e EXIT.
- Aggiunti COUNTRY, REGION e SALARY annuale allineati. Il simbolo della
  valuta usa l'impostazione del gioco (GBP / EUR / USD). La cifra e' quella
  effettivamente pattuita nella trattativa e salvata nella carriera.
- Il pulsante RECORD e' ora accessibile anche dal calendario UNEMPLOYED.
  Prima del primo ingaggio mostra NO PREVIOUS CLUB RECORD.
- Sfondo approvato a pieno schermo, senza nomi di squadre nelle immagini:
  i cartelli riportano CLUB; nomi reali sono testo dinamico del gioco.
- Titolo WELCOME TO THE CLUB ridisegnato con il font SWOS nitido senza
  cambiare la disposizione approvata delle finestre e dei pulsanti.

TEST TEMPORANEO DEL FLOW SACKED
- SOLO PER QUESTA BUILD DI PROVA: al PRIMO benvenuto FANS=19 e BOARD=20,
  totale 39, inferiore alla soglia 40.
- Premendo CONTINUE si arriva al calendario; il primo clic su PLAY MATCH
  (o NEXT MATCH) apre immediatamente il popup SACKED! SENZA disputare
  la partita. RETURN UNEMPLOYED apre il calendario disoccupato.
- Il test si attiva una sola volta per sessione, per non bloccare tutti
  i successivi contratti. Non e' una modifica definitiva alla difficolta'.
- Per il gameplay normale dopo la prova occorrera' disattivare il test.

TRASFERIMENTI
- Non alterati i callback originali. CheckTransfers abilita TRANSFERS
  quando la stagione e' attiva e non e' terminata; il codice di InitCareer
  azzera il flag di fine stagione all'inizio. La verifica runtime su Windows
  resta da fare sul PC del giocatore.

COME AVVIARE SU WINDOWS
1. Estrai tutto lo ZIP in una cartella NUOVA, senza sovrascrivere vecchie build.
2. Apri SWOS_Expanded_0_10_17 e fai doppio clic su AVVIA_SWOS.bat.
3. Il BAT prepara MSYS2 se necessario e compila SWOS.exe la prima volta.
4. Dopo il primo avvio, l'eseguibile resta disponibile nella cartella.

NOTE
- Il pacchetto contiene sorgenti OpenSWOS, dati, asset, script e licenze.
- Non contiene un SWOS.exe Windows precompilato: la compilazione avviene sul
  tuo PC con MSYS2; non e' stato possibile eseguire il gioco Windows qui.
- Un nuovo ingaggio dopo esonero ricostruisce ancora la stagione dall'inizio,
  come gia' dichiarato nella 0.10.16 (limitazione nota del motore).
- La schermata RECORD mostra lo stipendio del contratto corrente; la
  cronologia dei salari per singola stagione non e' ancora memorizzata.

```

---

## LEGGIMI_0118_IT.txt

```text
SWOS EXPANDED 0.10.18 - ROSA INTERATTIVA E FIX
=============================================

NOVITA - VIEW SQUAD
- Nuovo sfondo pixel art approvato con allenatore, squadra e stadio.
- Rimangono i menu originali GOALS, TRAINING, EXIT e le funzioni di rosa.
- Cinque stelle per giocatore, da nere a gialle, ricavate dalle sette
  statistiche reali 0..15 (non dall'immagine dimostrativa).
- Formula diversa per difensori, centrocampisti e attaccanti; portieri
  valutati tramite il loro indice di prezzo, poiche' nel database SWOS
  hanno spesso tutte le sette abilita' a zero.
- Il popup mostra le statistiche originali: passing, velocity, heading,
  tackling, control, speed, finishing e il valore form corrente.
- Le statistiche vengono lette dai PlayerFile effettivi della carriera:
  dopo trasferimenti o modifiche ai giocatori non sono bloccate al JSON
  iniziale. L'ordine segue la formazione/rosa del gioco.
- HOVER: mouse sulla riga, oppure frecce / D-pad del joypad per selezionare
  un calciatore. Il popup compare senza premere FIRE. La navigazione
  originale della rosa e' conservata.

CORREZIONI
- CLUB OFFER: ripristinato lo sfondo ufficio della versione precedente,
  con titoli e valori dinamici disegnati dal font originale SWOS; cartello
  CLUB generico, nessun nome di squadra fissato nello sfondo.
- SACKED: immagine e pulsante a pieno schermo, senza sfondo precedente ai lati.
- MANAGEMENT RECORD: un esonero viene registrato subito nella stagione,
  e sostituisce IN COMPETITION con SACKED per le competizioni ancora aperte.
- UNEMPLOYED -> RECORD -> BACK: ridisegno del menu disoccupato al ritorno,
  per non mostrare i pixel della squadra o dello schermo precedente.
- CALENDARIO: frecce rosse e scrolling mantengono la logica originale,
  ma il pannello viene ridisegnato completamente dopo ogni scroll per
  eliminare il clipping nelle righe inferiori.

IMPORTANTE: TEST ESONERO ANCORA ATTIVO
- Come richiesto nella prova precedente, SOLO al primo WELCOME della
  sessione FANS=19 e BOARD=20, totale 39: PLAY MATCH/NEXT MATCH
  mostra subito SACKED. Le offerte successive usano la logica normale.
- La prossima build stabile potra' disattivare questo test con una patch.

AVVIO SU WINDOWS
1. Estrai lo ZIP in una cartella NUOVA.
2. Apri SWOS_Expanded_0_10_18 e avvia AVVIA_SWOS.bat.
3. Il primo avvio installa/chiede MSYS2 e compila SWOS.exe dai sorgenti.
4. Le volte successive usa l'eseguibile gia' compilato.

VERIFICHE
- PASS: controllo sintattico GCC dei sorgenti principali modificati.
- PASS: test unita' per stelle 1..5, formula Ian Hendon (4 stelle),
  statistiche del JSON inglese, liking e soglia di esonero.
- PASS: smoke test headless 320x256 con nuovo sfondo, stelle, tooltip,
  welcome, sacked a pieno schermo e protezione dei bordi framebuffer.
- PASS: verifica delle risorse RAW e del layout calendario esistente.
- NON ESEGUITO: compilazione Windows x64 completa e test interattivo
  con joypad/mouse nel gioco reale (richiede MSYS2 e SDL2 su Windows).
- Questo ZIP NON contiene un SWOS.exe Windows precompilato.

NOTE
- Non sono stati modificati i dati delle squadre, i trasferimenti, il
  gameplay delle partite, il training o il sistema di salvataggio.
- Il popup mostra il campo form originale SWOS, non statistiche inventate
  come morale e fitness; quelle potranno essere aggiunte quando esisteranno.

```

---

## LEGGIMI_0119_IT.txt

```text
SWOS EXPANDED 0.10.19 — SQUAD + CLUB BUSINESS
=============================================

CORREZIONI SQUAD
- Riga di selezione originale e riga azzurra riallineate alle stesse coordinate.
- Conferma del giocatore: mantenute le funzioni SWOS originali di selezione,
  deselezione e scambio; un solo ridisegno invece di tre presentazioni parziali.
- GOALS passa alla vista dei gol; BACK torna alla rosa normale. TRAINING ed
  EXIT conservano i callback del gioco originale.
- Tooltip piu stretto (107x99 px), vicino ai nomi e con valori accostati.
- Barra blu confinata nel frame.
- FORMA squadra: percentuale da ultimi cinque risultati, con penalita di
  viaggio e recupero fitness; separata dal campo FORM originale del singolo.

CALENDARIO
- Frecce rosse spostate a destra e verso l'interno del pannello; hitbox e
  focus rimangono allineati. Scrolling e partite originali invariati.

FACILITIES — LIVELLI 1-5
- SCOUT: stipendio crescente; ricerca giocatori visibili: livello 1 paese
  del club, livello 2 cinque paesi geograficamente piu vicini, livello 3
  stessa regione/continente, livello 4 quasi tutto il mondo, livello 5
  tutto il mondo. I trasferimenti e le trattative originali restano attivi.
- MEDICAL: +10/20/30/40/50% di probabilita di recupero supplementare per
  ciascun giocatore infortunato dopo una partita; non tocca le squalifiche.
- TRAVEL: le trasferte affaticano la squadra, da -14 al livello 1 a -2 al 5.
- TRAIN: la partita di allenamento migliora i modificatori di forma dei
  giocatori (non aumenta permanentemente i valori tecnici), secondo il livello.
- FITNESS: recupero settimanale della fatica di 2/4/6/8/10 punti e maggiore
  efficacia dell'allenamento insieme a TRAIN.
- YOUTH: i nuovi trialisti/giovani possono avere un indice di valore piu
  alto, a seconda dell'accademia; resta il limite originale di 30 giocatori.
- OFFICE: migliori proposte degli sponsor e maggior richiamo di tifosi.
- TUTTI: stipendi e manutenzione progressivi; spesa base per impiegati
  generici di 45 a settimana. Costi scalati nel saldo reale del club.

SPONSOR E TV
- Dal secondo turno settimanale: popup senza nuova immagine, con TRE offerte
  sponsor (local/national/premium). Selezionare una per firmare.
- Dal quarto turno settimanale: proposta TV fissa, basata su pubblico e
  qualita squadra, con pulsante ACCEPT.
- Se si esce senza firmare, la proposta riappare al successivo avanzamento.
- Incassi pagati a rate settimanali, non sommati due volte al bilancio.
- I contratti sono rigenerati a ogni nuova stagione e memorizzati nel save.

CLUB BUSINESS
- Schermata e illustrazione precedenti preservate.
- Il piccolo pulsante S/W accanto a FINANCES alterna la visualizzazione
  SETTIMANA (ritmo attuale) / STAGIONE (totale realmente maturato).
- Righe uscite: WAGES, TRAVEL, STAFF, TOTAL. STAFF comprende gli otto
  servizi e il personale generale; in ogni dettaglio facility e visibile
  il costo settimanale (SCOUT WAGES e MEDICAL WAGES espliciti).
- Righe entrate: SPONSOR, TV, GATE, INCOME; usano i contratti accettati.

IMPORTANTE
- Il test temporaneo dell'esonero automatico a gradimento 39 e DISATTIVATO.
- Questa distribuzione contiene i sorgenti aggiornati e i dati di gioco.
  Non contiene un SWOS.exe Windows precompilato. AVVIA_SWOS.bat compila con
  MSYS2 al primo avvio e successivamente avvia SWOS.exe.
- Sono stati eseguiti test unitari e smoke test grafici headless.
  La compilazione Windows completa e il test interattivo con joypad NON sono
  stati eseguiti in questo ambiente: verificare sul proprio PC.

AVVIO
1. Estrai tutto in una cartella nuova, senza sovrascrivere la vecchia.
2. Avvia AVVIA_SWOS.bat su Windows.
3. Per errori di compilazione allega COMPILAZIONE_0119.log.

```

---

## LEGGIMI_0120_IT.txt

```text
SWOS EXPANDED 0.10.20 — RIFINITURE ROSA E BUSINESS
=================================================

VIEW SQUAD
- Tooltip statistiche ancora piu compatto: 77 x 83 pixel, x=80,
  allineato circa a meta del nome del calciatore.
- Il popup compare sempre SOPRA o SOTTO la riga evidenziata, mai
  direttamente sopra: stelle, nome, posizione e nazionalita del giocatore
  selezionato restano visibili anche usando il joypad.
- Rimossa la cornice bianca orizzontale che tagliava le lettere:
  il focus ora e blu con due marcatori verticali bianchi laterali.
- Dopo aver selezionato un giocatore per lo scambio, la sua riga
  diventa arancione-rossa; la selezione successiva esegue lo scambio
  con il comportamento originale SWOS. GOALS, TRAINING ed EXIT invariati.
- Soppressi i vecchi sprite/testi di decorazione della schermata rosa,
  responsabili dei residui bianchi e delle bande blu ai bordi; lo sfondo
  originale pixel art e ripristinato prima del rendering della rosa.

FORMA INDIVIDUALE 0..100
- Tutti i 32 slot del club partono a 50/100; l'indicatore nel tooltip
  legge un valore individuale, non il campo PlayerFile.form originale.
- Valori di forma originali SWOS (inclusi i marcatori per riserva/prova)
  NON sono sovrascritti.
- Recupero settimanale: +1..+5 secondo FITNESS, max 100.
- Allenamento: +3..+15 secondo TRAIN e FITNESS, max 100.
- Partita: gli 11 titolari perdono 4 punti in casa e 5..11 in trasferta,
  in base al livello TRAVEL; nessuna penalita ai panchinari non utilizzati.
- Nuovi acquisti/giocatori in prova partono a 50/100; i valori sono salvati
  nel JSON di carriera e ripristinati al caricamento.
- Coefficiente di rendimento definito: 70% a forma 0, 85% a forma 50,
  100% a forma 100 (interpolazione lineare, con arrotondamento).
  NOTA: il coefficiente e disponibile nel codice e testato, ma NON viene
  ancora applicato alla simulazione delle partite: verra collegato in una
  prossima fase, come concordato, per evitare di cambiare le meccaniche
  originali prima di aver definito l'integrazione.
- La FORMA della squadra in alto (ultimi cinque risultati) rimane distinta
  dalla nuova condizione fisica individuale.

CLUB BUSINESS
- Il selettore W/S ora e un pulsante largo 64 pixel, alto 16,
  con la scritta intera SEASON o WEEK (destinazione del cambio vista).
- Il disegno, le finestre e il resto dei comandi Business restano uguali.

BUILD E VERIFICHE
- Controllo sintattico GCC per le sorgenti modificate, senza errori.
- Smoke test framebuffer 320x256: focus, popup, evidenziazione scambio,
  menu, sponsor, TV, calendario e Business; nessuna scrittura fuori buffer.
- Test unitari su forma 0..100, coefficiente, recupero, fatica e geometria
  del tooltip per tutte le 17 righe.
- La compilazione Windows x64 completa e la prova interattiva su joypad
  richiedono il PC di destinazione e MSYS2/SDL2; qui NON sono state eseguite.

AVVIO
1. Estrai tutto in una cartella NUOVA, senza sovrascrivere la precedente.
2. Su Windows fai doppio clic su AVVIA_SWOS.bat.
3. Al primo avvio il BAT compila SWOS.exe con MSYS2 (se gia installato,
   riutilizza la stessa installazione).
4. In caso di errore invia COMPILAZIONE_0120.log.

```

---

## LEGGIMI_0121_IT.txt

```text
SWOS EXPANDED 0.10.21 — FINANZE E ROSA
=====================================

CLUB BUSINESS
- Il pulsante WEEK/SEASON e' stato ridotto a 51x13 pixel e riposizionato
  nell'intestazione del pannello FINANCES, senza invadere le altre voci.
- Il numero della settimana e' relativo all'inizio della stagione:
  il sortKey SWOS 56, prima mostrato come WEEK 57, ora e' WEEK 01.
  Lo stesso conteggio corretto controlla l'arrivo delle offerte sponsor e TV.
- WAGES = soli stipendi dei calciatori (dato nativo SWOS).
- STAFF = costi generali SWOS + personale e manutenzione di tutte le 8
  strutture, incluso il costo fisso di 45 a settimana.
- TRAVEL = costo della trasferta giocata: 0 per una partita in casa.
  Le spese di trasferta sono sottratte dal saldo e contabilizzate nel
  registro originale SWOS, evitando di confondere i costi generali con i viaggi.
- Il costo per trasferta dipende dal livello TRAVEL: 100, 180, 320, 560, 950.
  Livelli superiori costano di piu' ma riducono la fatica.
- Passando il mouse su STAFF si apre un piccolo riquadro con 8 strutture,
  OTHER (costi generali + 45) e totale settimanale. Il menu e' opaco.
- Nei dettagli di ogni struttura la voce dei costi del personale e' STAFF.
  Il sottomenu TRAVEL mostra inoltre il costo reale PER TRASFERTA.
- MONTH e YEAR nella colonna BALANCE sono previsioni delle SPESE:
  spese correnti settimanali x 4 e x 52, rispettivamente.
  Non sono il saldo netto ne' il consuntivo di tutta la stagione.
- La vista SEASON continua a mostrare i costi realmente registrati
  dall'inizio della stagione. Sponsor e TV rimangono entrate effettive.

VIEW SQUAD
- Popup del giocatore 142x52 pixel, in due colonne compatte (4+4):
  PASSING / VELOCITY / HEADING / TACKLING e
  CONTROL / SPEED / FINISHING / FORM.
- Il popup e' opaco anche sui bordi, si posiziona sopra/sotto il calciatore
  selezionato e funziona con la selezione mouse/joypad originale.
- La riga blu e' stata rialzata di 2 pixel rispetto al testo; eliminati
  i marcatori laterali gialli. Il giocatore in attesa di scambio rimane
  evidenziato in arancione-rosso.
- I sottili margini laterali della tabella mostrano nuovamente pixel dello
  sfondo stadio gia' presente nel gioco, invece del colore blu piatto.
- Nessuna modifica ai pulsanti GOALS, TRAINING, EXIT o alla logica di scambio.

SALVATAGGI
- I nuovi campi expanded_travel_last_week e expanded_travel_season sono
  salvati nel JSON carriera insieme ai precedenti valori di economia e forma.

INSTALLAZIONE
1. Estrai lo ZIP in una cartella nuova, non sovrascrivere la 0.10.20.
2. Avvia AVVIA_SWOS.bat su Windows.
3. Se necessario, il BAT installa/riutilizza MSYS2 e compila SWOS.exe.
4. Se la compilazione fallisce, invia COMPILAZIONE_0121.log.

VERIFICHE
- Test GCC di sintassi dei sorgenti core: superato, con soli warning
  preesistenti relativi a membri di struct packed.
- Test unitari su costi strutture, forma, week 01, spese x52 e trasferte.
- Test framebuffer headless: 320x256, nessuna scrittura fuori buffer;
  controllati tooltip, selezione, swap, hover staff, sponsor e TV.
- NON e' stata eseguita una compilazione completa Windows o una prova
  reale con mouse e joypad; servono l'ambiente MSYS2/SDL2 di destinazione.

```

---

## LEGGIMI_0122_IT.txt

```text
SWOS EXPANDED 0.10.22 — BUSINESS / JOYPAD / SQUAD
================================================

COSA E' CAMBIATO

CLUB BUSINESS
- FINANCES / SEAS diventa FINANCES / YEAR; il tasto mostra YEAR oppure
  WEEK a seconda della vista da aprire.
- Il tasto WEEK/YEAR e' ora 40 x 11 pixel e si trova COMPLETAMENTE dentro
  il riquadro FINANCES. Non invade il pannello BALANCE.
- Con il joypad: GIU' da QUALSIASI pannello delle strutture -> WEEK/YEAR;
  ancora GIU' -> STAFF (sola anteprima); ancora GIU' -> BACK.
  Con SU da STAFF si ritorna a WEEK/YEAR. Le frecce laterali collegano
  i pulsanti, senza vicoli ciechi.
- STAFF e' una vera posizione del cursore selezionabile con joypad/tastiera:
  il dettaglio compare automaticamente, SENZA premere FIRE. Premere FIRE
  su STAFF non effettua alcuna operazione.
- Il mouse continua a mostrare lo stesso dettaglio quando passa su STAFF.
- Nel riquadro sono scritte per intero le voci OTHER, STADIUM, TRAINING,
  BUS STAFF, OFFICES, FITNESS, SCOUTING, MEDICAL, YOUTH, TOTAL.
  Non compare piu' l'abbreviazione TRANS. BUS STAFF = personale della
  struttura trasporti, NON il costo della trasferta.
- OTHER rimane composto dalle spese generali originali e dai 45 di base;
  il totale STAFF continua a includere il personale di tutte le 8 strutture.
- La spesa per le trasferte e' legata SOLO alla struttura del pullman:
  nel suo dettaglio si vede AWAY TRAVEL / MATCH. In FINANCES la riga
  contabile e' ora AWAY, non un secondo costo TRAVEL: 0 per la partita
  in casa, il costo effettivo per la trasferta. Nessun doppio addebito.
- YEAR nella vista FINANCES mostra il consuntivo dall'inizio della
  stagione; YEAR nel riquadro BALANCE rimane la previsione delle
  spese su 52 settimane. La logica dei soldi non cambia.

VIEW SQUAD
- FORM generale e' visibile in un piccolo riquadro opaco a sinistra
  del nome della squadra, senza coprire la nazionalita'.
- Tolta la striscia blu piatta sotto il nome della squadra, accanto
  alla nazionalita': ora riutilizza i pixel dello stadio gia' presente.
- Sistemata la navigazione joypad: dalla rosa, GIU' sull'ultima riga
  arriva a EXIT; dalla rosa, DESTRA arriva a GOALS; da GOALS, DESTRA
  porta a TRAINING, ancora DESTRA a EXIT.
- Anche nella vista GOALS si vedono BACK ed EXIT, entrambi raggiungibili
  con il joypad. BACK torna alla rosa normale; EXIT esce dal menu.
- Restano invariati i callback originali, i giocatori, il popup a 8
  statistiche, le evidenziazioni blu/arancione e la logica dello scambio.

INSTALLAZIONE SU WINDOWS
1. Estrai TUTTO lo ZIP in una cartella nuova.
2. Doppio clic su AVVIA_SWOS.bat (dentro la cartella estratta).
3. Il BAT usa MSYS2/MINGW64 gia' installato per compilare SWOS.exe alla
   prima esecuzione; le successive esecuzioni riutilizzano la build.
4. Se non trova MSYS2, imposta la variabile MSYS2_ROOT sulla cartella
   di installazione (per esempio C:\msys64) e riprova.
5. Se la compilazione non riesce, invia COMPILAZIONE_0122.log.

VERIFICHE E LIMITI
- PASS: controllo sintattico GCC dei sorgenti core, test framebuffer
  320x256, STAFF con focus senza FIRE, menu/joypad, YEAR, popup, FORM,
  EXIT, costi delle strutture, spese trasferta, sponsor/TV e condizioni.
- NON e' stata eseguita qui una compilazione/link Windows completa ne'
  una prova reale con joypad sul PC di destinazione: il BAT le esegue
  nel tuo ambiente MSYS2, dove e' gia' presente il compilatore.
- Il pacchetto NON contiene un SWOS.exe precompilato: viene creato dal BAT.

```

---

## LEGGIMI_0123_IT.txt

```text
SWOS EXPANDED 0.10.23 - BUSINESS / CALENDARIO / 4K
================================================

CORREZIONI

1. CLUB BUSINESS
- Il tasto WEEK/YEAR e' piu' piccolo (33 x 12 pixel), con caratteri
  5x8, e si trova interamente dentro l'intestazione FINANCES:
  x=168..200, y=187..198, sotto il bordo superiore e sopra il divisorio.
- La zona cliccabile e il focus del joypad coincidono con il nuovo tasto.
  FIRE cambia WEEK/YEAR come prima; GIU' dalle strutture lo seleziona.
- Il popup STAFF conserva tutte le diciture complete, ma adesso e'
  largo solo quanto serve: ogni importo segue la propria voce a 3 pixel
  di distanza, anziche' essere spinto al margine destro del riquadro.
- Il popup STAFF appare con hover del mouse o semplicemente selezionando
  STAFF con il joypad, senza FIRE. La contabilita' non cambia.
- Nella tabella FINANCES, la voce AWAY torna a chiamarsi TRAVEL.
  Le trasferte sono addebitate una sola volta, secondo il pullman;
  per una partita in casa il costo TRAVEL e' zero.

2. CALENDARIO
- Entrambe le frecce rosse sono state spostate 7 pixel a sinistra.
- Anche le rispettive aree cliccabili sono state spostate: non coprono
  piu' le scritte delle partite. Rimangono gli stessi callback e la
  stessa navigazione con joypad.

3. SCHERMO 3840 x 2160
- AVVIA_SWOS.bat applica CONFIGURA_4K.ps1 ad ogni avvio: fullscreen
  desktop, resolution=auto, widescreen=stretch, scaling=sharp.
- Su un monitor 3840x2160 il rendering viene allargato fino ai bordi,
  evitando le grandi bande nere laterali dovute al rapporto originale.
  Questo riempimento allarga anche la pixel art: e' una scelta voluta
  per provare il fullscreen 16:9 senza bande laterali.
- Se preferisci le proporzioni originali, cambia la voce widescreen
  in Documents\SWOS\SWOS.ini da stretch a true e avvia SWOS.exe
  direttamente; AVVIA_SWOS.bat riapplica il profilo 4K a ogni avvio.
- La resa effettiva e l'input a 3840x2160 devono essere provati sul PC.

4. SQUAD
- Nessuna modifica allo schermo squadra, come richiesto.

INSTALLAZIONE
1. Estrai TUTTO lo ZIP in una cartella nuova.
2. Fai doppio clic su AVVIA_SWOS.bat.
3. Il BAT trova MSYS2/MINGW64, compila SWOS.exe alla prima esecuzione,
   poi lo riutilizza agli avvii successivi.
4. In caso di errore manda COMPILAZIONE_0123.log.

TEST
- PASS: compilazione sintattica GCC dei sorgenti core modificati;
  test framebuffer 320x256, focus WEEK/YEAR, popup STAFF con distanza
  di 3 pixel, grafi joypad, costi e TRAVEL, squad, 8 strutture e calendario.
- NON VERIFICATO: compilazione/link completo Windows e prova reale
  con joypad e monitor 4K. Servono il tuo MSYS2 e il tuo hardware.

```

---

## LEGGIMI_0124_IT.txt

```text
SWOS EXPANDED 0.10.24 - FONT ORIGINALE / SCHERMO PIENO 4K
=========================================================

Questa versione parte dai sorgenti e da tutti gli asset della 0.10.23.
La grafica approvata di BUSINESS, SQUAD, CALENDARIO, CARRIERA e degli
altri menu Expanded rimane invariata. Nessun dato di gioco viene modificato.

CORREZIONI

1) CLUB BUSINESS
- Il tasto WEEK/YEAR usa di nuovo il font ORIGINALE SWOS,
  come le altre voci del menu. Conserva dimensioni 33x12 e posizione
  x=168, y=187; non oltrepassa il riquadro FINANCES.
- Restano l'hover STAFF compatto, il focus senza FIRE da joypad,
  la voce TRAVEL e tutte le correzioni della 0.10.23.

2) TUTTI I MENU CON GRAFICA EXPANDED / 3840x2160
- Risolto il margine residuo di 16 pixel a DESTRA e in BASSO:
  la grafica dei menu e' 320x256, il framebuffer originale 336x272.
- In modalita' fullscreen STRETCH il renderer ora visualizza soltanto
  i 320x256 pixel dell'interfaccia Expanded, estendendoli fino ai
  quattro bordi del monitor. La mappa e lo scudo originali non sono
  piu' visibili ai bordi di questi menu.
- Sono inclusi BUSINESS, SQUAD, CALENDARIO, MENU PRINCIPALE,
  CARRIERA, OFFERTE, RECORD e le altre schermate Expanded.
- Il ridimensionamento e' fatto SOLO in fase di visualizzazione:
  non altera i menu, i tasti del joypad, le coordinate interne,
  la simulazione o i file di salvataggio.
- Gli hover del mouse su STAFF e SQUAD sono rimappati sulle nuove
  proporzioni. La partita conserva il suo rendering completo.
- I menu nativi non personalizzati mantengono la loro grafica originale.
- Come gia' nella 0.10.23, STRETCH puo' modificare le proporzioni
  della pixel art per riempire il 16:9.

3) CALENDARIO / SQUAD
- Restano le frecce del calendario spostate a sinistra.
- SQUAD non e' stata modificata, a parte il ridimensionamento video.

INSTALLAZIONE
1. Estrai TUTTO lo ZIP in una cartella nuova (non sovrascrivere 0.10.23).
2. Fai doppio clic su AVVIA_SWOS.bat.
3. Il BAT trova MSYS2/MINGW64, compila SWOS.exe alla prima esecuzione,
   applica la configurazione 4K e avvia il gioco.
4. Se la compilazione fallisce, invia COMPILAZIONE_0124.log.
   Se il gioco si blocca, invia SWOS.log da Documenti\\SWOS.

VERIFICHE
- PASS: test automatico del menu BUSINESS, font originale, STAFF,
  finanze, TRAVEL, joypad, rosa e offerte.
- PASS: test di controllo delle coordinate di presentazione 320x256
  e del ripristino del rendering completo durante la partita.
- PASS: controlli sintattici GCC delle parti del core disponibili qui,
  integrita' dei dati e archivio ZIP.
- NON VERIFICATO QUI: build/link Windows MSYS2 e prova reale su monitor
  3840x2160 con mouse e joypad. Il BAT esegue il controllo Windows.

```

---

## LEGGIMI_0125_IT.txt

```text
SWOS EXPANDED 0.10.25 - STADIO / CARICAMENTO / FORMA & MORALE
================================================================
BASE: SWOS Expanded 0.10.24 (sorgenti, dati e schermate invariati salvo
le modifiche indicate sotto). Installare in cartella nuova.

1) SCHERMATA DI CARICAMENTO
- Lo stadio approvato e' in data/expanded/expanded_loading_bg.bmp,
  con copia PNG. Titolo senza TM; cartelli Giuzy & Ciat, Sensible
  Software, Ben Baker e Zlatko Karakas.
- Il fondale e' una texture COMPLETA. La barra e la percentuale sono
  disegnate dal renderer SDL2 come UI separata e possono essere spostate.
- La barra mostra progressi REALI per fasi completate del bootstrap
  (0,12,38,56,63,78,88,100), non un timer finto o una stima di byte.
- La vecchia schermata con il disco e' sostituita dalla nuova splash;
  a 100% il gioco prosegue verso il title/menu.
- Il caricamento originale completo si puo' ancora abilitare con
  SWOS_SHOW_INTRO=1, ma la nuova splash resta attiva dopo l'intro.
- Se il BMP manca, il gioco ripiega sul vecchio caricamento.

2) FORMA E MORALE
- La condizione INDIVIDUALE aggiunta in 0.10.20 non viene piu' usata,
  ne' visualizzata ne' salvata. Le statistiche originali di SWOS,
  comprese le sue meccaniche di forma, non sono state cancellate.
- FORM e MORALE ora sono di SQUADRA (0..100), persistenti in JSON.
- Form basata su ultimi cinque risultati, con ingresso graduale.
  Morale sale con le vittorie, scende con le sconfitte e tende a 65.
- La media delle due e' neutra a 65; >=100 da' +20%; <=20 da' -20%.
  La modifica e' applicata alle abilita' in partita della squadra gestita,
  senza cambiare le statistiche memorizzate dei calciatori.
- Cinque faccine, con fasce 0-19 rossa, 20-39 gialla, 40-59 gialla,
  60-79 gialla, 80-100 verde. Anche BOARD e FANS le usano.
- SQUAD: due placche FORM/MORALE con percentuali e faccine;
  nome del paese in riquadro con testo giallo; tooltip individuale
  con le sette abilita' originali, senza condizione individuale.

3) CLUB BUSINESS
- OTHER sostituito da ADMINISTRATION: 45.000 all'anno, valore
  PROVVISORIO valido finche' non definiamo la scala dei club.
  Le 52 rate settimanali sommano esattamente a 45.000.
- MANAGER mostra il salario annuo effettivamente accettato, con quota
  settimanale nel dettaglio STAFF. Il costo entra nel bilancio reale.
- Non si altera TRAVEL (solo trasferte), sponsor, TV o i costi
  degli otto impianti.

4) AVVIO
- Il BAT rileva MSYS2/MINGW64, compila il codice aggiornato, poi
  esegue il gioco. Non riutilizza SWOS.exe di versioni precedenti.
- Fullscreen e rendering 4K come nella 0.10.24.

VERIFICHE NELL'AMBIENTE DI SVILUPPO
- PASS: controlli GCC -fsyntax-only sui sei sorgenti core modificati.
- PASS: harness C di BUSINESS/SQUAD/FINANZE e test di FORM/MORALE.
- PASS: controlli Python della navigazione e dell'integrita' asset.
- NON ESEGUITO QUI: compilazione/link Windows MSYS2, SDL2 su Windows,
  verifica della barra a schermo, partite reali e prova su 3840x2160.
  Il BAT eseguira' la compilazione sul tuo computer.

ATTENZIONE: le abilita' SWOS sono discretizzate 0..7. Il bonus/malus
+/-20% si applica ai valori calcolati per la partita, ma il risultato
puo' essere quantizzato e non sempre cambia ciascun attributo.

```

---

## LEGGIMI_0127_IT.txt

```text
SWOS EXPANDED 0.10.27 — NOTE DI TEST (4 novembre 2026)
========================================================
Base: 0.10.26, che incorpora le correzioni BUSINESS e SQUAD richieste.

NOVITA 0.10.27
- MATCH STATS: GOAL ATTEMPTS -> SHOTS, CORNERS WON -> CORNERS,
  BOOKINGS -> YELLOW CARDS, SENDINGS OFF -> RED CARDS.
- JOYPAD: tasto + / START invia ESC invece di P. Deve aprire il
  normale flusso di uscita della partita, come il tasto ESC fisico.
  Il tasto - / BACK continua ad aprire le statistiche (S).
  Il click dello stick destro continua a inviare ESC (compatibilita).
- CARICAMENTO: mantenute percentuale e avanzamento a fasi reali.
  Quando arriva al 100%, resta visibile fino a completare almeno
  5 secondi dall'inizio, per facilitare gli screenshot. RIMUOVERE
  questo tempo minimo dopo l'approvazione definitiva della grafica.
- BUSINESS: popup STAFF con OFFICES nella colonna destra, TRAVEL al
  posto di BUS STAFF, niente righe annuali duplicate; ADMINISTRATION
  45.000 annui per club piccolo, stipendio manager contrattuale.
- SQUAD: nome club e nazione spostati in alto; niente sovrapposizione
  con tabella giocatori; FORM e MORALE di squadra.
- FACCE: cinque espressioni, 0-39 rosso, 40-59 giallo,
  60-100 verde, su SQUAD e BUSINESS.

AVVIO
Estrarre in una NUOVA cartella, avviare AVVIA_SWOS.bat.
Richiede MSYS2 MINGW64 e compila SWOS.exe al primo avvio.
In caso di errore inviare COMPILAZIONE_0127.log.

TEST IMPORTANTI
1. Durante una partita premere +: verificare menu uscita e BACK
   per tornare alla partita, con gioco fermo mentre il menu e aperto.
2. Premere -: verificare statistiche e nuove quattro etichette.
3. Verificare 5 secondi minimi di splash e 100% prima del titolo.
4. Verificare tooltip BUSINESS e riquadri/faccine SQUAD.

LIMITI DELLA VERIFICA
Verifica sintattica GCC di alcuni sorgenti e controlli statici.
NON e stata eseguita qui una compilazione Windows nativa o un test
interattivo con joypad. Il comportamento + -> menu va confermato
sul PC: la mappatura e identica al tasto ESC da tastiera.

```

---

## LEGGIMI_0128_IT.txt

```text
SWOS EXPANDED 0.10.28 - MODIFICHE E TEST

- Splash: attesa MINIMA TEMPORANEA 20 secondi per screenshot; percentuale di progresso reale resta a fasi.
- BUSINESS STAFF: quattro colonne allineate (nome/cifra e nome/cifra), TOTAL in basso a destra.
- Faccine BOARD/FANS/FORM/MORALE: 0-39 rosse, 40-59 gialle, 60-100 verdi, cinque espressioni dinamiche. Indici colore aggiornati per evitare faccine quasi nere.
- SQUAD: paese al centro sulla stessa riga e altezza di FORM/MORALE; BANK rientrato e nome squadra allineato.
- SQUAD background: ripristinata texture stadio sotto il riquadro paese, eliminando la grande banda blu uniforme.

Compilazione automatica tramite AVVIA_SWOS.bat (richiede MSYS2 gia installato).
Verificare a schermo i colori e il popup dopo la compilazione Windows.

```

---

## LEGGIMI_0130_IT.txt

```text
SWOS Expanded 0.10.30 - Note di prova

- Splash: ripristinato il cartello destro originale 0.10.27, senza alterare il resto dello stadio; LOADING centrato usando la larghezza reale del testo.
- BUSINESS: hover STAFF piu compatto, cifre vicine alle descrizioni.
- SQUAD: BANK ora largo come MORALE, allineato a destra; nome club largo come paese e abbassato appena sopra il paese. Ritoccate soltanto le strisce blu residue dello sfondo.
- SQUAD tooltip: accanto alle statistiche originali appare la variazione teorica (+ verde / - rosso) calcolata dal moltiplicatore forma/morale, arrotondata a 0.5. Il valore originale resta intatto. Il motore SWOS usa anche altri fattori, quindi il numero e indicativo, non una promessa sul valore finale in partita.
- 20 secondi di splash temporaneo invariati.
- Non testato nativamente su Windows; il BAT compila da MSYS2. Se necessario inviare COMPILAZIONE_0130.log.

```

---

## LEGGIMI_0131_IT.txt

```text
SWOS Expanded 0.10.31 — Source approvate e nuova integrazione

VIEW SQUAD: sfondo completo nuovo in raw 320x256 (palette originale SWOS); la UI rimane una seconda layer di rendering live, con dati e callbacks originali. Fonte UI approvata conservata come SOURCE_VIEW_SQUAD_UI_REFERENCE.png, NON stampata sullo sfondo. BANK allineato a VIEW SQUAD, in alto a destra; FORM/MORALE e altri elementi nella posizione della reference.

LOADING: SOURCE_LOADING_APPROVED.png preservata esattamente; runtime BMP letterboxed per rispettare proporzioni 4:3. Barra e percentuale SDL separati, coprono il testo e la barra statici nella fonte. LOADING centrato rispetto alla parola, non ai puntini. Hold 20 secondi temporaneo.

BUSINESS: hover STAFF piu compatto.

JOY +: dialogo di pausa indipendente dal comando ESC; START apre, START/B torna al gioco, A conferma l'uscita. SELECT/- continua ad aprire le statistiche. Questo percorso richiede verifica interattiva su Windows; non testato con controller fisico in questo ambiente.

Compilazione Windows tramite AVVIA_SWOS.bat e MSYS2. Controlli GCC locali non sostituiscono la prova reale.

```

---

## LEGGIMI_0132_IT.txt

```text
SWOS Expanded 0.10.32 — sorgenti Windows/MSYS2

SORGENTI APPROVATE, SEPARATE:
- data/expanded/SOURCE_LOADING_BACKGROUND_APPROVED.png (stadio 16:9 e cartelli)
- data/expanded/SOURCE_LOADING_TITLE_APPROVED.png (logo, in alto a sinistra)
- data/expanded/SOURCE_LOADING_BAR_APPROVED.png (riferimento grafico LOADING)

LIVELLI IN GIOCO:
- expanded_loading_bg.bmp: fondale completo
- expanded_loading_title.bmp: logo con color key
- expanded_loading_caption.bmp: testo LOADING con color key
- barra e percentuale sono generate in tempo reale da SDL, NON sono stampate nel fondale.

BUSINESS: ADMIN, popup STAFF piu stretto e numeri piu vicini.
SQUAD: elenco abbassato, righe portieri colorate diversamente.

ATTENZIONE: START/+ non e stato corretto in questa revisione: l'implementazione 0.10.31
usa ancora il dialogo di pausa aggiuntivo. Il codice ESC originale termina la partita
direttamente e non apre un menu, quindi non e sicuro mapparlo semplicemente a START.
La correzione richiede identificare il menu ESC desiderato e il relativo stato.

Non e stata eseguita una build Windows nativa; usare AVVIA_SWOS.bat.

```

---

## LEGGIMI_0132b_IT.txt

```text
SWOS Expanded 0.10.32b - test ripristino START/ESC

START/+ ripristinato al comando P di pausa originale.
ESC tastiera lasciato sul percorso originale del gioco.
Rimosso il menu di pausa/uscita aggiuntivo della 0.10.31-0.10.32.
SELECT/- e tutte le altre schermate invariati.

NOTA: il percorso ESC del game loop può interrompere direttamente la partita; verificare il comportamento sul PC.
La build Windows viene compilata dal BAT al primo avvio, non è collaudata qui.

```

---

## LEGGIMI_0133_IT.md

```text
# SWOS Expanded 0.10.33 — START / EXIT confirmation

- START/+ durante la partita: pausa e piccolo popup «DO YOU WANT TO EXIT?» con BACK e EXIT. BACK preselezionato.
- D-pad sinistra/destra cambia scelta, A conferma, B o START torna indietro. Tastiera: frecce, INVIO, ESC annulla.
- SELECT/− e la pausa originale P non sono stati modificati. ESC fuori dal popup resta originale.
- Business STAFF/WEEK: popup più stretto, importi avvicinati e meno spazio vuoto sotto.
- Squad tooltip: più stretto, bonus/malus espliciti anche +0, portiere contrassegnato in giallo.
- Schermata loading: risorse e grafica lasciate inalterate; durata minima impostata a 5 secondi.

Il BAT compila il gioco su Windows con MSYS2/MINGW64. Nessun eseguibile Windows precompilato incluso.

```

---

## LEGGIMI_0134_IT.md

```text
# SWOS Expanded 0.10.34 — correzioni input e tooltip

- START apre la conferma durante qualsiasi stato di partita attiva (_playGame), senza il filtro 21..31 che poteva impedire l’apertura.
- ESC durante una partita non provoca più l’uscita immediata; RIGHTSTICK non manda più ESC. Fuori partita ESC rimane invariato.
- START supportato anche come pulsante 7 per joystick SDL non mappati (la numerazione può variare per modello).
- STAFF/WEEK: colonna di destra spostata di 7 pixel verso i propri importi, titolo abbassato di 2 pixel.
- VIEW SQUAD: tooltip ristretto a 172 pixel, nome e righe abbassati di 2 pixel, bonus accostato al numero base.
- Bonus/malus: ricalcolati usando forma e morale correnti e la stessa funzione percentuale della partita. Con forma=65 e morale=65 il risultato è necessariamente +0 (100%). Anche attributi base pari a zero rimangono zero.
- Loading e SELECT invariati.

Il BAT compila su Windows con MSYS2. Non è stato eseguito un test fisico con joypad su Windows.

```

---

## LEGGIMI_0135_IT.md

```text
# SWOS Expanded 0.10.35

- START: corretta la posizione del popup di conferma rispetto alle coordinate logiche SDL. Il precedente codice usava la risoluzione fisica 4K, disegnando il popup fuori schermo e lasciando visibile soltanto l'oscuramento. BACK riprende la partita, EXIT richiede l'abbandono.
- SELECT: rimosso il blocco che impediva la visualizzazione delle statistiche durante il gioco attivo. Un secondo SELECT chiude le statistiche.
- BUSINESS STAFF/WEEK: importi della colonna sinistra spostati di 7 pixel a destra.
- VIEW SQUAD e grafica di caricamento: invariati.

Il pacchetto contiene sorgenti e BAT per la compilazione sul PC Windows; non contiene un eseguibile Windows precompilato. Il test fisico con joypad non è stato eseguito in questo ambiente.

```

---

## LEGGIMI_0136_IT.md

```text
# SWOS Expanded 0.10.36 — nuove UI MATCH STATS e EXIT MATCH

- Nuovi pannelli grafici pixel-art basati sulle due immagini approvate.
- Il verde del campo presente nelle anteprime NON è incluso nelle risorse: vengono visualizzati soltanto i pannelli sopra il campo congelato.
- MATCH STATS: valori e nomi disegnati a runtime dalle strutture di gioco; la grafica è statica ma i dati sono dinamici.
- EXIT MATCH: immagine approvata e contorno di selezione aggiunto in runtime. La logica START/BACK/EXIT rimane quella della 0.10.35.
- VIEW SQUAD, BUSINESS, loading e controlli di gioco invariati.

**Limite di test:** il codice è preparato per la compilazione con AVVIA_SWOS.bat su Windows/MSYS2; non è stato eseguito un test interattivo Windows/joypad in questo ambiente. Verificare allineamento e leggibilità dei numeri in partita. Il pannello di riferimento includeva testo già disegnato nell'immagine: le aree dei dati sono state ripulite per consentire valori dinamici.

```

---

## LEGGIMI_0137_IT.md

```text
SWOS Expanded 0.10.37 — Correzioni finali MATCH STATS ed EXIT MATCH.

MATCH STATS: rimosso il vecchio rendering sovrapposto; nuova tabella pulita con celle riservate ai dati reali, possesso dinamico e bordi completi.
EXIT MATCH: eliminata la cornice di selezione sovrapposta, aggiunta una freccia di selezione; bordi esterni completi. BACK inizialmente selezionato, START/SELECT invariati.

AVVIA_SWOS.bat compila tramite MSYS2 sul PC Windows. La compilazione e il test con joypad Windows non sono stati eseguiti in questo ambiente.

```

---

## LEGGIMI_0138_IT.md

```text
# SWOS Expanded 0.10.38
- MATCH STATS: risorsa grafica pulita senza testi prestampati, valori, etichette, nomi e risultato disegnati una sola volta dinamicamente.
- EXIT MATCH: rimossa freccia, cornice gialla di selezione attorno al pulsante attivo.
- Nessuna modifica alle logiche START, SELECT, BACK o EXIT.
- Non collaudato con eseguibile Windows.

```

---

## LEGGIMI_0_4_1.txt

```text
SWOS ORIGINALE 0.4.1 - PACCHETTO COMPLETO WINDOWS x64
====================================================
Questo archivio e' autonomo: include SWOS.exe, SDL2.dll e gli asset gia' presenti nei Test 0.2 + Fix 0.4.
NON estrarre dentro la cartella di una vecchia versione.
1. Estrai la cartella SWOS_Originale_0_4_1 dove preferisci.
2. Apri la cartella SWOS_Originale_0_4_1.
3. Controlla che SWOS.exe e SDL2.dll siano nella stessa cartella.
4. Fai doppio clic su GIOCA_SWOS_ORIGINALE.bat.
5. Se compare il menu, prova Friendly Game.
Se si blocca, invia %%USERPROFILE%%\Documents\SWOS\SWOS.log.

La versione 0.4.1 include il fix 0.4 che salta l'introduzione e copia alcune risorse aggiuntive. Non e' ancora stata testata su Windows reale: il funzionamento del gioco non e' garantito.

```

---

## LEGGIMI_FIX_04.txt

```text
SWOS ORIGINALE - FIX 0.4 (Windows x64)
===================================
Obiettivo: avviare i MENU e una PARTITA del SWOS originale.
Questo non e' SWOS Expanded e NON e' ancora testato su Windows.

COME INSTALLARE
1. Apri la cartella dove hai gia' estratto SWOS Test 0.2.
2. Estrai TUTTO questo ZIP nella medesima cartella, con le cartelle incluse.
3. Consenti la sostituzione di SWOS.exe.
4. Avvia GIOCA_SWOS_ORIGINALE.bat.
   Salta SOLO il filmato iniziale: non salta il caricamento dei menu o del gioco.
5. Se il menu appare, prova FRIENDLY GAME.

ALTERNATIVA PER DIAGNOSI:
- PROVA_INTRO_SENZA_MUSICA.bat: mostra l'introduzione ma salta la musica
  iniziale, per isolare l'eventuale problema audio.

Se compare un crash o lo schermo resta fermo, invia:
  %USERPROFILE%\Documents\SWOS\SWOS.log

Modifiche nel codice:
- Verifica delle dimensioni e del numero di bitplane dei RAW introduttivi.
- Tracce prima/dopo decodifica e avvio audio.
- SWOS_SKIP_INTRO=1 salta solo la sequenza iniziale.
- Frame opzionale LOADER76.RAW ignorato se assente.
- Tracce prima/dopo script immagini dei menu e cache menu.
- Aggiunti in grafs_aga i file OCS che il codice puo' caricare
  a profondita' mista (gli originali erano gia' nel Test 0.2).

Non modificare SWOS2020. La versione precedente e' recuperabile.

```

---

## LEGGIMI_PRIMA.txt

```text
SWOS Expanded 0.10.22 — BUSINESS e SQUAD / JOYPAD

Estrai lo ZIP in una nuova cartella, poi fai doppio clic su AVVIA_SWOS.bat.
Se MSYS2 e' gia' installato, il BAT compila automaticamente il gioco
la prima volta e poi lo avvia. Non occorre reinstallare il compilatore.

Leggi LEGGIMI_0122_IT.txt per le modifiche, i comandi del joypad e i test.
Se non compila, inviami COMPILAZIONE_0122.log.

```

---

## PATCH_0119_TO_0120.diff

```text
--- a/sorgenti/src/expanded_economy.c
+++ b/sorgenti/src/expanded_economy.c
@@ -13,6 +13,8 @@
 #include "expanded_squad_rating.h"
 #include "teams.h"
 #include "expanded_facility_rules.h"
+#include "expanded_player_condition.h"
+#include "expanded_squad_layout.h"
 
 extern int32_t _newBalance, _gateReceipts, _competitionBonuses;
 extern uint16_t _selTeamNumber, _currentSeasonShowing;
@@ -54,6 +56,26 @@
 uint8_t _expandedResults[5]={0},_expandedResultCount=0,_expandedResultHead=0;
 uint8_t _expandedTravelFatigue=0;
 uint8_t _expandedCurrentForm=50;
+/* 0.10.20: per-file-index player condition (NOT PlayerFile.form).
+ * Form is a 0..100 measure; original SWOS reserve/trialist sentinels remain
+ * untouched. All 32 career-team slots start at 50, including empty slots. */
+uint8_t _expandedPlayerCondition[32] = {
+    50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,
+    50,50,50,50,50,50,50,50,50,50,50,50,50,50,50,50
+};
+unsigned Expanded_PlayerCondition(unsigned fileIndex) {
+    return fileIndex<32 ? (_expandedPlayerCondition[fileIndex]>100?100:
+                           _expandedPlayerCondition[fileIndex]) : 50u;
+}
+unsigned Expanded_PlayerPerformancePct(unsigned fileIndex) {
+    return Expanded_ConditionPerformancePct(Expanded_PlayerCondition(fileIndex));
+}
+void Expanded_ResetPlayerCondition(unsigned fileIndex) {
+    if(fileIndex<32) _expandedPlayerCondition[fileIndex]=50;
+}
+static void reset_all_player_conditions(void) {
+    memset(_expandedPlayerCondition,50,sizeof _expandedPlayerCondition);
+}
 static uint8_t s_businessModalBusy=0;
 extern uint8_t _lastFinancialWeekIndex;
 extern volatile uint16_t _youreSacked;
@@ -61,6 +83,7 @@
 extern uint16_t _careerNumPlayers;
 extern uint8_t _playerFormModifiers[32];
 extern uint8_t _careerTeam[];
+extern uint8_t _currentMatchPlayers[32];
 extern uint16_t Rand(void);
 static uint32_t s_offerOptions[3];
 static uint32_t s_tvProposed=0;
@@ -114,6 +137,14 @@
     int64_t balance=(int64_t)_newBalance-(int64_t)staff+sponsor+tv;
     _newBalance=balance>INT32_MAX?INT32_MAX:balance<INT32_MIN?INT32_MIN:(int32_t)balance;
     unsigned recover=Expanded_FitnessRecovery(_expandedFacilityLevels[4]);
+    /* Every contracted player recovers gradually even without training.
+     * No overwrite of SWOS's signed playerFormModifiers. */
+    for(unsigned i=0;i<_careerNumPlayers && i<32;i++) {
+        unsigned fileIndex=(unsigned)_currentMatchPlayers[i];
+        if(fileIndex<32)
+            _expandedPlayerCondition[fileIndex]=Expanded_ConditionChange(
+                _expandedPlayerCondition[fileIndex],(int)(recover/2u));
+    }
     _expandedTravelFatigue=_expandedTravelFatigue>recover?
          (uint8_t)(_expandedTravelFatigue-recover):0;
     _expandedCurrentForm=(uint8_t)Expanded_RecentForm(_expandedResults,
@@ -126,11 +157,21 @@
          (uint8_t)(_expandedTravelFatigue-gain):0;
     _expandedCurrentForm=(uint8_t)Expanded_RecentForm(_expandedResults,
         _expandedResultCount,_expandedTravelFatigue);
-    /* The same career form modifiers used by the match engine, not fake
-     * permanent skill upgrades. Keep signed 8-bit range and trialists safe. */
+    /* Training improves the new 0..100 condition, with diminishing
+     * returns at 100, in addition to the original SWOS form modifiers. */
+    for(unsigned i=0;i<_careerNumPlayers && i<32;i++) {
+        unsigned fileIndex=(unsigned)_currentMatchPlayers[i];
+        if(fileIndex<32)
+            _expandedPlayerCondition[fileIndex]=Expanded_ConditionChange(
+                _expandedPlayerCondition[fileIndex],(int)gain);
+    }
+    /* Preserve SWOS's existing form and trialist handling. */
     for(unsigned i=0;i<_careerNumPlayers && i<32;i++) {
         int v=(int8_t)_playerFormModifiers[i];
-        if(v>=-16 && v<16) _playerFormModifiers[i]=(uint8_t)(int8_t)(v+(int)(gain/4u)>16?16:v+(int)(gain/4u));
+        if(v>=-16 && v<16) {
+            int next=v+(int)(gain/4u);
+            _playerFormModifiers[i]=(uint8_t)(int8_t)(next>16?16:next);
+        }
     }
 }
 void Expanded_ToggleFinanceView(void) {
@@ -257,6 +298,15 @@
     _expandedResults[_expandedResultHead]=(uint8_t)(result>0?3:result<0?0:1);
     _expandedResultHead=(uint8_t)((_expandedResultHead+1u)%5u);
     if(_expandedResultCount<5) _expandedResultCount++;
+    {
+        int cost=4+(away?(int)(Expanded_AwayFatigue(_expandedFacilityLevels[2])/2u):0);
+        for(unsigned i=0;i<11 && i<_careerNumPlayers;i++) {
+            unsigned fileIndex=(unsigned)_currentMatchPlayers[i];
+            if(fileIndex<32)
+                _expandedPlayerCondition[fileIndex]=Expanded_ConditionChange(
+                    _expandedPlayerCondition[fileIndex],-cost);
+        }
+    }
     if(away) {
         unsigned fatigue=_expandedTravelFatigue+Expanded_AwayFatigue(_expandedFacilityLevels[2]);
         _expandedTravelFatigue=(uint8_t)(fatigue>60?60:fatigue);
@@ -325,6 +375,7 @@
     _expandedResultCount=_expandedResultHead=_expandedTravelFatigue=0;
     memset(_expandedResults,0,sizeof _expandedResults);
     _expandedCurrentForm=50;
+    reset_all_player_conditions();
     _expandedFinanceSeasonView=0;
     _expandedBoardMood=(uint8_t)Expanded_InitialBoardLiking(
         _expandedManagerFame,_expandedContractPremiumPct,
@@ -348,6 +399,7 @@
     _expandedResultCount=_expandedResultHead=_expandedTravelFatigue=0;
     memset(_expandedResults,0,sizeof _expandedResults);
     _expandedCurrentForm=50;
+    reset_all_player_conditions();
     _expandedLowConfidenceMatches=0;
     _expandedContractPremiumPct=0;_expandedSuccessfulRaises=0;
     s_expandedDismissalPending=0;
@@ -540,8 +592,8 @@
      * ordinals stable, but is not visible or selectable. */
     _menuEntries[0].isInvisible=1; /* artwork replaces stock title */
     _menuEntries[9].isInvisible=0; /* compact WEEK/SEASON finance switch */
-    _menuEntries[9].x=189;_menuEntries[9].y=186;
-    _menuEntries[9].dwWidth=17;_menuEntries[9].dwHeight=13;
+    _menuEntries[9].x=143;_menuEntries[9].y=185;
+    _menuEntries[9].dwWidth=64;_menuEntries[9].dwHeight=16;
     _menuEntries[9].background=NO_BACKGROUND;
     _menuEntries[9].content=NO_FOREGROUND;
     _menuEntries[9].leftEntry=10;
@@ -795,7 +847,7 @@
     if (!menu || !menu->selectedEntry) return 1;
     unsigned ordinal=menu->selectedEntry->ordinal;
     int x,y,w,h;
-    if(ordinal==9) {x=189;y=186;w=17;h=13;}
+    if(ordinal==9) {x=143;y=185;w=64;h=16;}
     else if(ordinal==1) {x=243;y=108;w=70;h=18;}
     else if(ordinal==10) {x=5;y=3;w=47;h=17;}
     else if(ordinal>=2 && ordinal<=8) {
@@ -876,9 +928,10 @@
     snprintf(t,sizeof t,"FINANCES / %s %02u",_expandedFinanceSeasonView?"SEASON":"WEEK",
              (unsigned)_lastFinancialWeekIndex+1u);
     cb_text(9,189,white,t);
-    /* Small W/S switch: the approved art and finance panels remain intact. */
-    rect(190,187,15,11,42);cb_box_outline(190,187,15,11,204);
-    cb_center_narrow(197,189,gold,_expandedFinanceSeasonView?"W":"S");
+    /* Full-label WEEK/SEASON switch, wider but entirely inside the finance
+     * header. It shows the destination view, not an ambiguous W/S letter. */
+    rect(143,185,64,16,42);cb_box_outline(143,185,64,16,204);
+    cb_center_narrow(175,189,gold,_expandedFinanceSeasonView?"WEEK":"SEASON");
     cb_center(264,189,white,"BALANCE");
     static const char *const leftLabel[4]={"SPONSOR","TV","GATE","INCOME"};
     static const char *const rightLabel[4]={"WAGES","TRAVEL","STAFF","TOTAL"};
@@ -1194,7 +1247,7 @@
     for(unsigned i=28;i<=44;i++) {
         /* Native selection/drag rectangle must match the custom 8px row.
          * Its previous y was TWO rows above the live blue highlight. */
-        ui_move(i,10,90+(int)(i-28u)*8,300,8);
+        ui_move(i,10,Expanded_SquadRowY(i-28u),300,EXP_SQUAD_ROW_H);
         _menuEntries[i].background=NO_BACKGROUND;
         _menuEntries[i].content=NO_FOREGROUND;
     }
@@ -1633,6 +1686,8 @@
 extern uint8_t _shortCountryNames[];
 extern int32_t _newBalance;
 extern const PlayerFile *Expanded_SquadRowPlayer(unsigned row);
+extern unsigned Expanded_SquadRowFileIndex(unsigned row);
+extern uint16_t _squadPrevSelectedPlayerIndex;
 static ExpandedSquadSkills ui_player_skills(const PlayerFile *p) {
     ExpandedSquadSkills s={0};
     if(!p) return s;
@@ -1653,35 +1708,37 @@
 static void ui_squad_tooltip(unsigned row,const PlayerFile *p) {
     if(!p) return;
     ExpandedSquadSkills s=ui_player_skills(p);
-    const char *names[7]={"passing","velocity","heading","tackling",
-                          "control","speed","finishing"};
+    static const char *const names[7]={"PASSING","VELOCITY","HEADING","TACKLING",
+                                      "CONTROL","SPEED","FINISHING"};
     unsigned values[7]={s.passing,s.velocity,s.heading,s.tackling,
                         s.control,s.speed,s.finishing};
-    /* The tooltip follows focus from the REAL menu selection. D-pad/joypad
-     * movement selects a row without requiring FIRE; keyboard does the same. */
-    const int x=131,w=107,h=99;
-    int y=90+(int)row*8-7;
-    if(y<94) y=94;
-    if(y>126) y=126;
+    /* Compact popup: starts at the MIDPOINT of the player-name column.
+     * Position it strictly ABOVE or BELOW the focused 8px row, never on it.
+     * Bottom half moves above, upper half below. Joypad and mouse share
+     * selectedEntry, so both get exactly the same placement. */
+    const int x=EXP_SQUAD_TOOLTIP_X,w=EXP_SQUAD_TOOLTIP_W,
+              h=EXP_SQUAD_TOOLTIP_H;
+    const int y=Expanded_SquadTooltipY(row);
     rect(x+2,y+2,w-2,h-2,40);
     cb_box_outline(x,y,w,h,204);
-    rect(x+2,y+2,w-4,13,46);
-    ui_native_ellipsis(x+5,y+4,w-10,15,(const char*)p->playerName,-1);
+    rect(x+2,y+2,w-4,12,46);
+    ui_native_ellipsis(x+4,y+3,w-8,15,(const char*)p->playerName,-1);
     char line[32];
     for(unsigned i=0;i<7;i++) {
-        int yy=y+17+(int)i*10;
-        if(i&1) rect(x+3,yy-1,w-6,10,42);
-        ui_native_text(x+5,yy,72,2,names[i],-1);
+        int yy=y+15+(int)i*8;
+        if(i&1) rect(x+2,yy,w-4,8,42);
+        ui_native_text(x+4,yy,57,2,names[i],-1);
         snprintf(line,sizeof line,"%u",values[i]);
-        ui_native_text(x+w-6,yy,17,15,line,1);
-    }
-    /* No invented morale/fitness attributes: form is the signed runtime
-     * SWOS field; trialist/reserve are displayed by their real sentinels. */
-    if((uint8_t)p->form==PLAYER_FORM_TRIALIST) snprintf(line,sizeof line,"TRIAL");
-    else if((uint8_t)p->form==PLAYER_FORM_RESERVE) snprintf(line,sizeof line,"RES");
-    else snprintf(line,sizeof line,"%d",(int)p->form);
-    ui_native_text(x+5,y+88,72,2,"form",-1);
-    ui_native_text(x+w-6,y+88,24,15,line,1);
+        ui_native_text(x+w-5,yy,15,15,line,1);
+    }
+    /* This is a NEW per-player 0..100 condition, initially 50, not the
+     * SWOS PlayerFile.form byte (which encodes TRIAL/RESERVE). */
+    unsigned fileIndex=Expanded_SquadRowFileIndex(row);
+    unsigned condition=(uintptr_t)_squadTeamFilePtr==(uintptr_t)_careerTeam?
+                       Expanded_PlayerCondition(fileIndex):50u;
+    ui_native_text(x+4,y+72,57,2,"FORM",-1);
+    snprintf(line,sizeof line,"%u",condition);
+    ui_native_text(x+w-5,y+72,19,15,line,1);
 }
 static void ui_squad_render(void) {
     cb_blit(0,0,CB_W,CB_H,ui_squad);
@@ -1730,11 +1787,23 @@
     for(unsigned i=0;i<17;i++) {
         const PlayerFile *p=Expanded_SquadRowPlayer(i);
         if(!p) continue;
-        int y=90+(int)i*8;
-        if(selected==i) {
+        int y=Expanded_SquadRowY(i);
+        /* Pending swap is orange-red, even when the cursor moves away.
+         * The focused row is blue with white SIDE markers only: no white
+         * horizontal line is allowed to slice through player-name glyphs. */
+        unsigned fileIndex=Expanded_SquadRowFileIndex(i);
+        int swapping=_squadPrevSelectedPlayerIndex!=0xFFFFu &&
+                     fileIndex==_squadPrevSelectedPlayerIndex;
+        if(swapping) {
+            rect(10,y,300,8,72);
+            rect(10,y,3,8,10);
+            rect(307,y,3,8,10);
+        } else if(selected==i) {
             rect(10,y,300,8,11);
-            selectedPlayer=p;
+            rect(10,y,2,8,15);
+            rect(308,y,2,8,15);
         }
+        if(selected==i) selectedPlayer=p;
         snprintf(line,sizeof line,"%u",(unsigned)(uint8_t)p->shirtNumber);
         ui_native_text(24,y,17,2,line,1);
         ui_native_ellipsis(31,y,97,2,(const char*)p->playerName,-1);
@@ -1897,7 +1966,7 @@
                 name=(const char *)(uintptr_t)e->fg;
             ui_button(i,name);
         }
-        ui_native_text(43,246,70,15,"V0.10.19",0);
+        ui_native_text(43,246,70,15,"V0.10.20",0);
     } else if(kind==2) {
         Menu *m=(Menu *)_currentMenu;
         if(!m) return;
--- a/sorgenti/src/menus.c
+++ b/sorgenti/src/menus.c
@@ -1594,11 +1594,16 @@
 }
 
 static const PlayerFile *s_expandedSquadRow[17];
+static uint16_t s_expandedSquadRowIndex[17];
 void Expanded_SquadClearRowCache(void) {
     memset(s_expandedSquadRow,0,sizeof s_expandedSquadRow);
+    for(unsigned i=0;i<17;i++) s_expandedSquadRowIndex[i]=0xFFFFu;
 }
 const PlayerFile *Expanded_SquadRowPlayer(unsigned row) {
     return row<17 ? s_expandedSquadRow[row] : NULL;
+}
+unsigned Expanded_SquadRowFileIndex(unsigned row) {
+    return row<17 ? s_expandedSquadRowIndex[row] : 0xFFFFu;
 }
 /* SquadAfterDrawCommon (SWOS2.s line 48777)
  * AfterDraw callback for squad menu player rows. Sets row colors,
@@ -1643,7 +1648,10 @@
     uint16_t d6_breakPos, d4_fileIdx;
     uint8_t *player = GetPlayerInfoBreakPosition(teamBase, d7, &d6_breakPos, &d4_fileIdx);
     PlayerFile *pf = (PlayerFile *)(player + sizeof(TeamFile));
-    if(d7<17) s_expandedSquadRow[d7]=pf;
+    if(d7<17) {
+        s_expandedSquadRow[d7]=pf;
+        s_expandedSquadRowIndex[d7]=d4_fileIdx;
+    }
 
     /* Check if injured — DISPLAY decision (row goes red), so use the honest
      * status rather than IsPlayerInjured, whose testers'-rule (test_rules.h)
--- a/sorgenti/src/menu_system.c
+++ b/sorgenti/src/menu_system.c
@@ -27,6 +27,7 @@
 extern MenuEntry _menuEntries[];
 extern uintptr_t _currentMenuPtr;
 extern uint8_t _contestMenu[];
+extern uint8_t _squadMenu[];
 extern volatile uint16_t _menuStatus;
 extern uint16_t _menuFade;
 extern volatile uint16_t _exitMenu;
@@ -654,6 +655,14 @@
         extern int Expanded_UI_Active(void);
         if (Expanded_UI_Active()) return;
     }
+    /* The expanded squad is entirely redrawn by Expanded_UI_After. Stock
+     * decorative sprites, text columns and arrow fragments are redundant
+     * and could draw beyond the 320px image on widescreen. Preserve all
+     * callbacks (including row afterDraw, which caches real PlayerFile*). */
+    if (_currentMenuPtr==(uintptr_t)_squadMenu) {
+        extern int Expanded_UI_Active(void);
+        if (Expanded_UI_Active()) goto post_draw_callback;
+    }
 #endif
     /* deadVar03 check — if set, skip to afterDraw */
     if (_deadVar03 != 0)
@@ -1333,6 +1342,12 @@
 
     if (sel == (MenuEntry *)0)
         return;
+#ifdef SWOS_SDL_BUILD
+    if (_currentMenuPtr==(uintptr_t)_squadMenu) {
+        extern int Expanded_UI_Active(void);
+        if (Expanded_UI_Active()) return;
+    }
+#endif
 
     w = sel->dwWidth;
     h = sel->dwHeight;
--- a/sorgenti/src/save_format_json.c
+++ b/sorgenti/src/save_format_json.c
@@ -1097,6 +1097,7 @@
 extern uint32_t _expandedStaffLastWeek,_expandedSponsorLastWeek,_expandedTvLastWeek;
 extern uint8_t _expandedResults[5],_expandedResultCount,_expandedResultHead;
 extern uint8_t _expandedTravelFatigue,_expandedCurrentForm;
+extern uint8_t _expandedPlayerCondition[32];
 static const FieldDesc s_career_scalars[] = {
     FX("expanded_economy_club_id", _expandedEconomyClubId, FLD_U16),
     FX("expanded_stadium_level", _expandedStadiumLevel, FLD_U8),
@@ -1134,6 +1135,7 @@
     FX("expanded_recent_result_head", _expandedResultHead, FLD_U8),
     FX("expanded_travel_fatigue", _expandedTravelFatigue, FLD_U8),
     FX("expanded_current_form", _expandedCurrentForm, FLD_U8),
+    FA("expanded_player_condition", _expandedPlayerCondition, 32, FLD_BYTES),
     FX("expanded_manager_fame", _expandedManagerFame, FLD_U8),
     FX("expanded_manager_start_age", _expandedManagerStartAge, FLD_U8),
     FX("expanded_annual_salary", _expandedAnnualSalary, FLD_U32),
--- a/sorgenti/src/career_status.c
+++ b/sorgenti/src/career_status.c
@@ -2694,6 +2694,8 @@
     }
 
     _careerNumPlayers += 1;
+    { extern void Expanded_ResetPlayerCondition(unsigned);
+      Expanded_ResetPlayerCondition(freeSlot); }
     Career_UpdatePlayerWeeklyWageCache(freeSlot);
 
     return player;
--- a/sorgenti/src/transfer_market.c
+++ b/sorgenti/src/transfer_market.c
@@ -4307,6 +4307,9 @@
     SetBenchPlayersNumbers(&_careerTeam);
 
     _careerNumPlayers++;
+    /* A newly recruited player starts at 50/100 condition. */
+    { extern void Expanded_ResetPlayerCondition(unsigned);
+      Expanded_ResetPlayerCondition(freeSlot); }
     SquadNumbers_Free(freeSlot);        /* (real squad numbers: his own, or a free one; teams.c) */
     Career_UpdatePlayerWeeklyWageCache(freeSlot);
 
@@ -4806,6 +4809,8 @@
     /* Add to currentMatchPlayers */
     _currentMatchPlayers[_careerNumPlayers] = (uint8_t)freeSlot;
     _careerNumPlayers++;
+    { extern void Expanded_ResetPlayerCondition(unsigned);
+      Expanded_ResetPlayerCondition(freeSlot); }
 
     Career_UpdatePlayerWeeklyWageCache(freeSlot);
 }
--- a/sorgenti/src/build_info.c
+++ b/sorgenti/src/build_info.c
@@ -87,7 +87,7 @@
 
 const char *Build_Version(void)
 {
-    return "SWOS Expanded 0.10.19 - " PROJECT_NAME " " SWOS_BUILD_ID
+    return "SWOS Expanded 0.10.20 - " PROJECT_NAME " " SWOS_BUILD_ID
            " (" SWOS_BUILD_DATE ", " SWOS_PLATFORM SWOS_BUILD_RULES ")";
 }
 
--- a/AVVIA_SWOS.bat
+++ b/AVVIA_SWOS.bat
@@ -1,7 +1,7 @@
 @echo off
 setlocal EnableExtensions
 cd /d "%~dp0"
-title SWOS Expanded 0.10.19 - Avvio
+title SWOS Expanded 0.10.20 - Avvio
 
 REM La prima volta compila i sorgenti corretti; dopo avvia direttamente il gioco.
 call "%~dp0PREPARA_SWOS.bat"
@@ -27,7 +27,7 @@
 set "SWOS_TACTICS_JSON=0"
 set "SWOS_MENU_NAV_LOG=1"
 echo.
-echo === SWOS Expanded 0.10.19 - avvio ===
+echo === SWOS Expanded 0.10.20 - avvio ===
 "%~dp0SWOS.exe"
 set "RC=%ERRORLEVEL%"
 echo.
@@ -39,6 +39,6 @@
 :errore
 echo.
 echo Il gioco non e stato avviato.
-echo Se la compilazione e fallita, invia COMPILAZIONE_0119.log.
+echo Se la compilazione e fallita, invia COMPILAZIONE_0120.log.
 pause
 exit /b 1
--- a/PREPARA_SWOS.bat
+++ b/PREPARA_SWOS.bat
@@ -1,11 +1,11 @@
 @echo off
 setlocal EnableExtensions
 cd /d "%~dp0"
-title SWOS Expanded 0.10.19 - Preparazione
+title SWOS Expanded 0.10.20 - Preparazione
 
 REM Un eseguibile 0.10 precedente non e considerato valido.
-if exist ".SWOS_0119_COMPILATO" if exist "SWOS.exe" (
-  echo SWOS 0.10.19 pronto.
+if exist ".SWOS_0120_COMPILATO" if exist "SWOS.exe" (
+  echo SWOS 0.10.20 pronto.
   exit /b 0
 )
 
@@ -48,14 +48,14 @@
 "%MSYS_ROOT%\usr\bin\bash.exe" -lc "bash -s" < "%~dp0BUILD_SWOS.sh"
 if errorlevel 1 (
   echo.
-  echo COMPILAZIONE FALLITA: invia COMPILAZIONE_0119.log.
+  echo COMPILAZIONE FALLITA: invia COMPILAZIONE_0120.log.
   exit /b 1
 )
 if not exist "SWOS.exe" (
   echo ERRORE: compilazione terminata senza SWOS.exe.
   exit /b 1
 )
-> ".SWOS_0119_COMPILATO" echo SWOS Expanded 0.10.19 - compilazione completata
+> ".SWOS_0120_COMPILATO" echo SWOS Expanded 0.10.20 - compilazione completata
 exit /b 0
 
 :cerca_msys
--- a/BUILD_SWOS.sh
+++ b/BUILD_SWOS.sh
@@ -23,9 +23,9 @@
     exit 1
   fi
 done
-exec > >(tee "$game/COMPILAZIONE_0119.log") 2>&1
+exec > >(tee "$game/COMPILAZIONE_0120.log") 2>&1
 
-echo '==== SWOS Expanded 0.10.19 - compilazione Windows x64 ===='
+echo '==== SWOS Expanded 0.10.20 - compilazione Windows x64 ===='
 echo "Sorgenti: $src"
 echo "Gioco:    $game"
 echo 'Verifica delle dipendenze MSYS2...'
@@ -41,12 +41,12 @@
   -Isrc -Iinclude $(pkg-config --cflags sdl2) \
   src/menu_data.c src/menu_system.c src/menus.c src/expanded_career.c \
   src/expanded_economy.c src/career_init.c src/career_status.c src/career_teams.c src/transfer_market.c src/chairman_memo.c src/save_format_json.c src/input_text.c src/blitter.c src/build_info.c
-echo 'Compilo il gioco 0.10.19...'
+echo 'Compilo il gioco 0.10.20...'
 make -j4 NETPLAY=0
 if [[ ! -s bin/SWOS.exe ]]; then
   echo 'ERRORE: il compilatore non ha prodotto bin/SWOS.exe.'
   exit 1
 fi
-cp -f bin/SWOS.exe "$game/SWOS_0119_new.exe"
-mv -f "$game/SWOS_0119_new.exe" "$game/SWOS.exe"
+cp -f bin/SWOS.exe "$game/SWOS_0120_new.exe"
+mv -f "$game/SWOS_0120_new.exe" "$game/SWOS.exe"
 echo 'Compilazione completata: SWOS.exe aggiornato.'

```

---

## PATCH_0120_TO_0121.diff

```text
--- 0.10.20/sorgenti/src/expanded_economy.c
+++ 0.10.21/sorgenti/src/expanded_economy.c
@@ -15,6 +15,7 @@
 #include "expanded_facility_rules.h"
 #include "expanded_player_condition.h"
 #include "expanded_squad_layout.h"
+#include "expanded_business_rules.h"
 
 extern int32_t _newBalance, _gateReceipts, _competitionBonuses;
 extern uint16_t _selTeamNumber, _currentSeasonShowing;
@@ -53,6 +54,8 @@
 uint8_t _expandedFinanceSeasonView=0;
 uint32_t _expandedStaffSeason=0, _expandedSponsorPaidSeason=0, _expandedTvPaidSeason=0;
 uint32_t _expandedStaffLastWeek=0, _expandedSponsorLastWeek=0, _expandedTvLastWeek=0;
+uint32_t _expandedTravelLastWeek=0, _expandedTravelSeason=0;
+uint8_t _expandedBusinessStaffHover=0;
 uint8_t _expandedResults[5]={0},_expandedResultCount=0,_expandedResultHead=0;
 uint8_t _expandedTravelFatigue=0;
 uint8_t _expandedCurrentForm=50;
@@ -77,7 +80,8 @@
     memset(_expandedPlayerCondition,50,sizeof _expandedPlayerCondition);
 }
 static uint8_t s_businessModalBusy=0;
-extern uint8_t _lastFinancialWeekIndex;
+extern uint8_t _lastFinancialWeekIndex, _careerJobOfferPhaseBias;
+extern uint16_t _seasonWeeksDeducted;
 extern volatile uint16_t _youreSacked;
 extern int32_t _generalRunningCosts,_seasonRunningCostsAccum,_seasonRunningCostsAccum2;
 extern uint16_t _careerNumPlayers;
@@ -209,7 +213,7 @@
 void Expanded_BusinessMaybeOffer(void) {
     if(s_businessModalBusy || _youreSacked || _selTeamNumber==0xFFFFu ||
        _expandedEconomyClubId!=_selTeamNumber) return;
-    unsigned week=(unsigned)_lastFinancialWeekIndex+1u;
+    unsigned week=Expanded_FinanceWeek(_lastFinancialWeekIndex,_careerJobOfferPhaseBias);
     if(week<2) return;
     s_businessModalBusy=1;
     Expanded_PrepareOffers();
@@ -307,7 +311,21 @@
                     _expandedPlayerCondition[fileIndex],-cost);
         }
     }
+    /* Travel is charged per AWAY match, not every week.  Home = zero.
+     * Original weeklyRunningCosts is club overhead, never mislabeled as travel.
+     * All charges enter the same SWOS finance ledger and are saved. */
+    _expandedTravelLastWeek=away?Expanded_TravelMatchCost(_expandedFacilityLevels[2]):0;
     if(away) {
+        uint32_t cost=_expandedTravelLastWeek;
+        _expandedTravelSeason=sat_add(_expandedTravelSeason,cost);
+        int64_t nextBalance=(int64_t)_newBalance-(int64_t)cost;
+        _newBalance=nextBalance<INT32_MIN?INT32_MIN:(int32_t)nextBalance;
+        _generalRunningCosts=_generalRunningCosts>INT32_MAX-(int32_t)cost?
+            INT32_MAX:_generalRunningCosts+(int32_t)cost;
+        _seasonRunningCostsAccum=_seasonRunningCostsAccum>INT32_MAX-(int32_t)cost?
+            INT32_MAX:_seasonRunningCostsAccum+(int32_t)cost;
+        _seasonRunningCostsAccum2=_seasonRunningCostsAccum2>INT32_MAX-(int32_t)cost?
+            INT32_MAX:_seasonRunningCostsAccum2+(int32_t)cost;
         unsigned fatigue=_expandedTravelFatigue+Expanded_AwayFatigue(_expandedFacilityLevels[2]);
         _expandedTravelFatigue=(uint8_t)(fatigue>60?60:fatigue);
     }
@@ -372,6 +390,8 @@
     _expandedSponsorState=_expandedTvState=0;
     _expandedStaffSeason=_expandedSponsorPaidSeason=_expandedTvPaidSeason=0;
     _expandedStaffLastWeek=_expandedSponsorLastWeek=_expandedTvLastWeek=0;
+    _expandedTravelLastWeek=_expandedTravelSeason=0;
+    _expandedBusinessStaffHover=0;
     _expandedResultCount=_expandedResultHead=_expandedTravelFatigue=0;
     memset(_expandedResults,0,sizeof _expandedResults);
     _expandedCurrentForm=50;
@@ -396,6 +416,8 @@
     _expandedSponsorState=_expandedTvState=_expandedFinanceSeasonView=0;
     _expandedStaffSeason=_expandedSponsorPaidSeason=_expandedTvPaidSeason=0;
     _expandedStaffLastWeek=_expandedSponsorLastWeek=_expandedTvLastWeek=0;
+    _expandedTravelLastWeek=_expandedTravelSeason=0;
+    _expandedBusinessStaffHover=0;
     _expandedResultCount=_expandedResultHead=_expandedTravelFatigue=0;
     memset(_expandedResults,0,sizeof _expandedResults);
     _expandedCurrentForm=50;
@@ -422,6 +444,8 @@
     _expandedSponsorState=_expandedTvState=0;
     _expandedStaffSeason=_expandedSponsorPaidSeason=_expandedTvPaidSeason=0;
     _expandedStaffLastWeek=_expandedSponsorLastWeek=_expandedTvLastWeek=0;
+    _expandedTravelLastWeek=_expandedTravelSeason=0;
+    _expandedBusinessStaffHover=0;
     /* The annual scouting report refreshes with the season. It is a report
      * capacity metric, NOT fictitious players added to the database. */
     _expandedScoutingYear = _currentSeasonShowing;
@@ -452,7 +476,7 @@
     snprintf(_expandedFacilityDetailLines[0],72,"%s  LEVEL %u/5",s_facilityNames[idx],lv);
     if (idx==0) snprintf(_expandedFacilityDetailLines[1],72,"CAPACITY %lu",(unsigned long)s_capacity[lv-1]);
     else if (idx==1) snprintf(_expandedFacilityDetailLines[1],72,"TRAINING EFFECT +%u",Expanded_TrainingGain(lv,_expandedFacilityLevels[4]));
-    else if (idx==2) snprintf(_expandedFacilityDetailLines[1],72,"%s / FATIGUE %u",s_transportNames[lv-1],Expanded_AwayFatigue(lv));
+    else if (idx==2) snprintf(_expandedFacilityDetailLines[1],72,"AWAY TRAVEL: %c%u / MATCH",c,Expanded_TravelMatchCost(lv));
     else if (idx==3) snprintf(_expandedFacilityDetailLines[1],72,"SPONSOR APPEAL %u%%",Expanded_OfficeSponsorPct(lv));
     else if (idx==4) snprintf(_expandedFacilityDetailLines[1],72,"WEEKLY RECOVERY +%u",Expanded_FitnessRecovery(lv));
     else if (idx==5) snprintf(_expandedFacilityDetailLines[1],72,"SCOUT REACH: %s",
@@ -461,7 +485,7 @@
     else if (idx==7) snprintf(_expandedFacilityDetailLines[1],72,"YOUTH QUALITY +%u",Expanded_YouthPriceBonus(lv));
     else snprintf(_expandedFacilityDetailLines[1],72,"UPGRADE FOR BETTER CLUB SERVICES");
     snprintf(_expandedFacilityDetailLines[2],72,"%s: %c%u / WEEK",
-             idx==5?"SCOUT WAGES":idx==6?"MEDICAL WAGES":"STAFF",c,
+             "STAFF",c,
              Expanded_StaffWeekly(idx,lv));
     if(lv==5) snprintf(_expandedFacilityDetailLines[3],72,"MAXIMUM LEVEL REACHED");
     else snprintf(_expandedFacilityDetailLines[3],72,"UPGRADE COST: %c%lu",c,(unsigned long)facility_cost(idx));
@@ -592,8 +616,8 @@
      * ordinals stable, but is not visible or selectable. */
     _menuEntries[0].isInvisible=1; /* artwork replaces stock title */
     _menuEntries[9].isInvisible=0; /* compact WEEK/SEASON finance switch */
-    _menuEntries[9].x=143;_menuEntries[9].y=185;
-    _menuEntries[9].dwWidth=64;_menuEntries[9].dwHeight=16;
+    _menuEntries[9].x=164;_menuEntries[9].y=184;
+    _menuEntries[9].dwWidth=51;_menuEntries[9].dwHeight=13;
     _menuEntries[9].background=NO_BACKGROUND;
     _menuEntries[9].content=NO_FOREGROUND;
     _menuEntries[9].leftEntry=10;
@@ -841,21 +865,55 @@
     rect(x,y,w,1,color);rect(x,y+h-1,w,1,color);
     rect(x,y,1,h,color);rect(x+w-1,y,1,h,color);
 }
+static void cb_staff_popup(void) {
+    if(_expandedBusinessStaffHover) {
+        extern int32_t _weeklyRunningCosts;
+        const uint8_t white=2,gold=15;
+        char money[48];
+        uint64_t weeklyStaff=Expanded_FacilityWeeklyTotal()+nonneg(_weeklyRunningCosts);
+        /* A solid, compact 2-column cost breakdown.  The 'OTHER' item
+         * includes SWOS general running costs and the base 45/week staff;
+         * the other 8 entries are the real facility wage table. */
+        const int px=81,py=123,pw=154,ph=59;
+        rect(px,py,pw,ph,40);cb_box_outline(px,py,pw,ph,204);
+        rect(px+1,py+1,pw-2,10,46);
+        cb_native(px+pw/2,py+2,15,"STAFF / WEEK",0,pw-8);
+        static const char *const sn[10]={"OTHER","STAD","TRAIN","TRANS","OFFICE",
+                                         "FIT","SCOUT","MED","YOUTH","TOTAL"};
+        uint64_t amounts[10]={nonneg(_weeklyRunningCosts)+45u,
+            Expanded_StaffWeekly(0,_expandedFacilityLevels[0]),
+            Expanded_StaffWeekly(1,_expandedFacilityLevels[1]),
+            Expanded_StaffWeekly(2,_expandedFacilityLevels[2]),
+            Expanded_StaffWeekly(3,_expandedFacilityLevels[3]),
+            Expanded_StaffWeekly(4,_expandedFacilityLevels[4]),
+            Expanded_StaffWeekly(5,_expandedFacilityLevels[5]),
+            Expanded_StaffWeekly(6,_expandedFacilityLevels[6]),
+            Expanded_StaffWeekly(7,_expandedFacilityLevels[7]),weeklyStaff};
+        for(unsigned i=0;i<10;i++) {
+            int col=(int)(i/5u),yy=py+12+(int)(i%5u)*9;
+            int xx=px+4+col*76;
+            cb_native(xx,yy,i==9?15:white,sn[i],-1,49);
+            cb_compact(money,sizeof money,amounts[i],0);
+            cb_native(xx+72,yy,i==9?15:gold,money,1,26);
+        }
+    }
+}
 static int cb_draw_selection(void) {
     if (!cb_load_assets()) return 0;
     Menu *menu=(Menu *)_currentMenu;
-    if (!menu || !menu->selectedEntry) return 1;
+    if (!menu || !menu->selectedEntry) {cb_staff_popup();return 1;}
     unsigned ordinal=menu->selectedEntry->ordinal;
     int x,y,w,h;
-    if(ordinal==9) {x=143;y=185;w=64;h=16;}
+    if(ordinal==9) {x=164;y=184;w=51;h=13;}
     else if(ordinal==1) {x=243;y=108;w=70;h=18;}
     else if(ordinal==10) {x=5;y=3;w=47;h=17;}
     else if(ordinal>=2 && ordinal<=8) {
         static const uint8_t cardByOrdinal[9]={0,0,1,3,5,0,2,4,6};
         x=5+44*cardByOrdinal[ordinal];y=133;w=43;h=48;
-    } else return 1;
+    } else {cb_staff_popup();return 1;}
     cb_box_outline(x,y,w,h,15);
     cb_box_outline(x+1,y+1,w-2,h-2,6);
+    cb_staff_popup(); /* keep the hover above the selected facility outline */
     return 1;
 }
 static int cb_draw_full(void) {
@@ -906,32 +964,32 @@
         cb_center_narrow(x+21,159,white,captions[card]);
         for(unsigned j=0;j<lv;j++) rect(x+4+j*7,171,6,6,15);
     }
-    /* Week shows current running rate; season shows ACTUAL accumulated
-     * money. This avoids dividing cumulative SWOS wages by 52 twice. */
+    /* Week: current wages + club overhead and staff, with travel charged
+     * only for the most recently played AWAY match. Season: booked totals.
+     * The native weekly running cost is overhead, NOT travel. */
     uint64_t gate=nonneg(_gateReceipts)+nonneg(_competitionBonuses);
     extern int32_t _weeklyPlayerWages,_weeklyRunningCosts;
+    uint64_t weeklyWages=nonneg(_weeklyPlayerWages);
+    uint64_t weeklyStaff=Expanded_FacilityWeeklyTotal()+nonneg(_weeklyRunningCosts);
+    uint64_t weeklyTravel=_expandedTravelLastWeek;
     uint64_t sponsor=_expandedFinanceSeasonView?_expandedSponsorPaidSeason:
                      (_expandedSponsorState?_expandedShirtSponsor/52u:0u);
     uint64_t tv=_expandedFinanceSeasonView?_expandedTvPaidSeason:
                 (_expandedTvState?_expandedTvRights/52u:0u);
-    uint64_t wages=_expandedFinanceSeasonView?nonneg(_playerWagesBill):
-                    nonneg(_weeklyPlayerWages);
-    uint64_t travel=_expandedFinanceSeasonView?
-                     (nonneg(_generalRunningCosts)>_expandedStaffSeason?
-                       nonneg(_generalRunningCosts)-_expandedStaffSeason:0u):
-                     nonneg(_weeklyRunningCosts);
-    uint64_t staff=_expandedFinanceSeasonView?_expandedStaffSeason:
-                    Expanded_FacilityWeeklyTotal();
+    uint64_t wages=_expandedFinanceSeasonView?nonneg(_playerWagesBill):weeklyWages;
+    uint64_t travel=_expandedFinanceSeasonView?_expandedTravelSeason:weeklyTravel;
+    uint64_t staff=_expandedFinanceSeasonView?
+                   (nonneg(_generalRunningCosts)>_expandedTravelSeason?
+                    nonneg(_generalRunningCosts)-_expandedTravelSeason:0u):weeklyStaff;
     uint64_t gateDisplay=_expandedFinanceSeasonView?gate:
-                         gate/((unsigned)_lastFinancialWeekIndex+1u);
+                         gate/(_seasonWeeksDeducted?_seasonWeeksDeducted:1u);
     uint64_t income=sponsor+tv+gateDisplay,expense=wages+travel+staff;
-    snprintf(t,sizeof t,"FINANCES / %s %02u",_expandedFinanceSeasonView?"SEASON":"WEEK",
-             (unsigned)_lastFinancialWeekIndex+1u);
+    unsigned week=Expanded_FinanceWeek(_lastFinancialWeekIndex,_careerJobOfferPhaseBias);
+    snprintf(t,sizeof t,"FINANCES / %s %02u",_expandedFinanceSeasonView?"SEAS":"WEEK",week);
     cb_text(9,189,white,t);
-    /* Full-label WEEK/SEASON switch, wider but entirely inside the finance
-     * header. It shows the destination view, not an ambiguous W/S letter. */
-    rect(143,185,64,16,42);cb_box_outline(143,185,64,16,204);
-    cb_center_narrow(175,189,gold,_expandedFinanceSeasonView?"WEEK":"SEASON");
+    /* The button fits between the header and the panel divider. */
+    rect(164,184,51,13,42);cb_box_outline(164,184,51,13,204);
+    cb_native(189,187,gold,_expandedFinanceSeasonView?"WEEK":"SEASON",0,46);
     cb_center(264,189,white,"BALANCE");
     static const char *const leftLabel[4]={"SPONSOR","TV","GATE","INCOME"};
     static const char *const rightLabel[4]={"WAGES","TRAVEL","STAFF","TOTAL"};
@@ -949,11 +1007,15 @@
     int64_t balance=_newBalance;
     cb_amount(money,sizeof money,(uint64_t)(balance<0?-balance:balance),balance<0?-1:0);
     cb_center_narrow(263,206,gold,money);
-    int64_t net=(int64_t)income-(int64_t)expense;
-    cb_compact(money,sizeof money,(uint64_t)((net<0?-net:net)*(_expandedFinanceSeasonView?1u:4u)),net<0?-1:1);
+    /* Forecast, NOT an extrapolation of an already accumulated season.
+     * Each displayed figure is the current weekly expense multiplied by
+     * 4 or 52.  A home match contributes zero to weekly travel. */
+    uint64_t projected=weeklyWages+weeklyStaff+weeklyTravel;
+    cb_compact(money,sizeof money,projected*4u,-1);
     snprintf(t,sizeof t,"MONTH %s",money); cb_center_narrow(263,225,white,t);
-    cb_compact(money,sizeof money,(uint64_t)((net<0?-net:net)*(_expandedFinanceSeasonView?1u:52u)),net<0?-1:1);
+    cb_compact(money,sizeof money,Expanded_AnnualExpenses(projected),-1);
     snprintf(t,sizeof t,"YEAR %s",money); cb_center_narrow(263,237,white,t);
+    cb_staff_popup();
     return 1;
 }
 #else
@@ -1247,7 +1309,7 @@
     for(unsigned i=28;i<=44;i++) {
         /* Native selection/drag rectangle must match the custom 8px row.
          * Its previous y was TWO rows above the live blue highlight. */
-        ui_move(i,10,Expanded_SquadRowY(i-28u),300,EXP_SQUAD_ROW_H);
+        ui_move(i,10,Expanded_SquadRowY(i-28u)-2,300,EXP_SQUAD_ROW_H);
         _menuEntries[i].background=NO_BACKGROUND;
         _menuEntries[i].content=NO_FOREGROUND;
     }
@@ -1708,40 +1770,44 @@
 static void ui_squad_tooltip(unsigned row,const PlayerFile *p) {
     if(!p) return;
     ExpandedSquadSkills s=ui_player_skills(p);
-    static const char *const names[7]={"PASSING","VELOCITY","HEADING","TACKLING",
-                                      "CONTROL","SPEED","FINISHING"};
-    unsigned values[7]={s.passing,s.velocity,s.heading,s.tackling,
-                        s.control,s.speed,s.finishing};
-    /* Compact popup: starts at the MIDPOINT of the player-name column.
-     * Position it strictly ABOVE or BELOW the focused 8px row, never on it.
-     * Bottom half moves above, upper half below. Joypad and mouse share
-     * selectedEntry, so both get exactly the same placement. */
+    static const char *const names[8]={"PASSING","VELOCITY","HEADING","TACKLING",
+                                      "CONTROL","SPEED","FINISHING","FORM"};
+    unsigned fileIndex=Expanded_SquadRowFileIndex(row);
+    unsigned condition=(uintptr_t)_squadTeamFilePtr==(uintptr_t)_careerTeam?
+                       Expanded_PlayerCondition(fileIndex):50u;
+    unsigned values[8]={s.passing,s.velocity,s.heading,s.tackling,
+                        s.control,s.speed,s.finishing,condition};
+    /* Two close columns (4+4), fully opaque right up to the outline.
+     * Both keyboard/joypad focus and mouse hover use this same geometry. */
     const int x=EXP_SQUAD_TOOLTIP_X,w=EXP_SQUAD_TOOLTIP_W,
               h=EXP_SQUAD_TOOLTIP_H;
     const int y=Expanded_SquadTooltipY(row);
-    rect(x+2,y+2,w-2,h-2,40);
+    rect(x,y,w,h,40); /* opaque border: no background visible through edges */
     cb_box_outline(x,y,w,h,204);
-    rect(x+2,y+2,w-4,12,46);
-    ui_native_ellipsis(x+4,y+3,w-8,15,(const char*)p->playerName,-1);
+    rect(x+1,y+1,w-2,11,46);
+    ui_native_ellipsis(x+4,y+2,w-8,15,(const char*)p->playerName,-1);
     char line[32];
-    for(unsigned i=0;i<7;i++) {
-        int yy=y+15+(int)i*8;
-        if(i&1) rect(x+2,yy,w-4,8,42);
-        ui_native_text(x+4,yy,57,2,names[i],-1);
+    for(unsigned i=0;i<8;i++) {
+        int col=(int)(i/4u), yy=y+14+(int)(i%4u)*9;
+        int xx=x+4+col*68;
+        if(i%4u & 1u) rect(xx-1,yy,w/2-2,9,42);
+        ui_native_text(xx,yy,53,2,names[i],-1);
         snprintf(line,sizeof line,"%u",values[i]);
-        ui_native_text(x+w-5,yy,15,15,line,1);
-    }
-    /* This is a NEW per-player 0..100 condition, initially 50, not the
-     * SWOS PlayerFile.form byte (which encodes TRIAL/RESERVE). */
-    unsigned fileIndex=Expanded_SquadRowFileIndex(row);
-    unsigned condition=(uintptr_t)_squadTeamFilePtr==(uintptr_t)_careerTeam?
-                       Expanded_PlayerCondition(fileIndex):50u;
-    ui_native_text(x+4,y+72,57,2,"FORM",-1);
-    snprintf(line,sizeof line,"%u",condition);
-    ui_native_text(x+w-5,y+72,19,15,line,1);
+        ui_native_text(xx+62,yy,20,15,line,1);
+    }
 }
 static void ui_squad_render(void) {
     cb_blit(0,0,CB_W,CB_H,ui_squad);
+    /* The approved background contains real stadium pixels above the squad
+     * table, but its narrow left/right gutters were flat navy.  Extend the
+     * EXISTING artwork into the two gutters, never the blue row fill. */
+    for(int yy=80;yy<231;yy++) {
+        int artY=68+((yy-80)/3)%11;
+        for(int xx=0;xx<4;xx++) {
+            cb_plot(xx,yy,ui_squad[artY*CB_W+xx]);
+            cb_plot(CB_W-1-xx,yy,ui_squad[artY*CB_W+CB_W-1-xx]);
+        }
+    }
     TeamFileEx *team=(TeamFileEx*)(uintptr_t)_squadTeamFilePtr;
     if(!team) return;
     char line[96];
@@ -1795,13 +1861,9 @@
         int swapping=_squadPrevSelectedPlayerIndex!=0xFFFFu &&
                      fileIndex==_squadPrevSelectedPlayerIndex;
         if(swapping) {
-            rect(10,y,300,8,72);
-            rect(10,y,3,8,10);
-            rect(307,y,3,8,10);
+            rect(10,y-2,300,8,72);
         } else if(selected==i) {
-            rect(10,y,300,8,11);
-            rect(10,y,2,8,15);
-            rect(308,y,2,8,15);
+            rect(10,y-2,300,8,11);
         }
         if(selected==i) selectedPlayer=p;
         snprintf(line,sizeof line,"%u",(unsigned)(uint8_t)p->shirtNumber);
@@ -1966,7 +2028,7 @@
                 name=(const char *)(uintptr_t)e->fg;
             ui_button(i,name);
         }
-        ui_native_text(43,246,70,15,"V0.10.20",0);
+        ui_native_text(43,246,70,15,"V0.10.21",0);
     } else if(kind==2) {
         Menu *m=(Menu *)_currentMenu;
         if(!m) return;
--- 0.10.20/sorgenti/src/save_format_json.c
+++ 0.10.21/sorgenti/src/save_format_json.c
@@ -1095,6 +1095,7 @@
 extern uint8_t _expandedSponsorState,_expandedTvState,_expandedFinanceSeasonView;
 extern uint32_t _expandedStaffSeason,_expandedSponsorPaidSeason,_expandedTvPaidSeason;
 extern uint32_t _expandedStaffLastWeek,_expandedSponsorLastWeek,_expandedTvLastWeek;
+extern uint32_t _expandedTravelLastWeek,_expandedTravelSeason;
 extern uint8_t _expandedResults[5],_expandedResultCount,_expandedResultHead;
 extern uint8_t _expandedTravelFatigue,_expandedCurrentForm;
 extern uint8_t _expandedPlayerCondition[32];
@@ -1128,6 +1129,8 @@
     FX("expanded_sponsor_paid_season", _expandedSponsorPaidSeason, FLD_U32),
     FX("expanded_tv_paid_season", _expandedTvPaidSeason, FLD_U32),
     FX("expanded_staff_last_week", _expandedStaffLastWeek, FLD_U32),
+    FX("expanded_travel_last_week", _expandedTravelLastWeek, FLD_U32),
+    FX("expanded_travel_season", _expandedTravelSeason, FLD_U32),
     FX("expanded_sponsor_last_week", _expandedSponsorLastWeek, FLD_U32),
     FX("expanded_tv_last_week", _expandedTvLastWeek, FLD_U32),
     FA("expanded_recent_results", _expandedResults, 5, FLD_U8),
--- 0.10.20/sorgenti/src/platform_sdl.c
+++ 0.10.21/sorgenti/src/platform_sdl.c
@@ -146,6 +146,27 @@
     x-=_menu_draw_x_offset;
     if(x<7 || x>312 || y<90 || y>=226) return -1;
     return (y-90)/8;
+}
+
+/* Club Business STAFF row: map the real mouse position through the same
+ * SDL logical-size letterboxing as squad hover. No fake menu entry is added;
+ * the existing finance toggle and facility callbacks remain unchanged. */
+int Platform_BusinessPointerOverStaff(void) {
+    if(!s_window || !s_renderer) return 0;
+    int mx,my,ww=0,wh=0,lw=0,lh=0;
+    SDL_GetMouseState(&mx,&my);
+    SDL_GetWindowSize(s_window,&ww,&wh);
+    SDL_RenderGetLogicalSize(s_renderer,&lw,&lh);
+    if(ww<=0 || wh<=0) return 0;
+    int hasLogical=lw>0 && lh>0;
+    if(!hasLogical) {lw=_screen_width;lh=_screen_height;}
+    if(lw<=0 || lh<=0) return 0;
+    double sx=(double)ww/lw,sy=(double)wh/lh;
+    double scale=sx<sy?sx:sy;
+    int x=hasLogical?(int)((mx-(ww-lw*scale)*0.5)/scale):(int)(mx/sx);
+    int y=hasLogical?(int)((my-(wh-lh*scale)*0.5)/scale):(int)(my/sy);
+    x-=_menu_draw_x_offset;
+    return x>=111 && x<=211 && y>=222 && y<=235;
 }
 
 /* A small RING of streaming textures. We upload into a DIFFERENT one each frame so
--- 0.10.20/sorgenti/src/menu_system.c
+++ 0.10.21/sorgenti/src/menu_system.c
@@ -1870,15 +1870,27 @@
     /* Mouse hover uses the SAME selectedEntry as the keyboard and joypad.
      * Moving the D-pad immediately updates the tooltip without FIRE. */
     {
-        extern uint8_t _squadMenu[];
+        extern uint8_t _squadMenu[],_expandedClubOverviewMenu[];
         extern int Platform_SquadPointerMovedRow(void);
+        extern int Platform_BusinessPointerOverStaff(void);
         extern void Platform_SquadMouseTracking(int);
+        extern uint8_t _expandedBusinessStaffHover;
         static int mouseTracking=-1;
         int inSquad=_currentMenuPtr==(uintptr_t)_squadMenu;
-        if(inSquad!=mouseTracking) {
-            Platform_SquadMouseTracking(inSquad);
-            mouseTracking=inSquad;
-        }
+        int inBusiness=_currentMenuPtr==(uintptr_t)_expandedClubOverviewMenu;
+        if((inSquad||inBusiness)!=mouseTracking) {
+            Platform_SquadMouseTracking(inSquad||inBusiness);
+            mouseTracking=inSquad||inBusiness;
+        }
+        /* Hover is cosmetic: never changes selectedEntry, callbacks or
+         * controller focus. A redraw occurs only on enter/leave. */
+        if(inBusiness) {
+            int hover=Platform_BusinessPointerOverStaff();
+            if(hover!=_expandedBusinessStaffHover) {
+                _expandedBusinessStaffHover=(uint8_t)hover;
+                DrawMenu();
+            }
+        } else _expandedBusinessStaffHover=0;
         if(inSquad && !_fire && !_shortFire) {
             int row=Platform_SquadPointerMovedRow();
             if(row>=0 && row<17) {
--- 0.10.20/sorgenti/src/build_info.c
+++ 0.10.21/sorgenti/src/build_info.c
@@ -87,7 +87,7 @@
 
 const char *Build_Version(void)
 {
-    return "SWOS Expanded 0.10.20 - " PROJECT_NAME " " SWOS_BUILD_ID
+    return "SWOS Expanded 0.10.21 - " PROJECT_NAME " " SWOS_BUILD_ID
            " (" SWOS_BUILD_DATE ", " SWOS_PLATFORM SWOS_BUILD_RULES ")";
 }
 
--- 0.10.20/sorgenti/include/expanded_facility_rules.h
+++ 0.10.21/sorgenti/include/expanded_facility_rules.h
@@ -22,6 +22,14 @@
     if(level<1) level=1;
     if(level>5) level=5;
     return level*10u;
+}
+/* Ticket, fuel and hotel expenses per AWAY fixture; home is zero.
+ * Higher comfort means higher cost but less fatigue. */
+static inline unsigned Expanded_TravelMatchCost(unsigned level) {
+    static const uint16_t price[5]={100,180,320,560,950};
+    if(level<1) level=1;
+    if(level>5) level=5;
+    return price[level-1];
 }
 static inline unsigned Expanded_AwayFatigue(unsigned level) {
     static const uint8_t cost[5]={14,11,8,5,2};
--- 0.10.20/sorgenti/include/expanded_squad_layout.h
+++ 0.10.21/sorgenti/include/expanded_squad_layout.h
@@ -2,8 +2,8 @@
 #define EXPANDED_SQUAD_LAYOUT_H
 /* All coordinates are original SWOS 320x256 indexed framebuffer pixels. */
 enum { EXP_SQUAD_ROW_TOP=90, EXP_SQUAD_ROW_H=8,
-       EXP_SQUAD_TOOLTIP_X=80, EXP_SQUAD_TOOLTIP_W=77,
-       EXP_SQUAD_TOOLTIP_H=83 };
+       EXP_SQUAD_TOOLTIP_X=74, EXP_SQUAD_TOOLTIP_W=142,
+       EXP_SQUAD_TOOLTIP_H=52 };
 static inline int Expanded_SquadRowY(unsigned row) {
     return EXP_SQUAD_ROW_TOP+(int)row*EXP_SQUAD_ROW_H;
 }

```

---

## PATCH_0121_TO_0122.diff

```text
--- 0.10.21/sorgenti/src/expanded_economy.c
+++ 0.10.22/sorgenti/src/expanded_economy.c
@@ -615,22 +615,42 @@
     /* The legacy generic upgrade row stays in the packed layout to keep
      * ordinals stable, but is not visible or selectable. */
     _menuEntries[0].isInvisible=1; /* artwork replaces stock title */
-    _menuEntries[9].isInvisible=0; /* compact WEEK/SEASON finance switch */
-    _menuEntries[9].x=164;_menuEntries[9].y=184;
-    _menuEntries[9].dwWidth=51;_menuEntries[9].dwHeight=13;
+    /* Finance switch is entirely INSIDE the FINANCES header (right edge 201,
+     * divider begins at 207).  It has a real fire callback, unlike STAFF. */
+    _menuEntries[9].isInvisible=0;
+    _menuEntries[9].x=161;_menuEntries[9].y=185;
+    _menuEntries[9].dwWidth=40;_menuEntries[9].dwHeight=11;
     _menuEntries[9].background=NO_BACKGROUND;
     _menuEntries[9].content=NO_FOREGROUND;
-    _menuEntries[9].leftEntry=10;
-    _menuEntries[9].rightEntry=1;
-    _menuEntries[9].upEntry=4;
-    _menuEntries[9].downEntry=10;
-    _menuEntries[10].rightEntry=9;
+    _menuEntries[9].leftEntry=5;
+    _menuEntries[9].rightEntry=8;
+    _menuEntries[9].upEntry=3;
+    _menuEntries[9].downEntry=11;
+    /* STAFF is a FOCUS-ONLY menu entry: no onSelect, no cost changes, no
+     * FIRE required.  Moving the D-pad onto it opens the same tooltip as
+     * mouse hover; moving away closes it. */
+    _menuEntries[11].isInvisible=0;
+    _menuEntries[11].isDisabled=0;
+    _menuEntries[11].x=111;_menuEntries[11].y=222;
+    _menuEntries[11].dwWidth=94;_menuEntries[11].dwHeight=12;
+    _menuEntries[11].background=NO_BACKGROUND;
+    _menuEntries[11].content=NO_FOREGROUND;
+    _menuEntries[11].onSelect=0;
+    _menuEntries[11].leftEntry=9;
+    _menuEntries[11].rightEntry=9;
+    _menuEntries[11].upEntry=9;
+    _menuEntries[11].downEntry=10;
+    /* All central facility cards go DOWN to the finance toggle, then DOWN
+     * again to the STAFF tooltip.  No invisible dead-end in this path. */
     for (unsigned i=1; i<=8; ++i) {
         _menuEntries[i].background=NO_BACKGROUND;
         _menuEntries[i].content=NO_FOREGROUND;
-    }
-    /* BACK must also redraw the custom artwork when focus moves away.
-     * Otherwise CallDrawMenuItem would paint a stock SWOS box over it. */
+        _menuEntries[i].downEntry=9;
+    }
+    /* BACK stays available by LEFT from the first card, and after STAFF. */
+    _menuEntries[10].upEntry=11;
+    _menuEntries[10].downEntry=5;
+    _menuEntries[10].rightEntry=5;
     _menuEntries[10].background=NO_BACKGROUND;
     _menuEntries[10].content=NO_FOREGROUND;
 }
@@ -865,21 +885,29 @@
     rect(x,y,w,1,color);rect(x,y+h-1,w,1,color);
     rect(x,y,1,h,color);rect(x+w-1,y,1,h,color);
 }
+static int cb_staff_hover_active(void) {
+    /* Mouse and controller share the tooltip; the pointer is NOT allowed to
+     * steal controller focus just because it was left over the STAFF row. */
+    Menu *menu=(Menu *)_currentMenu;
+    return _expandedBusinessStaffHover ||
+           (menu && menu->selectedEntry && menu->selectedEntry->ordinal==11);
+}
 static void cb_staff_popup(void) {
-    if(_expandedBusinessStaffHover) {
+    if(cb_staff_hover_active()) {
         extern int32_t _weeklyRunningCosts;
         const uint8_t white=2,gold=15;
         char money[48];
         uint64_t weeklyStaff=Expanded_FacilityWeeklyTotal()+nonneg(_weeklyRunningCosts);
-        /* A solid, compact 2-column cost breakdown.  The 'OTHER' item
-         * includes SWOS general running costs and the base 45/week staff;
-         * the other 8 entries are the real facility wage table. */
-        const int px=81,py=123,pw=154,ph=59;
+        /* Fully spelled-out labels, still only 5 rows per column. OTHER
+         * remains native overhead + base 45. BUS STAFF is personnel at the
+         * transport facility; the away-trip charge belongs ONLY to the bus
+         * facility's detail and the existing AWAY expense ledger. */
+        const int px=14,py=112,pw=292,ph=70;
         rect(px,py,pw,ph,40);cb_box_outline(px,py,pw,ph,204);
-        rect(px+1,py+1,pw-2,10,46);
+        rect(px+1,py+1,pw-2,11,46);
         cb_native(px+pw/2,py+2,15,"STAFF / WEEK",0,pw-8);
-        static const char *const sn[10]={"OTHER","STAD","TRAIN","TRANS","OFFICE",
-                                         "FIT","SCOUT","MED","YOUTH","TOTAL"};
+        static const char *const sn[10]={"OTHER","STADIUM","TRAINING","BUS STAFF","OFFICES",
+                                         "FITNESS","SCOUTING","MEDICAL","YOUTH","TOTAL"};
         uint64_t amounts[10]={nonneg(_weeklyRunningCosts)+45u,
             Expanded_StaffWeekly(0,_expandedFacilityLevels[0]),
             Expanded_StaffWeekly(1,_expandedFacilityLevels[1]),
@@ -890,11 +918,11 @@
             Expanded_StaffWeekly(6,_expandedFacilityLevels[6]),
             Expanded_StaffWeekly(7,_expandedFacilityLevels[7]),weeklyStaff};
         for(unsigned i=0;i<10;i++) {
-            int col=(int)(i/5u),yy=py+12+(int)(i%5u)*9;
-            int xx=px+4+col*76;
-            cb_native(xx,yy,i==9?15:white,sn[i],-1,49);
+            int col=(int)(i/5u),yy=py+14+(int)(i%5u)*10;
+            int xx=px+5+col*144;
+            cb_native(xx,yy,i==9?15:white,sn[i],-1,88);
             cb_compact(money,sizeof money,amounts[i],0);
-            cb_native(xx+72,yy,i==9?15:gold,money,1,26);
+            cb_native(xx+134,yy,i==9?15:gold,money,1,38);
         }
     }
 }
@@ -904,7 +932,8 @@
     if (!menu || !menu->selectedEntry) {cb_staff_popup();return 1;}
     unsigned ordinal=menu->selectedEntry->ordinal;
     int x,y,w,h;
-    if(ordinal==9) {x=164;y=184;w=51;h=13;}
+    if(ordinal==9) {x=161;y=185;w=40;h=11;}
+    else if(ordinal==11) {x=111;y=222;w=94;h=12;}
     else if(ordinal==1) {x=243;y=108;w=70;h=18;}
     else if(ordinal==10) {x=5;y=3;w=47;h=17;}
     else if(ordinal>=2 && ordinal<=8) {
@@ -985,14 +1014,14 @@
                          gate/(_seasonWeeksDeducted?_seasonWeeksDeducted:1u);
     uint64_t income=sponsor+tv+gateDisplay,expense=wages+travel+staff;
     unsigned week=Expanded_FinanceWeek(_lastFinancialWeekIndex,_careerJobOfferPhaseBias);
-    snprintf(t,sizeof t,"FINANCES / %s %02u",_expandedFinanceSeasonView?"SEAS":"WEEK",week);
+    snprintf(t,sizeof t,"FINANCES / %s %02u",_expandedFinanceSeasonView?"YEAR":"WEEK",week);
     cb_text(9,189,white,t);
-    /* The button fits between the header and the panel divider. */
-    rect(164,184,51,13,42);cb_box_outline(164,184,51,13,204);
-    cb_native(189,187,gold,_expandedFinanceSeasonView?"WEEK":"SEASON",0,46);
+    /* 40x11: the complete outline stays inside the FINANCES title box. */
+    rect(161,185,40,11,42);cb_box_outline(161,185,40,11,204);
+    cb_native(181,187,gold,_expandedFinanceSeasonView?"WEEK":"YEAR",0,36);
     cb_center(264,189,white,"BALANCE");
     static const char *const leftLabel[4]={"SPONSOR","TV","GATE","INCOME"};
-    static const char *const rightLabel[4]={"WAGES","TRAVEL","STAFF","TOTAL"};
+    static const char *const rightLabel[4]={"WAGES","AWAY","STAFF","TOTAL"};
     uint64_t lv[4]={sponsor,tv,gateDisplay,income};
     uint64_t rv[4]={wages,travel,staff,expense};
     for(unsigned i=0;i<4;i++) {
@@ -1059,7 +1088,9 @@
     snprintf(t,sizeof t,"SPONSOR %c%lu",c,(unsigned long)(sponsors/12)); label(84,189,7,t);
     snprintf(t,sizeof t,"WAGES %c%lu",c,(unsigned long)(nonneg(_playerWagesBill)/12)); label(237,189,7,t);
     snprintf(t,sizeof t,"TV %c%lu",c,(unsigned long)(_expandedTvRights/12)); label(84,200,7,t);
-    snprintf(t,sizeof t,"TRAVEL %c%lu",c,(unsigned long)(nonneg(_generalRunningCosts)/12)); label(237,200,7,t);
+    { extern int32_t _weeklyRunningCosts;
+      snprintf(t,sizeof t,"STAFF %c%lu",c,(unsigned long)(Expanded_FacilityWeeklyTotal()+nonneg(_weeklyRunningCosts)));
+      label(237,200,7,t); }
     snprintf(t,sizeof t,"GATE/PRIZE %c%lu",c,(unsigned long)((nonneg(_gateReceipts)+nonneg(_competitionBonuses))/12));label(84,211,7,t);
     snprintf(t,sizeof t,"MONTHLY: +%c%lu / -%c%lu",c,(unsigned long)(income/12),c,(unsigned long)(expense/12));
     label(160,222,7,t);
@@ -1307,27 +1338,49 @@
      * NO_BACKGROUND on row entries makes joypad movement request a full
      * redraw, keeping the live player tooltip in sync with focus. */
     for(unsigned i=28;i<=44;i++) {
-        /* Native selection/drag rectangle must match the custom 8px row.
-         * Its previous y was TWO rows above the live blue highlight. */
+        /* Native selection/drag rectangle matches the custom 8px row. */
         ui_move(i,10,Expanded_SquadRowY(i-28u)-2,300,EXP_SQUAD_ROW_H);
+        MenuEntry *e=&_menuEntries[i];
+        e->background=NO_BACKGROUND;
+        e->content=NO_FOREGROUND;
+        e->leftEntry=-1;
+        e->rightEntry=(int8_t)(_isSquadViewMode?11:8);
+        e->upEntry=(int8_t)(i==28?(_isSquadViewMode?11:9):i-1);
+        e->downEntry=(int8_t)(i==44?(_isSquadViewMode?11:9):i+1);
+        e->directionLeft=e->directionRight=e->directionUp=e->directionDown=0;
+        e->skipLeftEntry=e->skipRightEntry=e->skipUpEntry=e->skipDownEntry=-1;
+    }
+    for(unsigned i=6;i<=11;i++) {
         _menuEntries[i].background=NO_BACKGROUND;
         _menuEntries[i].content=NO_FOREGROUND;
     }
-    for(unsigned i=6;i<=10;i++) {
-        _menuEntries[i].background=NO_BACKGROUND;
-        _menuEntries[i].content=NO_FOREGROUND;
-    }
-    /* Only move the existing interactive buttons, no new callbacks. */
+    /* Preserve all original callbacks. The normal bottom row is
+     * GOALS <-> TRAINING <-> EXIT.  In goals view, BACK <-> EXIT. */
     ui_move(8,146,236,52,18);  /* GOALS */
     ui_move(6,201,236,59,18);  /* TRAINING */
-    ui_move(9,263,236,52,18);  /* EXIT (club) */
-    ui_move(10,263,236,52,18); /* EXIT (national) */
-    ui_move(11,146,236,52,18); /* original GOALS -> BACK */
-    if(_isSquadViewMode) {
-        _menuEntries[11].isInvisible=0;
-        _menuEntries[11].background=NO_BACKGROUND;
-        _menuEntries[11].content=NO_FOREGROUND;
-    }
+    ui_move(9,263,236,52,18);  /* EXIT (edit) */
+    ui_move(10,263,236,52,18); /* EXIT (goals view) */
+    ui_move(11,146,236,52,18); /* GOALS -> BACK */
+    _menuEntries[10].isInvisible=_isSquadViewMode?0:1;
+    _menuEntries[11].isInvisible=_isSquadViewMode?0:1;
+    static const unsigned buttons[]={8,6,9,10,11};
+    for(unsigned j=0;j<sizeof buttons/sizeof buttons[0];j++) {
+        MenuEntry *e=&_menuEntries[buttons[j]];
+        e->directionLeft=e->directionRight=e->directionUp=e->directionDown=0;
+        e->skipLeftEntry=e->skipRightEntry=e->skipUpEntry=e->skipDownEntry=-1;
+        e->upEntry=44;
+        e->downEntry=28; /* one DOWN from EXIT returns to the roster */
+    }
+    _menuEntries[8].leftEntry=44;
+    _menuEntries[8].rightEntry=6;
+    _menuEntries[6].leftEntry=8;
+    _menuEntries[6].rightEntry=9;
+    _menuEntries[9].leftEntry=6;
+    _menuEntries[9].rightEntry=8;
+    _menuEntries[11].leftEntry=44;
+    _menuEntries[11].rightEntry=10;
+    _menuEntries[10].leftEntry=11;
+    _menuEntries[10].rightEntry=11;
 }
 static void ui_sacked_prepare(void) {
     Menu *m=(Menu *)_currentMenu;
@@ -1820,6 +1873,7 @@
         extern void *_countriesTable[];
         if(_countriesTable[country]) {
             snprintf(line,sizeof line,"(%.24s)",(const char*)_countriesTable[country]+1);
+            ui_native_text(161,70,140,0,line,0); /* dark shadow on stadium */
             ui_native_text(160,69,140,2,line,0);
         }
     }
@@ -1895,9 +1949,11 @@
         FormatPlayerValueOrStatusText((uint8_t*)p-sizeof(TeamFile),value);
         ui_native_text(311,y,55,2,(const char*)value,1);
     }
-    /* Team form: real last five results, adjusted by travel and fitness. */
+    /* Form is legible in its OWN small plaque, beside the team name.
+     * Previously it was unframed over the country caption at y=69. */
+    rect(6,50,80,17,40);cb_box_outline(6,50,80,17,204);
     snprintf(line,sizeof line,"FORM %u%%",(unsigned)_expandedCurrentForm);
-    ui_native_text(53,69,90,15,line,0);
+    ui_native_text(46,55,72,15,line,0);
     if(team->team.coachName[0]) {
         snprintf(line,sizeof line,"COACH: %.22s",(const char*)team->team.coachName);
         ui_native_ellipsis(9,242,134,2,line,-1);
--- 0.10.21/sorgenti/src/menu_data.c
+++ 0.10.22/sorgenti/src/menu_data.c
@@ -10780,6 +10780,13 @@
     MenuCmd_OnSelect onSelect;
     uint16_t end;
 } CB_MenuAction;
+typedef struct {
+    MenuCoords16 rc;
+    MenuCmd_Nav nav;
+    MenuCmd_Colors colors;
+    MenuCmd_TextPtr text;
+    uint16_t end;
+} CB_MenuFocusOnly;
 struct {
     MenuHeader hdr;
     MenuMark wideCoords;
@@ -10787,6 +10794,7 @@
     CB_MenuAction facilities[8];
     CB_MenuAction upgrade;
     CB_MenuAction back;
+    CB_MenuFocusOnly staffHover;
     MenuMark end;
 } _expandedClubOverviewMenu = {
     /* Real SWOS menu hitboxes; all pixel artwork is painted AFTER the
@@ -10833,10 +10841,14 @@
     { { 10, 148, 105, 16 }, { MENU_CMD_NAV, -1, 10, 5, 5 },
       { MENU_CMD_COLORS, 0xB }, { MENU_CMD_TEXT_PTR, 0, s_clubOverviewUpgrade },
       { MENU_CMD_ONSELECT_FIRE, Expanded_ToggleFinanceView }, MENU_CMD_END },
-    /* BACK at bottom left, never covered by any other drawing. */
-    { { -19, -33, 47, 17 }, { MENU_CMD_NAV, -1, 5, 5, 1 },
+    /* BACK at top left, never covered by any other drawing. */
+    { { -19, -33, 47, 17 }, { MENU_CMD_NAV, -1, 5, 11, 5 },
       { MENU_CMD_COLORS, 0xC }, { MENU_CMD_TEXT_PTR, 0, s_clubOverviewBack },
       { MENU_CMD_ONSELECT_FIRE, SetExitMenuFlag }, MENU_CMD_END },
+    /* 11: STAFF tooltip focus target. No ONSELECT command on purpose:
+     * moving focus here reveals the breakdown; FIRE changes nothing. */
+    { { 87, 186, 94, 12 }, { MENU_CMD_NAV, 9, 9, 9, 10 },
+      { MENU_CMD_COLORS, 0x8 }, { MENU_CMD_TEXT_PTR, 0, s_emptyFacility }, MENU_CMD_END },
     MENU_MARK_END_MENU
 };
 #pragma pack(pop)
--- 0.10.21/sorgenti/src/build_info.c
+++ 0.10.22/sorgenti/src/build_info.c
@@ -87,7 +87,7 @@
 
 const char *Build_Version(void)
 {
-    return "SWOS Expanded 0.10.21 - " PROJECT_NAME " " SWOS_BUILD_ID
+    return "SWOS Expanded 0.10.22 - " PROJECT_NAME " " SWOS_BUILD_ID
            " (" SWOS_BUILD_DATE ", " SWOS_PLATFORM SWOS_BUILD_RULES ")";
 }
 
--- 0.10.21/AVVIA_SWOS.bat
+++ 0.10.22/AVVIA_SWOS.bat
@@ -1,7 +1,7 @@
 @echo off
 setlocal EnableExtensions
 cd /d "%~dp0"
-title SWOS Expanded 0.10.21 - Avvio
+title SWOS Expanded 0.10.22 - Avvio
 
 REM La prima volta compila i sorgenti corretti; dopo avvia direttamente il gioco.
 call "%~dp0PREPARA_SWOS.bat"
@@ -27,7 +27,7 @@
 set "SWOS_TACTICS_JSON=0"
 set "SWOS_MENU_NAV_LOG=1"
 echo.
-echo === SWOS Expanded 0.10.21 - avvio ===
+echo === SWOS Expanded 0.10.22 - avvio ===
 "%~dp0SWOS.exe"
 set "RC=%ERRORLEVEL%"
 echo.
@@ -39,6 +39,6 @@
 :errore
 echo.
 echo Il gioco non e stato avviato.
-echo Se la compilazione e fallita, invia COMPILAZIONE_0121.log.
+echo Se la compilazione e fallita, invia COMPILAZIONE_0122.log.
 pause
 exit /b 1
--- 0.10.21/PREPARA_SWOS.bat
+++ 0.10.22/PREPARA_SWOS.bat
@@ -1,11 +1,11 @@
 @echo off
 setlocal EnableExtensions
 cd /d "%~dp0"
-title SWOS Expanded 0.10.21 - Preparazione
+title SWOS Expanded 0.10.22 - Preparazione
 
 REM Un eseguibile 0.10 precedente non e considerato valido.
-if exist ".SWOS_0121_COMPILATO" if exist "SWOS.exe" (
-  echo SWOS 0.10.21 pronto.
+if exist ".SWOS_0122_COMPILATO" if exist "SWOS.exe" (
+  echo SWOS 0.10.22 pronto.
   exit /b 0
 )
 
@@ -48,14 +48,14 @@
 "%MSYS_ROOT%\usr\bin\bash.exe" -lc "bash -s" < "%~dp0BUILD_SWOS.sh"
 if errorlevel 1 (
   echo.
-  echo COMPILAZIONE FALLITA: invia COMPILAZIONE_0121.log.
+  echo COMPILAZIONE FALLITA: invia COMPILAZIONE_0122.log.
   exit /b 1
 )
 if not exist "SWOS.exe" (
   echo ERRORE: compilazione terminata senza SWOS.exe.
   exit /b 1
 )
-> ".SWOS_0121_COMPILATO" echo SWOS Expanded 0.10.21 - compilazione completata
+> ".SWOS_0122_COMPILATO" echo SWOS Expanded 0.10.22 - compilazione completata
 exit /b 0
 
 :cerca_msys
--- 0.10.21/BUILD_SWOS.sh
+++ 0.10.22/BUILD_SWOS.sh
@@ -23,9 +23,9 @@
     exit 1
   fi
 done
-exec > >(tee "$game/COMPILAZIONE_0121.log") 2>&1
+exec > >(tee "$game/COMPILAZIONE_0122.log") 2>&1
 
-echo '==== SWOS Expanded 0.10.21 - compilazione Windows x64 ===='
+echo '==== SWOS Expanded 0.10.22 - compilazione Windows x64 ===='
 echo "Sorgenti: $src"
 echo "Gioco:    $game"
 echo 'Verifica delle dipendenze MSYS2...'
@@ -41,12 +41,12 @@
   -Isrc -Iinclude $(pkg-config --cflags sdl2) \
   src/menu_data.c src/menu_system.c src/menus.c src/expanded_career.c \
   src/expanded_economy.c src/career_init.c src/career_status.c src/career_teams.c src/transfer_market.c src/chairman_memo.c src/save_format_json.c src/input_text.c src/blitter.c src/build_info.c
-echo 'Compilo il gioco 0.10.21...'
+echo 'Compilo il gioco 0.10.22...'
 make -j4 NETPLAY=0
 if [[ ! -s bin/SWOS.exe ]]; then
   echo 'ERRORE: il compilatore non ha prodotto bin/SWOS.exe.'
   exit 1
 fi
-cp -f bin/SWOS.exe "$game/SWOS_0121_new.exe"
-mv -f "$game/SWOS_0121_new.exe" "$game/SWOS.exe"
+cp -f bin/SWOS.exe "$game/SWOS_0122_new.exe"
+mv -f "$game/SWOS_0122_new.exe" "$game/SWOS.exe"
 echo 'Compilazione completata: SWOS.exe aggiornato.'
--- 0.10.21/LEGGIMI_PRIMA.txt
+++ 0.10.22/LEGGIMI_PRIMA.txt
@@ -1,5 +1,8 @@
-SWOS Expanded 0.10.14 - Offerta e Trattative
+SWOS Expanded 0.10.22 — BUSINESS e SQUAD / JOYPAD
 
-Estrai in una nuova cartella, poi avvia AVVIA_SWOS.bat.
-La prima volta compila SWOS.exe con MSYS2 (come nelle versioni precedenti).
-Per le novita e i limiti di questa build leggi LEGGIMI_0114_IT.txt.
+Estrai lo ZIP in una nuova cartella, poi fai doppio clic su AVVIA_SWOS.bat.
+Se MSYS2 e' gia' installato, il BAT compila automaticamente il gioco
+la prima volta e poi lo avvia. Non occorre reinstallare il compilatore.
+
+Leggi LEGGIMI_0122_IT.txt per le modifiche, i comandi del joypad e i test.
+Se non compila, inviami COMPILAZIONE_0122.log.

# Asset binario: data/expanded/expanded_squad_bg.raw
# Aggiornati 1410 pixel indicizzati nella striscia y=67..76 sotto il paese.

```

---

## PATCH_0122_TO_0123.diff

```text
--- a/sorgenti/src/expanded_economy.c
+++ b/sorgenti/src/expanded_economy.c
@@ -615,11 +615,12 @@
     /* The legacy generic upgrade row stays in the packed layout to keep
      * ordinals stable, but is not visible or selectable. */
     _menuEntries[0].isInvisible=1; /* artwork replaces stock title */
-    /* Finance switch is entirely INSIDE the FINANCES header (right edge 201,
-     * divider begins at 207).  It has a real fire callback, unlike STAFF. */
+    /* 0.10.23: compact toggle INSIDE the finance title strip, clear of
+     * its top border (y=185) and bottom divider (y=199). The joypad
+     * hitbox exactly covers the visible button. */
     _menuEntries[9].isInvisible=0;
-    _menuEntries[9].x=161;_menuEntries[9].y=185;
-    _menuEntries[9].dwWidth=40;_menuEntries[9].dwHeight=11;
+    _menuEntries[9].x=168;_menuEntries[9].y=187;
+    _menuEntries[9].dwWidth=33;_menuEntries[9].dwHeight=12;
     _menuEntries[9].background=NO_BACKGROUND;
     _menuEntries[9].content=NO_FOREGROUND;
     _menuEntries[9].leftEntry=5;
@@ -837,6 +838,18 @@
     else if(align>0) DrawMenuTextRightAligned((uint16_t)x,(uint16_t)y,color,(uint8_t *)text,font);
     else DrawMenuTextCentered((uint16_t)x,(uint16_t)y,color,(uint8_t *)text,font);
 }
+/* Original 5x8 bitmap: a readable compact font for the tiny YEAR/WEEK
+ * toggle, independent of the wider native SWOS sprite font. */
+static void cb_tiny_label(int centerX,int y,uint8_t color,const char *s) {
+    if(!s) return;
+    int len=(int)strlen(s),x=centerX-(len*6-1)/2;
+    for(int i=0;i<len;i++) {
+        const uint8_t *glyph=cb_glyph((unsigned char)s[i]);
+        for(int row=0;row<8;row++)
+            for(int bit=0;bit<5;bit++)
+                if(glyph[row] & (1u<<(4-bit))) cb_plot(x+i*6+bit,y+row,color);
+    }
+}
 static void cb_text_step(int x,int y,uint8_t color,const char *str,int step) {
     (void)step;
     cb_native(x,y,color,str,-1,CB_W-x-2);
@@ -898,14 +911,8 @@
         const uint8_t white=2,gold=15;
         char money[48];
         uint64_t weeklyStaff=Expanded_FacilityWeeklyTotal()+nonneg(_weeklyRunningCosts);
-        /* Fully spelled-out labels, still only 5 rows per column. OTHER
-         * remains native overhead + base 45. BUS STAFF is personnel at the
-         * transport facility; the away-trip charge belongs ONLY to the bus
-         * facility's detail and the existing AWAY expense ledger. */
-        const int px=14,py=112,pw=292,ph=70;
-        rect(px,py,pw,ph,40);cb_box_outline(px,py,pw,ph,204);
-        rect(px+1,py+1,pw-2,11,46);
-        cb_native(px+pw/2,py+2,15,"STAFF / WEEK",0,pw-8);
+        /* Full labels, but no enormous blank gap before their amounts.
+         * Columns size themselves from the real sprite-font widths. */
         static const char *const sn[10]={"OTHER","STADIUM","TRAINING","BUS STAFF","OFFICES",
                                          "FITNESS","SCOUTING","MEDICAL","YOUTH","TOTAL"};
         uint64_t amounts[10]={nonneg(_weeklyRunningCosts)+45u,
@@ -917,12 +924,27 @@
             Expanded_StaffWeekly(5,_expandedFacilityLevels[5]),
             Expanded_StaffWeekly(6,_expandedFacilityLevels[6]),
             Expanded_StaffWeekly(7,_expandedFacilityLevels[7]),weeklyStaff};
+        int colWidth[2]={0,0};
+        for(unsigned i=0;i<10;i++) {
+            cb_compact(money,sizeof money,amounts[i],0);
+            int needed=cb_text_width(sn[i],5)+3+cb_text_width(money,5);
+            int col=(int)(i/5u);
+            if(needed>colWidth[col]) colWidth[col]=needed;
+        }
+        const int pad=6,between=8,py=116,ph=67;
+        const int pw=2*pad+colWidth[0]+between+colWidth[1];
+        const int px=(CB_W-pw)/2;
+        rect(px,py,pw,ph,40);cb_box_outline(px,py,pw,ph,204);
+        rect(px+1,py+1,pw-2,11,46);
+        cb_native(px+pw/2,py+2,15,"STAFF / WEEK",0,pw-8);
         for(unsigned i=0;i<10;i++) {
-            int col=(int)(i/5u),yy=py+14+(int)(i%5u)*10;
-            int xx=px+5+col*144;
-            cb_native(xx,yy,i==9?15:white,sn[i],-1,88);
+            int col=(int)(i/5u),yy=py+13+(int)(i%5u)*10;
+            int xx=px+pad+(col?colWidth[0]+between:0);
+            cb_native(xx,yy,i==9?15:white,sn[i],-1,colWidth[col]);
             cb_compact(money,sizeof money,amounts[i],0);
-            cb_native(xx+134,yy,i==9?15:gold,money,1,38);
+            /* Just three pixels between the label and its own number. */
+            cb_native(xx+cb_text_width(sn[i],5)+3,yy,
+                      i==9?15:gold,money,-1,colWidth[col]);
         }
     }
 }
@@ -932,7 +954,7 @@
     if (!menu || !menu->selectedEntry) {cb_staff_popup();return 1;}
     unsigned ordinal=menu->selectedEntry->ordinal;
     int x,y,w,h;
-    if(ordinal==9) {x=161;y=185;w=40;h=11;}
+    if(ordinal==9) {x=168;y=187;w=33;h=12;}
     else if(ordinal==11) {x=111;y=222;w=94;h=12;}
     else if(ordinal==1) {x=243;y=108;w=70;h=18;}
     else if(ordinal==10) {x=5;y=3;w=47;h=17;}
@@ -1016,12 +1038,13 @@
     unsigned week=Expanded_FinanceWeek(_lastFinancialWeekIndex,_careerJobOfferPhaseBias);
     snprintf(t,sizeof t,"FINANCES / %s %02u",_expandedFinanceSeasonView?"YEAR":"WEEK",week);
     cb_text(9,189,white,t);
-    /* 40x11: the complete outline stays inside the FINANCES title box. */
-    rect(161,185,40,11,42);cb_box_outline(161,185,40,11,204);
-    cb_native(181,187,gold,_expandedFinanceSeasonView?"WEEK":"YEAR",0,36);
+    /* 33x12, 2px below the frame top and above the divider. Even when
+     * focused the button never overlaps the title strip's outer border. */
+    rect(168,187,33,12,42);cb_box_outline(168,187,33,12,204);
+    cb_tiny_label(184,189,gold,_expandedFinanceSeasonView?"WEEK":"YEAR");
     cb_center(264,189,white,"BALANCE");
     static const char *const leftLabel[4]={"SPONSOR","TV","GATE","INCOME"};
-    static const char *const rightLabel[4]={"WAGES","AWAY","STAFF","TOTAL"};
+    static const char *const rightLabel[4]={"WAGES","TRAVEL","STAFF","TOTAL"};
     uint64_t lv[4]={sponsor,tv,gateDisplay,income};
     uint64_t rv[4]={wages,travel,staff,expense};
     for(unsigned i=0;i<4;i++) {
@@ -1291,8 +1314,8 @@
      * renderer handles all rows after CareerGameListCommon has filled them.
      * The stock scroll callbacks on entries 16/17 remain unchanged. */
     if(_currentMenuPtr==(uintptr_t)_contestMenu) {
-        ui_move(16,20,49,14,14);
-        ui_move(17,20,162,14,14);
+        ui_move(16,12,49,14,14);
+        ui_move(17,12,162,14,14);
         for(unsigned i=16;i<=17;i++) {
             _menuEntries[i].background=NO_BACKGROUND;
             _menuEntries[i].content=NO_FOREGROUND;
@@ -1579,9 +1602,9 @@
     cb_box_outline(5,39,310,bottom-39,7);
     rect(7,41,306,1,2);
     rect(7,(unsigned)(bottom-3),306,1,2);
-    ui_move(17,20,bottom-22,14,14);
-    ui_arrow(26,53,0);
-    ui_arrow(26,bottom-21,1);
+    ui_move(17,12,bottom-22,14,14);
+    ui_arrow(19,53,0);
+    ui_arrow(19,bottom-21,1);
 }
 void Expanded_UI_ClearFixtureArea(void) {
     if(!ui_load() || _currentMenuPtr!=(uintptr_t)_contestMenu) return;
@@ -1603,11 +1626,11 @@
     /* Remove any part of the old down arrow still inside the new panel.
      * This narrow strip never touches the fixture text (starts at x=29). */
     int oldArrow=oldBottom-22;
-    if(oldArrow<bottom) rect(19,(unsigned)oldArrow,16,(unsigned)(bottom-oldArrow),42);
+    if(oldArrow<bottom) rect(11,(unsigned)oldArrow,17,(unsigned)(bottom-oldArrow),42);
     cb_box_outline(5,39,310,bottom-39,7);
     rect(7,(unsigned)(bottom-3),306,1,2);
-    ui_move(17,20,bottom-22,14,14);
-    ui_arrow(26,bottom-21,1);
+    ui_move(17,12,bottom-22,14,14);
+    ui_arrow(19,bottom-21,1);
     ui_fixture_panel_bottom=bottom;
 }
 static void ui_native_ellipsis(int x,int y,int maxWidth,uint8_t color,
--- a/sorgenti/src/build_info.c
+++ b/sorgenti/src/build_info.c
@@ -87,7 +87,7 @@
 
 const char *Build_Version(void)
 {
-    return "SWOS Expanded 0.10.22 - " PROJECT_NAME " " SWOS_BUILD_ID
+    return "SWOS Expanded 0.10.23 - " PROJECT_NAME " " SWOS_BUILD_ID
            " (" SWOS_BUILD_DATE ", " SWOS_PLATFORM SWOS_BUILD_RULES ")";
 }
 
--- a/CONFIGURA_4K.ps1
+++ b/CONFIGURA_4K.ps1
@@ -15,7 +15,7 @@
  if ($videoStart -ge 0 -and $lines[$i] -match '^\s*\[') { $videoEnd = $i; break }
 }
 if ($videoStart -lt 0) { $lines.Add('[video]'); $videoStart=$lines.Count-1; $videoEnd=$lines.Count }
-$settings = [ordered]@{ fullscreen='desktop'; scaling='sharp'; crt_mask='off'; subpixel_scroll='false' }
+$settings = [ordered]@{ fullscreen='desktop'; widescreen='stretch'; resolution='auto'; scaling='sharp'; crt_mask='off'; subpixel_scroll='false' }
 foreach ($key in $settings.Keys) {
  $found=$false
  for ($i=$videoStart+1; $i -lt $videoEnd; $i++) {
--- a/AVVIA_SWOS.bat
+++ b/AVVIA_SWOS.bat
@@ -1,7 +1,7 @@
 @echo off
 setlocal EnableExtensions
 cd /d "%~dp0"
-title SWOS Expanded 0.10.22 - Avvio
+title SWOS Expanded 0.10.23 - Avvio
 
 REM La prima volta compila i sorgenti corretti; dopo avvia direttamente il gioco.
 call "%~dp0PREPARA_SWOS.bat"
@@ -27,7 +27,7 @@
 set "SWOS_TACTICS_JSON=0"
 set "SWOS_MENU_NAV_LOG=1"
 echo.
-echo === SWOS Expanded 0.10.22 - avvio ===
+echo === SWOS Expanded 0.10.23 - avvio ===
 "%~dp0SWOS.exe"
 set "RC=%ERRORLEVEL%"
 echo.
@@ -39,6 +39,6 @@
 :errore
 echo.
 echo Il gioco non e stato avviato.
-echo Se la compilazione e fallita, invia COMPILAZIONE_0122.log.
+echo Se la compilazione e fallita, invia COMPILAZIONE_0123.log.
 pause
 exit /b 1
--- a/PREPARA_SWOS.bat
+++ b/PREPARA_SWOS.bat
@@ -1,11 +1,11 @@
 @echo off
 setlocal EnableExtensions
 cd /d "%~dp0"
-title SWOS Expanded 0.10.22 - Preparazione
+title SWOS Expanded 0.10.23 - Preparazione
 
 REM Un eseguibile 0.10 precedente non e considerato valido.
-if exist ".SWOS_0122_COMPILATO" if exist "SWOS.exe" (
-  echo SWOS 0.10.22 pronto.
+if exist ".SWOS_0123_COMPILATO" if exist "SWOS.exe" (
+  echo SWOS 0.10.23 pronto.
   exit /b 0
 )
 
@@ -48,14 +48,14 @@
 "%MSYS_ROOT%\usr\bin\bash.exe" -lc "bash -s" < "%~dp0BUILD_SWOS.sh"
 if errorlevel 1 (
   echo.
-  echo COMPILAZIONE FALLITA: invia COMPILAZIONE_0122.log.
+  echo COMPILAZIONE FALLITA: invia COMPILAZIONE_0123.log.
   exit /b 1
 )
 if not exist "SWOS.exe" (
   echo ERRORE: compilazione terminata senza SWOS.exe.
   exit /b 1
 )
-> ".SWOS_0122_COMPILATO" echo SWOS Expanded 0.10.22 - compilazione completata
+> ".SWOS_0123_COMPILATO" echo SWOS Expanded 0.10.23 - compilazione completata
 exit /b 0
 
 :cerca_msys
--- a/BUILD_SWOS.sh
+++ b/BUILD_SWOS.sh
@@ -23,9 +23,9 @@
     exit 1
   fi
 done
-exec > >(tee "$game/COMPILAZIONE_0122.log") 2>&1
+exec > >(tee "$game/COMPILAZIONE_0123.log") 2>&1
 
-echo '==== SWOS Expanded 0.10.22 - compilazione Windows x64 ===='
+echo '==== SWOS Expanded 0.10.23 - compilazione Windows x64 ===='
 echo "Sorgenti: $src"
 echo "Gioco:    $game"
 echo 'Verifica delle dipendenze MSYS2...'
@@ -41,12 +41,12 @@
   -Isrc -Iinclude $(pkg-config --cflags sdl2) \
   src/menu_data.c src/menu_system.c src/menus.c src/expanded_career.c \
   src/expanded_economy.c src/career_init.c src/career_status.c src/career_teams.c src/transfer_market.c src/chairman_memo.c src/save_format_json.c src/input_text.c src/blitter.c src/build_info.c
-echo 'Compilo il gioco 0.10.22...'
+echo 'Compilo il gioco 0.10.23...'
 make -j4 NETPLAY=0
 if [[ ! -s bin/SWOS.exe ]]; then
   echo 'ERRORE: il compilatore non ha prodotto bin/SWOS.exe.'
   exit 1
 fi
-cp -f bin/SWOS.exe "$game/SWOS_0122_new.exe"
-mv -f "$game/SWOS_0122_new.exe" "$game/SWOS.exe"
+cp -f bin/SWOS.exe "$game/SWOS_0123_new.exe"
+mv -f "$game/SWOS_0123_new.exe" "$game/SWOS.exe"
 echo 'Compilazione completata: SWOS.exe aggiornato.'

```

---

## PATCH_0123_TO_0124.diff

```text
--- a/sorgenti/src/expanded_economy.c
+++ b/sorgenti/src/expanded_economy.c
@@ -838,17 +838,10 @@
     else if(align>0) DrawMenuTextRightAligned((uint16_t)x,(uint16_t)y,color,(uint8_t *)text,font);
     else DrawMenuTextCentered((uint16_t)x,(uint16_t)y,color,(uint8_t *)text,font);
 }
-/* Original 5x8 bitmap: a readable compact font for the tiny YEAR/WEEK
- * toggle, independent of the wider native SWOS sprite font. */
-static void cb_tiny_label(int centerX,int y,uint8_t color,const char *s) {
-    if(!s) return;
-    int len=(int)strlen(s),x=centerX-(len*6-1)/2;
-    for(int i=0;i<len;i++) {
-        const uint8_t *glyph=cb_glyph((unsigned char)s[i]);
-        for(int row=0;row<8;row++)
-            for(int bit=0;bit<5;bit++)
-                if(glyph[row] & (1u<<(4-bit))) cb_plot(x+i*6+bit,y+row,color);
-    }
+/* WEEK/YEAR uses the SAME original SWOS font as the rest of the screen.
+ * Its 33x12 focus box is unchanged: this is only a typeface correction. */
+static void cb_finance_toggle_label(int centerX,int y,uint8_t color,const char *s) {
+    cb_native(centerX,y,color,s,0,29); /* 2px padding inside 33px box */
 }
 static void cb_text_step(int x,int y,uint8_t color,const char *str,int step) {
     (void)step;
@@ -1041,7 +1034,7 @@
     /* 33x12, 2px below the frame top and above the divider. Even when
      * focused the button never overlaps the title strip's outer border. */
     rect(168,187,33,12,42);cb_box_outline(168,187,33,12,204);
-    cb_tiny_label(184,189,gold,_expandedFinanceSeasonView?"WEEK":"YEAR");
+    cb_finance_toggle_label(184,189,gold,_expandedFinanceSeasonView?"WEEK":"YEAR");
     cb_center(264,189,white,"BALANCE");
     static const char *const leftLabel[4]={"SPONSOR","TV","GATE","INCOME"};
     static const char *const rightLabel[4]={"WAGES","TRAVEL","STAFF","TOTAL"};
@@ -2107,7 +2100,7 @@
                 name=(const char *)(uintptr_t)e->fg;
             ui_button(i,name);
         }
-        ui_native_text(43,246,70,15,"V0.10.21",0);
+        ui_native_text(43,246,70,15,"V0.10.24",0);
     } else if(kind==2) {
         Menu *m=(Menu *)_currentMenu;
         if(!m) return;
--- a/sorgenti/src/platform_sdl.c
+++ b/sorgenti/src/platform_sdl.c
@@ -114,6 +114,25 @@
 /* ---- SDL state ----------------------------------------------------- */
 static SDL_Window   *s_window;
 static SDL_Renderer *s_renderer;
+/* The approved Expanded menu artwork is 320x256, while the original Amiga
+ * display is 336x272. In stretch/fullscreen mode present only the artwork's
+ * true rectangle across the entire output; otherwise the 16px right/bottom
+ * margin reveals the old map/crest background. This is PRESENTATION-ONLY:
+ * menu hitboxes, framebuffer, save data and match camera are untouched. */
+static volatile int s_menu_art_fullscreen;
+static int s_stretch;
+void Platform_SetMenuArtFullscreen(int enabled)
+{
+    s_menu_art_fullscreen = enabled != 0;
+}
+static void menu_art_mouse_coords(int *x, int *y)
+{
+    if (s_menu_art_fullscreen && s_stretch && _screen_width == 336 &&
+        _screen_height == 272) {
+        *x = *x * 320 / 336;
+        *y = *y * 256 / 272;
+    }
+}
 /* The roster is the ONLY legacy menu with a mouse hover tooltip. Return a
  * row only when the pointer actually MOVES, so a parked mouse never steals
  * selection from the joypad/d-pad. SDL logical letterboxing is respected. */
@@ -143,6 +162,7 @@
     } else {
         x=(int)(mx/sx);y=(int)(my/sy);
     }
+    menu_art_mouse_coords(&x, &y);
     x-=_menu_draw_x_offset;
     if(x<7 || x>312 || y<90 || y>=226) return -1;
     return (y-90)/8;
@@ -165,6 +185,7 @@
     double scale=sx<sy?sx:sy;
     int x=hasLogical?(int)((mx-(ww-lw*scale)*0.5)/scale):(int)(mx/sx);
     int y=hasLogical?(int)((my-(wh-lh*scale)*0.5)/scale):(int)(my/sy);
+    menu_art_mouse_coords(&x, &y);
     x-=_menu_draw_x_offset;
     return x>=111 && x<=211 && y>=222 && y<=235;
 }
@@ -664,7 +685,8 @@
  * show more pitch (true). SDL's logical size keeps the picture's shape, so for stretch it
  * is replaced by a per-axis scale from the picture to the window, worked out every frame:
  * SDL re-applies a logical size across window changes by itself, but not a scale. */
-static int s_stretch;
+/* s_stretch is declared with the menu presentation state above so mouse
+ * hover can use the same output-to-artwork transform as the renderer. */
 
 static void apply_picture_scaling(void)
 {
@@ -1856,6 +1878,23 @@
      * region explicitly. A NULL src would stretch the strip into the picture. */
     const SDL_Rect worldSrc = { 0, 0, _screen_width, _screen_height };
 
+    /* 0.10.24: fill the entire 4K/16:9 screen with the APPROVED 320x256
+     * Expanded artwork, not the unrelated 336x272 SWOS map/crest underneath.
+     * Do it at presentation time, so all native drawing, selection outlines,
+     * joypad navigation and partial redraws keep their original coordinates.
+     * The match renderer is unaffected (GameLoop resets this flag). */
+    if (s_stretch && s_menu_art_fullscreen && _screen_width == 336 &&
+        _screen_height == 272) {
+        const SDL_Rect menuSrc = { 0, 0, 320, 256 };
+        if (n == 1)
+            SDL_RenderCopy(s_renderer, tex, &menuSrc, NULL);
+        else {
+            SDL_FRect menuDst = { 0.0f, 0.0f, w, h };
+            SDL_RenderCopyF(s_renderer, tex, &menuSrc, &menuDst);
+        }
+        return; /* no old HUD or scrolling fringe over menu artwork */
+    }
+
     if (qx == 0.0f && qy == 0.0f) {
         if (n == 1) {
             SDL_RenderCopy(s_renderer, tex, &worldSrc, NULL);   /* unchanged fast path */
--- a/sorgenti/src/menu_system.c
+++ b/sorgenti/src/menu_system.c
@@ -1388,6 +1388,17 @@
     _menu_draw_x_offset = WIDE_MARGIN + AGA_MENU_FG_SHIFT_BYTES;
 #else
     _menu_draw_x_offset = WIDE_MARGIN;
+#endif
+#ifdef SWOS_SDL_BUILD
+    /* Only the 320x256 Expanded artwork is scaled edge-to-edge. Native menus
+     * retain their original 336x272 presentation; no gameplay or camera crop. */
+    {
+        extern void Platform_SetMenuArtFullscreen(int);
+        extern int Expanded_UI_Active(void);
+        extern uint8_t _expandedClubOverviewMenu[];
+        Platform_SetMenuArtFullscreen(Expanded_UI_Active() ||
+            _currentMenuPtr == (uintptr_t)_expandedClubOverviewMenu);
+    }
 #endif
 
     BlitterCopyPanelToScreen2();
--- a/sorgenti/src/game_loop.c
+++ b/sorgenti/src/game_loop.c
@@ -784,6 +784,11 @@
      * offset 0 — the pitch fills the whole frame. Menu/HUD centering is re-established
      * by DrawMenu when the frontend draws again. */
     _menu_draw_x_offset = 0;
+#ifdef SWOS_SDL_BUILD
+    /* A live match always presents its FULL 336x272 (or wider) world.
+     * Never inherit the 320x256 menu-art crop after leaving a menu. */
+    { extern void Platform_SetMenuArtFullscreen(int); Platform_SetMenuArtFullscreen(0); }
+#endif
     ManageAdvertisements();
     DoGoalkeeperSprites();
     UpdateControlledPlayerNumbers();
--- a/sorgenti/src/build_info.c
+++ b/sorgenti/src/build_info.c
@@ -87,7 +87,7 @@
 
 const char *Build_Version(void)
 {
-    return "SWOS Expanded 0.10.23 - " PROJECT_NAME " " SWOS_BUILD_ID
+    return "SWOS Expanded 0.10.24 - " PROJECT_NAME " " SWOS_BUILD_ID
            " (" SWOS_BUILD_DATE ", " SWOS_PLATFORM SWOS_BUILD_RULES ")";
 }
 
--- a/AVVIA_SWOS.bat
+++ b/AVVIA_SWOS.bat
@@ -1,7 +1,7 @@
 @echo off
 setlocal EnableExtensions
 cd /d "%~dp0"
-title SWOS Expanded 0.10.23 - Avvio
+title SWOS Expanded 0.10.24 - Avvio
 
 REM La prima volta compila i sorgenti corretti; dopo avvia direttamente il gioco.
 call "%~dp0PREPARA_SWOS.bat"
@@ -27,7 +27,7 @@
 set "SWOS_TACTICS_JSON=0"
 set "SWOS_MENU_NAV_LOG=1"
 echo.
-echo === SWOS Expanded 0.10.23 - avvio ===
+echo === SWOS Expanded 0.10.24 - avvio ===
 "%~dp0SWOS.exe"
 set "RC=%ERRORLEVEL%"
 echo.
@@ -39,6 +39,6 @@
 :errore
 echo.
 echo Il gioco non e stato avviato.
-echo Se la compilazione e fallita, invia COMPILAZIONE_0123.log.
+echo Se la compilazione e fallita, invia COMPILAZIONE_0124.log.
 pause
 exit /b 1
--- a/PREPARA_SWOS.bat
+++ b/PREPARA_SWOS.bat
@@ -1,11 +1,11 @@
 @echo off
 setlocal EnableExtensions
 cd /d "%~dp0"
-title SWOS Expanded 0.10.23 - Preparazione
+title SWOS Expanded 0.10.24 - Preparazione
 
 REM Un eseguibile 0.10 precedente non e considerato valido.
-if exist ".SWOS_0123_COMPILATO" if exist "SWOS.exe" (
-  echo SWOS 0.10.23 pronto.
+if exist ".SWOS_0124_COMPILATO" if exist "SWOS.exe" (
+  echo SWOS 0.10.24 pronto.
   exit /b 0
 )
 
@@ -48,14 +48,14 @@
 "%MSYS_ROOT%\usr\bin\bash.exe" -lc "bash -s" < "%~dp0BUILD_SWOS.sh"
 if errorlevel 1 (
   echo.
-  echo COMPILAZIONE FALLITA: invia COMPILAZIONE_0123.log.
+  echo COMPILAZIONE FALLITA: invia COMPILAZIONE_0124.log.
   exit /b 1
 )
 if not exist "SWOS.exe" (
   echo ERRORE: compilazione terminata senza SWOS.exe.
   exit /b 1
 )
-> ".SWOS_0123_COMPILATO" echo SWOS Expanded 0.10.23 - compilazione completata
+> ".SWOS_0124_COMPILATO" echo SWOS Expanded 0.10.24 - compilazione completata
 exit /b 0
 
 :cerca_msys
--- a/BUILD_SWOS.sh
+++ b/BUILD_SWOS.sh
@@ -23,9 +23,9 @@
     exit 1
   fi
 done
-exec > >(tee "$game/COMPILAZIONE_0123.log") 2>&1
+exec > >(tee "$game/COMPILAZIONE_0124.log") 2>&1
 
-echo '==== SWOS Expanded 0.10.23 - compilazione Windows x64 ===='
+echo '==== SWOS Expanded 0.10.24 - compilazione Windows x64 ===='
 echo "Sorgenti: $src"
 echo "Gioco:    $game"
 echo 'Verifica delle dipendenze MSYS2...'
@@ -40,13 +40,13 @@
 gcc -std=gnu11 -fsyntax-only -DSWOS_SDL_BUILD=1 -D_FILE_OFFSET_BITS=64 -include include/swos_platform.h \
   -Isrc -Iinclude $(pkg-config --cflags sdl2) \
   src/menu_data.c src/menu_system.c src/menus.c src/expanded_career.c \
-  src/expanded_economy.c src/career_init.c src/career_status.c src/career_teams.c src/transfer_market.c src/chairman_memo.c src/save_format_json.c src/input_text.c src/blitter.c src/build_info.c
-echo 'Compilo il gioco 0.10.23...'
+  src/expanded_economy.c src/platform_sdl.c src/game_loop.c src/career_init.c src/career_status.c src/career_teams.c src/transfer_market.c src/chairman_memo.c src/save_format_json.c src/input_text.c src/blitter.c src/build_info.c
+echo 'Compilo il gioco 0.10.24...'
 make -j4 NETPLAY=0
 if [[ ! -s bin/SWOS.exe ]]; then
   echo 'ERRORE: il compilatore non ha prodotto bin/SWOS.exe.'
   exit 1
 fi
-cp -f bin/SWOS.exe "$game/SWOS_0123_new.exe"
-mv -f "$game/SWOS_0123_new.exe" "$game/SWOS.exe"
+cp -f bin/SWOS.exe "$game/SWOS_0124_new.exe"
+mv -f "$game/SWOS_0124_new.exe" "$game/SWOS.exe"
 echo 'Compilazione completata: SWOS.exe aggiornato.'
--- a/LEGGIMI.txt
+++ b/LEGGIMI.txt
@@ -1,6 +1,6 @@
-SWOS Expanded 0.10.23 - BUSINESS, CALENDARIO, VIDEO 4K
+SWOS Expanded 0.10.24 - BUSINESS, CALENDARIO, VIDEO 4K
 ======================================================
 Estrai lo ZIP completo in una NUOVA cartella.
 Doppio clic su AVVIA_SWOS.bat per compilare e avviare il gioco con MSYS2.
-Per le correzioni leggi LEGGIMI_0123_IT.txt.
-Se la compilazione fallisce, invia COMPILAZIONE_0123.log.
+Per le correzioni leggi LEGGIMI_0124_IT.txt.
+Se la compilazione fallisce, invia COMPILAZIONE_0124.log.

```

---

## PATCH_09_TO_10.diff

```text
--- a/src/expanded_career.c
+++ b/src/expanded_career.c
@@ -211,43 +211,62 @@
     ShowMenu(_expandedHallMenu);
 }
 
-/* Expanded 0.9: job search uses the native contest/calendar geometry,
- * but NEVER calls the original fixture generator before InitCareer(). */
+/* Expanded 0.10: the unemployed screen uses a cloned copy of the original
+ * packed contest menu. No fake custom buttons or alternate panel geometry. */
 #include <time.h>
-extern uint8_t _expandedUnemployedMenu[];
+extern uint8_t *Expanded_UnemployedCalendarMenu(void);
 extern uint8_t _expandedClubOfferMenu[];
+extern uint8_t _expandedOfferPreviewMenu[];
 extern uint8_t _teamFileBuffer[];
 extern uint16_t _teamsLoaded;
 extern int16_t LoadTeamFile(uint16_t extNumber);
 extern void DrawMenu(void);
 extern void SetExitMenuFlag(void);
-extern void FillRectangle(uint16_t,uint16_t,uint16_t,uint16_t,uint16_t);
 extern uint16_t DrawMenuTextCentered(uint16_t,uint16_t,uint16_t,uint8_t *,uint8_t *);
 extern void *_charTables[];
 extern void *_countriesTable[262];
 extern MenuEntry _menuEntries[];
 extern uint8_t _currentMenu[];
 extern uint16_t _fireResetFlag;
+extern void Expanded_FormatDate(char *,size_t,unsigned,unsigned);
 static unsigned s_waitWeeks=0;
 static unsigned s_nextOfferWeek=0;
-static unsigned s_offerCount=0;
 static int s_waitAccepted=0;
 static uint16_t s_waitTeam=0;
 static uint8_t s_waitCountry=8;
+static uint8_t s_waitLeague=0;
 static char s_waitTeamName[40]="";
 static char s_waitCountryName[40]="";
+static char s_previewPlayers[16][28];
+static char s_previewTeams[16][24];
+static unsigned s_previewCount=0;
+static unsigned s_previewMode=0; /* 1=squad, 2=league */
 static unsigned s_randomState=1;
 static unsigned next_random(void) {
-    /* Small local PRNG: does not consume SWOS match simulation RNG. */
     s_randomState ^= s_randomState << 13;
     s_randomState ^= s_randomState >> 17;
     s_randomState ^= s_randomState << 5;
     return s_randomState;
 }
-static unsigned next_delay(void) { return 1u + next_random()%5u; }
+static unsigned next_delay(void) {return 1u + next_random()%5u;}
+/* Recheck the actual team file every time, including before acceptance.
+ * The original GetTeamPointer has an unbounded search on an invalid ID. */
+static int find_offer_team(TeamFileEx *out) {
+    if(!s_waitTeam || LoadTeamFile(s_waitCountry)!=0) return 0;
+    unsigned count=*(const uint16_t*)_teamFileBuffer;
+    if(!count || count>256) return 0;
+    const TeamFileEx *teams=(const TeamFileEx*)(_teamFileBuffer+2);
+    for(unsigned i=0;i<count;i++) {
+        const TeamFile *t=&teams[i].team;
+        if(t->countryNumber==s_waitCountry &&
+           t->teamOrdinal==(uint8_t)s_waitTeam) {
+            if(out) memcpy(out,&teams[i],sizeof *out);
+            return 1;
+        }
+    }
+    return 0;
+}
 static void choose_small_club(void) {
-    /* Real teams from the shipped data. Select a random club from the
-     * bottom league of a randomly chosen playable national file. */
     static const uint8_t files[]={8,13,14,20,21};
     unsigned start=next_random()%(sizeof files/sizeof files[0]);
     s_waitTeam=0;
@@ -256,15 +275,17 @@
         _teamsLoaded=0;
         if(LoadTeamFile(country)!=0) continue;
         unsigned count=*(const uint16_t*)_teamFileBuffer;
-        if(!count || count>93) continue;
+        if(!count || count>256) continue;
         const TeamFileEx *teams=(const TeamFileEx*)(_teamFileBuffer+2);
-        unsigned maxLeague=0, candidates=0;
+        unsigned maxLeague=0,candidates=0;
         for(unsigned i=0;i<count;i++)
             if(teams[i].team.countryNumber==country && teams[i].team.league<32 &&
                teams[i].team.league>maxLeague) maxLeague=teams[i].team.league;
+        if(maxLeague<2) continue;
         for(unsigned i=0;i<count;i++)
-            if(teams[i].team.countryNumber==country && teams[i].team.league==maxLeague) candidates++;
-        if(!candidates || maxLeague<2) continue;
+            if(teams[i].team.countryNumber==country && teams[i].team.league==maxLeague)
+                candidates++;
+        if(!candidates) continue;
         unsigned selected=next_random()%candidates;
         for(unsigned i=0;i<count;i++) {
             const TeamFile *t=&teams[i].team;
@@ -272,62 +293,63 @@
             if(selected--==0) {
                 s_waitTeam=(uint16_t)((country<<8)|t->teamOrdinal);
                 s_waitCountry=country;
+                s_waitLeague=(uint8_t)maxLeague;
                 snprintf(s_waitTeamName,sizeof s_waitTeamName,"%.17s",(const char*)t->teamName);
                 if(_countriesTable[country])
                     snprintf(s_waitCountryName,sizeof s_waitCountryName,"%.28s",
                              (const char*)_countriesTable[country]+1);
                 else snprintf(s_waitCountryName,sizeof s_waitCountryName,"COUNTRY %u",country);
+                if(!find_offer_team(NULL)) {s_waitTeam=0;continue;}
                 return;
             }
         }
     }
 }
 void Expanded_UnemployedInit(void) {
-    /* No club-dependent commands have onSelect callbacks. The VIEW OFFER
-     * action appears only when an offer is pending. */
-    _menuEntries[5].isInvisible=(s_waitTeam==0 || s_waitWeeks<s_nextOfferWeek);
-    _menuEntries[4].isInvisible=(s_waitTeam!=0 && s_waitWeeks>=s_nextOfferWeek);
-    ((Menu*)_currentMenu)->selectedEntry=&_menuEntries[
-        (s_waitTeam!=0 && s_waitWeeks>=s_nextOfferWeek) ? 5 : 4];
+    /* These ordinals belong to the *original* contest menu layout.
+     * All club-specific entries are hidden, not merely grey and clickable. */
+    for(unsigned i=0;i<44;i++)
+        _menuEntries[i].isInvisible=!(i<=3 || i==7 || i==9 || i==10 || i==18);
+    int offerReady=s_waitTeam && s_waitWeeks>=s_nextOfferWeek;
+    _menuEntries[7].isInvisible=offerReady;
+    _menuEntries[10].isInvisible=!offerReady;
+    ((Menu*)_currentMenu)->selectedEntry=&_menuEntries[offerReady?10:7];
 }
 static void contest_line(unsigned y,const char *line,unsigned color) {
     DrawMenuTextCentered(160,(uint16_t)y,(uint16_t)color,(uint8_t*)line,(uint8_t*)_charTables[0]);
 }
 void Expanded_UnemployedDraw(void) {
     char line[90];
-    /* This is the original calendar panel geometry (304x116 at y=31),
-     * not a second independently designed grid of buttons. */
-    FillRectangle(8,31,304,116,8);
-    FillRectangle(10,33,300,112,0);
-    FillRectangle(12,35,296,13,9);
-    contest_line(37,"FIXTURES / CALENDAR",7);
-    snprintf(line,sizeof line,"WEEK %u  -  UNEMPLOYED",s_waitWeeks+1);
-    contest_line(59,line,7);
+    char date[40];
+    Expanded_FormatDate(date,sizeof date,1996,s_waitWeeks);
+    contest_line(39,date,2);
+    contest_line(58,"NO MATCH - UNEMPLOYED",7);
     if(s_waitTeam && s_waitWeeks>=s_nextOfferWeek) {
-        contest_line(81,"CLUB OFFER RECEIVED",2);
-        contest_line(103,"VIEW OFFER FOR CLUB AND COUNTRY",7);
+        contest_line(80,"CLUB OFFER RECEIVED",2);
+        contest_line(104,"VIEW OFFER TO CONTINUE",7);
     } else {
-        contest_line(81,"WAITING FOR AN OFFER FROM A CLUB",7);
-        contest_line(103,"NO MATCH SCHEDULED",7);
-    }
-    contest_line(129,"CLUB COMMANDS UNAVAILABLE",6);
+        contest_line(80,"WAITING FOR A CLUB OFFER",7);
+        snprintf(line,sizeof line,"WEEK %u - FORWARD TIME",s_waitWeeks+1);
+        contest_line(104,line,7);
+    }
+    contest_line(128,"CLUB ACTIONS DISABLED",6);
 }
 void Expanded_UnemployedWait(void) {
     if(s_waitTeam && s_waitWeeks>=s_nextOfferWeek) return;
     if(s_waitWeeks<100) s_waitWeeks++;
     if(s_waitWeeks>=s_nextOfferWeek && !s_waitTeam) choose_small_club();
-    _fireResetFlag=1; /* one physical press = exactly one week */
+    if(!s_waitTeam && s_waitWeeks>=s_nextOfferWeek)
+        s_nextOfferWeek=s_waitWeeks+next_delay();
+    _fireResetFlag=1;
     Expanded_UnemployedInit();
     DrawMenu();
 }
 void Expanded_OfferDraw(void) {
     char line[100];
-    FillRectangle(22,39,276,119,8);
-    FillRectangle(24,41,272,115,0);
-    contest_line(52,"A CLUB WANTS TO HIRE YOU",7);
-    snprintf(line,sizeof line,"CLUB: %.32s",s_waitTeamName); contest_line(77,line,2);
-    snprintf(line,sizeof line,"COUNTRY: %.32s",s_waitCountryName);contest_line(98,line,7);
-    contest_line(126,"ACCEPT OR DECLINE THE OFFER",7);
+    contest_line(48,"A CLUB WANTS TO HIRE YOU",7);
+    snprintf(line,sizeof line,"CLUB: %.25s",s_waitTeamName); contest_line(74,line,2);
+    snprintf(line,sizeof line,"COUNTRY: %.25s",s_waitCountryName);contest_line(93,line,7);
+    snprintf(line,sizeof line,"DIVISION: %u",(unsigned)s_waitLeague+1);contest_line(110,line,7);
 }
 void Expanded_UnemployedShowOffer(void) {
     if(!s_waitTeam || s_waitWeeks<s_nextOfferWeek) return;
@@ -336,27 +358,74 @@
     else { Expanded_UnemployedInit(); DrawMenu(); }
 }
 void Expanded_UnemployedAccept(void) {
-    if(!s_waitTeam || s_waitWeeks<s_nextOfferWeek) return;
+    if(!s_waitTeam || !find_offer_team(NULL)) {
+        fprintf(stderr,"[expanded10] rejected invalid team id %04x\n",s_waitTeam);
+        s_waitTeam=0;
+        SetExitMenuFlag();
+        return;
+    }
     s_waitAccepted=1;
     SetExitMenuFlag();
 }
 void Expanded_UnemployedReject(void) {
     s_waitTeam=0;
-    s_offerCount++;
     s_nextOfferWeek=s_waitWeeks+next_delay();
     SetExitMenuFlag();
+}
+void Expanded_OfferPreviewDraw(void) {
+    char line[90];
+    contest_line(31,s_previewMode==1?"CLUB SQUAD":"CLUB COMPETITION",2);
+    snprintf(line,sizeof line,"%.23s - %.18s",s_waitTeamName,s_waitCountryName);
+    contest_line(48,line,7);
+    if(s_previewMode==1) {
+        for(unsigned i=0;i<16;i++) {
+            snprintf(line,sizeof line,"%02u %.25s",i+1,s_previewPlayers[i]);
+            contest_line(61+i*10,line,7);
+        }
+    } else {
+        snprintf(line,sizeof line,"DOMESTIC DIVISION %u",(unsigned)s_waitLeague+1);
+        contest_line(69,line,7);
+        contest_line(83,"TEAMS IN THIS DIVISION",2);
+        for(unsigned i=0;i<s_previewCount && i<10;i++)
+            contest_line(96+i*11,s_previewTeams[i],7);
+        contest_line(214,"STANDINGS NOT YET SIMULATED",6);
+    }
+}
+void Expanded_OfferViewSquad(void) {
+    TeamFileEx team;
+    if(!find_offer_team(&team)) return;
+    for(unsigned i=0;i<16;i++)
+        snprintf(s_previewPlayers[i],sizeof s_previewPlayers[i],"%.22s",
+                 (const char*)team.players[i].playerName);
+    s_previewMode=1;
+    ShowMenu(_expandedOfferPreviewMenu);
+}
+void Expanded_OfferViewLeague(void) {
+    if(LoadTeamFile(s_waitCountry)!=0) return;
+    unsigned count=*(const uint16_t*)_teamFileBuffer;
+    if(count>256) return;
+    const TeamFileEx *teams=(const TeamFileEx*)(_teamFileBuffer+2);
+    s_previewCount=0;
+    for(unsigned i=0;i<count && s_previewCount<16;i++) {
+        if(teams[i].team.countryNumber!=s_waitCountry ||
+           teams[i].team.league!=s_waitLeague) continue;
+        snprintf(s_previewTeams[s_previewCount++],24,"%.17s",
+                 (const char*)teams[i].team.teamName);
+    }
+    s_previewMode=2;
+    ShowMenu(_expandedOfferPreviewMenu);
 }
 int Expanded_WaitForFirstOffer(uint16_t *team) {
     if(!team) return 0;
     s_randomState=(unsigned)time(NULL) ^ (unsigned)(uintptr_t)team ^
                   ((unsigned)_expandedManagerStartAge<<16) ^ 0xA529B41Du;
     if(!s_randomState) s_randomState=1;
-    s_waitWeeks=0; s_offerCount=0; s_waitAccepted=0; s_waitTeam=0;
+    s_waitWeeks=0; s_waitAccepted=0; s_waitTeam=0;
     s_nextOfferWeek=next_delay();
-    ShowMenu(_expandedUnemployedMenu);
-    if(!s_waitAccepted || !s_waitTeam) return 0;
+    ShowMenu(Expanded_UnemployedCalendarMenu());
+    if(!s_waitAccepted || !s_waitTeam || !find_offer_team(NULL)) return 0;
     *team=s_waitTeam;
-    fprintf(stderr,"[expanded09] accepted job: country=%u club=%s after %u weeks\n",
-            (unsigned)s_waitCountry,s_waitTeamName,s_waitWeeks);
+    fprintf(stderr,"[expanded10] accepted club %s (%u) after %u weeks\n",
+            s_waitTeamName,(unsigned)s_waitCountry,s_waitWeeks);
     return 1;
 }
--- a/src/expanded_economy.c
+++ b/src/expanded_economy.c
@@ -34,6 +34,10 @@
 uint8_t _expandedBoardMood = 50;
 uint8_t _expandedFanMood = 50;
 uint32_t _expandedSupporters = 0;
+uint8_t _expandedLowConfidenceMatches = 0;
+extern volatile uint16_t _youreSacked;
+/* Original SWOS remains in charge of displaying the sacking and job offers.
+ * Never invoke a modal menu from inside a match-result callback. */
 extern uint8_t _expandedManagerFame;
 static void ensure_club(void);
 static uint8_t clamp_mood(int value) {
@@ -46,6 +50,16 @@
     int fans = result>0 ? 7 : result<0 ? -8 : 1;
     _expandedBoardMood=clamp_mood((int)_expandedBoardMood+board+money);
     _expandedFanMood=clamp_mood((int)_expandedFanMood+fans);
+    if ((_expandedBoardMood<=12 && _expandedFanMood<=20) ||
+        (_expandedBoardMood<=5 && _expandedFanMood<=35)) {
+        if (_expandedLowConfidenceMatches<255) _expandedLowConfidenceMatches++;
+        if (_expandedLowConfidenceMatches>=3) {
+            fprintf(stderr,"[expanded10] dismissal: board=%u fans=%u after %u bad reviews\n",
+                (unsigned)_expandedBoardMood,(unsigned)_expandedFanMood,
+                (unsigned)_expandedLowConfidenceMatches);
+            _youreSacked=1;
+        }
+    } else _expandedLowConfidenceMatches=0;
     int delta = result>0 ? 125 : result<0 ? -90 : 15;
     if(delta<0 && _expandedSupporters<(uint32_t)(-delta)) _expandedSupporters=0;
     else _expandedSupporters=(uint32_t)((int32_t)_expandedSupporters+delta);
@@ -98,6 +112,7 @@
     _expandedShirtSponsor = _expandedStadiumSponsor = _expandedTvRights = 0;
     _expandedBoardMood = clamp_mood(50+(int)_expandedManagerFame*5);
     _expandedFanMood = clamp_mood(40+(int)_expandedManagerFame*9);
+    _expandedLowConfidenceMatches=0;
     _expandedSupporters = s_capacity[_expandedFacilityLevels[0]-1] *
                           (uint32_t)_expandedFanMood / 100u;
 }
@@ -108,6 +123,7 @@
     _expandedEconomyLastSeason = 0xFFFFu;
     _expandedShirtSponsor = _expandedStadiumSponsor = _expandedTvRights = 0;
     _expandedBoardMood=50; _expandedFanMood=50; _expandedSupporters=0;
+    _expandedLowConfidenceMatches=0;
 }
 static int32_t add_saturating(int32_t value, uint32_t gain)
 {
@@ -287,6 +303,10 @@
     /* The legacy generic upgrade row stays in the packed layout to keep
      * ordinals stable, but is not visible or selectable. */
     _menuEntries[9].isInvisible=1;
+    for (unsigned i=1; i<=8; ++i) {
+        _menuEntries[i].background=NO_BACKGROUND;
+        _menuEntries[i].content=NO_FOREGROUND;
+    }
 }
 void Expanded_ClubOverviewIcons(void) {
     static const char *const names[8]={"STADIUM","TRAIN","TRAVEL","OFFICE",
@@ -303,6 +323,33 @@
     if (!entry || entry->ordinal < 1 || entry->ordinal > 8) return;
     _expandedFacilitySelected=(uint8_t)(entry->ordinal-1);
     ShowMenu(_expandedFacilityDetailMenu);
+}
+
+/* Calendar date, same formatter in the match calendar and the club ledger.
+ * Career finance week is the best available weekly clock; it is not a
+ * simulation of a full day-by-day football fixture schedule. */
+static const char *const s_months[]={"JAN","FEB","MAR","APR","MAY","JUN",
+                                     "JUL","AUG","SEP","OCT","NOV","DEC"};
+static const char *const s_days[]={"SUN","MON","TUE","WED","THU","FRI","SAT"};
+static int is_leap(unsigned y) {return (y%4==0 && (y%100!=0 || y%400==0));}
+void Expanded_FormatDate(char *out,size_t cap,unsigned season,unsigned week) {
+    unsigned year=season>=1900?season:(season>=90?1900+season:2000+season);
+    if(year<1990 || year>2200) year=1996;
+    unsigned month=7, day=1; /* 1 August, season opening */
+    unsigned dayIndex=week*7;
+    static const unsigned lengths[12]={31,28,31,30,31,30,31,31,30,31,30,31};
+    while(dayIndex) {
+        unsigned remaining=lengths[month]+(month==1&&is_leap(year))-day;
+        if(dayIndex<=remaining) {day+=dayIndex;break;}
+        dayIndex-=remaining+1;
+        day=1;
+        if(++month==12) {month=0;year++;}
+    }
+    /* Tomohiko Sakamoto's weekday algorithm; 0=Sunday. */
+    static const int offsets[]={0,3,2,5,0,3,5,1,4,6,2,4};
+    int y=(int)year-(month<2);
+    unsigned wd=(unsigned)((y+y/4-y/100+y/400+offsets[month]+(int)day)%7);
+    snprintf(out,cap,"%s %02u %s %u",s_days[wd],day,s_months[month],year);
 }
 extern uint8_t _seasonPlaying;
 extern uint8_t _lastFinancialWeekIndex;
@@ -332,18 +379,22 @@
     uint32_t income=sponsors+_expandedTvRights+nonneg(_gateReceipts)+nonneg(_competitionBonuses);
     uint32_t expense=nonneg(_playerWagesBill)+nonneg(_generalRunningCosts);
     rect(20,142,280,2,7);
-    snprintf(t,sizeof t,"SEASON %u/%02u  FINANCE WEEK %u",(unsigned)(1900u+_seasonPlaying),
-             (unsigned)((_seasonPlaying+1)%100), (unsigned)_lastFinancialWeekIndex);
+    Expanded_FormatDate(t,sizeof t,1996u+(unsigned)_currentSeasonShowing,
+                        (unsigned)_lastFinancialWeekIndex);
+    /* Display the exact same date as the calendar; income remains estimates. */
     label(160,145,2,t);
-    label(82,156,7,"INCOME"); label(238,156,7,"EXPENSES");
-    snprintf(t,sizeof t,"SPONSOR %c%lu",c,(unsigned long)(sponsors/12)); label(84,167,7,t);
-    snprintf(t,sizeof t,"WAGES %c%lu",c,(unsigned long)(nonneg(_playerWagesBill)/12)); label(237,167,7,t);
-    snprintf(t,sizeof t,"TV %c%lu",c,(unsigned long)(_expandedTvRights/12)); label(84,178,7,t);
-    snprintf(t,sizeof t,"TRAVEL %c%lu",c,(unsigned long)(nonneg(_generalRunningCosts)/12)); label(237,178,7,t);
-    snprintf(t,sizeof t,"GATE/PRIZE %c%lu",c,(unsigned long)((nonneg(_gateReceipts)+nonneg(_competitionBonuses))/12));label(84,189,7,t);
+    snprintf(t,sizeof t,"WEEKLY: +%c%lu / -%c%lu",c,(unsigned long)(income/52),
+             c,(unsigned long)(expense/52));
+    label(160,155,7,t);
+    label(82,165,7,"INCOME"); label(238,165,7,"EXPENSES");
+    snprintf(t,sizeof t,"SPONSOR %c%lu",c,(unsigned long)(sponsors/12)); label(84,175,7,t);
+    snprintf(t,sizeof t,"WAGES %c%lu",c,(unsigned long)(nonneg(_playerWagesBill)/12)); label(237,175,7,t);
+    snprintf(t,sizeof t,"TV %c%lu",c,(unsigned long)(_expandedTvRights/12)); label(84,185,7,t);
+    snprintf(t,sizeof t,"TRAVEL %c%lu",c,(unsigned long)(nonneg(_generalRunningCosts)/12)); label(237,185,7,t);
+    snprintf(t,sizeof t,"GATE/PRIZE %c%lu",c,(unsigned long)((nonneg(_gateReceipts)+nonneg(_competitionBonuses))/12));label(84,195,7,t);
     snprintf(t,sizeof t,"MONTHLY: +%c%lu / -%c%lu",c,(unsigned long)(income/12),c,(unsigned long)(expense/12));
-    label(160,199,7,t);
+    label(160,205,7,t);
     snprintf(t,sizeof t,"ANNUAL: +%c%lu / -%c%lu",c,(unsigned long)income,c,(unsigned long)expense);
-    label(160,209,7,t);
-    snprintf(t,sizeof t,"BALANCE %c%ld",c,(long)_newBalance);label(160,221,2,t);
-}
+    label(160,215,7,t);
+    snprintf(t,sizeof t,"BALANCE %c%ld",c,(long)_newBalance);label(160,225,2,t);
+}
--- a/src/menu_data.c
+++ b/src/menu_data.c
@@ -1,3 +1,4 @@
+#include <string.h>
 /*
  * Amiga SWOS RE Project
  * SWOS Amiga C/68000 Reversed by Ben Baker (https://github.com/benbaker76)
@@ -5253,9 +5254,9 @@
     { MENU_CMD_TEXT_PTR, 0x0, _expandedAgeText },
     { MENU_CMD_ONSELECT_FIRE, Expanded_EditManagerAge },
     MENU_CMD_END,
-    { { 46, 38, 228, 12 }, { MENU_CMD_COLORS, 0x9 },
+    { { 46, 39, 228, 11 }, { MENU_CMD_COLORS, 0x9 },
       { MENU_CMD_TEXT_PTR, 0, _expandedUnknownHint }, MENU_CMD_END },
-    { { 28, 51, 264, 11 }, { MENU_CMD_COLORS, 0x9 },
+    { { 28, 51, 100, 11 }, { MENU_CMD_COLORS, 0x9 },
       { MENU_CMD_TEXT_PTR, 0, _expandedBoardFansHint }, MENU_CMD_END },
     MENU_MARK_END_MENU
 };
@@ -10818,9 +10819,16 @@
       { MENU_CMD_ONSELECT_FIRE, SetExitMenuFlag }, MENU_CMD_END },
     MENU_MARK_END_MENU
 };
+extern void Expanded_OfferViewSquad(void);
+extern void Expanded_OfferViewLeague(void);
+extern void Expanded_OfferPreviewDraw(void);
+static char s_offerSquad[]="VIEW SQUAD";
+static char s_offerLeague[]="VIEW LEAGUE";
 struct {
     MenuHeader hdr; MenuMark byteCoords;
     MenuEntry_Byte_Colors_TextPtr title;
+    MenuEntry_Byte_Nav_Colors_TextPtr_OnSelect squad;
+    MenuEntry_Byte_Nav_Colors_TextPtr_OnSelect league;
     MenuEntry_Byte_Nav_Colors_TextPtr_OnSelect accept;
     MenuEntry_Byte_Nav_Colors_TextPtr_OnSelect reject;
     MenuEntry_Byte_Nav_Colors_TextPtr_OnSelect back;
@@ -10828,15 +10836,61 @@
 } _expandedClubOfferMenu = {
     { 0, GlobalRetn, Expanded_OfferDraw, 1 }, MENU_MARK_BYTE_COORDS,
     { { 65, 0, 190, 15 }, { MENU_CMD_COLORS, 0x17 }, { MENU_CMD_TEXT_PTR, 0, s_offerTitle }, MENU_CMD_END },
-    { { 27, 181, 120, 16 }, { MENU_CMD_NAV, -1, 2, 3, 3 },
+    { { 30, 126, 120, 16 }, { MENU_CMD_NAV, -1, 2, 5, 3 },
+      { MENU_CMD_COLORS, 0xB }, { MENU_CMD_TEXT_PTR, 0, s_offerSquad },
+      { MENU_CMD_ONSELECT_FIRE, Expanded_OfferViewSquad }, MENU_CMD_END },
+    { { 170, 126, 120, 16 }, { MENU_CMD_NAV, 1, -1, 5, 4 },
+      { MENU_CMD_COLORS, 0xB }, { MENU_CMD_TEXT_PTR, 0, s_offerLeague },
+      { MENU_CMD_ONSELECT_FIRE, Expanded_OfferViewLeague }, MENU_CMD_END },
+    { { 30, 167, 120, 16 }, { MENU_CMD_NAV, -1, 4, 1, 5 },
       { MENU_CMD_COLORS, 0xB }, { MENU_CMD_TEXT_PTR, 0, s_offerAccept },
       { MENU_CMD_ONSELECT_FIRE, Expanded_UnemployedAccept }, MENU_CMD_END },
-    { { -120, 181, 120, 16 }, { MENU_CMD_NAV, 1, -1, 3, 3 },
+    { { 170, 167, 120, 16 }, { MENU_CMD_NAV, 3, -1, 2, 5 },
       { MENU_CMD_COLORS, 0xC }, { MENU_CMD_TEXT_PTR, 0, s_offerReject },
       { MENU_CMD_ONSELECT_FIRE, Expanded_UnemployedReject }, MENU_CMD_END },
-    { { 110, 210, 100, 15 }, { MENU_CMD_NAV, -1, -1, 1, 1 },
+    { { 110, 205, 100, 15 }, { MENU_CMD_NAV, -1, -1, 3, 1 },
       { MENU_CMD_COLORS, 0x9 }, { MENU_CMD_TEXT_PTR, 0, s_offerReturn },
       { MENU_CMD_ONSELECT_FIRE, SetExitMenuFlag }, MENU_CMD_END },
     MENU_MARK_END_MENU
 };
+struct {
+    MenuHeader hdr; MenuMark byteCoords;
+    MenuEntry_Byte_Colors_TextPtr title;
+    MenuEntry_Byte_Nav_Colors_TextPtr_OnSelect back;
+    MenuMark end;
+} _expandedOfferPreviewMenu = {
+    { 0, GlobalRetn, Expanded_OfferPreviewDraw, 1 }, MENU_MARK_BYTE_COORDS,
+    { { 65, 0, 190, 15 }, { MENU_CMD_COLORS, 0x17 },
+      { MENU_CMD_TEXT_PTR, 0, s_offerTitle }, MENU_CMD_END },
+    { { 110, 231, 100, 16 }, { MENU_CMD_NAV, -1, -1, -1, -1 },
+      { MENU_CMD_COLORS, 0xC }, { MENU_CMD_TEXT_PTR, 0, s_offerReturn },
+      { MENU_CMD_ONSELECT_FIRE, SetExitMenuFlag }, MENU_CMD_END },
+    MENU_MARK_END_MENU
+};
 #pragma pack(pop)
+
+/* Expanded 0.10: the unemployed calendar is a *clone of the exact original
+ * contest menu* (including its central 304x116 panel and button positions).
+ * Only callbacks/text differ. The original menu is never modified. */
+static Menu_Contest s_unemployedOriginalCalendar;
+extern void Expanded_UnemployedInit(void);
+extern void Expanded_UnemployedWait(void);
+extern void Expanded_UnemployedShowOffer(void);
+extern void Expanded_UnemployedDraw(void);
+uint8_t *Expanded_UnemployedCalendarPointer(void) {
+    return (uint8_t*)&s_unemployedOriginalCalendar;
+}
+uint8_t *Expanded_UnemployedCalendarMenu(void) {
+    memcpy(&s_unemployedOriginalCalendar,&_contestMenu,sizeof s_unemployedOriginalCalendar);
+    s_unemployedOriginalCalendar.hdr.onInitPtr=Expanded_UnemployedInit;
+    s_unemployedOriginalCalendar.hdr.onReturnPtr=GlobalRetn;
+    s_unemployedOriginalCalendar.hdr.onDrawPtr=0;
+    s_unemployedOriginalCalendar.hdr.defaultEntry=7;
+    s_unemployedOriginalCalendar.entry7_text.textPtr=s_jobWait;
+    s_unemployedOriginalCalendar.entry7_onSelect.funcPtr=Expanded_UnemployedWait;
+    s_unemployedOriginalCalendar.entry10_text.textPtr=s_jobView;
+    s_unemployedOriginalCalendar.entry10_onSelect.funcPtr=Expanded_UnemployedShowOffer;
+    s_unemployedOriginalCalendar.entry9.text.textPtr=s_jobBack;
+    s_unemployedOriginalCalendar.entry9.onSelect.funcPtr=SetExitMenuFlag;
+    return (uint8_t*)&s_unemployedOriginalCalendar;
+}
--- a/src/menu_system.c
+++ b/src/menu_system.c
@@ -1364,6 +1364,16 @@
     BlitterCopyPanelToScreen2();
     DrawMenuPanelBackground();
 
+    /* The packed menu is unpacked again after returning from each facility.
+     * Reapply transparent icon hitboxes on EVERY redraw, not only onInit:
+     * otherwise native blue entry backgrounds reappear over the artwork. */
+    {
+        extern uint8_t _expandedClubOverviewMenu[];
+        extern void Expanded_ClubOverviewInit(void);
+        if (_currentMenuPtr == (uintptr_t)_expandedClubOverviewMenu)
+            Expanded_ClubOverviewInit();
+    }
+
     /* Clear all alreadyDrawn flags */
     count = (int16_t)menu->numEntries;
     entry = (MenuEntry *)_menuEntries;
@@ -1384,6 +1394,28 @@
     for (i = 0; i < count; i++) {
         DrawMenuItem(entry);
         entry = (MenuEntry *)((uint8_t *)entry + MENUENTRY_STRIDE);
+    }
+    /* Reuse the real contest panel while unemployed; only the fixture text
+     * is replaced. It must be drawn AFTER entry18's panel background. */
+    {
+        extern uint8_t *Expanded_UnemployedCalendarPointer(void);
+        extern void Expanded_UnemployedDraw(void);
+        if (_currentMenuPtr == (uintptr_t)Expanded_UnemployedCalendarPointer())
+            Expanded_UnemployedDraw();
+    }
+    /* Real date above the original contest calendar (week advances with the
+     * career's own financial-week clock). */
+    {
+        extern uint8_t _contestMenu[];
+        extern uint8_t _lastFinancialWeekIndex;
+        extern uint16_t _currentSeasonShowing;
+        extern void Expanded_FormatDate(char *,size_t,unsigned,unsigned);
+        if (_currentMenuPtr == (uintptr_t)_contestMenu) {
+            char date[48];
+            Expanded_FormatDate(date,sizeof date,1996u+(unsigned)_currentSeasonShowing,
+                                (unsigned)_lastFinancialWeekIndex);
+            DrawMenuTextCentered(160,18,2,(uint8_t*)date,(uint8_t*)_charTables[0]);
+        }
     }
     /* Expanded hub pictograms are overlaid on their clickable native entries. */
     {
@@ -1427,16 +1459,10 @@
             (uint8_t *)Expanded_CareerProfileCaption(), (uint8_t *)_charTables[0]);
     {
         extern void CareerMenuInit(void);
-        if (menu->onInit == (MenuVoidFn)CareerMenuInit)
-            DrawMenuTextCentered(160, 232, 2,
-                (uint8_t *)"UNKNOWN FAME: START UNEMPLOYED", (uint8_t *)_charTables[0]);
-    }
-    {
-        extern void ClubBusinessInit(void);
-        if (menu->onInit == (MenuVoidFn)ClubBusinessInit)
-            DrawMenuTextCentered(160, 212, 7,
-                (uint8_t *)"PRESS I - INFRASTRUCTURE", (uint8_t *)_charTables[0]);
-    }
+        /* Career status is already in the packed career menu at y=38.
+         * The former duplicate at y=232 covered the EXIT button. */
+    }
+    /* Club Overview has direct facility buttons: no old I shortcut. */
     if (menu->onInit == (MenuVoidFn)SWOSMainMenuInit) {
         /* SWOS Expanded title: preserve the original colourful sprite
          * and use the stock Amiga font for the new subtitle. */
@@ -1744,7 +1770,9 @@
         extern uint8_t _expandedFacilityDetailMenu[];
         extern uint8_t _expandedClubOverviewMenu[];
         extern uint8_t _expandedUnemployedMenu[];
+        extern uint8_t *Expanded_UnemployedCalendarPointer(void);
         extern uint8_t _expandedClubOfferMenu[];
+        extern uint8_t _expandedOfferPreviewMenu[];
         if ((_currentMenuPtr == (uintptr_t)_expandedNationalityPickerMenu ||
              _currentMenuPtr == (uintptr_t)_expandedHallMenu ||
              _currentMenuPtr == (uintptr_t)_expandedInfrastructureMenu ||
@@ -1752,7 +1780,9 @@
              _currentMenuPtr == (uintptr_t)_expandedFacilityDetailMenu ||
              _currentMenuPtr == (uintptr_t)_expandedClubOverviewMenu ||
              _currentMenuPtr == (uintptr_t)_expandedUnemployedMenu ||
-             _currentMenuPtr == (uintptr_t)_expandedClubOfferMenu) &&
+             _currentMenuPtr == (uintptr_t)Expanded_UnemployedCalendarPointer() ||
+             _currentMenuPtr == (uintptr_t)_expandedClubOfferMenu ||
+             _currentMenuPtr == (uintptr_t)_expandedOfferPreviewMenu) &&
             (_currentKey == 27 || _currentKey == 127 || _currentKey == 8)) {
             SetExitMenuFlag();
             return;
@@ -2067,6 +2097,8 @@
         extern uint8_t _expandedClubOverviewMenu[];
         extern uint8_t _expandedUnemployedMenu[];
         extern uint8_t _expandedClubOfferMenu[];
+        extern uint8_t _expandedOfferPreviewMenu[];
+        extern uint8_t *Expanded_UnemployedCalendarPointer(void);
         extern uint8_t _swosMainMenu[];
         if (packedMenu == _expandedNationalityPickerMenu ||
             packedMenu == _expandedHallMenu ||
@@ -2075,23 +2107,16 @@
             packedMenu == _expandedFacilityDetailMenu ||
             packedMenu == _expandedClubOverviewMenu ||
             packedMenu == _expandedUnemployedMenu ||
-            packedMenu == _expandedClubOfferMenu) {
+            packedMenu == Expanded_UnemployedCalendarPointer() ||
+            packedMenu == _expandedClubOfferMenu ||
+            packedMenu == _expandedOfferPreviewMenu) {
             if (oldPackedMenu != 0) {
                 if (oldPackedMenu == (uintptr_t)_swosMainMenu) {
                     extern void CommonMenuExit(void);
                     CommonMenuExit();
-                    /* Do not resurrect the extra multiplayer/empty bars when
-                     * returning from Hall of Fame. Preserve Continue/Save if
-                     * a competition is already active. */
-                    if (packedMenu == _expandedHallMenu) {
-                        extern uint16_t _gameType;
-                        _menuEntries[14].isInvisible = 1; /* MULTIPLAYER */
-                        if (_gameType == 0) {
-                            _menuEntries[11].isInvisible = 1;
-                            _menuEntries[12].isInvisible = 1;
-                            _menuEntries[13].isInvisible = 1;
-                        }
-                    }
+                    /* Restore exactly the main-menu state prepared by its
+                     * own initializer; hiding ordinal 11..14 here used to
+                     * erase LOAD OLD COMPETITION after Hall of Fame. */
                 }
                 DrawMenu();
             }
--- a/src/menus.c
+++ b/src/menus.c
@@ -5075,6 +5075,7 @@
     _menuEntries[10].fg = (uintptr_t)(_expandedManagerFame == 0 ?
                                       _expandedCareerStartButton : _aChooseTeam);
     _menuEntries[16].isInvisible = (_expandedManagerFame != 0);
+    _menuEntries[17].isInvisible = 1; /* obsolete FINANCES/FANS hint */
 }
 
 
--- a/src/save_format_json.c
+++ b/src/save_format_json.c
@@ -1090,6 +1090,7 @@
 extern uint32_t _expandedShirtSponsor, _expandedStadiumSponsor, _expandedTvRights;
 extern uint8_t _expandedBoardMood, _expandedFanMood;
 extern uint32_t _expandedSupporters;
+extern uint8_t _expandedLowConfidenceMatches;
 static const FieldDesc s_career_scalars[] = {
     FX("expanded_economy_club_id", _expandedEconomyClubId, FLD_U16),
     FX("expanded_stadium_level", _expandedStadiumLevel, FLD_U8),
@@ -1110,6 +1111,7 @@
     FX("expanded_board_mood", _expandedBoardMood, FLD_U8),
     FX("expanded_fan_mood", _expandedFanMood, FLD_U8),
     FX("expanded_supporters", _expandedSupporters, FLD_U32),
+    FX("expanded_low_confidence_matches", _expandedLowConfidenceMatches, FLD_U8),
     FX("expanded_manager_fame", _expandedManagerFame, FLD_U8),
     FX("expanded_manager_start_age", _expandedManagerStartAge, FLD_U8),
     FX("expanded_annual_salary", _expandedAnnualSalary, FLD_U32),

```

---

## STORICO_0102_IT.txt

```text
SWOS EXPANDED 0.10.2 - CORREZIONI MENU E STABILITA'
================================================

PER AVVIARE SU WINDOWS 64 BIT
1) Estrai l'intero ZIP in una cartella NUOVA (non sovrascrivere la 0.10.1B).
2) Apri la cartella SWOS_Expanded_0_10_2.
3) Fai doppio clic su AVVIA_SWOS.bat.
4) La prima volta, il BAT compila SWOS.exe con MSYS2 MINGW64.
   Sul tuo PC MSYS2 e' gia' installato: non devi applicare patch.
5) Gli avvii successivi usano l'eseguibile compilato.

ATTENZIONE: nello ZIP NON c'e' un nuovo SWOS.exe gia' compilato.
Il BAT lo genera sul PC Windows. Non confondere questo pacchetto con
una versione Windows gia' testata in esecuzione.

CORREZIONI 0.10.2
- View Squad e View League: prevenuta la barra grigia a tutto schermo
  dovuta al pulsante BACK fuori dai limiti e a un underflow unsigned
  nel disegno di rettangoli SDL.
- Preview offerte: testi e controlli disegnati nell'ordine corretto,
  con pulsante BACK visibile.
- Club Business: caselle selezionabili allineate ai disegni,
  finanze sotto le infrastrutture e pulsante BACK visibile.
- Disoccupato: BACK torna alla schermata di creazione carriera,
  senza saltare al menu principale.
- Nazionalita': corretta la causa dei nomi sbagliati: la tabella
  delle squadre (TEAM.xxx) non usa gli stessi indici delle nazionalita'.
  Ora il menu usa nomi canonici per tutti i 68 codici e salva il codice
  associato alla riga realmente selezionata.
- Incluso stdlib.h mancante in menus.c per compilatori GCC recenti.

TEST ESEGUITI QUI SUI SORGENTI
- Controllo sintattico GCC sui sette file modificati (SDL stub locale).
- Test automatico delle offerte (test11_offer).
- Test del clipping rettangoli SDL (test12_rect_clip).
- Test geometria menu (test12_ui_layout).
- Test 68 nazionalita' (test09_nationality).
- Test layout finanze (test08_club).
- Test struct Continue (test0101b_continue_layout).

DA VERIFICARE SUL PC
- Compilazione completa MSYS2 e avvio Windows.
- View Squad / View League, accettazione offerta, ritorno dal disoccupato,
  Club Business e nazionalita' nei diversi mondi.
- Il sistema di disoccupazione NON simula ancora tutti i risultati e le
  classifiche prima dell'assunzione: questo rimane un lavoro separato.

SE LA COMPILAZIONE FALLISCE: invia COMPILAZIONE_0102.log.
SE IL GIOCO SI BLOCCA: invia SWOS.log da Documenti\SWOS.

I sorgenti completi sono in sorgenti/; non serve applicare file .diff.

```

---

## STORICO_0105_IT.txt

```text
SWOS EXPANDED 0.10.5 - QUATTRO MENU PIXEL ART
=============================================

AVVIO (WINDOWS x64)
1. Estrai TUTTO lo ZIP in una nuova cartella, senza sovrascrivere la 0.10.4.
2. Apri la cartella SWOS_Expanded_0_10_5.
3. Fai doppio clic su AVVIA_SWOS.bat.
4. Il primo avvio compila automaticamente SWOS.exe usando MSYS2 MINGW64.
   Nelle versioni successive avvii con lo stesso BAT senza ricompilare.
5. Non occorre applicare patch o usare comandi a mano.

NOVITA' 0.10.5
- Titolo: logo SENSIBLE WORLD OF SOCCER EXPANDED completo, allenatore
  sul campo e tutti i comandi originali, compresi quelli contestuali.
- Creazione carriera: nuova scena con l'allenatore che arriva al club,
  mantenendo MR/MS, nome, cognome, nazionalita', ruolo, eta', fama,
  scelta squadra/disoccupazione, indietro e le opzioni originali.
- Calendario: immagine della partita sullo sfondo, elenco partite originale
  (con scroll), data nella barra superiore e tre colonne di comandi.
  La stessa grafica si applica al calendario da disoccupato.
- Club Business: Back in alto a sinistra, piu' spazio per pannelli e
  informazioni, sette facility cliccabili, upgrade e cinque immagini
  dello stadio legate al livello reale del club.
- Gli altri menu e il motore delle partite rimangono quelli della 0.10.4.

NOTE IMPORTANTI
- Questa versione aggiorna GRAFICA e posizioni dei comandi, non il motore
  economico o la simulazione completa delle competizioni durante la
  disoccupazione. I dati non ancora simulati non sono stati inventati.
- Lo ZIP contiene i sorgenti completi e i dati grafici, NON un nuovo EXE
  Windows precompilato. Il BAT lo crea sul tuo PC; qui non ho potuto
  eseguire la compilazione e il test interattivo su Windows.
- Se MSYS2 e' gia' installato, il BAT lo riutilizza automaticamente.
- I file data/expanded/*.raw sono necessari al gioco: non spostarli.
- Le immagini PREVIEW_0105_*.png sono solo anteprime statiche illustrative.

VERIFICHE EFFETTUATE
- Controllo sintattico GCC delle modifiche a menu, framebuffer e dati.
- Test statici: quattro artwork 320x256, nove hitbox Club Business,
  cinque stadi diversi e callback originali conservati.
- Test framebuffer C con ASan/UBSan: rendering e selezione nei quattro menu,
  upgrade stadio e controllo dei limiti del buffer: PASS.
- Non e' un test end-to-end del gioco su Windows.

IN CASO DI PROBLEMI
- Se la compilazione fallisce: invia COMPILAZIONE_0105.log.
- Se il gioco si blocca: invia SWOS.log da Documenti\SWOS.
- Se una selezione non corrisponde al pulsante, manda uno screenshot
  e indica se usavi mouse, tastiera o joypad.

```

---

## TEST_REPORT_0106.txt

```text
SWOS Expanded 0.10.6 - REPORT VERIFICHE
=======================================

Verifiche eseguite in questo ambiente:
- GCC -fsyntax-only (expanded_economy.c, menu_data.c, menu_system.c,
  expanded_career.c, career_init.c, blitter.c, build_info.c): PASS
- test16_menu_callbacks.py: PASS (16 callback originali in ordine)
- test15_four_menus.py: PASS
- test14_club_art.py: PASS
- test12_ui_layout.py: PASS
- tests/run_test11_offer.sh: PASS
- test14_club_art.c con AddressSanitizer + UndefinedBehaviorSanitizer: PASS
- Immagini indicizzate 320x256 e 5 stadi 154x88: PASS
- Frame nuovo menu carriera dentro 320x256: PASS
- Navigazione titolo: collega soltanto voci visibili/abilitate; duplicate
  Hall of Fame nascosta; Load Old Competition resta accessibile.

NON ESEGUITI:
- Compilazione completa con MSYS2 Windows x64 e test end-to-end interattivo.
  La prima compilazione avviene tramite AVVIA_SWOS.bat sul PC Windows.

Integrita' dei dati conservati dalla versione precedente:
sorgenti/src/menu_data.c: IDENTICO alla 0.10.5 (SHA256 05c4098cb8ea25ab)
sorgenti/src/menus.c: IDENTICO alla 0.10.5 (SHA256 04663e09d5170969)
data/expanded/club_business_bg.raw: IDENTICO alla 0.10.5 (SHA256 a559774cce388aa1)
data/expanded/club_stadium_1.raw: IDENTICO alla 0.10.5 (SHA256 7c704783c435ab7e)
data/expanded/club_stadium_2.raw: IDENTICO alla 0.10.5 (SHA256 3f66a4e742a2a000)
data/expanded/club_stadium_3.raw: IDENTICO alla 0.10.5 (SHA256 2e33435ba299f9ac)
data/expanded/club_stadium_4.raw: IDENTICO alla 0.10.5 (SHA256 f3f53092f8669eec)
data/expanded/club_stadium_5.raw: IDENTICO alla 0.10.5 (SHA256 c00ba1ddbc505331)

```

---

## TEST_REPORT_0107.txt

```text
SWOS Expanded 0.10.7 - VERIFICHE E LIMITI
=======================================

PASS - GCC -fsyntax-only: expanded_economy.c, menu_system.c, menu_data.c,
       expanded_career.c, career_init.c, blitter.c, build_info.c.
PASS - test14_club_art.c compilato con AddressSanitizer + UBSan:
       titolo con righe dinamiche, focus, aggiornamento parziale,
       carriera con AGE (18-70), calendario, memoria framebuffer e
       stadio dinamico/upgrade.
PASS - test15_four_menus.py: asset indicizzati, menu e callback.
PASS - test12_ui_layout.py: Club Business e navigazione Back.
PASS - test16_menu_callbacks.py: 16 callback originali, Hall of Fame.
PASS - test17_compact_menus.py: nuovo layout e font originale.
PASS - tests/run_test11_offer.sh: offerte e menu disoccupato.
PASS - asset club_business_bg.raw e club_stadium_1..5.raw identici alla 0.10.6.
PASS - src/menu_data.c e src/menus.c identici alla 0.10.6.

NON VERIFICATI IN QUESTO AMBIENTE:
- Compilazione completa di SWOS.exe con MSYS2 Windows x64.
- Test interattivo completo di tutte le transizioni di menu sul PC Windows.

AVVIA_SWOS.bat esegue la compilazione Windows al primo avvio. Non e' incluso
un nuovo EXE gia' compilato; non e' presente un vecchio EXE mascherato.

```

---

## TEST_REPORT_0108.txt

```text
SWOS Expanded 0.10.8 - VERIFICHE
================================

PASS - GCC -fsyntax-only sui moduli modificati:
       menu_data.c, menu_system.c, expanded_career.c, expanded_economy.c,
       career_init.c, input_text.c, blitter.c, build_info.c.
PASS - test18_ui.c con AddressSanitizer/UBSan:
       navigazione verticale della carriera, selezione titolo, calendario,
       framebuffer, protezioni di memoria e cinque stadi dinamici.
PASS - test18_menu_fixes.py:
       nome/cognome editabili, clipping, menu disoccupato, font business,
       asset originali e 16 callback invariati.
PASS - test17_compact_menus.py / test16_menu_callbacks.py:
       16 funzioni del menu titolo e Hall of Fame.
PASS - test15_four_menus.py / test12_ui_layout.py:
       hitbox Club Business e ritorno Back.
PASS - tests/run_test11_offer.sh:
       rifiuto e nuova offerta, accettazione, anteprime, navigazione;
       pulsanti inutilizzati realmente nascosti.
PASS - Nuova grafica indexed 320x256 con palette SWOS, tutti gli asset presenti.
PASS - Club Business e cinque immagini dello stadio identici alla 0.10.7.

NON VERIFICATI:
- Build completa Windows x64 con MSYS2 in questo ambiente.
- Test manuale interattivo Windows (inserimento nome, focus, finanze,
  ritorno da offerte e caricamento salvataggi).

Il pacchetto contiene sorgenti + BAT, NON un EXE precompilato.

```

---

## TEST_REPORT_0109.txt

```text
SWOS EXPANDED 0.10.9 - VERIFICHE
================================
PASS GCC -fsyntax-only: menu_system, expanded_economy, expanded_career,
     menu_data, career_init, input_text, blitter, build_info.
PASS test19_ui.c ASan/UBSan: faccine e percentuali, finanze senza EUR,
     settimana, campi calendario, clip barra blu, stati originali dei tasti,
     navigazione carriera, protezioni framebuffer e stadi.
PASS test18_menu_fixes.py: clipping sui veri ordinal 39/43,
     file originali dei callback, risorse e asset stadio.
PASS test17_compact_menus.py, test16_menu_callbacks.py,
     test15_four_menus.py, test14_club_art.py, test12_ui_layout.py.
PASS tests/run_test11_offer.sh: disoccupazione e offerte.
NOT TESTED: Windows x64 full build + input/gameplay in MSYS2/Windows.
Package is source + data + BAT, not a prebuilt executable.

```

---

## TEST_REPORT_0110.txt

```text
SWOS EXPANDED 0.10.10 - TEST REPORT
=================================

PASS - GCC -fsyntax-only (SDL build):
  menu_data.c, menu_system.c, menus.c, expanded_career.c,
  expanded_economy.c, career_init.c, input_text.c,
  blitter.c, build_info.c.
  NOTE: SDL.h is not installed in this Linux environment. menus.c
  was checked with a minimal SDL typedef shim; the other sources
  were checked with the normal game headers.

PASS - test19_ui.c (UBSan):
  compact single-line fixture rows, D. THREE, L. CUP R2 L1,
  team text truncation, bounded highlight, arrows inside panel,
  original disabled button states, finance week, mood icons,
  career navigation, framebuffer canary, 5 stadium variants.

PASS - test11_offer.c (UBSan):
  unemployed weeks, offers, team selection, accept/reoffer.

PASS - tests/test12_ui_layout.py
PASS - tests/test14_club_art.py
PASS - tests/test15_four_menus.py
PASS - tests/test16_menu_callbacks.py
PASS - tests/test17_compact_menus.py
PASS - tests/test18_menu_fixes.py

PASS - byte-for-byte unchanged title art, stadium art and
  packed menu_data.c callbacks vs 0.10.9.
PASS - all 320x256 indexed background assets regenerated and checked.

NOT TESTED - full MinGW Windows EXE build / interactive gameplay.
  The ZIP includes the complete game assets, source and BAT; the BAT
  builds SWOS.exe on the user's Windows machine using MSYS2.

ASan not available in this sandbox (shadow-memory reservation fails).
UBSan and framebuffer guard tests were run instead.

```

---

## TEST_REPORT_0111.txt

```text
SWOS EXPANDED 0.10.11 - TEST REPORT
==================================

PASS - GCC -fsyntax-only, original SDL build headers:
  menu_data.c, menu_system.c, expanded_career.c, expanded_economy.c,
  career_init.c, input_text.c, blitter.c, build_info.c.
PASS - GCC -fsyntax-only menus.c (minimal SDL typedef shim).
  One pre-existing packed-member alignment warning remains in menus.c.
PASS - test19_ui.c under GCC UBSan:
  club mood and stadiums, facility upgrade, original title links,
  manager fields and navigation, calendar compact fixtures, date and
  button visibility, framebuffer guard bytes.
PASS - test11_offer.c under GCC UBSan:
  unemployed progression, team offers, previews, accept/reject.
PASS - test12, test14, test15, test16, test17, test18 Python regression tests.
PASS - test21_0111_layout.py: grey gutter eliminated, panels centered,
  manager field positions and COSTS header, original upgrade button.
PASS - 320x256 indexed artwork regenerated reproducibly from the
  supplied artwork source and game palette.
PASS - unchanged original packed menu_data.c and title/stadium assets.

NOT TESTED - full Windows MinGW64 build or interactive Windows execution.
The included BAT performs the full build using MSYS2 on the user's PC.

```

---

## TEST_REPORT_0113.txt

```text
SWOS EXPANDED 0.10.13 - TEST REPORT
==================================

PASS - GCC syntax check of 9 edited/relevant C modules (SDL platform shim).
       menus.c retains one existing packed-member alignment warning.
PASS - test19_ui.c under GCC UndefinedBehaviorSanitizer:
       panel height for 8 and 14 fixtures, actual row count adjustment,
       scroll-arrow position, no stale frame after scrolling, framebuffer guard.
PASS - test23_calendar_full.py: native 14-row limit removed only for the
       pixel-art contest calendar; no changes to fixture data or callbacks.
PASS - test22_career_input.c: Enter commits NAME/SURNAME without reopening.
PASS - Python menu/art regression tests 12,14,15,16,17,18,21,23.
PASS - Title, Career, Business, Unemployed and Calendar art byte-for-byte
       identical to 0.10.12. Original menu_data.c and input_text.c unchanged.
PASS - BAT points to 0.10.13 and creates a fresh SWOS.exe on first launch.

NOT TESTED - full Windows MinGW64 build and interactive gameplay on Windows.
             AVVIA_SWOS.bat compiles on the user's MSYS2 Windows installation.

```

---

## TEST_REPORT_0114.txt

```text
SWOS EXPANDED 0.10.14 - TEST REPORT
=================================
PASS - GCC syntax check (menu_data.c, menu_system.c, expanded_career.c,
       expanded_economy.c, career_init.c, input_text.c, blitter.c, build_info.c).
PASS - test11_offer.c ASan/UBSan: valid club, salary range, plus/minus,
       manager fame, exactly five rejected demands, next offer.
PASS - test19_ui.c UBSan: background blit, interactive offer buttons,
       salary selector, original calendar layout, frame guard.
PASS - Python tests 12,14,15,16,17,18,21,23.
PASS - Indexed offer art is exactly 320x256 bytes; original art unchanged.
PASS - BAT/SH script references 0.10.14 and includes new required asset.
NOT TESTED - Full Windows MinGW64 build and live gameplay.
LIMITATION - Pre-hire VIEW COMPETITIONS is read-only preview; the live
             competition viewer requires career season initialization.

```

---

## TEST_REPORT_0115.txt

```text
SWOS EXPANDED 0.10.15 - TEST REPORT
=================================
PASS - GCC syntax checks: menu_data.c, menu_system.c, expanded_career.c,
       expanded_economy.c, career_init.c, input_text.c, blitter.c,
       build_info.c (Linux host; SDL-dependent menus.c excluded).
PASS - test11_offer.c with AddressSanitizer/UBSan: playable team,
       negotiations +/- unchanged, fame-dependent chance, 5 rejections,
       accepted salary stored as annual amount, squad/league previews.
PASS - test19_ui.c with UBSan: annual salary caption, new absolute
       button coordinates, all seven actions inside 320x256, focus border,
       salary selector, calendar and club-business regression checks.
PASS - Python tests 12,14,15,16,17,18,21,23.
PASS - Original first office/contract concept converted to a full-screen
       320x256 indexed asset (81,920 bytes); 108 different palette indices
       in the lower half (no solid-blue bottom or sidebars).
PASS - AVVIA_SWOS.bat / PREPARA_SWOS.bat / BUILD_SWOS.sh all stamped 0.10.15.
NOT TESTED - Complete Windows MinGW64 compilation or live gameplay.
NOTE - The source-only BAT distribution compiles SWOS.exe on first launch.
NOTE - Pre-hire competition preview remains read-only as in 0.10.14.

```

---

## TEST_REPORT_0116.txt

```text
SWOS Expanded 0.10.16 - Test report
- GCC -fsyntax-only: expanded_economy, expanded_career, menu_data,
  menus, career_status, save_format_json (SDL header stub for menus.c).
- Test C: five fan tiers, board penalties from raises and finances,
  counteroffer possible for UNKNOWN, large counteroffer impossible.
- Assets: 320x256 welcome indexed 81920 bytes, 292x136 sacked indexed 39712 bytes.
- Native Windows runtime/visual test: NOT PERFORMED (requires MSYS2/SDL2 on Windows).
- Known: mid-season rehire safely resets the competition season.

```

---

## TEST_REPORT_0117.txt

```text
SWOS Expanded 0.10.17 - Test report
- GCC syntax-only (Linux + SDL stub): PASS (expanded_career, expanded_economy, career_status, menus).
- Assets 320x256: title, career, calendar, unemployed, offer, welcome, record.
- Sacked popup 292x136, native indexed palette.
- Original club names neutralized in static offer, welcome and sacked images.
- Windows end-to-end build and interactive game testing: NOT PERFORMED (requires MSYS2 on Windows).
- Transfer callback original retained, new-season flag reset verified in source.
- Demo dismissal: first welcome fans=19 board=20, first PLAY MATCH/NEXT MATCH triggers popup.
- C unit tests (linked and run): PASS five fame tiers/contract model; PASS one-shot 39-point dismissal.

```

---

## TEST_REPORT_0118.txt

```text
SWOS Expanded 0.10.18 - QA report
GCC syntax-only with SDL stub (Linux): PASS
  expanded_economy.c expanded_career.c menus.c menu_system.c menu_data.c career_status.c
C unit test role-weighted stars: PASS
Python JSON validation / Leyton Orient Ian Hendon: PASS
C liking / negotiation acceptance unit test: PASS
C first-sacking demo one-shot test: PASS
C record persistence on sacking: PASS
Headless indexed UI 320x256: PASS
  welcome, fullscreen sacked, squad star pixels, selected-row tooltip, framebuffer canary
Legacy static menu/calendar checks: PASS
Windows MSYS2 x64 full compilation: NOT RUN (toolchain unavailable in this environment)
Live joystick/mouse/transfer/gameplay testing: NOT RUN (requires game on Windows)
One-shot 39-point sacking test: STILL ENABLED as requested.

```

---

## TEST_REPORT_0119.txt

```text
SWOS EXPANDED 0.10.19 — TEST REPORT

PASS: syntax GCC for expanded_economy.c, menu_data.c, menu_system.c,
      menus.c, career_status.c, career_teams.c, transfer_market.c,
      save_format_json.c (no errors; 20 existing packed-struct warnings).
PASS: test_expanded_facilities_0119.c: all eight progressive weekly costs,
      medical 10-50%, away fatigue, fitness, training, youth, last-five form.
PASS: test_expanded_ui_0119.c headless 320x256 framebuffer:
      calendar arrow placement, 14/8/5-row panel, menu geometry,
      no framebuffer guard corruption, squad row selection alignment,
      GOALS/BACK mode hitbox, sponsor offers week 2, TV week 4,
      one-time signing, staff cash-flow, scouting 1/2/3/5 reach,
      last-five result and away penalty.
PASS: source callback audit for original SquadCommon swap/SelectGoals,
      original transfer market callbacks, sponsor and TV menus, save fields.

NOT TESTED: full Windows x64 link and interactive keyboard/mouse/joypad
on the target PC (MSYS2/SDL2 Windows environment unavailable here).

```

---

## TEST_REPORT_0120.txt

```text
SWOS EXPANDED 0.10.20 — TEST REPORT

PASS: GCC syntax check of expanded_economy.c, menu_system.c,
      save_format_json.c, transfer_market.c, career_status.c, menus.c.
      Only pre-existing packed-struct warnings.
PASS: test_expanded_condition_0120.c — form 0..100, default 50,
      performance factor 85%, clamping, fatigue, recovery and popup
      positioning for all 17 player rows (no overlap with selected row).
PASS: test_expanded_ui_0120.c — 320x256 framebuffer guard, selected-row
      white SIDE marker (no horizontal frame), popup below row, orange-red
      swap highlight, original GOALS/BACK hitbox, sponsors/TV, finance,
      calendar and scouting checks.
PASS: source audit — original SquadCommon swap, SelectGoals,
      SelectTraining and Club Business finance toggle callbacks retained;
      all new condition data persisted as career_scalars.
NOT TESTED: full Windows executable link and interactive mouse/joypad
      on user's Windows PC (MSYS2 and SDL2 Windows environment unavailable).
NOT INTEGRATED YET: the new condition-based performance factor in match sim.

```

---

## TEST_REPORT_0121.txt

```text
SWOS EXPANDED 0.10.21 — TEST REPORT

PASS: GCC -fsyntax-only expanded_economy.c, save_format_json.c,
      menu_system.c, transfer_market.c, career_status.c.
      Only pre-existing packed-struct address warnings.
PASS: test_expanded_condition_0121.c — player form 0..100, 50 default,
      performance scaling, tooltip never covers focused row across 17 slots.
PASS: test_expanded_facilities_0119.c — facility wages, recovery,
      fatigue, scouting/training progression.
PASS: test_expanded_ui_0121.c — native framebuffer 320x256 guards,
      business button geometry, staff hover opaque panel, all 8 STAFF labels,
      TRAVEL cost in submenu, finance week 56->01, sponsor delayed until
      season week 2, season-relative TV, 52-week forecast, squad tooltip,
      blue focus alignment, orange-red pending swap, stadium side gutters,
      away travel charged to balance and season ledger, home travel zero.
PASS: JSON save scalar descriptors include new travel expense values.

NOT TESTED: full Windows executable link and interactive mouse/joypad on
            the destination PC (MSYS2 and SDL2 Windows SDK unavailable).
NOT CHANGED: original match simulator and pending condition-based performance
             integration from 0.10.20.

```

---

## TEST_REPORT_0122.txt

```text
SWOS EXPANDED 0.10.22 — TEST REPORT

PASS  GCC -fsyntax-only -DSWOS_SDL_BUILD=1:
      expanded_economy.c, menu_data.c, menu_system.c, transfer_market.c,
      career_status.c, save_format_json.c, build_info.c.
      Solo warning preesistenti di membri packed (vedi log).
PASS  test_expanded_ui_0122.c: 320x256 framebuffer guard, YEAR, bottone
      dentro FINANCES, focus-only STAFF, hover senza FIRE, popup opaco
      e voci complete, OTHER, AWAY, grafo direzionale joypad, riquadro
      FORM, EXIT nella rosa e nella vista GOALS, popup giocatore,
      scambio, offerte sponsor/TV, costi trasferta casa/fuori.
PASS  test_expanded_nav_0122.py: entry STAFF packed senza callback FIRE;
      toggle FINANCES, BACK, GOALS e EXIT mantengono callback originali.
PASS  test_expanded_squad_json_0118.py: skill reali e art 320x256.
PASS  test_expanded_facilities_0119.c: personale delle 8 strutture,
      progressione dei costi, forma, recupero, allenamento e trasporti.
PASS  test_expanded_condition_0121.c: forma giocatori 0..100, popup
      senza sovrapposizioni, recupero e fatica.
PASS  Dimensioni art e dati SWOS originali; asset squad indicizzato
      aggiornato soltanto nella striscia sotto la nazionalita'.

NON VERIFICATO IN QUESTO AMBIENTE:
- Link completo di SWOS.exe per Windows (MSYS2/SDL2 Windows non disponibili).
- Prova interattiva con un joypad reale e un mouse sul PC Windows.

ESECUZIONE SUL PC DELL'UTENTE:
- AVVIA_SWOS.bat -> PREPARA_SWOS.bat -> BUILD_SWOS.sh -> SWOS.exe.
- In caso di errore: COMPILAZIONE_0122.log.

```

---

## TEST_REPORT_0123.txt

```text
SWOS EXPANDED 0.10.23 - REPORT DEI TEST
======================================

PASS  gcc -std=gnu11 -fsyntax-only (SWOS_SDL_BUILD=1):
      expanded_economy.c, menu_data.c, menu_system.c, build_info.c.
PASS  test_expanded_ui_0123.c: rendering e limiti framebuffer 320x256,
      WEEK/YEAR dentro il frame, font piccolo, hover STAFF e joypad senza FIRE,
      popup opaco largo meno di 210 pixel, distanza voce/importo 3px,
      TRAVEL, contabilita' settimanale e annuale, rosa/EXIT e sponsor/TV.
PASS  test_expanded_nav_0123.py: menu packed, callback originali,
      navigazione STAFF, calendario (frecce e hitbox), profilo 4K stretch.
PASS  test_expanded_facilities_0119.c: costi di tutte le strutture,
      allenamento, recupero, personale e trasporti.
PASS  test_expanded_condition_0121.c: condizione e popup squadra.
PASS  test_expanded_squad_json_0118.py: rosa e valori originali.
PASS  Integrita' file asset e dimensioni artwork originale.

NON VERIFICATO IN QUESTO AMBIENTE:
- Compilazione/link Windows MSYS2 e prova con joypad reale.
- Rendering reale sul monitor 3840x2160 dell'utente.

ESECUZIONE WINDOWS:
- AVVIA_SWOS.bat -> PREPARA_SWOS.bat -> BUILD_SWOS.sh -> SWOS.exe.
- Log errori: COMPILAZIONE_0123.log.

```

---

## TEST_REPORT_0124.txt

```text
SWOS EXPANDED 0.10.24 - REPORT TEST
==================================
PASS: test_expanded_ui_0124.c (native WEEK/YEAR, 33x12 hitbox,
      STAFF hover, focus joypad, FINANCES, TRAVEL, popup, SQUAD EXIT,
      squad form, sponsor/TV, costi).
PASS: test_expanded_nav_0124.py (callback originali, frecce calendario,
      full-screen menu art crop, mouse coordinate mapping, reset match).
PASS: test_expanded_squad_json_0118.py (rosa originale e navigazione).
PASS: GCC -std=gnu11 -fsyntax-only (core C senza SDL disponibile qui):
      expanded_economy.c, menu_system.c, game_loop.c, build_info.c.
PASS: bash -n BUILD_SWOS.sh; verifica dimensioni e hash asset originali.
PASS: test di integrita' ZIP.

NON VERIFICATO IN QUESTO AMBIENTE:
- Compilazione/link completo Windows MSYS2 (libreria di sviluppo SDL2
  non presente nel container); BAT Windows prepara le dipendenze.
- Presentazione interattiva reale su monitor 3840x2160 e joypad.

AVVIA_SWOS.bat -> PREPARA_SWOS.bat -> BUILD_SWOS.sh -> SWOS.exe
Errori compilazione: COMPILAZIONE_0124.log

```