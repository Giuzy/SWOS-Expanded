/* Competition Engine: bounded detailed history plus compact edition archive.
 * New-engine contract only: NOT wired to OG gameplay or to the .CAR writer.
 * The caller must atomically checkpoint results, RNG, events and qualification
 * facts before marking them durable. Never prune an active/unresolved edition.
 */
#ifndef EXPANDED_COMPETITION_HISTORY_H
#define EXPANDED_COMPETITION_HISTORY_H
#include <stdint.h>
#include <string.h>
#include "expanded_competition_identity.h"
#include "expanded_world_time.h"

typedef enum ExpandedHistoryResult {
    EXP_HISTORY_INVALID = -3,
    EXP_HISTORY_CONFLICT = -2,
    EXP_HISTORY_FULL = -1,
    EXP_HISTORY_OK = 0,
    EXP_HISTORY_ALREADY = 1,
    EXP_HISTORY_BLOCKED = 2
} ExpandedHistoryResult;

typedef enum ExpandedHistoryLookup {
    EXP_HISTORY_NOT_FOUND = 0,
    EXP_HISTORY_DETAILED = 1,
    EXP_HISTORY_EDITION_ARCHIVED = 2
} ExpandedHistoryLookup;

/* Results, participants, civil date and an opaque OG-effect fingerprint
 * are retained for the current and immediately preceding July-June season.
 * The fingerprint is NOT a substitute for persisting the actual RNG state. */
typedef struct ExpandedFixtureDetail {
    ExpandedFixtureKey key;
    ExpandedClubKey home, away;
    ExpandedFixtureScore score;
    int32_t played_day;
    uint64_t effect_fingerprint;
} ExpandedFixtureDetail;

/* Minimal permanent history: ONE record per completed edition; no per-match
 * scores, formations, player stats or seed tables for old seasons. Club IDs
 * are stable world-pack IDs, never mutable TeamFile ordinals/checksums.
 * Qualification facts are distinct durable state, NOT reconstructed here. */
typedef struct ExpandedEditionSummary {
    ExpandedEditionKey edition;
    int32_t closed_day;
    uint32_t champion, runner_up;
    uint32_t match_count;
    uint8_t qualification_facts_secured; /* 0 until durable downstream facts */
} ExpandedEditionSummary;

typedef struct ExpandedCompetitionHistory {
    ExpandedFixtureDetail *details;
    uint32_t detail_count, detail_capacity;
    ExpandedEditionSummary *summaries;
    uint32_t summary_count, summary_capacity;
} ExpandedCompetitionHistory;

static inline int ExpandedHistory_ValidStorage(const ExpandedCompetitionHistory *h)
{
    return h && h->details && h->summaries &&
           h->detail_count <= h->detail_capacity &&
           h->summary_count <= h->summary_capacity;
}

/* 2027-01 through 2027-06 belong to the season beginning July 2026.
 * The same rule applies to editions whose actual closing day is in a later
 * season, but an open edition is never eligible for compression. */
static inline int ExpandedHistory_Season(int32_t day, uint16_t *season)
{
    ExpandedWorldDate date;
    if (!season || !ExpandedWorld_IndexToDate(day, &date)) return 0;
    *season = (uint16_t)(date.year - (date.month < 7u ? 1u : 0u));
    return 1;
}

static inline int ExpandedHistory_KeyBefore(ExpandedFixtureKey a,
                                           ExpandedFixtureKey b)
{
    if (a.edition.competition.world != b.edition.competition.world)
        return a.edition.competition.world < b.edition.competition.world;
    if (a.edition.competition.stable_competition !=
        b.edition.competition.stable_competition)
        return a.edition.competition.stable_competition <
               b.edition.competition.stable_competition;
    if (a.edition.start_day != b.edition.start_day)
        return a.edition.start_day < b.edition.start_day;
    if (a.edition.sequence != b.edition.sequence)
        return a.edition.sequence < b.edition.sequence;
    if (a.stage != b.stage) return a.stage < b.stage;
    if (a.tie != b.tie) return a.tie < b.tie;
    if (a.leg != b.leg) return a.leg < b.leg;
    return a.replay < b.replay;
}

static inline const ExpandedFixtureDetail *ExpandedHistory_FindFixture(
    const ExpandedCompetitionHistory *h, ExpandedFixtureKey key)
{
    uint32_t lo = 0u, hi;
    if (!ExpandedHistory_ValidStorage(h)) return 0;
    hi = h->detail_count;
    while (lo < hi) {
        const uint32_t mid = lo + (hi - lo) / 2u;
        const ExpandedFixtureKey found = h->details[mid].key;
        if (ExpandedFixtureKey_Equal(found, key)) return &h->details[mid];
        if (ExpandedHistory_KeyBefore(found, key)) lo = mid + 1u;
        else hi = mid;
    }
    return 0;
}

static inline const ExpandedEditionSummary *ExpandedHistory_FindSummary(
    const ExpandedCompetitionHistory *h, ExpandedEditionKey edition)
{
    uint32_t i;
    if (!ExpandedHistory_ValidStorage(h)) return 0;
    for (i = 0u; i < h->summary_count; ++i)
        if (ExpandedEditionKey_Equal(h->summaries[i].edition, edition))
            return &h->summaries[i];
    return 0;
}

/* A summary also acts as an edition-wide tombstone after fixture details
 * have been discarded. An old fixture must NEVER be simulated again. */
static inline ExpandedHistoryLookup ExpandedHistory_Lookup(
    const ExpandedCompetitionHistory *h, ExpandedFixtureKey key)
{
    if (!ExpandedHistory_ValidStorage(h) || !ExpandedFixtureKey_Valid(key))
        return EXP_HISTORY_NOT_FOUND;
    if (ExpandedHistory_FindFixture(h, key)) return EXP_HISTORY_DETAILED;
    if (ExpandedHistory_FindSummary(h, key.edition))
        return EXP_HISTORY_EDITION_ARCHIVED;
    return EXP_HISTORY_NOT_FOUND;
}

/* Only record an already PLAYED single-writer ledger result. Never derive or
 * invent a score. The caller provides real participants/date and an opaque
 * fingerprint of effects; an existing entry with different facts is a fault. */
static inline ExpandedHistoryResult ExpandedHistory_RecordPlayed(
    ExpandedCompetitionHistory *h, const ExpandedFixtureLedger *ledger,
    ExpandedFixtureKey key, ExpandedClubKey home, ExpandedClubKey away,
    int32_t played_day, uint64_t effect_fingerprint)
{
    uint32_t i, pos;
    const ExpandedFixtureLedgerEntry *match = 0;
    const ExpandedFixtureDetail *previous;
    ExpandedWorldDate date;
    if (!ExpandedHistory_ValidStorage(h) || !ledger || !ledger->entries ||
        ledger->count > ledger->capacity || !ExpandedFixtureKey_Valid(key) ||
        !ExpandedClubKey_Valid(home) || !ExpandedClubKey_Valid(away) ||
        home.world != key.edition.competition.world ||
        away.world != key.edition.competition.world ||
        ExpandedClubKey_Equal(home, away) ||
        played_day < key.edition.start_day ||
        !ExpandedWorld_IndexToDate(played_day, &date))
        return EXP_HISTORY_INVALID;
    for (i = 0u; i < ledger->count; ++i)
        if (ExpandedFixtureKey_Equal(ledger->entries[i].key, key)) {
            match = &ledger->entries[i];
            break;
        }
    if (!match || match->state != EXP_FIXTURE_PLAYED)
        return EXP_HISTORY_BLOCKED;
    previous = ExpandedHistory_FindFixture(h, key);
    if (previous) {
        if (ExpandedClubKey_Equal(previous->home, home) &&
            ExpandedClubKey_Equal(previous->away, away) &&
            previous->played_day == played_day &&
            previous->effect_fingerprint == effect_fingerprint &&
            ExpandedFixtureScore_Equal(previous->score, match->score))
            return EXP_HISTORY_ALREADY;
        return EXP_HISTORY_CONFLICT;
    }
    if (ExpandedHistory_FindSummary(h, key.edition))
        return EXP_HISTORY_CONFLICT; /* closed/archived edition is immutable */
    if (h->detail_count == h->detail_capacity) return EXP_HISTORY_FULL;
    pos = 0u;
    while (pos < h->detail_count &&
           ExpandedHistory_KeyBefore(h->details[pos].key, key)) ++pos;
    if (pos < h->detail_count)
        memmove(&h->details[pos + 1u], &h->details[pos],
                (size_t)(h->detail_count - pos) * sizeof h->details[0]);
    h->details[pos].key = key;
    h->details[pos].home = home;
    h->details[pos].away = away;
    h->details[pos].score = match->score;
    h->details[pos].played_day = played_day;
    h->details[pos].effect_fingerprint = effect_fingerprint;
    ++h->detail_count;
    return EXP_HISTORY_OK;
}

/* End-of-edition facts come from the real OG competition state. The entire
 * edition's fixture ledger must be PLAYED and represented in the journal.
 * No premature closure or synthesized champion is allowed. */
static inline ExpandedHistoryResult ExpandedHistory_CloseEdition(
    ExpandedCompetitionHistory *h, const ExpandedFixtureLedger *ledger,
    ExpandedEditionKey edition, int32_t closed_day,
    uint32_t champion, uint32_t runner_up)
{
    uint32_t i, played = 0u, recorded = 0u;
    const ExpandedEditionSummary *previous;
    ExpandedWorldDate date;
    if (!ExpandedHistory_ValidStorage(h) || !ledger || !ledger->entries ||
        ledger->count > ledger->capacity || !champion ||
        (runner_up && runner_up == champion) ||
        edition.competition.world >= (uint16_t)WORLD_COUNT ||
        !edition.competition.stable_competition ||
        closed_day < edition.start_day ||
        !ExpandedWorld_IndexToDate(edition.start_day, &date) ||
        !ExpandedWorld_IndexToDate(closed_day, &date))
        return EXP_HISTORY_INVALID;
    previous = ExpandedHistory_FindSummary(h, edition);
    if (previous)
        return previous->closed_day == closed_day &&
               previous->champion == champion &&
               previous->runner_up == runner_up ?
               EXP_HISTORY_ALREADY : EXP_HISTORY_CONFLICT;
    for (i = 0u; i < ledger->count; ++i) {
        const ExpandedFixtureLedgerEntry *entry = &ledger->entries[i];
        const ExpandedFixtureDetail *detail;
        if (!ExpandedEditionKey_Equal(entry->key.edition, edition)) continue;
        if (entry->state != EXP_FIXTURE_PLAYED) return EXP_HISTORY_BLOCKED;
        detail = ExpandedHistory_FindFixture(h, entry->key);
        if (!detail || detail->played_day > closed_day ||
            !ExpandedFixtureScore_Equal(detail->score, entry->score))
            return EXP_HISTORY_BLOCKED;
        ++played;
    }
    for (i = 0u; i < h->detail_count; ++i)
        if (ExpandedEditionKey_Equal(h->details[i].key.edition, edition))
            ++recorded;
    if (!played || played != recorded) return EXP_HISTORY_BLOCKED;
    if (h->summary_count == h->summary_capacity) return EXP_HISTORY_FULL;
    h->summaries[h->summary_count++] = (ExpandedEditionSummary){
        edition, closed_day, champion, runner_up, played, 0u
    };
    return EXP_HISTORY_OK;
}

/* Must be called ONLY after relevant final tables, qualification entrants,
 * title-holder facts and any open dependency are durably checkpointed.
 * The summary remains intact for Hall of Fame/history and anti-replay. */
static inline ExpandedHistoryResult ExpandedHistory_FactsSecured(
    ExpandedCompetitionHistory *h, ExpandedEditionKey edition)
{
    uint32_t i;
    if (!ExpandedHistory_ValidStorage(h)) return EXP_HISTORY_INVALID;
    for (i = 0u; i < h->summary_count; ++i)
        if (ExpandedEditionKey_Equal(h->summaries[i].edition, edition)) {
            if (h->summaries[i].qualification_facts_secured)
                return EXP_HISTORY_ALREADY;
            h->summaries[i].qualification_facts_secured = 1u;
            return EXP_HISTORY_OK;
        }
    return EXP_HISTORY_BLOCKED;
}

/* Retain full fixture details for the current and previous July-June
 * seasons of EACH ACTUAL MATCH, not the season when its edition started.
 * Example: a Copa starting in February 2027 has matches in both seasons
 * 2026 and 2027. At July 2028, its post-July 2027 matches are still in
 * the previous season and MUST remain detailed. Never compact an ACTIVE
 * edition, nor an edition without durable downstream qualification facts.
 * Preflight all candidate removals before touching the caller's buffers. */
static inline ExpandedHistoryResult ExpandedHistory_Compact(
    ExpandedCompetitionHistory *h, int32_t today, uint32_t *removed)
{
    uint16_t current;
    uint32_t i, write = 0u;
    if (removed) *removed = 0u;
    if (!ExpandedHistory_ValidStorage(h) || !removed ||
        !ExpandedHistory_Season(today, &current))
        return EXP_HISTORY_INVALID;
    for (i = 0u; i < h->detail_count; ++i) {
        const ExpandedFixtureDetail *d = &h->details[i];
        const ExpandedEditionSummary *summary;
        uint16_t season;
        if (!ExpandedHistory_Season(d->played_day, &season))
            return EXP_HISTORY_INVALID;
        if (d->played_day > today) return EXP_HISTORY_CONFLICT;
        if ((uint32_t)season + 1u >= (uint32_t)current) continue;
        summary = ExpandedHistory_FindSummary(h, d->key.edition);
        if (!summary || !summary->qualification_facts_secured ||
            summary->closed_day > today)
            return EXP_HISTORY_BLOCKED;
    }
    for (i = 0u; i < h->detail_count; ++i) {
        uint16_t season = 0u;
        /* The preflight above already checked all dates. */
        (void)ExpandedHistory_Season(h->details[i].played_day,
                                     &season);
        if ((uint32_t)season + 1u >= (uint32_t)current)
            h->details[write++] = h->details[i];
    }
    *removed = h->detail_count - write;
    h->detail_count = write;
    return EXP_HISTORY_OK;
}

static inline int ExpandedHistory_ValidateRestore(
    const ExpandedCompetitionHistory *h)
{
    uint32_t i, j;
    if (!ExpandedHistory_ValidStorage(h)) return 0;
    for (i = 0u; i < h->detail_count; ++i) {
        const ExpandedFixtureDetail *d = &h->details[i];
        ExpandedWorldDate date;
        if (!ExpandedFixtureKey_Valid(d->key) ||
            !ExpandedClubKey_Valid(d->home) ||
            !ExpandedClubKey_Valid(d->away) ||
            d->home.world != d->key.edition.competition.world ||
            d->away.world != d->key.edition.competition.world ||
            ExpandedClubKey_Equal(d->home, d->away) ||
            d->played_day < d->key.edition.start_day ||
            !ExpandedWorld_IndexToDate(d->played_day, &date) ||
            (i && !ExpandedHistory_KeyBefore(h->details[i-1u].key, d->key)))
            return 0;
    }
    for (i = 0u; i < h->summary_count; ++i) {
        const ExpandedEditionSummary *s = &h->summaries[i];
        ExpandedWorldDate date;
        if (s->edition.competition.world >= (uint16_t)WORLD_COUNT ||
            !s->edition.competition.stable_competition || !s->champion ||
            !s->match_count ||
            (s->runner_up && s->runner_up == s->champion) ||
            s->qualification_facts_secured > 1u ||
            s->closed_day < s->edition.start_day ||
            !ExpandedWorld_IndexToDate(s->edition.start_day, &date) ||
            !ExpandedWorld_IndexToDate(s->closed_day, &date))
            return 0;
        for (j = 0u; j < i; ++j)
            if (ExpandedEditionKey_Equal(s->edition,h->summaries[j].edition))
                return 0;
    }
    return 1;
}
#endif /* EXPANDED_COMPETITION_HISTORY_H */
