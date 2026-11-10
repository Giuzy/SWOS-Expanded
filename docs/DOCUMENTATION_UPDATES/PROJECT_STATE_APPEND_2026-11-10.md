
---

## SWOS_DOC_SYNC_2026_11_10 — Stato e decisioni aggiornati (10/11/2026)

**Riferimento esaminato:** branch `dev/0.10`, HEAD `f33041df7204936da22e6a066977b0dc382093b4`, versione sorgente `0.10.75`. **Non è stata generata né testata una nuova build** in questo aggiornamento. `docs/GAME_DESIGN.md` diventa la fonte tematica delle scelte e idee di gioco, da consultare insieme al codice.

### Decisioni confermate

- **Design grafico:** SOLO TITLE, NEW CAREER, LOADING hanno grafica considerata finale. Le vecchie approvazioni grafiche di BUSINESS/SQUAD/MATCH STATS/TRANSFER 1 e degli altri menu sono **superate** per il *layout e gli asset*; dati e callback restano da salvaguardare. Tutte le altre schermate saranno ridisegnate con approvazione singola.
- **Obsolescenza:** tutti gli asset grafici della precedente fase di menu diversi dalle tre schermate finali sono **OBSOLETI DA SOSTITUIRE**, non necessariamente **orfani eliminabili**. Il prossimo BAT di pulizia deve eliminarli **solo quando non vi sono più dipendenze attive**; preservare fallback necessari, sprite condivisi e asset di gameplay. Nessuna cancellazione effettuata ora.
- **Base dati prima di tutto:** completare la carriera con squadre/competizioni già disponibili. L'espansione fino a molte divisioni/paesi (obiettivo discusso: fino a 8 livelli per paese quando pertinente) e relativi database è posticipata all'ultima fase.
- **Mondo unico:** tempo **a date/eventi**, non un incremento soltanto settimanale; i paesi possono avere calendari differenti e coppe infrasettimanali. Simulare risultati, classifiche, mercato, ritiri e progressione a ogni evento; elaborare periodicamente economia e allenamenti. Maggior dettaglio solo dove serve, senza partite simulate fisicamente per ogni club.
- **Settimana in UI:** numero della settimana visibile nella maggioranza delle schermate della carriera e una sola origine temporale. Convenzione ISO vs stagione ancora da definire, quindi NON hardcodare una formula nuova senza discuterla.
- **Nuova azione:** ADVANCE TO NEXT EVENT in CALENDAR e UNEMPLOYED; deve avanzare attraverso tutti gli eventi senza ignorarli o prendere decisioni autonome.
- **Chairman:** nome/età nel contratto e WELCOME; ritiro e successore automatico nello stesso club (soglie ancora da definire).
- **Giocatori:** YOUTH/RES/TITOLARE come status di rosa, sviluppo/ritiro, giovani stagionali e potenziale conosciuto per i giovani della propria squadra, esterni da osservare; niente prestiti TRIAL automatici. Semantica originale dei campi SWOS da verificare prima di cambiare codice.

### Codice esaminato e limiti da correggere

- `sorgenti/src/expanded_career.c`: il disoccupato usa `s_waitWeeks`; `Expanded_UnemployedWait()` incrementa settimane e genera offerte; `Expanded_WaitForRehire()` nota che il reingaggio avvia una nuova stagione anziché entrare nella stagione reale in corso. **Da correggere nel Living World Engine**, non considerare già risolto.
- `sorgenti/src/expanded_economy.c`: CALENDAR e UNEMPLOYED condividono in parte il rendering delle fixture e dei pulsanti. I layout sono **da riprogettare**; non cancellarne prima le risorse RAW ancora caricate.
- `sorgenti/include/build_info.h`: `SWOS_EXPANDED_VERSION` vale `0.10.75` alla lettura. È distinto da vecchi screenshot/riassunti 0.10.54; verificare di nuovo il prossimo HEAD.
- Alcuni file `assets/title/buttons/*_OBSOLETO.png` e `data/expanded/*_OBSOLETO.*` sono già marcati come obsoleti nel nome. **Non eliminare** i file del nuovo TITLE/LOADING/NEW CAREER solo per errore di percorso o per somiglianza.

### Ordine operativo corrente

1. Audit del tempo e del calendario; definire transizione ad eventi cronologici persistenti con il database attuale, test regressione.
2. Offerte che rispettano stagione reale, chairman, contratto; grafica OFFER demo prima della versione finale.
3. NEXT EVENT + WEEK visibile quasi ovunque, senza fissare il vecchio layout.
4. Giovani, scouting, mercato AI, ritiri e finanze/allenamento.
5. Verifiche Windows/joypad/save di lunga durata; grafica dei menu rimanenti; database allargato solo alla fine.

### Test e consegne

**Solo documentazione predisposta.** Nessuna compilazione, nessun test Windows, nessuna modifica al codice di gioco, nessun commit e nessun push eseguiti per questa raccolta. Aggiornare questo stato dopo i test reali; dettagli e stati [APPROVATO]/[PROPOSTA]/[DA DEFINIRE] sono in `docs/GAME_DESIGN.md`.
