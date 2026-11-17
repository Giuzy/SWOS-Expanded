# F12 — pannello unico mondo OG (codice 0.13.70_16)

## 0.13.70_17 — presentazione OTHER
- COPPE INT e NAZIONALI sono visualizzate sotto la sezione OTHER, senza cambiare il denominatore per competizione. Coordinate header y=458, righe 477 e 500, due colonne. Non introdotte nuove categorie sportive.


## Perimetro autorizzato
- Sostituire solo l'overlay SDL F12. Rimuovere le pagine secondarie F12+C e l'ispettore F10 dei dati (F10 originale dell'Arcade intatto); F9 continua a fotografare il frame finale.
- Una schermata, font bitmap 5x7 già esistente in platform_sdl.c con advance di 7 px, dimensione logica 960x540. Solo lettere Z e _ aggiunte all'alfabeto bitmap. Niente asset nuovi da installare, niente nuovo simulatore, niente alterazioni dei controlli delle partite. Shortcut F12+1..8 preesistenti ancora funzionanti per diagnosi/test, ma non occupano spazio nel pannello.
- Titolo WORLD/BUILD, giorno civile live, SETTIMANA della stagione dal 1 luglio e stagione OG; rosso se i due riferimenti temporali non coincidono. La settimana non usa il contatore cumulativo ExpandedWorldLive_Weeks e viene ricalcolata dopo un restore. Tutte le metriche vengono ricavate dai dati residenti *ad ogni disegno*, senza cache indipendente e senza creare/avanzare una partita. Una nuova carriera o il RESTORE della .CAR cambiano lo snapshot solo quando cambia effettivamente lo stato OG.

## Denominatori e numeratori
- CAMPIONATI [6 regioni]: **nazioni completamente caricate / nazioni che hanno almeno una divisione OG definita**. Per essere completa devono esistere nel corrente anno OG tutti i contesti per le divisioni originali del paese. Nel mondo 26 i campionati statali brasiliani definiti dal pacchetto (13) contano come mancanti finché non divengono residenti.
- COPPE NAZIONALI [6 regioni]: **nazioni con tutti gli slot di coppa definiti residenti / nazioni con almeno una coppa nazionale definita**. Le definizioni vengono dalla tabella OG del paese più World_Cups (solo le coppie paese/slot, senza doppio conteggio). Le coppe nazionali non presenti né nella tabella OG né negli input del mondo richiedono ancora confronto esterno con le altre versioni SWOS; il pannello non può dedurre definizioni esterne.
- COPPE INT: **tornei internazionali per club residenti / definiti nel mondo**. Storico: Champions, Cup Winners', UEFA Cup, Libertadores 1996 = 4 attesi. Moderno: tre UEFA native e 12 competizioni da World_ContiCups = 15 attesi. La sola definizione Copa moderna non è prova di residenza.
- NAZIONALI: definizioni OG di WORLD CUP, i 6 campionati confederali per nazionali, 8 gruppi qualificazione al mondiale, 8 qualificazioni europee e le 5 qualificazioni continentali = 28 nomi OG quando tutti installati. Il motore residente globale non conserva ancora contesti autonomi delle nazionali, quindi non contare PRESET o la selezione privata del manager come residenti.
- AVANZAMENTO: rapporto **nazioni/tornèi con gara OG osservata / nazioni/tornèi attesi**; richiede che ogni divisione o coppa di quel paese registri almeno una gara. Nel contempo il valore `GARE N` riporta la somma dei match effettivamente registrati nelle strutture OG correnti. Questo NON è `match già giocati/match totali del calendario`, perché il denominatore delle fixture datate non è completo per tutti i formati. Non inventare i match mancanti; il rapporto evolve con ogni FORWARD TIME dove il residente avanza realmente.
- Verde solo con numeratore=denominatore non zero; rosso quando manca almeno un residente/avanzamento; grigio 0/0 senza definizioni. Il pannello non prova che siano corrette qualificazioni, fixture individuali o salvataggio dopo riavvio: serve diagnostica Windows.

## Controlli, esito atteso e limitazioni
- Il pulsante debug +1 MESE nel calendario carriera e' previsto per una build successiva, non incluso nel presente intervento F12.
- Entrambi mondi: F12 prima di START mostra nessun OG residente; START crea le metriche; FORWARD TIME le aggiorna; OFFERTA/ingaggio non devono azzerare conteggi; TITLE + CONTINUE nello stesso processo e chiusura/riavvio + .CAR devono ripristinarle se save validato.
- Screenshot dimostrativo riproducibile: `SWOS_F12_DASHBOARD_DEMO.png` (in chat, **numeri inventati solo per impaginazione**, font pixel esatto del renderer, non prova OG).
- Controllo statico delle stringhe/menu F12 e degli stati residenti: da eseguire prima della pubblicazione. Compilazione Windows e gameplay **NON PROVATI**.
