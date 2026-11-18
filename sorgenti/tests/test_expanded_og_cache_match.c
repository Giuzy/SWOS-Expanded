/* Isolated OG manager-cache fixture lookup; no live engine, no CAR. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "expanded_og_cache_match.h"
#include "expanded_og_fixture_history.h"
static void fixture(unsigned char *p,unsigned phase,unsigned comp,
                    unsigned home,unsigned away,int played,
                    unsigned gh,unsigned ga)
{
    const unsigned short stride=22;
    memset(p,0,22u);
    p[0]=(unsigned char)comp;p[1]=(unsigned char)phase;
    memcpy(p+2,&stride,sizeof stride);
    p[5]=1;p[6]=(unsigned char)(played?0:1);
    p[10]=1;p[11]=(unsigned char)(played?0x82:0);
    p[12]=(unsigned char)(home>>8);p[13]=(unsigned char)home;
    p[14]=(unsigned char)(away>>8);p[15]=(unsigned char)away;
    const unsigned short score=(unsigned short)((gh<<8)|ga);
    memcpy(p+16,&score,sizeof score);
}
int main(void)
{
    unsigned char b[66];
    ExpandedOGCacheMatch m;
    ExpandedOGCacheStatus s;
    fixture(b,48,0,0x0102,0x0304,1,2,1);
    fixture(b+22,56,0,0x0102,0x0304,0,0,0);
    fixture(b+44,48,1,0x0102,0x0304,1,5,0);
    s=ExpandedOGCache_Find(b,sizeof b,3,0,48,0x0102,0x0304,&m);
    assert(s==EXP_OG_CACHE_PLAYED && m.packed_goals==0x0201 &&
           m.flags==0x82 && m.record_end==22u && m.used_bytes==66u);
    assert(ExpandedOGCache_Find(b,sizeof b,3,0,56,0x0102,0x0304,&m)==EXP_OG_CACHE_SCHEDULED);
    assert(ExpandedOGCache_Find(b,sizeof b,3,0,48,0x0304,0x0102,&m)==EXP_OG_CACHE_NOT_FOUND);
    assert(ExpandedOGCache_Find(b,sizeof b,3,2,48,0x0102,0x0304,&m)==EXP_OG_CACHE_NOT_FOUND);
    assert(ExpandedOGCache_Find(b,sizeof b,3,1,48,0x0102,0x0304,&m)==EXP_OG_CACHE_PLAYED &&
           m.packed_goals==0x0500);
    fixture(b+44,48,0,0x0102,0x0304,1,4,4);
    assert(ExpandedOGCache_Find(b,sizeof b,3,0,48,0x0102,0x0304,&m)==EXP_OG_CACHE_AMBIGUOUS);
    /* Large calendars are legitimate: the old CAR's 1870-byte limit
     * cannot hold 300 * 22 = 6600 bytes, the new 8192-byte blob can. */
    {
        static unsigned char big[EXP_OG_CACHE_CAPACITY];
        size_t used=0u;
        for(unsigned i=0;i<300u;i++)
            fixture(big+i*22u,48u+(i%8u),0u,
                    0x0102u,0x0304u,1,2,1);
        assert(ExpandedOGCache_Measure(big,sizeof big,300u,&used)
               && used==6600u);
        assert(!ExpandedOGCache_Measure(big,EXP_OG_CACHE_LEGACY_SIZE,
                                        300u,&used));
        assert(!ExpandedOGCache_Measure(big,8192u,65535u,&used));
        big[2]=0;big[3]=0;
        assert(!ExpandedOGCache_Measure(big,sizeof big,300u,&used));
    }
    {
        /* Large single OG entry: stride is uint16, not arbitrarily <=256.
         * 30 records require a 370-byte stride and must remain readable. */
        unsigned char large[EXP_OG_CACHE_CAPACITY]={0};
        unsigned short stride=370u;
        size_t used=0u;
        memcpy(large+2,&stride,sizeof stride);
        large[5]=30u;
        assert(ExpandedOGCache_Measure(large,sizeof large,1u,&used)
               && used==370u);
        large[5]=31u;
        assert(!ExpandedOGCache_Measure(large,sizeof large,1u,&used));
    }
    {
        /* Existing shorter CAR: count and occupancy are fully inside
         * the old limit, so it remains loadable after zeroing the tail. */
        static unsigned char legacy[EXP_OG_CACHE_LEGACY_SIZE];
        size_t used=0u;
        for(unsigned i=0;i<36u;i++)
            fixture(legacy+i*22u,48u+(i%8u),0u,
                    0x0102u,0x0304u,1,2,1);
        assert(ExpandedOGCache_Measure(legacy,sizeof legacy,36u,&used)
               && used==792u);
    }

    /* The 0.13.104 Windows CARs contain a played calendar row for the
     * phase but with another pair. It is a TRUE ID mismatch, not an
     * unknown/not-found result or a reason to accept its score. */
    {
        unsigned char phase[44];
        ExpandedOGCacheMatch row;
        fixture(phase,76,0,0x0847,0x0844,1,2,1);
        fixture(phase+22,99,0,0x0857,0x0847,0,0,0);
        assert(ExpandedOGCache_Find(phase,sizeof phase,2,0,76,
                                   0x0857,0x0847,&row)==
                                   EXP_OG_CACHE_NOT_FOUND);
        assert(ExpandedOGCache_FindPhaseSlot(phase,sizeof phase,2,
                                            0,76,&row)==
                                   EXP_OG_CACHE_PLAYED);
        assert(row.home==0x0847 && row.away==0x0844 &&
               row.packed_goals==0x0201 && row.flags==0x82u);
        /* The same oriented pair at another week remains SCHEDULED.
         * Never reinterpret it as the played week 76 result. */
        assert(ExpandedOGCache_FindPhaseSlot(phase,sizeof phase,2,
                                            0,99,&row)==
                                   EXP_OG_CACHE_SCHEDULED);
        assert(row.home==0x0857 && row.away==0x0847);
        assert(ExpandedOGCache_FindPhaseSlot(phase,sizeof phase,2,
                                            0,75,&row)==
                                   EXP_OG_CACHE_NOT_FOUND);
        /* Two distinct fixtures at same slot/phase cannot be inferred
         * to be the correct one without a native fixture identity. */
        phase[23]=76;
        assert(ExpandedOGCache_FindPhaseSlot(phase,sizeof phase,2,
                                            0,76,&row)==
                                   EXP_OG_CACHE_AMBIGUOUS);
        phase[23]=99;
        phase[2]=0;phase[3]=0;
        assert(ExpandedOGCache_FindPhaseSlot(phase,sizeof phase,2,
                                            0,76,&row)==
                                   EXP_OG_CACHE_CORRUPT);
    }


    /* 1996/97: same oriented pair in an older cycle must not count
     * as SCORE_OR_PHASE_DIVERGENCE with the local later fixture.
     * Only a same-phase resident match is a genuine collision. */
    {
        uint8_t journal[3u*20u]={0};
        uint8_t local[20u]={0};
        ExpandedOGFixtureHistory check;
        local[0]=0u;local[1]=33u;local[2]=3u;local[3]=105u;
        local[4]=0x07u;local[5]=0xcdu; /* 1997 */
        local[8]=0x21u;local[9]=0x08u;
        local[10]=0x21u;local[11]=0x0cu;
        local[12]=1u;local[13]=3u;local[15]=2u;
        memcpy(journal,local,sizeof local);
        journal[3]=66u;journal[12]=4u;journal[13]=2u;
        journal[15]=1u; /* same pair, earlier native phase */
        check=ExpandedOGFixtureHistory_Check(journal,1u,local);
        assert(check.historical_other_phase==1u &&
               check.resident_same_phase==0u);
        memcpy(journal+20u,local,sizeof local);
        journal[20u+15u]=1u; /* same phase and pair */
        check=ExpandedOGFixtureHistory_Check(journal,2u,local);
        assert(check.historical_other_phase==1u &&
               check.resident_same_phase==1u &&
               check.resident_same_phase_score_conflict==0u);
        journal[20u+13u]=5u;
        check=ExpandedOGFixtureHistory_Check(journal,2u,local);
        assert(check.resident_same_phase_score_conflict==1u);
    }

    b[2]=0;b[3]=0;
    assert(ExpandedOGCache_Find(b,sizeof b,3,0,48,0x0102,0x0304,&m)==EXP_OG_CACHE_CORRUPT);
    assert(ExpandedOGCache_Find(b,20,1,0,48,0x0102,0x0304,&m)==EXP_OG_CACHE_CORRUPT);
    puts("PASS_ISOLATED_ONLY: OG keycache phase identity, native historical cycle/collision, 1870/8192 validation; no gameplay");
    return 0;
}
