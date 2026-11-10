# SWOS Expanded — Registro obsolescenza grafica

**Data:** 10 novembre 2026 · **Fonte:** albero GitHub `Giuzy/SWOS-Expanded`, ramo `dev/0.10`, HEAD `f33041d` (ricontrollare prima della pulizia).  
**Scopo:** distinguere **grafica vecchia da rifare** e **file realmente eliminabile**. Non è una lista di cancellazione eseguibile.

## Regola di classificazione

- `FINAL_GRAPHICS` = grafica finale da proteggere: TITLE, NEW CAREER, LOADING. È ancora possibile correggere difetti richiesti senza riprogettare.
- `OBSOLETE_VISUAL / KEEP_RUNTIME` = grafica della fase precedente da rifare, **ma attualmente o potenzialmente necessaria** a menu, fallback, codice o script. Non cancellare prima del nuovo asset + aggiornamento riferimenti + test.
- `ALREADY_NAMED_OBSOLETE / VERIFY_ORPHAN` = il nome file contiene già `OBSOLETO`. Non prova che sia inutilizzato; controllare tutti i riferimenti.
- `SHARED / DEPENDENCY_REVIEW` = componenti usati da più menu; non cancellare globalmente durante la riprogettazione di un singolo menu.
- `ORPHAN_VERIFIED` = nessun riferimento runtime/build/script, già sostituito, verificato: **unica categoria autorizzabile alla cancellazione tramite BAT**.

## A. Grafica finale protetta

- `assets/title/title_background_approved_2026-11-08.png` — TITLE.
- `assets/title/swos_expanded_logo_approved_2026-11-08.png` — logo canonico TITLE.
- `assets/title/buttons/title_primary_blank.png`, `title_secondary_blank.png`, `fullscreen_expand.png`, `windowed_collapse.png` — elementi del TITLE da verificare nella build ma **non da eliminare per vecchiaia**.
- `assets/career/new_career/` — asset NEW CAREER attivi e di riferimento, inclusi flag, checkbox, sfondo e pannelli; non tutta la directory è indiscriminatamente eliminabile.
- `assets/fonts/new_career/` — font bitmap associati NEW CAREER.
- `data/expanded/expanded_loading_bg_approved_2026-11-10.png`, `assets/loading/loading_fill.png`, `assets/loading/loading_panel.png` — LOADING, preservare.

## B. Schermate con grafica da rifare — esempi di file reali

**Tutti gli asset sotto sono da considerare visivamente superati, non automaticamente cancellabili.**

| Menu/sistema | Esempi rilevati nel repository | Stato e condizione di pulizia |
|---|---|---|
| BUSINESS | `assets/business/business_ui_approved_2026-11-07.bmp`; `data/expanded/club_business_bg.raw`; `data/expanded/club_stadium_1.raw` … `club_stadium_5.raw` | `OBSOLETE_VISUAL / KEEP_RUNTIME`: il Business e lo stadio potrebbero caricarli ancora. Gli sprite stadio sono dati dinamici: non rimuoverli senza sostituto. |
| CALENDAR | `data/expanded/expanded_calendar_bg.raw` | `OBSOLETE_VISUAL / KEEP_RUNTIME`: codice menu corrente lo carica. |
| UNEMPLOYED | `data/expanded/expanded_unemployed_bg.raw` | `OBSOLETE_VISUAL / KEEP_RUNTIME`: fallback e renderer vanno aggiornati prima. |
| CLUB OFFER | `data/expanded/expanded_offer_bg.raw` | `OBSOLETE_VISUAL / KEEP_RUNTIME`. |
| WELCOME | `data/expanded/expanded_welcome_bg.raw` | `OBSOLETE_VISUAL / KEEP_RUNTIME`. |
| SACKED | `data/expanded/expanded_sacked_popup.raw` | `OBSOLETE_VISUAL / KEEP_RUNTIME`. |
| MANAGEMENT RECORD | `data/expanded/expanded_record_bg.raw` | `OBSOLETE_VISUAL / KEEP_RUNTIME`. |
| SQUAD | `data/expanded/expanded_squad_bg.raw` | `OBSOLETE_VISUAL / KEEP_RUNTIME`. |
| MATCH STATS | `assets/match_stats/match_stats_ui_approved_2026-11-06.png`; `data/expanded/match_stats_ui.bmp`; `development_assets/match_stats/` | `OBSOLETE_VISUAL / KEEP_RUNTIME` per asset caricati; riferimenti grafici di sviluppo non cancellabili senza verifica. |
| EXIT MATCH | `data/expanded/match_exit_ui.bmp` | `OBSOLETE_VISUAL / KEEP_RUNTIME`. |
| TITLE / NEW CAREER legacy | `data/expanded/expanded_title_bg.raw`; `data/expanded/expanded_career_bg.raw` | Le **vecchie immagini** sono sostituite dal nuovo design, ma potrebbero essere ancora fallback. Non eliminare senza modificare caricamenti. |
| TRANSFERS, TRAINING, altri menu | Qualsiasi grafica precedente eventualmente in `assets/`, `data/expanded/`, `bin/` o incluso nei generatori | Da individuare analizzando l'albero, non indovinare percorsi. Layout sempre da ridisegnare. |

## C. File che già portano il suffisso OBSOLETO

- `assets/title/buttons/career_OBSOLETO.png`
- `assets/title/buttons/classic_modes_OBSOLETO.png`
- `assets/title/buttons/continue_career_OBSOLETO.png`
- `assets/title/buttons/exit_game_OBSOLETO.png`
- `assets/title/buttons/hall_of_fame_OBSOLETO.png`
- `assets/title/buttons/options_OBSOLETO.png`
- `data/expanded/expanded_loading_bg_OBSOLETO.bmp`
- `data/expanded/expanded_loading_bg_OBSOLETO.png`
- `data/expanded/expanded_loading_caption_OBSOLETO.bmp`
- `data/expanded/expanded_loading_title_OBSOLETO.bmp`

**Stato:** `ALREADY_NAMED_OBSOLETE / VERIFY_ORPHAN`. La presenza del suffisso non basta per eliminare: controllare anche build, loader, generatori e altri script che potrebbero copiarli o elaborarli.

## D. Asset condivisi o di supporto: NON eliminare alla cieca

- `assets/ui/controller_abc_approved_2026-11-07.bmp` e `assets/ui/legends/` — controlli e legende potenzialmente riutilizzati in più menu.
- `assets/fonts/expanded/` e font originali SWOS — renderer di caratteri; nessuna cancellazione indiscriminata.
- Immagini utilizzate nella **partita** o nella simulazione, file di squadre, `*.dat` e salvataggi — **non sono oggetto della pulizia grafica**.
- Asset di sviluppo/reference necessari alla revisione di UI e font: esaminarli singolarmente. L'obiettivo è ridurre l'installazione, non distruggere le fonti che servono a riprodurre un asset approvato.

## E. Specifica del prossimo BAT di pulizia

1. **Inventariare** tutti i file grafici e i riferimenti incrociati in sorgenti C/include, JSON, script shell/BAT, generatori e documentazione operativa.
2. **Separare** i file in `FINAL_GRAPHICS`, `OBSOLETE_VISUAL / KEEP_RUNTIME`, `SHARED`, `ORPHAN_CANDIDATE` e `ORPHAN_VERIFIED`.
3. **Fornire un report dei candidati precisi**; non fare cancellazioni generiche per estensione, directory o data.
4. **Aggiornare prima i caricamenti e i fallback** dei menu riprogettati e testati, verificando gioco e salvare/caricare.
5. **Solo alla build di pulizia autorizzata** consentire al BAT di rimuovere i soli `ORPHAN_VERIFIED`, preservando `.gitkeep` e le directory attive; distinguere i file cancellati localmente da quelli rimossi da GitHub.
6. **Ripetere i test** di avvio e navigazione (mouse, joypad, tastiera, partita→menu) e annotare peso risparmiato e file cancellati.

**Promemoria ricorrente da regola storica:** controllare la necessità di un BAT di pulizia ogni 10 build. La versione consultata è `0.10.75`; il prossimo punto di controllo naturale della decina sarebbe `0.10.80` se la serie procede normalmente. Il nuovo comando dell'utente è di preparare l'eliminazione dei grafici superati **al prossimo BAT di pulizia**, non di rimuovere ora gli asset attivi.

**Nota:** questo inventario non è un test automatico completo dei riferimenti. Nessun file del gioco è stato cancellato in questa operazione.
