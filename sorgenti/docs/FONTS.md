## 21/11/2026 — v00.13.239 FIX VIEW BOUNDS + NEXT MATCH BUTTON ORIGINAL OG
Screenshot reale v238: in testa lista appare mezza pagina vuota, freccia porta oltre prima/ultima gara. Soluzione visiva: clamp pagina 0..max(0,numero-gare-14), solo min(numero-gare,14) righe dipinte, box e hotbox freccia adattati alla lista corta; focus centrato quando possibile. Rettifica user: NEXT MATCH OG entry10 di nuovo visibile, non nascondere/disegnare diversamente; callback e enabled nativi. Font sprite _charTables[0] invariato; QA visivo WIN 3840×2160, HITBOX/mouse/Pro non provato. Fonte bug/regression RG-014.

## 21/11/2026 — 00.13.238 CAREER CALENDAR ONE BUTTON + NATIVE CUP OG, VISUAL QA PREP
User decision: NO visible NEXT MATCH in the covered CAREER calendar, preserve original callback for other modes. Single entry7 is FORWARD TIME on dates without manager fixture (advance ONLY to next OG manager day), PLAY MATCH on match date TODAY; no false pre-date 03/09 when clock shows 28/08. Original 14-row DPN/Copa merged civil calendar red arrows ordinal16/17 retain real native callbacks and hitboxes aligned x14; persistent viewport centered at visible row7 with blank pads at ends; blue row always matches the exact same OG manager fixture that the PLAY action will execute, now gated against overdue/unknown by shared read-only planner. On native ConteSetButtons after last fixture re-hide NEXT; when selected keyboard/Pro focus was on hidden entry10 return to entry7. No change glyph size/hitboxes other than genuine red arrow alignment, native font used. Cup phases remain from Competition Info and actual OG results, KO quarters/semis/final original type1 menu accessible via same PLAY only when due.
Refs: renderer FULL `sorgenti/src/expanded_economy.c` SHA 6607d2038094ded3f5a53335b57f21e94b73c5ac, native buttons/arrow callbacks `sorgenti/src/menus.c` SHA55cb99928589f179c8c07a615b6bacfdfd82331c, original manager menu `expanded_midseason_join.c` a236eccd77fd815fe921c6d7cf6ac9280908f998, header viewport9f02194a, action599e69f2; true KO code career_status0ed5acf7/diy_competition323649b3. UI viewport pure C11 GCC -Werror ISO PASS earlier, static source scoped23/23, real Windows screenshot mouse click/red arrows/Pro and KO team names not tested. No PNG modifications or new font assets made; comply future PNG demo approval before new UI art. RG-014/010, Issue #1.

## 21/11/2026 — UI CALENDARIO BOLIVIA: FRECCE ROSSE HOTBOX + RIGA CORRENTE CENTRATA (RG-014, PREP NO COMMIT)
Fonte user corrente e source HEAD dev/0.10 04c43027 v237. Il renderer `expanded_economy.c::ui_render_merged_calendar` dipinge testo con FONT NATIVO SWOS 320×256 x29..310, 14 righe alte 9px y53+9i e un highlight blu sul vero evento OG attuabile; non modificare font/PNG/glyph. **Problema V:** pagina locale sempre0 e centrale solo quando la partita era dopo le prime14 → frecce originali cliccate non cambiano righe, blu fuori centro. Inoltre glyph frecce rosse disegnato a cx19 top y53 / bottom y169 con panel bottom190 mentre i veri hotspot `_menuEntries[16/17]` da JSON (e parser +24,+36) erano top x24/y52 e bottom x24/y183, non allineati. **Soluzione preparata:** originali onSelect `ContestMenuScrollUp/Down` su Bolivia chiamano `Expanded_UI_BoliviaCalendarScroll(-1/+1)` per spostare unicamente il viewport del calendario completo 30 DPN + Copa, senza alterare `_teamSelectionKeyCache` o OG. `ui_bolivia_sync_scroll_hitboxes` conserva rect originali e usa x14 w12, top y49 h14, bottom y=bottom-24=166 (bottom190), ripristino geometry fuori WORLD2627 country45. Riga highlight OG ancorata al centro #7 (y116) con 14 righe fisse, slot vuoti agli estremi; dopo click frecce l'utente può consultare altre gare, il cambio fixture/re-entry ricentra. Niente finte gare/doppie date e no salvataggio scroll in CAR (presentazione locale).
Code blob visual `f339dd3c3e79f10c08960e47654a66258d4df8e5`, callbacks `ee8962f8d633f1d456ef2b341bc9f94ae3ba4dcb`, header `9f02194a0ab6fc8cc25984ca3b35e8f762ddb54d`, ISO test `76ea6c298129522a0844d7509d9bf88aaf222cfa`, runner `79b0274e0a7c9c9e039054f3243ab6f9891e8253`. Test GCC C11 stage header/test/runner SHA identici e PASS; STATIC original JSON arrows/hotbox + source cross-file 18/18 PASS. Test visivo Windows reale su scaling/joypad/mouse, hitbox click primo ultimo, 9697/classic, SAVE/CONTINUE e invalid focus **NON PROVATI**. Sorgenti/docs/FONTS.md guida pixel art autoritativa, non introdurre nuovi caratteri/grafica. Non committato.

## 21/11/2026 — FOCUS BUG VISUAL QA FOR MERGED COPA ROW (00.13.229 PREPARED)
Old renderer highlighted only original League cache rows using menuEntries[37].isInvisible, but appended Copa row in merged display was zero-highlight, even when NEXT/SIM OG targeted that exact Cup fixture. New prepared visualization uses same OG action plan, highlights one correct Cup row with BLUE rect x81..310, removes blue from League while Cup is actionable, and if Cup is chronologically beyond first 14 entries scrolls VISUAL window only. Native 14-row font, date DD/MM, COPA+phase yellow, DPN, opponent and red arrows remain. No separate focus overlay that changes menu callbacks or TSKC. In-date clash/overdue Cup must not be painted as playable.
Test native screenshot Windows for two cases Cup 03/09 prior to Liga 04/09 and League prior to Cup, 14+ scroll, postgame focus moves; check controller/mouse A/B and no ghost blue border. Scores old preJuly only if strict OG journal witness else '?' (not a second V), and entire 2026 REAL historical data must be sourced separately. HEAD 58058d99/00.13.228, source prep UI blob 25b1cd2bfc2440a86c7ef0777448cb64b5e4b15b; WIN screenshot post-fix not tested; no new fonts/PNG or approved layout changes.
## 21/11/2026 — FONT CHECK 228: FASI COPA PER RIGA SENZA TRONCARE DATE/OPPONENTI

Fonte user screenshot menu 21/11: nel compatto calendario Bolivia righe 9px x53..170, colonna data DD/MM x29/34px, competizione x65/36px, descrizione x102/68px, sede x178/18px, score x196/24px, opponent x210/100px. Confrontando dimensioni font SWOS native (_charTables[0]) e nuove fasi da `BoliviaCup_PhaseName`, la stringa COMPLETA `COPA BOLIVIA - QUARTER-FINAL` NON può entrare nelle 104px delle due colonne senza rubare spazio a data/sede/risultato/opponent o ridurre il font. **Compromesso esplicito per build 228 ST preparata:** `COPA` giallo nel campo competition + `GROUP STAGE`, `DERBY`, `QUARTER-FINAL`, `SEMI-FINAL`, `FINAL` nel campo stage, con `ui_native_ellipsis` solo se eccede 68px; il titolo per esteso `COPA BOLIVIA` resta in Competition Info. Non inventare stage per incontri futuri e non cambiare caratteri/PNG o font nuovo per forzare tutto. Per gruppi derby non nuovo torneo. Il `QUARTER-FINAL` potrebbe essere abbreviato dall'ellipsis in base alla larghezza reale, verificare il testo renderizzato su Windows prima di dichiararlo conforme. Nuovo codice conserva precedente bugfix stage futuro e score Liga e aggiunge score OG read-only Cup per 0-0/1-0/2-1, non alterare colori GLIF/hitbox/controller.

**QA obbligatoria screenshot Win**: nome Copa info completo, riga con giallo COPA e fase, scroll 14, focus, mouse/tastiera/Switch Pro, 1px margin, risultato orientato, no righe Cup a giugno se non disputate, DD/MM, DPN, single clock, save/reload. Il test degli header in Linux non certifica i glifi. Fonte SHAs source expanded_economy.c be16b268c32de65a85415aa74cae4c64756eb448, bolivia_cup_sequence.h 3728c86d69d87f669134c140255716dde61f211d, IG/GP NP.

## 21/11/2026 — QA VISIVO PROSSIMA BUILD BOLIVIA: COPA BOLIVIA E RISULTATI OG (USER SCREENSHOTS)

Fonte screenshot Windows reale 00.13.227: 01/07 e 08/08 Nacional Potosi. **Richiesta:** non `COPPA` italiano ma `COPA BOLIVIA`, risultati `x-y` dove presenti nello stato OG (anziche' solo `-`), future non giocate ancora `V`, senza alterare punteggi/salvataggi. Precheck font/colonne già presenti: colonna comp x65 max36px, seconda colonna x102 max68px; font nativo `_charTables[0]`, `ui_native_text` e `ui_native_ellipsis`, non nuove texture/glifi. Per *specifico residente Copa Bolivia* proporre etichetta `COPA` + seconda `BOLIVIA` così il nome completo e' leggibile nel layout; non sostituire automaticamente nome di altre coppe diverse. Sono **due campi** vicini, non una nuova lingua/localizzazione. Score `x-y` usa x196 max24px, 'V' x196 max12px; controllare 0-0, 10-0, locale/trasferta, overflow, scelta colore giallo, 14 righe scroll e frecce, bordi/margini >=1px. `menus.c` fornisce score reali attraverso entry39/41, entry40 contiene separatore '-', `expanded_economy.c` non li ricompone quando mark non vuoto; solo renderer va cambiato.

**Stato:** soluzione ST proposta per prossima build, NO screenshot dopo fix, NO approvazione grafica finale, NO test WIN/GP/SAVE. Non alterare CLASSIC/DIY/SEASON/9697 o i fonts. Dato storico pre-1 luglio va risolto separatamente dal layout: il font non puo' creare risultati mancanti.

## 21/11/2026 — Calendario club 00.13.227, DPN/COPPA/date nella lista (PREPARATO)

Render SDL `expanded_economy.c::ui_paint_compact_fixture` utilizza **SOLO il font sprite nativo SWOS `_charTables[0]`** tramite `ui_native_text`/GetTextWidth e `ui_native_ellipsis`, SENZA nuovi glifi/PNG o font raster. Pannello a coordinate native 320×256, 14 righe massimo di 9 px, y=53+9*i (ultimo 170), bordo x5..315 e fondo<=190, pulsanti sotto y195. SOLO nella carriera del manager assunto in Bolivia (WORLD_2627 country45), senza cambiare le colonne originali di 9697/CLASSIC/DIY/SEASON o altri paesi. Colonne righe: data `DD/MM` x29 max34px; competizione x65 max36px (`DPN` per Liga Bolivia e `COPPA` **gialla palette 15** per tutte le Cup locali visibili nella carriera Bolivia (tipo1/2) e la Copa Bolivia OG proiettata; negli altri mondi resta il nome originale della competizione e il colore Cup già previsto); seconda etichetta/team x102 max68px; (H)/(A) x178 max18px; V/score x196 max12/24px; avversario x210 max100px. Testo lungo passa per `ui_native_ellipsis`, nessuna larghezza forzata/fondata su font approssimato, focus native del menu rimane. Il banner giallo `COPA BOL...` fuori dal calendario è stato rimosso; le gare Copa vere sono raggruppate e ordinate nel renderer readonly dalla tabella OG del manager, non scritte nei record TSKC. La logica di scorrimento originale resta; 8/8 righe Cup per 16 club sono state verificate solo da una simulazione isolata JS, NON screenshot/hitbox Windows. 

**UI QA ancora OBBLIGATORIA prima di dichiarare visual OK:** PNG o screenshot demo con il font reale e valori reali (Nacional 03/09 vs Tomayapo, Liga 04/09), misurazione glifi e margini >=1 px, giallo COPPA chiaramente leggibile nel riquadro e non nell'header, 14 righe, scroll e frecce cliccabili, mouse/tastiera/Pro Controller, NESSUNA regressione 9697/CLASSIC/DIY/SEASON. Non confondere il codice preparato con demo approvata o UI gameplay verificata.

## F12 SDL (00.13.191, 18/11/2026)
- F12 usa esclusivamente il font bitmap **5x7**, passo monospaziato di **7 px**, logica 960x540, non il charset SWOS da 6/8 px. `splash_text_tinted` in `platform_sdl.c` applica il passo anche alle lettere non note; i glifi `>`, `Q`, `J`, `+`, `(`, `)` sono aggiunti alla tabella, nessuna altra metrica cambiata.
- Divisione verticale x378, prima colonna x26 (dati attuali); confronto assunzione e salute a x395, nel pannello [12,948] e righe [12,528]. Stringhe dello status sono troncate a 16, 8 e 42 caratteri; niente testo fuori bordo dal renderer per le righe nuove.
- Nuovo F12 senza asset raster: qualsiasi PNG dimostrativo rimane SOLO preview, la UI e' composta da testo/cifre dinamiche originali. Verifica pixel/GLIF e leggibilita' Windows ancora da svolgere.

# SWOS fonts — adding glyphs (Amiga + DOS)

The SWOS font is a **charset sprite sheet**: a 320×256 grid of glyph cells, each glyph
having a *small* (6px) and *big* (8px) variant. At draw time the game maps a string byte →
a sprite via the **conversionTable**, and the **image-script** defines each sprite's cell
rectangle. Adding a glyph (`$`, `€`, accents, …) means: draw it into a free cell, wire its
conversionTable entry + image-script rect, re-encode the sheet to the platform format, and
(for non-Latin-1 codepoints) add a translation byte mapping.

Two external trees drive this:
- **`C:\Projects\Python\SWOS_Font`** — `swos_font.py`, the glyph generator.
- **`C:\Projects\GitHub\SwosGfx`** (`bin\Debug\net8.0\SwosGfx.exe`) — the RAW ↔ PNG charset
  codec for both the Amiga (4-bitplane + RNC) and DOS charset formats.

---

## The pieces

| Piece | What it is | Where |
| --- | --- | --- |
| `CHARSET.RAW` | Amiga charset — 4-bitplane bitmap, RNC-compressed. DOS has its own charset in the DOS graphics set. | `bin/grafs*` |
| **conversionTable** | `byte → sprite index`, indexed by `byte − 0x20`; separate small + big tables (big sprite = small + 57). | `src/swos_globals.c` (uint16; sprite pointer table `_spriteGraphics` sized 1310 entries in `src/swos_amiga_data_stubs.c` — 1220 game slots + font-extension slots 1220..1303) |
| **image-script** | Per-sprite cell rect: `0xC000\|idx, (x<<8)\|y, (w<<8)\|h`, terminated by `0xFFFF`. | `src/intro_menu_image_script_cmds.inc` |
| `swos_font.py` | Injects new glyphs into free sheet cells; emits the C wiring. | `SWOS_Font/` |
| `glyphs.txt` | Editable glyph art (`*`=ink, `-`=blank) — the source of truth after the first run. | `SWOS_Font/` |
| `integration.txt` | The generated C wiring to apply (image-script + conversionTable + lowercase map). | `SWOS_Font/` |

---

## Process — add a glyph

1. **Decode** the stock charset to a PNG with **SwosGfx** → `CHARSET.png` (the 320×256 sheet
   `swos_font.py` reads).

2. **Author the glyph** in `swos_font.py`:
   - Punctuation → `PUNCT_SMALL` / `PUNCT_BIG` (hand-drawn `*`/`-` art).
   - Accented capitals → `ACCENTS` (derived from a base letter + an accent row; auto-compressed
     one row so the accent fits and the glyph keeps its native height, like the stock Ä/Ö/Ü).
   - Ligatures / symbols / currency → `LIGATURES` (hand-drawn art). `$` is derived from `S`
     (`dollar_art`); `€` is an authored C-with-two-bars.
   - **Codepoints > 0xFF** (no single Latin-1 byte: `Œ`, `Ÿ`, `€`) → assign a **spare
     conversionTable byte** in `CP_OVERRIDE` — a byte whose `conv[byte−0x20]` is 0. Current
     map: `Œ`=0x8C, `Ÿ`=0x9E, **`€`=0x9D** (chosen next to the stock pound 0x9C).

3. **Run** `python3 swos_font.py`. It:
   - Seeds the new glyph into `glyphs.txt` (the drop shadow, palette idx 8, is added
     automatically — author the white shape only).
   - Writes `new_charset.png` (sheet with the glyph injected into a free cell), `preview.png`,
     `new_charset_zoom.png`, and **`integration.txt`** (the C wiring).
   New glyphs are placed at sprite slots **1220+** — above the game's 0–226 (charset/menus)
   and the 227–1219 DOS player overlay — so they survive in **both** Amiga and DOS-graphics modes.

4. **Refine** the shape in `glyphs.txt`, check `preview.png`, re-run until it looks right.
   (`glyphs.txt` is only seeded once; after that your edits there are the source of truth.)

5. **Encode** `new_charset.png` back to `CHARSET.RAW` with **SwosGfx** — verified commands
   (`SwosGfx.exe` runs from WSL via interop *or* natively on Windows; run from `SWOS_Font/`):
   ```sh
   GFX="/mnt/c/Projects/GitHub/SwosGfx/SwosGfx/bin/Debug/net8.0/SwosGfx.exe"

   # a) PNG -> 8-bit indexed BMP — SwosGfx encodes from a BMP, not a PNG.
   python3 -c "from PIL import Image; Image.open('new_charset.png').save('new_charset.bmp')"

   # b) Amiga OCS font (bin/grafs/CHARSET.RAW). CHARSET uses the "menu" palette; 4 bitplanes
   #    + RNC are SwosGfx's defaults. -palette IS REQUIRED on encode (not auto-detected).
   "$GFX" -amiga -bmp -output=raw -palette=menu  new_charset.bmp  CHARSET.RAW

   # c) ALWAYS verify lossless: decode the new RAW back and diff indices vs the source BMP.
   #    Must be 0-diff (pixel-identical) — proves the encode round-trips the exact sheet.
   "$GFX" -amiga -raw -output=bmp  CHARSET.RAW  roundtrip.bmp
   ```
   Verify (c) in Python: `Image.open('new_charset.bmp').convert('P')` indices ==
   `Image.open('roundtrip.bmp').convert('P')` indices, 0 differences.
   - **AGA** (`bin/grafs_aga/CHARSET.RAW`): same, add `-bitplanes=8`.
   - **DOS** (`bin/grafs_dos/CHARSET.DAT`, `bin/grafs_2020/charset.dat`): the per-sprite path
     (`-dos -sprites-import <dir>` reading `sprNNNN.bmp`). The on-screen **text font is always
     the Amiga `grafs/charset.raw`** even in DOS-graphics mode, so the OCS encode above is what
     currency/menu glyphs need; the DOS `.dat` only matters for in-match digit overlays.
   - **Deploy**: copy the verified `CHARSET.RAW` over `bin/grafs/CHARSET.RAW` (the file the game
     loads — `strings_data.c`: `aGrafsCharsetRa = "grafs/charset.raw"`).

6. **Apply `integration.txt`** to the game — `scripts/gen_font_ext.py` does the mechanical part:
   ```sh
   python3 scripts/gen_font_ext.py   # integration.txt -> src/font_ext_script.inc + font_ext_conv.inc
   ```
   It regenerates the image-script rects (`font_ext_script.inc`, appended to `introMenuImageScript`)
   and the `conversionTable` `SB(idx, small, big)` entries (`font_ext_conv.inc`, `#include`d by
   `FontExt_ApplyConv` in `swos_globals.c`). Then, by hand:
   - **> 0xFF codepoints only**: add the spare-byte case to `tr_dup` (`src/translations.c`) —
     e.g. `cp == 0x20AC ? 0x9D` for `€` — so translated text round-trips through `tr()`.
   - **Deploy**: copy the re-encoded `CHARSET.RAW` over `bin/grafs/CHARSET.RAW`, then rebuild.

   > **Glyph order is logical, NOT minimal-diff.** Put each new glyph where it reads naturally
   > (`€` next to `$`, etc.). Inserting one shifts the sprite indices below it — that's expected
   > and fine: `gen_font_ext.py` regenerates the FULL wiring from `integration.txt`, so the
   > `.inc`s always match the sheet. **The sheet (`CHARSET.RAW`) and the `.inc`s are a matched
   > pair** — re-encode AND re-run `gen_font_ext.py` together and redeploy `CHARSET.RAW`, or the
   > glyphs misalign. The same applies to every charset you ship (AGA, DOS) — re-encode each.

---

## Currency symbols

A glyph becomes the money symbol via `Currency_Set(name)` in `src/string_utils.c`, which
patches the leading byte of every money format string (`aPound`, `a1k`, `a0m`, `a01m`):

| `SWOS.ini [general] currency=` | byte | glyph |
| --- | --- | --- |
| `pound` (default) | 0x9C | `£` (stock) |
| `dollar` | 0x24 | `$` (font ext) |
| `euro` | 0x9D | `€` (font ext) |

The supported languages (`en/de/es/fr/it/nl`) need only these three: `£` (UK English),
`$` (US English), and `€` for the five Eurozone languages (de/es/it/fr/nl). No other
currency glyph is required — the Swiss franc is the only near-miss and it's written `CHF`,
not a single symbol.

---

## Files

- `SWOS_Font/swos_font.py` — generator · `glyphs.txt` — editable art · `integration.txt` — wiring
- `SwosGfx.exe` — RAW ↔ PNG codec (Amiga + DOS)
- Game: `src/intro_menu_image_script_cmds.inc` (image-script), `src/swos_globals.c`
  (conversionTable), `src/swos_amiga_data_stubs.c` (sprite-table size), `src/translations.c`
  (`tr_dup` for > 0xFF), `src/string_utils.c` (`Currency_Set`)

---

## Expanded UI — inventario renderer e regole di ingombro (2026-11-07)

Verificato nel ramo `dev/0.10`:

- Font SWOS originale: **small 6 px**, **big 8 px**, charset sprite-sheet `CHARSET.RAW` e `conversionTable` (dettagli sopra). Le **larghezze non sono tutte fisse**: `GetTextWidth` somma `sprHeader->pixWidth + charSpacing` e usa `spaceWidth` per gli spazi.
- Font grafico Expanded aggiuntivo: `sorgenti/src/expanded_club_font.h`, matrici ASCII 32–126 **5×8 px**. Il generatore `sorgenti/tools/generate_club_business_art.py` usa avanzamento **6 px** nel proprio `ptext`. Questo è verificato per quel generatore, non prova che ogni schermata runtime usi quel font.
- Funzioni del renderer `sorgenti/src/text_render.c`: `GetTextWidth(text, charsTable)` misura; `DrawMenuText(x,y,color,text,charsTable)` disegna; `DrawMenuTextCentered` centra mediante larghezza misurata; `DrawMenuTextRightAligned` allinea a destra. Il disegno di ciascun glifo passa per `blitStringToFrontBuffer`/per il percorso SDL `blitGlyphBitplanes`.
- Prima di progettare un menu, determinare dal **codice della schermata attiva** la `charsTable` usata, i valori `spriteIndexOffset`, `charSpacing`, `spaceWidth`, le coordinate effettive e l'eventuale scala. Per una stringa di esempio o con importi lunghi, misurare con la stessa tabella o equivalente esatto; non usare larghezze approssimative.
- **Regola per asset UI con righe e due bordi orizzontali da 1 px**: `1 bordo + m margine + h glifo + m margine + 1 bordo`, con `m >= 1 px` e uguale sopra e sotto. Font alto 6 px: minimo **10 px** (`1+1+6+1+1`). Font alto 8 px: minimo **12 px** (`1+1+8+1+1`). Se la riga è più alta, ripartire simmetricamente il margine; non estendere questa regola all'intero motore.
- Per Business Screen pulita: testi, cifre, stadio, faccine e focus/selezione sono elementi dinamici; le icone delle facilities sono statiche. L'asset con testi di prova precede la versione pulita e deve usare coordinate riproducibili, non font inventati.
