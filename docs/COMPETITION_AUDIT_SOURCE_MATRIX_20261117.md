# Audit comparativo competizioni e collegamenti — 17 novembre 2026
**Build di riferimento:** dev/0.10, 0.13.70_3. **Stato:** audit statico di definizioni e collegamenti; non collaudo Windows. Non alterare regole storiche in base a dati moderni.

## Fonti: quattro ecosistemi, due stagioni
1. SWOS 96/97 GOG/DOS originale, SWOS 2020 e OpenSWOS: tre riferimenti richiesti dall'utente per 1996/97. L'utente conferma Libertadores presente in CAREER in tutti e tre. **Per GOG e SWOS 2020 non sono disponibili nel repository tre dataset di competizioni distinti da confrontare byte-per-byte**: la presente verifica usa le definizioni originali ricostruite in `data_json/competitions/`, il codice OG e i 4 TEAM.NNN OpenSWOS ricevuti. Non dichiarare comparazione integrale dei tre installati.
2. SWOS 96/97 Mod storica 4.1.2 e codice aminta (fonte pubblica https://worldofswos.com/en/ e https://github.com/aminta/swos-9697-mod ): fonte espansa del 1997; dichiara Libertadores, Supercopa, Copa CONMEBOL, coppe asiatiche/africane, CONCACAF, Intercontinentale, Italia C1/C2 e paesi storici aggiuntivi in carriera. **L'archivio binario storico della Mod con ID/partite per tutti i tornei non è qui disponibile**.
3. SWOS Mod 4.1.2 World 2026/27 (https://github.com/aminta/swos-9697-mod/blob/v4.1.2/release/world-2026-27-competitions.md): confronto degli inventari nel repo `docs/reference/mod2627_4_1_2_competitions.tsv`, `data_json/worlds/2627/`, `world_data.c` e implementazioni. 91 paesi, 134 divisioni, 101 coppe domestiche/di lega, 13 statali, 1 Coppa Italia Serie D, 3 UEFA, 12 altre continentali; 10 supercoppe inventariate separatamente.
4. **Non confondere** tre riferimenti storici 96/97 con i due mondi di Expanded. La Mod aggiunge nuove competizioni storiche che GOG/OG di partenza non coprivano: bisogna recuperarle per 1996/97 senza copiare regole 2026/27.

## Conteggi realmente letti dai JSON di Expanded
| Ambito | 1996/97 | 2026/27 | Stato |
|---|---:|---:|---|
| Record League (include CUSTOM in 96) | 66 | 91 | DEFINITI, non ogni lega residente |
| Divisioni nei record (inclusi 3 CUSTOM nel 96) | 91 | 134 | DEFINITE |
| Stessa identità league con dimensioni differenti | 48 | 48 | Cambiare mondo deve cambiare il record; nessun trapianto |
| Record league aggiunti solo alla banca moderna | – | 26 | Alcuni esistevano già nel calcio 1997 ma mancano al database storico Expanded |
| Record solo nella banca storica | 1 | – | CUSTOM non equivale a campionato nazionale |

## Differenze paese-per-paese delle dimensioni nei record comuni
I valori sono il numero di squadre nei gironi/divisioni **nell'ordine dei dati OG**. Non sono una prova di calendari, playoff o partecipanti dei tre installati.
| Record OG | 96/97 | 26/27 |
|---|---|---|
| `_italianLeague` | 18+20 | 20+20+20+20+20+17+17+17+18+17+18+17+17+18 |
| `_albanianLeague` | 16 | 10 |
| `_argentinianLeague` | 20+20 | 30+18+18 |
| `_australianLeague` | 12+12+14+12 | 12+16+14+12 |
| `_austrianLeague` | 10 | 12 |
| `_bolivianLeague` | 12 | 16 |
| `_brasilianLeague` | 24 | 20+20 |
| `_bulgarianLeague` | 16 | 14 |
| `_colombianLeague` | 16 | 20 |
| `_croatianLeague` | 12 | 10 |
| `_ecuadorLeague` | 12 | 16 |
| `_spanishLeague` | 22+20 | 20+22 |
| `_estonianLeague` | 12 | 10 |
| `_frenchLeague` | 20+22 | 18+18 |
| `_germanLeague` | 18+18 | 18+18+20 |
| `_ghanaLeague` | 8 | 18 |
| `_greekLeague` | 18 | 14 |
| `_hungarianLeague` | 16 | 12 |
| `_indianLeague` | 6 | 13+11 |
| `_irishLeague` | 12 | 10 |
| `_icelandicLeague` | 10 | 12 |
| `_israeliLeague` | 16 | 14 |
| `_japaneseLeague` | 14+14 | 20+20 |
| `_lithuanianLeague` | 12 | 10 |
| `_luxembourgLeague` | 12 | 16 |
| `_mexicanLeague` | 20+20 | 18+16 |
| `_malteseLeague` | 10 | 12 |
| `_dutchLeague` | 18+18 | 18+20 |
| `_northernIrishLeague` | 16 | 12 |
| `_norwegianLeague` | 14 | 16 |
| `_newZealandLeague` | 10+10+10 | 12+10+10 |
| `_peruvianLeague` | 16 | 18 |
| `_portugeseLeague` | 18 | 18+18 |
| `_romanianLeague` | 18 | 16 |
| `_russianLeague` | 18 | 16 |
| `_scottishLeague` | 10+10+10+10 | 12+10+10+10 |
| `_elSalvadorLeague` | 10+12+12+12 | 12+12+12+9+10 |
| `_sanMarinoLeague` | 10 | 16 |
| `_yugoslavianLeague` | 10 | 14 |
| `_slovenianLeague` | 16 | 10 |
| `_swedishLeague` | 14 | 16 |
| `_taiwaneseLeague` | 12 | 8+7 |
| `_turkishLeague` | 18 | 18+20 |
| `_ukrainianLeague` | 18 | 16 |
| `_uruguayanLeague` | 12+14 | 16+14 |
| `_americanLeague` | 10 | 30 |
| `_venezuelanLeague` | 22 | 14 |
| `_welshLeague` | 20 | 16 |

## Record presenti soltanto nei dati 2026/27
Sono assenti dal semplice inventario originale di Expanded 1996/97; **non** significa necessariamente che quei paesi/campionati non esistessero storicamente. Vanno confrontati separatamente con le estensioni 1997 della Mod.
| Record | Squadre moderne |
|---|---|
| `_worldAndLeague` | 10 |
| `_worldArmLeague` | 12 |
| `_worldAzeLeague` | 12 |
| `_worldBihLeague` | 10 |
| `_worldCanLeague` | 8 |
| `_worldChnLeague` | 16 |
| `_worldCrcLeague` | 10 |
| `_worldEgyLeague` | 20 |
| `_worldGeoLeague` | 10 |
| `_worldGibLeague` | 11 |
| `_worldGuaLeague` | 12 |
| `_worldHonLeague` | 12 |
| `_worldIrnLeague` | 18 |
| `_worldKazLeague` | 16 |
| `_worldKorLeague` | 12 |
| `_worldKosLeague` | 10 |
| `_worldKsaLeague` | 18 |
| `_worldLieLeague` | 6 |
| `_worldMarLeague` | 16 |
| `_worldMdaLeague` | 8 |
| `_worldMkdLeague` | 10 |
| `_worldMneLeague` | 10 |
| `_worldPanLeague` | 12 |
| `_worldQatLeague` | 12 |
| `_worldTunLeague` | 16 |
| `_worldUaeLeague` | 14 |

Record presenti soltanto in 96/97: `_customsLeague` (20+22+22).

## Coppe e torneo: differenze funzionali certificate dal codice
| Famiglia | 1996/97 Expanded | 2026/27 Expanded | Difetto / prova necessaria |
|---|---|---|---|
| UEFA club | Champions (16 OG type2), Coppa Coppe e UEFA (type1) residenti prima annata | Tre UEFA type2 a 36 club inizializzate prima annata | 1996 Champions ancora difettosa nel calendario dopo ingaggio; qualificazioni anni futuri non collegate |
| Libertadores | `_copaLibertadores` compId 162 type2 a 20 iscritti **dell'edizione 1996**, benché la Mod 1997 utilizzi l'edizione 1997 in carriera | `CT_SPEC` e manifest 47 club ma prima edizione/fixture non collegate | 96 banca storica non ha il record nel registry al LOADING; manca stato residente datato; NON sostituire automaticamente seed 1996 con 1997 |
| CONMEBOL storiche | Supercopa Libertadores, Copa CONMEBOL, Recopa, Interamericana da censire/collegare per date reali | Libertadores, Sudamericana, Recopa | Extra-storiche della Mod non equivalgono alle tre moderne |
| CONCACAF/AFC/CAF/OFC storiche | Coppe club storiche e partecipanti dai dati Mod storico da integrare | Specifiche moderne in `World_ContiCups`, 12 tornei extra-UEFA | Nessuna prova completa di contesto OG residente per-fixture |
| Brasile statali | Non adottare record 2026 nel 1996 senza fonti | 13 definizioni `World_States` / `states.c` | **MANCANTI dal calendario mondiale residente** (verificato utente), non bastano simulazioni di fine stagione |
| Coppa Italia Serie D | Non importare torneo moderno nel 1996 | `World_CupD` e `world_cupd.c` | **MANCANTE dal calendario mondo OG** (verificato utente); slot manager != competizione mondiale |
| Supercoppe | Solo se storicamente esistenti nell'edizione pertinente | 10 nomi nel manifest minimo + altre da verificare | Manca verifica residenza/ricorrenza per paese/anno |
| Nazionali | Record OG World Cup, Europei, Copa América, Gold Cup, African Cup, Asian Cup, Oceania e qualificazioni | La Mod 4.1.2 NON ha ancora rose nazionali 2026/27 complete | Definizione non prova calendario/fixture/ripresa; evitare false coppe annuali |

**Club moderni:** la Mod dichiara 2.385 record (2.003 nelle divisioni, 119 soltanto negli statali, 104 fuori dalle categorie, 159 ospiti continentali). L'inventario Expanded precedente di 1.995 club riconosciuti **non è un totale omogeneo**: non sottrarre e chiamare la differenza “squadre mancanti” senza audit dei file e degli ID.

## Audit dei collegamenti — cosa è verificato e cosa no
- `world_switch.c`: due banche statiche separate, storica salvata prima che il grafo Copa dinamico venga installato nella moderna. Possibile causa dell'assenza del record storico; un collegamento non può essere dichiarato corretto finché non è confermato il mapping del registrar e dei pointer OG.
- `world_data.c`: generato dai JSON moderni. L'installazione delle competizioni segue l'epoca; l'identità di club è `file:ordinal`, con `World_FileCountry`, `World_Slot` e secondo file per oltre 256 club. Controllare le identità dei partecipanti di OG coppe e qualificazioni, non solo il nome della squadra.
- `career_init.c` / `career_status.c`: `Expanded_OGWorld_AdvanceLeague` e `Expanded_OGWorld_AdvanceCup` popolano leghe/coppe domestiche; `Expanded_OGWorld_AdvanceUefa` gestisce solo prima edizione UEFA, non qualificazioni successive. Libertadores storica NON collegata a `expanded_og_world_run_year`; nel viewer manca adattatore dedicato.
- `save_format_json.c`: `living_world` JSON NON costituisce snapshot integrale di `s_expOG`, `s_expOGCups` e `s_expOGUefa`. Verifica post-restart non provata: da implementare prima di certificare continuità.
- Dalla 0.13.70_2 la diagnostica `[og-no-data]` e `[og-no-data-summary]` rileva definizioni/contesti mancanti (per nazionali usa REVIEW se la data non è stata verificata). **Non valuta ancora correttamente ogni regolamento interno**: servono verifiche reali delle partite e confronto fonti.

## Griglia di verifica prima di PASS — per ciascuna epoca
Dati e dimensioni dei record; numero squadre e identificativi anche multi-file; partecipanti iniziali e per stagione; gruppo e calendario; data OG e durata edizioni oltre il 1 luglio; promozioni/retrocessioni/spareggi; qualificazioni internazionali; coppe e nazionali solo quando previste; fixture e risultato per singola partita; save/restore RNG, rose, tabelle, storico dopo processo terminato; classifiche/bind manager senza reset. Ogni voce deve riportare SOURCE_CONFIRMED, CODE_CONNECTED, WINDOWS_TESTED separatamente.

**Conclusione:** l'audit è AVVIATO e ha differenze reali; non è “completo” fino ad acquisire/verificare dataset indipendenti GOG/SWOS 2020/OpenSWOS e gli stati internazionali della Mod storica, oltre ai test Windows.
