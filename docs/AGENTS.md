## 22/11/2026 — INPUT APPROVATO ULTIMO: SOLO TASTIERA E JOYPAD, MOUSE DISATTIVATO
La decisione utente «togli i controlli mouse da tutto il gioco lascia solo joypad e tastiera» prevale sulle indicazioni precedenti di conservazione mouse in §5 e negli esempi storici. In Windows SDL2 non accettare click, hover, drag, right-click, focus o selezione via mouse in nessuna modalità/schermata, compresa conferma uscita. Servono navigazione pad/keyboard coerente e tutti i bottoni raggiungibili; B=BACK, A=SELECT, X=INFO solo dove esistono callback; ESC/ENTER tastiera e text input intatti. Gli eventi OS finestra/chiusura/Alt+Invio NON sono controlli mouse in-game e rimangono. Touch web virtual pad è separato: non cancellarlo senza audit multipiattaforma. Rif. RG canonical 22/11, HEAD dev/0.10 v241, preparati mouse platform f5e939..., input f08a3a...; Windows/GP N/P. Ogni riferimento storico al mouse nelle righe successive è SUPERATO da questo paragrafo.

## 21/11/2026 — ULTIMA DECISIONE USER: OFFERTA FACOLTATIVA NON BLOCCA MANAGER ASSUNTO, NEW OFFERS LAMPEGGIA (v240)
**PRECEDE le precedenti note v238/v239 sugli stop per qualunque nuova offerta.** Se il manager è già vincolato a un club OG, una nuova offerta di lavoro è facoltativa: non blocca né il postmatch automatico né FORWARD TIME manuale. Rimane visibile in JOB OFFERS, evidenziata in NEW OFFERS con il lampeggio del menu OG originale. Solo un'offerta **accettata** (_teamNoOfferAccepted!=0xffff), esonero, fine carriera/stagione, recupero o altri stop realmente obbligatori sospendono il coordinatore. Il cambio club mantiene unica autorità OG, data e risultati; il ramo sorgente mantiene però rebuild slot locali (V), test completo di cambio club/CONTINUE Windows ancora necessario. Fonte ZIP Win v238+CAR readonly 19/09 e RG-014/010; base dev/0.10 HEAD fc60e9bb v239, patch v240 in preparazione ISO puro PASS, compilazione Win NON PROVATA.

## 21/11/2026 — ULTIMA RETTIFICA USER: NEXT MATCH ORIGINALE OG E VIEWPORT SENZA PADDING (v00.13.239)
PRECEDENZA su istruzione v238 in testa che nascondeva NEXT MATCH: user «rimetti next match al comportamento OG, errore mio»: l'entry10 CAREER Bolivia torna visibile con vera callback OG e stato nativo ContestSetButtons, senza simulatore sostitutivo. La entry7 conserva PLAY MATCH alla data / FORWARD TIME altrove. Frecce rosse non devono scorrere oltre prima/ultima gara, con focus centrato soltanto quando lo spazio esiste. Blocco dopo primo DERBY WIN v238 segnalato e NON risolto/provato in assenza di log/diagnostica dell'incontro. PRECHECK e cause/test dettagliati nel registro RG-014. Versione sorgenti v00.13.239; Win compile/gameplay/CAR NON PROVATI.

## 21/11/2026 — ULTIME DECISIONI USER PREVALENTI (UNICO COMANDO + COPA INTERA OG)
**Supersedes only prior 21/11 v234 requirement that NEXT MATCH must remain VISIBLE in the new covered CAREER calendar:** show a SINGLE primary button FORWARD TIME when NO OG manager fixture is due today; PLAY MATCH only ON the real due date; remove NEXT MATCH visibly (do not delete original handler for other games/worlds). Red arrows must scroll the real chronology, the exact OG fixture is highlighted blue in center even for rows1..14, with matching dates and true Copa phases/scores. TODAY is actual civil clock, not next-match date before the action. After a completed original OG matchday and successful durable CAR checkpoint, advance automatically by the SAME coordinator to the NEXT manager fixture or first interactive stop, never play manager without input or add redundant result screen.
**Newest additional explicit authorization:** «potresti metterla tutta col nuovo metodo?» Apply the same original OpenSWOS tournament MENU also to Copa Bolivia **KO type1** (4 QF ties × two legs, 2 SF × two legs, 1 Final × two legs = 14 matches), in addition to 64 group type2; no direct score-only substitute. Preserve identical original OG callbacks, CPU/manager, bracket, qualifiers, knockout rounds and per-match CAR, no new second simulator or clock. Group menu is ShowDiyLeaguesTournamentMenu, KO is ShowDIYCupMenuWithParams and original DIYCupOnSelect, these need different PRE/POST. Code experiment target 00.13.238 staged only for WORLD2627 Bolivia top division (other worlds/CLASSIC/DIY/SEASON original behavior), known RG-014 OPEN until Windows end-to-end and SAVE/EXIT/CONTINUE. Same RG PRECHECK, source/guide SHA and recovery checkpoints in Issue #1; prior historical guidance retained for regression evidence, not precedent that blocks latest approval.
Current base dev/0.10 HEAD04c43027 v237. Latest source preparati: career_status0ed5acf70dbbb77aebcb256448c48580f874d562, diy_competition323649b3ce20e6cefd4e67f6ec5c0f1d91c98b8d, midseason a236eccd77fd815fe921c6d7cf6ac9280908f998, menus55cb99928589f179c8c07a615b6bacfdfd82331c, UI6607d2038094ded3f5a53335b57f21e94b73c5ac, KO modal headerca064e2cd4a8e80a27c53ecd29caea652469e60f, pure action599e69f2 and viewport9f02194a. ISO KO 14× C11 GCC14.2 with negatives PASS exact blob SHA; scoped source ST23/23, full WIN GP/CAR/KO NO. Single commit only when requested (user did request), version build_info v238 target, no unrelated branch.

## 21/11/2026 — REGOLA UI CALENDARIO: FRECCE ROSSE REALI E FOCUS PARTITA CENTRATO
Decisione utente dopo 00.13.237: nell'elenco delle gare del club, i pulsanti rossi SU/GIÙ devono scorrere realmente il calendario mostrato e la gara OG corrente di PLAY MATCH deve essere selezionata in blu **al centro** del pannello, non soltanto visibile presso il margine. Quando l'utente preme le frecce, può esplorare liberamente le altre gare; dopo una nuova gara OG attuabile o al rientro nel calendario, focus torna automaticamente centrale. Centro del pannello compatto 14 righe n.7 con padding vuoto agli estremi, MAI partita inventata e MAI cambio dell'unico OG fixture target; focus su un OG fixture soltanto, nessuna selezione in caso di ambiguità. Mouse e controller devono usare stesse callback/hitbox coerenti con frecce realmente disegnate, altri mondi/modalità conservano menu nativo. Bug/source V: `expanded_economy.c::ui_render_merged_calendar` riparte da page0 ogni frame, `menus.c::ContestMenuScrollUp/Down` agisce su TSKC anziché sulla rappresentazione unificata Bolivia; `contestMenu.json` ord16/17 hitbox non coincide con glyph disegnato (font/geometry `sorgenti/docs/FONTS.md`). Nuovo FIX preparato a blob, NON committato, dettagli registro RG-014 e `docs/ENGINE_CODE_DISCOVERIES.md` del 21/11. Confermare dopo prova Windows: input mouse/dpad, frecce prima/dopo gara Copa/DPN, hover e blu centrale, 14 righe, CAR intatti; il test C11 isolato non è screenshot/GP.

## 21/11/2026 — REGOLA USER PRIORITARIA: IL RISCHIO NON DEVE PARALIZZARE LE IMPLEMENTAZIONI
**Decisione vincolante:** «non devi mai trattare ogni rischio non risolto come motivo per non tentare l'implementazione». GitHub contiene i commit precedenti, perciò il ramo `dev/0.10` è per sviluppare e verificare modifiche REALI e reversibili: il PRECHECK RG e le prove servono a identificare gli invarianti e programmare il test, **NON** a usare rischi generici come scusa per non scrivere codice. Dopo l'audit, costruire un esperimento autentico per la feature richiesta, prima in un perimetro verificabile, poi ampliare. Usare i blob/tree Issue #1 e i commit come punti di recupero; gli esperimenti non committati non distruggono HEAD. Se una prova non passa, documentare il bug e tentativo fallito subito nelle guide e Issue con SHA, correggerlo oppure non includerlo nel commit: non abbandonare per sola incertezza, non promettere GP Windows se non testato. **Non** inventare surrogati, risultati, ACK o tasti fittizi; il divieto dei workaround non equivale a divieto di sperimentare sulla soluzione originale. Proteggere i CAR validi (copie di test/salvataggi atomici), gli invarianti OG 50Hz/ABI e i rami main/release/backup. Un blocco al commit di una parte richiede una prova specifica che quella patch può produrre danno non contenibile, non una generica ipotesi; il resto autorizzato procede. Solo un commit finale dopo richiesta esplicita dell'utente (questa richiesta lo autorizza). Aggiornare in parallelo il testo delle istruzioni di progetto ChatGPT (massimo 8000 caratteri) in deposito versionato; l'utente applica manualmente le impostazioni ChatGPT che l'agente non può scrivere tramite GitHub.
**Fonte:** richiesta user dopo commit v00.13.235, branch HEAD `3a0b856fe4d2056cfdf774c3d2faaa186c4cf001`, situazione RG-014. Stato regola APPROVATA, non va interpretata come approvazione a distruggere salvataggi o dichiarare PASS non ottenuti. Questa regola ha precedenza su formulazioni storiche che dicevano «fermarsi davanti a ogni rischio»; resta vigente il blocco ai surrogati.

## 21/11/2026 — DECISIONE USER SUI TASTI OG v234 (RICHIAMO PRIORITARIO)
Fonte: screenshot e ZIP `DIAGNOSTICA_SESSIONE_1(20261121-181238).zip`, 00.13.234 Windows reale; calendario DPN/Copa dichiarato CORRETTO dall'utente. Non modificarlo per eliminare il vecchio SIMULATE MATCH aggiuntivo. Nel menu CAREER ripristinare entry10 **NEXT MATCH** originale `_aNextMatch`, callback OG `NextMatch`→`Career_NextMatch`; NESSUNA intercettazione boliviana ordinal10 / simulazione score-only accessoria. PLAY MATCH entry7 può continuare a usare il solo OG resident manuale attuale, senza mentire che questo fornisca il ciclo OG classifica/VIEW RESULT: RG-014 T0-3/T0-5 resta OPEN. Non toccare entry10 disabilità/visibilità native, non bypassare `Expanded_BoliviaLeaguePreflight` e non saltare Copa anteriore. Nuove modifiche solo con PRECHECK, guide + Issue #1 e QA Windows; source blobs preparati, NON build né commit.

## 21/11/2026 — REGOLA ASSOLUTA ANTI-SCORCIATOIE / ANTI-SOSTITUZIONI NON AUTORIZZATE (USER)
**MAI, MAI E POI MAI** implementare una soluzione diversa, ridotta, surrogata, simulata o «che sembra funzionare» soltanto perché non si riesce a fare quanto richiesto. Il fatto che la nuova scorciatoia dia un risultato plausibile, passi un controllo headless o usi una funzione OG per il solo score **non la rende la funzione approvata**. Non inventare pulsanti, passaggi, guardie o percorsi UI alternativi; non tagliare la sequenza nativa, non rinominare semplicemente un callback per mascherare il cambio di comportamento. Se la soluzione autorizzata presenta difficoltà, fermarsi sul punto, documentare causa/contratto/test/limiti e **chiedere prima** se un eventuale compromesso sia accettabile. Senza consenso, lasciare OPEN e preservare l'implementazione originale funzionante; NON committare un surrogato come fix e non dichiarare completo. Annotare immediatamente anche gli esperimenti falliti nelle guide e nella Issue #1, con SHA dei preparati da scartare.
**Caso motivante V:** RG-014 Copa Bolivia v229: il comando dedicato SIMULATE MATCH produce un solo score OG attraverso `Expanded_OGWorld_BoliviaManagerPlay`, ma salta l'intero menu e ciclo competizione OG mostrato in video (classifica prima di scegliere, PLAY/COACH MATCH, VIEW MATCH/VIEW RESULT, gare CPU con aggiornamenti successivi). Anche una successiva patch che apre il menu OG e lo chiude dopo una sola data **non equivale** al comportamento richiesto e l'utente l'ha esplicitamente respinta. Prima di dichiarare corretta la Copa, deve essere integrato l'intero ciclo OG vero, dalla stessa autorità resident+RNG+fixture+CAR, senza secondo motore; Windows gameplay e CONTINUE devono provarlo. Questo documento ha precedenza su suggerimenti di scorciatoia in note storiche, che rimangono fonte audit e non autorizzazione.


## 21/11/2026 — REGOLA UI APPROVATA: FOCUS SULLA GARA DI PLAY MATCH, NEXT MATCH RIMANE OG

Per OGNI calendario Expanded/Career unificato il rigo blu selezionato DEVE rappresentare lo STESSO e unico fixture OG (mondo/edizione/tipo/fase/id/coppia/data) che il vero pulsante PLAY MATCH sta per eseguire. **Rettifica USER del 21/11 dopo test build 00.13.234:** il pulsante superiore destro della carriera NON deve essere SIMULATE MATCH supplementare; ripristinare testo originale **NEXT MATCH** e il suo vero callback OG, senza fare `Expanded_BoliviaManagerIntercept(...,10)`, e senza creare una seconda scorciatoia di score Copa. Finché RG-014 non integra la modale completa OG Cup con classifica/PLAY/COACH/VIEW RESULT/CPU, il NEXT MATCH originale può essere fermato dal normale preflight anti-salto quando la gara Copa precede la DPN: non attribuirgli un falso fixture blu, non bypassare l'ordine eventi e non chiamare questo la modale Copa completata. Non lasciare blu su Liga quando PLAY MATCH ha Copa prima; usare lo stesso plan readonly per il focus. Se la gara e' oltre il 14° rigo, renderla visibile senza avanzare TSKC, cambiare callback/RNG o simulare. Collisioni/ambiguita' non devono mostrare un focus giocabile. Non alterare calendari approvati, CLASSIC/DIY/SEASON, 9697, CAR validi o mappatura fasi Competition Info.

Risultati pre-01/07: usare SOLO risultati testimoniati da OG con identita' unica, oppure segnalare sconosciuto, senza scrivere nella keycache o inventare punteggi. ATTENZIONE: catch-up OG primo anno puo' produrre score simulati che NON equivalgono a risultati storici REAL 2026 verificati; import storico reale resta requisito separato. Root/test/limiti: RG-001/014, Competition Engine Guide, FONTS e workflow, HEAD 58058d99 v228; patch preparata per futura 229, Win/GP NO, commit NO.

## 21/11/2026 — OBBLIGO STESSA FASE IN COMPETITION INFO E CALENDARIO (DECISIONE USER)

Per OGNI competizione Expanded/Career, il campo fase delle singole gare in calendario deve descrivere la STESSA fase mostrata da COMPETITION INFO, dalla medesima fonte metadata OG e stessa funzione di mapping delle etichette; mai duplicare tabelle divergenti in due schermate. La fase Derby nella Copa Bolivia è una partita interserie della GROUP STAGE, non un round KO; le altre sono QUARTER-FINAL, SEMI-FINAL, FINAL solo dopo che OG ha generato gli accoppiamenti. Compromessi del font nativo (COPA+PHASE, nome completo nel viewer) richiedono dimostrazione screenshot prima di definitiva approvazione. Per un manager devono esistere PLAY MATCH manuale e simulazione della STESSA gara OG (non LWE); il test annullato non deve creare risultati, witness o CAR. Prima di modificare il calendario, verificare le date individuali, rinvii e stato reale al 1/7; pausa giugno Bolivia NON globale. Retrocompatibilità CAR e gameplay Classic/DIY/Season sempre vincolanti. L'orologio civile per fixture resta la fase motore successiva alla build di chiusura bug 227.

## 21/11/2026 — DECISIONE FUTURA APPROVATA: DATA CIVILE PER GARA FUORI DALLE FASCE OG (NON ANCORA IMPLEMENTATA)

**Precedenza nella ROADMAP FUTURA solo Expanded/Career:** l'utente ha approvato di sostituire l'autorita' delle date dei fixture OG (ottavi mensili/fasce) con **un unico calendario SWOS Expanded a data civile esatta per incontro**, da realizzare **DOPO** la prossima build che risolve e prova gli ultimi bug 00.13.227. Questa e' una modifica ESPLICITA della precedente decisione «OG unico calendario delle date». Restano **vincolanti**: UN SOLO clock/calendario effettivo, nessun motore LWE parallelo, OG unica autorita' SPORTIVA per incontri, score, classifiche, KO, qualificazioni e trofei, fisica 50 Hz, CLASSIC/DIY/SEASON, CAR validi, prova manager+CPU stesso fixture. Non modificare ora i meccanismi OG a fasi del runtime perche' sono in uso nel 227, e non spostare una data cambiando solo il suo display; bisogna progettare migrazione integrata dei writer/reader, fixture identity, CAR e anti-replay.

**Componenti ESISTENTI da riusare (ST su HEAD c1ec6b6021adf7db9878b08f0b01b738dddcc61e):** `expanded_event_scheduler.h` calendario eventi ID64 e giorno civile, senza live dispatch/CAR; `career_init.c::Expanded_CompetitionCoordinator_Advance` coordinamento OG→clock, scheduler shadow July PEEK-only; `expanded_competition_identity.h` ledger key contrattuale non attivo; `expanded_world_live.c::ExpandedWorldLive_AdvanceToDate` cursore civile. Per la prossima fase motore unificare clock + registry + scheduler attuali prima di ampliare qualificazioni, storico e tre piloti. Contratti e test in `GAME_DESIGN.md`, `PROJECT_STATE.md`, `COMPETITION_ENGINE_IMPLEMENTATION_GUIDE.md`, `REGRESSION_HISTORY_AND_DEPENDENCIES.md` (RG-001 e dipendenze). **Nessun codice o commit da questa nota**, non annunciare WIN/GP PASS.

## 20/11/2026 — REGOLE OBBLIGATORIE DI COLLABORAZIONE, AUDIT E REGRESSIONI (VIGENTI)

**Precedenza:** questa sezione è il contratto operativo attuale. I paragrafi datati sotto sono storia del progetto e non trasformano implementazioni parziali in funzionalità completate. Consultare il registro canonico **[REGRESSION_HISTORY_AND_DEPENDENCIES.md](REGRESSION_HISTORY_AND_DEPENDENCIES.md)** e la procedura **[DEVELOPMENT_WORKFLOW.md](DEVELOPMENT_WORKFLOW.md)** PRIMA di modificare codice del motore.

1. **Correggere esplicitamente le affermazioni tecniche sbagliate dell'utente** con motivazione e alternativa; rispettare le intuizioni quando sono supportate da evidenze, anche se smentiscono una diagnosi precedente. Non assecondare né contraddire automaticamente.
2. **Avvisare PRIMA di scrivere codice** se la richiesta è impossibile, richiede un cambio architetturale significativo o non è realizzabile nel singolo intervento. *Difficile non significa impossibile*: illustrare opzioni, costo tecnico e compromessi, chiedendo approvazione sulle scelte di game design/architettura.
3. **Niente build quasi vuote o incomplete presentate come concluse.** Specificare con precisione cosa è implementato, cosa manca e cosa è soltanto preparato; non incrementare versione per la sola documentazione.
4. **Spiegare brevemente la logica tecnica di ogni bug** a un interlocutore che conosce le basi di C, C++ e Java: condizioni, ramo/struttura coinvolta, invarianti violate, conseguenze; non servono lezioni elementari.
5. **Distinguere sempre causa verificata, ipotesi e problema non compreso**; distinguere separatamente codice, test statico, C isolato, headless, Windows compilation, Windows gameplay e SAVE/CONTINUE. Non chiamare PASS una prova inesistente.
6. **Prima di toccare una routine mappare chiamanti, scrittori, lettori e dati condivisi**, non solo grep del nome: effetti su unico calendario OG, residenti competizioni, manager, world switch 9697/2627, qualifica, CAR, RNG, viewer e diagnostiche. Consultare incidenti analoghi nel registro; definire test di regressione e scenari di errore PRIMA del fix.
7. **Prima di dichiarare «finito» eseguire i test pertinenti e le regressioni prevedibili**, specificando SI/NO/NON PROVATO per ambito. Se Windows gameplay, manager o legacy CAR non sono stati realmente testati, dirlo. Un test headless 70 stagioni non dimostra il calendario manager.
8. **Coinvolgere l'utente prima di assumere compromessi** su regole delle competizioni, formati, storia al 1 luglio, manager/CPU, calendari, migrazione save e architettura. Se fonti approvate confliggono, chiedere quale prevale prima di agire su quel punto; continuare sul resto.
9. **PRECHECK OBBLIGATORIO prima di QUALSIASI modifica, non soltanto al motore.** Prima di cambiare codice, test, script, dati, asset, UI, salvataggi o guide, rileggere i casi pertinenti del [REGRESSION_HISTORY_AND_DEPENDENCIES.md](REGRESSION_HISTORY_AND_DEPENDENCIES.md), l'indice file→bug→dipendenze→test e gli eventuali sottocasi; controllare le correzioni passate, i tentativi falliti e gli effetti su chiamanti, writer, reader, strutture, invarianti e RG-T. Farlo **di nuovo quando emerge un bug/problema o cambia l'ambito**, anche se il registro è già stato consultato all'inizio. Cercare prove e fonti/codice collegati prima di proporre o applicare un fix; l'assenza di una scheda non dimostra assenza di regressioni.
10. **CONOSCENZA ACQUISITA → GUIDE AGGIORNATE, SENZA ATTESE.** Ogni nuova scoperta utile per implementazioni future sullo stesso argomento — bug, dipendenza, causa, limite OG/ABI/CAR, comportamento verificato, tentativo fallito, workaround, test, contratto di routine o decisione — va registrata **subito, prima di preparare il passo successivo**, nella guida tecnica pertinente (per es. ENGINE_CODE_DISCOVERIES, PROJECT_STATE, COMPETITION_ENGINE_IMPLEMENTATION_GUIDE, SAVE, GAME_DESIGN, IMPROVEMENTS o WORKFLOW); aggiornare **anche** il registro canonico RG se riguarda un incidente/dipendenza riutilizzabile, riusando l'ID o aggiungendo sottocasi senza duplicati. Annotare fonte, versione/HEAD, file e funzioni, writer/reader, causa V/I/?, esito ST/ISO/WIN/GP/NP, limiti e test di regressione, distinguendo osservazione da fix. Preparare la modifica documentale e depositarne immediatamente contenuto recuperabile e SHA nella [Issue #1](https://github.com/Giuzy/SWOS-Expanded/issues/1); **includere tali guide nello stesso commit della correzione/build pertinente, solo quando il commit è autorizzato**. Non lasciare scoperte nelle sole risposte chat, non anticipare commit e non incrementare versione per sole guide.

**Gate applicativo:** analisi sintomo → consultazione archivio → dipendenze e invarianti → piano dei test → modifica autorizzata → prove e regressioni → registro aggiornato con esiti dimostrati → checkpoint immediato nella **[Issue #1](https://github.com/Giuzy/SWOS-Expanded/issues/1)** dopo ogni preparato → commit consentito solo se esplicitamente autorizzato. **Nessuna seconda autorità** per gare, risultati, competizioni o calendario; preservare fisica OG 50 Hz, CLASSIC/DIY/SEASON e i CAR esistenti. Le guide non sostituiscono il test di gameplay.

## 20/11/2026 — MANUALE OPERATIVO CANONICO COMPETITION ENGINE (217–225)
Per OGNI nuova competizione o modifica di formato/fixture usare prima **[COMPETITION_ENGINE_IMPLEMENTATION_GUIDE.md](COMPETITION_ENGINE_IMPLEMENTATION_GUIDE.md)**: percorso A inventario/B validazione/C unico OG/D rose, file/ABI, ciclo edizioni, date 2026 e template 2627, vista readonly, KO, CAR, manager e QA. Include ricostruzione DIFF+chat 00.13.217–225, errori reali `0x5555`, `0xC0000005`, 16→32, compId164/semifinali, 2027 participant-only, promosse ordinal>15 e correzioni. Le sezioni storiche sottostanti restano a testimonianza del momento, non sovrascrivono le decisioni successive del 20/11. **Manuale/documentazione non equivale a implementazione o prova Windows.** Conservare checkpoint nella Issue #1 dopo OGNI cosa preparata; un solo commit dopo «committa».

## 20/11/2026 — REGOLA APPROVATA SUCCESSIVA: FORMAT/DATE 2026 RICORRENTI NEL MONDO 2627
**Decisione utente prevalente per stagioni future non conosciute:** una volta fissati e verificati formato e date per-gara dell'edizione 2026 di ciascuna competizione nel mondo 2026/27, quella definizione si RIPETE nelle edizioni di gioco 2027, 2028 e seguenti (stessi giorni/mesi/turni, anno aggiornato), fino a un aggiornamento ESPLICITO di regole/calendario nel progetto. Non e' una previsione del futuro: riportare la fonte REAL_2026 per 2026 e GAME_TEMPLATE_2026 per gli anni successivi. Ogni nuova edizione deve aggiornare le partecipanti da club/qualificazioni OG correnti, NON i punteggi o i vincitori precedenti. Creare il NUOVO residente nel solo OG a START/rollover, avanzare le gare una volta per data, non duplicare fixture, non sovrascrivere i CAR, rispettare gli ID edizione e verificare collisioni con tutte le altre competizioni. **Questa scelta 20/11/2026 sostituisce la precedente attesa di calendari ufficiali FUTURI impossibili da conoscere**, non cancella l'obbligo delle date reali per le edizioni storiche conosciute né l'audit generale.
**Mondo 1996/97 separato, DA DECIDERE:** preferenza dell'utente per l'applicazione dei veri cambiamenti storici di formato negli anni in cui sono avvenuti, previa ricostruzione dei regolamenti per paese/torneo; NON applicare ancora automaticamente, né inventare transizioni. Documentare e chiedere conferma sulle scelte storiche prima di implementarle.
**Stato implementazione:** la build preparata 00.13.225 applica questa regola solo alla Copa Bolivia per il 2027+, NON a tutte le coppe/leghe; le altre competizioni rimangono da collegare e validare. La regola approvata non costituisce prova di gameplay, CAR o collisioni globali.

## 20/11/2026 — REGOLA UNIVERSALE DELLE DATE REALI E DELL'AVVISO PRE-INIZIO
- **TUTTE** le competizioni dei mondi 9697 e 2627 devono avere date reali delle singole gare, con fonte verificabile, per OG/scheduler/viewer/save. Una stessa squadra NON puo' giocare due gare di competizioni diverse nello stesso giorno; verificare collisioni per ID club, giornata e rinvii prima di attivare gli eventi. In caso di collisione correggere le date confrontando gli aggiornamenti reali (anche il campionato), senza cambiare rose, regole, abbinamenti o risultati. Gli eventi precedenti il 1 luglio devono restare stato storico della stessa edizione, non essere spostati in avanti per comodita'.
- Quando una competizione e' nota ma **non ancora iniziata**, nella pagina OG classica, sotto classifiche/tabelloni, mostrare esattamente `Competition not started. Begins on DD/MM/YYYY (week WW)`. Usare la prima data CIVILE effettiva e la settimana coerente con il calendario, non il mese/fase OG arrotondato. Se la data non e' verificata mostrare `START DATE UNAVAILABLE` o non visualizzare una data, mai inventarla. Non confondere visibilita' dei partecipanti con l'inizio delle partite. Mantenere una sola autorita' OG.
- Questa e' una REGOLA APPROVATA; l'esistenza in AGENTS/GDD non significa che sia gia' cablata per TUTTE le competizioni. L'interfaccia nuova segue sempre il controllo font/layout approvato prima del codice.

## 20/11/2026 — REGOLA VINCOLANTE: VISIBILITÀ COMPETIZIONI SENZA ATTESA SORTEGGIO
Per entrambi i mondi, ogni competizione è consultabile APPENA i partecipanti OG sono noti; non aspettare la data del sorteggio per mostrare i dati già disponibili. Quando i partecipanti della nuova edizione non sono ancora noti, mostrare in sola lettura l'ULTIMA EDIZIONE PRECEDENTE OG DAVVERO SALVATA con anno chiaramente indicato. Mai copiare club/risultati precedenti nella nuova edizione né simulare all'apertura; se nessuno snapshot storico autentico esiste, mostrare indisponibilità. È decisione globale di design, NON tutta implementata: 00.13.220 rimuove solo l'attesa della Bolivia. Qualsiasi cambio di struttura resident/CAR richiede audit migrazioni.

### 20/11/2026 — Definizioni 4 gironi Bolivia + calendari ARG/PAR 2026; SOLO DATI, NON RUNTIME
Aggiunti `data_json/worlds/2627/competitions/copa_bolivia_2026_groups.json` (4 gruppi reali/16 ID unici) e `argentina_paraguay_2026_calendar.json` (Apertura/Clausura 2026 e stato al 1 luglio). Il cup boliviano OG rimane TYPE1 knockout, le leghe ARG/PAR restano ciascuna TYPE0 senza due edizioni residenti; spostare i soli begin/end produrrebbe gare false. Nessuna modifica a motore/CAR/saves né versione build. Fonti e checklist: docs/COPA_BOLIVIA_ARG_PAR_2026_DATA_AUDIT.md. Diagnostica Windows 217 ZIP non disponibile (solo screenshot). COMMIT DATI, NON FIX PLAYABLE.

### 20/11/2026 — 00.13.217: audit date sudamericane, guard viewer/5555; IMPLEMENTAZIONE PARZIALE
La nuova decisione approvata dal manager e': il 1 luglio non deve resettare una competizione OG ancora attiva; ogni torneo termina ed apre la successiva edizione alla propria data con gli stessi ID, fixture, punteggi e RNG, e SAVE/CONTINUE deve ricostituirli senza replay. Per la prima carriera 2026/27 fotografare i risultati REALI precedenti al 01/07/2026 per tutti i paesi; tra i titoli gia' noti Belgrano (43:21), Apertura argentino 24/05/2026. **NON ancora implementato nel runtime**: tabella dei 30 club, snapshot storici di coppe, ciclo per competizione e Libertadores 2027: ct_world_first_ready=0; 6 Q1/13 Q2 e sorteggi non approvati, CT round mensile non per-gara.
Intervento sicuro: Copa Bolivia in mondo 2627 `world_data.c` endMonth 32→87 (finale reale 25/11, nessun match preluglio; inizio OG 56 Agosto); viewer LEAGUE/CUP/TYPE2 firma l'intero contesto OG prima/dopo e riporta `[og-viewer-check]`; guard di memoria in `DIY_ValidateLeagueScratchBuffer` per range 5555 e delta positivi/negativi, **NON fix della causa 5555**. Manifest `data_json/worlds/2627/competitions/conmebol_2026_cutoff_audit.json` contiene 10 paesi e 11 coppe come **catalogo audit NON CONSUMATO DA OG**, tutte le altre coppe con primi incontri prima di luglio richiedono fonte fixture completa e veri stati residenti. Documentazione/QA: [COMPETITION_DATES_AND_JULY_2026_CUTOFF.md](COMPETITION_DATES_AND_JULY_2026_CUTOFF.md). Audit generale di date di TUTTE le competizioni nei due mondi, inclusi preliminari delle coppe nazionali e Europee, obbligatorio successivamente. Distinguere questo catalogo dalla partita e dal CAR: build/test Windows 217 **NON PROVATI**. Un solo commit, mai altro ramo.

## 20/11/2026 — 00.13.216: build fix F12 CAR (WINDOWS DA PROVARE)
La diagnostica Windows 00.13.215 del 20/11 (ZIP 20261119-235239, HEAD 21e0d698) fallisce PRIMA del gioco: `save_format_json.c:7212` usa `SaveCarZstd_IsFrame` durante il controllo sintattico senza `SWOS_CAR_ZSTD`, quindi manca il prototipo. Nel 216 la sonda F12 riconosce i primi 8 byte `SWOSZC3A` con `memcmp` indipendente dal macro, identica alla funzione Zstd; il loader continua a verificare frame, checksum e JSON. Non cambia CAR, retention, calendario, OG, SDL o script. Nessun test Windows 216 ancora disponibile; gli isolati non certificano la build. Un solo commit sul ramo dev/0.10.

## 19/11/2026 — 00.13.215: CAR unico Zstandard 3 e journal a crescita dinamica (WINDOWS NON PROVATO)
Decisione utente: SOLUZIONE A, un solo `AUTO9697.CAR`/`AUTO2627.CAR` portatile nella directory `save/` accanto all'EXE, compressione Zstandard 1.5.7 livello 3 compilata STATICAMENTE dai sorgenti BSD inclusi nel repo; documentazione e licenza in `docs/ZSTANDARD/`; nessuna DLL o installazione utente. Vecchi CAR schema5 JSON restano leggibili, senza conversione retroattiva silenziosa. Writer preflight OG/civile PRIMA compressione, poi singolo rename atomico su AUTO; file incompleti/corrotti rifiutati, CAR precedente intatto. Il witness OG ora usa hash/index e array che crescono sui match veri, senza tetto fisso 65.536/1.000.000 (restano limiti fisici int/size_t/formato). La retention parte dal PRIMO rollover e conserva una stagione dettagliata delle leghe OG chiuse con fonti qualificate; per edizioni completate eliminate archivia conteggio e hash reali per bloccare replay, campioni mantenuti dai dati OG esistenti e NON inventati. **Eccezione conservativa non ancora completata:** cup nazionali e continentali e fonti con accessi non verificati conservano i dettagli fino a prova closure/qualificazione, anche oltre un anno; Libertadores 2027 moderna ancora NON RESIDENT. F12 mostra dimensione CAR realmente compresso sul disco, dimensione JSON decompressa e numero di fixture DETTAGLIATE realmente nel CAR, non solo contatori RAM. Non definire PASS finche' non sono verificati Windows, 2028, SAVE→EXIT→CONTINUE, risultati e zero replay.

## 19/11/2026 — 00.13.214: scelta anno OG nei viewer delle coppe storiche
La diagnostica Windows 00.13.213 dimostra che 1997 CWC e UEFA Cup sono residenti a zero gare con campi 32/32 OG verificati, ma il viewer sceglieva il 1996 conservato; dopo l'assunzione passava la working copy, producendo `NOT RESIDENT compId=1/2 matches=2`. Un VIEW NON puo' selezionare per solo compId o per la prima identita' di record condiviso: usare mondo, paese, slot e **anno OG esatto**; normalizzare originale/copy e non usare il manager slot come permesso di lettura. Nessuna simulazione/viewer crea partite o modifica CAR. 2027 Libertadores non e' residente: messaggio esplicito `FIRST_EDITION_NOT_CONNECTED`, niente presunti PASS o fallback OG 1996 nel moderno. Q1/Q2 restano da concordare prima del sorteggio. Windows 214 NON PROVATA.

## 19/11/2026 — 00.13.213: libertadores 1996/97 viewer e perimetro OG (Windows NON PROVATO)
Un manager anche **non qualificato** puo' consultare il **vero** stato OG residente della Libertadores storica senza passare dal proprio slot 3. Soltanto un club nei 20 ID residenti puo' avere calendario giocabile; la lettura non deve avanzare, mutare, rigenerare, salvare o duplicare gare. Il fix 213 riguarda ESCLUSIVAMENTE il routing VIEW: prima del viewer usare puntatore originale o career copy e risolvere il canonico OG; non interpretare un test isolato di puntatori come prova runtime.
**PENDENTI NON AGGIRABILI:** integrazione partite del manager CONMEBOL qualificato col suo stesso residente (nessun doppio OG), 2026/27 Libertadores non residente nonostante compId=162 e manifest 47 club; sei nomi Q1 e tredici Q2 e accoppiamenti da approvare. Vietato inventare Q1/Q2, fixture, score, risultati, winner, CAR o dichiarare l'intero torneo implementato. Stato storico 1996 (20 OG, 5 gruppi) distinto dall'edizione 1997 Mod con detentore agli ottavi; mantenerne le regole OG separate. Nessuna build Windows dichiarabile dall'assistente.

## 19/11/2026 — PRECEDENZA DI PROGETTO: retention minima approvata
La decisione utente piu' recente **sostituisce** la precedente regola "stagione corrente + precedente". Nel CAR dettagli per le gare ordinarie **solo stagione corrente**; competizioni ancora aperte mantengono ogni dato di avanzamento OG, risultati/fixture necessari anche attraversando piu' annate; qualificazioni per club e nazionali mantengono i propri dati **soltanto finche' effettivamente necessari** alle competizioni dipendenti, senza timer minimo/massimo di due anni. In seguito rimane soltanto archivio sintetico per edizione (ID, vincitore, finalista eventuale, conclusione, numero gare, riferimenti anti-replay). La garbage collection deve fondarsi su dipendenze e conclusione OG autentica, non su semplice data del tick; deve preservare SAVE/CONTINUE e non generare un secondo simulatore. **Questa politica e' DESIGN approvato, NON ancora implementata nella 00.13.212.** Prima risolvere salvataggio OG fail-closed e verificare Windows; l'espansione 211 federazioni, fino a 8 divisioni, richiede nuovo dimensionamento dinamico, non alzare una sola costante.

## 19/11/2026 — 00.13.208: PUNTO 7 approvato; avviata Fase A/B importatore e validatore (NON motore OG completo)
**Decisione utente del 19/11, vincolante:** percorso A) convertire/catalogare definizioni di leghe, coppe, internazionali, nazionali, club e giocatori dei mondi 1996/97 e 2026/27 nei JSON esistenti; B) validare automaticamente ID, riferimenti, formati, quote, date, qualificazioni e limiti reali con errore preciso prima del runtime; C) collegare definizioni validate all'UNICO motore OG residente con gare/risultati/trofei veri, edizione successiva, SAVE/CONTINUE; D) estendere lo stesso metodo a club, giocatori, rose base ed extra. **Nessun editor grafico richiesto**: l'utente puo' descrivere in chat nuove competizioni, squadre o giocatori; l'assistente prepara JSON compatibili, sottopone a consenso regole nuove, modifica i dati e committa solo dopo «committa». Nessun nuovo simulatore di risultati, nessuna nuova toolchain da installare su Windows, CLASSIC/DIY/SEASON e salvataggi esistenti protetti. La sistemazione grafica del calendario e' rinviata. Questo inizio A/B di sola analisi dati e' indipendente dai gate gameplay B/C ancora aperti nel motore; **non** sblocca ancora nuove coppe giocabili.
**CODICE 208, primo passo reale A1/B1:** `sorgenti/scripts/validate_data.py` (facoltativo per chi mantiene gli asset, solo Python standard gia' usato dagli strumenti di import del repository; NON viene invocato dalla build Windows) inventaria i JSON classici sotto `data_json/competitions/`, le leghe overlay 2026/27 e l'edizione Libertadores provvisoria e verifica contatori, ID, riferimenti paese/competizione, seed, league-block, qualificazioni, date ISO, formati/partecipanti, percorsi team file, con severita' ERROR/WARNING/PENDING e output JSON opzionale. Con `--teams` analizza anche rose native e UUID, **non ancora** i riferimenti completi squad_extras. `sorgenti/tests/test_validate_data.py` aggiunge quattro test sintetici per casi validi, sentinel OG e dati errati. Formato JSON attuale riutilizzato, non migrato a nuovo schema incompatibile. E' un validatore **preflight offline** e non ancora un validator C a caricamento, né prova che una coppa sia RESIDENT o PLAYED. Per aggiungere una competizione veramente giocabile serviranno collegamenti OG verificati, ID/archivio/CAR sicuri e regole adottate.
**Scoperte verificate:** nel tree repo `data_json/competitions/` contiene 10 file classici, `data_json/teams/` 76 file, `data_json/worlds/2627/teams/` 101 file e `.../competitions/leagues.json` 91 definizioni (Italia 14 divisioni, contro alcuni limiti vecchi nelle guide). `libertadores_2027_first.json` ha 47 entranti (28 diretti e 19 preliminari) ma dichiara `DATA_ONLY_NOT_YET_LINKED_TO_OG_MATCHES`: NON giocabile per il solo JSON. La lista classica `countries.json` ripete identico BRAZIL/46, il nuovo validatore segnala WARNING, non confonde con due squadre diverse; le sentinel OG `-2` e `252` in `qualifying.comp_cup` non sono ID torneo. Nel seed bucket `bucket22Full` l'ulteriore slot e' il terminatore OG. File grandi di team/player non tutti letti dal connettore: controllo profondo reale **NON PROVATO**. Test Python e Windows 208 **NON ESEGUITI**. Dettaglio: [PUNTO7_IMPORT_VALIDATOR_A1_0013208_20261119.md](PUNTO7_IMPORT_VALIDATOR_A1_0013208_20261119.md).

## 19/11/2026 — 00.13.207: luglio manuale, intercettazione PLAY MATCH + NEXT MATCH
La **diagnostica Windows reale 00.13.206** `DIAGNOSTICA_SESSIONE_1(20261119-171631).zip` conferma compilazione, nessun PANIC/crash e clock post-partita operativo. Dopo FINISH manuale 2026/27: `[og-calendar] BOUND ... next-phase=56` (1 agosto), `[career-july-menu] eligible=1` al 1 luglio ma **zero** `[career-july-forward]`: il pulsante principale PLAY MATCH e' entry **7**, non la entry **10** su cui 205/206 installavano FORWARD. [CODICE 207] Entrambi i veri handler `PlayMatchMenu` (7) e `NextMatch` (10) intercettano prima di OG: quando world.today e' luglio e il prossimo incontro OG del club e' in agosto o oltre, avanzano 7 giorni col **solo coordinator OG**, salvano e non giocano la prima partita. Entrambi i pulsanti mostrano FORWARD TIME, usando font/hitbox OG; fino all'ultima fase sicura (28 luglio se la prima gara e' 1 agosto), poi il prossimo clic gioca **solo** la vera gara OG. Test puro aggiunto. **Eccezione importante:** nella diagnostica 1996/97, club `2c1b` (country44/div3) ha un vero incontro OG gia' il **4 luglio 1997** (phase49), e 66 risultati residenti al 1 luglio: non si possono posticipare queste gare fino ad agosto senza alterare il calendario OG; scelta da discutere separatamente. Nessuna amichevole ancora implementata. Nessuna modifica a match, CAR, CLASSIC/DIY/SEASON, automatico o disoccupato. **Windows 207 NON PROVATO**. [Report](OG_PRESEASON_MANUAL_ENTRY7_0013207_20261119.md).

## 19/11/2026 — 00.13.206, CAREER luglio e PANIC 6006 a fine stagione
Diagnostica **Windows 00.13.205** `DIAGNOSTICA_SESSIONE_1(20261119-161815).zip`: compilazione riuscita, ma nessuna chiamata `[career-july-forward]`; il test del target data era solo isolato. Il club dopo FINISH rimaneva su 1 luglio senza comando funzionante. La gara reale 2026/27 a phase80 veniva acquisita OG prima del crash; dopo `[world-cupd] round 3 decided` il motore segnalava `PANIC 0x6006` (FindTeamInDIY: club non trovato nella competizione). [CODICE 206] Riprova la preparazione dell'entry10 FORWARD TIME nell'effettivo `Expanded_UI_Prepare` (oltre a ContestMenuOnReturn), elimina la dipendenza da flag nativi di stagione potenzialmente obsoleti, mantiene il controllo OG residente, data/fixture reale e la barriera anti-replay; log `[career-july-menu]` diagnostica eleggibilita'. In `Career_FillCurrentSeasonInfo`, un nome storico della coppa NON basta a indicare che il club vi partecipa: verificare puntatore e contesto vivo prima di cercare la squadra; assenza in coppa opzionale -> stato originale `-104`, mai risultato inventato. `FindTeamInDIY` conserva PANIC per lega principale e contesti non riconosciuti, restituisce `0xffff` esclusivamente per assenza legittima in coppa opzionale CAREER e aggiunge log prima di altri PANIC. Nessuna modifica a score OG, CAR, CLASSIC/DIY/SEASON, mondo disoccupato o asset; restano irrisolti i mismatch del keycache gara `2c31` contro `2c30`. **Windows 206 e gameplay NON PROVATI**. Report: [OG_CAREER_JULY_PANIC_0013206_20261119.md](OG_CAREER_JULY_PANIC_0013206_20261119.md).

## 19/11/2026 — 00.13.205, FORWARD CAREER luglio + data post-partita
Dalla vera ZIP Windows 204: UNEMPLOYED AVANZA fino ad agosto in entrambi i mondi, mentre dopo FINISH SEASON il club in 2027 ha WORLD TODAY=1 luglio, NEXT OG=1 agosto, nessun comando weekly e la partita manager OG non aggiorna il clock civil. [CODICE] In CAREER impiegato il pulsante esistente NEXT MATCH diventa temporaneamente FORWARD TIME per il precampionato (stessa UI/callback ripristinato fuori scope); utilizza solo coordinator OG, non rigioca la prima partita. Ogni tick e' limitato alla **fase OG precedente** (28 luglio prima della phase56 del 1 agosto), non al 31 per evitare simulazione precoce con CEIL native phase. Un risultato vero della lega manager fa avanzare WORLD TODAY al giorno OG tramite stesso coordinator prima di autosave, solo se resident/local sono coerenti; nessuna variazione per UNEMPLOYED, DEBUG, CLASSIC/SEASON/DIY, salvataggi preesistenti. [TEST] nuovi casi C puri, **Windows 205 NON PROVATO**. Dettagli: [OG_CAREER_JULY_FORWARD_CLOCK_0013205_20261119.md](OG_CAREER_JULY_FORWARD_CLOCK_0013205_20261119.md).

## 19/11/2026 — 00.13.204: decisione July1 / Scotland stage0 e ingaggio
[APPROVATO] Normale FORWARD TIME da disoccupato, quando i 7 giorni attraversano la fine stagione, si ferma **esattamente al 1 luglio**; la pressione successiva avanza dalla nuova data. Saltare 30 giugno -> 7 luglio non e' piu' ammesso. [CODICE PREPARATO] Scozia 2627 div1-3: 4 cicli con `SP_TAB[7]=0` NON sono split reali; div0 con `SP_TAB[7]=1` resta protetta. Il residente contiene 15 risultati prima dell'ingaggio di Stirling Albion: l'handoff deve validarli per contatori/ID/classifiche senza ripeterli. Preflight anticipato prima di salary/autosave e della prima mutazione in PLAY MATCH. [QA] Windows 204 NON PROVATO; non concludere che il crash e' risolto finche' il test dell'utente non lo conferma. Report [OG_SCOTLAND_JULY1_HIRE_0013204_20261119.md](OG_SCOTLAND_JULY1_HIRE_0013204_20261119.md).

## 19/11/2026 — 00.13.203: GCC Windows 202, macro CAREER locale
La ZIP Windows `DIAGNOSTICA_SESSIONE_1(20261119-144856).zip` prova che la 00.13.202 NON compila: `expanded_economy.c:864:26` `GAME_TYPE_CAREER undeclared`, gioco non avviato. `GAME_TYPE_CAREER` vale `4` ma è definito **solo** dentro `menus.c:2661`, non in `game_types.h`. La 00.13.203 usa `_gameType==4u` in questa unica nuova funzione di presentazione della data, come altri sorgenti OG, lasciando WORLD TODAY, OG next fixture, Business, salvataggi e gameplay inalterati. Gli isolati Windows della 202 (19 PASS) non compilano il gioco completo, quindi non ne provavano il build. **Windows build 203 NON PROVATA** finché l'utente non ricompila. Report: [OG_OFFER_WORLD_DATE_0013202_20261119.md](OG_OFFER_WORLD_DATE_0013202_20261119.md).

## 19/11/2026 — 00.13.202: la data dopo l'offerta e' WORLD TODAY, non il prossimo fixture OG
Screenshot 00.13.201 `image(20261119-142841).png`: con EDINBURGH CITY e 2026/27 il banner dell'interfaccia CAREER mostra `SAT 01 AUG 2026`. Audit sorgente: `expanded_economy.c::Expanded_UI_After` e le 3 date Business formattavano `Expanded_Career_NativeCalendarKey()`, che indica **il prossimo incontro OG** (key56=Aug1), mentre il mondo ha una data civile separata `ExpandedWorldLive_CalendarState().today`. La 00.13.202 corregge SOLO la sorgente della data disegnata in CAREER SDL: conserva 30 giugno se si firma quel giorno, mostra 1 luglio dopo il vero FINISH, mantiene invariati le fixture OG, il world clock, SAVE e CLASSIC/DIY/SEASON. Date WORLD mancanti sono indisponibili (no fallback inventato). Test C isolato previsto e Windows da verificare. **Non crea partite amichevoli/preseason in luglio; non risolve il bug separato della data civile dopo una gara manager.** Vedere [report](OG_OFFER_WORLD_DATE_0013202_20261119.md).

## 19/11/2026 — 00.13.201: OG residente, manager cache e identità vere
La diagnostica Windows 200 (ZIP ore 13:38) prova 9697 Scozia div3 1997 `selected9/required10` con manager East Stirling `0x2112` tra gli iscritti OG ma assente da `_selectedTeams`. `findTeamInCache` risolve il manager a `_careerTeam`, quindi il selettore non lo copia; `InitTeams` salva `-1`, che nel mondo residente OG non è permesso. La 201 aggiunge solo ai nuovi resident CAREER un controllo degli ID autentici e una copia/riconciliazione esatta di questo record con offset nonnegativo, anche per coppe senza bye, senza toccare selettori generali, promo o OG gameplay. In una coppa del manager vale ancora il blocco anti-doppia partita `MANAGER_SLOT_UNBOUND`. **Windows 201 NON PROVATO**; source test puramente isolato. El Salvador 2627 potenzialmente diverso, da verificare. [Report](OG_RESIDENT_CACHED_MANAGER_IDENTITY_0013201_20261119.md).

## 19/11/2026 — 00.13.200: recovery dopo fallimento passaggio manuale, scheduler solo SHADOW
Fonte obbligatoria per ripresa: Issue #1 `https://github.com/Giuzy/SWOS-Expanded/issues/1`, **prima del primo accesso** al repo; sovrascrivere checkpoint DOPO OGNI PASSAGGIO, prima di quello successivo, con percorso e motivo/ultimo completato/prossimo non eseguito. Le ZIP Windows 12:56 hanno BUILD **198** e **199** (non 200); compilazione GCC senza warning in quelle due build, nessuna prova Windows200. 199: El Salvador (country51) div4 new-year 2027 OG seleziona 9/10 club, cup17/18; stato resident2027/civil2026 dopo rollover partial e 162932 retry local fixture. 200 mette latch su preflight+year fault e guard su league/cup per non ripetere gare e salvare il vecchio CAR; aggiunge diagnostics dei veri ID squadra. **NON corregge i partecipanti o il cambio anno**: vietato inventare un club e vietato alterare le promozioni Mod senza approvazione. Scheduler collegato SOLO come stack SHADOW peek milestone 30/06: non fare `Complete`, fixture dispatch, nuova data/CAR finché non esistono claim persistenti e commit OG/RNG/CAR atomico. Zero regressioni CLASSIC/DIY/SEASON; verifiche Windows utente pendenti. [Audit completo](COMPETITION_ENGINE_SCHEDULER_SHADOW_AND_YEAREND_AUDIT_0013200_20261119.md).

## Regola operativa deposito ripresa — 19/11/2026 (obbligatoria)
**PRIMA azione dopo una nuova richiesta SWOS, una interruzione o una nuova chat:** leggere integralmente la Issue #1 `https://github.com/Giuzy/SWOS-Expanded/issues/1`, recuperare *ultimo passaggio COMPLETATO, percorso tecnico scelto e perche', dati/prove e passo seguente NON eseguito*, poi verificare HEAD/versione. Dopo OGNI singolo passaggio (lettura, verifica, scelta, test o edit), **PRIMA** di quello successivo **sovrascrivere la stessa Issue** con checkpoint autosufficiente e cumulativo, senza perdere prove e scelte; non attendere commit o fine sessione. La Issue e' indipendente da commit. Pubblicare codice/guide nel ramo solo dopo «committa»; un solo commit finale. La motivazione riproducibile va documentata, non la catena interna privata di pensiero. [Audit corrente](COMPETITION_ENGINE_A2_SOURCE_AUDIT_0013199_20261119.md).

## 19/11/2026 — 00.13.198: COORDINATORE LIVE A1, NOTA VINCOLANTE
Prima integrazione reale, NON completamento del Competition Engine: un entry point SDL comune per START, FORWARD TIME da disoccupato, preflight BIND, validazione LOAD. Solo OG esegue/calcola incontri, Expanded aggiorna il cursore civile senza LWE e senza secondo esecutore. Un fault post-mutazione richiede ricaricare il CAR valido precedente; non dichiarare rollback atomico. FINISH SEASON manuale e percorso FORWARD del manager restano legacy INVARIATI per la richiesta attuale; owner per singola competizione, claim CPU-manager e transazioni CAR da implementare prima di attivare scheduler. Debug bottoni 4/5: renderer testo/sfondo dopo altri overlay SWOS, nessun asset. Report [COMPETITION_ENGINE_STEP1_COORDINATOR_0013198_20261119.md](COMPETITION_ENGINE_STEP1_COORDINATOR_0013198_20261119.md). Non interpretare la diagnostica 196 come compilazione 198.

## 19/11/2026 — PRECEDENZA ESPRESSA DEL PIANO COMPETITION ENGINE (18/11)
La decisione approvata il 18/11/2026 PREVALE sulle sole vecchie prescrizioni OG-only del 14-15/11 incompatibili: Expanded coordinera' l'unica data civile, il calendario mondiale, edizioni, qualificazioni, risultati persistenti e CAR della carriera; OG resta l'unico esecutore calcistico (50 Hz originale) e la sorgente degli algoritmi verificati di partite/competizioni. Nessuna seconda simulazione LWE, nessuna gara ripetuta, nessun reset del mondo all'ingaggio/esonero, CLASSIC/DIY/SEASON invariati. La decisione e' un OBIETTIVO APPROVATO: scheduler e claim non sono ancora dispatcher live. La correzione 00.13.197 non conclude il motore. Vedere COMPETITION_ENGINE_ARCHITECTURE_TODO_20261118.md e OG_DEBUG_AND_TYPE2_FIX_0013197_20261119.md.

## 19/11/2026 — 00.13.195: audit diagnostiche Windows 194 e fix manuale OG
- Sessioni Windows 00.13.194 `DIAGNOSTICA_SESSIONE_1(20261119-094420).zip` e `DIAGNOSTICA_SESSIONE_2(20261119-094421).zip`: build Windows completata senza warning nei log. Dopo FINISH SEASON 2627 `INITIAL_TEAM_SELECTION_MISMATCH country=8 div=3 year=2027 selected=23 required=24`, nuovo CAR bloccato; dopo FINISH SEASON 9697 `FILE_BALANCE_MISMATCH file-country=14 offset=294 count=44 sum-mod256=255` e `PANIC 0x5555`. Presente conflitto `[cup-conflict] WINNER` durante replay del vecchio callback. AUTO UNEMPLOYED apre 2027 (134 leghe) e 1997 (88 leghe), ma qualificazioni parziali.
- Origine individuata: il vecchio callback manuale OG rieseguiva campionati, coppe domestiche e tornei tipo 2 già elaborati dalla stessa autorità OG residente. Nuovo uso read-only della copia temporanea residente nelle routine legacy di fine stagione: mantiene ordinamento, qualificazioni, promozioni/retrocessioni OG senza nuovo esecutore gare. Se il residente non esiste, l'originale callback rimane sorgente OG unica per quella competizione; se un residente esiste ma è invalido, blocca il rollover senza inventare dati.
- `DIY_ValidateLeagueScratchBuffer` ora registra i delta reali dallo START del buffer (non dal cursore post-scan); il solo CAREER OG-only usa latch errore nonfatale per `0x5555` e blocca il nuovo CAR prima di InitializeNewSeason. Non equiparare assenza di crash a transizione riuscita.
- Libertadores 1996: nel log AUTO OG sono presenti 20 qualificati CONMEBOL, CWC invece incompleta. Consentita cattura **parziale e indipendente** dei 20 ID OG e attivazione della sola nuova Copa residente: altre coppe senza lista completa restano PENDING. Libertadores 2026/27 rimane `NOT_RESIDENT`: `ct_world_prepare_first` non dispone ancora dell'input verificato e non si costruisce alcun campo fittizio.
- DEBUG 30 JUNE: corregge solo la whitelist del menu UNEMPLOYED (`ord==14u`); il pulsante/callback OG già esistente raggiunge il 30 giugno del mondo attivo senza cambiare stagione. Calcoli OG eseguiti, nessun salto fittizio al 1 luglio. La 00.13.195 è da testare su Windows dall'utente, così come warning, menu, prestazioni, SAVE/CONTINUE e dati delle coppe.
- Analisi, limiti e checklist: [OG_ROLLOVER_DIAGNOSTIC_FIX_0013195_20261119.md](OG_ROLLOVER_DIAGNOSTIC_FIX_0013195_20261119.md). Unico commit autorizzato sul ramo dev/0.10; main/release/backup invariati.

## 19/11/2026 — 00.13.195: DEBUG 30 JUNE UNEMPLOYED
- Base dev/0.10 HEAD 5935d6c2aa09efe6ea295e099d5307107493fee3 (00.13.194). Pulsante `DEBUG 30 JUNE` gia' esistente nella schermata UNEMPLOYED come `_menuEntries[14]`, accanto a `DEBUG +1 MONTH` e raggiungibile con mouse/tastiera/joypad; il callback `Expanded_UnemployedDebugJune30` e' presente.
- Errore reale individuato: `menu_system.c::MenuProc` permette le entry 7,9,11,12,13 e 10 condizionale, ma NON la 14: il guard `[unemployed-menu] BLOCK hidden/club action` respinge il pulsante. Patch minima: aggiunta `ord==14u` alla whitelist mantenendo invisibilita', disabled e callback null come guardie di sicurezza.
- Nessun nuovo simulatore: `ExpandedDebug_June30BeforeSeason` calcola la data della stagione in corso (1996/97 -> 30 giugno 1997, 2026/27 -> 30 giugno 2027); `Expanded_UnemployedWait` invoca `Expanded_OGWorld_AdvanceToDate(target)` PRIMA di `ExpandedWorldLive_AdvanceToDate`, aggiorna le offerte e resta al 30 giugno. Il 1 luglio avanza con FORWARD TIME ordinario. Fallimenti OG non vengono contati come successi.
- La prova isolata esistente `sorgenti/tests/test_expanded_debug_june30.c` copre le due epoche ma NON attesta menu/partite Windows. Non toccare CLASSIC/DIY/SEASON, stagione del manager, formato CAR o altri pulsanti. Versione aggiornata a `00.13.195` nel commit unico autorizzato.
- Stato: pulsante e callback esistenti SI, bug del dispatch verificato staticamente SI, patch preparata SI; build Windows, zero warning, click A/B/mouse, avanzamento OG con coppe/leghe e SAVE/CONTINUE NON PROVATI.

## 19/11/2026 — 00.13.194: FINISH SEASON OG invisibile senza saltare ordinamento e mobilita'
- Diagnostica Windows 00.13.193 (sessione 1 20261119-091533): UNEMPLOYED apre il secondo anno OG con 134 leghe (2627) e 88 (9697) senza invalidi; 9697 ha ancora qualificazioni `FIELD_INCOMPLETE`, SAVE/CONTINUE non provato. EMPLOYED fallisce 2627 per `country=44 division=3`, 12 squadre richieste/11 reali, e 9697 per `PANIC 0x5555` su bilancio dei trasferimenti di divisione.
- Correzione alla 00.13.193: `ShowDiyLeaguesTournamentMenu` ordina la classifica con `DIY_SortLeagueTableByStandings` PRIMA che qualificazioni/promozioni leggano gli indici. Non saltare MAI quel callback OG per nascondere la UI: ripristinato in `career_season.c` e `career_status.c`.
- Soltanto Expanded CAREER OG-only: `MenuProc` elimina l'attesa artificiale di 50 frame per OG menu e SDL non presenta i frame intermedi quando `_seasonUpdateInProgress` (continua eventi/input/audio); CLASSIC/DIY/SEASON e altre piattaforme invariate. Log finale `[og-season-ui] HEADLESS_NATIVE_MENUS`.
- Preflight OG nuovo residente: `INITIAL_TEAM_SELECTION_MISMATCH` rileva 12/11 immediatamente senza inserire squadre inventate o salvare stati incoerenti; `FILE_BALANCE_MISMATCH` aggiunge dettagli del bilancio 0x5555 senza nascondere il PANIC.
- Diagnosi tecnica e checklist in [OG_SEASON_MANUAL_HEADLESS_FIX_0013194_20261119.md](OG_SEASON_MANUAL_HEADLESS_FIX_0013194_20261119.md). Nuovo codice **NON compilato e NON testato su Windows** da assistente, nessuna garanzia che il cambio manuale sia gia' riparato. L'utente compila/testa dopo GitKraken; warning nuovi da correggere subito. Unico commit autorizzato.

## 19/11/2026 — 00.13.193: diagnostica prestazioni e stagione OG
- Solo un commit su dev/0.10 dopo «committa»; build Windows e gameplay sono testati dall'utente e non devono essere dichiarati PASS in assenza di log. Versione progressiva del build_info.h aggiornata.
- F12+9 e' una pagina separata di tempi per il mondo attivo; F12+0 ripristina la salute OG. I valori (ultimo/media/picco/totale, numero chiamate) non entrano nei CAR. Sono strumentati OG world, campionati, coppe, internazionali, overhead, FORWARD disoccupato, passaggio stagione manuale, vecchio callback fine stagione, SAVE, LOAD, boot e render SDL. Dopo switch del mondo non mescolare i campioni.
- Il vecchio callback OG di fine stagione resta necessario per qualificazioni e mobilita'; elimina solo le schermate read-only ripetitive durante il callback, NON salta calcoli ne' crea seconde partite. Segnalare esplicitamente che il costo residuo puo' ancora essere alto.
- Il rollover confronta la data civile reale con 1 luglio dell'anno previsto prima di autorizzare il nuovo salvataggio. Il residente dei campionati non ammette indice -1 del manager; validare gli offset prima dell'uso e bloccare senza risimulare. Non dichiarare risolto il crash 96/97 senza diagnostica.
- Test statici: schema/struttura delle modifiche controllati, compilazione completa e warning Windows NON PROVATI. Stato dettagliato: docs/F12_PERFORMANCE_FORWARD_0013193_20261119.md.

## 18/11/2026 — 00.13.186: cambio stagione OG, coppe e diagnostica
- Corretto il percorso di cambio manuale per avanzare prima il mondo OG residente al 1 luglio, poi il calendario ausiliario, poi ricollegare il club; se il mondo o il bind non sono pronti l'autosave della nuova stagione e' **bloccato** (CAR precedente intatto). Interrotto il ri-tentativo senza fine quando il preflight bridge OG rifiuta una partita; guard sempre attivo, nessuna partita fittizia.
- Riuso degli ID qualificati dalle ORIGINALI funzioni `career_eurocups.c` / `uefa_access.c` per preparare nuove edizioni residenti UEFA 2026/27 e tre coppe storiche 1996/97, piu' Copa storica quando presente il campo nativo valido. No seed dell'anno precedente replicati; continuazione da CAR usa contesti OG gia' residenti.
- Rollover nativo del contatore stagioni delle coppe nazionali anche da disoccupato senza doppia simulazione. Audit `[og-cup-entry]` confronta seed primo anno e posti seconda stagione con gli ID del residente; senza piazzamenti OG finali completi segnala FAIL e non inventa qualificati. Il test `OG_FIRST_EDITION_CUP_SEED` distingue origine nativa o manuale approvata; dati manuali NON installati.
- Profiling `[og-forward-perf]` e `[og-forward-cycle]`: campionati, coppe, internazionali, overhead, millisecondi, confronto 1996/97 vs 2026/27. Parser Windows aggiornato. Mismatch del calendario e marcatori preservati come errori non risolti.
- **LIMITI APERTI**: qualificazioni senza manager NON connesse al nuovo campo UEFA; il test le segnala FAIL invece di PASS/NOT_TESTED neutro. CAF/AFC/OFC/CONCACAF/Libertadores moderna, accessi completi delle coppe nazionali, storico dei residenti precedenti e modalita' manuale richiedono lavoro ulteriore; nessun calendario/risultato inventato. Report completo: [OG_SEASON_ROLLOVER_0013186_20261118.md](OG_SEASON_ROLLOVER_0013186_20261118.md).
- Verifica isolata GCC/Clang del confronto ID e provenance prima edizione PASS; **build Windows, warning globali e gameplay 00.13.186 NON PROVATI**. Unico commit dopo autorizzazione `committa`; versione 00.13.186.

## 18/11/2026 — 00.13.185: test ID qualificati -> competizione OG dell'anno dopo
- Aggiunto audit read-only del pool qualificati prodotto dai percorsi OG di fine stagione, confrontato **per world, edition year, competition slot e club ID** con le squadre delle VERE edizioni residenti dopo luglio. Non bastano i conteggi. Log `[og-qual-next] SOURCE/ROUTE/SUMMARY` e `[og-qual-next-source]` per ciascun ID nativo, diagnostica aggregata `OG_NEXT_SEASON_QUALIFICATION`.
- Nuova edizione assente = **FAIL_NEW_EDITION_MISSING**; elenco diverso/duplicato = FAIL, mai PASS fittizio. Unoccupato senza sorgente, domestic cup access, altre confederazioni CT_ED e qualificazione 2026/27 Libertadores **NON TESTATI** da questo percorso. L'engine di qualificazione futura NON e' stato implementato; non simulare o inserire squadre per far passare l'audit.
- Test puro del matcher e parser sintetico nel runner; nessuna modifica a motore OG, RNG, calendario, CLASSIC o CAR. Full build e gameplay Windows **NON PROVATI**. Report: [OG_QUALIFICATION_ROLLOVER_TEST_0013185_20261118.md](OG_QUALIFICATION_ROLLOVER_TEST_0013185_20261118.md). Versione progressiva 00.13.185 dopo commit 184.

## 18/11/2026 — 00.13.184: correzione diagnostica OG, mismatch calendario e conteggio revisioni
- Verificati i CAR e i log delle tre sessioni Windows **0.13.104** (20261118-140823/140825). Il bridge locale OG -> residente ha 91 commit di stato (66 2627 + 25 9697), zero WRITE_BLOCKED, checksum journal validi. Questo e' un fatto dei log ricevuti; la 00.13.184 non e' ancora compilata/testata su Windows.
- Corretto il falso **CPU_FIXTURE_NOT_OBSERVED**: per una lega con manager, sia gare manager (source=2) sia CPU locali (source=3) sono prodotte dal medesimo OG e testimoniate nel CAR prima del commit del residente. Non cercare una seconda simulazione CPU residente, che sarebbe vietata. Log locali ora distinguono attesa commit snapshot da collisione con una gara OG residente della **stessa fase**. Un incontro residente della stessa coppia ma **di fase differente** e' un ciclo storico, non **SCORE_OR_PHASE_DIVERGENCE**: due stagioni/cicli non vengono fusi per sola coppia casa/trasferta. Collisione reale di stessa fase blocca il nuovo CAR.
- **CACHE_UNVERIFIED** non era solo una falsa segnalazione: nei 10 casi osservati il keycache presenta una gara DIVERSA nella fase della partita nativa; esempio 2627 fase 76: OG 0857-0847, calendario 0847-0844. Le classifiche native coincidono (row-diff=0) ma il calendario e' davvero disallineato. Nuova classificazione read-only **CALENDAR_PAIR_MISMATCH**, con ID, score e flag della riga calendario. Non sovrascrivere risultati, rose, RNG o fixture per occultare il problema. La riconciliazione calendario OG/keycache resta **APERTA**, non PASS.
- L'analizzatore Windows `audit_windows_runtime_log.c` distingue ora `OG_MANAGER_LOCAL_OG_BRIDGE` (witness/commit/blocchi) e `OG_MANAGER_CALENDAR_PAIR_IDENTITY=FAIL_REAL_PAIR_MISMATCH` (coppie realmente discordanti). Anche il test statico del parser contiene un caso di mismatch, senza spacciarlo per gameplay verificato.
- Nuovi test isolati `ExpandedOGCache_FindPhaseSlot` e `ExpandedOGFixtureHistory_Check`: calendario con fase valida ma coppia errata, coppia altrove solo scheduled, molteplici entry/corruzione, stessa coppia in fase storica diversa, collisione della stessa fase e score diverso. Contratti oggetto di test C senza nuova simulazione. Windows 00.13.184 **NON PROVATO**.
- **Regola numerazione richiesta dall'utente, prevalente sulla vecchia regola del 10/11**: il primo commit del ciclo 0.13 e' 0.13.01; nel ramo dev/0.10, fino alla 0.13.104 inclusa, 207 commit totali meno 24 merge = **183 revisioni effettive**. Conta anche commit di soli fix, docs e asset, senza contare merge tecnici. Il successivo commit diventa **00.13.184**; incrementare il terzo campo di 1 per ogni successiva revisione non-merge autorizzata, mantenendo la forma `00.13.xxx` finche' l'utente non modifica milestone. Questa regola sostituisce la precedente eccezione per commit solo documentali.
- Ripulita la copia duplicata `docs/reference/COMPETITION_FORMAT_AUDIT.csv` (stesso blob SHA di `docs/COMPETITION_FORMAT_AUDIT.csv`); mantenuta quest'ultima come copia canonica. Il CSV generato nei pacchetti diagnostici non viene modificato.
- Rapporto e procedura: [OG_DIAGNOSTIC_CORRECTIONS_0013184_20261118.md](OG_DIAGNOSTIC_CORRECTIONS_0013184_20261118.md). Nessun merge, checkout o modifica al ramo main/release.

## 18/11/2026 — 0.13.104: bridge campionato OG locale -> residente (manager + CPU)
- Sui campionati OG ordinari con manager attivo, il percorso **locale OG** esegue le gare; `Expanded_OGWorld_AdvanceLeague` NON lancia un secondo esecutore CPU residente per la stessa lega. Dopo ogni gara reale (manager **o** CPU locale), `Expanded_OGWorld_PreflightLocalLeague` verifica prima della partita che contatori e classifica per ID club coincidano con il residente; il callback finale registra l'esatto risultato OG nel journal e `Expanded_OGWorld_CommitLocalLeagueFixture` controlla l'incremento atteso di una gara, punti/reti/WDL, rimappa tutti gli `teamSelOffset` (manager `-1` -> indice nativo), quindi copia competizione, squadre e RNG nel residente **senza seconda simulazione**.
- Il journal portatile CAR resta `format=1`, `record_size=20`: nuovo `source=3` indica gare CPU eseguite nel **contesto locale OG** (1=residente, 2=manager). Per le leghe ordinarie si preserva il `DIYTournamentState` residente, poiche' OG non aggiorna il suo buffer torneo in assenza di gironi; non si importano indici di stage locali senza mappa validata. A carico del CAR esistente, nessun nuovo file di salvataggio.
- **Fail closed:** ID/riga duplicata o mancante, baseline locale diversa, salto di contatore, score/delta di classifica incoerenti, journal duplicato oppure residente mancante -> `[og-manager-bridge] WRITE_BLOCKED` e il CAR precedente e' preservato; nessun match riprodotto. Dopo esonero il residente torna al proprio percorso CPU.
- **Limiti da non nascondere:** bridge attivo SOLO campionati `hasGroups==0` e `s_expOGBound` valido; tornei a gironi/split, coppe domestiche/internazionali, transfer/post-match extra, caricamento di vecchi CAR **gia' divergenti**, e competizioni che superano il 1 luglio richiedono ancora integrazione specifica. `CPU_FIXTURE_NOT_OBSERVED` puo' comparire prima del commit snapshot locale; il segnale di applicazione e' `[og-manager-bridge] COMMITTED`. **Fase 3 NON conclusa**. Verifiche Windows della 0.13.104: **NON PROVATE**. Test C del remap/score isolato: disponibili nel nuovo runner; NON sono gameplay.
- Rapporto e protocollo: [OG_LOCAL_RESIDENT_BRIDGE_013104_20261118.md](OG_LOCAL_RESIDENT_BRIDGE_013104_20261118.md).

## 18/11/2026 — 0.13.103: audit CPU/manager, orientamento cache e test storico
- Corrette **cinque** inizializzazioni ad array annidati in `sorgenti/tests/test_expanded_competition_history.c` (`{{0}}` -> `{0}`) che avevano prodotto cinque errori `-Werror=missing-braces` in `run_test_expanded_competition_history` nelle tre sessioni Windows 0.13.102. Il test sulla 0.13.103 e' **NON PROVATO**.
- Il confronto read-only `[og-manager-fixture]` aggiunge `reverse-cache`: cerca anche la coppia orientata inversamente, con reti speculari; `PLAYED_MIRRORED` indica solo un candidato e **NON** risolve il binding, non autorizza merge, non cambia lo `status`. Nessuna simulazione, classifica, RNG, journal, CAR o gameplay modificati da questo audit.
- Anomalie 0.13.102: `CPU_FIXTURE_NOT_OBSERVED` 5/5 partite del manager significa assenza di controparte osservata nel journal residente a quell'istante, NON prova di doppia gara; `CACHE_UNVERIFIED` 4/5 significa che la ricerca della chiave orientata/fase nel cache non conferma il risultato, NON prova di CAR corrotto; `TABLES_DIVERGED` 1/5 (2627, country=51 div=4, 3-1; residente=15 incontri, locale=24; 10 righe differenti, 0 unmapped) indica stati OG non riconciliati. Il dato nativo non deve essere sovrascritto o risimulato sulla base del solo punteggio. Caso 9697: dopo gara 1-0, residente=45 e locale=48. La correzione manager/residente **NON E' ANCORA RISOLTA**, single-writer fase 3 aperto.
- Analisi e protocollo: [OG_MANAGER_DIVERGENCE_AND_HISTORY_TEST_013103_20261118.md](OG_MANAGER_DIVERGENCE_AND_HISTORY_TEST_013103_20261118.md). Full build Windows, warnings e runtime della nuova 0.13.103 **NON PROVATI**; l'utente compila/testa. Non eliminare CAR, non reimportare score e non avanzare automaticamente residenti divergenti.

## 18/11/2026 — 0.13.102: witness fixture OG, confronto FORWARD e cache CAR 8192
- Implementato **witness osservazionale** di risultati gia' prodotti da OG per campionato/coppa residenti e partite manageriali. Solo CAREER SDL; nessun callback di nuova simulazione o variazione di gameplay/RNG. Identita' con mondo implicito, competizione, anno-edizione, fase, progressivo nativo, coppia orientata, variante; indice hash per intercettare doppia scrittura o score divergente, bloccando un NUOVO CAR se rilevati. Questo NON e' ancora un single-writer preventivo tra manager e CPU.
- Ogni `Expanded_OGWorld_AdvanceToDate` (compresi FORWARD da disoccupato e salti debug) confronta prima/dopo contatori nativi e numero di partite osservate. Il confronto per coppia manager/residente e' registrato separatamente: esito esplicito anche quando ID o score non sono riconciliati.
- CAR: cache OG raw e fixup endian `1870 -> 8192` con validazione occupazione prima del write, preflight read-only di vecchi CAR 1870 e nuovi 8192, tail zero in LOAD Windows. Lo stesso CAR atomico include `og_resident.fixture_journal` con wire portatile/checksum; CONTINUE verifica checksum e ripristina testimoni senza rieseguire gare; i vecchi CAR senza il journal sono `partial=1`, senza inventare score.
- **Restano aperti:** binding single-writer preventivo manager/CPU, tutte le fixture datate esattamente, qualificazioni e tre edizioni, summary storico finale, test di regressione Windows. Non marcare fase 3 completa. Gli stati OG veri sono sempre autoritativi. Obiettivo invariato: nuove competizioni definite tramite dati/regole, senza codice C dedicato per ogni torneo.
- Report tecnico e protocollo SI/NO/NON PROVATO: [OG_RESULT_WITNESS_SAVE_013102_20261118.md](OG_RESULT_WITNESS_SAVE_013102_20261118.md). Full build e gameplay Windows 0.13.102 NON PROVATI; warning nuovi da verificare sulla compilazione completa.

## 18/11/2026 — 0.13.101: risultanze OG calendario/manager, prova di riconciliazione READ-ONLY
- Le gare visibili nel calendario del club non sono inventate: `Career_UpdateStatsAfterMatch` conserva punteggio/flag (piu' eventuali rigori) nella `teamSelectionKeyCache`, le routine DIY OG aggiornano classifiche/tabelloni, e il CAR ne conserva una rappresentazione. Questo **NON dimostra** ancora il journal datato di tutte le gare CPU del mondo.
- `Expanded_OGWorld_BindClub` copia il campionato residente nel contesto locale; il test `Expanded_OGWorld_DebugCanAdvanceClub` blocca divergenze. Nuova verifica runtime `[og-manager-fixture]` confronta score gia' prodotto, ID orientati, fase, conteggi e classifica per club; usa parser bounded della keycache e test C isolato. **E' un audit non mutante, NON un merge ne' una seconda simulazione.**
- Scoperta critica: `career_teams.c` dichiara una cache runtime di 8192 byte, `save_format_json.c` serializza il raw cache e gli override endianness con limite 1870. Il nuovo log misura `cache-used` / `match-end` e segnala `car-capacity-risk=YES` se eccedono; nessuna perdita nel CAR reale e' provata finche' non si esamina il suo contenuto. Prima di aumentare il salvataggio, definire restore compatibile con CAR preesistenti.
- Obiettivo centrale invariato: importare competizioni da OG/altri SWOS/nuove definizioni con regole condivise e senza patch C a torneo. Single-writer effettivo, CAR atomico con ledger ed eventi e fase 3 **NON COMPLETATI**. Compilazione completa e gioco Windows 0.13.101 **NON PROVATI**. [Rapporto e checklist](COMPETITION_ENGINE_OG_MANAGER_RECONCILIATION_013101_20261118.md).

## 18/11/2026 — 0.13.100: archiviazione per DATA DELLA GARA e test diagnostici
- Corretto il contratto di compattazione del journal: l'anno-stagione e' calcolato sul giorno effettivo della **singola gara**, NON sull'anno d'inizio dell'edizione. Una coppa iniziata a febbraio 2027 puo' avere partite sia nella stagione 2026/27 sia nella stagione 2027/28; queste ultime restano dettagliate fino al termine della finestra biennale.
- Prima di compattare: rifiutare risultati datati nel futuro, edizioni ancora in corso, conclusioni ancora future e fatti di qualificazione non confermati. Test aggiornato sulle partite di aprile/novembre e sulla protezione del calendario luglio-giugno.
- Il runner Windows diagnostico riconosce ed esegue due test isolati Competition Engine (scheduler e retention), riportando scope COMPETITION_ENGINE_ISOLATED, MA NON certifica gara reale, checkpoint CAR o gameplay.
- **OBIETTIVO VINCOLANTE DI QUESTO LAVORO:** poter aggiungere/importare competizioni OG, altre versioni SWOS e originali tramite definizione e regole parametrizzate, senza dover modificare codice specifico per ogni torneo; facilitarne diagnostica, calendario, qualificazioni, storico, salvataggi e nuove edizioni. Il collegamento OG e' un prerequisito, non il traguardo.
- Audit: nei residenti `Career_AutoAdvanceLeagueFixtures` e `Career_AutoAdvanceCupMatches` chiamano `DIY_PlayOrSimulateLeagueFixture` e `DIY_PlayCupMatchAndAdvance`, che gia' producono ed applicano risultati OG. In `Expanded_OGWorld_AdvanceUefa` il caso manager usa `PENDING_MANAGER_BIND` e NON deve essere sostituito con un secondo writer; per le coppe nazionali esiste `managed_excluded`. Nel binding campionato e' documentato il blocco quando locale/residente divergono. **Non esiste oggi un ID fixture comune verificato per entrambi i percorsi, ne' un CAR che checkpointi lo stesso risultato col nuovo scheduler**. Il gate 2d/2e/2f e quello della fase 3 restano APERTI; non attivare un nuovo dispatcher o millantare completamento.
- Stato consegna: modifica minima a codice isolato e runner, nessuna modifica ai callback OG, al formato CAR o al gameplay. Test Linux/Windows della nuova build e warning nuovi NON PROVATI fino a esecuzione. [Rapporto tecnico](COMPETITION_ENGINE_RECONCILIATION_GATE_013100_20261118.md).

## 18/11/2026 — 0.13.99: regola journal due stagioni nel Competition Engine (codice isolato)
- Decisione dell'utente: **solo la stagione corrente e quella precedente** conservano risultati delle singole gare nel nuovo journal; le edizioni precedenti rimangono nell'archivio con **solo vincitore, finalista se applicabile, giorno di chiusura, numero gare e ID immutabile dell'edizione**. L'archivio NON duplica formazione, reti per partita, statistiche, calendari, ranking o RNG.
- In `expanded_competition_history.h`: record giocati provenienti dal ledger single-writer, deduplica/anti-riscrittura, tombstone per edizioni concluse, chiusura verificata e compattazione condizionata; uso dell'anno stagionale luglio-giugno, nessun reset della coppa ancora attiva al 1 luglio. **Non comprimere finche' qualificazioni e dipendenze non sono persistite separatamente e confermate**.
- Test aggiunti per archiviazione, duplicati, world 9697/2627 e coppe cross-year; test C isolato e build Windows 0.13.99 **NON PROVATI in questa consegna**. Nessuna integrazione live OG o nuovo campo CAR; il gate fase 3 non e' completo fino a dispatcher single-writer, checkpoint CAR con RNG e CONTINUE verificato. [Rapporto](COMPETITION_ENGINE_HISTORY_RETENTION_20261118.md).

## REGOLA — 18/11/2026: RIMOZIONE DEI LOG OBSOLETI
- Quando si verifica una segnalazione legacy, eliminare o riclassificare le stampe ormai fuorvianti e il rumore ripetitivo. Conservare SEMPRE gli errori veri, i warning GCC, i dettagli diagnostici necessari, i contatori riassuntivi e i limiti non implementati. Non eliminare messaggi per far apparire PASS un controllo. Le categorie normali (es. club fuori lega) possono essere aggregate anziche' stampate riga per riga. Prima audit del codice, poi modifica minima, con compilazione e test Windows esplicitamente NON PROVATI se assenti.

## REGOLA PERMANENTE — 18/11/2026: SEGNALAZIONE COMPLETA LOG E WARNING
- In OGNI analisi diagnostica riferire esplicitamente tutti i warning di compilazione, errori, messaggi runtime anomali, SKIP/FAIL, avvisi di salvataggio e altre anomalie osservate: distinguere cause corrette, messaggi innocui ma presenti e problemi aperti. Non omettere un evento perche' non bloccante o noto. Dichiarare per ogni categoria la quantita' o NON VERIFICATO se il log non la misura. Ogni warning GCC va corretto subito, non soppresso; un commit non equivale a compilazione/test Windows.

# SWOS Expanded — AGENTS.md

## REGOLA PERMANENTE approvata 18/11/2026 — WARNING DI COMPILAZIONE
- Ogni warning emerso da una compilazione va affrontato SUBITO: individuare causa e gravita', correggerne il codice senza sopprimerlo/disabilitarlo e verificarne la sparizione con una NUOVA compilazione completa. I warning di stile e quelli con rischio runtime si correggono entrambi; mai dichiararli superflui, ignorarli o assegnare un PASS statico. Se una correzione cambia algoritmi OG, formati CAR o scelte di design, chiedere il consenso prima. **Zero warning e zero errori restano obiettivi misurabili, NON risultati promessi.**
- 0.13.91: audit completo dei 91 warning della full build Windows 0.13.90 e patch mirata; [rapporto e verifiche pendenti](WARNINGS_AUDIT_01391_20261118.md). Il numero finale dopo compilazione Windows 0.13.91 e' **NON PROVATO**. Non spegnere `-Waddress-of-packed-member`, non toccare salvataggi originali, CLASSIC/DIY/SEASON o il motore OG in modo non autorizzato.


## 18/11/2026 — 0.13.90: warning GCC visibili e retry CAR Windows
- [Audit, codice e test pendenti](WARNINGS_AND_SAVE_RETRY_01390_20261118.md): su 0.13.89 compilazione completa Windows 4 warning (Hall init, puntatore stack del sorteggio, due commenti). Correzioni minime 0.13.90; `-Waddress-of-packed-member` riattivato, NON dichiarare zero warning finche' un intero rebuild Windows non lo conferma.
- Un salvataggio menu-exit 2627 della sessione 2 falliva in MoveFileExA con Win32 5 DOPO la serializzazione. Aggiunti retry limitati per access denied/sharing/lock con pause massime totali 750 ms; vecchio CAR mai cancellato. La causa esterna non e' provata; test Win32 reali, 96/26 e CLASSIC/DIY/SEASON ancora PENDENTI. Non cambiare formato CAR, gameplay o altri warning soppressi senza audit.


## 18/11/2026 — 0.13.89: audit warning packed applicato (Windows PENDENTE)
- Correzione mirata dei 29 `-Waddress-of-packed-member` osservati nei tre log Windows della 0.13.88. Sei file sorgente/header, più test C isolato. [Rapporto tecnico](PACKED_ALIGNMENT_AUDIT_20261118.md). Nessuna modifica alle strutture packed, agli algoritmi OG a 50 Hz, alle competizioni o al formato CAR; non disabilitare i warning.
- Dove necessario, copie temporanee allineate e `memcpy` delle rappresentazioni di puntatori/int32; nelle callback mutanti letture dei campi dal parent packed vivo. `FindTeamInDIY` e altri vecchi cast richiedono audit separato. Le 29 espressioni sono state revisionate nel codice, ma compilazione completa Windows, confronto warning e roundtrip CAR **NON PROVATI**.

## 18/11/2026 — 0.13.88: punto 2 con gate runtime ancora aperti
- Report e TODO: [COMPETITION_ENGINE_POINT2_20261118.md](COMPETITION_ENGINE_POINT2_20261118.md). Il codice ora condivide la regola degli override TEAM storici fra teams.c e TeamDb, e mantiene cache separate per world+mode. Non cancellare le cache alla selezione del mondo: potrebbero esserci puntatori CUSTOM già restituiti.
- Il nuovo header expanded_competition_identity.h contiene ID stabili da registry futuro e claim per fixture, ma NON è collegato a OG, manager, calendario o CAR. Non dichiarare funzionante il single-writer del mondo né il punto2 completo con il solo test header.
- Versione sorgenti 0.13.88; test Windows dell'utente PENDENTI. Prima sistema base poi B storico. Zero LWE, CLASSIC/DIY/SEASON invariati per progetto.


## 18/11/2026 — Fase 1 Competition Engine: audit statico CHIUSO, decisione B
- [COMPETITION_ENGINE_PHASE1_CLOSURE_20261118.md](COMPETITION_ENGINE_PHASE1_CLOSURE_20261118.md): audit source e RNC France/Turkey concluso, ma **NON** nuovo engine o test Windows. L'utente approva **B**, formati/regolamenti storici evolutivi **DOPO** il sistema basico funzionante. Champions OG a 16 squadre oltre il 1996/97 solo sandbox tecnico; non pubblicare il 1997/98 a 16 come storia reale.
- **Incoerenza confermata:** `teams.c::LoadTeamFile` usa per CAREER 9697 `data/9697/TEAM.013/.038` (France 20, Turkey 18 prime squadre), `team_db.c::read_country` prende generici (France 18, Turkey 17), cache `s_country` priva di mondo. Servono resolver comune/isolamento cache e test prima di nuovi ID; NON confondere TeamDb UUID/skill checksum con ClubKey stabile in carriera. Nessuna modifica a codice o asset in questo commit.


## 18/11/2026 — Audit OG di fattibilita' + coppa pilota: riferimento obbligatorio
- L'audit statico [COMPETITION_ENGINE_OG_AUDIT_20261118.md](COMPETITION_ENGINE_OG_AUDIT_20261118.md) distingue RIUSA, ADATTA e NON CHIAMARE LIVE. Per il Competition Engine **non usare un solo simulatore per tutti i tornei**: `CalculateViewResult`, `world_conti.c`, `uefa_access.c`, `world_cupd.c` usano logiche/RNG diversi. Mai chiamare `Career_AdvanceToNextSeason_UpdateCompetitions` integralmente nel nuovo orologio, perche' simula e muta competizioni; mai creare un secondo writer.
- **Pilota proposto:** Champions OG 1996/97, type-2, 16 squadre e quattro gironi; tre edizioni consecutive SOLO dopo verifica fonti qualificati, formato delle successive e binding manager. Francia/Turchia e gli altri file sono stati corretti nel codice 0.13.69 secondo audit precedente, ma il preflight e' ancora obbligatorio. In assenza di dati attendibili, STOP e richiesta decisione, nessun riempimento inventato.
- Punto 1 TODO: **analisi statica parziale completata**, mappa side effect esaustiva/golden/riproducibilita' ancora PENDENTI; non proclamare PASS o avviare big-bang. Solo DOC, build 0.13.87 invariata.


## Decisione VINCOLANTE 18/11/2026 — Competition Engine, prevale dove confligge con OG-unico 14/11
- **APPROVATO, NON IMPLEMENTATO:** Expanded diventera' il gestore unico delle competizioni mondiali, data civile, fixture, qualificazioni, edizioni, risultati persistenti, storico e CAR. OG rimane il riferimento degli algoritmi calcistici e della partita 50 Hz INVARIATA. Le precedenti regole 14–15/11 «OG unico proprietario anche dei calendari e dell'intero ciclo delle competizioni» sono [STORICO/SUPERATO] limitatamente all'autorita' sul calendario/competizione; continuano a valere: niente doppia gara, risultati inventati, doppio LWE, reset del mondo all'ingaggio, modifiche CLASSIC/DIY/SEASON, cancellazione CAR.
- TODO completa e vincolante: [COMPETITION_ENGINE_ARCHITECTURE_TODO_20261118.md](COMPETITION_ENGINE_ARCHITECTURE_TODO_20261118.md). **Sette attivita' ≠ sette versioni:** tre traguardi A fondamenta, B coppa pilota, C importatore. Coppa pilota = una coppa SWOS gia' esistente, preferibilmente continentale, TRE edizioni consecutive con qualificazioni, fixture per ID, save/CONTINUE e controllo no-double-sim. Seguire gate GO/NO-GO, STOP su duplicazioni, perdita save, regressioni. Nessuna migrazione massiva prima del pilota e secondo formato.
- Nuovi dettagli di origine del mondo, archivio integrale pluridecennale, coppa pilota specifica, regole personalizzate richiedono verifica/decisione; nessuna nuova toolchain. **Questo e' un commit solo documentale, 0.13.87 invariata.**


## Regola aggiornata — 17/11/2026, root semplificata (prevale sulle vecchie istruzioni BAT)
- Nella root restano **AVVIA_SWOS.bat e AVVIA_DIAGNOSTICA.bat**; COMPILA_SWOS.bat e PREPARA_SWOS.bat, insieme agli altri strumenti BAT, stanno in `diagnostica/`. Il launcher AVVIA_SWOS richiama PREPARA con percorso assoluto; tutte le compilazioni usano ancora la root come directory di lavoro.
- Log Windows in `diagnostica/log/SWOS.log` e `diagnostica/log/COMPILAZIONE_0140.log` per avvio ordinario; ogni sessione AVVIA_DIAGNOSTICA copia il log corrente nella propria directory isolata. Marker compilazione e backup video in `diagnostica/`. Conservare file preesistenti e salvare le tracce precedenti, non cancellare i CAR.
- `ASSETS.ini` effettivo in `assets/ASSETS.ini`; la copia nella root e' ridondante. `SDL2.dll` e `SWOS.ini` rimangono **accanto all'EXE** per il loader e le impostazioni native, salvo specifica modifica futura approvata. I BAT e le modifiche ai percorsi vanno verificati da Windows utente, non solo dal sorgente.

## Regola vincolante aggiornata — 17 novembre 2026: commit per intervento significativo
- La regola precedente «un solo commit per build» e' sostituita: **un commit separato per ciascuna modifica significativa e coerente**, in ordine di priorita' concordato. Non raggruppare fix indipendenti in un commit omnibus; non creare commit inutili per singole righe dello stesso intervento.
- L'autorizzazione corrente dell'utente «prendi la lista priorita' e builda da li» include la registrazione dei commit pertinenti alle priorita' approvate. Per nuove attivita' fuori elenco o nuove scelte di gameplay rimane necessaria l'approvazione.
- Ogni commit deve avere scopo, diff e verifica statica/isolata distinti; la build **Windows** e i test gameplay sono sempre eseguiti e attestati dall'utente dopo GitKraken. Non dichiarare PASS senza prova.
- Conservare la storia con commit lineari su `dev/0.10`, senza toccare main, release o backup. Verificare l'HEAD prima di ogni pubblicazione e non perdere modifiche altrui. Non cancellare save o asset runtime.
- Mantenere la numerazione della build secondo la politica esistente; annotare le scoperte nelle guide al commit pertinente.

## Regole correnti — 15 novembre 2026 (prevalgono sullo storico)
- Nei mondi 1996/97 e 2026/27 **solo OpenSWOS OG** e' autorita' per carriera, data, calendario, partite, risultati, classifiche, coppe, replay, spareggi e trofei. LWE puo' aggiungere esclusivamente dati/eventi non gestiti da OG. Le vecchie sezioni su simulazione di fixture/risultati LWE sono [STORICO/SUPERATO].
- A NEW CAREER -> START gli stati OG delle leghe residenti si inizializzano e raggiungono il 1 luglio prima di mostrare i menu; FORWARD TIME avanza gli eventi originali una sola volta. L'ingaggio/esonero collega/scollega il manager senza rifare la stagione. **Coppe residenti, storico fixture, salvataggio dell'intero mondo e test pluriennali sono tuttora PENDENTI.**
- Windows Expanded portatile: `save/AUTO9697.CAR` e `save/AUTO2627.CAR` accanto a `SWOS_Expanded.exe`; `SWOS.ini` e `SWOS.log` accanto all'EXE. Il precedente `save/AUTOSAVE.CAR` e' SOLO lettura legacy; non cancellarlo o sovrascriverlo automaticamente. Le istruzioni Windows in `Documents\\SWOS` valgono per OpenSWOS upstream, non per Expanded.
- L'utente compila e testa su Windows/MSYS2 dopo GitKraken. Non dichiarare compilato/testato un gioco solo perche' e' stata pubblicata una patch. Commit separati per intervento significativo approvato; vedi regola 17/11/2026. Per il codice successivo a 0.13.45_08 la versione si aggiorna a **0.13.46** secondo il versionamento a incrementi numerici di AGENTS.md.


## 0. Regole fondamentali ad alta priorità — workflow Codex (2026-11-09)

### Diario tecnico obbligatorio — decisione 13 novembre 2026

**Prima di analizzare codice, leggere tutte le scoperte documentate** in
`docs/ENGINE_CODE_DISCOVERIES.md`, oltre ad `AGENTS.md`,
`PROJECT_STATE.md` e alle guide pertinenti. Non ripetere analisi gia'
verificate ignorandone le conclusioni. Ogni nuova scoperta su codice,
asset o comportamento runtime va aggiunta al registro, con file/funzione,
riproduzione, prove, limiti e stato (ipotesi, codice, test C, Windows)
**nel prossimo commit autorizzato pertinente**. Le prove Windows mancanti
non diventano mai PASS, neppure quando il test C e' verde.

**Una feature alla volta, completata e verificata prima della successiva.**
Priorita' vincolante: CALENDARIO (incluso salto 5 anni e 30 anni senza
deriva), poi CAMPIONATI OG (senza simulazione LWE), poi COPPE nazionali/internazionali e
nazionali, poi stress integrato Windows. Non aggiungere funzionalita'
nelle fasi successive durante il lavoro di correzione della prima,
salvo strumentazione necessaria a verificarla. L'utente decide quando
la prova e' sufficiente. Nessun test statico/sintetico attesta da solo
il funzionamento integrato.



### Test runtime vincolanti (decisione del 13 novembre 2026)
MAI pilotare, falsificare, attenuare o aggirare un test per ottenere PASS. Nessun controllo statico (grep, presenza di stringhe/simboli o sola ispezione del codice) nella suite del motore: ammessi solo come revisione esterna alla suite. I test devono eseguire il codice reale e verificare risultati/invarianti concreti. Ogni FAIL deve mostrare fase, data, ID, atteso ed effettivo dove disponibili. Le prove negative intenzionali vanno marcate EXPECTED_REJECTION e devono fallire se l'input errato viene accettato. Controlli non eseguiti = NOT_TESTED, mai PASS. Un PASS della suite isolata non dimostra il funzionamento integrato Windows/OG.


Queste regole governano ogni futura richiesta di sviluppo. Le istruzioni esplicite dell'utente e i limiti della richiesta corrente restano vincolanti: l'autonomia riguarda le procedure autorizzate, non nuove decisioni progettuali. Preservare integralmente il lavoro locale non committato.

### Procedura obbligatoria

1. Consultare le regole applicabili, questo file, `PROJECT_STATE.md` e `docs/DEVELOPMENT_WORKFLOW.md`; verificare lo stato reale del progetto.
2. Controllare ramo, HEAD, file modificati/non tracciati, codice attivo e dipendenze interessate prima di intervenire.
3. Per contraddizioni autentiche, indicare le decisioni in conflitto, presentare le alternative e chiedere all'utente quale prevale prima di modificare la parte controversa. Non trattare note esplicitamente storiche o superate come nuove decisioni da riaprire.
4. Ricercare e proporre spontaneamente metodi tecnici migliori secondo la regola sulle alternative qui sotto.
5. Per nuove UI esaminare codice attivo, `sorgenti/docs/FONTS.md`, font e funzioni di rendering, misure dei testi e valori massimi, coordinate, dimensioni, hitbox, callback e navigazione con mouse, tastiera e joypad.
6. Preparare prima un PNG dimostrativo con testi e dati riproducibili dal motore; ottenere l'approvazione prima di generare l'asset pulito e implementarlo. Applicare i dettagli della procedura UI in sezione 4.
7. Dopo le approvazioni pertinenti, modificare i file locali, compilare e verificare quanto possibile nel perimetro autorizzato. L'approvazione grafica non estende da sola l'autorizzazione a implementazione, sostituzioni o alternative tecniche non richieste. Rispettare ogni divieto corrente di build, avvio o installazione.
8. Preservare gameplay, fisica, salvataggi, controlli, layout e asset approvati salvo richiesta esplicita.
9. Non effettuare automaticamente commit, push, pull, merge, checkout o cambi di ramo. Ogni operazione richiede autorizzazione esplicita; GitKraken rimane lo strumento di sincronizzazione Git scelto dall'utente.
10. Riassumere sempre modifiche, test effettivamente eseguiti, test pendenti, limitazioni ed eventuali problemi.

### Alternative tecniche: proposta obbligatoria, adozione autorizzata

Ricercare e proporre spontaneamente alternative tecnicamente migliori, anche se l'utente non le conosce o non le richiede espressamente. Valutare, quando pertinenti: motori grafici e renderer; SDL2, SDL3 e altre librerie; shader, effetti e animazioni; generazione di immagini, sprite e pixel art; modelli e strumenti AI specializzati; automazione dello sviluppo e degli asset; metodi più semplici per ottenere qualità grafica superiore.

Confrontare le alternative rilevanti per qualità, prestazioni, compatibilità Windows, manutenzione, tempi, costi e rischi. Raccomandare chiaramente la soluzione migliore, motivandola rispetto al progetto reale.

**REGOLA VINCOLANTE: non implementare mai un'alternativa tecnica proposta autonomamente senza averla prima presentata all'utente e ottenuto la sua approvazione esplicita.** Una richiesta generica di implementazione non autorizza cambi di motore, renderer, architettura, strumenti, gameplay o asset approvati. La ricerca di alternative non autorizza installazioni, aggiornamenti o migrazioni; mantenere la soluzione approvata finché l'utente non decide diversamente.

**Decisione definitiva:** CLASSIC MODES rimane una voce principale del Title Menu e apre il relativo sottomenu. Non trattarla come una contraddizione da riaprire.

## 1. Scopo del documento

Questo documento contiene le regole permanenti che ogni assistente AI deve rispettare durante lo sviluppo di SWOS Expanded.

Prima di iniziare una sessione di sviluppo:
- Leggere questo file.
- Consultare lo stato aggiornato del progetto e le decisioni precedenti pertinenti.
- Controllare il codice effettivamente presente nel ramo di lavoro.
- Non presumere che una funzionalità discussa sia già implementata.
- In caso di contraddizioni, chiedere spiegazioni all'utente


## 2. Filosofia del progetto

SWOS Expanded è un'espansione di Sensible World of Soccer basata su OpenSWOS.

Principi fondamentali:

- Preservare il gameplay originale delle partite, i controlli e il feeling SWOS.
- Espandere soprattutto la carriera manageriale, l'economia, le competizioni e la simulazione del mondo calcistico.
- Non sostituire il motore delle partite con un sistema completamente nuovo.
- Non eliminare funzionalità originali per semplificare l'implementazione.
- Integrare le nuove funzionalità nel gioco, evitando interfacce esterne non richieste.
- L'obiettivo finale comprende campionati e divisioni di tutto il mondo.

## 3. Regole di comunicazione e decisione

- Essere sinceri sulle capacità, sui risultati e sulle limitazioni.
- Non dichiarare implementata o funzionante una funzionalità senza verificarla.
- Chiedere chiarimenti quando una richiesta è ambigua o contraddice decisioni precedenti.
- Non prendere autonomamente decisioni importanti sul gameplay.
- Non aggiungere funzionalità soltanto perché sembrano interessanti.
- Proporre miglioramenti quando utili, ma distinguerli dalle richieste approvate.
- Conservare le idee non implementate nel registro del progetto, senza trattarle come obblighi.
- Fornire spiegazioni chiare e concise, preferibilmente in italiano.

## 4. Interfaccia grafica

### Stile

- **Nuovi asset grafici: seguire la pixel art illustrata degli sfondi di SWOS Expanded**, con ricchezza di dettagli, palette, ombreggiature e finiture coerenti con questi sfondi. I menu originali OG di SWOS **non sono un modello estetico da seguire** per progettare nuovi asset o schermate. Conservare le schermate già approvate salvo richiesta di modificarle.
- **Motivo:** vogliamo un’identità visiva Expanded uniforme e più curata; copiare i menu OG produrrebbe stili disomogenei e limiterebbe il nuovo design.
- Preferire il font originale SWOS per testi renderizzati dal gioco quando compatibile con la nuova UI.
- **Ogni nuovo artwork del frontend moderno va creato come PNG lossless nativo**, su una nuova superficie, non come BMP obbligatorio; gli asset storici BMP/RAW approvati non vanno convertiti o sostituiti senza richiesta. Gli asset precedenti restano riferimenti e si conservano intatti.
- **Ogni asset UI deve nascere senza sfondo incorporato**: l'esterno ai pannelli e ai disegni deve rimanere trasparente; uno sfondo illustrato, se richiesto, è un file separato. Non imporre una barra o una cornice generale se non richiesta.
- Nel PNG UI pulito gli elementi **dinamici** restano al renderer: testi e cifre, stadio variabile, faccine di BOARD/FANS e FORM/MORALE, focus, hover e selezione. Le icone di strutture statiche possono restare nell'asset solo se approvate.
- Nelle tabelle UI, con font alto **6 px**, rispettare almeno **1 px libero identico sopra e sotto** fra testo e bordi orizzontali (minimo 10 px inclusi due bordi da 1 px). Con font 8 px il minimo equivalente è 12 px.

- Non riprogettare schermate approvate senza richiesta.
- Evitare pannelli, cornici, ombre e decorazioni superflue.
- Mantenere testi, numeri e pulsanti leggibili e correttamente allineati.

### Procedura obbligatoria per progettare nuove UI grafiche (prima dell'asset PNG pulito)

1. **Analizzare il codice del menu realmente in uso**, recuperando funzioni di rendering dei caratteri, font effettivamente disponibili nel gioco, dimensioni native, scale, spaziature, allineamenti e coordinate di testi e valori. Non usare font o misure soltanto indicative senza dichiararlo.
2. **Calcolare gli ingombri reali** di tutte le etichette, dei nomi lunghi e degli importi con il numero massimo ragionevole di cifre (incluse unità, segni e separatori). Predisporre i pannelli in modo che testo e numeri renderizzati dal gioco siano leggibili, allineati e non si sovrappongano; se si cambia il layout rispetto al codice attuale, pianificare espressamente anche le nuove coordinate del renderer.
3. **Disegnare i pannelli in base a questi ingombri**, conservando lo stile illustrato pixel art degli sfondi di SWOS Expanded, NON quello dei menu OG. Separare l'eventuale illustrazione di sfondo e gli elementi dinamici dai pannelli.
4. **Prima proposta: PNG lossless dimostrativo**, con testi, numeri e valori nelle posizioni e con i font effettivamente riproducibili dal gioco; mostrarlo per approvazione prima della versione pulita.
5. **Dopo l'approvazione della proposta con dati**, creare il PNG UI pulito privo di testi e valori dinamici e con trasparenza reale, conservando layout e scritte statiche approvate. Gli sprite UI restano indipendenti dallo sfondo e il motore disegna dati e testi dinamici.
6. Verificare la versione pulita e le coordinate di rendering insieme, senza alterare asset già approvati o integrarli senza la relativa richiesta.

**Motivo della procedura:** evitare anteprime grafiche non riproducibili nel gioco, font inesistenti o importi troncati. Una proposta con dati reali permette di validare coordinate e ingombri prima dell'asset UI pulito. Il nuovo frontend usa PNG lossless; i BMP/RAW storici approvati restano invariati.

### Standard di generazione e risoluzione del frontend moderno (2026-11-08)
- **Risoluzione nativa di generazione di riferimento per sfondi full-screen: 1672×941 px.** Fare soltanto un ritaglio per ottenere **1664×936 px, 16:9 esatto**, senza ridimensionamento salvo richiesta esplicita. Salvare PNG lossless, conservando l'originale di generazione distinto dal file del gioco.
- **Avviso obbligatorio:** se la risoluzione effettiva prima del crop non è 1672×941, oppure dopo il crop non è 1664×936, comunicare entrambe le dimensioni e la differenza **prima** dell'approvazione o integrazione. Non chiamare "nativa 1920×1080" un'immagine ingrandita.
- **Frontend logico: 1920×1080; output video di riferimento: 3840×2160.** L'opzione video per scegliere 1080p, 1440p, 4K o desktop è programmata, non implementata. Sprite UI, pulsanti e logo PNG hanno dimensioni indipendenti ma scala coerente.
- Ogni schermata usa **sfondo PNG separato**, **logo SWOS Expanded PNG trasparente canonico condiviso** e **sprite PNG UI separati**. Testi, importi e valori dinamici rimangono nel renderer; input e callback SWOS rimangono funzionanti. I vecchi menu approvati non si modificano senza richiesta.
- **Soluzione B:** modernizzare progressivamente il frontend SDL2 in RGB/RGBA; preservare logica della partita OpenSWOS, controlli e salvataggi. Solo dopo test il nuovo asset può sostituire l'attuale sfondo di gioco.
- I **nuovi BMP non sono più obbligatori**. Resta obbligatorio preservare ogni BMP/RAW o PNG storico approvato finché non ne venga richiesta la sostituzione.

### Sfondi

**Non aggiungere immagini di sfondo alle nuove interfacce, ai menu o alle anteprime se l'utente non le richiede esplicitamente.**

Le schermate storiche con sfondi già approvati mantengono il proprio design.

Quando viene richiesto uno sfondo illustrato:
- Crearlo come asset separato dagli elementi interattivi.
- Non incorporare pulsanti, valori dinamici o barre di caricamento negli sfondi PNG definitivi; se richiesti, i dimostrativi della UI sono PNG separati. Il renderer gestisce gli elementi dinamici.
- Creare la UI separatamente senza sfondo per consentire il riposizionamento indipendente degli elementi UI.

### Schermate approvate

- BUSINESS: conservare asset e layout precedentemente approvati come riferimenti storici e mantenere la schermata esistente fino alla futura riprogettazione e nuova approvazione.
- Preservare il layout SQUAD approvato.
- Utilizzare il riferimento grafico approvato denominato **TRANSFER 1** per il menu principale del mercato.
- Non introdurre cornici esterne nel layout TRANSFER 1.
- Non modificare gratuitamente posizioni, dimensioni e gerarchia degli elementi.
- Le faccine di FORM e MORALE devono usare lo stesso set grafico canonico in tutte le schermate (BUSINESS, SQUAD, MATCH STATS e ogni altra UI), con espressioni, soglie, colori e dimensioni coerenti; non disegnarne varianti indipendenti. Eventuali modifiche devono essere condivise e verificate su tutte le schermate.
- MATCH STATS: mantenere l'asset grafico approvato in `development_assets/match_stats/` come riferimento visivo; separare l'illustrazione statica dai nomi, punteggi e valori dinamici, senza testi duplicati e senza modificare altri menu.

## 5. Joypad, mouse e tastiera

Ogni schermata deve essere utilizzabile con joypad.

- Tutti i pulsanti devono essere raggiungibili tramite navigazione direzionale.
- La selezione deve essere visivamente riconoscibile.
- Tooltip e hover devono funzionare premendo il tasto x sul joypad per attivare disattivare.
- EXIT, BACK e gli altri comandi non devono diventare irraggiungibili.
- Evitare blocchi, cicli di navigazione errati e perdita del focus.
- [SUPERATO dalla regola input 22/11/2026] Mantenere il supporto per mouse e tastiera.

**Legenda controller universale approvata:** usare, quando la schermata richiede queste azioni, il BMP nativo `assets/ui/controller_abc_approved_2026-11-07.bmp` (70×14, sfondo trasparente): **A SELECT**, **B BACK**, **X +INFO**. È una UI comune riutilizzabile, non esclusiva della Business Screen. La posizione **X=9, Y=6** è approvata per BUSINESS; nelle altre schermate le coordinate devono essere controllate sul layout e concordate, senza coprire testi o pannelli approvati. Non aggiungere la legenda nelle schermate dove un'azione non è disponibile e non far presumere che la stessa mappatura sia già implementata ovunque: collegare e testare le callback reali per tastiera, joypad e mouse prima di mostrarla come attiva. Non alterare il BMP approvato.

Ogni nuova schermata deve essere verificata anche dal punto di vista della navigazione.

## 6. Rendering e compatibilità

Piattaforma principale: Windows 10/11 x64.

- Mantenere SDL2 e la base OpenSWOS.
- Supportare 3840×2160 a schermo intero.
- **Durante lo sviluppo**, il launcher Windows `AVVIA_SWOS.bat` deve aprire il gioco in una **vera finestra massimizzata**, grande quanto l'area di lavoro del monitor, mantenendo barra del titolo, barra delle applicazioni e normale Alt+Tab. Non usare il fullscreen senza bordi al posto della finestra.
- La finestra development **deve restare massimizzata anche attraversando menu → partita → menu**: il cambio di vista/logical size SDL non deve reimpostare la dimensione della finestra massimizzata alla risoluzione originale. Preservare il normale ridimensionamento per le finestre non massimizzate e il fullscreen desktop delle release.
- **Nelle build release**, il gioco deve partire **sempre a schermo intero** (`fullscreen=desktop`, senza finestra di sviluppo massimizzata). Non trasferire le opzioni di avvio usate per il debug nel pacchetto release.
- **Motivo:** nello sviluppo è più comodo passare a editor, GitKraken e strumenti di test senza cambiare risoluzione; per i giocatori la release mantiene l'esperienza fullscreen 4K prevista.
- Evitare bande nere e residui grafici dei menu originali.
- Conservare la nitidezza della pixel art.
- Non alterare inutilmente il rendering delle partite.
- Il gioco mantiene la propria logica temporale originale a 50 Hz.
- Non introdurre netplay senza richiesta.



## 7. Modifiche al codice

Prima di modificare il codice:

1. Identificare i file e le funzioni realmente coinvolti.
2. Comprendere le dipendenze con le altre schermate e funzionalità.
3. Applicare modifiche mirate, evitando riscritture inutili.
4. Preservare la compatibilità con i salvataggi quando possibile.
5. Non cancellare codice o asset senza verificarne l'utilizzo.
6. Evitare regressioni nelle funzionalità già approvate.

Se una modifica richiede un intervento più ampio del previsto, spiegare il motivo prima di procedere.

## 8. Compilazione e test

Workflow locale approvato il 9 novembre 2026:

- Codex lavora direttamente sui sorgenti e sugli asset in `C:\Users\xgiuz\Downloads\Repo SWOS expanded\SWOS-Expanded`.
- Salvare i nuovi PNG approvati direttamente nei percorsi definitivi del progetto.
- Modifiche, compilazioni e test locali possono essere ripetuti senza commit intermedi, rispettando i limiti della richiesta corrente.
- Commit e push richiedono ciascuno autorizzazione esplicita dell'utente; approvare un asset non autorizza nessuno dei due.
- Non installare o aggiornare pacchetti e non avviare il gioco senza la relativa autorizzazione.
- A inizio sessione verificare ramo, HEAD e modifiche locali; preservare il lavoro non committato, anche tra sessioni.
- Riportare separatamente controlli statici, compilazioni e test reali eseguiti.

Preservare gli script necessari, inclusi:

- `AVVIA_SWOS.bat`
- `COMPILA_SWOS.bat`
- `PREPARA_SWOS.bat`
- `BUILD_SWOS.sh`

Non confondere un controllo statico del codice con un test reale su Windows.

## 9. GitHub e versionamento

Repository: `Giuzy/SWOS-Expanded`

Prima di ogni intervento, verificare il ramo di sviluppo corrente nel repository e nello stato del progetto; non presumere che rimanga sempre lo stesso.

- Non effettuare merge automatici con `main`.
- Non cambiare ramo senza verificare lo stato del progetto.
- Utilizzare commit con descrizioni chiare.
- Evitare commit che mescolano modifiche indipendenti.
- Non eliminare cronologia, asset o file importanti senza controllo.
- Non eseguire operazioni Git distruttive senza autorizzazione.
- Prima di ogni commit verificare quali file vengono modificati.
- Aggiornare questo file quando cambia una regola permanente di lavoro; registrare le modifiche al comportamento del gioco nella documentazione di progetto pertinente.

## 10. Gestione degli asset

GitHub e` la copia condivisa di lavoro per gli interventi da ChatGPT normale; GitKraken sincronizza la copia locale. Commit solo su autorizzazione esplicita.

### Formato dei nuovi asset di frontend: PNG lossless

- **La regola BMP nativo obbligatorio è annullata per i nuovi asset grafici.** Il nuovo frontend usa PNG lossless RGB per sfondi e PNG RGBA con trasparenza per loghi, pulsanti, pannelli e sprite. I BMP/RAW storici e approvati non vengono convertiti automaticamente.
- Il PNG è il nuovo asset di produzione per il frontend moderno; verificare formato, risoluzione reale, crop, alpha e supporto del loader prima dell'approvazione/inserimento in gioco.
- Prima dell'archiviazione verificare risoluzione, profondità colore, compatibilità con il loader del gioco e conservazione dei dettagli pixel art; seguire le dimensioni native e non cambiare le coordinate dei testi/valori dinamici senza richiesta.
- **Perché:** evitare conversioni tardive, caricamenti duplicati, disallineamenti, confusione tra versioni e lavoro ripetuto durante l'integrazione.
- Applicare la procedura §4 per nuovi asset UI: proposta PNG con dati reali, approvazione, poi PNG pulito con i soli elementi statici. Conservare separati bozze, sfondi, sprite UI e file di produzione.
- I vecchi file PNG già presenti o i nomi storici esplicitamente richiesti restano invariati finché non venga approvata una migrazione; non rompere riferimenti esistenti soltanto per normalizzare le estensioni.


- Conservare gli asset approvati.
- Distinguere asset utilizzati nel gioco, sorgenti grafiche e bozze.
- Non sostituire asset approvati senza richiesta.
- Non integrare automaticamente nel gioco un asset appena archiviato.
- Utilizzare nomi chiari e mantenere le varianti quando necessario.
- Per ogni schermata con asset approvati, archiviare **due file distinti**: uno sfondo illustrato senza elementi interattivi e una UI approvata separata senza sfondo. Non fondere sfondo e UI, così gli elementi possono essere modificati indipendentemente.
- Per TRANSFER 1 conservare i nomi esatti richiesti dall'utente: `transfer_bg_approved.png` per lo sfondo e `ui_approved.png` per l'interfaccia. Correggere sempre errori di ortografia e refusi, anche nei nomi dei file proposti, preservando i riferimenti funzionanti nel codice quando una rinomina richiede aggiornamenti coordinati.
- Verificare provenienza e licenze degli asset esterni.
- Caricare nuovi asset quando l'utente lo richiede, tramite i metodi disponibili.

## 11. Documentazione e continuità

Mantenere separati:

- `AGENTS.md`: regole permanenti di lavoro.
- `PROJECT_STATE.md`: stato attuale, funzionalità presenti, problemi aperti e prossimi interventi.
- Registro delle idee: proposte approvate, da definire, scartate o storiche.
- Documentazione tecnica e cronologia dei commit.
- `IMPROVEMENTS.md`: cronologia, organizzata per versione, degli interventi realizzati rispetto al SWOS originale (per esempio nuovi sfondi, menu e funzionalità), con riferimenti ai commit quando disponibili. In apertura descrivere la provenienza del progetto, dalle versioni originali di SWOS per Amiga alla base OpenSWOS ottenuta tramite reverse engineering, e riportare i crediti verificati degli autori e dei contributori, compresi Ben Baker e Zlatko Karakaš.

Non cancellare le vecchie idee dal registro: indicare quando sono state superate e da quale decisione.

## 12. Aggiornamento delle regole

Quando l'utente scrive **(regola)**, registrare la decisione e aggiornare `AGENTS.md` localmente quando la modifica è autorizzata, senza attendere un commit; commit e push richiedono autorizzazione esplicita.

Quando emerge una nuova regola permanente:

1. Verificare che sia una decisione effettivamente confermata.
2. Controllare eventuali contraddizioni con le regole precedenti.
3. Aggiornare `AGENTS.md` con il consenso dell'utente.
4. Attendere la richiesta esplicita di commit dell'utente.

Non modificare questo documento autonomamente soltanto perché una soluzione alternativa sembra migliore.

## 13. Correzione ortografica

- Correggere sempre gli errori di ortografia e i refusi nei testi, nella documentazione e nei nomi di nuovi asset/file.
- Se un nome errato è già usato nel codice o nei dati, aggiornare coerentemente i riferimenti prima di rinominarlo, per evitare regressioni.
- Quando una correzione cambia un nome già concordato, usare da quel momento la grafia corretta.

## 14. Regola fondamentale

**SWOS Expanded è un progetto condiviso: l'utente decide la direzione del gioco, l'assistente contribuisce alla progettazione e all'implementazione.**

L'assistente deve preservare il lavoro approvato, distinguere i fatti dalle ipotesi e chiedere chiarimenti prima di prendere decisioni importanti.
## 15. Commit su richiesta

- Quando l'utente scrive `committa` o chiede esplicitamente un commit, eseguire direttamente le modifiche richieste e creare il commit sul ramo di lavoro verificato, senza chiedere un'ulteriore conferma o tergiversare.
- Segnalare soltanto impedimenti concreti (per esempio permessi mancanti, conflitti o informazioni indispensabili assenti); non dichiarare un commit riuscito prima di riceverne conferma dal servizio.

## 16. Nome dell'eseguibile Windows

- Il file eseguibile del gioco deve chiamarsi esattamente `SWOS_Expanded.exe` in tutte le build Windows future.
- Mantenere coerenti il nome prodotto dalla compilazione, i percorsi e i riferimenti negli script di avvio e build (inclusi BAT e shell script), senza rinominare il repository o altri file non necessari.
- Verificare la compilazione e l'avvio su Windows prima di dichiarare applicata la rinomina al gioco.

## Gestione locale degli asset (2026-11-09)

Salvare i PNG approvati nei percorsi definitivi locali, verificandone formato e contenuto. L'approvazione autorizza il salvataggio, non la sostituzione di asset approvati o l'integrazione nel codice se non richieste. Nessun caricamento GitHub o commit automatico.

Il precedente trasferimento tramite blob/manifest è storico e abolito come workflow ordinario; vedere `asset-transfer/README.md`.

## Versionamento vigente — decisione 10 novembre 2026 (sostituisce regole precedenti)

- Formato ufficiale a tre numeri: **MAJOR.MILESTONE.COMMIT**. Primo commit del nuovo ciclo: **0.13.01**; poi **0.13.02**, **0.13.03**, ecc., con incremento **0.01 per commit di codice autorizzato**, senza più suffissi _01/_02.
- Un **nuovo milestone** cambia la parte centrale secondo l'incremento **0.1** indicato dall'utente. Nessun salto di milestone è automatico: per il prossimo salto concordare il valore completo prima di applicarlo. Le vecchie regole di avanzamento BUILD e delle release 0.20.00 sono **superate**, ma restano nello storico delle versioni.
- I commit esclusivamente documentali non forzano incremento della versione del gioco; aggiornare il GDD e gli altri registri nel prossimo commit pertinente autorizzato.
- Ogni commit è **sempre e soltanto esplicito**: nessun commit, push, pull, merge, checkout o cambio di ramo in assenza di richiesta chiara. Con «committa», verificare di nuovo HEAD, integrare modifiche concordate e versionare il sorgente.
- Aggiornare `SWOS_EXPANDED_VERSION` in `sorgenti/include/build_info.h` nello stesso commit di codice. La schermata TITLE mostra `Build ` + `Build_DisplayVersion()` senza modificare lo sfondo.
- Il ramo operativo resta **dev/0.10** fino a ordine diverso; non rinominare il ramo sulla base del solo numero versione, né toccare automaticamente `main` o `release/0.20`.
- **Nuovo workflow consegne:** codice, script e documenti direttamente su GitHub, mediante commit espressamente autorizzato. **ZIP solo quando occorre aggiungere nuovi asset grafici**, con i file nei percorsi corretti. GitKraken rimane lo strumento scelto per sincronizzare il PC.

## Workflow asset precedente — superato il 2026-11-09

Le procedure del 6–8 novembre tramite ZIP, `ASSETS_INBOX/` e trasferimenti GitHub sono abolite come workflow ordinario. È revocata ogni autorizzazione permanente a organizzare e committare automaticamente gli asset. La cartella inbox, verificata contenente soltanto `.gitkeep`, viene rimossa; non ricrearla. La cronologia delle consegne resta in `PROJECT_STATE.md`.

## Title Menu moderno e workflow aggiornato (8 novembre 2026)
- La scelta più recente per i **nuovi asset frontend è PNG lossless RGB/RGBA**, non BMP nativo obbligatorio. Il vecchio formato resta per asset storici, non per la nuova UI.
- I nuovi PNG approvati sono salvati direttamente nei percorsi definitivi locali (workflow del 9 novembre 2026).
- Il Title Menu richiesto dopo la 0.10.53 ha cinque voci di primo livello, con CAREER prominente; il secondo livello CLASSIC MODES conserva le funzioni SWOS originali, incluse CONTINUE/REPLAY/SAVE. Grafica user-selected e animazione di pressione SDL2 senza GIF. Il vecchio sfondo/pulsanti Title e F8 non devono essere più visibili una volta attivato il nuovo menu. Lo sfondo PNG e il logo canonico approvati rimangono invariati.
- Il nuovo codice non va presentato come compilato/testato Windows senza test. Non dichiarare che i binari PNG sono su GitHub quando sono presenti soltanto in locale.


## Focus e pressione dei pulsanti UI (regola utente, 2026-11-08)
- Il **frame giallo di selezione segue il contorno esterno effettivo dell'asset del pulsante**, comprese smussature e profondità 3D a destra/in basso. **Non usare un rettangolo sull'hitbox** quando il pulsante ha una silhouette diversa. Mantieni separati hitbox rettangolare (input), sprite PNG RGB/RGBA e contorno di focus generato/renderizzato.
- I pulsanti restano frontali, non inclinati; l'animazione di pressione SDL2 non deve ridimensionare o ricampionare il singolo sprite durante la pressione. Preferire una breve traslazione con **rimbalzo discreto di 1 px al rilascio**, senza deformare il PNG. Il focus segue il pulsante anche durante la pressione.
- Navigazione menu Title e sottomenu: compass SWOS verificato `0=su, 2=destra, 4=giù, 6=sinistra`; in elenchi verticali SU va alla voce precedente, GIÙ alla successiva, destra/sinistra non devono invertirne il verso.


## Pulizia file superati (decisione utente 2026-11-08)
- Quando si sostituiscono launcher/test/anteprime obsolete, rimuovere soltanto file **verificati non più necessari** e proteggere i nomi cancellati con `.gitignore` affinché copie locali rimaste sul PC non vengano reintrodotte per errore.
- Prima di eliminare un asset apparentemente obsoleto controllare tutti i riferimenti del codice e degli script di build: **non cancellare RAW/BMP storici ancora caricati**, asset approvati, riferimenti di altre schermate, sorgenti, dati o salvataggi.
- `AVVIA_SWOS.bat`, `COMPILA_SWOS.bat`, `PREPARA_SWOS.bat`, `BUILD_SWOS.sh` rimangono i launcher/script ufficiali; le versioni temporanee `AVVIA_E_TESTA_0.10.52.bat` e `AVVIA_E_TESTA_0.10.53.bat` sono state ritirate.
- Una cancellazione dal repository non garantisce la rimozione di copie non tracciate sul PC Windows: sincronizzare GitKraken e, se serve, rimuovere manualmente le copie locali dei soli percorsi archiviati come obsoleti; non usare pulizie generiche su tutto il repository.

 
## Asset base UI — regola di GENERAZIONE, senza selezione incorporata (2026-11-08)
- **Quando si genera un nuovo PNG base della UI**, NON includere mai il contorno giallo di selezione, focus o hover nel file grafico: essi sono effetti dinamici del renderer. La regola concerne la **generazione** dei nuovi PNG e non richiede di ridisegnare asset storici approvati.
- Mantieni alpha trasparente attorno alla sagoma, bordi decorativi blu/azzurri e spessore tridimensionale secondo l'asset approvato. Testi statici sono consentiti; valori, selezione e focus restano separati.
- Il renderer SDL2 produce il **contorno giallo dinamicamente** dall'alpha della sagoma completa, inclusi smussi e spessore. Non alterare l'arte approvata per generare o animare il focus.
- Poiché l'asset CONTINUE CAREER ha più pixel sorgenti del pulsante CAREER, dimensionare il focus separato per ottenere un contorno ugualmente spesso a schermo, senza ritoccare lo sprite.
- Il pulsante approvato **CONTINUE CAREER** è un PNG indipendente da CAREER; quando una carriera è attiva, renderizzare lo sprite `assets/title/buttons/continue_career.png` invece di `career.png`, senza ulteriori scritte sovrapposte dal motore. Se manca l'asset, lasciare il menu funzionante e segnalare l'errore, senza inventare un nuovo pulsante.

## Title 0.10.54 — icona modalità video e nuova build (2026-11-08)
- Icone approvate **senza testo e senza giallo**: espansione a quattro frecce verso l'esterno, riduzione a quattro frecce verso l'interno. Sono due PNG RGBA indipendenti da generare/ritagliare senza resize, posizionati in alto a sinistra del Title Menu, senza toccare lo sfondo o il logo.
- **Un solo simbolo alla volta**: in modalità finestra il tasto mostra le frecce esterne (azione fullscreen); in modalità fullscreen mostra frecce interne (azione finestra). La sorgente dello stato è la finestra SDL2 attuale; cambiare l'icona anche dopo Alt+Invio.
- Selezione con mouse/joypad/tastiera. Nel primo livello: SU da CAREER → icona; GIÙ da EXIT GAME → icona; GIÙ dall'icona → CAREER; SU dall'icona → EXIT GAME. Pressione e focus dinamici identici ai pulsanti, senza ridimensionare sprite durante l'animazione.
- **Development**: tornando dal fullscreen ripristinare una finestra vera, decorata e massimizzata se era massimizzata prima; **release**: fullscreen desktop reale. Conservare Alt+Invio e gameplay 50 Hz.

## Stile canonico dei nuovi pulsanti Expanded (regola 2026-11-08, 0.10.54_01)
- **Da ora in avanti**, per ogni nuovo pulsante del frontend moderno, usa lo stile pixel art blu dei pulsanti approvati del Title Menu: **vista frontale, bordi azzurri e spessore 3D più scuro a destra e sotto**. Nessuna inclinazione prospettica, nessun pannello esterno non richiesto.
- **Durante la generazione del PNG base** non includere MAI il contorno giallo di selezione. Focus/hover derivano dinamicamente dall'alpha e seguono tutto il profilo del pulsante compreso lo spessore. Non modificare asset storici già approvati per applicare la regola.
- Il pulsante selezionato ha una **pressione breve con bouncing discreto al rilascio**, gestita dal codice SDL2 tramite traslazione dei PNG originali, senza resize/riduzione di risoluzione. Questo comportamento è lo standard dei nuovi pulsanti Expanded salvo indicazione contraria dell'utente.
- I raggi del focus sono misurati **nei pixel del PNG sorgente**; per asset con differenti risoluzioni sorgenti adeguare il raggio affinché il bordo risultante abbia lo spessore visivo desiderato. Non confondere i pixel del PNG con quelli logici o dello schermo 4K.

## Workflow ChatGPT + GitHub + ZIP (decisione corrente 2026-11-09)
- Codex/Work **non** sono strumenti richiesti per SWOS Expanded. Il loro workflow locale e' storico; lavorare dalla chat normale tramite GitHub, con GitKraken per sincronizzare.
- Commit sul ramo verificato soltanto su richiesta esplicita. Nessun merge, force push, cancellazione o operazione sugli altri rami senza autorizzazione.
- Consegnare ZIP piccoli con eventuali asset/file nei percorsi definitivi; istruzioni per l'utente **direttamente nella chat e separatamente dagli ZIP**.
- Compilazione e test Windows devono essere dichiarati come effettuati soltanto dopo verifica reale dell'utente.
- NEW CAREER: scala UI 88,5% approvata per vedere tavolo/penna, sfondo senza resize; font bitmap diritto a pixel interi; frecce grafiche decorative non cliccabili; FAME cambia direttamente, niente submenu; checkbox dinamica dentro START UNEMPLOYED.

## NEW CAREER — revisione visuale 0.10.57 (2026-11-09)
- Font sui menu moderni: atlas bitmap PNG generati offline alla dimensione effettiva, bianchi e diritti come il mockup approvato; niente font 5x8 ingranditi come testo principale o ridimensionamento frazionario legato alla UI. Non modificare il Title gia' approvato.
- NEW CAREER AGE usa soltanto SINISTRA/DESTRA, limiti inclusivi 18-70; A non apre editor. NAME/SURNAME mostrano un cursore verticale al posto di '?' senza cambiare la logica InputText storica.
- Il focus dei pulsanti segue la silhouette alpha ed e' sottile (circa 2px sorgente); per i campi una sola traccia arrotondata, non un triplo bordo. Conservare l'UI al 88,5% e lo sfondo senza resize.

### OBSOLETO — marcatura degli asset sostituiti (2026-11-10)
Quando un asset approvato o storico viene sostituito da un nuovo asset runtime,
marcare il vecchio con suffisso `_OBSOLETO` prima dell'estensione, senza
sovrascriverlo e **solo dopo** avere verificato che nessun sorgente attivo lo
richiami. Lo stesso controllo si applica agli asset sostituiti in precedenza.
L'etichetta non autorizza una cancellazione immediata: il BAT periodico di
pulizia dovra' ricontrollare riferimenti, eventuali backup e file effettivamente
necessari prima di proporre la rimozione. Non rinominare mai un file ancora
richiamato dal runtime senza aggiornare tutti i relativi riferimenti.

---

## SWOS_DOC_SYNC_2026_11_10 — Nuove regole confermate (10 novembre 2026)

**Questa sezione prevale sui vecchi vincoli grafici incompatibili, senza cancellarne la storia.** Conflitti autentici con decisioni future vanno sottoposti all'utente prima di procedere.

1. **Design canonico sempre consultabile:** mantenere `docs/GAME_DESIGN.md` nel repository, accanto a `AGENTS.md`, `PROJECT_STATE.md`, `IMPROVEMENTS.md` e `docs/DEVELOPMENT_WORKFLOW.md`. Il GDD raccoglie decisioni, proposte, funzioni e note storiche, **marcate per stato**. Aggiornarlo al prossimo commit pertinente quando cambia una decisione o un design; non creare commit autonomi di sola documentazione salvo richiesta. Non presentare proposte come codice realizzato.
2. **Tre e solo tre schermate finali graficamente:** `TITLE`, `NEW CAREER` e `LOADING`. Tutti gli altri menu/sfondi della precedente fase grafica (compresi BUSINESS, SQUAD, CALENDAR, UNEMPLOYED, OFFER, WELCOME, SACKED, TRANSFERS, MATCH STATS e gli altri) sono **visivamente OBSOLETI, da riprogettare**. Il design dei vecchi pannelli non è più vincolante. Conservare le funzionalità, i dati e le callback esistenti; la nuova grafica di ciascun menu va nuovamente approvata. Un precedente asset «approved» può quindi essere **approvazione storica superata** sul piano grafico.
3. **Asset obsoleti e BAT di pulizia:** etichettare nella documentazione/inventario gli asset grafici pre-switch come **OBSOLETI — pending replacement**. Al **prossimo BAT di pulizia** eseguire la rimozione degli asset obsoleti **solo se realmente orfani**: prima analizzare riferimenti di sorgenti, script, JSON e fallback, poi sostituire i caricamenti con asset nuovi approvati, testare avvio e schermate; infine eliminare solo quelli inutilizzati, senza cancellazioni indiscriminate, senza rompere il gioco, preservando `.gitkeep`. Fino alla sostituzione gli asset obsoleti ancora in uso **rimangono fisicamente necessari**. Non classificare come eliminabili gli asset condivisi TITLE/NEW CAREER/LOADING o i dati di gioco soltanto per collocazione nella stessa directory. Nessuna rimozione automatica nel presente aggiornamento documentale.
4. **Motore del mondo:** priorità assoluta al **database delle squadre esistente**; l'espansione dei paesi/divisioni/database è l'ultima fase. Simulare eventi cronologici con **date globali** (anche incontri infrasettimanali) e mantenere risultati, classifiche, trasferimenti, ritiri e stagioni persistenti sia da occupato sia da disoccupato. Non simulare ogni dettaglio dei club non gestiti: simulare conseguenze coerenti. Nuova architettura da presentare e approvare prima della modifica al codice.
5. **Data e WEEK:** mostrare il numero della **settimana quasi ovunque** nei menu manageriali, con una sola fonte temporale, tenendo le coordinate e la convenzione esatta da definire nel design della schermata; non alterare il cronometro partita. Inserire l'azione **ADVANCE TO NEXT EVENT** sia in CALENDAR sia in UNEMPLOYED, senza saltare gli eventi intermedi né prendere decisioni per l'utente.
6. **Chairman e giovani:** nome + età del chairman in CLUB OFFER e WELCOME, età incrementale e sostituzione automatica al ritiro; YOUTH → RES (riserve) → TITOLARE come status dinamico, potenziale vero conosciuto per i giovani cresciuti nel club gestito e occultato per gli esterni senza scouting. Non assegnare prestiti TRIAL automatici; verificare prima il codice e preservare soltanto i fallback tecnici indispensabili alle rose insufficienti.
7. **Priorità di verifica:** leggere stato reale di ramo/HEAD, codice, save e dipendenze prima delle modifiche. Non chiamare «implementato» ciò che è soltanto in `GAME_DESIGN.md`. Non fare commit/push/checkout senza autorizzazione; consegnare ZIP soltanto per i nuovi asset grafici; codice/script/documenti tramite GitHub dopo «committa». Ogni prossimo commit autorizzato che modifica il gioco deve mantenere coerenti documenti, versione e test effettivi.

**Versione rilevata durante questa raccolta:** `dev/0.10`, HEAD `f33041df7204936da22e6a066977b0dc382093b4`, `SWOS_EXPANDED_VERSION` `0.10.75` (10/11/2026). Questo riferimento è storico e va riverificato all'avvio del prossimo intervento.


## Fase 1 — Data del mondo (10 novembre 2026)

- La prima versione del calendario civile deve accettare **giorno, mese e anno iniziali configurabili**: 1996/97 e 2026/27 sono esempi di pacchetti, non un vincolo 1996 hardcoded.
- Il numero WEEK concordato identifica la **settimana della stagione calcistica**, non la settimana ISO dell'anno civile. Per il futuro scheduler la stagione di riferimento dipenderà dal calendario del torneo/paese; per ora la nuova API conta settimane intere a partire dalla data iniziale fornita.
- Il primo commit del modulo può aggiornare solo le **date visualizzate** e i test; non equivale al Living World Engine, non autorizza modifiche alle chiavi delle fixture, al save e alle finanze.

## Fase 1–2: debug sviluppo e continuità mondiale (0.13.02, 10/11/2026)

- [REGOLA CONFERMATA] GitHub rimane il canale per codice, test, script e documentazione; ZIP **solo per asset grafici** nuovi. **Commit e push esclusivamente con «committa» esplicito**. In assenza di autorizzazione non pubblicare modifiche.
- [APPROVATO] Diagnostica veloce, senza nuovi asset: **F12** mostra/nasconde testo tecnico sopra il rendering esistente, **F9** salva un PNG dell'immagine finale *inclusa la diagnostica*. Il vecchio F12 per screenshot cede il posto al toggle; F11/PrintScreen rimangono disponibili per acquisizioni legacy.
- [APPROVATO] Quando il debug F12 è visibile, **1** alterna risultato sempre vincente, **2** sempre perdente (mutuamente esclusivi), **3** arma/disarma l'**esonero alla prossima azione di partita**. All'avvio tutti OFF; non salvarli nella carriera. Forzature risultati esclusivamente nella gestione dell'esito, **mai nei tick/fisica/controlli della partita**. Rimuovere/disabilitare i controlli di debug nella release.
- [APPROVATO] Non usare una squadra «fittizia» per simulare il mondo durante la disoccupazione. Le competizioni devono esistere e avanzare indipendentemente dal club dell'utente; consultarle già prima di accettare un'offerta, senza reset successivi.
- [STATO TECNICO] Il primo ledger datato è **un modulo autonomo testato, non ancora collegato al calendario delle competizioni live né al save**. Distinguere sullo schermo **NOT ACTIVE** dalle funzioni implementate e da quelle simulate in test. Non presentare il ledger come Living World Engine completo.


## 0.13.03 — Simulazione autonoma delle leghe, prima integrazione sperimentale

- [REGOLA TECNICA] Nessuna squadra fittizia per l'allenatore disoccupato. La simulazione pilota vive separatamente dall'occupazione, con **ID e forza delle rose installate realmente**; nessun caricamento di TEAM.* durante i callback dei menu modali. Il tempo mondiale dell'allenatore e le competizioni dovranno infine usare la stessa fonte cronologica.
- [AMBITO TEMPORANEO, NON FASE 1–2 COMPLETA] La prima prova carica i cinque paesi già verificati dal generatore offerte (codici 8, 13, 14, 20, 21), le loro divisioni supportate e — dove disponibile — il secondo file di squadre. Calendario round-robin provvisorio, per due stagioni, massimo 26 club per girone, con BYE nelle leghe dispari; eventuali divisioni troppo grandi non devono essere simulate parzialmente. Nessuna coppa ancora collegata.
- [REGOLA DI TRASPARENZA] Non chiamare «mondo persistente» una simulazione che vive solo nella RAM e non è salvata nel formato .CAR; non chiamare «ingresso a metà stagione implementato» se la chiamata corrente a `InitCareer()` resetta ancora le competizioni. Nel preview non confondere classifica pilota con classifica del motore carriera OpenSWOS.
- [APPROVATO] Il pannello F12 della 0.13.02 rimane **invariato** in questa revisione. Solo i dati del mondo pilota nella schermata VIEW LEAGUE sono nuovi; TITLE/NEW CAREER/LOADING e partita originale invariati.


## Decisioni di design 11 novembre 2026 (0.13.05)

- **[APPROVATO]** Data iniziale predefinita per la carriera europea: **1° luglio**, non più 1° giugno. Giugno resta parte del mondo simulato e del cambio di stagione; le competizioni di altri paesi devono avere calendari propri. Riferimento normativo dettagliato: `docs/GAME_DESIGN.md` §19.
- **[APPROVATO]** Precampionato di tutte le squadre: la squadra del manager organizza/gioca/simula amichevoli, le altre usano eventi economici/medici/morale/forma leggeri ma persistenti. La partita SWOS non cambia; nessuna amichevole occupa slot nativi di campionato.
- **[APPROVATO]** Prezzi biglietti impostabili dal manager in **CLUB**, con affluenza inversamente sensibile al prezzo e modificata da FANS LIKING, capienza, avversario e competizione. Predizione visibile e incassi contabilizzati **una sola volta**; nessuna grafica di CLUB approvata ancora.
- **[APPROVATO]** Una sola fonte dati e, dopo il redesign, un solo componente per **classifica** e **rosa** prima/dopo l'assunzione, con modalità sola lettura prima della firma.
- **[STATO, non promessa di completamento]** 0.13.05 implementa nel PILOTA l'orologio dal 1° luglio e amichevoli AI con aggregati solo RAM per i cinque paesi della prova. Non implementa ancora prezzo biglietti nel gioco, precampionato giocabile, coppe e passaggio dello stato pilota ai calendari nativi. F12, fisica partita e grafica finale restano invariati.


## Decisioni 11 novembre 2026 — sponsor e recupero calendario

- **[APPROVATO]** Ogni club mantiene sponsor che possono precedere il manager; numero e qualità dipendono dalla situazione (anche zero sponsor in crisi grave). Contratti con scadenza e opzioni di rinnovo; non annullarli quando cambia allenatore. **Non espandere oltre** il sistema per ora. Riferimento `docs/GAME_DESIGN.md` §21.
- **[PROPOSTA NON APPROVATA]** Forma individuale per giocatore, allenamento ordinario **collettivo** ed eventuali eccezioni individuali opzionali: da discutere senza modificare ora le statistiche o il database originale. Vedere §22.
- **[STATO TECNICO 0.13.06]** Esperimento di simulazione in carriera **attraverso i menu nativi**, non un import delle classifiche pilota. Le due fonti restano diverse fino alla progettazione della vera simulazione globale persistente. Non dichiarare uniformità dei punti senza il confronto e i test Windows.


### Commit per modifiche significative (11 novembre 2026)
**[REGOLA]** Nelle richieste di sviluppo, creare commit soltanto dopo
modifiche significative al gioco, agli asset o agli strumenti tecnici,
non dopo semplici analisi, controlli o micro-correzioni preparatorie.
Rimane richiesta autorizzazione esplicita per commit e push; nella
presente attività l'utente ha autorizzato i commit dei cambi significativi.
Verificare a ogni build il debug F12 e aggiungere, quando utile,
diagnostiche fondate su dati effettivamente misurabili. Non inventare
conteggi di manager/chairman o dimensioni dei database su disco.


### Sviluppo incisivo e rollback — decisione 11 novembre 2026
**[REGOLA UTENTE: «da ora in avanti spavalderia»]** Per interventi tecnici
espressamente autorizzati, preferire cambiamenti sostanziali e coerenti,
anche introducendo nuove funzioni o strutture dati quando migliorano
davvero l'architettura, invece di moltiplicare build con micro-fix
difensivi. Una build di sviluppo può essere sperimentale: mantenere
sempre identificato il commit stabile precedente per il rollback tramite
GitKraken e rendere espliciti rischi e test Windows ancora pendenti.
La regola NON autorizza commit/push non richiesti, modifiche a gameplay
o asset approvati non richieste, cancellazioni distruttive o scelte
arbitrarie in presenza di contraddizioni: l'utente continua a decidere.
Per il Living World Engine, mantenere il sistema di competizioni e le
tabelle calendario originali OpenSWOS come base, ampliandoli con un
registro mondiale condiviso, non sostituendoli con un motore parallelo.


## Regola — SWOS Expanded portable e salvataggi minimi (11 novembre 2026)
- Solo su Windows: **tutti** i dati scritti dal gioco nella cartella
  di SWOS_Expanded.exe: `save/` per .CAR, replay, tattiche e simili;
  `SWOS.ini` e `SWOS.log` accanto all'EXE. Non usare né ripiegare su
  Documenti/AppData. Installare in cartella scrivibile, errore visibile se
  impossibile scrivere. Non cambiare i target non-Windows.
- Savefile finale **il più piccolo possibile**: conservare soltanto delta,
  identificativi stabili, versioni dei data pack, semi deterministici
  e fatti persistenti irripetibili. Non duplicare database installati
  (`data/`, JSON/pack). Un download iniziale da 1,5 GB è accettabile.
  Questa è architettura APPROVATA, non interamente implementata.
- Decisione aggiornata 14 novembre 2026: **due slot autosave**, uno per
  mondo: `save/AUTO9697.CAR` e `save/AUTO2627.CAR`, ognuno sostituito
  atomicamente ai checkpoint sicuri. Il precedente `save/AUTOSAVE.CAR` è
  fonte di recupero compatibile in sola lettura, finché un nuovo checkpoint
  del mondo corrispondente riesce. Non convertirlo o cancellarlo da soli.
  CONTINUE considera il solo mondo attivo e verifica schema, kind e world.
  EXIT GAME e Title non cancellano; ritiro definitivo elimina soltanto
  il salvataggio attivo del proprio mondo; abbandono volontario andrà
  offerto come azione distinta e confermata.
- Per impedire di rigiocare un match serve il ripristino esatto dello
  stato live del motore OpenSWOS 50 Hz. Non salvare un mezzo stato né
  assegnare sconfitte arbitrarie per crash. Segnalare anti-save-scum
  durante il match come ANCORA DA IMPLEMENTARE.
- I vecchi save `Documents\SWOS\save` sono in cartella condivisa con
  OpenSWOS: l'utente ha autorizzato la cancellazione, ma MAI farla
  automaticamente dentro EXE/BAT avvio. Consentire una pulizia manuale
  delimitata a quelle estensioni, dopo conferma esplicita e test del
  nuovo autosave.


## Regole confermate 11/11/2026 — test pluridecennali e bilanciamento
- Il test principale del Living World deve coprire **almeno 70 stagioni complete**, senza click nei menu o partite da giocare manualmente: test isolato rapido e ripetibile via `TEST_LIVING_WORLD.bat`, con log nel gioco, controlli stagionali e save/reload. Non confondere questo harness con una prova end-to-end del manager OpenSWOS su Windows: entrambi i livelli serviranno.
- **PRIORITÀ ATTUALE:** terminare il Living World Engine prima del redesign TITLE, salvo fix che impediscono di giocare o salvare. Le azioni LOAD CAREER e NEW CAREER nel Title restano proposte differite, con conferma prima di sovrascrivere.
- **Parametri di simulazione:** dopo il completamento strutturale del motore discutere esplicitamente con l'utente probabilità, goal, forma, morale, infortuni, trasferimenti, economia, esoneri e distribuzioni. Non considerare approvati i placeholder e non introdurre nuove formule sportive senza discussione e autorizzazione. Questa fase interviene su calendario, persistenza e test, **non** calibra i valori.


## Decisioni 12/11/2026 — ordine conclusione Living World Engine
- **Prima dell'ultima fase di test**, come penultimo passo di costruzione, discutere con l'utente e approvare **tutti i dati iniziali e i valori numerici**: allenatori AI, chairman, calciatori, forma individuale, età/ritiri, opportunità offerte, crescita, mercato, finanze, probabilità risultati, infortuni, morale e condizione. Non introdurre statistiche o soglie provvisorie non concordate mentre si unifica l'architettura.
- Ordine corrente: 1) mondo avviato su START in NEW CAREER indipendentemente da assunzione, 2) unica fonte di calendari/risultati e completamento scheduler eventi, senza eliminare i callback OG necessari, 3) strutture dati persistenti per entità mondiali, 4) progettazione congiunta di tutti i valori come penultima fase, 5) test end-to-end e build obiettivo **0.14.00** dopo approvazione.
- La forma di squadra Expanded (0–100) e `_playerFormModifiers` nativo NON sono la stessa cosa di una nuova forma individuale persistente. Il campo originale `PlayerFile.form` contiene marcatori tecnici TRIALIST/RESERVE e non deve essere riusato senza un audit. Eventuali nuovi campi dinamici per giocatore saranno nel save, con valori iniziali concordati nel database/static pack.
- Per la prossima build il Title approvato non cambia: comando debug F12+N soltanto sul primo/secondo livello Title per aprire NEW CAREER nonostante `AUTOSAVE.CAR`. Conservare l'autosave precedente fino al **primo salvataggio riuscito della nuova carriera**. Se il setup è annullato, CONTINUE deve poterlo ricaricare. Non realizzare ora la UI finale LOAD CAREER/NEW CAREER.

## Conferma 12/11/2026 — presidenti e calendario
- [DECISIONE UTENTE] PRESIDENTI/CHAIRMAN NON anonimi nel design finale: ogni club avra' presidente con nome e eta', pensionamento e sostituzione automatica; nome ed eta' in CLUB OFFER e WELCOME. La dicitura OG `THE PRESIDENT` e' un placeholder corrente, NON una revoca del design approvato. Valori numerici (eta' al ritiro/distribuzioni) si decidono insieme nella penultima fase dati; non integrarli prematuramente.
- [ENGINE] Usare un singolo cursor cronologico giornaliero per gli eventi di lega del ledger mondiale. Preservare i callback OG di coppe e altre competizioni finche' il mondo non li copre realmente; non dichiarare unificazione completa soltanto perche' esiste un clock comune.

## 12/11/2026 — offerte e coppe: fonte unica, limiti dei collaudi
- Le offerte per allenatori disoccupati possono arrivare da qualunque campionato realmente importato nel Living World, indipendentemente dalla nazionalita' del manager. FORWARD TIME con offerta disponibile implica rifiuto implicito dell'offerta e prosecuzione del mondo; VIEW CLUB OFFER conserva ACCEPT/REJECT.
- Per le coppe OG (nazionali, internazionali e tornei a eliminazione), il sorteggio, gli spareggi, i replay e l'avanzamento dei turni devono restare gestiti dalle funzioni OpenSWOS originali. Expanded acquisisce i risultati reali e i vincitori in un registro persistente legato alla stessa data mondiale; non creare calendari paralleli inventati.
- Il test C di 70 stagioni verifica il registro cronologico/coppe con dati sintetici e save/reload, **non** dimostra che tutte le coppe mondiali vengano disputate durante disoccupazione o durante 70 stagioni del manager. Dichiarare l'integrazione globale completa solo dopo una prova end-to-end verificata su tutte le tipologie e gestione dei campionati/cup offscreen.

## Regola confermata 12/11/2026 — consegna test selezionabili
- Alla consegna di ogni nuova build di SWOS Expanded, fornire una checklist concreta di test Windows con risposta **SI / NO** per ogni voce. Il NO e' un risultato da riportare, non una bocciatura da occultare; aggiungere la possibilita' di annotare un difetto/log. Includere sempre versione, ramo, commit, cosa e' stato verificato automaticamente, cosa e' ancora da provare in Windows. Non confondere test sintetici con l'intero database reale.
- Il debug F12 puo' usare due colonne, colori e spaziatura per separare mondi, carriera, coppe, join e test senza alterare grafica e fisica dei menu. Finche' la carriera OG ha callback di coppa non presenti nel mondo, non eliminare l'eventuale catch-up: rendere esplicita la dipendenza e non spacciare un caricamento nascosto per unificazione completa.

## Regola 12/11/2026 — checklist inviabile
- Ogni build include checklist di prova Windows SÌ / NO / NON PROVATO con pulsante **Invia rapporto**, collegato a una vera azione di chat, ed eventualmente Copia.
- Patch esterne di altri autori con nuovi campionati, regole o rose: analizzare dopo il commit pertinente come proposta, senza importarle automaticamente prima di confronto con game design, codice e approvazione.

## Decisione 12/11/2026 — OpenSWOS gestisce le competizioni una volta sola
- Tutto ciò che l'engine originale gestisce (sorteggi, coppe, campionati, spareggi, rigori, replay e progressione) rimane di sua competenza. Il Living World coordina cronologia, identità e conseguenze persistenti. Ogni partita, risultato, classifica e trofeo ha una sola fonte autorevole; non creare simulatori o callback duplicati.
- Le coppe offscreen ancora non riusabili durante la disoccupazione vanno dichiarate PENDENTI fino a integrazione verificata. Non eliminare il recupero OG finché i suoi eventi storici non sono riproducibili senza perdite.
- Scorciatoie di risultato immediato sono solo debug: devono completare il percorso della partita originale e non toccare fisica/controlli normali.


### 12/11/2026 — prima offerta e copertura del test reale (decisioni)
- Prima offerta da disoccupato: un paese deve avere almeno 4 divisioni popolate nel mondo realmente schedulato, anche distribuite tra due file TEAM, e il club proviene dalla divisione piu' bassa disponibile (quando ci sono 8 divisioni si usa l'ottava). Il filtro sulle squadre in difficolta' si applica dopo aver verificato la classifica.
- Il BAT principale deve includere la prova 70 stagioni con i club domestici dei cataloghi reali e deve dichiarare separatamente nazionali, pool giocatori, squadre coppa ed eventuali funzionalita' non coperte. Nessun test diventa PASS se manca o fallisce una asserzione.

### 12/11/2026 — non modificare competizioni DIY standalone
- Le funzioni originali DIY (competizioni impostate liberamente), PRESET e CLASSIC MODES restano autonome e non devono scrivere nei registri/salvataggi Living World della carriera. Le funzioni C condivise possono rimanere comuni, ma gli hook Expanded devono operare solo con _gameType == GAME_TYPE_CAREER (4). Il flag _internationalAutoAdvanceFlag da solo non prova che si tratti di carriera.


### 13/11/2026 — carriera nazionale e idee segrete
- [DECISIONE CONFERMATA] Un allenatore può avere un SOLO contratto alla volta: CLUB oppure NAZIONALE. Escludere la contemporaneita'. Le offerte delle nazionali crescono con Fame, reputazione e trofei, da paesi piccoli fino a Brasile e altre grandi per allenatori Legend; rendimento della selezione, contratto, rinnovo, trattativa ed esonero seguono la struttura carriera, ma l'approvazione e' della FEDERAZIONE (non del board del club). Soglie numeriche Fame da decidere nella fase dati; non implementare valori arbitrari.
- [IDEA NON APPROVATA COME GAMEPLAY] Livelli segreti: allenatore oltre Legend al raggiungimento di un numero/tipo di trofei da definire, possibile livello stadio 6 "Maracana" da 120.000 posti con condizioni da definire, ed Easter egg. Non modificare rating, stadi, texture, capienze o effetti prima della scelta utente su nomi/soglie/condizioni.

### Diagnostica permanente (decisione del 13 novembre 2026)
Per ogni nuova funzione del motore o nuovo formato di competizione mantenere un audit dei DATI reali distinto dai test C isolati e dagli eventi della carriera Windows. I nuovi formati introdotti nel database partono NOT_TESTED fino a test funzionali significativi; non ereditano PASS da altri campionati. Per bug di campionato/coppie temporali/coppe, registrare ID, fase, data, risultato atteso/effettivo, primo errore, e preparare un BAT che raccolga i log della sessione quando possibile. Gli screenshot restano per anomalie visive o dettagli non disponibili nel log. Non pilotare mai i risultati.


## Decisione confermata 14 novembre 2026 — OG unico proprietario
- Per **entrambi** i mondi 1996/97 e 2026/27, il motore originale OpenSWOS è autorità di carriera, data, calendari, partite, risultati, competizioni e storico originale. Il LWE può **solo estendere** OG con finanze, trasferimenti, allenatori AI ed eventi aggiuntivi; non deve generare una seconda fonte autorevole di giornate, risultati o archivi. Questa decisione sostituisce le precedenti proposte di un secondo orologio civile LWE e di un calendario mondiale proprio.
- Non introdurre ora conversioni di date né ampliamenti archivi: verificare prima cosa conserva OG. Spostare il caricamento mondo al comando START solo dopo aver predisposto ripristino completo delle tabelle native/JSON in memoria senza riavvio, con scelta corretta del save per mondo e test Windows. Non considerare il refactoring delle inizializzazioni prova di integrazione completa.


## Regola confermata 14 novembre 2026 — commit unico e OG a START
- Per ogni intervento autorizzato preparare prima tutte le modifiche, controlli e documentazione; creare **uno e un solo commit finale**, mai una serie di commit correttivi salvo nuova autorizzazione.
- Dopo NEW CAREER -> START l'intero mondo OG e tutte le competizioni nazionali/internazionali devono essere inizializzati e avanzare indipendentemente dal club umano. START UNEMPLOYED non deve attivare un secondo simulatore; l'ingresso in club non deve creare calendari nuovi. Le classifiche dell'offerta devono provenire dai risultati originali OG per paese, divisione e stagione. Il codice attuale non certifica ancora l'intera fase START né il progresso OG per giornata: non dichiarare raggiunto l'obiettivo finché non verificato su Windows.


### 14/11/2026 — vincolo aggiuntivo all'audit Init/START/ingaggio
- Alla schermata LOADING possono essere preparati i DATI statici di 9697 e 2627, ma non installare simultaneamente due insiemi di pointer OG in `_competitionsTable`. Allo START scegliere e attivare una sola istanza mondiale OG, preservando entrambe le banche dati. Non chiamare routine che anticipino un'intera stagione solo per popolare le classifiche.
- `InitCareerWorld`, `InitCareerCompetitions`, `InitCareerManager`: nuova carriera una sola volta. `InitCareerVariables` non è un innocuo initializer: resetta `_currentSeasonShowing`, `_seasonPlaying`, squadra e game type. `InitializeNewSeason` NON può essere chiamato integralmente al reingaggio: muta contatori stagione, azzera slot e ricostruisce competizioni; dividerlo prima in fasi worldwide e club. `InitNewSeason` fa reset degli slot, non è un attach innocuo.
- All'atto della FIRMA, del CAMBIO CLUB o dell'ESONERO, il manager si collega/scollega, ma non ricrea la stagione OG o le classifiche; il tempo fa avanzare TUTTE le competizioni native progressivamente fra gli eventi, anche senza club umano. Stati per competizione persistenti e serializzabili; nessun secondo generatore LWE di risultati/calendari. Verificare il percorso OG, anziché dichiarare implementato dopo una sola chiamata ad `InitCareerCompetitions`.


### Regola 14/11/2026 — varianti Expanded delle routine OG
Se una funzione OG originale mescola inizializzazione del manager, club, stagione o simulazione e non e' adatta all'architettura approvata, sviluppare una variante Expanded **separando le responsabilita'**. Riutilizzare algoritmi OG per esito partita, fixture, coppe, spareggi e classifiche; niente secondo simulatore/autonomia LWE. Le varianti devono essere nominate chiaramente, documentate e sottoposte a test; non sostituire indiscriminatamente callback originali usate da CLASSIC/SEASON/DIY.
- A START devono nascere OG world e TUTTE le competizioni del mondo scelto, senza allenatore assegnato; FORWARD TIME avanza eventi per tutte e conserva contesti OG per competizione, salvabili; assunzione/esonero si limitano a collegare/scollegare il manager. Un refactor dei contatori e/o un log di inizializzazione NON dimostra che i campionati offscreen avanzino.


### 14/11/2026 — lifecycle OG e binding manager
Le competizioni OG non si ricreano alla firma; la routine Expanded di binding deve acquisire uno stato già esistente, validare i riferimenti delle squadre e NON simulare le partite storiche una seconda volta. Non modificare le routine OG originarie utilizzate in CLASSIC MODES salvo necessità comprovata. Per state non verificati (coppe, storici, serializzazione) NON dichiarare completata l'unificazione.

### 14/11/2026 — Finestre di mercato (decisione gameplay, NON ancora implementata)
- Acquisti e accordi possono essere conclusi anche fuori finestra: il trasferimento **effettivo della registrazione e della rosa** avviene all'apertura della prossima finestra applicabile, mai retroattivamente. Fino a quel momento il giocatore resta nel club d'origine. Gli **svincolati** sono ingaggiabili e utilizzabili tutto l'anno.
- La regola vale per manager e IA. Date per paese/lega, scadenze, salari/pagamenti e annullamenti restano da approvare. Non applicare date europee a tutti i mondi senza decisione.
- **PRIORITÀ OPERATIVA:** prima completare le competizioni OG mondiali a START, FORWARD TIME, VIEW WORLD read-only e salvataggio; mercato rinviato. La compilazione e il test Windows sono effettuati dall'utente dopo sincronizzazione GitKraken; non presentarli come eseguiti dall'assistente.

### 15/11/2026 — divieto callback invisibili in UNEMPLOYED
- Nel menu disoccupato (clone del Contest OG) sono invocabili SOLO FORWARD TIME, BACK, RECORD, DEBUG WORLD e VIEW CLUB OFFER quando l'offerta e' effettivamente presente. SQUAD, TRANSFERS, CLUB BUSINESS, NEXT MATCH, VIEW COMPETITIONS, pulsanti di stagione e frecce di calendario non sono azioni disoccupato: disabilitarne e azzerarne i callback, non solo nascondere la grafica.
- Ogni ritorno da un menu modale riesegue la whitelist del clone: RestorePreviousMenu() ri-esegue UnpackMenu() ma non l'onInit. MenuProc deve rifiutare selezioni stale o hitbox nascoste anche da mouse/joypad. Non cambiare il Contest originale o CLASSIC MODES.
- DEBUG WORLD deve essere un pulsante fisicamente visibile nel riquadro destro. La vista mondiale consulta stati OG esistenti; mai auto-generare risultati, né considerare zero partite al 1 luglio un errore senza controllare data/fase.

### 15/11/2026 — diagnostica OG residente e navigazione DEBUG WORLD
- F12 e i BAT principali DEVONO distinguere stati residenti OG, definizioni delle coppe presenti nei dati, callback di fine anno, assenza delle coppe residenti, save/ripresa non provati. Le suite sintetiche LWE archiviate restano test di componenti storici, NON pass dell'integrazione mondiale.
- `AVVIA_DIAGNOSTICA.bat` raccoglie il vero gameplay in `SWOS.log`; `ANALIZZA_SWOS_LOG.bat` usa la versione da `build_info.h`, non una stringa obsoleta. Il BAT `TEST_OG_WORLD.bat` distingue test C statici/dati da log Windows; ritorni 0=PASS specifico, 1=FAIL osservato, 2=INCOMPLETE. L'assenza delle coppe non puo' produrre PASS completo.
- Nel selettore paesi/competizioni aperto da DEBUG WORLD del menu UNEMPLOYED, BACK a profondita' >0 risale UNA pagina del selettore; soltanto BACK dal livello radice ritorna al menu UNEMPLOYED. Questa eccezione deve essere limitata al DEBUG WORLD disoccupato, non a CLASSIC o alla carriera con club.

### 15/11/2026 — stress 70 stagioni OG, futuro audit ritiri
- Test diagnostico F12+8 disponibile SOLO durante UNEMPLOYED: arma una progressione di 70 anni civili, eseguita dal successivo FORWARD TIME attraverso lo stesso `Expanded_OGWorld_AdvanceToDate` delle normali partite. Non usa il simulatore LWE, non fabbrica risultati. Per evitare risultati fittizi retrodatati, SOLTANTO nel test F12+8 si saltano i vecchi callback di coppa a fine anno (coppe OG residenti non ancora implementate).
- Il test cambia realmente lo stato e la data del mondo: si usa SOLO su nuova carriera di prova senza salvataggi importanti; non e' un test read-only, non autorizza cancellazioni automatiche.
- Ad ogni chiusura ed apertura annuale si registrano `[og-season-transition] CLOSE/OPEN` con veri contesti OG e match reali; il BAT `TEST_OG_70.bat` valuta 70 passaggi e data esatta tramite `SWOS.log`. Le pensioni dei giocatori e l'eta' manager (85) sono FUTURI controlli autonomi: un salto disoccupato non li dimostra. Le coppe residenti e il save globale restano NON PROVATO.

### 15/11/2026 — diagnostica 0.13.48
- F12+8 sul mondo OG: per evitare una singola chiamata di 70 anni, usare un anno nativo per iterazione del menu con data civile committata dopo ciascun anno. Non inventare incontri; non dichiarare risolto il crash finché il test Windows non arriva a FINISHED.
- Nei controlli cup, distinguere DEFINIZIONI OG da COPPE RESIDENTI. I contatori di goalStats[0] dei calciatori OG non certificano la UI TOP GOAL SCORERS. Le fixture sintetiche del solo parser non sono test del gioco.
- BACK del selettore DEBUG WORLD disoccupato via B o ESC/BACKSPACE: un livello per pressione. Il pad B non equivale a FIRE mentre quel selettore è attivo; CLASSIC e partita rimangono OG.
- VIEW COMPETITIONS da una offerta usa direttamente paese/divisione/anno dello stesso OG residente; nessuna seconda simulazione; controlli read-only prima/dopo.

### 15/11/2026 — resident OG national cups (0.13.50)
- National type-1 cup table slots 2 and 3 are initialized at START and advanced through the SAME dated OG phase loop as leagues, using InitDIYCupFromRecord plus Career_AutoAdvanceCupMatches and the native match function. Each context is keyed by country/slot/year, not a potentially shared compId.
- Never invoke Career_AutoAdvanceCupCompetitionFromRecord as a retrospective completed-year callback while resident OG is active; it creates an entire cup and can duplicate 70 years of results.
- Employed club cup slots are OG but not yet proven to bind to the resident world: exclude a cup containing the employed manager club from additional resident simulation and log MANAGER_SLOT_UNBOUND. This is NOT a complete join/sack sync and the world viewer is incomplete for that cup season.
- International/type-0 cups, permanent match histories, trophies, serializing all resident cup state and manager attachment still PENDING. No Windows gameplay claims before user tests.

### 15/11/2026 — precisazioni confermate per 0.13.51
- **NO secondo motore**, nemmeno per i marcatori: `PlayerFile.goalStats` degli stati OG residenti e' la sola sorgente sportiva. Le copie locali create da `InitTeams` dopo la firma sono UI/scratch, NON fonte autonoma per ripopolare le statistiche e NON devono azzerare il mondo. Qualsiasi aggiornamento dopo partite o cambi club deve restare OG e singolo.
- **VIEW COMPETITIONS da offerta**: mostra subito l'elenco OG delle competizioni del club nel paese gia' noto, senza selezione del paese; riutilizzare il picker DEBUG WORLD, non un menu duplicato. BACK risale **un solo livello** per pressione: viewer -> elenco -> offerta. Le coppe non residenti/adesioni non comprovate non vanno inventate.
- **Diagnostica unica**: `AVVIA_DIAGNOSTICA.bat` lancia compilazione/partita e poi tutte le suite automatizzate, log nuovi in `RISULTATI_DIAGNOSTICI/DIAGNOSTICA_SESSIONE_N`, storici in sottocartella distinta; no cancellazioni save. `TEST_LIVING_WORLD` alias della suite OG, nessuna seconda esecuzione. Audit coppe per paese+slot+anno, non solo il totale 101.
- Mantieni i limiti espliciti: coppe tipo1 nazionali slot2/3 e 70 passaggi di stagione OG non certificano tutte le coppe internazionali, i salvataggi del mondo, la carriera del manager o l'intero engine. Solo i test reali dell'utente su Windows possono confermare build/gameplay.

### 15/11/2026 — vincoli test 0.13.52 (seguire sempre)
- `AVVIA_DIAGNOSTICA.bat` e' l'unico punto di avvio di TUTTI i test automatici attuali e storici. Raccogliere output C, vecchi test LWE sintetici, audit runtime, compiler, errori boot e log nella nuova `RISULTATI_DIAGNOSTICI/DIAGNOSTICA_SESSIONE_N/` e ZIP automatico verificato. Non richiedere all'utente di lanciare separatamente BAT o test shell.
- Conservare prima il `SWOS.log` preesistente in `LOG_PRECEDENTI/`; dopo errore GCC dichiarare GAMEPLAY=NON PROVATO e non riutilizzare il log 0.13.50 per test 0.13.52. Non toccare save/*.CAR. Se lo ZIP fallisce, dichiararlo fallito e lasciare la cartella.
- Aggregatori e alias storici sono conteggiati nell'inventario ma non eseguiti due volte. I test LWE isolati NON sono un secondo motore del gioco e non provano campionati, marcatori o coppe OG. Test 70 anni F12+8 richiede un'azione dell'utente dentro la vera carriera Windows.
- Compilazione 0.13.51 non riuscita nel test utente: `teams.c:3622` implicit declaration `Expanded_UnemployedWorldPickerActive`; fix file scope presente nella 0.13.52, non ancora provato da GCC Windows. Report sempre differenziare test statici e vera compilazione/gameplay.

### 15/11/2026 — validazione finale 0.13.53 (decisioni utente)
- I check OG NON vanno addolciti: riconoscere bye, turni preliminari e qualificazioni native con i veri metadata `World_Cups` e il numero di gare OG, poi verificare risultati e avanzamento. `SELECTOR_UNDERFILLED` non e' automaticamente bug ma neppure PASS; mostrare per competizione la causa e l'evidenza. Non chiamare `WorldCups_Entrants` dall'audit (modifica seeds).
- Verificare separatamente coppe nazionali di club, tornei internazionali di club e nazionali con loro qualificazioni. La sola registrazione di una competizione non dimostra che sia residente o giocata. **Unico motore OG.**
- Dopo un load della carriera al 2096 data/anno osservati non dimostrano save completo: valutare resident `s_expOG`, `s_expOGCups`, match per fixture, ID, goalStats e RNG in entrambi i mondi, senza rigenerazioni inventate. Log post SAVE/RESTORE sono diagnostici e non entrano nel save.
- `AVVIA_DIAGNOSTICA.bat` resta unica procedura. Escludere dall'esecuzione attiva test legacy LWE obsoleti, conservandone fonti e la motivazione nel manifest. I warning 25 packed-member da GCC 0.13.52 non vanno soppressi con flag: prima analisi alignment sito per sito, specialmente serialize JSON/competizioni.

### 15/11/2026 — 0.13.54: validazione reale coppe OG con bye e visibilita' a START
- Non considerare `_numSelectedTeams < base.numTeams` automaticamente invalido: per una coppa OG `World_Cups()` puo' descrivere `n` partecipanti reali, `ties` partite di primo turno e bye per i restanti club; `WorldCups_Padded()` riempie soltanto gli slot tecnici del tabellone. Il filtro residente deve accettare il sottoinsieme **solo** se le identita' e il primo turno corrispondono esattamente alle regole OG, senza creare club, fixtures o risultati. Altrimenti segnala il vero problema.
- Tutte le coppe definite per ciascun paese devono essere visibili da NEW CAREER -> START, compreso il caso a zero partite, ma non dichiarare la disponibilita' completa senza runtime Windows. Il rapporto `OG_CUP_COUNTRIES` deve confrontare i paesi realmente auditati con i paesi definiti; NO falso PASS per un subset.
- Non confondere i 101 record club-domestic type1 con internazionali, nazionali, type0 e play-off; tenere lo scope onesto. Non correggere automaticamente `g_worldCups.seasons` soltanto perche' e' 0 dopo F12+8: e' necessario studiare i due hook OG di fine stagione per evitare duplicazioni, separato dal fix dei bye.
- Unico comando di test `AVVIA_DIAGNOSTICA.bat`, zip per sessione, user su Windows. 0.13.54 codice sorgente non equivale a build/test Windows; distinguere sempre SI/NO/NON PROVATO.

### 15/11/2026 - 0.13.55: audit OG, nessun falso PASS
- Sessione Windows 0.13.54 dell'utente: 70 passaggi fino al 2096, 134 leghe e 101 coppe domestiche residenti su 87 paesi senza fault del settore controllato; dichiarati anche due riavvii completi EXE con campionati/coppe/marcatori coerenti. Test manuale positivo; NON prova il salvataggio byte-per-byte di tutti gli stati OG e RNG.
- 0.13.55 introduce preflight compatibilita' JSON dopo autosave al TITLE (schema, kind e mondo), senza LOAD nella stessa carriera; il runner cattura copie read-only dei CAR esistenti nel ZIP e verifica identita' binaria, senza cambiare i salvataggi originali.
- Audit nominativo Champions/Europa/Conference o Coppa Campioni/Coppa delle Coppe/UEFA storiche, Libertadores, altre coppe di confederazione, Mondiali, Europei, tornei continentali nazionali e qualificazioni. I soli record e contatori OG NON attestano fixture reali: restano NOT_PROVEN.
- Dall'offerta consultare le stesse coppe domestiche OG residenti mostrate da DEBUG WORLD anche se il club non e' iscritto; membro=1 e non membro=0 non devono essere confusi con l'esistenza della coppa; contesto mancante=-1. Nessuna iscrizione o risultato inventato.
- L'assistente non compila/testa su Windows; 0.13.55 necessita la prova dell'utente dopo GitKraken con AVVIA_DIAGNOSTICA.bat. Non dichiarare completo tutto il motore: nazionali/internazionali residenti, save completo, primo manager e autosave mensile disoccupato restano separati.


### 16/11/2026 — World 2026/27 v4.1.2, offer preview, CT, safe cleanup
- Da NEW CAREER -> START tutte le competizioni disponibili nella Mod World 2026/27 v4.1.2 devono esistere nel mondo OG residente con nomi, ID, formato e calendario corretto, **senza calcolare risultati futuri a START**. La presenza di un record non certifica un fixture. Non sostituire OG con LWE o un secondo simulatore.
- **APPROVATO / NON ANCORA IMPLEMENTATO**: pulsante `INIZIA COME CT` che sceglie una nazionale iniziale nel mondo; riutilizzare l'intera carriera nazionale OG (convocazioni, gestione rosa, gare, qualificazioni, offerte successive, esonero). Nessuna data o stagione parallela. Nel 2026/27 aspettare l'arrivo reale dei contenuti 5.0 della Mod, senza inventarli.
- L'OFFERTA club apre lo stesso `_squadMenu` OG/Expanded usato dopo l'assunzione (stelle, statistiche), su copia di snapshot; nessun tasto gestionale visibile o attivabile. Preservare gli originali per CLASSIC/DIY/SEASON.
- Pulizia repository: `docs/` per guide e audit storici; AGENTS.md, PROJECT_STATE.md e IMPROVEMENTS.md rimangono in root per workflow. Licenze, BAT ufficiali e prerequisiti restano in root. `PULISCI_OBSOLETI_SWOS.bat` usa elenco esplicito, anteprima e conferma; esclude tutti gli stadi, salvataggi e file OG in uso. Nessuna cancellazione automatica durante BUILD_SWOS.sh.


### 17/11/2026 — attuazione controllata della decisione NO RESTART (0.13.67)
Il requisito 14/11 di preparare i dati statici al LOADING e attivare un solo mondo OG prima della carriera è applicato con snapshot in memoria, NON con un secondo processo o uno switch di una carriera già attiva. Il selettore in NEW CAREER ripristina banche statiche e compId e invalida la cache squadre; le scelte del manager e i salvataggi restano separati. Dettagli e limiti in `docs/WORLD_SWITCH_NO_RESTART_20261117.md`. La compilazione/test Windows resta NON PROVATA.
