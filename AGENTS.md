# SWOS Expanded — AGENTS.md

## 0. Regole fondamentali ad alta priorità — workflow Codex (2026-11-09)

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
- Mantenere il supporto per mouse e tastiera.

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
- Un solo `save/AUTOSAVE.CAR` duraturo, sostituzione atomica a checkpoint
  sicuri e ripristino dopo chiusura/restart. EXIT GAME e ritorno al Title
  non cancellano. Ritiro definitivo elimina lo slot; abbandono volontario
  andrà offerto come azione DISTINTA e confermata.
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
