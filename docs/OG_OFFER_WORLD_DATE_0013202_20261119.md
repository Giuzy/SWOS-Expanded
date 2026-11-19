# SWOS Expanded 00.13.202 — CORREZIONE DATA DOPO OFFERTA (SOLO UI)
19 novembre 2026. Base: `dev/0.10` commit `c1bfcbd46ca99549e4caf61ed0eaf7b1a3e54616`, 00.13.201.

## Evidenza
Screenshot utente `image(20261119-142841).png`: CAREER con EDINBURGH CITY, stagione 2026/27, riquadro superiore `SAT 01 AUG 2026` e prima partita di LEAGUE TWO ad agosto. L'utente riferisce che il salto apparente segue ACCETTARE OFFERTA dal 30 giugno, non FINISH SEASON. Non confondere lo screenshot con il diverso club Rochdale nei log di SESSIONE_2.

Diagnostiche Windows 00.13.201 precedenti: `AUTO9697.CAR.living_world.now=35610` e `AUTO2627.CAR.living_world.now=46567` corrispondono entrambi al **1 luglio** delle rispettive nuove annate. I log mostrano invece la PRIMA gara OG il 1 agosto. Il codice prova la causa grafica: `sorgenti/src/expanded_economy.c` nel renderer Approved UI `Expanded_UI_After` prendeva `Expanded_Career_NativeCalendarKey()` (la phase key della PROSSIMA partita) e la visualizzava come data di oggi; anche Business ripeteva lo stesso errore. Il renderer originale, senza UI Expanded, usa gia' `ExpandedWorldLive_CalendarState().today`.

## Correzione
- Il solo renderer CAREER SDL usa la data civile del **medesimo mondo**; primo contratto, rehire e manual FINISH mostrano oggi, non il primo kickoff. Quattro etichette (calendario + tre Business) usano una funzione comune `Expanded_FormatCurrentCareerDate`. Non cambia pixel, font, asset, hitbox e testi.
- Introduce un validatore/selector puro `expanded_career_date.h` chiamato dalla UI e testato a parte. In CAREER, se non esiste TODAY valido, mostra `DATE NOT AVAILABLE` senza ripiegare su OG next phase. In CLASSIC/DIY/SEASON mantiene esattamente la conversione precedente.
- Nessun `WorldLive_AdvanceToDate`, nessun `InitTeams`, nessuna simulazione o modifica di `_viewRandomPhase`, `_teamSelectionKeyCache`, RNG, CAR, premi, classifiche o regole. La FIRMA non forza luglio: se fatta il 30 giugno rimane il 30 giugno finche' l'evento mondiale autorizzato non passa al 1 luglio.
- **Le amichevoli di luglio NON sono implementate da questa correzione** e richiedono gli eventi veri del nuovo Competition Engine. Inoltre e' aperto il bug diverso di world.today che non avanza con tutte le gare manager, oltre a `OG_MANAGER_CALENDAR_PAIR_IDENTITY`. Non usare questa patch come surrogato del fixture single-writer o del scheduler live.

## Verifiche e prove Windows utente
Test puro `tests/test_expanded_career_date.c` agganciato al runner esistente `run_test_expanded_world_time.sh`: firma al June30, Today July1 con OG next Aug1, anno storico 1997, non-CAREER OG Aug1, dati mancanti o invalidi; controllare risultati reali dopo esecuzione. Verifica statica delle chiamate al nuovo helper nelle quattro posizioni; build Windows 00.13.202 e gameplay **NON PROVATI** fino alla diagnostica utente.

Checklist suggerita: dopo GitKraken compilare con script ufficiale e controllare **tutti i warning**. In entrambi i mondi: NEW CAREER disoccupato -> DEBUG 30 JUNE -> ACCETTA OFFERTA: il banner deve mostrare la **stessa data civile** presente prima nel calendario UNEMPLOYED (es. June30), senza modificare classifiche/fixture; dopo FINISH SEASON manuale banner **July1** e prime gare campionato eventualmente ad agosto. Verificare i tre pannelli Business, salvataggio e CONTINUE, disoccupato, CLASSIC/DIY/SEASON; allegare screenshot e ZIP, distinguendo data TODAY da NEXT MATCH.

## Addendum 00.13.203 — FIX GCC
Il test Windows `DIAGNOSTICA_SESSIONE_1(20261119-144856).zip` compilando 00.13.202 è **FALLITO** durante la verifica sintattica: `src/expanded_economy.c:864:26: error: 'GAME_TYPE_CAREER' undeclared`. Il macro è definito esclusivamente in `menus.c:2661`, mentre `expanded_economy.c` include `game_types.h` (che NON lo espone). La 00.13.203 cambia solo l'espressione in `_gameType==4u`, con commento sulla fonte; nessuna variazione alla logica, ai dati o alla UI. I test isolati time/CAREER del medesimo ZIP riportano PASS ma non rilevano errori del translation unit della UI. Serve la nuova compilazione Windows 203: **NON PROVATA**.
