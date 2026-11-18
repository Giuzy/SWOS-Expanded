/* Diagnostic-only classification of earlier resident OG fixtures.
 * Never use a home/away pair from a DIFFERENT phase to prove a replay.
 * Journal row (20 bytes): kind/country/div/phase, edition, serial,
 * oriented pair, score, leg, source, observation day.
 * The caller has already appended the new local record; only source=1
 * previously observed resident fixtures are compared. */
#ifndef EXPANDED_OG_FIXTURE_HISTORY_H
#define EXPANDED_OG_FIXTURE_HISTORY_H
#include <stdint.h>
#include <stddef.h>
#include <string.h>
typedef struct {
    uint32_t historical_other_phase;
    uint32_t resident_same_phase;
    uint32_t resident_same_phase_score_conflict;
} ExpandedOGFixtureHistory;
static inline ExpandedOGFixtureHistory ExpandedOGFixtureHistory_Check(
    const uint8_t *wire,size_t count,const uint8_t *local)
{
    ExpandedOGFixtureHistory out={0};
    if(!local || (!wire && count))return out;
    for(size_t i=0;i<count;++i){
        const uint8_t *r=wire+i*20u;
        if(r[15]!=1u || r[0]!=0u ||
           r[1]!=local[1] || r[2]!=local[2] ||
           memcmp(r+4,local+4,2u)!=0 ||
           memcmp(r+8,local+8,4u)!=0)continue;
        if(r[3]!=local[3]){
            ++out.historical_other_phase;continue;
        }
        ++out.resident_same_phase;
        if(r[12]!=local[12] || r[13]!=local[13])
            ++out.resident_same_phase_score_conflict;
    }
    return out;
}
#endif
