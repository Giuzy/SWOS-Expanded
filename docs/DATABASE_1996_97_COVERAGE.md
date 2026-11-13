# SWOS Expanded — Database 1996/97 e copertura reale dei test

**Verifica del catalogo nel repository `dev/0.10`: 13 novembre 2026.**

## Conteggi verificati (non sommabili senza distinguere le copie)

- **76** file JSON in `data_json/teams/`.
- **1.620** record squadra in file di tipo `team`: comprendono club di campionato, squadre fuori campionato, nazionali e CUSTOM.
- **25.920** record giocatore nelle rose delle 1.620 squadre: 16 slot per squadra.
- **900** record giocatore nel catalogo separato `POOLPLYR.json`.
- **26.820** *posizioni giocatore nei cataloghi* contando i 25.920 slot di rosa e le 900 posizioni POOLPLYR. Non prova 26.820 persone distinte.
- **80** record squadra supplementari nei tre cataloghi di iscritti alle coppe europee: `EUROCUP.json` (16), `EUROCWC.json` (32), `UEFACUP.json` (32). **Sono copie di partecipanti, non 80 nuove squadre**; le relative 1.280 posizioni giocatore non si sommano ai 26.820.
- **72** file nativi `data/TEAM.*` osservati nel repository. Il file `POOLPLYR` è separato.
- **1.303 club / 88 leghe**: risultato storico di un import LWE riportato da un test utente, NON conteggio dell'intero DB né garanzia per la build corrente.

## F12 / valori runtime

La pagina F12 deve distinguere:
- **OG LOADED FILES / OG LOADED TEAMS / PLAYERS**: file TEAM caricati correttamente almeno una volta dal loader OG dal lancio del processo e somma dei loro record (16 slot per squadra). Non sono i file contemporaneamente residenti, non sono giocatori unici, e non includono i 900 POOLPLYR.
- **LWE CLUBS / PLAYER SLOTS**: club effettivamente importati nel Living World e 16 slot per club, non entità complete dei calciatori.
- **Totale DB**: i numeri statici sopra, mai confonderli con quelli runtime. Una squadra fuori campionato o una nazionale può essere disponibile in SWOS senza essere ancora importata nel calendario LWE.

## Test: non confondere tre livelli

1. **Audit DB**: legge e valida i 76 cataloghi, tutte le 1.620 squadre e i 26.820 slot giocatore, e verifica le copie TMD. Non simula una partita.
2. **Stress LWE domestico 70 stagioni**: verifica risultati e calendario dei gruppi domestici compatibili. Non copre le nazionali e non costituisce prova di tutte le coppe OG.
3. **End-to-end OG**: resta da dimostrare per tutti i paesi, coppe nazionali, continentali, nazionali, qualificazioni, stagioni durante disoccupazione, assunzione, riassunzione, salvataggio e caricamento. L'hook OG per le coppe domestiche a fine stagione è un'integrazione parziale, **non** una dimostrazione che questi test passino.

I numeri in questa guida sono conteggi di record nelle sorgenti disponibili, non misure delle entità attive contemporaneamente in memoria. Per la versione DB 2026/27 occorrerà un audit separato.


## F10 — Diagnostica import OG/LWE (0.13.37_04)

Premere **F10 nei menu della carriera**, anche durante la disoccupazione, per aprire la pagina di sola lettura; F10 la richiude. F9 salva lo screenshot. F10 **durante le partite Arcade** conserva l'azione debug preesistente, non apre questa pagina.

La pagina separa:
- **Campionati:** paesi candidati e supportati, paesi senza record nativo adatto, TEAM file effettivamente caricati, righe analizzate, club LWE importati e divisioni schedulate.
- **Club esclusi:** quattro cause misurate per ciascun record realmente letto: fuori campionato, TEAM file diverso da quello atteso, divisione oltre il numero di divisioni OG, rifiuto dell'importatore LWE. Visualizza fino a 20 esempi con ID in esadecimale, file, divisione, motivo e nome. I dettagli di ogni coppa OG riconosciuta (paese, slot, ID) vengono scritti anche nel log.
- **Coppe:** conteggio record nelle posizioni OG secondaria/terziaria, record World_Cups, specifiche continentali e numero di match/titoli effettivamente testimoniati dal registro OG. **Un record esistente non prova che la coppa sia stata simulata**.
- **Nazionali:** non vengono impropriamente contate come squadre del calendario dei campionati LWE. Qualificazioni e tornei devono essere verificati separatamente.

I conteggi F10 sono relativi **ai TEAM file effettivamente aperti dall'importatore domestico** e non ai 76 cataloghi complessivi. Non sottrarre semplicemente 1.303 da 1.620 per stimare errori: nazionali, squadre fuori campionato, copie TMD, file non letti e rifiuti sono categorie diverse.

La classifica OG ora genera nel log `[join-standings] PASS/FAIL` quando si apre il relativo menu: confronto per ID di squadra con la tabella LWE, comprendente partite, W/D/L, gol fatti/subiti e punti. Il controllo è diagnostico e non corregge i dati. Per campionati OG in formato non gestito mostra SKIP, non PASS.

L'hook delle coppe OG durante la disoccupazione registra ora il numero di chiamate e le variazioni del registro di partite e vincitori e cerca di non ripetere stagioni già finalizzate. **Non dimostra ancora i tabelloni OG turno per turno, né le coppe continentali/nazionali durante l'assenza del manager.** I controlli aggiunti alla suite sono statici; il collaudo runtime Windows resta pendente.
