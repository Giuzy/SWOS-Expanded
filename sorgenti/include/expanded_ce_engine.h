/* SWOS Expanded — Fase 2: universal Competition Engine lifecycle.
 *
 * ONE generic engine for every competition type of every world:
 *   type0 league (single table, one phase, divisions chained by
 *         promotion/relegation), type1 knockout cup, type2 multi-phase
 *         (table/group phases and knockout phases in any order).
 * It owns: definitions (from data), editions and their state machine
 *   CREATED -> PARTICIPANTS_CONFIRMED -> SCHEDULED -> ACTIVE -> COMPLETED
 *   -> ARCHIVED,
 * phase generation, qualification/promotion with provenance, per-edition
 * blocks with dependency propagation, and the UL01 persistent section.
 * It NEVER produces a score: results come only from the CE ledger, written
 * by RunDue with the original OG QUICK / 50 Hz witness.
 *
 * Pure C11. Registry mutations go through ExpCeHost (runtime: the CAR
 * module; tests: the same CAR module or mocks).
 */
#ifndef EXPANDED_CE_ENGINE_H
#define EXPANDED_CE_ENGINE_H
#include <stddef.h>
#include <stdint.h>
#include "expanded_ce_rules.h"
#include "expanded_ce_table.h"
#include "expanded_universal_fixture_registry.h"

#define EXP_CE_MAX_PHASES 8u
#define EXP_CE_MAX_GROUPS 16u
#define EXP_CE_MAX_ENTRANTS 128u
#define EXP_CE_MAX_SOURCES 16u
#define EXP_CE_MAX_POOL 40u
#define EXP_CE_MAX_EXCLUDE 8u
#define EXP_CE_MAX_BREAKS 8u

typedef enum ExpCeEditionState {
    EXP_CE_ED_CREATED = 1,
    EXP_CE_ED_PARTICIPANTS = 2,   /* PARTICIPANTS_CONFIRMED */
    EXP_CE_ED_SCHEDULED = 3,
    EXP_CE_ED_ACTIVE = 4,
    EXP_CE_ED_COMPLETED = 5,
    EXP_CE_ED_ARCHIVED = 6
} ExpCeEditionState;

typedef enum ExpCeBlock {
    EXP_CE_BLOCK_NONE = 0,
    EXP_CE_BLOCK_DATA_MISSING = 1,
    EXP_CE_BLOCK_RULES_INVALID = 2,
    EXP_CE_BLOCK_RULES_CHANGED = 3,   /* data version differs from edition */
    EXP_CE_BLOCK_DATE_PLAN = 4,
    EXP_CE_BLOCK_COLLISION = 5,       /* same club, same day */
    EXP_CE_BLOCK_DEPENDENCY = 6,      /* a source edition is blocked */
    EXP_CE_BLOCK_TIE_UNRESOLVED = 7,
    EXP_CE_BLOCK_ROSTER = 8,
    EXP_CE_BLOCK_ENTRANTS = 9,
    EXP_CE_BLOCK_MATCH_REFUSED = 10,  /* pre-claim refusal of a fixture */
    EXP_CE_BLOCK_CAPACITY = 11
} ExpCeBlock;

typedef enum ExpCePhaseKind {
    EXP_CE_PHASE_TABLE = 1,       /* league or groups (round robin) */
    EXP_CE_PHASE_KNOCKOUT = 2
} ExpCePhaseKind;

typedef enum ExpCePairing {
    EXP_CE_PAIR_LISTED = 1,       /* explicit pairs/groups in edition data */
    EXP_CE_PAIR_SEEDED = 2,       /* 1 v N, 2 v N-1 ... by entrant order */
    EXP_CE_PAIR_CROSS_GROUPS = 3, /* A1-B2, B1-A2, C1-D2, D1-C2 ... */
    EXP_CE_PAIR_BRACKET = 4,      /* winners of ties 1-2, 3-4 ... */
    EXP_CE_PAIR_DRAW = 5,         /* deterministic CE game draw */
    EXP_CE_PAIR_POTS = 6          /* groups: one club per pot, game draw */
} ExpCePairing;

typedef enum ExpCeSource {
    EXP_CE_SRC_LISTED = 1,        /* certified list in data (first edition) */
    EXP_CE_SRC_START_DIVISION = 2,/* the division at career START (TEAM) */
    EXP_CE_SRC_RETAINED = 3,      /* stayed in the division */
    EXP_CE_SRC_PROMOTED = 4,
    EXP_CE_SRC_RELEGATED = 5,
    EXP_CE_SRC_LEAGUE_RANK = 6,   /* final rank of a league edition */
    EXP_CE_SRC_CUP_WINNER = 7,
    EXP_CE_SRC_CUP_RUNNER_UP = 8,
    EXP_CE_SRC_PHASE_RANK = 9,    /* rank in a group of the previous phase */
    EXP_CE_SRC_PHASE_WINNER = 10, /* knockout winner of the previous phase */
    EXP_CE_SRC_BYE = 11,
    EXP_CE_SRC_REALLOCATED = 12,  /* next rank after a duplicate */
    EXP_CE_SRC_JOIN = 13,         /* enters directly at a later phase */
    EXP_CE_SRC_KEPT_NO_MOVEMENT = 14 /* previous edition was partial (began
                                        before START): same clubs, no
                                        promotion/relegation applied */
} ExpCeSource;

typedef struct ExpCeEntrant {
    ExpandedClubKey club;
    uint8_t source;               /* ExpCeSource */
    uint8_t rank;                 /* rank in the source, 0 if none */
    uint8_t group;                /* LISTED group (1..), 0 none */
    uint8_t seed;                 /* order within source (1..), 0 none */
    uint8_t join_phase;           /* 0 = first phase, k = enters at phase k */
    uint32_t from_competition;    /* source competition id, 0 none */
    int32_t from_start_day;       /* source edition start day */
} ExpCeEntrant;

typedef struct ExpCeWindow {
    uint8_t first_m, first_d, last_m, last_d, last_year_offset;
    uint8_t weekdays, extra_weekdays, rest, nbreaks;
    uint8_t breaks[EXP_CE_MAX_BREAKS][4]; /* from_m, from_d, to_m, to_d */
} ExpCeWindow;

#include "expanded_universal_pairings.h"
typedef struct ExpCePhaseDef {
    uint8_t kind;                 /* ExpCePhaseKind */
    uint8_t groups;               /* TABLE: number of groups (>=1) */
    uint8_t meetings;             /* TABLE: round robin meetings 1..4 */
    ExpPairExtras extras;
    uint8_t qualify;              /* TABLE: qualifiers per group to next */
    uint8_t pairing;              /* ExpCePairing (KO or group formation) */
    uint8_t byes;                 /* KO: best seeds skip this phase */
    uint8_t higher_home;          /* KO: 1 higher seed hosts leg 1 (or the
                                     single match), 2 higher seed hosts the
                                     deciding leg / lower hosts a single one */
    /* v301 host rule (JSON host_rule NON_MEMBER_HOSTS): in a single-leg KO
     * round, when exactly ONE club of a pair belongs to the current edition
     * of this competition (e.g. the A-League), the OTHER club hosts. 0 =
     * none. Never applied to the last phase (neutral final). */
    uint32_t host_nonmember_comp;
    uint8_t has_window;
    ExpCeWindow window;           /* dates (GAME_TEMPLATE unless verified) */
    /* verified real round days of the FIRST edition (take precedence over
     * the window for that edition only; later editions use the window) */
    uint8_t ndays;
    int32_t days[16];
    ExpCeTableRules table;        /* TABLE phases */
    ExpCeMatchRules match;
    char missing[80];             /* non-empty: data missing for this phase */
} ExpCePhaseDef;

typedef struct ExpCeSourceSpec {  /* participants of the NEXT editions */
    uint8_t source;               /* LEAGUE_RANK / CUP_WINNER / CUP_RUNNER_UP */
    uint8_t rank_from, rank_to;   /* LEAGUE_RANK inclusive */
    uint8_t reallocate;           /* duplicate -> next rank of the source */
    uint32_t competition;
    /* v301 generic additions (JSON next_entrants):
     *  - LEAGUE_RANK "exclude": clubs that are skipped WITHOUT consuming a
     *    wanted slot (e.g. clubs of another federation);
     *  - LISTED: an ordered POOL of clubs ("clubs") of which the first
     *    "count" not already qualified are taken (placeholder TEST clubs for
     *    qualification paths the game does not represent yet). No random
     *    choice, no duplicate, an exhausted pool blocks the edition. */
    uint8_t count;                /* LISTED: clubs wanted */
    uint8_t nclubs;               /* LISTED: pool size */
    uint8_t nexclude;
    uint32_t clubs[EXP_CE_MAX_POOL];
    uint32_t exclude[EXP_CE_MAX_EXCLUDE];
} ExpCeSourceSpec;

typedef enum ExpCeOrigin {
    EXP_CE_ORIGIN_LEAGUE = 1,     /* leagues.json + league_calendar.json */
    EXP_CE_ORIGIN_JSON = 2        /* ce_competitions.json */
} ExpCeOrigin;

typedef struct ExpCeDef {
    uint32_t id;                  /* stable competition id (CE key) */
    uint16_t world;
    uint8_t country, type, origin;
    uint8_t viewer_button;        /* CE button in WORLD/VIEW COMPETITIONS */
    uint16_t viewer_native_id_plus_one; /* optional original menu alias, not a sporting ID */
    char name[40];
    /* league family link (origin LEAGUE) */
    uint8_t comp_id, division, divisions, promoted, relegated;
    uint16_t clubs;
    uint8_t nphases;
    ExpCePhaseDef phases[EXP_CE_MAX_PHASES];
    uint8_t nsources;
    ExpCeSourceSpec sources[EXP_CE_MAX_SOURCES];
    uint8_t has_expiry, expiry_m, expiry_d, expiry_year_offset;
    /* first edition (JSON) */
    uint16_t first_year;
    uint32_t nfirst;
    ExpCeEntrant first[EXP_CE_MAX_ENTRANTS];
    uint8_t first_pairs_listed;   /* first[] order = pairs/groups as listed */
    uint64_t rules_hash;
    char missing[96];             /* non-empty: definition cannot run */
    char source_file[48];
} ExpCeDef;

typedef struct ExpCeEdition {
    ExpandedEditionKey key;
    uint32_t def_id;
    uint16_t year;
    uint8_t state, phase, block, phase_ready;
    uint8_t successor;            /* 1 = next edition already created */
    uint8_t partial;              /* first edition began before career START:
                                     earlier results unavailable -> its final
                                     table cannot promote/qualify anyone */
    uint8_t participant_kind;     /* v299: ExpandedParticipantKind shared by
                                     every entrant (club OR national) */
    uint64_t rules_hash;
    int32_t expiry_day;
    uint32_t nentrants;
    ExpCeEntrant entrants[EXP_CE_MAX_ENTRANTS];
    uint32_t nfinal;
    uint32_t final_order[EXP_CE_MAX_ENTRANTS]; /* stable_club, rank order */
    char why[96];                 /* block reason (diagnostic) */
} ExpCeEdition;

typedef struct ExpCePostponement {
    uint64_t event_id;
    int32_t original_day, new_day;
    uint8_t reason;               /* ExpCeBlock that caused it */
} ExpCePostponement;

typedef struct ExpCeHost {
    /* Insert fixtures atomically (all or none), keeping registry invariants
     * (sorted events, paired ledger, no same club/day). 1 ok. */
    int (*insert)(void *user,const ExpandedUniversalFixture *f,uint32_t n);
    /* Remove every (PLAYED) fixture of an ARCHIVED edition. 1 ok. */
    int (*prune)(void *user,ExpandedEditionKey edition);
    /* Hold (1) or release (0) every pending event of an edition. */
    int (*hold)(void *user,ExpandedEditionKey edition,int hold);
    /* Move one HELD/PENDING fixture to a new civil day. */
    int (*reschedule)(void *user,uint64_t event_id,int32_t new_day);
    int (*archive)(void *user,ExpandedEditionKey edition,uint32_t champion,
                   uint32_t runner_up,int32_t closed_day);
    int (*club_ok)(void *user,ExpandedClubKey club); /* roster present */
    ExpCeNameCompare name_cmp;
    void *user;
    /* clubs of a division at career START (catalog/TEAM), for a league
     * whose first season ended before START. Returns the count (0 none).
     * Optional (NULL = such leagues stay blocked). */
    uint32_t (*division_clubs)(void *user,uint8_t country,uint8_t division,
                               ExpandedClubKey *out,uint32_t cap);
} ExpCeHost;

/* ---- engine state (one per process; world-tagged) ---- */
void ExpCe_Reset(void);
int ExpCe_AddDefinition(const ExpCeDef *def);   /* validates, keeps a copy */
uint32_t ExpCe_DefinitionCount(void);
const ExpCeDef *ExpCe_DefinitionAt(uint32_t i);
const ExpCeDef *ExpCe_Definition(uint32_t id);
uint32_t ExpCe_EditionCount(void);
const ExpCeEdition *ExpCe_EditionAt(uint32_t i);
const ExpCeEdition *ExpCe_Edition(ExpandedEditionKey key);
uint32_t ExpCe_PostponementCount(void);
const ExpCePostponement *ExpCe_PostponementAt(uint32_t i);

/* JSON definitions (ce_competitions.json). Returns definitions added;
 * malformed definitions are added WITH `missing` set (visible in the
 * capability matrix, never run). -1 on a malformed file. */
int ExpCe_LoadJsonDefinitions(const char *json,int len,uint16_t world,
                              const char *source_file);
/* League family definition from the OG league row (leagues.json). */
int ExpCe_LeagueDefinition(uint16_t world,uint8_t country,uint8_t comp_id,
    uint8_t division,uint8_t divisions,uint16_t clubs,uint8_t meetings,
    uint8_t points_for_win,uint8_t subs,uint8_t bench,uint8_t promoted,
    uint8_t relegated,uint8_t promoted_po,uint8_t relegated_po,
    const ExpCeWindow *window,ExpCeDef *out);
uint32_t ExpCe_LeagueCompetitionId(uint8_t country,uint8_t comp_id,
                                   uint8_t division);
/* Runtime builder (expanded_ce_definitions.c, SDL build): reset + league
 * divisions (ExpandedUniversalJsonLeagues, must be loaded) + the world's
 * ce_competitions.json. Returns league definitions added, -1 when the
 * JSON file exists but is malformed (leagues still added). */
int ExpCe_BuildWorldDefinitions(uint16_t world);
int ExpCe_LeagueFamilyChainOk(uint8_t country,uint8_t comp_id);
int ExpCe_PlanPhaseDays(const ExpCeDef *d,uint16_t year,uint32_t phase,
                        int32_t earliest,uint32_t rounds,int32_t *days);

/* START: register the first editions. League editions already present in
 * the registry (catalog fixtures) become ACTIVE/SCHEDULED; leagues without
 * fixtures and JSON definitions with missing data become blocked editions;
 * other JSON first editions are CREATED and generated by Tick. */
int ExpCe_BeginWorld(ExpandedUniversalRegistry *r,const ExpCeHost *host,
                     uint16_t world,uint16_t first_year);
/* Advance the lifecycle up to `today` (idempotent; never plays a match).
 * Returns the number of transitions made, -1 on an internal fault. */
int ExpCe_Tick(ExpandedUniversalRegistry *r,const ExpCeHost *host,
               int32_t today);

/* Rules of one fixture (its competition + phase). 1 ok. */
int ExpCe_RulesForFixture(const ExpandedUniversalFixture *f,
                          ExpCeMatchRules *match,ExpCeTableRules *table);
/* Phase shape of a fixture: kind (TABLE/KNOCKOUT) and meetings (OG quick
 * scorer: a single-meeting table is played without home advantage, as OG
 * does for playEachTeam==1). 1 ok. */
int ExpCe_FixturePhase(const ExpandedUniversalFixture *f,uint8_t *kind,
                       uint8_t *meetings);
/* Competition name (from the CE definition) of a fixture, for readers. */
int ExpCe_FixtureCompetitionName(const ExpandedUniversalFixture *f,
                                 char *out,size_t n);
/* Table rules of (competition, stage), for readers. */
int ExpCe_TableRules(ExpandedEditionKey edition,uint32_t stage,
                     ExpCeTableRules *table);
/* Local fault: block the edition (+ dependents), hold its events. */
int ExpCe_BlockEdition(ExpandedUniversalRegistry *r,const ExpCeHost *host,
                       ExpandedEditionKey edition,uint8_t block,
                       const char *why);
int ExpCe_EditionBlocked(ExpandedEditionKey edition);

/* UL01 persistent section (hex text inside the CAR JSON). */
char *ExpCe_EncodeHex(uint16_t world,size_t *len);        /* malloc'd */
int ExpCe_DecodeHex(const char *hex,size_t len,uint16_t world);
/* Same checks as Decode, engine state untouched (CAR preflight). */
int ExpCe_ValidateHex(const char *hex,size_t len,uint16_t world);
/* Deep copy of the persistent engine state (CAR snapshot path). */
typedef struct ExpCeSnapshot ExpCeSnapshot;
ExpCeSnapshot *ExpCe_SnapshotCapture(void);

char *ExpCe_SnapshotEncodeHex(const ExpCeSnapshot *s,uint16_t world,size_t *len);
void ExpCe_SnapshotRelease(ExpCeSnapshot *s);

/* After CONTINUE: check every edition against the CURRENT definitions
 * (rules hash) and re-validate blocked ones. */
int ExpCe_ValidateAfterRestore(ExpandedUniversalRegistry *r,
                               const ExpCeHost *host);

const char *ExpCe_StateName(uint8_t state);
const char *ExpCe_BlockName(uint8_t block);
const char *ExpCe_SourceName(uint8_t source);
#endif
