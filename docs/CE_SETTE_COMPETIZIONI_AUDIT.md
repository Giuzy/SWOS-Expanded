# Sette competizioni confermate nel Competition Engine — audit e stato (25/11/2026)

Base: `dev/0.10` HEAD f44d3c12, v00.13.300 → v00.13.301 (solo dati + test). Livello di prova: test ISOLATI con motore CE reale, OG simulato (mock). Build Windows, OG QUICK/50 Hz e gameplay: NON PROVATI.

## Stato per competizione

| # | Competizione | Stato | Dato / motivo |
|---|---|---|---|
| 7 | Supercoppa Italiana (id 1310978) | **FUNZIONANTE via dati** (test isolato PASS) | Definizione in `ce_competitions.json` (type 1, finale unica, Inter `20:1` – Lazio `20:9`, 22/12/2026 GAME_TEMPLATE, cambio edizione 21/12, supplementari e rigori). Edizione 2026 giocata, archiviata a scadenza (21/12/2027); edizione 2027 creata e BLOCCATA (`ENTRANTS`: nessuna regola partecipanti per le edizioni successive) = dipendenza dichiarata, nessun partecipante inventato. Nessuno slot OG. |
| 6 | Serie A | da collegare | `leagues.json` ha già 20 squadre, 2 giri; la scheda riga 186 (block131DateAudit) è un audit parziale delle date (120 gare osservate al 23/11, 40 giorni), non un calendario. Dipende dalle divisioni italiane a gironi paralleli (`league_families_need_rule=1`, Fase 3). |
| 1 | A-League Men (riga 207) | da collegare | scheda approvata: 12 club, 26 gare a testa in 28 giornate (22+4), 156 gare regolari, top 6 con 7 gare di finali, calendario nominale 16/10–giugno. `leagues.json` oggi: 12 squadre, 2 giri (22 gare). Le 4 gare aggiuntive richiedono una fase TABLE con coppie elencate (156 incontri nominali: 66 coppie, 42 volte x2 e 24 volte x3) o una regola generica non ancora approvata. |
| 2 | NPL NSW (riga 208 dei dati) | da collegare | 16 club, 30 turni, 6 qualificate alle finali (5 gare). |
| 3 | NPL Victoria (riga 209 dei dati) | da collegare | 14 club, 26 turni, finali 5 gare. |
| 4 | NPL Queensland (riga 210) | da collegare | 12 club, 22 turni, finali QF/EF/PF/GF. |
| 5 | Australia Cup (id 2884866) | **FUNZIONANTE via dati** (test isolato PASS) | 32 partecipanti reali (20 nel database + 12 squadre TEST `44:55..44:66`), 5 turni a gara singola (31 gare: 16/8/4/2/1), DRAW per R32 e R16, tabellone per QF/SF/F, finestre GAME_TEMPLATE_2026 dalla programmazione pubblicata, cambio edizione 29/01, supplementari e rigori, 5 sostituzioni in 3 finestre. Edizione 2026 giocata e archiviata; edizione 2027 BLOCCATA (`ENTRANTS`: preliminari regionali non implementati, nessun fallback casuale). NON rappresentati: sostituzione extra e finestra extra ai supplementari (oltre il limite panchina OG), ospitalità del club federale contro un A-League. |

## Anomalie dei dati di ricerca da confermare (V da lettura delle schede)
1. `OCEANIA/AUS/2627/LEAGUE_DIVISION.json`: i nomi di `competitionNaming` delle righe 208 e 209 sono OBSOLETI rispetto ai dati: la riga 208 si chiama «National Premier League South Australia» ma `roundTemplateAudit.identity` e il formato (16 club, 30 turni) sono quelli del NSW; la riga 209 si chiama «NPL Men's NSW» ma il formato (14 club, 26 turni) è quello del Victoria (nessuna identità esplicita). Le capienze 12+16+14+12 = 54 coincidono con `AUSTRALIA.json` (54 squadre). Non importo finché non confermi: A-League (div. 0), NSW 16, Victoria 14, Queensland 12.
2. I dati tecnici della Supercoppa (id, club, date) NON sono in nessuna scheda del repository (cercati `1310978`, «Supercoppa»): sono usati dal mandato dell'utente; la scheda `ITA/2627/LEAGUE_CUP.json` riga 323 è la Coppa Italia Serie C.
3. A-League: criteri delle 4 gare aggiuntive, precedenza AFC e panchina condizionale sono ancora P nelle schede.

## Dipendenze mancanti
- **Coppa Italia** non è un'edizione CE: senza di essa la Supercoppa 2027 non può calcolare il secondo partecipante (vincitore Coppa Italia; in caso di doppietta, finalista). Serie A (rank 1) è esprimibile con `next_entrants`.
- **Panchina OG vs regolamento moderno** (Supercoppa 7 in panchina, 5 sostituzioni): il CE usa bench 5 / subs 3 (OG nativo), differenza dichiarata, NON modificata senza approvazione.
- **Squadre TEST**: regola amministrativa non ancora implementata; punteggio TEST contro reale in attesa di conferma (proposta 0-3 con marcatura ADMIN_TEST). TEST contro TEST: 1-0 alla prima della fixture (approvato).
- **12 squadre TEST dell'Australia Cup**: da aggiungere ad `AUSTRALIA.json` oltre le 54 (nessuna sostituzione), con chiavi CE `44:55..44:66` e rose TEST; richiede un controllo di capienza (`TEAM_FILE_MAX_TEAMS`) e dell'importatore (`Team_IsOutsideLeague`, `league >= divisions` = esclusa dai campionati).

## Aggiornamento v00.13.301 (25/11/2026, dopo le conferme dell'utente)
- **Regola TEST approvata:** TEST contro squadra reale = vince la reale 3–0 (0–3 se la TEST è in casa); TEST contro TEST = vince UNA squadra scelta dalla COPPIA e dall'edizione (hash), non dalla posizione in fixture, quindi stessa vincitrice in andata e ritorno, 1–0. Modulo generico `expanded_ce_admin.c/.h`, dati e policy in `data_json/worlds/2627/competitions/test_clubs.json`, gancio nel percorso OG QUICK (`ExpandedUniversal_NativeRunOriginal`, partite CPU/VIEW RESULT/DEBUG), flag persistito `ADMIN_TEST` (0x04) nel risultato CAR. Rimuovere un club dall'elenco = rosa reale inserita. **Aperto:** PLAY MATCH interattivo contro una TEST resta la partita OG 50 Hz originale (nessun override): da decidere.
- **12 squadre TEST** in `AUSTRALIA.json` (ordinali 54–65, `league 4` = fuori da ogni divisione, rose «TEST» identiche, UUID unici; i 54 membri dei campionati sono invariati). Entrano solo nel catalogo universale (`import_file`), mai nel mondo-campionato.
- **Righe australiane confermate:** 208 = NPL NSW (16), 209 = NPL Victoria (14), 210 = NPL Queensland (12); A-League 12.
- Test: `run_test_expanded_ce_admin.sh`, `run_test_expanded_ce_real_competitions.sh` (JSON reale: Supercoppa e Australia Cup; 31 gare, 12 amministrative = 9 TEST-vs-reale + 3 TEST-vs-TEST nel mock), `run_test_expanded_ce_data.sh` (4 definizioni JSON), suite motore, live e catalogo PASS. NON PROVATO: avvio gioco con 66 squadre australiane, gancio nel percorso nativo OG, build+gameplay Windows.
- **Ancora da fare:** A-League (22+4), NPL NSW/Victoria/Queensland e Serie A (campionati: definizioni in `leagues.json`/`league_calendar.json` da riallineare alle schede approvate).
