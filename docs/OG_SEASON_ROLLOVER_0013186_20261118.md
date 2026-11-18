# SWOS Expanded 00.13.186 — OG season rollover, cup entrants and fail-closed bridge

Data: 18 novembre 2026. Ramo: `dev/0.10`. Base: `1e1cce3359c8159b753a2dcfe84b4df85cbbe333` (00.13.185).

## Diagnosi Windows precedente (00.13.185)
Nel log 2026/27 del 18/11 alle 15:19, dopo il cambio manuale verso il 2028, il residente OG restava nel 2027. Il loop del manager ha scritto 54.844 `[og-manager-bridge] WRITE_BLOCKED ... MISSING_OR_INCOMPATIBLE_RESIDENT` senza consumare incontri. L'autosave rifiutato preservava correttamente il CAR precedente. In 1996/97 da disoccupato il test qualificazioni indicava fonte assente. Si tratta di osservazioni reali della 00.13.185, **non** di un collaudo della 00.13.186.

## Interventi applicati (codice, NON gameplay verificato)

1. **Cambio manuale**: `ContinueCareerNextSeason` fa avanzare il RESIDENTE OG originale al 1 luglio del nuovo anno tramite `Expanded_OGWorld_AdvanceToDate` **prima** di spostare il calendario ausiliario. Poi tenta il bind nativo del club e la ricollocazione del calendario del manager. Registra `[og-manual-season]` con esito e millisecondi per native/clock/bind. Se manca un contesto, non scrive `new-season` CAR e non apre il calendario incoerente: precedente save preservato, errore esplicito.
2. **Loop infinito**: un preflight `WRITE_BLOCKED` pone un latch. Il percorso OG locale `Career_AutoAdvanceLeagueFixtures` e il suo chiamante si fermano invece di ritentare la stessa partita immutata. Il guard non si applica al mondo residente in avanzamento. Log del primo rifiuto, niente tempesta di decine di migliaia di righe; il blocco del CAR rimane **attivo**. Il latch si azzera al nuovo bind valido/reset mondo.
3. **Stagioni delle coppe nazionali in disoccupazione**: al singolo rollover nativo OG di luglio, `g_worldCups.seasons` viene portato all'indice esatto mediante `WorldCups_SeasonEnded` solo se non gia' raggiunto dall'OG manager. Nessun callback delle partite viene richiamato una seconda volta. `[og-cup-season-index]` evidenzia progressi e anomalie, senza rewind.
4. **Edizioni UEFA successive alla prima**: il motore riusa i pool ORIGINALI prodotti dalla logica `career_eurocups.c / uefa_access.c`. Verifica mondo, anno, conteggio (36 x 3 moderno; 16+32+32 storico), identita' e duplicati prima di costruire i soli record transitori della nuova edizione; non sovrascrive i template iniziali. La competizione prosegue nel normale esecutore OG; il manager puo' rimanere proprietario delle sue gare, senza simularle due volte. Include CWC/UEFA Cup storiche e Copa storica se disponibile il campo 20 valido. Al ripristino di una nuova edizione gia' residente nel CAR riprende gli ID reali esistenti senza nuova qualificazione/sorteggio. `NEW_EDITION_READY`, `RESUME_EXISTING_EDITION`, `NEW_EDITION_PENDING`, `EDITION_REPLACED` sono audit distinti. **Non dimostra ancora l'esattezza indipendente dei posti rispetto alle classifiche OG residenti**.
5. **Coppe nazionali, stagione iniziale e seconda**: nuovo `WorldCups_PreviewEntrants` legge la regola OG originale senza mutare seed/bracket. Confronta ID veri del residente, non soli numeri. Distingue `INITIAL_NATIVE_SEED` da `NATIVE_NEXT_YEAR_ACCESS`; una lista generata SENZA piazzamenti finali OG del paese **non puo' passare**, anche se coincidesse con la lista caricata. Le coppe senza regole verificabili restano NOT_TESTED. `[og-cup-entry]` riporta diagnostica per paese/cup e causa.
6. **Prima edizione/manuale**: l'helper isolato accetta soltanto seed OG reali o un insieme manuale **esplicitamente approvato** e identico all'edizione OG. Non viene installata nessuna lista manuale/fittizia in questa build: manca una fonte approvata per le competizioni prive di dati, specialmente il primo anno di alcune confederazioni. Gli esiti `FIRST_YEAR` non vengono confusi con `QUALIFIED_NEXT_YEAR`.
7. **FORWARD TIME**: contatori monotoni SDL per campionati, coppe nazionali, internazionali e overhead per fase `[og-forward-perf]`, piu' ciclo OG+journal `[og-forward-cycle]` e cambio manuale. Il parser riporta campioni e millisecondi 96/26 senza inventare tempi benchmark. Nessuna ottimizzazione del gameplay prima dei dati Windows.
8. **Parser runtime**: `OG_FIRST_EDITION_CUP_SEED`, `OG_NEXT_EDITION_DOMESTIC_CUP_IDS`, `OG_MANUAL_SEASON_SYNC`, `OG_FORWARD_PERFORMANCE`, oltre a `OG_NEXT_SEASON_QUALIFICATION`; un sync fallito e' regressione e NON PASS. Fixture sintetici verificano successi/errore, non costituiscono test nel gioco.

## Gap ancora aperti (NON nascondere con fake)
- **Qualificazioni delle coppe internazionali da DISOCCUPATO**: la sorgente OG di fine stagione resta agganciata alla carriera del club e non viene ancora emessa direttamente dagli stati residenti senza manager. Il test adesso lo segnala **FAIL_UNEMPLOYED_QUALIFICATION_NOT_CONNECTED**, non piu' `NOT_TESTED` neutro. Il motore conserva il campo precedente invece di inventare qualificati o rieseguire le leghe.
- **Libertadores moderna / CAF / AFC / OFC / CONCACAF**: richiedono campi OG indipendenti e access list per edizione. `CT_ED` non e' lo stesso record TYPE2 della Libertadores storica. Niente seed inventati o EXE esterni.
- **Piazzamenti finali delle coppe nazionali**: `g_worldCups` conserva i piazzamenti OG solo per un paese alla volta nella logica legacy. Da disoccupato il contatore della stagione ora progredisce, **non** significa che esista una tabella piazzamenti completa per le 87 nazioni. Senza essa il test deve segnalare `FAIL_OG_FINAL_PLACES_UNAVAILABLE`. Da progettare la persistenza del finale OG per tutti i paesi senza una seconda simulazione.
- **Precedenti edizioni e save**: quando nasce una nuova UEFA residente si rimpiazza l'ultimo contesto nella slot attiva; le gare rimangono nel journal, ma non e' ancora provato un viewer/snapshot completo della stagione precedente. Non alterata struttura CAR e non promesso storico completo.
- **Calendario manager vs OG e marcatori**: i precedenti `CALENDAR_PAIR_MISMATCH` e `UNEXPLAINED_SAME_SCOPE` rimangono diagnosi aperte; nessun punteggio alterato per far passare il test.
- **Regole manuali**: testata soltanto la distinzione di provenienza `NATIVE`/`MANUAL_APPROVED`/`MANUAL_UNVERIFIED` su dati sintetici. Nessuna UI/editing/import JSON di squadre manuali introdotto senza un elenco approvato.

## Verifiche isolate
- C11 `test_expanded_qualification_transition.c` con GCC e Clang e `-Wall -Wextra -Werror -pedantic`: PASS per identita', mondi/anni, mancanti/duplicati, provenienza prima edizione. Non prova il motore completo.
- `run_test_og_runtime_parser.sh`: ampliato con righe sintetiche per coppe, misure, cambio manuale e fallimenti; esecuzione del nuovo parser completo richiede verifica successiva nel normale ambiente diagnostico.
- Controlli testuali del codice aggiornato e degli anchor di patch; **nessuna build Windows 00.13.186 e nessuna partita realmente eseguita** dall'assistente.

## Verifiche Windows da eseguire
1. Sincronizzare GitKraken `dev/0.10`, compilare 00.13.186. Registrare warning/errori; eventuali warning vanno corretti subito.
2. Mondo 2026/27 con club: completare passaggio MANUALE 2027 -> 2028; l'esito atteso nel log e' `[og-manual-season] status=SYNCED` e `[og-bind]` per il 2028; **zero loop di WRITE_BLOCKED**; nuovo CAR accettato solo se il nativo coincide.
3. Ripetere due transizioni nel 1996/97 con club, poi SAVE/CONTINUE; verificare le due coppe classiche, la Champions e la Copa storica se disponibile.
4. Disoccupato in 96 e 26: attraversare il 1 luglio; verificare contatore coppe nazionali, status reali per access list, e FAIL esplicito sulle qualificazioni UEFA ancora non connesse. NON interpretare il mondo che avanza come successo delle qualificazioni.
5. Su prima e seconda edizione verificare partecipanti uno a uno e confrontare `[og-cup-entry]` e `[og-qual-next]` con il viewer. Verificare che il secondo anno non ricicli il primo.
6. Confrontare `[og-forward-perf]` e `[og-forward-cycle]` a pari fasi in entrambi i mondi; fornire ZIP diagnostici per individuare carichi evitabili.
7. Controllare mismatch calendario e marcatori, compatibilita' di vecchi AUTO9697.CAR/AUTO2627.CAR, senza sovrascrivere salvataggi preesistenti.

## Checklist
SI: modifiche sorgenti, audit per ID, controllo primo/secondo anno, log performance, protezione anti-loop/autosave, test puro C.
NO: qualificazioni internazionali da disoccupato pienamente collegate, tutte le altre confederazioni, compilazione Windows effettuata dall'assistente, import manuale approvato, persistenza completa delle classifiche finali per tutti i paesi.
NON PROVATO: build completa 00.13.186 senza warning, gameplay 96/26, cambio manuale, SAVE/CONTINUE, ogni nuova edizione/fixture.
