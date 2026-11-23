/* v00.13.290: persistent SYNTHETIC EDITION ARCHIVE (structure, validation,
 * binary codec). Implements the approved retention contract
 * (docs/COMPETITION_ENGINE_HISTORY_RETENTION_20261118.md): ONE record per
 * COMPLETED edition = ExpandedEditionSummary {edition id, closed day,
 * champion, runner-up, match count, qualification-facts flag}; no per-match
 * data. PURE module (no game globals, no I/O).
 * It never decides that a competition is over: Close() is a gate that accepts
 * a closure only if the registry proves EVERY fixture of the edition is PLAYED
 * and the supplied champion/runner-up took part; the caller (future
 * edition-closing code) must derive them from authentic OG results. */
#ifndef EXPANDED_EDITION_ARCHIVE_H
#define EXPANDED_EDITION_ARCHIVE_H
#include <stddef.h>
#include <stdint.h>
#include "expanded_competition_history.h"
#include "expanded_universal_fixture_registry.h"

typedef struct ExpandedEditionArchive {
    ExpandedEditionSummary *items;   /* sorted by edition key, unique */
    uint32_t count,cap;
} ExpandedEditionArchive;

typedef enum ExpandedArchiveResult {
    EXP_ARCHIVE_INVALID=-3, EXP_ARCHIVE_CONFLICT=-2, EXP_ARCHIVE_FULL=-1,
    EXP_ARCHIVE_OK=0, EXP_ARCHIVE_ALREADY=1,
    EXP_ARCHIVE_OPEN=2,       /* edition still has unplayed fixtures */
    EXP_ARCHIVE_UNKNOWN=3     /* no fixture of that edition in the registry */
} ExpandedArchiveResult;

void ExpArchive_Free(ExpandedEditionArchive *a);
const ExpandedEditionSummary *ExpArchive_Find(const ExpandedEditionArchive *a,
                                              ExpandedEditionKey edition);
/* Field rules: valid edition, closed_day>=start_day, champion!=0,
 * runner_up!=champion, match_count>0. Identical re-insert = ALREADY,
 * different record for the same edition = CONFLICT (never overwritten). */
ExpandedArchiveResult ExpArchive_Insert(ExpandedEditionArchive *a,
                                        const ExpandedEditionSummary *s);
/* Gate: archive an edition ONLY if the registry proves it is complete. */
ExpandedArchiveResult ExpArchive_CloseFromRegistry(ExpandedEditionArchive *a,
    const ExpandedUniversalFixture *fx,const ExpandedFixtureLedgerEntry *led,
    uint32_t n,ExpandedEditionKey edition,uint32_t champion,uint32_t runner_up,
    int32_t closed_day);
/* Anti-replay: every registry fixture of an archived edition must be PLAYED
 * (a SCHEDULED/CLAIMED one would let a closed edition be played again). */
int ExpArchive_AuditFixtures(const ExpandedEditionArchive *a,
    const ExpandedUniversalFixture *fx,const ExpandedFixtureLedgerEntry *led,
    uint32_t n);
int ExpArchive_Validate(const ExpandedEditionArchive *a,uint16_t world);
/* "UA01" binary: LE fields, FNV-1a/64 trailer. malloc'd; NULL on failure. */
uint8_t *ExpArchive_Encode(const ExpandedEditionArchive *a,uint16_t world,
                           size_t *len);
/* Decodes AND validates into a (previous contents freed). 0 on any defect. */
int ExpArchive_Decode(ExpandedEditionArchive *a,const uint8_t *buf,size_t len,
                      uint16_t expected_world);
/* Deep copy (snapshots). 0 on allocation failure. */
int ExpArchive_Copy(ExpandedEditionArchive *dst,const ExpandedEditionArchive *src);
#endif
