#ifndef EXPANDED_MATCH_BENCH_H
#define EXPANDED_MATCH_BENCH_H
#include <stddef.h>
#include <stdint.h>
#include "game_types.h"

/* Match-only storage. Do not change TeamGame, TeamFile, or CAR ABI. The
 * native eleven + five remain in TeamGame; only excess nominated players
 * live here. PlayerInfo.playerIndex remains the sole roster identity. */
enum { EXP_BENCH_XI=11, EXP_BENCH_OG=5, EXP_BENCH_MAX=15,
       EXP_BENCH_NATIVE=16, EXP_BENCH_EXTRA=10, EXP_BENCH_ROSTER_MAX=32 };
typedef struct ExpMatchBench {
    unsigned extra_count;
    PlayerInfo extra[EXP_BENCH_EXTRA];
} ExpMatchBench;

static inline unsigned ExpBench_Count(const ExpMatchBench *b)
{
    return EXP_BENCH_OG + (b && b->extra_count<=EXP_BENCH_EXTRA ? b->extra_count : 0u);
}

/* A failed registration leaves the old sidecar untouched. The caller must
 * supply real, initialized PlayerInfo from the same roster, in nomination
 * order. No filler players, aliases, or duplicated original slots. */
static inline int ExpBench_Init(ExpMatchBench *b,const TeamGame *native,
                               const PlayerInfo *extra,unsigned n)
{
    if(!b || !native || n>EXP_BENCH_EXTRA || (n && !extra))return 0;
    ExpMatchBench next={0};
    if(n) {
        uint32_t seen=0;
        for(unsigned i=0;i<EXP_BENCH_NATIVE+n;++i) {
            const PlayerInfo *p=i<EXP_BENCH_NATIVE ? &native->players[i] : &extra[i-EXP_BENCH_NATIVE];
            unsigned id=(uint8_t)p->playerIndex;
            if(id>=EXP_BENCH_ROSTER_MAX || (seen & (UINT32_C(1)<<id)))return 0;
            seen|=UINT32_C(1)<<id;
        }
        for(unsigned i=0;i<n;++i)next.extra[i]=extra[i];
    }
    next.extra_count=n;
    *b=next;
    return 1;
}

static inline PlayerInfo *ExpBench_Player(TeamGame *native,ExpMatchBench *b,unsigned slot)
{
    if(!native || slot>=EXP_BENCH_XI+ExpBench_Count(b))return NULL;
    return slot<EXP_BENCH_NATIVE ? &native->players[slot] : &b->extra[slot-EXP_BENCH_NATIVE];
}

/* Same record swap and outgoing position flag as OG's SubstitutePlayer.
 * Keep all 61 bytes, including file identity, name, cards, and goals. The
 * caller still owns the original counters, sprite/mark/name updates and
 * the legality checks for the outgoing player and substitution windows. */
static inline void ExpBench_SwapPlayerRecords(PlayerInfo *enter,PlayerInfo *leave)
{
    PlayerInfo incoming=*enter;
    leave->position=-1;
    *enter=*leave;
    *leave=incoming;
}

static inline int ExpBench_TrySwap(TeamGame *native,ExpMatchBench *b,
                                  unsigned entering,unsigned leaving)
{
    if(entering<EXP_BENCH_XI || leaving>=EXP_BENCH_XI)return 0;
    PlayerInfo *in=ExpBench_Player(native,b,entering),*out=ExpBench_Player(native,b,leaving);
    if(!in || !out || in->skip || in->position<0 || (uint8_t)in->numYellowCards>=2u)return 0;
    ExpBench_SwapPlayerRecords(in,out);
    return 1;
}

/* Index 0 remains the coach. Increasing scans forward, then retreats from
 * the end if there was no legal candidate: exactly OG's legacy navigation.
 * The callback is never called for 0 or for an out-of-range player. */
typedef int (*ExpBench_Selectable)(void *,unsigned);
static inline unsigned ExpBench_Move(unsigned from,unsigned count,int down,
                                    ExpBench_Selectable selectable,void *ctx)
{
    if(count>EXP_BENCH_MAX || from>count || !selectable)return 0;
    unsigned i=from;
    if(down) {
        if(i==count)return i;
        while(++i<=count)if(selectable(ctx,i))return i;
    }
    while(i>1u) {
        --i;
        if(selectable(ctx,i))return i;
    }
    return 0;
}

/* Geometry in the native pitch coordinate system (7 px per seat).
 * Upper bench grows upwards, keeping its lower end. Lower bench grows
 * downwards, keeping its upper end. Coordinates of coach and arrow use
 * the SAME translation; no screen/zoom state is mutated by this helper. */
static inline int ExpBench_SeatY(int legacy_coach_y,unsigned count,int upper,unsigned index)
{
    if(count<EXP_BENCH_OG || count>EXP_BENCH_MAX || index>count)return legacy_coach_y;
    return legacy_coach_y+7*(int)index-(upper ? 7*(int)(count-EXP_BENCH_OG) : 0);
}
static inline int ExpBench_AnchorStart(int old_start,int old_height,int new_height,int upper)
{
    return upper ? old_start+old_height-new_height : old_start;
}
#endif
