# SWOS Expanded — implementazione NEW CAREER (istruzioni per Codex locale)

**Stato:** pacchetto di lavoro, non implementazione. **Data:** 9 novembre 2026.

## 0. Prima di modificare

Lavorare unicamente nel repository Windows locale dell'utente, ramo `dev/0.10` (verificare effettivamente Git HEAD, non assumere che sia ancora `4290e8c`). Esistono modifiche locali precedenti NON committate, relative al workflow Codex, documentazione, scripts e toolchain: **non sovrascriverle**.

Leggere `AGENTS.md`, `PROJECT_STATE.md`, `docs/DEVELOPMENT_WORKFLOW.md`, `sorgenti/docs/FONTS.md`, il codice attivo di Career e Title, compresi `sorgenti/src/menu_data.c`, `menus.c`, `expanded_economy.c`, `career_init.c`, `input_text.c`, `menu_system.c`, `expanded_title_overlay.c`, `platform_sdl.c`, e le dipendenze reali. Verificare tutte le funzioni e coordinate prima di programmare. Non fare commit/push/pull/checkout/merge, non aggiornare o installare pacchetti.

## 1. Asset forniti

| Percorso | Contenuto | Dimensioni | Stato |
|---|---|---|---|
| `assets/career/new_career/background_approved_reference.png` | Sfondo illustrato senza UI | **1670×942 RGB8** | **Approvato, originale intatto** |
| `assets/career/new_career/ui_approved_reference.png` | Intero overlay UI senza testo su alpha | **1672×941 RGBA8** | **Approvato, originale intatto** |
| `development_assets/new_career/mockup_APPROVATO_solo_riferimento.png` | Mockup con numeri dimostrativi | **1670×942 RGB8** | Approvato come riferimento, NON asset runtime |
| `assets/fonts/expanded/expanded_ui_{24,32,42}.png` | Font-atlas RGBA | Dimensioni diverse | **Font di PROVA, non approvato** |
| `assets/fonts/expanded/expanded_ui_{24,32,42}.json` | Metriche glifi | — | Di prova |
| `tools/fonts/generate_atlas.py` | Generatore TTF/OTF → PNG+JSON | — | Tool di sviluppo |
| `development_assets/fonts/PROVA_FONT_NON_APPROVATA.png` | Confronto testi font | — | Bozza |

**Non inventare file PNG approvati oltre questi.** Il mockup include testi in immagine soltanto come prova visiva: testi in runtime vanno da atlante font.

## 2. Questioni obbligatorie prima di integrare (non decidere da solo)

1. **Dimensioni e crop.** Le regole del progetto richiedono sfondo generato a `1672×941` e ritaglio *senza resize* a `1664×936` 16:9. L'asset dello sfondo già approvato risulta invece `1670×942` (l'UI misura `1672×941`). Chiedere all'utente se autorizza un **crop puro** senza ricampionamento dai file esistenti, precisando che la generazione iniziale è diversa dalla regola, oppure se preferisce rigenerazione delle sorgenti canoniche. Non fare resize silenzioso.
2. **Focus baked-in.** Il PNG UI approvato include già un contorno giallo intorno alla prima riga TITLE. Le regole UI vietano il focus incorporato: deve seguire lo stato tramite motore, anche con joypad/mouse. Chiedere se è autorizzata la **pulizia mirata del contorno giallo della sola prima riga**, mantenendo intatto ogni altro pixel, oppure preferisce rigenerare una variante *clean*. Non trattare il PNG con bordo fisso come l'asset definitivo runtime senza decisione.
3. **Font definitivo.** La tecnica TTF → atlante bitmap PNG è approvata per i nuovi menu; la **famiglia tipografica precisa** no. L'atlante accluso è una prova DejaVu Sans Condensed Bold. Mostrare `PROVA_FONT_NON_APPROVATA.png` e ottenere approvazione o una scelta alternativa prima della UI definitiva. Il logo e i bottoni del Title Menu esistente non vanno toccati.
4. **Percorso di avvio.** FAME non decide più se il manager parte disoccupato. L'utente vuole **iniziare disoccupato con ogni fama** e mantenere la possibilità di scegliere la squadra. L'UI approvata attualmente contiene un unico pulsante lungo d'avvio: proporre il modo più coerente di esporre due azioni indipendenti **senza implementare bottoni nuovi non approvati**. Verificare le callback/visibilità e la semantica di `ChooseTeamAndStartCareer()` prima di mutare la logica. **Non simulare entrambe le scelte cambiando FAME.**

Sono scelte autentiche da confermare: puoi proseguire con analisi e preparazione indipendente, ma NON implementare unilateralmente queste parti.

## 3. Specifica grafica

- Mantenere **due layer separati**: sfondo illustrato RGB e overlay/sprite RGBA, senza inglobare il mondo nel pannello.
- In base all'UI, elementi statici illustrati separati dai testi: banner vuoto (`NEW CAREER` da font), righe TITLE / NAME / SURNAME / NATION / ROLE / AGE / FAME, pulsante grande, BACK, frecce. Il PNG fornito non contiene alcun testo.
- Il renderer stampa **tutte le etichette e i valori** usando il nuovo atlante, finché non viene approvato un testo statico baked-in. In particolare: `MR/MS`, nome, cognome, nazione e bandiera/codice, `COACH/PL/COACH`, età 18–70, fama (`UNKNOWN`, `AMATEUR`, `RISING STAR`, `ESTABLISHED`, `LEGEND`), etichette `START UNEMPLOYED`/eventuale `CHOOSE TEAM`, `BACK` e `NEW CAREER`.
- Non usare il testo originale SWOS per la nuova UI HD salvo fallback esplicitamente autorizzato. Nessuna nuova DLL durante runtime; gli atlanti sono generati *offline*.
- SDL2 HD riusa il percorso del Title; NON migrare a SDL3/Godot o cambiare renderer/architettura senza proposta e approvazione. Texture filtering pixel art nitido; coordinate e hitbox coerenti, focus/hover dinamico, bounce senza resize. Rispetta la finestra dev decorata massimizzata e release fullscreen desktop.
- Font metrico: .json mappa codepoint → `x,y,w,h,bearingX,bearingY,advance`. Il renderer usa baseline e pen advance dei glifi. Il file `generate_atlas.py` dipende da Pillow soltanto nello sviluppo; l'EXE deve leggere texture PNG e metriche precomputate, senza Python né SDL_ttf in esecuzione.
- `FONTS.md`: le metriche del charset legacy 6/8 px continuano a valere per vecchie UI; la nuova UI usa la famiglia approvata quando scelta. Misurare stringhe reali e casi estremi (8 caratteri NAME, 12 SURNAME, età, nazione, fama, comandi), senza tagli e senza clipping.

## 4. Logica esistente da preservare

Secondo il rapporto Codex raccolto dall'utente (verificare nuovamente nel codice): `CareerMenu()` e `_careerMenu` usano ora 320×256 con `ui_career_prepare()`, input originari e callback `ChangeSex`, `SetName`, `SetSurname`, `ChangeNationality`, `CoachOrPlayerCoach`, `Expanded_EditManagerAge`, `Expanded_ChangeManagerFame`, `ChooseTeamAndStartCareer`, `SetExitMenuFlag`. I selettori Nation/Fame usano modali originali. Non ricostruire o alterare la partita, economia, save format o gestione delle competizioni.

- La scelta della fama rimane un attributo del manager, non un comando per l'occupazione.
- START UNEMPLOYED deve condurre al calendario disoccupato a qualsiasi fama, tramite un percorso separato e verificato, preservando l'avanzamento del mondo e le offerte; CHOOSE TEAM usa invece il suo flusso esplicito.
- Verificare e integrare mouse per tutti i campi e modali: Codex ha segnalato che i click attuali non raggiungono genericamente NEW CAREER. Tastiera/joypad e tasto BACK devono rimanere completamente funzionanti. Non usare un rendering o un hover che attivi callback distruttive.
- **Progetto futuro separato:** calendario e schermata disoccupato verranno unificati in una sola UI con stati occupato/disoccupato. Non implementarli come parte di questa task, se non richiesto.
- `CLASSIC MODES` resta voce principale del Title Menu, come deciso.

## 5. Sequenza di sviluppo consigliata

1. Verificare Git e modifiche locali; inventariare callback, logica e dipendenze, senza editare i file approvati.
2. Mostrare le **quattro questioni** sopra all'utente e raccogliere le autorizzazioni necessarie. Non ridimensionare le immagini o cambiare semantica prima della risposta.
3. Presentare un'anteprima coerente con lo screenshot già approvato, usando i font candidati; verificare le misure reali.
4. Dopo le approvazioni: crop puro se autorizzato, eliminazione focus baked-in se autorizzata, eventuale splitting degli sprite senza alterazioni, generazione font definitivo e renderer di testo/overlay.
5. Collegare una singola volta le callback originali, controller+mouse+tastiera e focus/hover; implementare la scelta start indipendente da FAME come concordato.
6. Compilare usando `COMPILA_SWOS.bat` con MSYS2 preinstallato; **nessun pacman automatico**. Analizzare warning e sintassi, provare avvio e navigazione Windows con il permesso dell'utente; non dichiarare test non svolti.
7. Verificare `git diff --check`, riassumere modifiche, build e test. Nessun commit/push fino a esplicito `committa`.

## 6. Informazioni di ambiente (da confermare)

Repository locale dichiarato: `C:\Users\xgiuz\Downloads\Repo SWOS expanded\SWOS-Expanded`, rama `dev/0.10`; base iniziale noto `4290e8c` build `0.10.54_09`. L'utente ha già verificato MSYS2 `C:\msys64`, GCC 16.2.0, SDL2 2.32.10 e compilazione incrementale riuscita. Le modifiche di workflow locale esistono ma non sono state ancora committate al momento della consegna; NON annullarle.
