# SWOS Expanded 00.13.208 — Punto 7: inventario e primo validatore JSON

19 novembre 2026. Branch autorizzato dev/0.10, parent e16bbce5b03b9169bb088fb778eba726f80dd81a (207). Un solo commit richiesto. Windows build e gameplay restano test dell'utente.

## Decisione approvata

**A.** Convertire/catalogare le definizioni esistenti dei due mondi, riusando gli attuali JSON per campionati, coppe, tornei continentali, nazionali, club e giocatori. **B.** Validare preventivamente ID, formati, quote, date, squadre e riferimenti con diagnostica precisa del file/campo. **C.** Collegare i JSON alle VERE gare OG residenti, qualificazioni/edizioni e CAR, senza creare un nuovo simulatore. **D.** Estendere a club, giocatori e rose extra con UUID/ordinale coerenti. Nessun editor visuale necessario: l'utente richiede modifiche in chat, l'assistente prepara JSON e verifica, con decisioni sulle nuove regole e commit solo autorizzati.

## Realizzato nella prima tranche 208 (A1/B1)

- Nuovo sorgenti/scripts/validate_data.py, preflight **offline di sola lettura**, con Python standard già parte degli strumenti di sviluppo del repository. **Non** è chiamato dalla compilazione Windows né richiede installazione all'utente; non scrive JSON e non modifica CAR.
- Inventaria dieci JSON base data_json/competitions (nove collection e seed_buckets.json). Controlla conteggi, simboli ID/comp_id, riferimenti fra paese/competizione/career/qualification/league_blocks, country define e seed list, quote seed bucket e reali team_file esistenti.
- Per il mondo 2627 legge data_json/worlds/2627/competitions/leagues.json e Libertadores provvisoria; controlla numero delle divisioni, i 47 partecipanti (28 direct e 19 preliminary), duplicati e date ISO dell'edizione 2027. Dichiara sempre PENDING il collegamento OG: runtime_status DATA_ONLY_NOT_YET_LINKED_TO_OG_MATCHES.
- Inventaria i JSON dei team e squad_extras; con --teams esamina facoltativamente club OG, 16 calciatori base e UUID squadra/giocatore. I riferimenti completi ai player extra rimangono futuri.
- Diagnostica ERROR (exit 1), WARNING, PENDING, PASS_SCOPED e report macchina --json. PASS_SCOPED non implica competizione giocabile.
- Nuovo sorgenti/tests/test_validate_data.py: quattro test sintetici per formato base valido e marker OG, riferimento country seed inesistente, count sbagliato, country duplicato discordante. **TEST PYTHON NON ESEGUITI in questo turno**.

## Inventario verificato sul repository

| Fonte | Elementi |
|---|---:|
| data_json/competitions | 10 file |
| countries.json base | 72 righe, 71 number distinti (Brazil 46 duplicato identico) |
| leagues/cups/tournaments/playoffs base | 66 / 76 / 14 / 7 |
| career/qualifying base | 35 voci (compreso null finale) / 10 gruppi |
| Overlay 2627: leghe | 91; 26 simboli non esistenti nella base |
| data_json/teams | 76 file |
| data_json/worlds/2627/teams | 101 file |
| data_json/worlds/2627/squad_extras | 101 file |
| Libertadores 2027 prima edizione | 47 partecipanti, NON collegata alle partite OG |

I controlli strutturali indipendenti sul contenuto JSON restituito da GitHub hanno identificato 170 ID simbolici diversi e 163 comp_id numerici diversi, senza conflitti nel relativo grafo; verificati anche i percorsi team_file. Il vecchio countries.json ripete identico Brasile 46: warning (ma due record incompatibili con lo stesso numero sono errore). I marcatori nativi -2 e 252 di qualifying.comp_cup sono selector/sentinel e non tornei mancanti. Nel bucket22Full 23 posizioni rappresentano 22 quote e un terminatore OG. Non è una verifica integrale delle rose enormi: il connettore ha limiti di lettura per alcuni file JSON da megabyte.

## Come usare il preflight (solo sviluppo, non richiesto su Windows)

    python3 sorgenti/scripts/validate_data.py --world both --root .
    python3 sorgenti/scripts/validate_data.py --world both --root . --json
    python3 sorgenti/scripts/validate_data.py --world both --root . --teams
    python3 -m unittest discover -s sorgenti/tests -p 'test_validate_data.py'

## Limiti e prossimo lavoro

**A2** convertire i formati OG ancora senza JSON con classi di stato DEFINED/VALIDATED/RESIDENT/NEXT_QUALIFIED/SAVE_RESTORED; **B2** validare anche squad_extras e introdurre i controlli equivalenti nel loader C prima dell'inizializzazione OG; **C** collegare una prima competizione reale end-to-end con date, gare OG, edizioni, ID persistenti e SAVE/CONTINUE; **D** facilitare aggiunte di club e giocatori su JSON via assistente. Nessuna nuova partita simulata o punteggio retroattivo.

## QA e consegna

**SI**: sorgenti/JSON inventariati; controllo metadati e riferimenti reali tramite analisi indipendente; codice e test aggiunti come file di sviluppo; sei guide canoniche + guida di architettura + due guide dati aggiornate. **NON PROVATO**: esecuzione completa script Python sui JSON e 4 test Python, compilazione Windows 208, gameplay, coppa JSON giocabile, SAVE/CONTINUE, test multianno. **NO**: modifiche al motore partita, ai CAR, alle modalità CLASSIC/DIY/SEASON, alle definizioni dati esistenti, a installazioni Windows.
