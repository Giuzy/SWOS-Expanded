# SWOS Expanded 00.13.195 — diagnostiche Windows, rollover OG e DEBUG 30 JUNE

Data 19/11/2026. Base ramo `dev/0.10`: `5935d6c2aa09efe6ea295e099d5307107493fee3`, versione 00.13.194. Una sola revisione `00.13.195` richiesta dall'utente.

## Evidenza effettiva dalle due diagnosi 00.13.194

- **Sessione 1, 20261119-094420, mondo 2026/27:** compilazione Windows completata, zero warning/error segnalati nei due log di compilazione. Il primo rollover da disoccupato chiude il 2026 con **134** leghe e apre il 2027 con **134** leghe, contesti invalidi=0. Dopo ingaggio del club `0847`, FINISH SEASON impiega almeno 722 frame di schermate intermedi nascosti; il callback OG termina, ma il rollover fallisce all'apertura dell'edizione 2027: `INITIAL_TEAM_SELECTION_MISMATCH country=8 div=3 selected=23 required=24`. `WORLD TICK FAILED`, `NATIVE_WORLD_NOT_SYNCHRONIZED`, `AUTO2627.CAR` precedente preservato. Le tre competizioni UEFA moderne riportano fonte/entranti 36/36/36 con corrispondenza esatta; Libertadores 2026/27 resta `NOT_RESIDENT`, `CT_IN.nclub=0`, `NOT_MAPPED`.
- **Sessione 2, 20261119-094421, mondo 1996/97:** compilazione Windows completata, zero warning/error segnalati. Il rollover da disoccupato chiude il 1996 con **88** leghe e apre il 1997 con **88** leghe. Qualificazioni ancora incomplete per il campo della Cup Winners' Cup storica, ma da classifiche OG vengono individuati **20 qualificati Libertadores** (log `copa=20`). Dopo l'ingaggio del club `210c`, la funzione legacy di fine anno ricalcola la Coppa (`[cup-conflict] WINNER` diverso da quello già registrato) e poi `DIY_ValidateLeagueScratchBuffer` rileva `FILE_BALANCE_MISMATCH file-country=14 offset=294 count=44 sum-mod256=255` -> `PANIC 0x5555` / Windows `0xc000001d`.
- La diagnostica del 0x5555 della build 194 conteneva anche un difetto: registrava i delta dal puntatore già avanzato a fine buffer, quindi non stampava gli ID effettivamente coinvolti. Corretto il puntatore per la prossima diagnosi.

## Modifiche applicate al codice 00.13.195, ancora da provare su Windows

1. **Unico esecutore OG nel callback manuale:** `career_status.c` ora consuma tramite copia temporanea dei contesti residenti il vero stato OG della lega con classifiche ordinate, della coppa domestica e dei tornei tipo 2 UEFA/Libertadores storica, quando `_seasonUpdateInProgress` e carriera Expanded. I vecchi callback OG di promozioni/qualificazioni continuano a leggere tabelle e vincitori, ma un residente già elaborato non viene rigiocato. Restano i normali callback OG solo per competizioni prive di stato residente, che non hanno risultati da duplicare. La copia non inventa partite, squadre, punteggi o sorteggi e non altera gli stati OG conservati.
2. **Fail-closed diagnostico se la mobilità resta incoerente:** `career_season.c` espone un latch effimero di errore, controllato dopo ogni tappa della procedura. `diy_competition.c` per il solo `FINISH SEASON` di CAREER OG-only scrive l'errore 0x5555 e i primi 12 delta originali e termina senza il trap Windows; il chiamante blocca la nuova stagione prima di `InitializeNewSeason` e mantiene il CAR precedente. CLASSIC e le altre modalità mantengono il comportamento di OG. Questo è un guard di sicurezza, **NON prova che la mobilità sia ormai bilanciata**.
3. **Libertadores storica indipendente:** `Expanded_OGWorld_QualifyUnemployed` può conservare per il 1997 la lista originale di 20 club OG verificati (campioni e seconde delle nazionali CONMEBOL), anche se una distinta lista UEFA storica è incompleta. `Expanded_OGWorld_AdvanceUefa` consente soltanto alla Libertadores storica di preparare la propria successiva edizione residente, lasciando non preparate le altre edizioni UEFA prive dei qualificati reali. Il log distingue `PARTIAL_NATIVE_QUALIFICATION` e `PARTIAL_OG_FIELDS_COPA_READY`. La persistenza e il calendario risultante sono ancora da testare.
4. **Libertadores moderna:** nessun elenco di ingressi 2027 validato nel codice `ct_world_prepare_first`. Il 2026/27 **rimane incompleto**; vietato fabbricare 32 squadre per i gruppi, preliminari o risultati dal nulla. Il contesto OG 2026/27 va collegato solo a seguito di dati reali e formato approvati.
5. **UNEMPLOYED DEBUG 30 JUNE:** la build 194 contiene il pulsante `_menuEntries[14]` e il callback che calcola il 30 giugno 1997/2027 e avanza OG completo con `Expanded_OGWorld_AdvanceToDate`; una whitelist in `MenuProc` bloccava l'ordinal 14. La 195 permette `ord==14`, preservando focus, hitbox, menu e callback OG e senza superare il 30 giugno. Il passaggio al 1 luglio richiede FORWARD TIME ordinario.
6. Nessun formato CAR modificato, nessun nuovo simulatore LWE, nessuna modifica a partita 50 Hz, fisica o CLASSIC/DIY/SEASON. Versione incrementata a 00.13.195.

## Verifiche

| Test | Stato |
|---|---|
| Diagnostiche Windows 00.13.194 ispezionate | SI — due ZIP reali |
| Base ramo/HEAD, sorgenti originali e nuovo flusso OG | SI — audit statico |
| C/preprocessore dei file aggiornati | SI — solo verifica struttura |
| Date del pulsante 30 giugno (header già coperto da test isolato esistente) | SI — ispezione statica, non test gameplay |
| 00.13.195 compilato e senza warning Windows | NON PROVATO |
| Assunzione/fine anno 1996/97 senza crash | NON PROVATO |
| Assunzione/fine anno 2026/27 senza 23/24 | NON PROVATO |
| Coppe e qualificazioni complete in ogni edizione | NO — Libertadores moderna e CWC 96 ancora incomplete |
| SAVE/CONTINUE da Windows e integrità CAR dopo successivo anno | NON PROVATO |
| Test UI Switch Pro, mouse, tastiera, DEBUG 30 JUNE | NON PROVATO |

Prova suggerita: compilazione con BAT ufficiali, controllo zero warning, due nuove carriere di prova separate, primo FINISH SEASON con contratto per mondo, confrontare `[og-yearend-resident] LEAGUE_REUSED/CUP_REUSED/TYPE2_REUSED`, `[og-season-scratch]`, `[og-manual-season]`; controllare 1 luglio esatto e nessun replay/falsa qualificazione. Da UNEMPLOYED provare DEBUG 30 JUNE (30 giugno, NON 1 luglio) e il successivo FORWARD TIME; quindi salvare in CAR separato e testare CONTINUE soltanto quando autorizzato dall'utente.

**Non attribuire PASS ai test Windows della 00.13.195 prima di ricevere log o schermate dal PC dell'utente.**
