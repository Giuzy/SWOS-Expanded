# Competition Engine — punto 2, base C 0.13.88

18 novembre 2026. Ramo dev/0.10. Parent controllato: 1fcdfeef53226ccc06b3ef9b07900bb702c7ef93.
STATO: corretto routing/caching TeamDb; definite chiavi e un contratto di claim delle fixture in un nuovo header. Non esistono ancora fixture gestite dal nuovo contratto nel gioco e non sono stati modificati né il CAR né il motore di simulazione.

## 1. Bug Francia/Turchia: correzione sorgente
- Nuovo sorgenti/include/team_source.h: funzione pure-C TeamSource_HistoricCareerPath, usa data/9697/TEAM.012/.013/.038/.073 soltanto per WORLD_9697 + GAME_TYPE_CAREER=4. Nessuna modifica dei file binari. CLASSIC/DIY/SEASON restano sui generici.
- teams.c::LoadTeamFile riutilizza l'helper lasciando inalterati LoadFileEx, fallback, endian conversion, TeamPool e impatto delle leghe.
- team_db.c::read_country applica la stessa scelta sorgente, controlla esistenza con swos_data_path/fopen, verifica count/dimensioni e ricade sul file generico se storico assente/non valido. Non invoca LoadTeamFile da una query read-only, evitando mutazioni globali.
- TeamDb ora usa una cache s_country[WORLD_COUNT][2][256], per mondo, game-mode (CAREER oppure altro) e paese. Le entry già restituite restano allocate quando si cambia mondo, evitando puntatori dangling. Non è previsto il cambio di asset sul disco durante lo stesso processo.
- Premessa verificata in fase1: Francia generica 18 vs storica 20 club di serie A (42 record); Turchia generica 17 vs storica 18 (24 record). Anche i nomi associati ad alcuni ordinali sono diversi. Correggere la fonte NON è prova che i menu e i save Windows siano già senza errori.

## 2. Identità e singola presa in carico: contratti pronti, NON attivi nel mondo
- Nuovo sorgenti/include/expanded_competition_identity.h: ExpandedClubKey(world, stable_club), ExpandedCompetitionKey(world, stable_competition), ExpandedEditionKey(competition,start_day,sequence), ExpandedFixtureKey(edition,stage,tie,leg,replay). I valori stable sono responsabilità di un futuro registro world-pack, non dei checksum/UUID TeamDb né dei compId byte OG.
- ExpandedFixtureLedger è fornito dal chiamante come buffer e conta le fixture. Schedule e Begin distinguono una sola presa in carico; Finish scrive il risultato una volta e restituisce ALREADY_PLAYED per replica identica o CONFLICT per risultato diverso. ReleaseUnchanged è lecito solo se OG non ha prodotto effetti o il rollback è completo.
- Non è un generatore gare, non crea doppio orologio o LWE, non chiama OG, non gestisce il manager, non salva CAR. È deliberatamente scollegato dal gameplay finché claim, effetti OG, seed RNG e save non saranno una transazione unica.
- Aggiunto sorgenti/tests/test_expanded_competition_identity.c: verifica path 12/13/38/73, mondi/modi, overflow, distinzione mondo/edizione/leg/replay, doppio claim, risultato identico e conflitto. Un harness equivalente del solo header con tipi mock è stato compilato qui con cc -std=c11 -Wall -Wextra -Werror -pedantic: POINT2_ISOLATED_CONTRACT=PASS. Non è la compilazione del progetto né un test Windows del nuovo test GitHub.

## TODO canonica rimasta
- [x] 2a Unificazione della regola sorgente storica nel loader e in TeamDb.
- [x] 2b Cache TeamDb per mondo e modalità senza liberazione di puntatori in uso.
- [x] 2c Definizioni di chiave fixture e contratto header-only con test C isolato.
- [ ] 2d Registro vero dei ClubKey/CompetitionKey nel world-pack e mapping da team OG, senza riciclare ID se cambia club.
- [ ] 2e Collegare manager e AI allo STESSO writer per una sola competizione migrata, rimuovendo il vecchio writer SOLO per quella competizione.
- [ ] 2f Claim+risultato+effetti+RNG salvati atomici nel CAR e ripresi senza replay: dipende dal punto3.
- [ ] 2g Test Windows della cache RNC, 9697→2627→9697, CLASSIC/DIY/SEASON, CUSTOM, salvataggi preesistenti.
- [ ] 2h Golden OG/adapter e test duplicazione in gioco. Non chiamare il punto 2 gameplay COMPLETO prima di questi gate.

## QA/Checklist
| Controllo | Stato |
|---|---|
| Correzione fonte e separazione cache implementate | SI (sorgenti) |
| Contratto identità e claim presente | SI (sorgenti) |
| Harness del solo contratto | SI (isolato con mock) |
| Test compilazione SWOS_Expanded.exe Windows | NON PROVATO |
| Test TeamDb effettivo e modalità CLASSIC | NON PROVATO |
| Singolo writer attivo, CAR/CONTINUE, tre edizioni | NO |

Decisione B (formati storici evolutivi) rimane approvata ma POSTICIPATA dopo engine e competizioni basiche. Il prossimo passo è verificare Windows 0.13.88, poi realizzare registry e persistenza senza duplicare il vecchio LWE.
