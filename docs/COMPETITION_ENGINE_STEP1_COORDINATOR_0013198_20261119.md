> **Audit aggiuntivo prima del commit:** il fault-latch impedisce nuovi FORWARD, ma da solo non protegge SAVE; SaveJson_WriteCareer poteva sostituire un CAR anche dopo una mutazione OG parziale. Aggiunto quindi Expanded_CompetitionCoordinator_CanSave() e controllo prima di qualunque serializzazione/backup/scrittura della carriera. Se RECOVERY_REQUIRED, WRITE_BLOCKED e CAR preesistente intatto. Non e' un rollback; eventuali stati RAM modificati vanno scartati mediante LOAD valido.

# SWOS Expanded — Competition Engine / 00.13.198, coordinamento fase A1
Data: 19 novembre 2026. Base GitHub dev/0.10 HEAD 1689a52f441a6edb1cab124390ce0104eff88128, sorgenti versione 00.13.197. Commit unico finale autorizzato dall'utente.

## Evidenze disponibili, separazione dei test
- La nuova richiesta fornisce uno screenshot della 00.13.197 con UNEMPLOYED 1996/97, lunedi 1 luglio 1996. I comandi DEBUG +1 MONTH e DEBUG 30 JUNE vengono selezionati ma non sono disegnati; si vede la cornice di focus vuota nella colonna sinistra. BACK, FORWARD TIME, DEBUG WORLD e RECORD sono visibili. Questo e' un test **visivo e funzionale dell'utente**; non dimostra niente sui calendari delle coppe.
- Non e' stato allegato un nuovo ZIP di diagnostica della 00.13.197 nel messaggio. Le ultime diagnostiche ZIP effettivamente disponibili sono le sessioni 2/3 delle ore 10:46, **versione 00.13.196**, gia' analizzate. Nessuna affermazione sui risultati di compilazione, warning o crash 197 e' deducibile da quei due ZIP.
- Diagnostiche Windows 00.13.196: FINISH SEASON manuale nel 9697 e 2627 raggiunge INVALID_RESIDENT_TYPE2, VerifyTeamLoaded(0x0000), PANIC; mancano alcune seconde edizioni continentali e fonti di qualificazione. La 197 aveva corretto parte dei guard; l'utente ora segnala ancora il passaggio manuale non funzionante e il messaggio data. **Su istruzione espressa, NON intervenire sul cambio manuale in questo commit**. Non spacciarne lo stato per risolto.

## Scoperte di codice prima della modifica
- career_init.c::ChooseTeamAndStartCareer importa i TEAM world, chiama Expanded_OGWorld_Start e poi collega un club o attende un'offerta.
- expanded_career.c::Expanded_UnemployedWait determinava target civili e invocava in sequenza Expanded_OGWorld_AdvanceToDate e ExpandedWorldLive_AdvanceToDate. LWE in ExpandedWorldLive non esegue risultati, si limita al cursore compatibilita'; non esiste un secondo score engine. Se OG falliva, il chiamante NON avanzava il civile, ma una mutazione OG parziale non poteva essere annullata automaticamente.
- career_init.c::InitCareerClub ricostruisce ancora le slot locali del manager; Expanded_OGWorld_BindClub copia una lega residente verificata. Senza prova di contesto rimane un percorso legacy di compatibilita'. La gestione delle gare del manager e delle coppe non e' unificata dal nuovo wrapper.
- save_format_json.c::SaveJson_LoadCareer esegue controlli e ricostruzione pointer, poi ripristina og_resident. Era gia' presente un controllo di data OG/civile durante preflight. La nuova verifica READ-ONLY dopo il restore non modifica CAR, fixture o RNG.
- menu_system.c::DrawMenu disegna elementi menu, poi overlay Expanded_UI_After. La 197 aveva collegato l'input a entry 4/5 ma nello screenshot non appaiono riempimento e testo. La label nativa viene ripassata dopo i livelli grafici, nelle medesime hitbox, usando il font originale SWOS e NON un nuovo widget.

## Scelta implementata: coordinator compatibile
Nel modulo career_init.c sono ora disponibili i quattro entry point dichiarati in expanded_competition_coordinator.h:
1. Expanded_CompetitionCoordinator_Start: richiama esattamente UNA inizializzazione originale OG e verifica world/civil start; non aggiunge un altro calendario.
2. Expanded_CompetitionCoordinator_Advance: unico entry point del percorso disoccupato per invocare OG e poi consolidare il cursore civile Expanded. Preflight world/anno/stato/date; reject rewind; noop su data uguale. Traccia [competition-coordinator] START_READY / ADVANCE_COMMITTED / BLOCK / FAULT.
3. Expanded_CompetitionCoordinator_PreflightBind: prima del bind di club OG, verifica in sola lettura che stagione resident e data civile coincidano. Non ricrea nulla.
4. Expanded_CompetitionCoordinator_ValidateRestore: dopo restore del JSON CAR, verifica lo stesso contratto; se invalido segnala errore di LOAD e non convalida una carriera disallineata.
Se una chiamata OG o commit civile fallisce DOPO mutazione parziale, si alza un latch di recupero e non si ripete automaticamente un tick: e' necessario ripartire da un CAR valido. Questo **NON e' un vero rollback transazionale**, e i vecchi CAR devono essere mantenuti.

## Perche' non attivare ancora ExpandedEventScheduler (decisione tecnica)
Alternativa scartata: chiamare in parallelo il nuovo scheduler e OG sulle medesime gare. Motivo: expanded_event_scheduler.h e expanded_competition_identity.h documentano che non esiste un single-writer live condiviso con CPU/manager, ne' il claim + score + effetti + RNG + evento nello stesso checkpoint CAR. Attivarlo ora puo' generare partite duplicate e corrompere salvataggi. L'approccio proposto mantiene OG come SOLO calcolatore e sposta prima la logica di coordinamento verso un unico contratto, un percorso alla volta.

## Render UNEMPLOYED 00.13.198
Expanded_UnemployedPaintDebugButtons e' chiamata da DrawMenu dopo Expanded_UI_After, e disegna un fondo blu, il bordo e il testo centrato con DrawMenuTextCentered e charset _charTables[0], usando x/y/larghezza/altezza esatti delle entry native 4 e 5. Nessun PNG, nessuna hitbox inventata e nessuna modifica al font. Il focus Switch Pro viene ristampato quando si trova su uno di quei due pulsanti. Dopo le azioni DEBUG, la selezione torna rispettivamente a 4 e 5 (non alle vecchie entry 13/14 ormai nascoste). UI Windows ancora DA TESTARE.

## Confini della fase A1: cosa NON e' completato
- La gestione manager-occupato di FORWARD/NEXT MATCH continua a passare attraverso callback OG locali, senza centralizzazione completa; lo stesso vale per coppe e alcune transizioni di competizione.
- FINISH SEASON manuale resta invariato su richiesta. Le gare manager/CPU non condividono ancora un claim preventivo universale; il nuovo scheduler non produce gli eventi live.
- La sicurezza cross-process del CAR per match + RNG + calendario + ACK evento non e' transazionale end-to-end. Le copie residenti e i witness esistenti rimangono i soli salvati; non e' stato introdotto un secondo journal.
- Manifest importabile/validatore e tre edizioni consecutive di coppa pilota sono ancora DA FARE. Non considerare questa fase sufficiente per aggiungere un lotto di nuove competizioni.

## Piano immediato, senza rifare i fix manuali
GATE A2: audit dei writer reali OG nel percorso club occupato (og native league/local bridge, cup managed_excluded, UEFA PENDING_MANAGER_BIND) e mappa di owner per (world,comp,edition), inclusi change club, sacking e viewer. Migrare SOLO una competition pilot con autorita' non ambigua.
GATE B: fixture identity/claim PREVENTIVO og manager/CPU condiviso, commit del risultato OG, effetti e RNG, piu' persist CAR atomico. Nessuna nuova partita simulata per prove.
GATE C: nuovo close/qualification/validate/create-next-edition, senza modificare mai retroattivamente le partite; coppe cross-year continuano. Poi definizione di dati/validatore generico e tre edizioni OG certificate.

## Protocollo Windows utente
- Sincronizza GitKraken e compila con gli script Windows esistenti. Zero nuovi warning GCC: se presenti, correggere subito; i test isolati NON provano EXE.
- Apri NEW CAREER -> START UNEMPLOYED in ENTRAMBI i mondi: DEBUG +1 MONTH e DEBUG 30 JUNE visibili, testo in HITBOX reale, mouse/joypad A e B; 30 giugno termina esattamente il 30/06 dell'anno corretto, offerta OG solo se disponibile.
- Prova FORWARD TIME 1 settimana / 1 mese / 30 giugno; una sola avanzata OG per evento, date coerenti in [competition-coordinator], nessun rewind o ripetizione.
- Prova accettazione offerta e riapertura della stessa classifica/DEBUG WORLD: zero gare duplicate e nessun reset di altri contesti. Non confondere questa verifica con la completezza del match manager.
- Prova SAVE/CONTINUE 96 e 26 con data/squadre/competizioni e hash invariati. Eventuale blocco dopo errore OG richiede il CAR precedente; non reinserire fittiziamente gli eventi.
- Se provi FINISH SEASON manuale, segnala semplicemente il difetto rimasto: questa build non lo corregge. Verifica F12+9, log prestazioni, durata FORWARD, coppe 96/26 con diagnosi separate.

## Checklist
SI: repository/HEAD e guide ispezionati, nuovo coordinatore su percorsi START/UNEMPLOYED/BIND preflight/LOAD, nessuna seconda simulazione, testo UI sovrapposto alla hitbox vera, controlli statici dei file prima del commit.
NO: single-writer completo manager e CPU, manual FINISH, scheduler live, universal CAR transaction, tutti i tornei, importatore data-driven, 3 edizioni campione.
NON PROVATO: compilazione completa Windows 00.13.198, zero warning 00.13.198, UI e input della 198, SAVE/CONTINUE 198, annual rollover e performance 198.
