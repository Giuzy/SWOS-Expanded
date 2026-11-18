# SWOS Expanded 00.13.185 — test qualificazioni nella stagione successiva

18 novembre 2026 — `dev/0.10`. Baseline: `91f320213b8e76ef09b215d0c626eab01b0a5fac` (00.13.184).

## Obiettivo e metodo
Verificare che le IDENTITA' dei club qualificati dalle competizioni OG concluse nella stagione N compaiano **nella competizione giusta dell'edizione N+1**, non solo che il numero degli iscritti sia corretto. Il controllo e' READ-ONLY: non produce partite, sorteggi, promozioni, qualificazioni o risultati; non modifica RNG, stagioni, rose, calendario, CAR o classic modes.

Alla conclusione della stagione del manager, `Career_AdvanceToNextSeason_UpdateCompetitions` costruisce con gli algoritmi OG i quattro pool di qualificati. `Expanded_OGWorld_QualCaptureNative` acquisisce le coppie di byte `{country, ordinal}` in RAM diagnostica e registra nel log **ogni club qualificato** con `[og-qual-next-source]`. Dopo l'avvio dell'edizione successiva, `Expanded_OGWorld_QualAuditNextYear` legge SOLTANTO gli stati OG residenti veri: le tre coppe europee modern TYPE2, oppure Champions TYPE2 / CWC e UEFA Cup TYPE1 nel 96/97, e Copa Libertadores TYPE2 storica quando il pool nativo e' disponibile. Il confronto usa world ID, anno-edizione, competition slot e ID club orientato: no equiparazione per solo nome, numero di iscritti, punteggio o coppa dell'anno prima.

Ogni ROUTE riporta `PASS_EXACT_IDS`, `FAIL_NEW_EDITION_MISSING`, `FAIL_WRONG_QUALIFIERS` o `FAIL_INVALID_SOURCE`, con dimensioni delle due liste, mancanti, extra, duplicati, year e ID di esempio. `SUMMARY` per mondo/anno non diventa PASS se un'edizione manca. L'analizzatore `audit_windows_runtime_log.c` produce `OG_NEXT_SEASON_QUALIFICATION` per ogni sessione e distingue FAIL, INCOMPLETE e NOT_TESTED.

## Limiti espliciti
- Le liste OG di qualificazione vengono generate nella transizione carriera con club: se l'utente avanza da disoccupato senza questa fonte, `NOT_TESTED_SOURCE_UNAVAILABLE`, non PASS.
- Le nuove edizioni continentali residenti oltre la prima sono gia' segnate come `QUALIFICATION_NOT_CONNECTED` nel codice: l'audit le rileva come `FAIL_NEW_EDITION_MISSING` dove esiste il pool OG ma non il nuovo residente. Questa build **non implementa il passaggio di qualificati**.
- Coppe domestiche, competizioni extra-UEFA in `CT_ED`, tornei nazionali, spareggi/promozioni e regole complete di accesso multidipendenza richiedono un mappatore fonte->edizione separato; sono `NOT_TESTED`. Copa moderna non va letta come Copa TYPE2 storica.
- La prova riguarda l'identita' dell'elenco OG nativo vs ingresso nel relativo residente; non costituisce ancora verifica indipendente delle regole politiche dei posti per paese (vincitore coppa, piazzamenti, detentori ecc.).
- Il test non aggiunge nulla al CAR: dopo un riavvio della diagnostica, il snapshot RAM non viene recuperato; gli ID della fonte sono conservati nei log precedenti per eventuale audit incrociato. SAVE/CONTINUE/2a edizione in Windows sono da verificare.

## Test isolati (non gameplay)
`sorgenti/tests/run_test_expanded_qualification_transition.sh` compila col normale compilatore C11, `-Wall -Wextra -Werror -pedantic`, il matcher puro: insiemi corretti riordinati, club nella coppa sbagliata, mancante, duplicato, ID invalido, 0x0000 valido, overflow, anno errato e mondo errato. Aggiunto al runner diagnostico come `COMPETITION_ENGINE_ISOLATED`; un PASS qui non certifica qualificazioni reali.
`run_test_og_runtime_parser.sh` esercita il riconoscimento di una nuova edizione assente e una lista esatta nelle due epoche mediante log sintetici. Anche qui il risultato e' solo parser statico.

## Procedura Windows 00.13.185
1. Sincronizzare `dev/0.10`, compilare con `COMPILA_SWOS.bat` e verificare **0 errori / 0 warning**.
2. Creare o proseguire una carriera con club nel 1996/97; avanzare fino a dopo il 1 luglio dell'anno seguente, verificare `[og-qual-next] SOURCE`, `ROUTE`, `SUMMARY` e i nomi delle competizioni.
3. Ripetere nel 2026/27. Senza club, un `NOT_TESTED` e' previsto, non e' successo. Controllare che anno, ID, coppe e eventuali assenze siano evidenti.
4. Ripetere dopo SAVE/CONTINUE e trasmettere ZIP diagnostici + compilazione. La foto dello schermo del calendario non prova che tutte le squadre siano corrette: confrontare gli ID nel log e lo stato residente.
5. Non risolvere un FAIL copiando i qualificati nella nuova coppa: il collegamento futuro dovra' rispettare regolamenti, turni, sorteggi OG e singolo writer.

## Checklist
- SI: strumento read-only, confronto per ID, scenari negativi isolati, output aggregato parser, guida.
- NO: circuito completo di qualificazione dell'anno successivo, tutte le competizioni, persistenza della diagnostica in CAR.
- NON PROVATO: compilazione Windows 00.13.185, warning globali, partita e qualificazioni 96/26, rollover disoccupato, SAVE/CONTINUE, coppe trans-stagionali.
