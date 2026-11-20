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
