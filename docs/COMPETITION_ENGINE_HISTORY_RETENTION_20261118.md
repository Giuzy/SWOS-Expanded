> **STORICO SUPERATO (decisione 19/11/2026):** la finestra di due stagioni dettagliate descritta sotto e' stata sostituita da **una sola stagione corrente dettagliata**, mantenendo tutti i dati necessari delle competizioni aperte e delle qualificazioni solo finche' necessarie; archivio storico minimo permanente. **Non ancora implementato nella 00.13.212.** Per il piano corrente vedere `docs/GAME_DESIGN.md` e il checkpoint Issue #1.\n\n## 18/11/2026 — 0.13.100: archiviazione per DATA DELLA GARA e test diagnostici
- Corretto il contratto di compattazione del journal: l'anno-stagione e' calcolato sul giorno effettivo della **singola gara**, NON sull'anno d'inizio dell'edizione. Una coppa iniziata a febbraio 2027 puo' avere partite sia nella stagione 2026/27 sia nella stagione 2027/28; queste ultime restano dettagliate fino al termine della finestra biennale.
- Prima di compattare: rifiutare risultati datati nel futuro, edizioni ancora in corso, conclusioni ancora future e fatti di qualificazione non confermati. Test aggiornato sulle partite di aprile/novembre e sulla protezione del calendario luglio-giugno.
- Il runner Windows diagnostico riconosce ed esegue due test isolati Competition Engine (scheduler e retention), riportando scope COMPETITION_ENGINE_ISOLATED, MA NON certifica gara reale, checkpoint CAR o gameplay.
- **OBIETTIVO VINCOLANTE DI QUESTO LAVORO:** poter aggiungere/importare competizioni OG, altre versioni SWOS e originali tramite definizione e regole parametrizzate, senza dover modificare codice specifico per ogni torneo; facilitarne diagnostica, calendario, qualificazioni, storico, salvataggi e nuove edizioni. Il collegamento OG e' un prerequisito, non il traguardo.
- Audit: nei residenti `Career_AutoAdvanceLeagueFixtures` e `Career_AutoAdvanceCupMatches` chiamano `DIY_PlayOrSimulateLeagueFixture` e `DIY_PlayCupMatchAndAdvance`, che gia' producono ed applicano risultati OG. In `Expanded_OGWorld_AdvanceUefa` il caso manager usa `PENDING_MANAGER_BIND` e NON deve essere sostituito con un secondo writer; per le coppe nazionali esiste `managed_excluded`. Nel binding campionato e' documentato il blocco quando locale/residente divergono. **Non esiste oggi un ID fixture comune verificato per entrambi i percorsi, ne' un CAR che checkpointi lo stesso risultato col nuovo scheduler**. Il gate 2d/2e/2f e quello della fase 3 restano APERTI; non attivare un nuovo dispatcher o millantare completamento.
- Stato consegna: modifica minima a codice isolato e runner, nessuna modifica ai callback OG, al formato CAR o al gameplay. Test Linux/Windows della nuova build e warning nuovi NON PROVATI fino a esecuzione. [Rapporto tecnico](COMPETITION_ENGINE_RECONCILIATION_GATE_013100_20261118.md).

# SWOS Expanded — Competition Engine: journal di due stagioni e archivio minimo
Data 18 novembre 2026 · base 0.13.98 · revisione codice 0.13.100 · ramo dev/0.10

## Decisione esplicita dell'utente (VINCOLANTE)
Il nuovo Competition Engine conserva i dati **gara per gara** soltanto per **due stagioni**:
1. stagione corrente (1 luglio – 30 giugno);
2. stagione precedente (1 luglio – 30 giugno).

I **dettagli delle singole gare** delle stagioni più vecchie vengono compattati solamente se l'edizione è chiusa e tutte le dipendenze sono protette. Per determinare la stagione di ogni gara vale la sua **data effettiva di svolgimento**, anche quando l'edizione attraversa il 1° luglio; una coppa non viene compattata solo perché iniziata in un anno precedente. L'archivio mantiene il riepilogo dell'edizione.

L'archivio storico minimo di ogni edizione contiene:
- ID stabile di mondo, competizione ed edizione;
- data di chiusura effettiva;
- vincitore (ClubKey), finalista se applicabile;
- numero complessivo delle gare disputate.

Non conservare nell'archivio storico vecchio: tabellini, risultati per-gara, marcatori, formazioni, infortuni per-fixture, date intermedie, classifiche giornaliere, calendario di tutte le gare, RNG copia per gara o rose complete. **Restano separatamente persistiti i fatti che servono ancora a generare edizioni successive**: qualificati, tabelle finali se sorgenti di posti, detentori, eventuali informazioni essenziali di licenza/regola. L'archivio minimo non è l'unica fonte di questi fatti.

## Implementation point 3, rev. 0.13.100
Nuovo `sorgenti/include/expanded_competition_history.h`:
- `ExpandedFixtureDetail`: ID fixture del punto 2, club reali, score GIÀ registrato nel ledger one-writer, giorno civile esatto, fingerprint opaco degli effetti OG. **Non simula partite**.
- `ExpandedEditionSummary`: ID edizione, giorno di chiusura, vincitore, finalista, numero gare, marker di fatti di qualificazione protetti. Il marker vale **solo** dopo effettiva persistenza durevole da parte del futuro chiamante.
- `ExpandedHistory_RecordPlayed`: accetta solo una partita `EXP_FIXTURE_PLAYED` esistente, rifiuta collisioni e riscritture, non inventa score né side effects; una summary archiviata fa da tombstone e impedisce il ritorno di una gara già eliminata.
- `ExpandedHistory_CloseEdition`: rifiuta il passaggio a chiusa se il ledger non contiene TUTTE le gare di quell'edizione come giocate e rappresentate nel journal; i dati vincitore/finalista vanno forniti dall'OG reale.
- `ExpandedHistory_FactsSecured`: contratto esplicito per dire che le dipendenze e i qualificati sono stati salvati; il codice NON verifica autonomamente un CAR.
- `ExpandedHistory_Compact`: due passi: preflight di tutte le entry eleggibili e poi compattazione; se una vecchia edizione è incompleta, non chiusa o non ha dati qualificazione salvati, NON elimina nulla. Non compatta un torneo cross-year solo perché è passato il 1 luglio, né risultati con data futura; conserva le partite della stagione precedente anche quando la loro edizione è iniziata due stagioni fa.
- `ExpandedHistory_ValidateRestore`: audit pure read-only dell'array dei dettagli, ID, ordinamento, campi e unicità summary. Non importa un CAR né tocca OG.

Test headless C `sorgenti/tests/test_expanded_competition_history.c` e runner `run_test_expanded_competition_history.sh`: richiedono compilazione con la libreria di date vera del progetto, testano deduplica, blocco per edizioni incomplete, persist facts, passaggio di luglio, coppa febbraio–novembre, 96/97 e 26/27, tombstone e validazione di una copia in memoria. Inseriti nella suite `run_test_engine_suite.sh`. Test scheduler cross-year aggiunti a `test_expanded_event_scheduler.c`.

## Necessario per completare davvero la fase 3 (NON IMPLEMENTATO)
Il nuovo modulo è solo un **contratto isolato**. La fase 3 architetturale **NON è ancora chiusa**: mancano il collegamento del registro fixture agli ID reali di tutte le competizioni, un adapter writer manager/CPU realmente unico, l'integrazione con lo scheduler civile in `Expanded_OGWorld_AdvanceToDate`, il checkpoint CAR atomico contenente events + results + OG effects + RNG + qualification facts, il restore senza replay e i test interprocesso. Collegare solo il journal ad OG senza queste garanzie creerebbe un rischio concreto di partite doppie e salvataggi incoerenti.

Non modificare qui né `career_status.c` e `save_format_json.c` né i CAR preesistenti per forzare una falsa chiusura. Il motore OG, la partita a 50 Hz, i due mondi, le modalità CLASSIC/DIY/SEASON, il viewer e gli asset rimangono invariati. Nessuna seconda autorità per gare/risultati viene attivata.

## Verifiche e protocollo Windows
| Controllo | Stato |
| --- | --- |
| Decisione archivio due stagioni registrata nei documenti | SI (sorgenti preparati) |
| Strutture dati, anti-duplica e compattazione condizionata implementate in codice isolato | SI (statico) |
| Test isolati con C reale compilati ed eseguiti in questa consegna | NON PROVATO |
| Compilazione completa Windows, 0 warning / 0 error | NON PROVATO |
| Journal collegato alle partite OG, manager e CPU | NO |
| CAR atomico con scheduler, journal e RNG, retrocompatibilità | NO |
| Ripresa cross-process 96/26, coppe cross-year 3 edizioni | NON PROVATO |
| Assenza duplicazioni e integrità gameplay OG dopo CONTINUE | NON PROVATO |

Per il test Windows successivo: sincronizzare GitKraken, compilare con `diagnostica/COMPILA_SWOS.bat`, leggere integralmente `diagnostica/log/COMPILAZIONE_0140.log` e segnalare ogni warning/error. Non modificare o cancellare `save/AUTO9697.CAR`, `save/AUTO2627.CAR` e `save/AUTOSAVE.CAR` originali; usare backup separati e salvataggi di test. La nuova regola non cambia ancora il formato dei CAR esistenti.
