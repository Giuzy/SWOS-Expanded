# SWOS Expanded — priorità 17 novembre 2026, build 0.13.66

**Repository:** Giuzy/SWOS-Expanded, dev/0.10. Base 0.13.65 commit 2965fd86664a1990478c3f58b0ff646fbd5a0c0e. **Questa build non è compilata/collaudata su Windows dall'assistente.** Compilazione e test gameplay appartengono all'utente. La versione cambia solo perché sono state modificate funzioni di codice.

## Diagnostica reale delle sessioni Windows dell'utente
- **SESSIONE 21:** SWOS.log di 242 byte, non dimostra un run START fino alla simulazione.
- **SESSIONE 22 / 0.13.65:** build Windows riuscita; `[world-team-source] world=9697 file=12 source=OG_BINARY count=16`. Due squadre escluse dal campionato nel file (FC OTP e KUPS, entrambe league=4); **altre quattordici** riportano league=0. Il record OG Finlandia divisione 0 richiede **dodici**. L'originale selettore termina con `[verify-fill] panic 0x8885: extNumber=12 leagueFilter=0 expectedCount=12 actualCount=14 tfCount=16`. La causa NON è un fallback JSON: la 0.13.64 aveva già corretto quel punto. Non assumere quali siano i due club ulteriormente da togliere; serviranno il mapping/regolamento OG originale e confronto dei veri dati 1996/97.
- L'audit formati allegato alla sessione 22 analizza 66 record league, 76 cup, 14 tournament e 7 playoff del JSON storico; `COMP_FORMAT_DATA_VALIDATION=PASS` indica SOLO validità strutturale. `COMP_FORMAT_OG_LEAGUE_HANDOFF`, `DOMESTIC_CUPS_RUNTIME`, `CONTINENTAL_CUPS_RUNTIME`, `SAVE_LOAD_RUNTIME` = NOT_TESTED. Non è un audit esaustivo delle regole 2026/27 e non prova il gameplay.

## Interventi REALMENTE PREPARATI nella 0.13.66
- **Crash START 1996/97:** `career_status.c::Expanded_OGWorld_AdvanceLeague` effettua una verifica pre-OG dei club per divisione dai veri TEAM.NNN; se l'input non corrisponde al record nativo, registra `[og-native-league-preflight] UNSUPPORTED ... expected= ... actual= ...` e marca la singola lega come **NON RESIDENTE**, senza chiamare l'originale selettore che fa panic. `career_init.c` registra LEAGUE_UNSUPPORTED senza contarlo come giocato o completato. La carriera può continuare con le leghe valide. **Non è correzione del formato Finlandia: 12 contro 14 rimane aperto e non si inventano gare o classifiche.** La preflight può far emergere altre incompatibilità da risolvere.
- **Picker DEBUG WORLD / OFFERTA:** `menu_system.c::expanded_picker_wrap` attraversa ogni entry effettivamente visibile e abilitata, salta le invisibili e torna circolarmente in cima/fondo, per colonna quando il layout lo richiede, senza toccare CLASSIC. Il movimento ridisegna tutto il menu, incluso il NUOVO focus. Questo può rendere selezionabili le voci UEFA/Libertadores precedentemente saltate, ma non le rende automaticamente RESIDENTI.
- **VIEW SQUAD OFFERTA:** in `input_sdl.c`, B del controller chiude prima il popup X se presente; una nuova pressione B esce dalla rosa read-only verso OFFERTA, senza attraversare due livelli insieme. Rimangono da verificare focus e mouse.
- **Versione:** 0.13.66, file `sorgenti/include/build_info.h`; nessuna modifica ad asset, gameplay della partita, save esistenti.

## Lista integrale richieste e stato della 0.13.66
| # | Priorità | Intervento | Stato |
|---|---|---|---|
| 1 | ALTA | Passaggio 1996/97 ↔ 2026/27 senza riavvio e doppio database pronto al LOADING | **NON IMPLEMENTATO**. `expanded_new_career.c::debug_switch_world` chiama ancora CreateProcess; `World_InstallBaked` muta 65 record di leghe OG, offset globali team e puntatori di paesi, e il successivo caricamento JSON/registri richiede ripristino completo del contesto. Cambiare World_Set a caldo senza snapshot di tutti i record crea commistioni. Richiede intervento separato e test incrociato prima di abilitare. Non effettuare switch in un salvataggio attivo. |
| 2 | MASSIMA | Regole/partecipanti/turni/promozioni e qualificazioni 96/26 | **AUDIT PARZIALE**: fonte e formati diversi confermati; 1996 Finlandia incompatibile; JSON audit sessione 22 isolato non sufficiente per regole dei due mondi. Evitare modifica automatica dei formati. |
| 3 | MASSIMA | Crash START soprattutto 1996/97 | **PROTEZIONE PRELIMINARE**: preflight individua e isola mismatch invece di entrare in panic; non corregge il record Finlandia, né prova tutti gli altri percorsi START. |
| 4 | ALTA | DEBUG WORLD selezione tre UEFA e Libertadores | **NAVIGAZIONE CORRETTA NEL CODICE**; voci non residenti restano esplicitamente NOT_RESIDENT, senza simulazione automatica. |
| 5 | ALTA | Liste DEBUG WORLD/OFFERTA, wrap/BACK/B | **MODIFICA CODICE** per UP/DOWN ciclici, focus; BACK e B precedenti preservati, test Windows pendente. |
| 6 | ALTA | VIEW SQUAD navigazione/focus/input | **PARZIALE**: gerarchia B-popup/rosa corretta. Focus e tutte le modalità/input da provare. |
| 8 | ALTA | Rimozione dipendenza riavvio / EXE cambio mondo | **PENDENTE**; nessun ENGLISH.EXE esterno richiesto per Libre/conti (eliminato 0.13.64), ma riavvio tramite SWOS_Expanded.exe ancora presente per world-switch. |
| 9 | ALTA | Diagnostica separata per i due mondi | **AGGIUNTO** preflight runtime con atteso/effettivo e distinzione RESIDENT/UNSUPPORTED. Test statici precedenti non diventano gameplay PASS. |
| 10 | ALTA | Guide | **AGGIORNATE** con sessioni 21/22, dettagli delle scoperte e limiti. |

## Controlli dopo GitKraken (Windows dell'utente)
1. Compilare la 0.13.66 col flusso abituale `AVVIA_DIAGNOSTICA.bat` / `AVVIA_SWOS.bat`, senza cambiare toolchain. Verificare GCC completato e versione.
2. START disoccupato 1996/97: cercare `[og-native-league-preflight] ... country=12 division=0 expected=12 actual=14`, poi verificare che non appaia `PANIC 0x8885` e che il calendario continui. **Finlandia resterà NON RESIDENTE**, dunque non dichiarare mondo completo.
3. START occupato 1996/97: verificare separatamente. START 2026/27: controllo su leghe e coppe native; nessun fallback di ID storico.
4. DEBUG WORLD e OFFERTA: su Europa provare le 3 UEFA; in Sud America la Libertadores; UP/DOWN dall'ultima alla prima voce, controllo della cornice focus; B/ESC torna un livello senza flicker. Con viewer assente il messaggio `NOT RESIDENT` è corretto, non PASS del torneo.
5. OFFERTA -> VIEW SQUAD: X informazioni, B chiude popup; seconda pressione B torna all'offerta. Test mouse e joypad Switch Pro.
6. 5+ anni FORWARD TIME; salvataggi separati AUTO9697.CAR/AUTO2627.CAR; no cancellazione/overwrite automatici. Conservare log e ZIP delle diagnostiche.
7. NO RESTART: **NON PROVARE COME RISOLTO**. Resta riavvio dell'eseguibile finché il doppio ripristino delle tabelle OG non sarà implementato.

## Checklist
- Commit codice/guide pubblicato: verificare SHA dopo il push.
- Compilazione Windows: **NON PROVATO per 0.13.66**.
- Crash 1996/97 eliminato senza perdita di campionato: **NO** (protezione disponibile, campionato incompatibile isolato).
- Switch senza riavvio: **NO**.
- Regolamenti completi di entrambe le epoche: **NON PROVATO**.
- Picker voci visibili e focus: **CODICE SÌ / GAMEPLAY NON PROVATO**.
- VIEW SQUAD B-popup: **CODICE SÌ / GAMEPLAY NON PROVATO**.
- Salvataggio e ripresa: **NON PROVATO**.

