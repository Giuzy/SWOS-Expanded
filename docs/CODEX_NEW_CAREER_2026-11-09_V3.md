# SWOS Expanded — NEW CAREER, Codex V3 — 9 novembre 2026

**Consegna:** grafica approvata + guida implementativa per Codex sul PC Windows. NON e' un eseguibile, codice implementato, commit o test.

## Font: NUOVA decisione approvata dopo V2

Il font in uso per i NUOVI menu deve essere **coerente con il lettering reale dei pulsanti del Title Menu gia' approvato**. Questo sostituisce sia il provvisorio DejaVu Sans Condensed Bold, sia il lettering bianco/rosso/inclinato dell'ultimo esperimento generato, che l'utente ha ESPRESSAMENTE BOCCIATO. Il riferimento visivo si recupera nei file LOCALI reali, senza chiedere nuovamente il PNG all'utente:

- `assets/title/buttons/career.png`
- `assets/title/buttons/hall_of_fame.png`
- `assets/title/buttons/classic_modes.png`
- `assets/title/buttons/options.png`
- `assets/title/buttons/exit_game.png`
- eventualmente `assets/title/buttons/continue_career.png`

Il testo di tali pulsanti potrebbe essere **baked-in** nel PNG: NON presupporre che esista il TTF originale o un alfabeto completo. **Leggere il sorgente `sorgenti/docs/FONTS.md` e il renderer Title**, analizzare texture, caratteri, altezze, tratti, ombre, avanzamenti e dimensioni reali. Individuare un TTF/OTF utilizzabile legittimamente che riproduca l'aspetto, oppure proporre una ricostruzione bitmap dei glifi, spiegando differenze e licenza; NON inventare un font esatto. Preparare una preview pixel-accurata con le frasi piu' lunghe reali (incluso `START UNEMPLOYED`, `AGE (18–70)`, `RISING STAR`, `ESTABLISHED`, `PL/COACH`, `NEW CAREER`) affiancata ad un PNG Title approvato. **Chiedere l'approvazione visiva prima di dichiararlo font canonico.** Preferire atlas PNG RGBA8+metriche accurate, generabili offline con `tools/fonts/generate_atlas.py` (script generico, adattabile). Nessuna dipendenza aggiuntiva Python/SDL_ttf a runtime. NON ridisegnare o toccare i pulsanti del Title.

In V3 **NON sono inclusi gli atlanti DejaVu**: sono stati rimossi per impedire di usarli per errore. Lo script generatore resta disponibile; non contiene font proprietari.

## Prima di qualsiasi edit

Leggi `AGENTS.md`, `PROJECT_STATE.md`, `IMPROVEMENTS.md`, `docs/DEVELOPMENT_WORKFLOW.md`, `sorgenti/docs/FONTS.md`, codice attivo, art e dipendenze. Verifica `git status`, ramo e HEAD LOCALI, preservando i cambi non committati. Su GitHub lo snapshot consultato di `dev/0.10` era `4290e8c7cb2cc3864a26cbbf9cf7b6c98604a662`, ma il locale puo' essere piu' recente. Nessun pull/checkout, commit, push, merge o cancellazione automatica. Non alterare Title, Business, Transfer, Calendar, altri menu, gameplay, salvataggi, match a 50Hz, controller o configurazione SDL2. In caso di contrasto REALE tra documentazione, asset approvati e codice, segnala fonti e conseguenze e chiedi la decisione all'utente prima di toccare la parte controversa. Prosegui solo con aspetti indipendenti.

## Asset approvati e file tecnici

- `assets/career/new_career/background_approved_original_1670x942.png` — sfondo autonomo RGB8, **approvato**, senza menu, originale inalterato.
- `assets/career/new_career/ui_approved_clean_transparent.png` — UI blu a **sette righe**, senza testi/spunta/focus giallo, **approvata visivamente**. PNG RGBA8 ricavato tecnicamente dalla fonte su nero opaco: verificare e se necessario correggere solo il bordo alpha, confrontando il disegno originale.
- `assets/career/new_career/checkbox_off_approved_rgba.png` — box vuoto, blu scuro, sprite RGBA8.
- `assets/career/new_career/checkbox_on_approved_rgba.png` — identico stile/fondo blu scuro, spunta VERDE, sprite RGBA8.
- `development_assets/new_career/` — riferimenti originali e preview diagnostica. `preview_composition_UNAPPROVED.png` NON e' un layout approvato.
- `tools/fonts/generate_atlas.py` — script offline solo di supporto, nessuna famiglia font approvata inclusa.

**Dimensioni:** sfondo e UI originali sono **1670×942**, non il formato canonico 1664×936. Segnalarlo e proporre eventualmente un **crop puro**, senza ridimensionamento, con coordinate e verifica delle parti visibili; richiedere approvazione prima di applicarlo. Conservare originali. Il crop a 1664x936 comporterebbe **6 pixel rimossi in larghezza e 6 in altezza**: proporre l'ancoraggio prima di decidere. La regola generativa 1672x941 non e' stata seguita per questi asset gia' approvati.

**Separazione tecnica dei tasti:** nell'UI fornita i due tasti blu inferiori sono ancora disegnati nel pannello; per simulare verde/rosso, focus e bouncing evitare che il tasto statico resti visibile sotto l'animazione. Estrarre sprite dedicati SENZA cambiare stile/posizione, e ricostruire l'area sotto di loro oppure usare copertura/composizione equivalente con prova visiva e approvazione di eventuali variazioni. Il titolo e tutti i valori devono essere disegnati dal renderer, non incorporati nelle texture.

## Campi originali reali e testo a runtime

Non usare gli ultimi mockup errati con `CURRENCY`, `START YEAR`, `DIFFICULTY` o `MANAGER`. L'originale `NEW CAREER` ha esattamente sette campi, in quest'ordine:

1. **TITLE** → MR / MS (tramite sistema originale)
2. **NAME** → editable, lunghezza reale da codice (riferimento remoto: 8)
3. **SURNAME** → editable (riferimento remoto: 12)
4. **NATION** → nazionalita' originale, eventuale bandiera solo se disponibile e mappata correttamente
5. **ROLE** → COACH / PL/COACH secondo codice originale
6. **AGE (18–70)** → etichetta col range a SINISTRA, a DESTRA solo valore (18–70)
7. **FAME** → UNKNOWN, AMATEUR, RISING STAR, ESTABLISHED, LEGEND

Titolo `NEW CAREER` in alto; in basso tasto lungo `START UNEMPLOYED` e tasto corto `START`, **non BACK**. I testi sono tutti dinamici e a font bitmap: misurare larghezze massime, avanzamenti dei glifi, hitbox e margini PRIMA di fissare coordinate. Non alterare il rapporto grafico approvato. I campi editabili NAME/SURNAME non sono cicli di valori; AGE originariamente apre input numerico 18..70: le frecce disegnate vanno collegate ad una vera funzione oppure chiarire modifiche necessarie; niente frecce decorative che non fanno nulla.

## Logica interattiva DEFINITIVA

- FAME completamente indipendente dal percorso iniziale; valori UNKNOWN..LEGEND non abilitano/disabilitano il toggle.
- `START UNEMPLOYED`: **toggle**, non avvia la partita quando premuto. **OFF** = tasto blu, checkbox vuota a DESTRA; **ON** = tasto verde (trasformazione/tint **a runtime**, come CONTINUE CAREER), checkbox con spunta VERDE su FONDO SCURO. Clic/scegli A cambia stato; il focus giallo resta indipendente dalla spunta. Il default del toggle non e' stato esplicitamente scelto dall'utente: **non imporre un valore non verificato senza segnalarlo**; riprendere eventualmente il comportamento del flusso originale o chiedere quando serve.
- `START` (tasto inferiore corto) sostituisce BACK: **blu finche' dati obbligatori invalidi/non completi; rosso quando e' consentito avviare**, sempre a runtime, non bake nella texture. Codex deve identificare la validazione originale; non inventare un nuovo requisito di selezione TEAM nella schermata.
- Premendo START con toggle ON: entra SUBITO nella carriera DISOCCUPATA esistente, calendario/attesa offerte, senza squadra selezionata e senza aprire il selettore team.
- Premendo START con toggle OFF: apre il **selettore squadre originale**, dove l'utente sceglie la squadra. Non selezionarla automaticamente.
- Non aggiungere campo TEAM, secondo bottone CHOOSE TEAM o popup aggiuntivo. Non alterare la FAME impostata.
- Retrocompatibilita' save: non cambiare struttura dei file. Se serve nuova flag di menu, renderla stato di setup, non campo persistente senza approvazione.
- Entrare/uscire da popups con B/ESC/tastiera/mouse/joypad immediato. No bottone BACK in questa schermata. La legenda A/B/Y globale e' prevista in futuro: **non inventarla o sostituire tutti gli altri BACK in questo intervento**.

**Indizi codice GitHub consultato:** `sorgenti/src/menus.c`, `CareerMenuFinish()` decide oggi il testo della singola entry con `_expandedManagerFame == 0`, mostrando `_expandedCareerStartButton` contro `_aChooseTeam`. `Expanded_ChangeManagerFame()` forza il repaint; `Expanded_EditManagerAge()` usa InputText e valida 18..70. Codex deve controllare il CODICE LOCALE e recuperare le callback esistenti per i due flussi, senza inventare una scorciatoia o perdere il supporto legacy.

## Visual, runtime e test

Asset nuovi PNG lossless RGB8 per sfondo e RGBA8 per UI/sprite; focus/hover giallo disegnati a runtime secondo sagoma alpha e navigazione; pulsanti blu con spessore destro/sotto, bouncing di pressione senza resize, pad/keyboard/mouse. Niente frame di selezione baked nel PNG. Stato verde e stato rosso applicati SOLO ai tasti interessati, non all'intero pannello; checkbox ON/OFF separati. Se i PNG forniti presentano bordo o crop problematico, chiedi anziche' sostituire con redesign. Nessuna modifica a gameplay fisica, salvataggi, altre UI, release config o motore SDL2.

**Sequenza consigliata:** (1) esamina sorgenti e diff; (2) verifica loader/PNG/alpha; (3) mostra confronto font Titolo + mockup dinamico con 7 righe e due tasti; (4) mostra crop proposto; (5) implementa rendering/focus e callback mantenendo la compatibilita'; (6) compila davvero sul Windows locale, prova toggle ON/OFF con livelli diversi di FAME, edit text/nation/age, START, focus e ritorno B/ESC, mouse/tastiera/Switch Pro; (7) presenta hash, diff, build e test reali/pendenti. **NON COMMITTARE** senza `committa` esplicito.

Il tono della risposta a chi usa Codex sia in ITALIANO, amichevole e naturale, ma preciso e trasparente. Non sommergere l'utente di domande gia' risolte.
