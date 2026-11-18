/* SWOS Expanded: ID and one-writer fixture claim contract (not active gameplay).
 * This does NOT schedule matches, simulate scores, or persist a CAR journal.
 * Integrate with OG+atomic save before calling it in the live world. */
#ifndef SWOS_EXPANDED_COMPETITION_IDENTITY_H
#define SWOS_EXPANDED_COMPETITION_IDENTITY_H
#include "world.h"

/* IDs come from a stable world-pack registry, never from a mutable squad
 * checksum, a naked country ordinal, or the native 8-bit compId alone. */
typedef struct ExpandedClubKey {
    uint16_t world;
    uint32_t stable_club;
} ExpandedClubKey;
typedef struct ExpandedCompetitionKey {
    uint16_t world;
    uint32_t stable_competition;
} ExpandedCompetitionKey;
typedef struct ExpandedEditionKey {
    ExpandedCompetitionKey competition;
    int32_t start_day;            /* civil day index, not an OG phase */
    uint16_t sequence;            /* multiple editions can start together */
} ExpandedEditionKey;
typedef struct ExpandedFixtureKey {
    ExpandedEditionKey edition;
    uint32_t stage, tie;          /* nonzero stage/tie identifiers */
    uint16_t leg, replay;         /* zero denotes the original match */
} ExpandedFixtureKey;
typedef struct ExpandedFixtureScore {
    uint16_t home, away;
    uint16_t penalties_home, penalties_away;
    uint8_t flags;                /* ruleset-defined meaning */
} ExpandedFixtureScore;

typedef enum ExpandedFixtureState {
    EXP_FIXTURE_SCHEDULED = 1,
    EXP_FIXTURE_CLAIMED = 2,
    EXP_FIXTURE_PLAYED = 3
} ExpandedFixtureState;
typedef enum ExpandedFixtureDecision {
    EXP_FIXTURE_INVALID = -3,
    EXP_FIXTURE_CONFLICT = -2,
    EXP_FIXTURE_FULL = -1,
    EXP_FIXTURE_OK = 0,
    EXP_FIXTURE_BUSY = 1,
    EXP_FIXTURE_ALREADY_PLAYED = 2
} ExpandedFixtureDecision;
typedef struct ExpandedFixtureLedgerEntry {
    ExpandedFixtureKey key;
    ExpandedFixtureScore score;
    ExpandedFixtureState state;
} ExpandedFixtureLedgerEntry;
typedef struct ExpandedFixtureLedger {
    ExpandedFixtureLedgerEntry *entries;    /* supplied by future edition state */
    uint32_t count, capacity;
} ExpandedFixtureLedger;

static inline int ExpandedClubKey_Valid(ExpandedClubKey a)
{
    return a.world < (uint16_t)WORLD_COUNT && a.stable_club != 0u;
}
static inline int ExpandedClubKey_Equal(ExpandedClubKey a, ExpandedClubKey b)
{
    return a.world == b.world && a.stable_club == b.stable_club;
}
static inline int ExpandedCompetitionKey_Equal(ExpandedCompetitionKey a,
                                                ExpandedCompetitionKey b)
{
    return a.world == b.world && a.stable_competition == b.stable_competition;
}
static inline int ExpandedEditionKey_Equal(ExpandedEditionKey a,
                                            ExpandedEditionKey b)
{
    return ExpandedCompetitionKey_Equal(a.competition, b.competition) &&
           a.start_day == b.start_day && a.sequence == b.sequence;
}
static inline int ExpandedFixtureKey_Equal(ExpandedFixtureKey a, ExpandedFixtureKey b)
{
    return ExpandedEditionKey_Equal(a.edition, b.edition) &&
           a.stage == b.stage && a.tie == b.tie &&
           a.leg == b.leg && a.replay == b.replay;
}
static inline int ExpandedFixtureKey_Valid(ExpandedFixtureKey k)
{
    return k.edition.competition.world < (uint16_t)WORLD_COUNT &&
           k.edition.competition.stable_competition != 0u &&
           k.stage != 0u && k.tie != 0u;
}
static inline int ExpandedFixtureScore_Equal(ExpandedFixtureScore a,
                                             ExpandedFixtureScore b)
{
    return a.home == b.home && a.away == b.away &&
           a.penalties_home == b.penalties_home &&
           a.penalties_away == b.penalties_away && a.flags == b.flags;
}
static inline ExpandedFixtureLedgerEntry *ExpandedFixtureLedger_Find(
    ExpandedFixtureLedger *ledger, ExpandedFixtureKey key)
{
    uint32_t i;
    if (!ledger || !ledger->entries || ledger->count > ledger->capacity)
        return 0;
    for (i = 0; i < ledger->count; ++i)
        if (ExpandedFixtureKey_Equal(ledger->entries[i].key, key))
            return &ledger->entries[i];
    return 0;
}
/* Both AI and manager must claim the same fixture before using OG.
 * Duplicates never produce a second writable entry. */
static inline ExpandedFixtureDecision ExpandedFixtureLedger_Schedule(
    ExpandedFixtureLedger *ledger, ExpandedFixtureKey key)
{
    ExpandedFixtureLedgerEntry *entry;
    if (!ledger || !ledger->entries || ledger->count > ledger->capacity ||
        !ExpandedFixtureKey_Valid(key))
        return EXP_FIXTURE_INVALID;
    entry = ExpandedFixtureLedger_Find(ledger, key);
    if (entry)
        return entry->state == EXP_FIXTURE_PLAYED ?
            EXP_FIXTURE_ALREADY_PLAYED : EXP_FIXTURE_BUSY;
    if (ledger->count == ledger->capacity)
        return EXP_FIXTURE_FULL;
    entry = &ledger->entries[ledger->count++];
    entry->key = key;
    entry->state = EXP_FIXTURE_SCHEDULED;
    return EXP_FIXTURE_OK;
}
static inline ExpandedFixtureDecision ExpandedFixtureLedger_Begin(
    ExpandedFixtureLedger *ledger, ExpandedFixtureKey key)
{
    ExpandedFixtureLedgerEntry *entry = ExpandedFixtureLedger_Find(ledger, key);
    if (!ExpandedFixtureKey_Valid(key) || !entry)
        return EXP_FIXTURE_INVALID;
    if (entry->state == EXP_FIXTURE_PLAYED)
        return EXP_FIXTURE_ALREADY_PLAYED;
    if (entry->state != EXP_FIXTURE_SCHEDULED)
        return EXP_FIXTURE_BUSY;
    entry->state = EXP_FIXTURE_CLAIMED;
    return EXP_FIXTURE_OK;
}
static inline ExpandedFixtureDecision ExpandedFixtureLedger_Finish(
    ExpandedFixtureLedger *ledger, ExpandedFixtureKey key, ExpandedFixtureScore score)
{
    ExpandedFixtureLedgerEntry *entry = ExpandedFixtureLedger_Find(ledger, key);
    if (!ExpandedFixtureKey_Valid(key) || !entry)
        return EXP_FIXTURE_INVALID;
    if (entry->state == EXP_FIXTURE_PLAYED)
        return ExpandedFixtureScore_Equal(entry->score, score) ?
            EXP_FIXTURE_ALREADY_PLAYED : EXP_FIXTURE_CONFLICT;
    if (entry->state != EXP_FIXTURE_CLAIMED)
        return EXP_FIXTURE_BUSY;
    entry->score = score;
    entry->state = EXP_FIXTURE_PLAYED;
    return EXP_FIXTURE_OK;
}
/* Only when OG has not changed a single persistent field. A partially
 * simulated fixture must NOT be reset and replayed without full rollback. */
static inline ExpandedFixtureDecision ExpandedFixtureLedger_ReleaseUnchanged(
    ExpandedFixtureLedger *ledger, ExpandedFixtureKey key)
{
    ExpandedFixtureLedgerEntry *entry = ExpandedFixtureLedger_Find(ledger, key);
    if (!ExpandedFixtureKey_Valid(key) || !entry)
        return EXP_FIXTURE_INVALID;
    if (entry->state == EXP_FIXTURE_PLAYED)
        return EXP_FIXTURE_ALREADY_PLAYED;
    if (entry->state != EXP_FIXTURE_CLAIMED)
        return EXP_FIXTURE_BUSY;
    entry->state = EXP_FIXTURE_SCHEDULED;
    return EXP_FIXTURE_OK;
}
#endif
