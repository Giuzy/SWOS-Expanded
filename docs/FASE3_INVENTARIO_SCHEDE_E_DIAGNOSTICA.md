## 24/11/2026 — Estensione documentale Fase 3 punto 5: CT e nazionali (DECISIONI, NON CODICE)

**Fonte utente:** chat 24/11/2026 e Issue #1 commenti 7118989627, 7119080740, 7119108083; testo completo nella sezione iniziale di `docs/GAME_DESIGN.md` preparata come blob `bc295228d4d810d25e24c2559c3431fa66f733aa`. È un aggiornamento del **design**, non cambia lo stato v298 di §5/§7: **nazionali/CT ancora solo inventario, FASE 3 NON chiusa**.

**Vincolante:** incarico unico CT **oppure** club; partenza CT e offerte federali per fama; contratto federale nello stesso flusso dei club; federazione esonera soprattutto per risultati con fans/federazione; sponsor federali; allenamento analogo al club quando i convocati sono disponibili; CPU convoca in automatico; identità/eleggibilità della nazionalità corretta. Per la lista convocabili mostrare sempre **club di provenienza**, nome, ruolo e gli stati presenti. Il gioco propone la rosa iniziale automaticamente tra i migliori eleggibili; il CT può modificare, **svuotare completamente**, **ripristinare proposta automatica**. **Niente ricerca con testo da digitare**. Numero convocati da regolamenti reali delle singole competizioni. Date di notifica e termine liste da fonti applicabili (FIFA 15 giorni per notifica, NON per forza pubblicazione lista). Tornei mondiali/continentali/nazionali validi per epoca; nessuna Nations League inventata nel 9697.

**NON approvato / non definito:** UI definitiva (filtri ruolo, ordinamento e paginazione proposte soltanto), schema/adapter nazionale, numeri contratto, stipendio e durata, budget/finanze/sponsor, formula quantitativa per esonero, limiti giocatori per specifico torneo, integrazione eleggibilità beyond nazionalità database. **Non aggiungere campi JSON CE**, non fare passare selezioni nazionali come club key, non alterare UI OG senza consenso; verificare OG Season/roster, writer/reader, CAR e due mondi.

**RG-T non ancora eseguiti:** START CT, offerte cambio impiego senza doppio incarico, elenco nazionalità corretta positivo/negativo, auto-pick/svuota/ripristina, scadenze, CPU, gare, ranking, esonero, SAVE/EXIT/CONTINUE, 9697 e CLASSIC/DIY/SEASON. Nessuna prova Windows/gioco. **Punto 5 = NON IMPLEMENTATO** anche dopo questo documento.

# FASE 3 — Correzioni diagnostica 00.13.297, inventario competizioni, schede nello schema CE esistente (v00.13.298)

**Stato:** source + test isolati Linux + compilazione Linux (tutti i `.c`; falliscono solo i noti `netplay*.c` per il sottomodulo GGPO assente). **Windows, gameplay, SAVE/EXIT/CONTINUE reali: NON PROVATI.** Fase 3 **NON completata**: vedi §7.
Base: `dev/0.10` HEAD `82453cc` (v00.13.297). Precheck RG eseguito: RG-FASE2-CE-UNIVERSAL, RG-CE-NO-FIXTURE, RG-FASE2-LIBERTADORES-FIRST-DATES, SAVE.md (contenitori CAR).

## 0. Regola utente 24/11/2026 sulle schede (vincolante)
Le schede delle competizioni dei mondi 9697 e 2627 si scrivono **solo con i campi CE già esistenti** (`id,name,country,type,source,first_edition,next_entrants,phases[kind,groups,meetings,qualify,pairing,byes,higher_home,window],table{points,tiebreak,provenance},match{legs,knockout,extra_time,penalties,away_goals,bench,subs,windows,halftime_free,et_extra_subs,et_extra_windows,discipline,provenance}`; per le leghe `substitutes`/`max_subs` di `leagues.json`). Niente nuove proprietà, categorie o meccaniche. Dato essenziale mancante → competizione bloccata con il meccanismo esistente (`missing`), mai valori inventati. Limite OG: 5 riserve; regolamenti con panchina più lunga vanno **segnalati**, non ridotti in silenzio. `windows=0` nel CE significa «nessun limite di finestre» (è la regola OG: `ExpCeRules_FromOgLeague` lo imposta con provenienza `OG_NATIVE`), **non** «dato sconosciuto». `OG_NATIVE`, `GAME_TEMPLATE` e `VERIFIED` non sono equivalenti. Una regola reale non descrivibile dallo schema si documenta qui come limite + decisione necessaria.

## 1. Diagnostica `DIAGNOSTICA_SESSIONE_2(20261124-003441)` — esito per anomalia

| | Anomalia | Causa | Livello | Azione v298 |
|---|---|---|---|---|
| A | 4 edizioni `DATE_PLAN` (38,38,26,22 giornate) | `RUN1/SWOS.log` righe 57–137: la CAR `BEFORE_RUN1` (scritta dalla v296) aveva le edizioni 2026 di Brasile (comp 90, div1+div2, id 794427649/794427905) e India (comp 95, div1+div2, id 1281294593/1281294849) bloccate DATA_MISSING; il CONTINUE v297 le ha rivalutate (`EDITION_REEVALUATED`) e ha creato le edizioni **2027-01-01** solo con l'orologio al **07/02/2027**; `generate_phase` pianifica da `today+1` e 38/26/22 giornate non entrano più nella finestra (01-01..06-03, 01-01..04-03). In una carriera nuova v297 le stesse edizioni vengono pianificate prima dell'apertura (sonda: 134/134 divisioni pianificabili dal primo giorno di finestra; margini minimi 2–24 giorni). | **V** | Nessuna giornata tagliata, nessun riposo ridotto, nessuna partita di recupero inventata. Se un'edizione di lega è confermata **a finestra già aperta** e non entra, l'**intera famiglia** (tutte le divisioni ancora PARTICIPANTS senza fixture) passa alla **finestra legale successiva con gli stessi club** (`EDITION_DEFERRED_TO_NEXT_WINDOW`). Fail-closed se una divisione è già pianificata, se l'anno successivo esiste già o se la finestra non era ancora aperta (allora è un vero problema di dati e resta DATE_PLAN). Al CONTINUE i blocchi DATE_PLAN di leghe PARTICIPANTS **senza alcuna fixture** vengono rilasciati (la CAR `AFTER_RUN2` dell'utente rientra in questo caso). **Decisione sportiva da confermare:** per quelle leghe la stagione 2027 non si gioca (CAR legacy riparate). Alternativa non applicata: giocare la 2027 in finestra allungata (date inventate). **Dato da rivedere (non modificato):** la finestra Brasile `01-01..06-03` di `league_calendar.json` (copiata dai codici OG in v280) non corrisponde alla Série A reale 2026 (38 giornate fino al 02/12, blob lotto A `9b0eb97e8248e1a7c71846e612ebcef7241b4e2e`); India `01-01..04-03` idem da verificare. |
| B | 14 edizioni `DATA_MISSING` | Italia (country 20, comp 60/61, 14 divisioni: A, B, C×3, D×9): totali promozioni/retrocessioni bilanciati ma livelli a gironi paralleli; il CE di Fase 2 accetta solo catene semplici. | **V** | Resta DATA_MISSING (motivo esatto nel log). **Scoperta:** i dati del mod 2627 contengono già una regola OG di distribuzione (`world_data.c: kWorld2627Groups` per Italia, Argentina, Australia, El Salvador; porting di `gplay.c` in `src/league_groups.c`: chi resta conserva il girone, chi sale/scende va nel girone della propria area, altrimenti nel più vicino con posto). Usarla nel CE è una **meccanica nuova** → non implementata senza approvazione. **Sospetto (I):** i codici area della tabella Italia valgono 17..248 (es. 0xA0/0xA2/0x91, 0xF0..0xF8) e `apply_one` li confronta grezzi con il numero di gironi (`area < L->groups`), quindi tutti i club in movimento partono dall'ultimo girone; sembra un byte impacchettato del mod non decodificato (nibble basso = girone?). Da verificare sul `gplay.c` originale prima di qualsiasi uso; riguarda anche il fine stagione OG legacy 2627. |
| C | CAR `SWOSZC4A` poi `SWOSZC3A` | `RUN2/SWOS.log`: 4 salvataggi; lo scrittore a snapshot si attiva **dopo 3 salvataggi identici al byte** (`[car-snapshot] ACTIVATED`, riga 5919) → i salvataggi di RUN2 usano ancora lo scrittore inline a frame singolo (`SWOSZC3A`); nella sessione precedente lo snapshot era già attivo (`SWOSZC4A`, blocchi indicizzati). Tutte le decodifiche `hash=VERIFIED`, `EQUIVALENCE roster=PASS ce=PASS`, `[car-writer] DURABLE_AND_BYTE_VERIFIED`. Entrambi i formati sono leggibili. | **V** | Comportamento previsto, **non regressione**. Nessuna CAR modificata. Restore certificato: ancora `NOT_AUTOMATICALLY_PROVEN` (CONTINUE_CONFRONTO). |
| D | Audit diagnostico | `tests/audit_windows_runtime_log.c` è centrato sul mondo OG nativo: per una carriera CE i contatori OG restano 0 per costruzione → `NO_OG_START_EVIDENCE`. | **V** | Aggiunta sezione CE separata: `CE_RUNTIME`, `CE_BLOCKS` per tipo (DATA_MISSING/DEPENDENCY = dati o decisione; altri = errore motore/dati), `CE_INTEGRITY`, `CE_VERDICT` (`FAIL` / `ENGINE_OR_DATA_BLOCKS_PRESENT` / `NO_WORLD_FAULT_OBSERVED (gameplay not validated)`), `OG_NATIVE_WORLD_CHECKS=NOT_APPLICABLE_CE_WORLD`; per mondo CE `WINDOWS_OG_INTEGRATION=NOT_APPLICABLE`, `OVERALL=INCOMPLETE` o `FAIL`, **mai PASS**. Il codice di uscita del runner (`exit=1` con `new-unclassified=28`) è preesistente. |

## 2. Inventario (Punto 1) — generato dai dati, non scritto a mano
Generatore: `sorgenti/tools/competition_inventory.py` (`--list` per l'elenco voce per voce). Stati: `CE_RUNTIME` = definizione caricata ed eseguita dal CE; `CE_BLOCKED` = caricata e bloccata localmente; `OG_NATIVE_ONLY` = presente in tabelle OG/mod ma non definizione CE (2627: non eseguita in carriera CE; 9697: carriera OG legacy invariata). Una voce «rappresentata» **non** è una competizione storicamente certificata.



Generato da `sorgenti/tools/competition_inventory.py`; nessun dato web.

### Mondo 9697 — 185 voci

| categoria | stato | voci |
|---|---|---:|
| CONTINENTAL_CLUB | OG_NATIVE_ONLY | 4 |
| FRIENDLY_OR_CUSTOM | OG_NATIVE_ONLY | 3 |
| LEAGUE_CUP | OG_NATIVE_ONLY | 3 |
| LEAGUE_DIVISION | OG_NATIVE_ONLY | 88 |
| NATIONAL_CUP | OG_NATIVE_ONLY | 58 |
| NATIONAL_TEAM_QUALIFIER_GROUP | OG_NATIVE_ONLY | 9 |
| NATIONAL_TEAM_TOURNAMENT | OG_NATIVE_ONLY | 13 |
| PROMOTION_PLAYOFF | OG_NATIVE_ONLY | 7 |

### Mondo 2627 — 351 voci

| categoria | stato | voci |
|---|---|---:|
| CONTINENTAL_CLUB | CE_BLOCKED | 1 |
| CONTINENTAL_CLUB | OG_NATIVE_ONLY | 12 |
| LEAGUE_CUP | OG_NATIVE_ONLY | 13 |
| LEAGUE_DIVISION | CE_RUNTIME | 134 |
| LEAGUE_PARALLEL_GROUPS_SPEC | OG_NATIVE_ONLY | 4 |
| LEAGUE_PLAYOFF_PLAYOUT_SPEC | OG_NATIVE_ONLY | 35 |
| LEAGUE_SPLIT_SPEC | OG_NATIVE_ONLY | 38 |
| NATIONAL_CUP | CE_BLOCKED | 1 |
| NATIONAL_CUP | OG_NATIVE_ONLY | 88 |
| NATIONAL_TEAM_QUALIFIER_GROUP | OG_NATIVE_ONLY | 2 |
| NATIONAL_TEAM_TOURNAMENT | OG_NATIVE_ONLY | 7 |
| REGIONAL_STATE_LEAGUE | OG_NATIVE_ONLY | 13 |
| SUPERCUP_OR_INTERCONTINENTAL | OG_NATIVE_ONLY | 3 |

### 2627 — esistenti nella realtà ma NON rappresentati (solo nome)

- SUPERCUP_OR_INTERCONTINENTAL: UEFA SUPER CUP — UNAVAILABLE
- SUPERCUP_OR_INTERCONTINENTAL: FIFA CLUB WORLD CUP — UNAVAILABLE
- SUPERCUP_OR_INTERCONTINENTAL: NATIONAL SUPER CUPS (per country) — UNAVAILABLE
- CONTINENTAL_CLUB: OFC PRO LEAGUE (2026 edition ended before START) — UNAVAILABLE
- NATIONAL_TEAM_TOURNAMENT: UEFA / CONCACAF NATIONS LEAGUE — UNAVAILABLE
- LEAGUE_DIVISION: LIGUE 3 FRANCE (professional from 2026/27) — UNAVAILABLE
- LEAGUE_DIVISION: J3 LEAGUE JAPAN — UNAVAILABLE


Note: le 134 divisioni 2627 sono eseguibili dal CE ma l'Italia (14) si blocca dal 2027 (§1 B); 90/91 famiglie sono catene semplici. Copa Libertadores compare due volte (definizione CE bloccata + tabella OG nativa): è la stessa competizione reale, nessuna delle due gira in parallelo. Nel 9697 `_euroCup` (Coppa dei Campioni), `_cupWinnersCup`, `_uefaCup`, `_copaLibertadores` sono le continentali di club OG.

## 3. Schede (Punto 2) — stato nello schema CE esistente
- **Leghe (134 divisioni 2627):** `table.points` da `points_for_win`, `match.subs`=`substitutes`, `match.bench`=`max_subs`, provenienza **OG_NATIVE** (valori del mod, non verificati contro i regolamenti 2026/27). Distribuzione 2627: 88 record 3 sostituzioni/5 panchina, 2 record 3/3, 1 record 4/5; 9697: 59 record 3/5 con 3 punti, 4 record 3/5 con 2 punti, 2 record 3/3, 1 record 4/5. `windows=0` = regola OG senza finestre. I regolamenti reali moderni (5 cambi in 3 finestre più intervallo, panchine da 7 a 12) **non sono stati verificati competizione per competizione**: sono `UNAVAILABLE`; dove la panchina reale supera 5 il regolamento è **incompatibile con il limite OG** e va segnalato, non ridotto.
- **COPA BOLIVIA (2949378), COPA LIBERTADORES (13107458):** invariate (dati utente c40d124), bloccate dal loro `missing`. Libertadores `match.provenance=GAME_TEMPLATE` (5/3): **non** verificato; la provenienza vale per l'intero blocco `match`, quindi un valore verificato e uno di template nello stesso blocco non possono essere marcati `VERIFIED`.
- **Continentali, coppe nazionali, statali, nazionali (OG_NATIVE_ONLY):** nessuna definizione CE aggiunta in questo passo. Le regole native esistono nelle tabelle del mod (giri, gare singole/doppie, supplementari, rigori), ma ingressi e qualificazioni non sono mappati a chiavi club CE `country:slot`: un loro inserimento oggi sarebbe solo una scheda bloccata.
- **Ricerca sudamericana verificata** (blob Issue #1 `06041ffbe3978c845e282952f75ad9bba4ed6978`, `ffb0a15287e89dbeabcd70a149f0211134695e08`, recuperati e letti integralmente): gironi reali Libertadores 2026 (8×4) e qualificate agli ottavi, gironi Sudamericana 2026 e prime/seconde, 8 coppie playoff (da riconciliare con la pagina ufficiale), Belgrano campione Apertura (24/05/2026), Olimpia campione Apertura (classifica G19 provvisoria, non finale). **Non usati a runtime:** mancano chiavi club CE, risultati partita per partita, classifiche complete, regola di sorteggio degli ottavi. Restano dati di riferimento.

### Regole reali NON descrivibili dallo schema attuale (limiti, decisione necessaria)
1. Due tornei per stagione (Apertura/Clausura) con classifica annuale cumulata (Argentina, Paraguay, Colombia…): lo schema ha una sola tabella per fase; nel mod esistono tabelle `Combined`.
2. Retrocessione per *promedio* su 3 stagioni (Argentina, Paraguay): nessun campo; il mod ha `kWorld2627Promedio43`.
3. Split campionato/salvezza (`kWorld2627Splits`, 38 voci) e playoff/playout di lega (`kWorld2627Playoffs`, 35 voci): il type0 CE non li modella.
4. Gironi paralleli con distribuzione geografica (Italia, Argentina, Australia, El Salvador): §1 B.
5. Spareggio fra competizioni diverse (Sudamericana: seconde contro terze dei gironi Libertadores): le sorgenti `next_entrants` sono LEAGUE_RANK/CUP_WINNER/CUP_RUNNER_UP.
6. Promozioni condizionate da licenza (J.League) e numero variabile di promossi.
7. Panchine reali > 5 (limite OG).
8. Competizioni per nazionali: il CE gestisce solo chiavi club; convocazioni, CT e calendario FIFA sono fuori schema.

## 4. Prima edizione e successive (Punto 3) — stato
- Le fixture del catalogo con data precedente allo START (`past_at_start`) **non vengono inserite né simulate** (`expanded_universal_live.c:318`): il passato pre-START è `UNAVAILABLE`, l'edizione è `partial` e non promuove/qualifica (v297). Risultati reali storici fino al 30/06/2026: **nessuno importato** (non presenti nei dati in forma club-key).
- Dopo il 30/06/2026 le date sono generate dal CE dalle finestre (`GAME_TEMPLATE`); i risultati sono solo OG. Le edizioni a cavallo d'anno non si azzerano al 30/06 (scadenza per edizione).
- 9697: dati OG originali invariati; nessuna data/formato cambiato; la carriera 9697 resta OG legacy (CE attivo solo per 2627: `ExpandedUniversalLive_Active`).

## 5. Adapter, nazionali, scenari manager (Punti 4–6) — stato
- **Punto 4:** nessun nuovo adapter in questo passo (vincolo utente §0). Gli adapter esistenti (type0/1/2, KO, gironi, qualificazioni) restano quelli della Fase 2; nessun ABI OG, CAR o cache cambiati salvo la sezione UL01 già esistente.
- **Punto 5 (nazionali/CT):** **non** preparato oltre l'inventario (9 voci 2627, 22 voci 9697 fra tornei e gironi di qualificazione); nessuna interfaccia aggiunta.
- **Punto 6 (START con club, disoccupato, FORWARD, offerte, esonero, CONTINUE):** coperto solo dai test isolati esistenti (`test_expanded_universal_live`, `test_expanded_midseason_join`, `test_og_offer_rollback`, `test_save_car_zstd`) più il nuovo scenario CAR v297 → CONTINUE. Verifica Windows per scenario: **NON PROVATA**.

## 6. Test (Linux, isolati)
| Test | Esito |
|---|---|
| `test_expanded_ce_engine`: nuovi `CAR297_DATE_PLAN_BLOCK_RELEASED_AT_CONTINUE`, `DATE_PLAN_LATE_CONFIRMED_FAMILY_DEFERRED_SAME_CLUBS_NO_SHORTENING` (12 fixture per divisione tutte nella finestra 2028, nessun club due volte nello stesso giorno), `DATE_PLAN_REPAIRED_BEFORE_WINDOW_PLANS_SAME_YEAR` (controllo negativo: nessun rinvio se la finestra non è aperta), `ExpCe_PlanPhaseDays` positivo/negativo | PASS |
| Suite `run_all_diagnostic_tests.sh` | 27/27 PASS (runner `exit=1` preesistente per 28 non classificati) |
| `audit_windows_runtime_log` su log sintetico CE | sezione CE emessa, `OVERALL=INCOMPLETE`, nessun PASS |
| Compilazione Linux `make X86_64=1 -k` | tutti i `.c` compilano; falliscono solo i noti netplay/GGPO |
| Windows, 50 Hz, QUICK, menu, SAVE/EXIT/CONTINUE reali | **NON PROVATI** |

## 7. Perché la Fase 3 non è completata
Punti 1 (inventario) e diagnostica A–D: fatti. Punto 2: schede complete solo per i campi OG_NATIVE delle leghe; nessuna scheda verificata `VERIFIED` per panchina/sostituzioni reali; continentali/coppe/nazionali senza definizione CE (servono mapping chiavi club e regole verificate). Punti 3–6: stato descritto sopra, in parte già garantito dal motore, in parte non iniziato. Decisioni utente necessarie: (a) conferma del rinvio di famiglia per le CAR legacy (§1 A); (b) uso nel CE della regola OG di distribuzione nei gironi paralleli e verifica della decodifica delle aree (§1 B); (c) come rappresentare i limiti 1–8 di §3 (nuovi campi = estensione dello schema, oggi vietata).

### 24/11/2026 — AUDIT CODICE CT/NAZIONALI (V da sorgente, HEAD 9104b3b, nessuna modifica)
- **L'OG ha già una carriera da CT a incarico unico:** `career_init.c:3601` — se il paese della squadra scelta è 80..85 (regioni), `_isNationalSelector=1`; accettare un'offerta internazionale (`job_offers.c` `checkNational`, `_aInternationalJ_0` «INTERNATIONAL JOB OFFER FROM %a») fa diventare `_selTeamNumber` la nazionale (il club si lascia). Le offerte dipendono da fase casuale, fiducia del presidente (≥0xC8) ed RNG; `_sameNationalCallFlag/_lastNationalityCall` evitano doppioni.
- **Convocazioni OG:** `Career_InitInternationalSquadAndLookupTablesForNewSeason` (career_teams.c:2894) **svuota** 20 posti (nessuna preselezione automatica); il CT aggiunge giocatori con `NationalTeamAddPlayer` → `AddInternationalPlayerHome/Abroad` e li toglie con `NationalTeamRemovePlayers`; limite 20 (`menus.c:3716/7494`). `Career_CopyUnavailablePlayerToCareerTeamSlot` **copia** i dati del giocatore nel record della nazionale (identità duplicata: da verificare per disponibilità/forma condivise).
- **Nel mondo 2627 CE:** `manager_key` (expanded_universal_live.c:72) usa `ExpandedUniversal_NativeTeamKey`, che accetta solo squadre con slot di mondo: per una nazionale restituisce 0 → il CE la tratta come **manager senza club** (nessuna fixture, solo FORWARD). Le competizioni per nazionali sono `NO_WORLD_RESIDENT` (diagnostica 297): **oggi un CT 2627 non gioca nessuna partita**.
- **Mancano per il 2627:** identità di nazionale nel CE (il registro, il ledger e la CAR UE02/UL01 conoscono solo `ExpandedClubKey{world,stable_club}`); partecipanti/sorteggi/gironi reali delle edizioni 2026–2028 (WC 2026 in corso allo START, qualificazioni WC 2030, EURO 2028, Nations League 2026/27, AFCON 2027, Asian Cup 2027, Gold Cup, OFC); regole di lista per torneo.
- **Decisioni bloccanti (richieste all'utente prima del codice):** (1) come rappresentare una nazionale come partecipante CE senza «falsa chiave club» (cambio di struttura del registro e della CAR); (2) fonte e raccolta dei dati delle competizioni 2627; (3) modifiche alla UI convocazioni OG (preselezione automatica, svuota, ripristina, club di provenienza) da approvare graficamente; (4) se nel 9697 (carriera OG legacy, già funzionante con CT) si debba cambiare qualcosa.

### 24/11/2026 — v00.13.299: CT/NAZIONALI, PRIMA IMPLEMENTAZIONE (PARZIALE; Windows NON PROVATO)
Decisioni utente 24/11 (AskUserQuestion): **tipo partecipante esplicito**; **solo motore, dati 2627 dopo** (competizioni reali UNAVAILABLE); **logica convocazioni ora, UI OG dopo approvazione grafica**; **9697 OG invariato**.

**Fatto (source + test isolati Linux):**
- `ExpandedClubKey.kind` (`EXP_PARTICIPANT_CLUB` / `EXP_PARTICIPANT_NATIONAL`, `expanded_competition_identity.h`). Il campo sta in coda alla struttura: i letterali `{world,stable_club}` restano CLUB; 22 letterali in src/test resi espliciti. Una nazionale è `ExpandedNationalKey(world, numero squadra OG)` (file regione 80..85, `stable_club = numero+1`), **mai** uno slot club; `Equal` confronta anche il tipo.
- Ogni edizione ha **un solo tipo di partecipante** (`ExpCeEdition.participant_kind`); club e nazionali mescolati → blocco locale ENTRANTS. Qualificazioni da un'altra edizione ereditano il tipo della sorgente.
- CAR: fixture **UE03** (UE02 + 2 colonne tipo; i salvataggi UE02 si leggono con tipo=CLUB, test dedicato); ciclo di vita **UL02** (tipo per edizione/partecipante + convocazioni; UL01 ancora letto). Controllo copertura rose (`CarCoversClubs`) salta le fixture tra nazionali.
- Convocazioni CE (`ExpCe_Callup*`, stato nello stesso blob UL02 e nello snapshot): proposta automatica tra **eleggibili (nazionalità OG uguale) e disponibili**, ordine OG prezzo → somma attributi → identità stabile, **nessuna soglia minima**; aggiunta manuale con rifiuto per nazionalità errata, indisponibile, doppione, già in un'altra lista, lista piena; rimozione; svuota; ripristina (= nuova proposta automatica). Limite lista = regola della competizione quando nota, altrimenti capacità OG nativa **20** (`menus.c:3716`); la capacità di memoria 64 non è una regola.
- 2627 live: `ExpandedUniversal_NativeTeamKey` mappa una squadra dei file 80..85 alla chiave NAZIONALE → il CT OG (`_isNationalSelector`) è riconosciuto come manager della propria nazionale (prima risultava «senza club»).

**NON fatto / bloccato (dichiarato):**
- **Rose CPU delle nazionali per le partite:** `ExpandedUniversalCatalog_Roster` non restituisce squadre nazionali → una fixture nazionale con avversario CPU viene rifiutata prima del claim (blocco locale `ROSTER_MISSING`, nessun risultato inventato). Serve un adapter che componga il record OG della nazionale dai convocati (modello OG `Career_CopyUnavailablePlayerToCareerTeamSlot`), con decisione su dati condivisi forma/infortuni.
- **Raccolta candidati dal mondo reale** (scansione rose club per nazionalità) e collegamento della lista CE alla schermata OG: non collegati; UI OG invariata (approvazione grafica richiesta).
- **Finestre internazionali, date di notifica/lista, disponibilità presso i club:** nessun dato → non applicate.
- **Competizioni per nazionali 2627** (WC 2026 in corso allo START, qualificazioni 2030, EURO 2028, Nations League, AFCON, Asian Cup, Gold Cup, OFC): **UNAVAILABLE**, nessuna definizione aggiunta, nessun campo JSON nuovo. Oggi un CT 2627 non ha fixture → solo FORWARD.
- **Offerte federali per fama, contratto, esonero, sponsor, allenamento in nazionale:** restano il flusso OG esistente (`job_offers.c checkNational`, incarico unico già OG); nessuna nuova formula; non verificati nel 2627 CE.
- 9697: invariato (carriera OG legacy).

**Test (Linux, isolati):** `test_expanded_ce_engine` +6 PASS (`NATIONAL_EDITION_KO_EXPLICIT_KIND_NO_CLUB_KEY`, `CALLUP_AUTO_BEST_ELIGIBLE_AVAILABLE_OG_CAPACITY`, `CALLUP_MANUAL_WRONG_NATIONALITY_UNAVAILABLE_DUPLICATE_REJECTED`, `CALLUP_CLEAR_RESTORE_ONE_LIST_PER_PLAYER_NO_CLUB_AS_NATIONAL`, `SAVE_EXIT_CONTINUE_NATIONAL_KIND_AND_CALLUPS_UE03_UL02`, `NATIONAL_AND_CLUB_MIXED_EDITION_BLOCKED_LOCALLY`); `test_expanded_car_compact` `UE02_LEGACY_READ_KINDS_CLUB=PASS`; suite 27/27; compilazione Linux OK (salvo netplay/GGPO noti). **NON PROVATI:** START CT nel gioco, offerte/cambio incarico/esonero nel 2627, partite manager/CPU tra nazionali, qualificazioni reali, Windows, CLASSIC/DIY/SEASON in gioco.

### 24/11/2026 — v00.13.300: CARRIERA CT = ROUTINE OG ORIGINALI IN ENTRAMBI I MONDI (CAMBIO DI STRATEGIA UTENTE)
**Decisione utente 24/11:** usare per il 2026/27 le stesse funzioni OG del CT del 1996/97 (offerte, incarico, convocazioni, selezione giocatori, formazione, tattiche, esoneri, navigazione), con UI e comportamento OG; nessun percorso separato; il CE resta l'unica autorità 2627 per calendario/competizioni/risultati/qualificazioni; aggiungere in NEW CAREER l'avvio diretto come CT con soluzione provvisoria minima. Le espansioni concordate (preselezione automatica, svuota/ripristina, club di provenienza, limiti per torneo, finestre) restano **solo documentate** in GAME_DESIGN.md e questa guida.

**Rimosso (duplicava l'OG):** modulo convocazioni CE v299 (`ExpCe_Callup*`, stato, test). UL02 scrive sempre 0 liste; una CAR v299 con liste si decodifica e le liste vengono scartate (`UL02_V299_CALLUPS_DISCARDED`, test dedicato).
**Mantenuto (collegamento CE, non duplicato):** tipo partecipante esplicito (`ExpandedClubKey.kind`), `ExpandedNationalKey`, `participant_kind` per edizione, CAR UE03/UL02, `NativeTeamKey` per i file 80..85.

**Difetti nei percorsi condivisi trovati e corretti (causa I dal sorgente, Windows non provato):**
1. Passaggio d'incarico RG-027 (`career_init.c ContinueCareer`): la verifica post-attach richiedeva `Expanded_OGWorld_ClubBound()` (9697) o un roster club (2627) → accettare un'offerta INTERNAZIONALE finiva in «NEW JOB BIND FAILED» in entrambi i mondi. Ora una nazionale (file 80..85) non richiede il bind di un campionato club; nel 2627 `ExpandedUniversalLive_ManagerBound` accetta la chiave NAZIONALE.
2. Bind della stagione manuale (`og-manual-season`): per una nazionale `Expanded_OGWorld_BindClub` falliva → `bound=0`, stagione non pronta. Ora saltato per le nazionali.
3. `InitCareerClub`: non tenta più il bind di un contesto club per una nazionale (prima: solo log «NO EXISTING CONTEXT»).

**NEW CAREER provvisorio:** il pulsante esistente «START UNEMPLOYED» con FIRE cicla fra tre stati: club (spento) → START UNEMPLOYED → **START AS NATIONAL TEAM COACH**. Nessun nuovo elemento grafico, cambia solo la didascalia. In modalità CT la nazionale è quella della **nazionalità scelta per il manager** (`GetNationalTeamNumber(_plNationality)`), avviata con la stessa `InitCareer` OG (`_isNationalSelector=1`, competizioni OG della nazionale come slot) e lo stesso autosave «first-contract» del club. Se per quella nazionalità l'OG non ha una nazionale → messaggio «NO NATIONAL TEAM FOR THIS NATIONALITY» e ritorno al setup. Il filtro OG `IsTeamCodeOkForNationalSelector` non è usato: è il vincolo temporale delle OFFERTE (stagione ≥3), non un controllo di validità. UI definitiva da approvare.

**Limiti reali (V dal sorgente/dati):**
- **2627, partite:** il CE non ha ancora competizioni per nazionali (dati UNAVAILABLE); gli slot OG della nazionale non sono letti dal flusso 2627 (CE unica autorità), quindi **un CT 2627 non gioca partite** finché il CE non riceve le competizioni. Offerte, incarico, convocazioni/selezione OG, tattiche ed esonero usano le routine OG ma non sono provati in gioco nel 2627.
- **2627, dati nazionali:** il mondo 2627 non ha file regione propri: le nazionali (file 80..85) e la tabella `_nationalTeams` (63 squadre) sono quelle 96/97 (fallback del loader `dj_world_path`). Nazionalità moderne senza voce (es. codici ≥118 nel selettore) → nessuna nazionale. I giocatori convocabili sono comunque quelli del database 2627 (le routine OG «add player home/abroad» leggono le rose del mondo attivo).
- **9697:** stesse routine OG, invariate salvo le due correzioni di bind (1, 2).

**Test:** suite 27/27 PASS; `test_expanded_ce_engine` (tipo nazionale, mixed blocked, SAVE/CONTINUE UE03/UL02, `UL02_V299_CALLUP_RECORDS_DISCARDED_ON_CONTINUE`) PASS; car_compact/universal_car/live PASS; compilazione Linux OK (salvo netplay/GGPO). **NON PROVATI:** START AS NATIONAL TEAM COACH, offerta internazionale accettata, convocazioni OG, partite, esonero, SAVE/EXIT/CONTINUE reale in entrambi i mondi su Windows; CLASSIC/DIY/SEASON in gioco.
