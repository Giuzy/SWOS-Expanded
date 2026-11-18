# 00.13.190 — Guardia fine campionato OG dopo ingresso del manager

18 novembre 2026. Ramo `dev/0.10`. Base `f4c191b881e23f118a178da7e77cec073ebc6eee`.

## Osservazioni dai quattro ZIP Windows della 00.13.189
- `DIAGNOSTICA_SESSIONE_2(20261118-175824).zip`: nel 1996/97, dopo `[og-bind]` (club 084b, divisione 3), residente a 552 gare in lega di 24 squadre e due giri; lo stato locale raggiunge erroneamente 553, seguito da `STALE_OR_DIVERGENT_LOCAL_BASE` e ripetuti `duplicate team fixture`.
- `DIAGNOSTICA_SESSIONE_4(5).zip`: nel 2026/27 stessa anomalia sul club 0852, divisione 3, con 552 partite residenti e 553 locali.
- Le sessioni 1 e 3 non dimostrano un crash all'ingaggio; attestano solo altri percorsi del test. Compilazione Windows precedente 00.13.189: nei log nessun errore/warning GCC esplicito; non trasferire tale esito alla nuova build.
- `Career_AutoAdvanceLeagueFixtures` bloccava l'ulteriore fixture per il solo esecutore mondiale `s_expOGCurrent`, non per la lega locale del manager. Il suo ramo `catch_up_phase` non interrompeva il ciclo sul `s_expOGBridgeBlocked` gia' protetto nel ciclo normale.

## Intervento C
- `expanded_og_league_bridge.h`: predicato puro con 2–4 giri: `n*(n-1)/2*cycles`; vale solo se la lega locale e il residente OG sono entrambe esattamente concluse.
- `Expanded_OGWorld_ManagerLeagueFinished` controlla club legato, identita' e formato del residente corretto, senza mutare lo stato.
- `career_status.c` interrompe l'auto-advance prima di tentare gare ulteriori; protegge anche il loop principale e il catch-up e chiude il retry infinito quando il bridge blocca una fixture.
- `diy_competition.c` rifiuta la chiamata diretta dopo il completamento, **prima** di `matchStarted`, dell'accesso alle squadre e di qualsiasi risultato.
- Il test isolato `test_expanded_og_league_bridge.c` copre 552/552, 553/552, 552/551, 1104/1104 e formati invalidi. Predicato separato verificato con GCC/Clang C11 `-Wall -Wextra -Werror -pedantic`, PASS senza warning. Non e' un test dell'intero gioco.
- Nessuna nuova simulazione, fixture, ID, qualificato, data o risultato. Nessuna modifica a formato CAR, file save preesistenti, seed, coppe o CLASSIC.

## Test da svolgere su Windows
1. GitKraken sincronizza solo `dev/0.10`, build ufficiale e controlla warning/errori della 00.13.190.
2. In entrambi i mondi, da UNEMPLOYED dopo la chiusura della lega accetta un'offerta: entra nella carriera senza freeze, senza gara 553 e senza valanga di `duplicate team fixture`.
3. Verifica che classifiche, calendario e risultati OG siano identici prima e dopo l'ingaggio; prova un ingaggio a stagione in corso.
4. Prova autosave e CONTINUE nei due mondi, senza cancellare o sovrascrivere CAR storici.
5. Controlla separatamente le qualificazioni e le edizioni successive delle coppe: la sola presenza dei residenti non dimostra il corretto passaggio stagionale.

**Stato:** modifiche e test isolato SI. Build Windows 00.13.190 NON PROVATO; gameplay, gestione errori runtime, save e qualificazioni tutte le coppe NON PROVATI.
