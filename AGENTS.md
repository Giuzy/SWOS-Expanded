# SWOS Expanded — AGENTS.md

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
- Non riprogettare schermate approvate senza richiesta.
- Evitare pannelli, cornici, ombre e decorazioni superflue.
- Mantenere testi, numeri e pulsanti leggibili e correttamente allineati.

### Sfondi

**Non aggiungere immagini di sfondo alle nuove interfacce, ai menu o alle anteprime se l'utente non le richiede esplicitamente.**

Le schermate storiche con sfondi già approvati mantengono il proprio design.

Quando viene richiesto uno sfondo illustrato:
- Crearlo come asset separato dagli elementi interattivi.
- Non incorporare pulsanti, valori dinamici o barre di caricamento nell'immagine.
- Creare la UI separatamente senza sfondo per consentire il riposizionamento indipendente degli elementi UI.

### Schermate approvate

- Preservare il layout BUSINESS approvato.
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

Ogni nuova schermata deve essere verificata anche dal punto di vista della navigazione.

## 6. Rendering e compatibilità

Piattaforma principale: Windows 10/11 x64.

- Mantenere SDL2 e la base OpenSWOS.
- Supportare 3840×2160 a schermo intero.
- **Durante lo sviluppo**, il launcher Windows `AVVIA_SWOS.bat` deve aprire il gioco in una **vera finestra massimizzata**, grande quanto l'area di lavoro del monitor, mantenendo barra del titolo, barra delle applicazioni e normale Alt+Tab. Non usare il fullscreen senza bordi al posto della finestra.
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

Workflow Windows approvato:

- GitHub contiene il codice sorgente.
- L'assistente prepara e registra le modifiche.
- L'utente aggiorna il repository locale tramite GitKraken.
- L'utente compila ed esegue il gioco sul proprio PC.
- L'utente comunica i risultati dei test.

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

GitHub è l'archivio di riferimento per gli asset del progetto.

### Formato obbligatorio fin dall'inizio: BMP

- **Tutti i nuovi asset grafici di SWOS Expanded devono essere creati in BMP fin dall'inizio del nostro flusso di lavoro e non devono essere convertiti in BMP alla fine.**
- Il BMP è il file di produzione da creare, verificare, approvare e caricare su GitHub. Non trattare PNG o altri formati come asset finali approvati da convertire solo al momento dell'implementazione.
- Prima dell'archiviazione verificare risoluzione, profondità colore, compatibilità con il loader del gioco e conservazione dei dettagli pixel art; seguire le dimensioni native e non cambiare le coordinate dei testi/valori dinamici senza richiesta.
- **Perché:** evitare conversioni tardive, caricamenti duplicati, disallineamenti, confusione tra versioni e lavoro ripetuto durante l'integrazione.
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

Quando l'utente scrive **(regola)**, registrare la decisione e aggiornare `AGENTS.md` al successivo commit esplicitamente autorizzato; non creare commit automaticamente.

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

## Gestione autonoma degli asset GitHub

- Quando vengono creati, approvati o modificati asset per SWOS Expanded, l'assistente deve caricarli personalmente nel repository GitHub, nella cartella coerente con la schermata o funzione, e committarli solo su richiesta esplicita. Non delegare all'utente il caricamento manuale.
- Per i file binari usare gli strumenti GitHub per blob Base64, tree e commit quando disponibili; verificare che i file risultino effettivamente presenti sul ramo prima di dichiarare il caricamento completato.
- Se un impedimento tecnico concreto blocca il trasferimento, comunicarlo esplicitamente senza dichiarare il lavoro completato.

## Numerazione build e release (regola utente, 2026-11-06)
- Conservare il formato originale a tre parti `MAJOR.RELEASE.BUILD`, ad esempio `0.10.40`; non abbreviare in `0.40`.
- Ogni nuova build esplicitamente richiesta incrementa soltanto l'ultima parte: `0.10.51` -> `0.10.52`.
- Una nuova release importante incrementa la parte centrale e azzera l'ultima: la prossima release importante dopo la serie `0.10.xx` sarà `0.20.00`; le modifiche successive `0.20.01`, `0.20.02`, ecc.
- Usare normalmente **due cifre** per entrambi i contatori RELEASE e BUILD. Aggiungere una terza cifra **solo dopo 99**: piccole modifiche `0.10.98` → `0.10.99` → `0.10.100` → `0.10.101`; release importanti `0.98.00` → `0.99.00` → `0.100.00` → `0.101.00`. Non concatenare le cifre di incrementi diversi (quindi niente `0.10.410` o `0.10.411` dopo `0.10.49`).
- Il numero della build deve comparire nella schermata del titolo al posto della scritta gialla attuale, senza alterare lo sfondo. La rimozione di `DrawMenuText` in `menu_system.c` non ha eliminato quella scritta: identificare prima la fonte reale.

## Commit intermedi e build su richiesta (regola utente, 2026-11-06)
- Non effettuare mai commit o push senza richiesta esplicita dell'utente. Le richieste di modifica da sole non autorizzano commit.
- Ogni commit intermedio autorizzato che modifica il gioco conserva la build base e incrementa il suffisso progressivo: `0.10.51_01`, `0.10.51_02`, ..., `0.10.51_99`, `0.10.51_100`.
- Il suffisso conta i commit intermedi di codice della serie, non tutti i commit del repository; commit solo documentali non devono necessariamente avanzarlo.
- Quando l'utente chiede esplicitamente una nuova build, eliminare il suffisso e incrementare BUILD una sola volta: `0.10.51_03` -> `0.10.52`. Registrare la nuova build con commit autorizzato.
- Il titolo deve mostrare `Build ` seguito dalla versione completa (incluso il suffisso), tramite `Build_DisplayVersion()`.
- Aggiornare `SWOS_EXPANDED_VERSION` in `sorgenti/include/build_info.h` nello stesso commit di codice che avanza il suffisso o pubblica la build; l'hash Git identifica il commit esatto.
- GitHub richiede un commit per salvare cambi persistenti sul branch; si possono preparare modifiche localmente senza commit.
- Poiché la build corrente è `0.10.51`, il primo commit intermedio di codice è `0.10.51_01`. `0.10.50_01` è valido solo se la base è `0.10.50`.

## Convenzione dei branch (decisione utente, 2026-11-06)
- Il lavoro ordinario della serie `0.10.xx` prosegue sul branch `dev/0.10`, senza rinominarlo a ogni piccola build.
- Il numero esatto della build resta in `sorgenti/include/build_info.h` e compare automaticamente nel titolo.
- Per una release importante si prepara un branch `release/0.20` e la versione `0.20.00`; la stessa convenzione si applica alle serie successive.
- Il vecchio branch `release/0.10.40` resta temporaneamente disponibile come riferimento per permettere la sincronizzazione sicura di GitKraken; non cancellarlo prima della verifica locale.

## Cartella unica di ingresso asset — ASSETS_INBOX (regola utente, 2026-11-06)

- La cartella permanente `ASSETS_INBOX/`, alla radice del repository, è l'unico punto di deposito manuale per nuovi asset (PNG, sprite, animazioni e altri file).
- L'utente copia i nuovi file in `ASSETS_INBOX/` nel repository locale e li pubblica sul ramo di lavoro verificato tramite Commit e Push (attualmente `dev/0.10`).
- L'utente autorizza permanentemente l'assistente a organizzare e committare, senza ulteriore conferma, i nuovi file trovati in `ASSETS_INBOX/` dopo un push segnalato dall'utente o durante una successiva sessione di lavoro sul repository. L'assistente verifica i file su GitHub, individua le cartelle definitive appropriate e li sposta con un commit separato. Non assumere che ogni push venga rilevato automaticamente fuori dalla conversazione.
- Durante lo spostamento preservare esattamente il contenuto binario e il nome degli asset approvati, salvo diversa istruzione; non sovrascrivere asset esistenti senza verifica e consenso.
- Non integrare automaticamente gli asset nel gioco soltanto perché sono stati archiviati.
- Mantenere `ASSETS_INBOX/.gitkeep` nel repository affinché la cartella rimanga tracciata anche quando non contiene altri file.
- Questa procedura è un'alternativa pratica al caricamento diretto tramite strumenti GitHub quando il trasferimento binario non è possibile; preferire il caricamento diretto quando effettivamente disponibile.
