# Competition Engine — dove OG conserva i risultati e prova di riconciliazione
18 novembre 2026 · dev/0.10 · versione 0.13.101

## Perche' il calendario mostra le gare gia' giocate
Il calendario SWOS e' operativo anche senza il nuovo Competition Engine. I risultati delle partite del MANAGER sono registrati da `Career_UpdateStatsAfterMatch` nelle entry di `_teamSelectionKeyCache` (matchFlags, packedGoals, aggregate, penaltyScore). Il formato `TeamSelectionKeyCacheEntry` (10 B di testata + record da 12 B) contiene slot, fase e ID delle squadre. `DIY_PlayOrSimulateLeagueFixture` aggiorna anche classifiche e progressione OG; `DIY_PlayCupMatchAndAdvance` aggiorna i tabelloni. Il CAR JSON conserva la cache e gli slot delle competizioni. La visibilita' di una partita non significa pero' che ogni score CPU di ogni lega del mondo sia in questa cache: i residenti OG archiviano stati e classifiche e NON hanno ancora un journal universale di fixture datate.

## Il punto preciso della duplicazione potenziale
- `Expanded_OGWorld_BindClub` legge una lega OG residente e ne copia una VISTA adattata nel contesto locale del manager (riga manager con teamSelOffset -1). Al momento del contratto non genera alcun risultato.
- `Expanded_OGWorld_AdvanceLeague` avanza una COPIA residente con i callback nativi OG; `Career_PostMatchUpdate` e le UI del manager usano anche i propri slot OG.
- `Expanded_OGWorld_DebugCanAdvanceClub` gia' confronta due match count e la classifica prima di permettere alcuni avanzamenti di debug: su divergenza blocca, non riscrive. Questa protezione e' evidenza dell'assenza di un merge generale certificato, NON dell'assenza di risultati nel calendario.
- Per le UEFA del club allenato, `PENDING_MANAGER_BIND` blocca la seconda simulazione residente; le coppe nazionali hanno un limite analogo. Questi percorsi non sono ancora un collegamento universale.

## Prova di codice 0.13.101 — NON mutante, utile per preparare il merge
- Nuovo `expanded_og_cache_match.h`: lookup bounded delle entry OG, con chiave slot + fase + coppia ORIENTATA di club; BE degli ID, host-native per stride/scores; `PLAYED`, `SCHEDULED`, `NOT_FOUND`, `AMBIGUOUS`, `CORRUPT`. Una chiave ambigua non autorizza a fondere due incontri. Da sola la chiave non e' un Edition/Fixture ID del Competition Engine.
- Dopo un risultato reale del manager in `DIY_PlayOrSimulateLeagueFixture`, `Expanded_OGWorld_AuditManagerLeagueResult` (solo CAREER SDL) confronta score della cache, identita' club, anno, fase e tabelle per ClubID, neutralizzando l'offset locale -1. Registra `[og-manager-fixture]` con `cache-score`, `world-matches`, `local-matches`, `row-diff`, `unmapped`, `status`.
- **NESSUNA modifica alla partita, alle classifiche, all'RNG, al CAR o al motore di avanzamento.** La prova osserva, ma non sostituisce o ripete una partita. Il primo Windows log dira' se e quando i due stati sono gia' allineati.

## Nuova scoperta CRITICA da verificare, non ignorare
In `career_teams.c` il buffer runtime `_teamSelectionKeyCache` e' di **8192 byte**, ma `save_format_json.c` serializza `team_selection_key_cache` con `FA(..., 1870, FLD_BYTES)`, e il normalizzatore `cache_scores/cache_strides` usa `TSKC_SIZE=1870`. Non c'e' prova che un salvataggio reale del tester abbia usato record oltre byte 1870, ma l'incompatibilita' di capacita' e' reale e va gestita prima di affidarci alla cache per il journal completo. Il nuovo audit misura `cache-used` e `match-end`: se uno supera 1870, scrive `car-capacity-risk=YES`. **NON estendere alla cieca il blob a 8192** prima di avere un loader compatibile per i CAR esistenti e preflight in lettura.

## Test e gate
- Test locale C puro: compilato con `-std=c11 -Wall -Wextra -Werror -pedantic` e passato (lookup esatto, fase diversa, score, ripetizione, corruzione); non e' gameplay.
- Suite e runner diagnostico: includono il test (da eseguire su Windows).
- Compilazione completa 0.13.101, warning/errori Windows, esecuzione della vera partita, save/reload: **NON PROVATI**.
- Nuovo motore single-writer: **NO**. CAR scheduler/journal atomico: **NO**. Fase 3: **ANCORA APERTA**.
- Gate successivo: dopo prova `[og-manager-fixture]` su campionato 96/97 e 26/27, scegliere un protocollo unico manager/CPU e una vera ID fixture, propagare lo score OG senza ripetere simulazione, e checkpoint CAR atomico. Non fare una semplice copia della classifica senza validare fixture/effetti/RNG.
- Obiettivo prodotto invariato: inserimento di competizioni OG, altri SWOS e create dall'utente tramite definizioni/regolamenti; almeno tre edizioni, qualificazioni autentiche, nessun codice C speciale per competizione.

## Protocollo per l'utente Windows
Sincronizzare GitKraken sul ramo dev/0.10; backup esterno di AUTO9697.CAR / AUTO2627.CAR; compilare con COMPILA_SWOS.bat e controllare TUTTI i warning. In test con save separati: START, assumere un club, disputare/simulare UNA partita di campionato, verificare risultato nel calendario del club e in DEBUG WORLD; salvare, chiudere e CONTINUE; allegare SWOS.log e diagnostica. Cercare `[og-manager-fixture]`: se `CACHE_UNVERIFIED` o `car-capacity-risk=YES`, NON dichiarare integrazione riuscita. Non cancellare vecchi CAR.
