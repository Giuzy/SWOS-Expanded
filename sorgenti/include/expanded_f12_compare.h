#ifndef EXPANDED_F12_COMPARE_H
#define EXPANDED_F12_COMPARE_H
/* Developer-only, read-only comparison of OG resident competition identities.
 * No pointers to OG objects, simulation, RNG, or persistence. */
#include <stdint.h>
#include <string.h>
#define EXP_F12_MAX_CONTEXTS 800u
/* 0 = domestic league, 1 = domestic/UEFA knockout cup,
 * 2 = UEFA type-2 or historical Copa type-2. */
typedef struct {
    uint8_t kind, country, slot, viewable, phase;
    uint16_t year;
    uint32_t matches;
} ExpandedF12Context;
typedef struct {
    uint8_t valid, world;
    uint16_t year, count;
    ExpandedF12Context rows[EXP_F12_MAX_CONTEXTS];
} ExpandedF12Snapshot;
typedef struct {
    uint32_t checked, missing, appeared, viewLost, viewGained;
    uint32_t matchesBack, phaseBack, duplicates;
    uint8_t comparable;
} ExpandedF12Diff;
static inline int ExpandedF12_SameIdentity(const ExpandedF12Context *a,
                                           const ExpandedF12Context *b)
{
    return a->kind==b->kind && a->country==b->country &&
           a->slot==b->slot && a->year==b->year;
}
static inline const ExpandedF12Context *ExpandedF12_Find(
    const ExpandedF12Snapshot *s,const ExpandedF12Context *wanted)
{
    for(uint16_t j=0;j<s->count;++j)
        if(ExpandedF12_SameIdentity(&s->rows[j],wanted))return &s->rows[j];
    return NULL;
}
static inline void ExpandedF12_Compare(const ExpandedF12Snapshot *before,
                                       const ExpandedF12Snapshot *after,
                                       ExpandedF12Diff *out)
{
    if(!out)return;
    memset(out,0,sizeof(*out));
    if(!before || !after || !before->valid || !after->valid ||
       before->world!=after->world || before->year!=after->year ||
       before->count>EXP_F12_MAX_CONTEXTS ||
       after->count>EXP_F12_MAX_CONTEXTS)return;
    out->comparable=1u;
    for(uint16_t i=0;i<before->count;++i) {
        const ExpandedF12Context *b=&before->rows[i];
        for(uint16_t j=0;j<i;++j)
            if(ExpandedF12_SameIdentity(b,&before->rows[j]))++out->duplicates;
        const ExpandedF12Context *a=ExpandedF12_Find(after,b);
        if(!a){++out->missing;continue;}
        ++out->checked;
        if(b->viewable && !a->viewable)++out->viewLost;
        if(!b->viewable && a->viewable)++out->viewGained;
        if(a->matches<b->matches)++out->matchesBack;
        if(a->phase<b->phase)++out->phaseBack;
    }
    for(uint16_t i=0;i<after->count;++i) {
        const ExpandedF12Context *a=&after->rows[i];
        for(uint16_t j=0;j<i;++j)
            if(ExpandedF12_SameIdentity(a,&after->rows[j]))++out->duplicates;
        if(!ExpandedF12_Find(before,a))++out->appeared;
    }
}
#endif
