/* Standalone competition-history regression. No OG runtime/CAR claims. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "expanded_competition_history.h"

static int32_t date(uint16_t year, uint8_t month, uint8_t day)
{
    int32_t result = -1;
    assert(ExpandedWorld_DateToIndex(
        (ExpandedWorldDate){year, month, day}, &result));
    return result;
}
int main(void)
{
    ExpandedFixtureLedgerEntry matches[8] = {0};
    ExpandedFixtureLedger ledger = {matches, 0u, 8u};
    ExpandedFixtureDetail detail[8] = {0};
    ExpandedEditionSummary summaries[8] = {0};
    ExpandedCompetitionHistory h = {detail, 0u, 8u, summaries, 0u, 8u};
    ExpandedFixtureKey first = {
        {{WORLD_2627, 301u}, 0, 0u}, 1u, 1u, 0u, 0u
    };
    ExpandedFixtureKey second;
    ExpandedFixtureKey open;
    ExpandedClubKey home = {WORLD_2627, 1001u}, away = {WORLD_2627, 1002u};
    ExpandedFixtureScore score = {2u, 1u, 0u, 0u, 0u};
    uint16_t season = 0u;
    uint32_t removed = 99u;
    first.edition.start_day = date(2027, 2, 1);
    second = first; second.tie = 2u;
    open = first; open.edition.competition.stable_competition = 302u;
    open.edition.start_day = date(2027, 3, 1);

    assert(ExpandedHistory_Season(date(2027, 6, 30), &season) &&
           season == 2026u);
    assert(ExpandedHistory_Season(date(2027, 7, 1), &season) &&
           season == 2027u);
    assert(ExpandedHistory_ValidateRestore(&h));

    assert(ExpandedFixtureLedger_Schedule(&ledger, first) == EXP_FIXTURE_OK);
    assert(ExpandedFixtureLedger_Begin(&ledger, first) == EXP_FIXTURE_OK);
    assert(ExpandedFixtureLedger_Finish(&ledger, first, score) == EXP_FIXTURE_OK);
    assert(ExpandedHistory_RecordPlayed(&h, &ledger, first, home, away,
           date(2027, 4, 1), 101u) == EXP_HISTORY_OK);
    assert(ExpandedHistory_RecordPlayed(&h, &ledger, first, home, away,
           date(2027, 4, 1), 101u) == EXP_HISTORY_ALREADY);
    assert(ExpandedHistory_RecordPlayed(&h, &ledger, first, home, away,
           date(2027, 4, 1), 102u) == EXP_HISTORY_CONFLICT);

    assert(ExpandedFixtureLedger_Schedule(&ledger, second) == EXP_FIXTURE_OK);
    assert(ExpandedHistory_CloseEdition(&h, &ledger, first.edition,
           date(2027, 11, 1), 1001u, 1002u) == EXP_HISTORY_BLOCKED);
    assert(ExpandedHistory_Compact(&h,date(2028,8,1),&removed) ==
           EXP_HISTORY_BLOCKED && removed == 0u && h.detail_count == 1u);
    assert(ExpandedFixtureLedger_Begin(&ledger, second) == EXP_FIXTURE_OK);
    assert(ExpandedFixtureLedger_Finish(&ledger, second, score) == EXP_FIXTURE_OK);
    assert(ExpandedHistory_RecordPlayed(&h, &ledger, second, away, home,
           date(2027, 11, 1), 201u) == EXP_HISTORY_OK);
    assert(ExpandedHistory_CloseEdition(&h, &ledger, first.edition,
           date(2027, 11, 1), 1001u, 1002u) == EXP_HISTORY_OK);
    assert(ExpandedHistory_CloseEdition(&h, &ledger, first.edition,
           date(2027, 11, 1), 1001u, 1002u) == EXP_HISTORY_ALREADY);
    assert(ExpandedHistory_Compact(&h, date(2028, 7, 1), &removed) ==
           EXP_HISTORY_BLOCKED && h.detail_count == 2u);
    assert(ExpandedHistory_FactsSecured(&h, first.edition) == EXP_HISTORY_OK);
    assert(ExpandedHistory_FactsSecured(&h, first.edition) == EXP_HISTORY_ALREADY);
    /* A closed edition contains a match from November: time cannot rewind
     * to the earlier July checkpoint and erase knowledge of that game. */
    assert(ExpandedHistory_Compact(&h, date(2027, 7, 1), &removed) ==
           EXP_HISTORY_CONFLICT && removed == 0u);
    /* February-start tournament: April was 2026/27, November 2027/28.
     * July 2028 must retain the November fixture as previous season. */
    assert(ExpandedHistory_Compact(&h, date(2028, 7, 1), &removed) ==
           EXP_HISTORY_OK && removed == 1u && h.detail_count == 1u);
    assert(ExpandedHistory_Lookup(&h, second) == EXP_HISTORY_DETAILED);
    assert(ExpandedHistory_Compact(&h, date(2029, 7, 1), &removed) ==
           EXP_HISTORY_OK && removed == 1u && h.detail_count == 0u);
    assert(h.summary_count == 1u && h.summaries[0].match_count == 2u);
    assert(h.summaries[0].champion == 1001u &&
           h.summaries[0].runner_up == 1002u);
    assert(ExpandedHistory_Lookup(&h, first) ==
           EXP_HISTORY_EDITION_ARCHIVED);
    assert(ExpandedHistory_RecordPlayed(&h, &ledger, first, home, away,
           date(2027, 4, 1), 101u) == EXP_HISTORY_CONFLICT);
    assert(ExpandedHistory_ValidateRestore(&h));

    /* Never discard evidence of a result that claims to lie in the future. */
    {
        ExpandedFixtureDetail staged[2] = {0};
        ExpandedEditionSummary complete[2] = {0};
        ExpandedCompetitionHistory check = {staged,1u,2u,complete,1u,2u};
        staged[0] = (ExpandedFixtureDetail){
            first,home,away,score,date(2028,2,1),123u
        };
        complete[0] = h.summaries[0];
        assert(ExpandedHistory_Compact(&check,date(2028,1,1),&removed) ==
               EXP_HISTORY_CONFLICT && check.detail_count == 1u);
    }

    /* A still-running old edition is NEVER compressed at July rollover. */
    assert(ExpandedFixtureLedger_Schedule(&ledger, open) == EXP_FIXTURE_OK);
    assert(ExpandedFixtureLedger_Begin(&ledger, open) == EXP_FIXTURE_OK);
    assert(ExpandedFixtureLedger_Finish(&ledger, open, score) == EXP_FIXTURE_OK);
    assert(ExpandedHistory_RecordPlayed(&h, &ledger, open, home, away,
           date(2027, 7, 15), 301u) == EXP_HISTORY_OK);
    assert(ExpandedHistory_Compact(&h, date(2029, 7, 1), &removed) ==
           EXP_HISTORY_BLOCKED && h.detail_count == 1u);
    assert(ExpandedHistory_CloseEdition(&h, &ledger, open.edition,
           date(2029, 8, 1), 1001u, 0u) == EXP_HISTORY_OK);
    assert(ExpandedHistory_FactsSecured(&h, open.edition) == EXP_HISTORY_OK);
    assert(ExpandedHistory_Compact(&h, date(2029, 7, 1), &removed) ==
           EXP_HISTORY_BLOCKED); /* not yet closed on this date */
    assert(ExpandedHistory_Compact(&h, date(2029, 8, 1), &removed) ==
           EXP_HISTORY_OK && removed == 1u);
    assert(ExpandedHistory_ValidateRestore(&h));

    /* 1996/97 uses exactly the same retention rule; never cross world IDs. */
    {
        ExpandedFixtureKey old = first;
        old.edition.competition.world = WORLD_9697;
        old.edition.start_day = date(1996, 8, 1);
        old.tie = 3u;
        assert(ExpandedHistory_Season(old.edition.start_day, &season) &&
               season == 1996u);
        assert(ExpandedHistory_Season(date(1997, 6, 30), &season) &&
               season == 1996u);
        assert(ExpandedHistory_Lookup(&h, old) == EXP_HISTORY_NOT_FOUND);
    }
    {
        ExpandedCompetitionHistory copy = h;
        ExpandedEditionSummary saved[8];
        memcpy(saved, summaries, sizeof saved);
        copy.summaries = saved;
        assert(ExpandedHistory_ValidateRestore(&copy));
        saved[1].edition = saved[0].edition;
        assert(!ExpandedHistory_ValidateRestore(&copy));
    }
    puts("COMPETITION_HISTORY_RETENTION=PASS_ISOLATED_ONLY");
    puts("COMPETITION_HISTORY_CAR=NOT_IMPLEMENTED");
    return 0;
}
