# SWOS Expanded 00.13.187 — Qualificazione OG durante disoccupazione

Data: 18 novembre 2026. Ramo: dev/0.10. Base verificata: 82c6000a430a0d68ea19a16aaaa34d1ca56f7c5b (00.13.186).

## Scopo
Colmare la mancanza osservata nel test del mondo 1996/97 in disoccupazione e nel percorso equivalente del 2026/27: `NOT_TESTED_SOURCE_UNAVAILABLE` non deve più essere un'omissione del callback di fine stagione del manager. Il qualificato deriva soltanto dal vecchio OG residente, senza simulare una partita due volte.

## Percorso del codice
1. `expanded_og_world_run_year(closedYear,56)` termina la stagione OG residente. Prima di cancellare il contesto dell'anno, `Expanded_OGWorld_QualifyUnemployed(closedYear,nextYear)` legge la lega di massima divisione conclusa dal contesto OG (ordine dei veri finalisti secondo `teamEntryOffsets`), con squadre/ID reali. Legge anche il vincitore delle coppe OG nazionali e continentali dal `DIYTournamentState.header.winnerTeam`; ID non presente nella rosa residente non è valido.
2. Le stesse funzioni OG della carriera tradizionale selezionano i qualificati: primo mondo 96/97 con `Career_PrepareEuropeanCupQualificationState`, `Career_ProcessCompetitionForEuropeanCupQualification`, e `Career_DrawEuropeanCupTeamsAndCopyGrids`; mondo 26/27 con `UefaQual_Country/UefaQual_Fill` e le stesse verifiche dei detentori. La prima Copa storica usa `Career_ProcessCompetitionForCopaLibertadoresQualification` quando i campi reali sono disponibili. Il protocollo non esegue `Career_AdvanceToNextSeason_UpdateCompetitions` né `Conti_Country` sul mondo disoccupato, perché potrebbero duplicare gare/edizioni già concluse.
3. Per il manager **inesistente**, la funzione originale di append non tocca il TMD del manager. I suoi buffer e il focus vengono ripristinati, il seed Copa del primo anno resta invariato e le liste Europa/Champions/CWC restano in scratch fino alla conferma dei conteggi. Campi invalidi, mancanti o duplicati sono segnalati; il classico selettore non può attingere dai vecchi buffer dei finalisti se la finale nazionale non è verificata.
4. Il blocco qualificati di ogni mondo/anno è catturato con `Expanded_OGWorld_QualCaptureNative`. Il codice già presente in 186 lo usa per creare le nuove UEFA TYPE2/coppie CWC/UEFA storiche soltanto se il campo dell'anno corrisponde al residente e contiene il numero esatto di squadre. Il campo Copa storica richiede 20 club reali. **Non** sono alterati i formati del primo anno.
5. Nuovo campo opzionale `og_resident.qual_next` nello stesso JSON CAR, con formato binario portatile a 524 byte (anno, mondo, 4 conteggi e liste ID), checksum FNV-1a, duplicati rifiutati. Su READ verifica checksum, mondo, anno (corrente o successivo), ID, duplicati e, dove esiste un residente di quell'anno, confronto insiemistico degli ID. Un CAR 186 senza il campo non inventa le qualificazioni. Tutti gli stati OG e il fixture journal rimangono nel loro sistema originale.

## Audit
- `[og-unemployed-qual]`: quantità di sorgenti presenti, campi generati o incompleti, eventuale fonte mancante
- `[og-unemployed-qualification]`: esito del passaggio cronologico residente
- `[og-qual-next]`: verifica ID e conteggi della nuova edizione, già presente
- `[og-qual-car]`: SAVE, PRECHECK e RESTORE con checksum
- `OG_UNEMPLOYED_QUALIFICATION` e `OG_UNEMPLOYED_QUALIFICATION_CAR` nel parser Windows
- Test sintetici del parser aggiornati; test isolato matcher GCC/Clang con -Wall -Wextra -Werror -pedantic PASS. Non dichiarare compilazione completa o gameplay.

## Limiti espliciti
- Le classifiche/campioni di alcuni paesi potrebbero non essere residenti o completi: in tale caso la qualificazione resta INCOMPLETE_FIELD e la nuova edizione non viene aperta artificialmente.
- Il percorso storico richiede finalisti di coppe che non sempre sono ricostruibili dal vecchio contesto residente; un fallback non verificato è rifiutato. Il comportamento con tutti i club non è stato ancora collaudato su Windows.
- Coppe internazionali non-UEFA moderne e tutte le loro dipendenze reali (Libertadores moderna/CT e altre confederazioni) richiedono adapter separati. Nessuna competizione non provata viene presentata come funzionante.
- La modifica non risolve mismatch coppie calendario, marcatori, economia o performance. Misure e test 186 restano attivi.
- Nessuna build dell'intero progetto Windows/Linux, nessun `SWOS_Expanded.exe` 187 compilato dall'assistente, nessuna partita in-game provata.

## Checklist Windows utente
1. GitKraken sincronizzato su dev/0.10, 00.13.187, COMPILA_SWOS.bat senza warning/errori.
2. Nuova carriera disoccupato 1996/97: FORWARD TIME fino a 1/7/1997; cercare `[og-unemployed-qualification] status=OG_FIELD_CAPTURED`, `[og-qual-next]` e l'anno/partecipanti delle tre coppe storiche. Se INCOMPLETE_FIELD, inviare le righe `[og-unemployed-qual-country]` per individuare le definizioni OG mancanti.
3. Ripetere in 2026/27 fino a 1/7/2027; verificare le tre coppe UEFA e nessuna partita già giocata ripetuta.
4. SAVE/CONTINUE in entrambi i mondi: `[og-qual-car] SAVE` + `RESTORE ... CHECKSUM_VERIFIED`; ID e campi per ogni coppa identici prima/dopo.
5. Ripetere passaggio stagione con manager (il percorso OG esistente deve restare invariato); salvare ZIP diagnostico e controllare mismatch calendario/marcatori.
6. NON cancellare CAR precedenti; su errore preservare il vecchio autosave.

## Stato
Codice preparato e test C isolato: SI. Commit finale: da indicare nel log GitHub. Verifica Windows completa: NON PROVATO. Qualificazioni 1996/97 e 2026/27 effettivamente riuscite con tutte le squadre reali: NON PROVATO. No doppia simulazione nei nuovi callback: per costruzione (nessun esecutore delle partite chiamato nel nuovo percorso), ma da confermare sulla diagnostica.
