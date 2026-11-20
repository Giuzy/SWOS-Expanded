# SWOS Expanded — 20/11/2026, definizioni Bolivia/Argentina/Paraguay (NON runtime)

## Decisione e risultato

Richiesta utente: 4 gironi reali della Copa Bolivia 2026, date 2026 di Apertura/Clausura Argentina e Paraguay; unico commit su dev/0.10. I dati delle squadre boliviane sono mappati ai 16 ordinali effettivi del file `data_json/worlds/2627/teams/BOLIVIA.json`, senza duplicati. Le definizioni sono in:

- `data_json/worlds/2627/competitions/copa_bolivia_2026_groups.json`
- `data_json/worlds/2627/competitions/argentina_paraguay_2026_calendar.json`

**Stato: DATA DEFINITION ONLY, NON applicato al motore OG né ai CAR.** Non chiamare questa modifica un fix del viewer KO o una nuova build giocabile. Il codice attuale usa `kWorld2627_worldBolCup` come `CompetitionCup` type 1 (16 club, eliminazione diretta), mentre il formato reale necessita una `CompetitionTournament` type 2 (4 gruppi da 4 -> quarti -> semifinali -> finale) e giornate derby intergruppo; reinterpretare i byte del record sarebbe un errore. Le 16 squadre reali nel DB sono corrette; non sostituire il record type 1 con un type 2 senza migrazione controllata di resident, menu, scheduler, fixture, qualificati e SAVE/CONTINUE.

## Copa Bolivia 2026 — gruppi reali

| A | B | C | D |
|---|---|---|---|
| San Antonio Bulo Bulo | Always Ready | Guabirá | Bolívar |
| ABB | Real Oruro | Oriente Petrolero | GV San José |
| Aurora | Real Tomayapo | The Strongest | Blooming |
| Real Potosí | Nacional Potosí | FC Universitario | Independiente Petrolero |

La prima carriera comincia 01/07/2026 con zero gare di coppa e sorteggio storico del 22/07 ancora da applicare. L'edizione gioca da agosto a novembre/dicembre, con quarti, semifinali e finale in andata/ritorno; le giornate extra intergruppo e i singoli fixture vanno verificati prima del collegamento OG. Fonte FBF: https://fbf.com.bo/copa-pacena-2026/ ; gironi: https://erbol.com.bo/deporte/la-copa-bolivia-ya-tiene-hoja-de-ruta

## Argentina e Paraguay 2026 — date verificate e stato corretto al cutoff

| Nazione | Apertura 2026 | Clausura 2026 |
|---|---|---|
| Argentina | 22/01–24/05 (Belgrano campione 3-2 River) | **23/07–12/12** (finale) |
| Paraguay | **23/01–24/05** | **24/07–30/11** (aggiornamento APF; precedente previsione 29/11) |

Argentina: 30 squadre, due zone da 15, playoff + tabla anual; la classifica del girone Apertura deve venire dai risultati reali, non dall'ultima gara vinta. Paraguay: 12 squadre, 22 giornate per edizione + classifica annuale. **Al 1 luglio 2026 Apertura è chiuso, Clausura non è ancora iniziato**; manca l'import completo di punteggi, graduatorie e tabellone dell'Apertura. Fonti: AFA https://www.afa.com.ar/salarios/posts/el-torneo-clausura-de-la-liga-profesional-comenzara-este-jueves ; LPF https://www.ligaprofesional.ar/notas/institucional/2026/08/05/se-confirmaron-fechas-y-sedes-de-las-finales/ ; APF https://www.apf.org.py/noticias/la-espera-termino-arranca-el-clausura

**Perché non cambiare solo le fasce OG:** `kWorld2627_argentinianLeague` e `kWorld2627_paraguayanLeague` sono ciascuno un unico campionato OG type 0: il primo ha 30 club, due giri completi (58 gare per club), non corrispondenti ai due tornei argentini del 2026. Spostare begin/end a luglio-dicembre crea un calendario falso e può produrre errore in rollover. Occorrono due edizioni OG per anno civile, mantenendo un unico proprietario per gare e save e il titolo Apertura 2026 verificato. Non mutare i record caricati da salvataggi esistenti senza migrazione.

## Diagnostica e checklist

In questa chat sono arrivati solo i tre screenshot del viewer Copa Bolivia 217; nessuno ZIP/log Windows 217 è stato allegato. Uno screenshot mostra il vecchio `ROUND OF 16`; non è prova di uno stato tipo-2. 

| Controllo | Stato |
|---|---|
| 4 gruppi con 16 ID boliviani distinti | SI (dato statico) |
| Date AFA/APF e due edizioni esplicite nel JSON | SI (dato statico) |
| Partite reali pre-1/7 e tabella storica Apertura | NO |
| Copa Bolivia type-2 residente / gironi nel viewer | NO |
| Argentina/Paraguay edizioni OG separatamente datate | NO |
| Test Windows di compilazione/gameplay/CAR | NON PROVATO |

Prossimo codice: estendere una volta l'adapter OG residente per `type 2` nazionale e per due edizioni di lega per anno, con prova completa SAVE/CONTINUE e prevenzione doppi match. Non alterare classic/DIY/season.
