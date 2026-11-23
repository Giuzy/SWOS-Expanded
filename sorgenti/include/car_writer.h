/* SWOS Expanded: ONE ordered writer for the automatic career CAR (SDL).
 *
 * v00.13.287. Takes an already serialised AND validated JSON image, then
 * encodes -> atomically replaces the file -> reads it back and compares it
 * BYTE FOR BYTE with the validated image. It runs inline (synchronous) or on
 * ONE worker thread (asynchronous), through the SAME code path.
 *
 * Invariants (tested in tests/test_car_writer.c):
 *  - single writer, single job in flight: Submit() first waits for the
 *    previous job, so a generation can never overtake or be overwritten by an
 *    older one; generations are strictly increasing;
 *  - the job OWNS its JSON buffer; the worker never reads game globals;
 *  - encode/write/verify are replaceable (CarWriterOps), so a future compact
 *    CAR, cached blocks or roster diffs plug in without touching the callers
 *    or the thread/ordering logic;
 *  - mandatory saves (CLAIMED, completion, exit, quit) use async=0: durable
 *    and verified when Submit returns, after any previous async job finished;
 *  - an async failure is never silent: CarWriter_TakeFailure() reports it
 *    once so the caller can treat its RAM-only backlog as unsaved again.
 *
 * ---- FUTURE (design contract, NOT implemented in this version) ----------
 * Phase 2b "complete immutable snapshot": the main thread would capture a
 * self-sufficient CarSnapshot (owned copies of rosters, CE registry/ledger/
 * events, LWE image and every career field the serializer reads) in a few
 * milliseconds; a pure serializer CarSerialize(const CarSnapshot *) (no
 * globals) would then produce the JSON image ON THE WORKER, and the job
 * below would carry the snapshot instead of a ready buffer
 * (CarWriteJob.kind == CAR_JOB_SNAPSHOT). Compact fixtures, cached blocks and
 * roster diffs are alternative encode()/verify() implementations (or a
 * snapshot serializer variant) selected by CarWriteJob.format, with the
 * container magic deciding how CONTINUE decodes; old CAR stay readable.
 */
#ifndef SWOS_CAR_WRITER_H
#define SWOS_CAR_WRITER_H
#include <stddef.h>
#include <stdint.h>
#ifdef SWOS_SDL_BUILD

typedef enum CarWriteResult {
    CAR_WRITE_OK = 0,
    CAR_WRITE_ENCODE_FAILED = 1,  /* nothing touched on disk */
    CAR_WRITE_IO_FAILED = 2,      /* previous file retained */
    CAR_WRITE_VERIFY_FAILED = 3,  /* file replaced but NOT byte-identical */
    CAR_WRITE_BAD_ARGUMENT = 4,
    CAR_WRITE_PREPARE_FAILED = 5  /* worker could not build/validate the image */
} CarWriteResult;

typedef enum CarJobKind {
    CAR_JOB_JSON_BUFFER = 0       /* today: a validated JSON image */
    /* CAR_JOB_SNAPSHOT = 1       future phase 2b, see the header comment */
} CarJobKind;

typedef enum CarWriteFormat {
    CAR_FORMAT_ZSTD_SINGLE_FRAME = 0 /* 'SWOSZC3A' container, today */
    /* CAR_FORMAT_COMPACT / _BLOCKS / _ROSTER_DIFF: future encoders */
} CarWriteFormat;

/* Replaceable pipeline stages. All run on the thread that executes the job. */
typedef struct CarWriterOps {
    /* json -> malloc'd frame, already frame-verified; 1 = ok */
    int (*encode)(const uint8_t *json,uint32_t len,
                  uint8_t **frame,uint32_t *frame_len);
    void (*free_frame)(uint8_t *frame);
    void (*free_json)(uint8_t *json);
    /* atomic replace; returns 0 on success (previous file kept on failure) */
    int (*write)(const char *name,const uint8_t *frame,uint32_t frame_len);
    /* read the file back and prove it equals the validated image; 1 = ok */
    int (*verify)(const char *name,const uint8_t *json,uint32_t len);
    /* OPTIONAL (v288, snapshot jobs): runs on the executing thread BEFORE
     * encode. Builds the full JSON image from the job context ONLY (an
     * immutable snapshot, never game globals), validates it, and returns a
     * buffer owned by the writer (freed through free_json). 1 = ok. */
    int (*prepare)(void *ctx,uint8_t **json,uint32_t *len);
    void (*free_ctx)(void *ctx);
} CarWriterOps;

typedef struct CarWriterStats {
    uint64_t submitted,completed,failed;
    uint32_t last_prepare_ms;
    uint64_t last_generation;       /* last COMPLETED generation */
    uint32_t last_encode_ms,last_write_ms,last_verify_ms,last_total_ms;
    int last_ok;
} CarWriterStats;

void CarWriter_SetOps(const CarWriterOps *ops); /* required before Submit */
/* Snapshot job: takes ownership of ctx (freed through ops->free_ctx, also on
 * every failure). The image is produced by ops->prepare ON THE EXECUTING
 * THREAD from ctx alone. Same ordering/one-in-flight rules as Submit. */
int CarWriter_SubmitPrepared(const char *name,void *ctx,int async,
                             uint64_t *generation);
/* Takes ownership of json_owned (freed through ops->free_json).
 * async=0: runs inline, returns the CarWriteResult.
 * async=1: waits for the previous job, hands this one to the worker and
 *          returns CAR_WRITE_OK once ACCEPTED (outcome via Wait/Take).
 * generation (optional) receives the job's generation number. */
int CarWriter_Submit(const char *name,uint8_t *json_owned,uint32_t len,
                     int async,uint64_t *generation);
/* Blocks until no job is in flight. 1 = last completed job succeeded. */
int CarWriter_Wait(void);
int CarWriter_Busy(void);
/* 1 once per failed ASYNC job since the last call (then cleared). */
int CarWriter_TakeFailure(void);
void CarWriter_Stats(CarWriterStats *out);
/* Joins the worker (also registered with atexit on first async use). */
void CarWriter_Shutdown(void);

/* ---- v289: pure helpers (no globals, unit-tested) ----------------------- */
/* Final image = main[0,p1s) + a + main[p1e,p2s) + b + main[p2e,main_len).
 * Requires 0<=p1s<=p1e<=p2s<=p2e<=main_len. Returns the image length, or
 * (size_t)-1 on bad ranges / too small cap. out==NULL: returns the length
 * only. A trailing NUL is written when cap allows. */
size_t CarImage_Splice(const char *main,size_t main_len,size_t p1s,size_t p1e,
                       size_t p2s,size_t p2e,const char *a,size_t alen,
                       const char *b,size_t blen,char *out,size_t cap);

/* Self-validating activation of the snapshot path. After `proofs_needed`
 * consecutive saves in which the snapshot serialiser reproduced the inline
 * bytes exactly, the snapshot path turns ON for the rest of the session; one
 * mismatch turns it OFF for the session (inline stays authoritative).
 * Overrides are passed in (environment is read by the caller). */
typedef enum CarGateState {
    CAR_GATE_VERIFYING = 0, CAR_GATE_ON = 1, CAR_GATE_OFF = -1
} CarGateState;
typedef struct CarGate {
    int state,proofs,proofs_needed;
} CarGate;
typedef struct CarGateEnv {
    int force_on;      /* SWOS_CAR_SNAPSHOT: on without proof */
    int verify_only;   /* SWOS_CAR_SNAPSHOT_VERIFY: compare forever, never on */
    int force_off;     /* SWOS_CAR_SNAPSHOT_OFF */
} CarGateEnv;
void CarGate_Init(CarGate *g,int proofs_needed);
/* 1 = use the snapshot for this save; *shadow=1: also keep inline bytes
 * authoritative and compare. 0 = plain inline save. */
int CarGate_Decide(CarGate *g,const CarGateEnv *env,int *shadow);
/* Report the verdict of a shadow comparison (equal=1 byte-identical). */
void CarGate_Report(CarGate *g,const CarGateEnv *env,int equal);
#endif
#endif
