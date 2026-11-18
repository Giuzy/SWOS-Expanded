# SWOS Expanded — warning GCC e sostituzione CAR Windows (0.13.90)
Data: 18/11/2026. Base: `dev/0.10`, HEAD `8c6a7aa54e0c8922eb5ef0fb5234aabf1556af54` (0.13.89).

## Osservazioni REALI nelle tre diagnostiche Windows 0.13.89
- `DIAGNOSTICA_SESSIONE_1(20261118-074636).zip`: compilazione completa, **4 warning**: `expanded_career.c:196 -Wmissing-braces` (1), `league_phase.c:267 -Wdangling-pointer` (1), `save_format_json.c:3141,3442 -Wcomment` (2).
- `DIAGNOSTICA_SESSIONE_2(10).zip` e `DIAGNOSTICA_SESSIONE_3(7).zip`: **0 warning nel log**, ma fanno solo ricompilazione incrementale di alcuni file e NON provano un progetto senza warning.
- Le righe GCC della 0.13.89 includono ancora **`-Wno-address-of-packed-member`**. Per questo i 29 warning della 0.13.88 NON possono essere dichiarati realmente eliminati dalla 0.13.89. La 0.13.90 riattiva esplicitamente **`-Waddress-of-packed-member`** nel Makefile. Il cambio CFLAGS aggiorna lo stamp delle opzioni e richiede ricompilazione delle unità; il conteggio effettivo rimane **NON PROVATO** fino al log Windows successivo.
- La sessione 2, mondo **2627**, contiene una singola `[autosave] atomic replace failed (Win32 5)` subito DOPO `SERIALIZED_COMPLETE` e `READY_FOR_ATOMIC_SAVE`. Seguono `[autosave] FAILED ... previous file retained` e `[autosave-title] status=WRITE_FAILED`. Nella stessa sessione ci sono salvataggi successivi riusciti. `Win32 5` è **access denied**: il log non identifica chi bloccasse il file; scanner/indicizzazione/condivisione sono ipotesi, non diagnosi dimostrate.
- Nei log presenti vi sono preflight di compatibilità e caricamenti riusciti; non affermano da soli che OG, risultati o l'intero CAR siano stati confrontati campo per campo.

## Modifiche circoscritte 0.13.90
1. `expanded_career.c`: inizializzazione zero di `HallRecord top[5]` con `{0}`, senza cambiare record o ordinamento.
2. `league_phase.c`: `s_country` diventa una copia statica di `LEAGUE_PHASE_CLUBS` byte invece di conservare l'indirizzo dell'array `drawn` di stack del chiamante. La copia avviene a inizio `LeaguePhase_Draw`, con seed e ordinamento identici. Nessuna nuova RNG, simulazione o modifica ai calendari.
3. `save_format_json.c`: eliminate due sequenze commento ambigue; nessuna variazione di serializzazione, CAR o schema.
4. `sorgenti/Makefile`: riattivato l'avviso `-Waddress-of-packed-member`, SENZA disabilitare nuovi warning.
5. `swos_io.c::swos_io_save_atomic`: dopo flush/close della `.tmp`, `MoveFileExA(MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)` viene ritentata **solo** per `ERROR_ACCESS_DENIED(5)`, `ERROR_SHARING_VIOLATION(32)`, `ERROR_LOCK_VIOLATION(33)`: quattro pause (50,100,200,400 ms), massimo **cinque tentativi totali**. Errore diverso: fallimento immediato. Successo dopo retry: log `RECOVERED`; fallimento finale: codice Win32, numero di tentativi, cleanup solo della `.tmp`; **mai cancellato o rinominato il CAR precedente** prima del successo. Vecchio schema/load e altri target invariati.

## Controlli realmente eseguiti
- **SI — ispezione sorgenti e log**: individuati 4 warning effettivi e flag GCC soppressivo, esaminati tutti e 3 i nuovi ZIP e la catena `SaveJson_WriteCareer` -> `swos_io_save_atomic` -> `SaveCareerFile`/menu.
- **SI — test C limitati su Linux, NON il gioco**: `-Wall -Wextra -Werror -Wdangling-pointer -Wmissing-braces -Wcomment -Waddress-of-packed-member` su tipi/uso dei campi di prova: PASS; fixture con mock Win32 sulla politica retry (successo immediato, 2/4 fallimenti temporanei, fallimento persistente, errore non ritentabile): PASS_MOCK_ONLY. Il mock è dimostrativo, non prova che `MoveFileExA` reale riesca.
- **NON PROVATO**: compilazione integrale MinGW64 Windows 0.13.90, warning dopo riattivazione del flag, simulazione realistica di lock Win32 e successo del retry, nuovo save/CONTINUE e roundtrip 96/26, controller, gameplay, classici, replay.
- **NON COPERTO**: riabilitare in massa gli altri `-Wno-*` storici del progetto, audit globale dei cast packed (in particolare `FindTeamInDIY`) o riprogettazione completa dell'I/O. La richiesta qui è riferita ai warning visibili e al warning packed precedentemente mascherato.

## Verifica Windows obbligatoria
1. Sincronizzare `dev/0.10` con GitKraken, leggere Build `0.13.90` e compilare con `diagnostica/COMPILA_SWOS.bat` (nessuna toolchain nuova). Controllare le righe GCC: **`-Waddress-of-packed-member` attivo**, `-Wno-address-of-packed-member` assente, compilazione completa e numero di warning totali; inviare il nuovo `COMPILAZIONE_0140.log` in caso di warning/error.
2. Proteggere esternamente `save/AUTO2627.CAR`, `save/AUTO9697.CAR`, `save/AUTOSAVE.CAR`. Usare solo CAR di prova. Provare 2026/27 e 1996/97: aprire, salvare, TITLE, CONTINUE dopo riavvio, confronto data/rose/risultati, avanzamento leghe/coppe.
3. Leggere in `SWOS.log` eventuali `atomic replace busy`, `RECOVERED` o `atomic replace failed Win32=...`; non dichiarare che `Win32 5` è risolto se non viene riprodotto e verificato su Windows.
4. Verificare CLASSIC/DIY/SEASON, formazione, sorteggio 36 club, mouse/pad e partita OG 50 Hz. **STOP** su save scritto male, cambio data, risultato duplicato o nuovo crash; non sovrascrivere CAR affidabili.

Questa patch è una mitigazione del fallimento transitorio Win32 e una correzione mirata dei warning osservati: non certifica assenza futura di errori esterni di file sharing o un numero finale di warning pari a zero.
