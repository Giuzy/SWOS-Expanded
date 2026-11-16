# SWOS Expanded — Coppe OG, CT_S1 e OpenSWOS (16 novembre 2026)

## Fonti confrontate e gerarchia

- SWOS Expanded `dev/0.10`, HEAD iniziale di questo intervento `b662927a9dc93fd6ccf0246e5c80fda9f7003992`, versione 0.13.59.
- OpenSWOS pubblico `benbaker76/OpenSWOS`, branch `main`: `src/career_init.c`, `src/career_eurocups.c`, `src/league_phase.c`, `src/world_conti.c`, `src/conti_season.c`, `docs/WORLD_2026_27.md`.
- SWOS 96/97 Mod 4.1.2 `aminta/swos-9697-mod`: `tools/conti.c` (`ct_new1`, `s1list`, `ct_round`), `LICENSE`.
- Screenshot dell'utente: OpenSWOS Windows, World 2026/27, **UEFA CHAMPIONS LEAGUE FIRST ROUND** con tabella iniziale di 18 righe visibili, statistiche a zero e frecce di scorrimento. L'utente riferisce che anche le altre coppe si aprono in OpenSWOS, ma lo screenshot documenta direttamente **solo la Champions**. **La visione non dimostra da sola stato residente indipendente da manager, date e save in Expanded**.
- Evidenza Windows Expanded 0.13.59: da OFFERTA `OG DATI MANCANTI` per tutte le coppe provate; selezione Libertadores ritorna al calendario senza tabellone. Non dichiarare PASS.

## Due formati europei distinti — NON mescolare

1. **WORLD 9697**: Coppa Campioni OG storica (fase a gruppi tradizionale, non il formato moderno 36 club), Coppa Coppe e UEFA Cup con i propri tabelloni/round storici. Nessuna sostituzione dei record originali con `CompetitionLeaguePhase`.
2. **WORLD 2627**: tre record `CompetitionLeaguePhase` di tipo 2, 36 squadre ciascuno. Champions e Europa League: 8 giornate, 4 pot da 9; Conference League: 6 giornate, 6 pot da 6. La classifica UEFA e' un'unica tabella da 36, anche se lo scratch OG usa 2 gruppi tecnici da 18. E' OpenSWOS `LeaguePhase_InstallCups` che sostituisce i primi tre record nella tabella carriera e il riferimento in `_europeTable`. Non generare una seconda competizione LWE.
3. In OpenSWOS l'inizializzazione standard del tipo 2 usa il **wrapper** `InitCompetitionTeamSelectionFromAltTableHeader`: chiama `LoadSomeEuroCup`, azzera selezione, imposta `_numNeededTeams` e chiama `Career_InitCompetitionTeamSelectionFromAltTableHeaderSpec`. Expanded 0.13.59 saltava il wrapper. Correzione predisposta per la 0.13.60, da verificare su Windows.

## CT_S1 della SWOS 96/97 Mod 4.1.2: prova binaria

Eseguibile fornito dall'utente `ENGLISH.EXE`, **2.700.288 byte**, SHA-256:
`f2787b4135d1a9fc6b3d555569f0603a06fd9dee4c9ceb60170979c8132802b1`.

- Offset `0x28A0B0`: firma ASCII `S1`; header 8 byte.
- Numero entry: **178** record da 5 byte ciascuno (176 identita' club distinte; alcuni club possono comparire in piu' coppe).
- Seed little-endian: `0x20270801`.
- Seguono **12** elenchi di fase principale: conteggi [32, 32, 2, 16, 16, 32, 8, 16, 16, 0, 8, 0]; ciascun elenco ha 1 byte per il conteggio e 2 byte per ogni team-word.
- Dimensione blocco verificata: **1.266 byte**; SHA-256 del solo blocco:
`efed7cb926d9ac0afb66f9b219801eff969a643c80cba2095d3688df56ad6447`.
- In `tools/conti.c`, `ct_new1()` importa la prima edizione da questo blocco; `s1list()` conserva l'ordine dei gironi/turni principali. `ct_round()` usa gli elenchi fissi solo nella prima edizione, non ridisegna i partecipanti. Nella 0.13.60 il port seleziona i medesimi elenchi **solo quando la relativa fase nativa viene effettivamente avanzata** (non a START). La lettura verifica anche FNV-1a 32-bit `0xD49AF6E0`, oltre a firma, seed, dimensioni e contatori.
- Il codice OpenSWOS pubblico possiede il punto `CT_S1` ma commenta che **non include un blocco season-1** e avvia le continentali attraverso il normale ciclo OG. Non presumere quindi che lo screenshot OpenSWOS utilizzi automaticamente il blocco S1 della Mod 4.1.2.
- **Licenza**: il codice della Mod e' MIT; il suo `LICENSE` dichiara espressamente che *i dati calcistici NON sono sotto MIT*. Non incorporare `ENGLISH.EXE` ne' la lista completa degli iscritti nel GitHub pubblico senza permesso del titolare.

## Intervento preparato 0.13.60 — stato reale

- `sorgenti/src/world_conti.c`: lettore opzionale della **copia locale** dell'ENGLISH.EXE della Mod 4.1.2 in `ct_world_prepare_first()`. Verifica lunghezza, firma, numero iscritti, seed, capienza e 12 liste, poi crea `CT_ED` OG con seed e iscritti originali e prepara `CT_IN` **senza disputare partite a START**. Un eseguibile assente o diverso produce diagnostica `SOURCE_MISSING_OR_INVALID`, non iscritti inventati. L'utente deve disporre della propria copia nella directory di lavoro del gioco (o `data/`).
- `sorgenti/src/career_init.c`: invocazione della preparazione S1 soltanto a START 2026/27 (prima stagione) con stato leggibile nei log. Non modifica 1996/97.
- `sorgenti/src/career_status.c`: percorso UEFA tipo 2 usa il wrapper OG completo; durante fase 0 non esclude il manager dall'inizializzazione del contesto, mentre nelle fasi successive continua a proteggere le gare del manager non ancora collegate. **Attenzione verificata nel codice:** `LoadSomeEuroCup()` riconosce soltanto i tre puntatori dei record storici e non carica da solo i moderni type-2; per il 2026/27 la presenza delle rose dipende dal percorso world TMD/file e la selezione di 36 rimane da provare. La diagnostica `INIT_COUNT_MISMATCH` rende il difetto osservabile.
- `sorgenti/src/menus.c`, `menu_system.c`, `input_sdl.c`, `expanded_economy.c`: la rosa in sola lettura da OFFERTA non mostra, non seleziona, non attiva e non rende cliccabili le entry EXIT/CONTINUE; B torna direttamente all'OFFERTA.
- **Non risolto**: CT_S1 fornisce solo iscritti e ordine dei turni principali; la trasposizione nel torneo residente OG, gli accoppiamenti datati una volta sola, il viewer completo di Libertadores/Sudamericana e le altre continentali, la persistente snapshot dei 12 tornei e il collegamento manager-vs-mondo non sono verificati. Il normale `ct_world_advance_month()` usa mesi, non le date esatte delle fixture OG, quindi NON puo' essere invocato indiscriminatamente al primo giorno del mese.
- L'attuale viewer `Expanded_OGWorld_ViewLeague()` manda i type-2 2026/27 al percorso UEFA per ID 0..2; ID extraeuropei come 128 devono avere un percorso *specifico* e non essere confusi con la Champions. In questa revisione l'errore diventa esplicito anziche' un ritorno silenzioso, **non** un viewer residente funzionante.
- L'inizializzazione europea read-only resta da provare su Windows 10/11: nessun PASS automatico per i messaggi `OG DATI MANCANTI`.

## Nuova regressione segnalata (fuori perimetro di correzione immediata)

Il proprietario ha osservato che **WORLD 9697 crasha a NEW CAREER -> START**, sia con START UNEMPLOYED sia iniziando con una squadra. La regressione non e' stata diagnosticata né corretta in questo intervento, come richiesto. Non chiamare il 1996/97 collaudato e non far passare il test 70 stagioni come test Windows.

## Prove richieste dopo la sincronizzazione Windows

- 2026/27 START disoccupato, controllare log `[og-conti-s1]` e `[og-uefa-resident]`; distinguere `PREPARED_ONLY` da partita residente.
- OFFERTA -> VIEW SQUAD: EXIT/CONTINUE assenti, senza hitbox; B torna immediatamente all'offerta con una pressione; verificare anche X INFO e focus mouse/joypad.
- OFFERTA -> VIEW COMPETITIONS prima e dopo l'ingaggio: UEFA 3 e Libertadores, tipo 2 OG, selezione read-only e date; **se manca il residente, risultato = FAIL/INCOMPLETE**.
- Verificare su entrambi i mondi eventi datati, calendario, save/reload, continuità dopo l'ingaggio ed anni successivi. 1996/97 START attualmente da registrare FAIL riferito dall'utente; test Windows di questa build NON PROVATO.

Nessun merge/checkout di rami, nessuna cancellazione di salvataggi o intervento su fisica di partita.

## Seguito 0.13.61 — confronto con le prove OpenSWOS e guida operativa

Confermato nel sorgente `benbaker76/OpenSWOS/src/league_phase.c`: `LeaguePhase_InstallCups` sostituisce sia `_careerContests` sia il riferimento nel selettore `_europeTable.compCup`. La 0.13.60 Expanded ne sostituiva solo il primo; la 0.13.61 **ripara questa differenza** in `sorgenti/src/league_phase.c`, senza toccare i record 1996/97 o il file generato `world_data.c`. I partecipanti UEFA 2026/27 erano già nei tre `CompetitionLeaguePhase` del mondo, non richiedono ENGLISH.EXE. I residenti preesistenti possono quindi essere trovati dal picker attraverso il record corretto; conferma *visiva* Windows ancora necessaria.

Le coppe storiche 1996/97 usano tre record OG differenti: `_euroCup` type-2, `_cupWinnersCup` e `_uefaCup` type-1. Preparata l'inizializzazione residente della **prima stagione** nel world executor con snapshot delle copie OG; per le due type-1 si utilizza un marcatore numerico virtuale 254 distinto dalle competizioni nazionali; la ricerca read-only del viewer usa il puntatore OG originario. Le qualificazioni delle stagioni successive, gli ID duplicati e il crash START 96/97 preesistente non sono considerati risolti. **Non cambiare algoritmi, numero club, leggi della partita o calendari originali.**

Per le altre 12 continentali extra-UEFA: OpenSWOS possiede `world_conti.c`/ `conti_season.c`, ma il collegamento ad eventi OG residenti e viewer Expanded non è completo. Il materiale fornito dall'utente include `ENGLISH.EXE` Mod 4.1.2 con blocco `S1`, 178 iscrizioni di prima edizione. Il loader facoltativo della 0.13.60 legge il sorgente locale per non redistribuire dati calcistici non MIT; **il codice originale delle coppe non dipende dall'eseguibile**. Non dichiarare Libertadores pronta finché non sono disponibili fixture datate, viewer autentico e salvataggio dello stato residente, senza risimulazioni.

**Verifiche richiesta all'utente:** uno screenshot OpenSWOS 2026/27 Libertadores a START (prima delle partite) con partecipanti, un secondo dopo FORWARD TIME, e — se disponibile — save originale senza rischiare la carriera personale. Poi compilazione e test su Windows dell'Expanded 0.13.61 da GitKraken: 26/27 tre nomi moderni, 96/97 crash START e tre coppe storiche, Libertadores non dichiarata PASS, B da OFFER squad. Assenza di compile e runtime test Windows nel commit è esplicita.
