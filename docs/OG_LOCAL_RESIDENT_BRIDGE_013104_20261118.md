# SWOS Expanded 0.13.104 — trasferimento incrementale OG locale → residente
Data 18 novembre 2026 · `dev/0.10` · base `37c468fcad14b64f58f3087431c34677dc3af293`.

## Motivo
Nella 0.13.102 il mondo residente restava a 15 gare e la lega locale del manager ne aveva 24 (2026/27); il caso 1996/97 mostrava 45 contro 48, poi 54 e 59. Non era sicuro applicare soltanto il risultato visibile del manager, perche' il contesto locale aveva giocato anche gare CPU. `TABLES_DIVERGED`, `CACHE_UNVERIFIED`, `CPU_FIXTURE_NOT_OBSERVED` sono indicatori distinti, non prove di doppi incontri.

## Cambiamenti concreti (non una seconda simulazione)
1. La vera funzione OG `DIY_PlayOrSimulateLeagueFixture` chiama `Expanded_OGWorld_PreflightLocalLeague` **prima** di iniziare un match del campionato ordinario del manager. Il controllo confronta il numero di partite e ogni statistica W/D/L/GF/GA/points/gamesPlayed dei club risolti per ID reale. Il mondo attivo non deve aver gia' elaborato quella gara.
2. Dopo il risultato OG vero, `Expanded_OGWorld_ObserveFixture` conserva l'identita' nativa e lo score. Registra tutti gli incontri CPU di questo contesto locale con `origin=3`, oltre ai manager con `origin=2`; la fonte residente mantiene `origin=1`. Non esiste una nuova funzione per calcolare un punteggio. L'ordine è journal → copia (se testimone valido).
3. `Expanded_OGWorld_CommitLocalLeagueFixture` valida `resident.matchesPlayedCount + 1 == local.matchesPlayedCount`, e confronta l'unico incremento delle statistiche con la coppia orientata e le regole native dei punti. Costruisce la mappa di tutti i club tra i due contesti; `teamSelOffset==-1` diventa l'indice residente del club allenato, gli altri indici vengono risolti per `(countryNumber,teamOrdinal)`, mai per posizione/rank. Copia l'intero `DIYCompetitionEx` e le `TeamFileEx` (conservando i `teamControls` CPU del residente), e registra i tre byte RNG. **Non richiama** OG per rigenerare la partita.
4. Finche' il club e' ancora gestito, `Expanded_OGWorld_AdvanceLeague` **non simula di nuovo** quella lega nel residente: attende gli snapshot del percorso locale, mantenendo attivi gli altri campionati/coppie di coppe. Dopo l'esonero il manager non possiede piu' quel percorso.
5. Per il campionato ordinario `hasGroups==0`, `DIY_SyncProgressToDiyFileBufferCopy` non modifica il buffer torneo. Si conserva l'immagine residente per gli indici `teamIds`/`teamIndexOrder`/gruppi e si aggiorna lo stato match dalla competizione. Le competizioni `hasGroups!=0` richiedono una **rimappatura specifica del torneo**, per cui il bridge le blocca anziche' copiare ciecamente gli indici locali.

## Protezione salvataggi e regressioni
- Non cambia il wire CAR: `og_resident.fixture_journal` resta `format=1,record_size=20`, e il validatore gia' esistente accetta `source=3`. La serializzazione nativa esistente conserva lega, squadre, RNG e il journal nello stesso CAR atomico.
- Qualsiasi `[og-manager-bridge] WRITE_BLOCKED` (mappa errata, tabella o seriale non sequenziale, contesto mancante, risultati discordanti) rende non scrivibile un CAR nuovo tramite il guard `s_expOGFixtureIssues`, lasciando intatto quello precedente. Un salvataggio precedente alla 104 con lega locale **gia' non riconciliata** potrebbe essere rifiutato al primo nuovo match; non viene riparato inventando risultati.
- `[og-manager-world-fixture] CPU_FIXTURE_NOT_OBSERVED` puo' essere registrato prima della copia locale, poiche' il record residente distinto non esiste ancora: dopo un trasferimento corretto il segnale attendibile e' `[og-manager-bridge] COMMITTED` e lo stato del viewer. `CACHE_UNVERIFIED` resta un audit indipendente del vecchio keycache.
- Nessuna mutazione di CLASSIC/DIY/SEASON, nessun cambiamento di fisica/controlli partita OG e nessun cambio formato/tipo di file CAR. Non sono state apportate modifiche a netplay.

## Test e limiti
- Nuovo helper C puro `expanded_og_league_bridge.h` testato con un harness isolato su GCC e Clang (`-Wall -Wextra -Werror -pedantic`), inclusi: ordine righe diverso, manager `-1`, transizione 2–1, delta di classifica errato, ID duplicato. Aggiunto `run_test_expanded_og_league_bridge.sh` al runner diagnostico. Il test isolato NON simula partite e NON e' un PASS Windows.
- **NON PROVATI:** compilazione Windows completa (e relativi warning), match giocati Windows in 96/97 e 26/27, 2+ match manager e gare CPU locali fra loro, collaudo SAVE/CONTINUE, esonero/ripresa CPU, gironi/split/coppe. I mondi restano unici, senza duplicazione autorizzata. La fase 3 e' ancora aperta: scheduler live universale, tutte le competizioni, date esatte, checkpoint completo e anni multipli richiedono ancora verifiche e interventi.

## Checklist Windows — compilare con GitKraken
1. Backup ESTERNO di `save/AUTO9697.CAR` e `save/AUTO2627.CAR`, nessuna cancellazione o sovrascrittura automatica.
2. Sincronizzare `dev/0.10`, compilare da `COMPILA_SWOS.bat`: 0 errori, 0 warning; eseguire `AVVIA_DIAGNOSTICA.bat`, verificare `run_test_expanded_og_league_bridge=PASS_ISOLATED_ONLY`. Condividere se FAIL.
3. In entrambe le carriere **di prova** 1996/97 e 2026/27, prendere club di lega ordinaria senza gruppi. Giocare una partita e un turno con CPU locali; cercare per ogni risultato `[og-manager-bridge] COMMITTED`, nessun `WRITE_BLOCKED`.
4. Verificare in DEBUG WORLD e calendario che classifica/risultato del manager e contatori coincidano; ripetere un'altra partita, poi FORWARD TIME. Non devono apparire incontri risimulati, `TWO_WRITERS` o `SCORE_CONFLICT`.
5. Salvare, chiudere completamente, CONTINUE; confermare stessi risultati e progressi e la continuita' `[og-fixture-journal] RESTORE`. Successivamente controllare esonero/nuovo impiego senza reset del campionato.
6. Un test con gironi/split **non e' previsto come riuscito**: richiede un adattatore separato. Se nel log compare `WRITE_BLOCKED`, non forzare un nuovo CAR; inviare diagnostica senza eliminare i precedenti.
