# SWOS Expanded 00.13.194 — FINISH SEASON OG senza slideshow / controllo cambio manuale

19 novembre 2026. Ramo: `dev/0.10`. Base: `5a0229332050d18bf9b1a5d2ce3485a9b068d1ea` (00.13.193).

## Evidenza Windows dalla DIAGNOSTICA_SESSIONE_1(20261119-091533).zip (build 00.13.193)

- **Unemployed, 2026/27:** `[og-season-transition] CLOSE year=2026 resident-leagues=134 ... invalid=0`, `OPEN year=2027 resident-leagues=134 invalid=0`. Calendario residente avanzato, non dimostra SAVE/CONTINUE o tutte le qualificazioni.
- **Unemployed, 1996/97:** `CLOSE year=1996 resident-leagues=88 ... invalid=0`, `OPEN year=1997 resident-leagues=88 invalid=0`; `[og-unemployed-qualification] ... status=FIELD_INCOMPLETE`. Il rollover del mondo e' osservato, qualificazioni complete **NON confermate**.
- **Employed, 2026/27:** nella nuova annata un contesto OG per `country=44 division=3 year=2027` aveva `comp.base.numTeams=12` e `teamCount=11`. `BLOCK_INVALID_TEAM_COUNT` nella prima giornata utile, `NATIVE_WORLD_NOT_SYNCHRONIZED`; il vecchio CAR viene protetto dal guard.
- **Employed, 1996/97:** crash `[PANIC] reasonCode=0x5555`; il codice OG `DIY_ValidateLeagueScratchBuffer` emette 0x5555 se il saldo dei movimenti promozione/retrocessione del team file caricato non e' zero. Non equivale a una prova della specifica squadra o divisione difettosa.

## Analisi del codice e correzione

**Difetto nella 00.13.193:** due rami di `FINISH SEASON` evitavano di chiamare `ShowDiyLeaguesTournamentMenu` per non mostrare classifiche. Il callback NON e' solo grafico: chiama **`DIY_SortLeagueTableByStandings`**, che ordina gli indici di classifica utilizzati da `DIY_UpdatePromotionRelegationScratchCounts`, qualificazioni e pesi di mobilita'. Saltarlo elimina un effetto OG indispensabile, potendo causare squilibri. Questo e' un errore di implementazione verificabile staticamente; il nesso preciso con entrambi i fallimenti Windows deve ancora essere collaudato.

- **`career_season.c`, `career_status.c`:** ripristinate le chiamate native al menu del campionato durante il passaggio di stagione, compreso l'ordinamento OG. Non si disattivano qualificazioni, ordinamento o callback, e non si crea un secondo esecutore di gare.
- **`menu_system.c`:** solo in `_gameType==4` e `EXPANDED_WORLD_OG_ONLY`, `controlMask=0xffff` passa immediatamente al callback originale senza il **hold artificiale di 50 frame per menu**. La funzionalita' di auto-uscita OG e' preservata; CLASSIC/DIY/SEASON e Amiga mantengono il loro comportamento.
- **`platform_sdl.c`:** durante `_seasonUpdateInProgress` in CAREER OG-only, SDL continua a pompare input/eventi e audio ma NON aggiorna la texture o presenta ogni schermata intermedia. Il precedente frame resta stabile fino al ritorno alla UI normale; contatore `[og-season-ui] HEADLESS_NATIVE_MENUS` segnala i frame evitati. Non altera il loop a 50 Hz delle partite.
- **`career_status.c`:** nuova verifica in fase di creazione del residente: se il numero originale di squadre selezionate differisce dal `numTeams` OG, `[og-resident-id] INITIAL_TEAM_SELECTION_MISMATCH` interrompe la costruzione PRIMA di salvare un contesto incoerente. Non aggiunge la dodicesima squadra, non modifica classifiche e non tenta di risimulare gare.
- **`diy_competition.c`:** un eventuale `0x5555` nel solo percorso CAREER Windows registra `FILE_BALANCE_MISMATCH` e fino a 12 delta per ID globale; mantiene originale `PanicHaltLoop`. L'errore non e' nascosto ne' interpretato come PASS.
- Nessun file CAR riscritto deliberatamente, nessuna nuova versione del salvataggio, nessun cambio a classifiche gia' storicizzate e nessun commit su `main`/release.

## Verifiche e prossimi test Windows (l'utente compila)

| Controllo | Esito |
|---|---|
| Codice ripristina callback e ordinamento OG | SI — ispezione statica |
| Rimozione hold di 50 frame solo CAREER OG-only | SI — ispezione statica |
| Rendering nascosto con eventi SDL conservati | SI — ispezione statica |
| Preflight 12/11 e log 0x5555 senza falsi risultati | SI — ispezione statica |
| HEAD/versione e singolo commit remoto | Da verificare dopo commit |
| Compilazione Windows 00.13.194, warning | NON PROVATO |
| Nessuna schermata intermedia, velocita' effettiva | NON PROVATO |
| FINISH SEASON manager 2026/27 | NON PROVATO |
| FINISH SEASON manager 1996/97 | NON PROVATO |
| UNEMPLOYED 2026/27 e 1996/97, secondo ciclo | NON PROVATO (solo prima stagione vista sulla 00.13.193) |
| Qualificazioni, promozioni/retrocessioni e cup continuity | NON PROVATO |
| AUTO2627.CAR e AUTO9697.CAR SAVE/CONTINUE coerenti | NON PROVATO |

Test prioritari: compila con i BAT ufficiali, verifica subito eventuali warning; ripeti FINISH SEASON 2026/27 e 1996/97, controlla data civile 1 luglio, visibilita' del menu, log `[og-season-ui]`, `[og-resident-id]`, `[og-season-scratch]`, `[og-manual-season]`; verifica 11/12 e 0x5555; poi salva, chiudi, CONTINUE, confronta risultati/fixture e campionati residenti. Se un contesto manca, mantenere il CAR precedente, non inventare squadre o risultati.

**Il test automatico e l'analisi strutturale dei sorgenti non sono una prova di compilazione Windows o gameplay.**
