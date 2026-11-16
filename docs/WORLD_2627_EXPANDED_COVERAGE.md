# SWOS Expanded 0.13.55/0.13.56 vs Mod v4.1.2 — copertura verificata

## 16/11/2026 — perimetro aggiornato dopo analisi e interruzione chat

L'inventario 2026/27 della Mod 4.1.2 e' SOLO la fonte di partenza, non la copertura massima del progetto. La carriera Expanded deve integrare anche le competizioni del 1996/97 e del 2026/27 esistenti realmente, incluse quelle nate come PRESET; classificare per CLUB / NAZIONALI, evitare doppie Champions, conteggi e calendari anacronistici. Le 10 supercoppe elencate nella reference non esauriscono quelle nazionali presenti davvero in Asia, Africa e altre regioni. [Inventario e regole 2 mondi](COMPETITIONS_TWO_WORLDS_SPEC.md).

Versione **0.13.59 — patch da PROVARE**: tentativo di collegare 3 UEFA come OG type-2, prima annata 36 reali ciascuna, con snapshot/advance per native phase e viewer read-only; `expanded_og_world_run_year` richiama `Expanded_OGWorld_AdvanceUefa`. Non confondere `ct_world_prepare` già esistente nella 0.13.56 con un executor completato: alla Libertadores **manca CT_IN iniziale reale**. `ct_world_advance_month` non va lanciato senza qualificati e senza risoluzione del doppio motore. Escluso il catalogo internazionale testuale dal menu OFFERTA: sono solo gli stati OG effettivi a dare fixture, risultati e standings. Compatibilità manager/FASE e save non certificati; test Windows e 70 anni necessari.

Data: 16/11/2026. Baseline GitHub dev/0.10 HEAD 262a4566. Non è prova gameplay 0.13.56.

## Regole di lettura
- **DEFINITO**: record/fonti presenti. **RESIDENTE OG**: stato di competizione da START che avanza solo alle sue date. **TESTATO**: osservato nel test Windows pertinente.
- I 134 campionati e le 101 coppe domestiche risultavano residenti nella sessione Windows 6 della build 0.13.55; formule, ID multipli, continuità di save e date restano da verificare singolarmente.
- **MANCANTE NEL MONDO RESIDENTE** non vuol dire assente nel database.

## Differenze più importanti
| Categoria | Mod 4.1.2 | Dati Expanded | Residenti OG su START | Da fare |
|---|---:|---|---|---|
| Divisioni/gironi | 134 | 134 | 134 osservati nella 0.13.55 | Formule locali e storico |
| Coppe nazionali/lega | 101 | 101 | 101 osservate nella 0.13.55 | Fixture, passaggi e save |
| Stati Brasile | 13 | sì | NON PROVATO / non integrato | Inizializzazione e date OG |
| Coppa Serie D | 1 | sì | NON PROVATO / non integrato | Inizializzazione e date OG |
| UEFA | 3 | sì | **MANCANTI** | Stato residente, qualificazioni, calendario, viewer |
| Altre coppe continentali | 12 | sì | **MANCANTI** | Stato residente, avanzamento OG, viewer |
| Supercoppe | 10 | definizioni non sufficienti | **MANCANTI/INCOMPLETO** | Inizializzazione/slot e risultati |

## Elenco nomi delle competizioni extra domestiche ancora da rendere residenti
- **MANCANTE NEL MONDO RESIDENTE** — Brasile — Paulista
- **MANCANTE NEL MONDO RESIDENTE** — Brasile — Carioca
- **MANCANTE NEL MONDO RESIDENTE** — Brasile — Mineiro
- **MANCANTE NEL MONDO RESIDENTE** — Brasile — Gaucho
- **MANCANTE NEL MONDO RESIDENTE** — Brasile — Paranaense
- **MANCANTE NEL MONDO RESIDENTE** — Brasile — Catarinense
- **MANCANTE NEL MONDO RESIDENTE** — Brasile — Goiano
- **MANCANTE NEL MONDO RESIDENTE** — Brasile — Paraense
- **MANCANTE NEL MONDO RESIDENTE** — Brasile — Baiano
- **MANCANTE NEL MONDO RESIDENTE** — Brasile — Cearense
- **MANCANTE NEL MONDO RESIDENTE** — Brasile — Pernambucano
- **MANCANTE NEL MONDO RESIDENTE** — Brasile — Alagoano
- **MANCANTE NEL MONDO RESIDENTE** — Brasile — Mato-Grossense
- **MANCANTE NEL MONDO RESIDENTE** — Italia — COPPA ITALIA SERIE D
- **MANCANTE NEL MONDO RESIDENTE** — CHAMPIONS LEAGUE
- **MANCANTE NEL MONDO RESIDENTE** — EUROPA LEAGUE
- **MANCANTE NEL MONDO RESIDENTE** — CONFERENCE LEAGUE
- **MANCANTE NEL MONDO RESIDENTE** — COPA LIBERTADORES
- **MANCANTE NEL MONDO RESIDENTE** — COPA SUDAMERICANA
- **MANCANTE NEL MONDO RESIDENTE** — RECOPA SUDAMERICANA
- **MANCANTE NEL MONDO RESIDENTE** — CONCACAF CHAMPIONS CUP
- **MANCANTE NEL MONDO RESIDENTE** — AFC CHAMPIONS LEAGUE ELITE
- **MANCANTE NEL MONDO RESIDENTE** — AFC CHAMPIONS LEAGUE TWO
- **MANCANTE NEL MONDO RESIDENTE** — AFC CHALLENGE LEAGUE
- **MANCANTE NEL MONDO RESIDENTE** — CAF CHAMPIONS LEAGUE
- **MANCANTE NEL MONDO RESIDENTE** — CAF CONFEDERATION CUP
- **MANCANTE NEL MONDO RESIDENTE** — CAF SUPER CUP
- **MANCANTE NEL MONDO RESIDENTE** — OFC CHAMPIONS LEAGUE
- **MANCANTE NEL MONDO RESIDENTE** — INTERCONTINENTAL CUP
- **MANCANTE NEL MONDO RESIDENTE** — ITALIAN SUPER CUP
- **MANCANTE NEL MONDO RESIDENTE** — SPANISH SUPER CUP
- **MANCANTE NEL MONDO RESIDENTE** — TURKISH SUPER CUP
- **MANCANTE NEL MONDO RESIDENTE** — FA COMMUNITY SHIELD
- **MANCANTE NEL MONDO RESIDENTE** — GERMAN SUPER CUP
- **MANCANTE NEL MONDO RESIDENTE** — TROPHEE DES CHAMPIONS
- **MANCANTE NEL MONDO RESIDENTE** — SAUDI SUPER CUP
- **MANCANTE NEL MONDO RESIDENTE** — CAMPEON DE CAMPEONES
- **MANCANTE NEL MONDO RESIDENTE** — BRAZILIAN SUPER CUP
- **MANCANTE NEL MONDO RESIDENTE** — UEFA SUPER CUP

## Confronto dei 91 paesi (dati domestici)
| Paese (ID OG) | Divisioni Mod | Divisioni Expanded | Coppe Mod | Stato dati/residenza |
|---|---:|---:|---:|---|
| England (8) | 4 | 4 | 2 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Italy (20) | 14 | 14 | 2 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Spain (35) | 2 | 2 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Germany (14) | 3 | 3 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| France (13) | 2 | 2 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Portugal (29) | 2 | 2 | 2 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Holland (25) | 2 | 2 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Belgium (2) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Turkey (38) | 2 | 2 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Czech Rep. (6) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Greece (15) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Poland (28) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Denmark (7) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Norway (27) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Cyprus (5) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Switzerland (37) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Austria (1) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Scotland (33) | 4 | 4 | 2 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Sweden (36) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Croatia (4) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Israel (19) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Hungary (16) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Ukraine (39) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Serbia (41) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Romania (30) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Slovenia (34) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Azerbaijan (53) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Slovakia (78) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Bulgaria (3) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Eire (18) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Iceland (17) | 1 | 1 | 2 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Armenia (52) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Moldova (92) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Finland (12) | 1 | 1 | 2 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Kosovo (88) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Kazakhstan (86) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Bosnia-Herz. (54) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Latvia (21) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Faroe Is. (11) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Malta (24) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Liechtenstein (90) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Estonia (10) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Albania (0) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| N. Macedonia (93) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Lithuania (22) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| N. Ireland (26) | 1 | 1 | 2 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Gibraltar (63) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Andorra (47) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Belarus (76) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Luxembourg (23) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Montenegro (94) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Georgia (61) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Wales (40) | 1 | 1 | 2 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| San Marino (32) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Russia (31) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Usa (73) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Mexico (60) | 2 | 2 | 0 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Canada (56) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Costa Rica (58) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Guatemala (68) | 1 | 1 | 0 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Honduras (70) | 1 | 1 | 0 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Panama (95) | 1 | 1 | 0 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| EL Salvador (51) | 5 | 5 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Surinam (66) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Brazil (46) | 2 | 2 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Argentina (43) | 3 | 3 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Colombia (49) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Uruguay (71) | 2 | 2 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Ecuador (50) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Chile (48) | 1 | 1 | 2 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Paraguay (64) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Peru (65) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Bolivia (45) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Venezuela (77) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Saudi Arabia (89) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Japan (55) | 2 | 2 | 2 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| South Korea (87) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| China (57) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Qatar (96) | 1 | 1 | 2 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| U.a.e. (98) | 1 | 1 | 2 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Iran (74) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Australia (44) | 4 | 4 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Taiwan (67) | 2 | 2 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| India (75) | 2 | 2 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| New Zealand (62) | 3 | 3 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Egypt (59) | 1 | 1 | 2 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Morocco (91) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Tunisia (97) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Algeria (42) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| South Africa (69) | 1 | 1 | 2 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |
| Ghana (79) | 1 | 1 | 1 | Conteggio presente; runtime domestico 0.13.55 osservato, dettaglio da provare |

## Carriera nazionali — decisione approvata
**INIZIA COME CT**: alla creazione carriera Expanded si può scegliere una nazionale iniziale (in aggiunta all'inizio da club o disoccupato). Mantenere **tutti gli algoritmi OG**: convocazioni, rose internazionali, qualificazioni, incontri, competizioni, offerte di nazionali durante carriera club, esonero e cambio incarico. Il mondo OG esiste già a START e la firma da CT aggancia il manager SENZA resettare data, risultati o stagione. Il menu di start da CT è **APPROVATO / NON IMPLEMENTATO**; 1996/97 ha percorso OG da CT, mentre la Mod 2026/27 4.1.2 non contiene ancora le nazionali: monitorare v5.0 (Nations League, Europei 2028, Mondiali 2030, offerte e convocazioni).

## Evoluzione future Mod/SWOS
Ogni mondo/versione futura avrà un manifest versionato di ID, competizioni, formati, semantica degli eventi e schema di save; nuove release non entrano automaticamente nel motore se richiedono nuove callback. Preservare Classic, DIY, Season, fisica 50 Hz, retrocompatibilità e diritto d'uso dei dati. Nessuna duplicazione degli eventi: solo OG.

## UI e pulizia
Offerta club: usare lo stesso `_squadMenu` OG+Expanded con stelle e snapshot read-only, nascondere/disabilitare tasti gestionali. Non eliminare l'OG VIEW SQUAD comune a CLASSIC; eliminare soltanto il percorso duplicato dell'offerta. BAT root `PULISCI_OBSOLETI_SWOS.bat`: lista chiusa di asset `_OBSOLETO`, anteprima e conferma ELIMINA, stadi e save esclusi. Conservare AGENTS.md/PROJECT_STATE.md/IMPROVEMENTS.md in root, documentazione storica in docs/archive e CSV in docs/reference.

## Test
`sorgenti/tests/run_test_mod2627_coverage.sh` segnala ogni divisione mancante per country ID, definizioni continentali, stati Brasile e cupD. Lo stato giocato delle coppe continentali resta **NON PROVATO** finché l'OG non produce veri fixture e save/load riscontrabili da una sessione Windows.

## 0.13.57: visible catalogue only, NOT resident OG competitions
The 2026/27 offer VIEW COMPETITIONS path now shows 3 UEFA and 12 extra-UEFA names as DEFINITIONS ONLY before opening the existing national OG viewer. It does not create fixtures, participants, match dates or results. Missing resident states (Brazil 13 states, Italian Serie D Cup, supercups, UEFA and continental) remain to be integrated and tested on Windows. A visible catalogue is not proof of a completed integration.

## 0.13.58 - scorer UI and coverage, Windows pending
The read-only native league viewer now retains TOP GOAL SCORERS even without an old team selection cache. Per-country/division/year `NO_PLAYER_SCORERS` diagnostics flag leagues with scored matches but zero player goals; own goals remain possible. This does not initialize nonexistent cup/continental scorer contexts. `patch_title_b_01063.sh` accepts the 0.13.57 additional offer B=BACK guard instead of aborting Windows build before GCC. Windows tests of all 134 divisions and individual cup formats remain pending.
