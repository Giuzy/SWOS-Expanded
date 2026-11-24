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
