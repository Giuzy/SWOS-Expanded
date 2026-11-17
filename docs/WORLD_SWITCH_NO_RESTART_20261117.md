# SWOS Expanded — NO RESTART: due banche statiche OG e switch nella stessa sessione

**17 novembre 2026 — preparazione 0.13.67, ramo dev/0.10, base HEAD 6c877e7.**
La build Windows e il gameplay **NON SONO STATI COMPILATI/COLLAUDATI dall'assistente**. L'utente compila e testa su Windows dopo GitKraken.

## Decisione già approvata (AGENTS.md, 14/11; GAME_DESIGN.md)
1. Preparare **entrambi i database statici 1996/97 e 2026/27 al LOADING**, ma installare **un solo namespace di puntatori OG** alla volta. Il mondo dell'effettiva carriera viene scelto prima di NEW CAREER -> START e START crea **una sola istanza residente OG**; non si copia o converte alcuna carriera viva in corso.
2. Lo switch si effettua **senza chiudere il gioco, senza riavviare SWOS_Expanded.exe e senza dipendenze da un secondo EXE**, nel selettore NEW CAREER; scelta futura FOOTBALL WORLD in OPTIONS ancora da progettare/approvare come UI separata.
3. Mondi, formati, squadre, ID e save restano separati: 1996/97 usa TEAM.NNN originali, 2026/27 i file JSON `data_json/worlds/2627/teams/` e il relativo grafo; `AUTO9697.CAR` e `AUTO2627.CAR` non vengono cancellati o sovrascritti dal cambio. `AUTOSAVE.CAR` legacy resta solo recupero in lettura.
4. Vige sempre un **solo** mondo OG per risultati, calendari e stagione. Lo switch statico in NEW CAREER non genera incontri, non azzera/cambia uno stato di carriera salvato e non anticipa partite.

## Verifica MOD 4.1.2 e differenza implementativa
La MOD `aminta/swos-9697-mod` utilizza:
- `tools/careerworld.py::w_ns_set`: per ogni mondo mantiene le righe del namespace contenenti `countriesTable`, `competitionsTable`, `teamsCountryNumbers`; al cambio ripristina le righe e azzera `TEAMSLOADED`, `WTPHYS` affinché due file di stesso ID non vengano confusi. `w_ns_ld` applica il namespace del mondo del save prima del caricamento.
- `tools/swe.c::reg_enter/reg_leave`: installa temporaneamente il namespace della lista visitata e torna a quello precedente.
- Test specifici `tools/hosttest/tests/test_wmodes.py` esercitano attraversamenti e ripristini; non sono prove Windows Expanded.

**Differenza port Expanded:** `World_InstallBaked` sovrascrive anche `CompetitionLeague` e registri globali, `DataJson_InstallCompetitionGraph` sovrascrive oggetti compilati in-place, crea oggetti/registri dinamici e utilizza un `installed` statico monouso; `LeaguePhase_InstallCups` muta la regione Europa e `_careerContests`. Per questo non basta copiare il solo trio di puntatori della MOD.

## Codice 0.13.67: switch senza EXE
- `sorgenti/src/world_switch.c` (NUOVO, compilato dal wildcard di Makefile) conserva **290 aree mutabili dichiarate in `competition_data.h`**, inclusi record di competizione compilati, i `Leagues` / playoff / gruppi, tabelle dei paesi, tabelle competizioni, `_teamsCountryNumbers` e `_careerContests`. Esclude i `*Copy` che contengono dati runtime di una carriera e non devono essere scambiati tra i mondi. Ciascuna banca è una copia byte-faithful di una fase pre-carriera, non un secondo simulatore. Le allocazioni/nome JSON del mondo moderno restano vive (i relativi puntatori vengono ripristinati, non riletti ad ogni switch).
- `FrontEndPreLoopSetup` invoca `WorldSwitch_PrepareAtLoading`: salva la base statica 9697 **prima** di `World_InstallBaked`; controlla `WORLD.ini` e fingerprint prima di preparare il 2627; per quello esegue nell'ordine `World_InstallBaked`, `DataJson_InstallCompetitionGraph`, `WorldCups_Install`, `Career_InstallCopaLibertadoresPresetEntry` e ne salva il namespace. Ripristina il mondo scelto all'avvio. Se pacchetto moderno mancante/incompatibile, segnala 2627 NON DISPONIBILE (non attiva record incompatibili); se fallisce la memoria mantiene solo il boot tradizionale.
- `competition_data.c`: `Comp_ExportWorldRegistry/Comp_ImportWorldRegistry` preservano il registro `compId -> Competition*` dei due mondi, inclusi i template JSON dinamici; non modificano i COPY di carriera, gli algoritmi OG o il formato save.
- NEW CAREER: seleziona `WorldSwitch_SelectBeforeCareer(target)` nel **medesimo processo**, senza `CreateProcessA`, senza `exit()`, senza riavvio SDL e senza variabili ambiente ereditate da un processo figlio. Dopo l'attivazione invalida `_teamsLoaded`, `_lastLoadedExtNumber`, `_teamFileTeamCount` e `_teamFileDataSize`; ogni team file successivo viene ricaricato dal mondo giusto. Il testo dell'azione segnala NO RESTART; dati non pronti => `WORLD DATA UNAVAILABLE`.
- I due mondi non sono installati contemporaneamente nei pointer OG: il banco non attivo è soltanto memoria passiva, non viene avanzato/simulato. Il contesto residente della carriera viene creato successivamente nel normale percorso START.

## Diagnostica 0.13.67
Log attesi in SWOS.log:
- `[world-switch] LOADING STATIC_BANKS prepared=9697:1 2627:1 active=... areas=290 bytes-per-bank=...`
- Ogni switch `[world-switch] IN_PROCESS world=9697|2627 switches=N teams-cache=INVALIDATED static-registry=RESTORED other-world=INACTIVE save=UNTOUCHED`
- `[world-debug] SWITCH IN_PROCESS active=... no-restart`
- Se mancante/mismatched: `[world-switch] 2627 UNAVAILABLE...`, `[world-switch] REFUSED...`; nessun cambio parziale.

## Test Windows richiesti — DIVERSI dai controlli sul sorgente
1. Dopo GitKraken build con toolchain abituale; verificare versione 0.13.67 nel log.
2. Avvio 2627 -> NEW CAREER -> seleziona 9697 -> 2627 -> 9697 -> 2627 **senza chiudere la finestra o generare un nuovo PID/processo**. Dopo ogni scelta aprire la lista delle squadre, verificare paese, conteggio, ID, regolamenti, eventuali seconda-team-file del 2627. Non creare save durante la sola selezione.
3. Da ciascun mondo premere START UNEMPLOYED, verificare sorgente `OG_BINARY` storica e JSON 2627; verificare crash Finlandia 96/97 e `PANIC 0x8885` separatamente (la 0.13.66 aveva solo una preflight di isolamento).
4. Chiudere/riaprire un nuovo eseguibile solo per prova di persistenza: `AUTO9697.CAR` e `AUTO2627.CAR` devono restare distinti e dati compatibili; il cambio nella stessa sessione **non** deve cancellare né convertire un file di salvataggio.
5. Entrare in una vecchia carriera, tornare al TITLE e verificare che il cambio sia possibile soltanto nella configurazione di una **nuova** carriera, non mentre il gameplay usa lo stato residente.
6. Confrontare `_competitionsTable`, `_countriesTable`, `_teamsCountryNumbers`, `compId` delle coppe e struttura delle leghe, inclusi 1996/97 Finlandia e 2026/27 Serie D/Italia secondo i rispettivi dati. Qualsiasi mismatch => NON PROVATO/NO, non registrare PASS.

## Limiti espliciti
- STATIC namespace switch != mondo OG completo: non risolve le Libertadores, la Finlandia 12/14, gli altri formati incompleti, i fixture giornalieri, i save delle coppe residenti e i bug di BACK.
- Questa build non implementa il nuovo controllo grafico finale OPTIONS; l'entry NEW CAREER esistente è riutilizzata senza asset nuovi.
- Cambi di mondo a carriera ATTIVA vietati: solo selettore preliminare NEW CAREER; i salvataggi sono legati al loro mondo e non si convertono.
- La MOD usa un binario DOS patchato mentre Expanded è C/SDL2: si riutilizza l'architettura, non codice di EXE.
- Rischi da testare su Windows: registri e puntatori invalidi, eventuali altri cache di file dopo selezioni, setup dei mondi con pacchetti parziali, versione ABI, riapertura di UI già caricata. Non dichiarare «FUNZIONA» senza prova Windows.

### Diagnostica dell'ultima build
Il parser `sorgenti/tests/audit_windows_runtime_log.c` conta i marker `LOADING STATIC_BANKS` e `IN_PROCESS` in entrambe le direzioni. `WORLD_SWITCH_IN_PROCESS=BOTH_DIRECTIONS_OBSERVED` significa **solo** che il log Windows ha visto cambi in entrambe le direzioni; le verifiche dei database e delle rose vanno fatte separatamente, non sono un PASS implicito. Se non ci sono switch: `NOT_TESTED`.
