/* Isolated OG manager-cache fixture lookup; no live engine, no CAR. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "expanded_og_cache_match.h"
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
    b[2]=0;b[3]=0;
    assert(ExpandedOGCache_Find(b,sizeof b,3,0,48,0x0102,0x0304,&m)==EXP_OG_CACHE_CORRUPT);
    assert(ExpandedOGCache_Find(b,20,1,0,48,0x0102,0x0304,&m)==EXP_OG_CACHE_CORRUPT);
    puts("PASS_ISOLATED_ONLY: OG cache keyed results and 1870/8192 capacity validation; duplicate/corrupt fail closed");
    return 0;
}
