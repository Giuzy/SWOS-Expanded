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

- Utilizzare uno stile pixel art coerente con SWOS e con le schermate già approvate.
- Preferire il font originale SWOS.
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

Quando l'utente scrive **(regola)**, interpretare il testo associato come una nuova regola permanente esplicita: aggiornare sempre `AGENTS.md` e creare un commit dedicato sul ramo di lavoro, segnalando con trasparenza eventuali problemi tecnici che impediscano il commit.

Quando emerge una nuova regola permanente:

1. Verificare che sia una decisione effettivamente confermata.
2. Controllare eventuali contraddizioni con le regole precedenti.
3. Aggiornare `AGENTS.md` con il consenso dell'utente.
4. Creare un commit dedicato e descrittivo.

Non modificare questo documento autonomamente soltanto perché una soluzione alternativa sembra migliore.

## 13. Correzione ortografica

- Correggere sempre gli errori di ortografia e i refusi nei testi, nella documentazione e nei nomi di nuovi asset/file.
- Se un nome errato è già usato nel codice o nei dati, aggiornare coerentemente i riferimenti prima di rinominarlo, per evitare regressioni.
- Quando una correzione cambia un nome già concordato, usare da quel momento la grafia corretta.

## 14. Regola fondamentale

**SWOS Expanded è un progetto condiviso: l'utente decide la direzione del gioco, l'assistente contribuisce alla progettazione e all'implementazione.**

L'assistente deve preservare il lavoro approvato, distinguere i fatti dalle ipotesi e chiedere chiarimenti prima di prendere decisioni importanti.
## 15. Nome dell'eseguibile Windows

- Il file eseguibile del gioco deve chiamarsi esattamente `SWOS_Expanded.exe` in tutte le build Windows future.
- Mantenere coerenti il nome prodotto dalla compilazione, i percorsi e i riferimenti negli script di avvio e build (inclusi BAT e shell script), senza rinominare il repository o altri file non necessari.
- Verificare la compilazione e l'avvio su Windows prima di dichiarare applicata la rinomina al gioco.
