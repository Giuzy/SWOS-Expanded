/* Universal Competition Engine state carried inside the SAME career CAR.
 * No sidecar, no replacement for Zstandard or swos_io_save_atomic.
 * Dynamic per-world registry; absent/null field keeps legacy CAR compatible.
 */
#ifndef EXPANDED_UNIVERSAL_CAR_H
#define EXPANDED_UNIVERSAL_CAR_H
#ifdef SWOS_SDL_BUILD
#include "expanded_universal_fixture_registry.h"
#include "mjson.h"
#include <stddef.h>

/* Register verified fixture INPUTS on the active world only; this cannot
 * start a second game, change the career calendar or invent a score. */
ExpandedUniversalRegistry *ExpandedUniversal_CarRegistry(void);
int ExpandedUniversal_CarBegin(ExpandedWorldDate date,uint32_t capacity);
void ExpandedUniversal_CarResetCurrent(void);

/* SaveJson_WriteCareer_Impl calls Emit before the closing JSON object.
 * The same prewrite validator, Zstd3 roundtrip and atomic writer remain
 * responsible for durability. Old CAR without this optional field is valid. */
int ExpandedUniversal_CarEmit(mjson_print_fn_t output,void *user);
/* v288: immutable snapshot of the CE registry for the CarWriter thread.
 * Capture on the main thread between completed events; Emit/Release may run
 * anywhere (they read only the snapshot). Capture of an inactive registry
 * returns 1 with *out==NULL (Emit then writes null, like the live path). */
typedef struct ExpandedUniversalCarSnapshot ExpandedUniversalCarSnapshot;
int ExpandedUniversal_CarSnapshotCapture(ExpandedUniversalCarSnapshot **out);
size_t ExpandedUniversal_CarSnapshotBytes(const ExpandedUniversalCarSnapshot *s);
int ExpandedUniversal_CarEmitSnapshot(ExpandedUniversalCarSnapshot *s,
                                      mjson_print_fn_t output,void *user);
void ExpandedUniversal_CarSnapshotRelease(ExpandedUniversalCarSnapshot *s);
/* Read-only integrity check BEFORE any legacy/OG world mutates on load. */
int ExpandedUniversal_CarValidate(const char *json,int size);
/* Apply only after the preflight checks passed (no replay). */
int ExpandedUniversal_CarRestore(const char *json,int size);
/* v284 side-effect-free helpers for a TRANSACTIONAL CONTINUE: they parse the
 * candidate CAR into scratch memory and never touch the live registry. */
/* 1 = CE section present; *day = its civil clock. */
/* v290: synthetic edition archive (per world, persisted in the CAR as
 * $.universal_archive; absent = empty). CloseEdition archives an edition ONLY
 * if every fixture of it is PLAYED in the live registry and champion/runner-up
 * took part (returns ExpandedArchiveResult). It never closes anything by
 * itself. */
#include "expanded_edition_archive.h"
int ExpandedUniversal_ArchiveCloseEdition(ExpandedEditionKey edition,
    uint32_t champion,uint32_t runner_up,int32_t closed_day);
uint32_t ExpandedUniversal_ArchiveCount(void);
const ExpandedEditionSummary *ExpandedUniversal_ArchiveAt(uint32_t index);
int ExpandedUniversal_ArchiveContains(ExpandedEditionKey edition);
int ExpandedUniversal_CarPeekClock(const char *json,int size,int32_t *day);
/* v289: world passed explicitly, so a worker thread never reads World_Current(). */
int ExpandedUniversal_CarValidateFor(const char *json,int size,uint16_t world);
int ExpandedUniversal_CarPeekClockFor(const char *json,int size,uint16_t world,
                                      int32_t *day);
uint16_t ExpandedUniversal_CarSnapshotWorld(const ExpandedUniversalCarSnapshot *s);
/* 1 = every fixture club of the CE section is in sorted_keys (stable_club). */
int ExpandedUniversal_CarCoversClubs(const char *json,int size,
    const uint32_t *sorted_keys,uint32_t n);
/* After a restore failed past the point of no return: lock the live
 * registry (no play, no saves) instead of continuing half-restored. */
void ExpandedUniversal_CarMarkRecovery(void);
/* Fase 2 — Competition Engine services (registry of the current world).
 * Lifecycle text ("UL01" hex, without prefix) read from the restored CAR,
 * ownership to the caller; NULL = absent (pre-v296 CAR). */
char *ExpandedUniversal_CarTakeLifecycle(void);
/* FIFA ranking (2627): "FR01;" text read from the CAR (NULL = absent),
 * and the one recorder called by the CE completion writers. */
char *ExpandedUniversal_CarTakeFifa(void);
/* v00.13.355 discipline section ("DS01;") and its recorder */
char *ExpandedUniversal_CarTakeDiscipline(void);
int ExpandedUniversal_DisciplineRecord(const ExpandedUniversalFixture *f,
                                       const ExpandedUniversalMatchWitness *w);
struct ExpFifaUndo;
int ExpandedUniversal_FifaRecord(const ExpandedUniversalFixture *f,
                                 const ExpandedFixtureScore *sc,struct ExpFifaUndo *undo);
/* Atomic insert (all or none) with the full registry proof. */
int ExpandedUniversal_CarInsertFixtures(const ExpandedUniversalFixture *f,
                                        uint32_t n);
/* Remove the (all PLAYED) fixtures of an archived edition. */
int ExpandedUniversal_CarPruneEdition(ExpandedEditionKey edition);
/* HELD (1) / PENDING (0) for the SCHEDULED fixtures of an edition. */
int ExpandedUniversal_CarHoldEdition(ExpandedEditionKey edition,int hold);
int ExpandedUniversal_CarRescheduleEvent(uint64_t event_id,int32_t new_day);

/* Real CAR callbacks for the generic dispatcher, not fake acknowledgements.
 * Claim = durable CLAIMED before original game; Complete = persist proposed
 * PLAYED+event COMPLETE+OG/RNG/state in exactly one existing AUTO*.CAR.
 * The in-memory proposal is restored until RunDue finishes its own ACK. */
/* Atomic transaction after a full CPU quick block has completed in RAM.
 * A crash before this call commits nothing from the current block. */
int ExpandedUniversal_CarSaveCpuBatch(uint32_t completed_count);
/* v287: same, optionally written by the CarWriter thread (async=1). */
int ExpandedUniversal_CarSaveCpuBatchEx(uint32_t completed_count,int async);
/* v284: put a CLAIMED fixture back to SCHEDULED on disk after a clean manual
 * abort of the 50 Hz match (original OG behaviour). */
int ExpandedUniversal_CarSaveRelease(void *user,
    const ExpandedUniversalRegistry *registry,
    const ExpandedUniversalFixture *fixture);
int ExpandedUniversal_CarSaveClaim(void *user,
    const ExpandedUniversalRegistry *registry,
    const ExpandedUniversalFixture *fixture);
int ExpandedUniversal_CarSaveCompletion(void *user,
    const ExpandedUniversalRegistry *registry,
    const ExpandedUniversalFixture *fixture,
    const ExpandedUniversalCompletion *proposal);
#endif
#endif
