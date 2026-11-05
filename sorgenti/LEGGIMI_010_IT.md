# SWOS Expanded 0.10 — test stabilita Windows x64

## INSTALLAZIONE
Estrai lo ZIP in una cartella NUOVA e avvia `AVVIA_EXPANDED_010.bat`.
Non sovrascrivere 0.9, 0.6 o i salvataggi. SDL2.dll e i dati sono inclusi.

## CORREZIONI IMPLEMENTATE
- CLUB OVERVIEW: gli 8 riquadri sono ora hitbox trasparenti; il motore
  ripristina la trasparenza a ogni redraw e dopo il ritorno da una facility.
  Eliminati i riquadri blu opachi sopra la grafica e le finanze.
- CARRIERA: tolta la seconda scritta UNKNOWN in fondo sopra EXIT e nascosta
  la vecchia frase FINANCES/FANS. La nota rimane sotto il titolo CAREER.
- HALL OF FAME: eliminato il codice che nascondeva per errore LOAD OLD
  COMPETITION e altre voci del menu principale al ritorno.
- DISOCCUPAZIONE: la schermata ora clona il VERO menu contest/calendario
  originale di SWOS, con le stesse posizioni dei pulsanti e lo stesso
  pannello; sostituisce soltanto il testo della partita e l'azione centrale.
  Le azioni relative al club sono invisibili e non selezionabili.
- OFFERTA: VIEW SQUAD mostra i 16 calciatori del file originale del club;
  VIEW LEAGUE mostra i club della stessa divisione; BACK ritorna all'offerta.
  Prima di accettare viene validato l'ID della squadra nel file TEAM.
- DATA: formato giorno/settimana/mese/anno, partendo dal 1 agosto 1996;
  il calendario in carriera usa l'indice settimanale finanziario SWOS,
  quello da disoccupato usa il contatore FORWARD TIME.
- CLUB: data uguale a quella del calendario, stime settimanali, mensili e
  annuali delle entrate/uscite. Sono stime, non addebiti ripetuti.
- ESONERO: due parametri persistenti BOARD e FANS. Se entrambi molto bassi
  (board <=12 e fans <=20, oppure board <=5 e fans <=35) per tre partite
  valutate consecutive, scatta il flag di esonero del motore originale.
  Il contatore si azzera quando la situazione migliora.
- Testi nuovi del gioco in inglese, grafica originale pixel art SWOS.

## LIMITI IMPORTANTI (NON IMPLEMENTATI IN QUESTA VERSIONE)
- Le competizioni NON vengono ancora simulate dal motore originale durante
  la disoccupazione. Accettando un club, il motore della carriera parte ancora
  dall'inizio della stagione: NON e' un vero ingresso a campionato iniziato.
  Questo richiede una sincronizzazione piu' profonda dei calendari e dei
  risultati di tutti i campionati, da realizzare senza compromettere le partite.
- La data in carriera usa la settimana finanziaria disponibile; non e' ancora
  il calendario giornaliero ufficiale delle singole leghe.
- La nuova logica di esonero usa il flag SWOS; la schermata di comunicazione
  del board e' quella originale e richiede verifica sul PC Windows.
- Le statistiche della schermata VIEW LEAGUE sono per ora l'elenco dei club,
  non una classifica simulata.

## VERIFICHE
- Cross-compilazione Windows PE32+ x86-64: OK.
- Test nativo C per calendario gregoriano e doppia soglia di esonero: OK.
- ZIP verificato CRC e contenuti: OK.
- NON e' stato possibile eseguire interattivamente su Windows; non e'
  corretto dichiarare risolto il crash fino al test dell'utente.
  Se l'offerta continua a mandare in crash, invia SWOS.log e la squadra scelta.

## PROVENIENZA
OpenSWOS (Ben Baker e contributori, GPL), asset SWOS originali con diritti
ai rispettivi titolari, SDL2 secondo la sua licenza.
