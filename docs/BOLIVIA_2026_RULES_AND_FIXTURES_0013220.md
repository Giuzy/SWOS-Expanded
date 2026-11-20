> **RETROSPETTIVA 20/11/2026:** qui sono conservate le scoperte delle **00.13.220/221** (64 date autentiche e primo OG dispatch, REGOLE ancora problematica). La correzione definitiva delle sentinelle e della doppia indirezione, la fase KO 222/223, il viewer 224 e la competizione ricorrente 225 sono ricostruite, con SHA, cause e test, nel **[manuale Competition Engine](COMPETITION_ENGINE_IMPLEMENTATION_GUIDE.md)**. Non confondere le annotazioni «NON GIOCABILE» di questa build storica con quelle future; il manager e la qualificazione internazionale restano separatamente da verificare.

## AGGIORNAMENTO 20/11/2026 — 00.13.221, collegamento OG datato per fase a gironi; test Windows obbligatorio
Le 64 coppie e date ufficiali della fase a gironi restano nella tabella `bolivia_cup_sequence.h`, SENZA risultati importati. `BoliviaCup_ChronologicalFixtures` crea una permutazione per data civile (rinvii della fase 1/2 compresi), preservando `fixtureId` e le 64 coppie esistenti nel residente OG/vecchi CAR. La routine `Expanded_OGWorld_AdvanceBolivia` ora invoca `DIY_PlayOrSimulateLeagueFixture(0,0)` OG una volta per evento CPU scaduto, verificando il journal CUP e il contatore; blocca una gara del manager finche' non esiste il bridge manager/CPU OG (nessun risultato inventato). Totale 64 partite = 32 per sezione nativa x2; corretto l'offset del puntatore base di ciascuna riga e tutte le sentinelle TEXT_NO_PTR nella pagina Regole OG. Per il campionato la finestra nativa e' stata portata ad Apr–Dic come FONTI REALI, ma il singolo calendario OG non contiene ancora tutte le 240 date: collisioni future ancora da verificare, NON DARE PASS. Compilazione Windows 221, test gioco e salva/riprendi NON PROVATI.
Fonti: https://es.wikipedia.org/wiki/Primera_Divisi%C3%B3n_de_Bolivia_2026 ; https://es.wikipedia.org/wiki/Copa_Bolivia_2026 ; https://premiumsportsbo.com/vuelve-la-copa-bolivia-la-liga-entra-en-un-largo-parentesis/ .

# SWOS Expanded 00.13.220 — Copa Bolivia: regole, visibilità, calendario reale (intervento parziale)

## Evidenza e causa
Il test Windows dell'utente, build 00.13.219, conferma che la sola pagina REGOLE crasha (le altre schermate funzionano). Il log segnala eccezione 0xC0000005, lettura da -1. Nel menu generico type2, menu_data.c usa MENU_CMD_TEXT_NO_PTR, sentinella 0xFFFFFFFF, per le entry numeriche 11/13 e per le stringhe di fase. Tuttavia DesignDIYTournamentReinit -> DesignDIYTournament_UpdateTwoNumericFieldsText esegue Int2Ascii su quei puntatori PRIMA del callback OnDraw. DesignDIYTournamentOnDraw scrive inoltre StringCopy/PrintFormatted sugli altri puntatori sentinella. Il codice ora fornisce buffer statici scrivibili a numerici e sei descrizioni/qualifiche, solo per Copa Bolivia type2 world2627 in read-only. Nessuna modifica ai menu CLASSIC/DIY/SEASON. Conferma definitiva Windows 220 ancora NON PROVATA.

Un'altra correzione: BoliviaCup_OGPair puntava a &_diyCompetitionStart + n*18, ma le righe OG di 18 byte sono in _diyCompetitionStart.leagueStandings[n] (overlay relocato). Ora usa il membro vero. Non prova che le partite siano già in programma.

## Calendario storico 2026 / prima edizione
Sorgente pair/date: https://www.bolivia.com/futbol/liga-boliviana/copa-bolivia/resultados
La vecchia generazione round-robin era valida solo per le 64 combinazioni, ma non per il calendario REALE: San Antonio vs Aurora è del 18 agosto, non San Antonio vs Real Potosí. I 64 incontri aggiornati riportano home/away, le 8 giornate, 48 intra-gruppo +16 derby delle giornate 2 e 4 e le date reali dall'18 agosto all'8 ottobre, compresi i recuperi di una giornata giocati dopo la successiva (es. Nacional Potosí–Real Potosí del 22 settembre). Il C test isolato verifica 16 club, 4 home+4 away, matchday, unicità e date campione. Non importare mai risultati REALI nella carriera OG iniziata il 1 luglio.

## Regola globale di visibilità APPROVATA
Per TUTTE le competizioni e i due mondi: il viewer deve mostrare l'edizione corrente APPENA si conoscono i partecipanti OG, senza attendere cerimonie di sorteggio. Se gli iscritti dell'edizione nuova sono sconosciuti, mostra in sola lettura l'ULTIMA EDIZIONE PRECEDENTE OG davvero salvata, chiaramente etichettata con l'anno. Nessuna simulazione all'apertura, nessuna copia dei partecipanti dell'anno precedente come se fossero qualificati nella nuova stagione. Se manca un residente OG storico, segnalare indisponibilità. Questa regola è ora approvata/documentata, ma NON ancora implementata genericamente; nella 220 è rimosso solo l'attesa del 22 luglio della Bolivia già residente con i 16 club.

## Gate aperti: non dichiarare Bolivia completata
- Il solo calendario dati non avanza il motore OG; manca il dispatch delle 64 gare all'esatta data civile, una sola volta e in ordine cronologico reale (inclusi i rinvii).
- Manager/CPU devono giocare la medesima partita OG; l'originale slot manager Bolivia resta non collegato.
- Quarta fase: ottavi NON esistono; 8 ai quarti, due leg per QF/SF/F, 14 partite nativamente prodotte dall'OG, più campione/qualificati.
- Edizione 2027+ deve selezionare la prima divisione OG attuale (promosse dentro, retrocesse fuori) e sorteggiare senza riusare i 2026 fixture.
- SAVE/CAR: verificare esecuzione, cursor e record di match e cross-process CONTINUE; non sovrascrivere old CAR.
- Visibilità delle vecchie edizioni per TUTTE le competizioni ancora da collegare a una retention vera.

## Verifiche
SI: HEAD 49df539 controllato, 16 ID unici e 64 date/pair reali verificati con controllo statico indipendente; nessun risultato generato, nessun nuovo simulatore.
CODICE PREPARATO: buffer per REGOLE, puntatori righe OG, gate sorteggio rimosso.
NON PROVATO: Windows compile 00.13.220, RULES senza crash, SAVE/CONTINUE con gara, test anni multipli.
NO: 64 match giocati, manager, 14 KO, qualificazioni, 2027+, regola vista precedente globale.
