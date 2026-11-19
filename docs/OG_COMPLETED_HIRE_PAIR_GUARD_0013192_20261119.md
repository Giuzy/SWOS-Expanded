# 00.13.192 — Ingresso manager a stagione conclusa e coppia OG/keycache

**Data:** 19 novembre 2026. **Ramo:** dev/0.10. **Parent verificato:** efcf0bae2adeb667c5ad3ac96955c51f88ebe1e3 (00.13.191). **Ambito autorizzato:** 4 interventi da utente, unico commit richiesto.

## Fonti Windows originali esaminate (00.13.191)

- DIAGNOSTICA_SESSIONE_1(20261119-072627).zip: world 2026/27, bind team 3333, divisione 4, match OG=90, data 2027-06-30; PositionClubCalendar segnalava \`no future event year=2026 phase=144\`, ma la sincronizzazione continuava a considerare come prossimo evento del calendario nativo una data 2026. Ingresso non allineato.
- DIAGNOSTICA_SESSIONE_2(20261119-072628).zip: world 1996/97, bind 2c14 ottobre 1996, prossimo evento phase=80, index=25/27; il normale ingresso prima della fine del calendario non presenta la stessa ripetizione.
- DIAGNOSTICA_SESSIONE_3(20261119-072629).zip: world 1996/97, bind team 3327, divisione 3, OG 132 incontri, phase=139; \`no future event\`, poi **161.420** righe \`REJECT_EXTRA_FIXTURE played=132\` senza uscita dal menu. Prima della chiusura, una vera partita OG manager phase=80, home=2c14, away=2c1e, score=2-0, campionato residente aggiornato a 129 gare. Nella cache nativa del manager, alla STESSA fase, esiste invece home=2c14, away=2c24 e score=2-0 con flag 0x82: \`CALENDAR_PAIR_MISMATCH\` reale. Score e tabelle OG erano coerenti, la coppia della cache no.
- Log compilazione 00.13.191: nessun GCC \`warning:\` o \`error:\` esplicito. Il fatto non certifica questa nuova versione.

## Modifiche

1. **Calendario a contratto finito.** \`expanded_og_calendar_plan\` conserva l'offset dell'ULTIMO elemento valido anche quando non trova eventi futuri. \`PositionClubCalendar\` restituisce 1 se posizionato sul primo evento futuro, 2 se OG completa **esattamente** entrambe le copie residente/locale, 0 altrimenti (no falsi successi). Solo nel caso 2 collega l'indice all'ultimo header valido, imposta i flag OG di stagione terminata e prepara il riepilogo tramite \`Career_FillCurrentSeasonInfo\`; non cambia data, RNG, roster, risultati o competizioni. \`InitCareerClub\` riconosce l'esito 2 senza dire che il calendario e' corrotto. La transizione resta il VERO \`FINISH SEASON\` del giocatore; il codice non la esegue automaticamente.
2. **Stop della gara oltre fine lega.** \`Career_AutoAdvanceLeagueFixtures\` esce PRIMA di dereferenziare il vecchio cursore, segnando la schermata DIY completa. \`DIY_PlayOrSimulateLeagueFixture\` continua a rifiutare la gara N+1, ma registra UN SOLO log per club/anno e non lascia il menu nel percorso di match attivo. \`DIYLeagueTournamentMatchCommon\` aggiorna focus e disponibilita' dei comandi per offrire EXIT/FINISH invece di riprovare. \`Expanded_MidseasonJoin_Begin\` non avvia l'autopilot dei risultati storici quando il manager e' gia' collegato a una lega OG realmente completa; in questo caso nessuna azione storica viene ripetuta.
3. **F12 CALENDARIO CLUB.** Query read-only del prossimo indice/phase con validazione dei 8192 byte di keycache; \`FUTURE\`, \`COMPLETE\` (richiede residente OG verificato), \`BEHIND\`, \`INVALID\`, \`N/D\`. COMPLETE mostra \`FINITA - PROSSIMO FINISH SEASON\` in PARZIALE, non un PASS universale. Nessun renderer/PNG aggiuntivo, il font esistente 5x7 resta identico. La riga F12 \`PAIR OG/CACHE\` mostra check/rebound/mismatch; limiti coppe ora sono riportati sulla stessa riga \`COPPE NAZ OG\`.
4. **CALENDAR_PAIR_MISMATCH, correzione LIMITATA.** Dopo che il risultato e' stato realmente calcolato da OG e committato nel residente, una singola riga keycache gia' marcata GIOCATA puo' correggere i soli quattro byte degli ID casa/trasferta. Richiede *contemporaneamente*: stesso manager, slot, fase OG nativa e fase cache, una sola riga nella fase (nessuna ambiguita'), score OG gia' IDENTICO a quello salvato nella cache, contatori e classifiche OG locale/residente coerenti, bridge non bloccato, campo partecipanti originari diverso e coppia nativa ancora assente in quella fase. Vengono conservati integralmente punteggio, flags, rigori, ordine/strides cache, OG, journal, classifica, RNG e salvataggi precedenti. Se manca UNA prova, \`BLOCK_PAIR_RECONCILIATION\` e resta \`CALENDAR_PAIR_MISMATCH\`; nessuna mutazione. Un vecchio CAR con una coppia gia' errata NON viene riparato retroattivamente all'apertura.
5. **Analizzatore**: distingue \`PARTIAL_CACHE_IDS_REBOUND_NEEDS_WINDOWS_CHECK\` da \`FAIL_REAL_PAIR_MISMATCH\`; mostra \`OG_COMPLETED_HIRE_CALENDAR\` con conteggi di bind completi e handoff osservati. Non forza PASS sui log. I nuovi contatori F12 sono transienti e non serializzati.

## Test in repository

- \`sorgenti/tests/test_expanded_midseason_join.c\`: nuovo predicato puro di calendario per fine verificate, cache incompleta, arretrata e indice finale, inclusa la compatibilita' con sentinel index==count.
- \`sorgenti/tests/test_expanded_og_cache_match.c\`: caso reale 2c14-2c24 -> 2c14-2c1e, mantenendo TUTTI i byte eccetto le 4 identita' e prova di idempotenza; rifiuto con score diverso, manager estraneo, fase ambigua, fase errata, record corrotto. Runner gia' presente in \`run_all_diagnostic_tests.sh\`.
- In questa consegna i file sono stati verificati staticamente e aggiornati su GitHub, **non** compilati/eseguiti nella toolchain Windows. Non considerare PASS tali test solo perche' esistono nel repo.

## Checklist di verifica (da effettuare sul PC Windows)

- [ ] COMPILA_SWOS.bat 00.13.192: 0 errori, correggere eventuali warning GCC prima del prossimo intervento
- [ ] 2026/27: da disoccupato fino al 30 giugno, accetta offerta; niente freeze, F12 \`CALENDARIO CLUB\` = FINITA, segue FINISH SEASON originale
- [ ] 1996/97: da disoccupato fino a giugno, accetta offerta; niente 161k rifiuti / crash, stagione precedente intatta
- [ ] 1996/97 ingaggio ottobre e 2026/27 ingaggio in stagione: F12 \`CALENDARIO CLUB\` = FUTURE senza salto eventi
- [ ] Partita manager con cache non allineata: verifica \`REBOUND_EXACT_OG\` solo dove la prova e' esatta, oppure \`BLOCK_PAIR_RECONCILIATION\` senza risultati alterati
- [ ] Nelle due carriere: F12 PRE/POST ingaggio invariato, coppe invarianti, SAVE/CONTINUE, CAR vecchi preservati
- [ ] Screenshot F9+F12 e ZIP sessioni 1/2/3; verificare anche passando il 1 luglio

**Limiti:** non e' stato realizzato un resolver universale per tutti i calendari OG/split/coppe, ne' provato il passaggio qualificazioni per tutte le competizioni. Un caso non esattamente riconciliabile continua a essere segnalato. Il precedente crash e il blocco dovranno essere confermati risolti dal test Windows dell'utente.

Stato: modifiche codice **SI**; documentazione **SI**; commit dopo autorizzazione **SI al termine**; build Windows/gameplay 00.13.192 **NON PROVATO**; compatibilita' completa nuovi/vecchi CAR **NON PROVATO**.
