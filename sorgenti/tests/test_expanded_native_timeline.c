/* Isolated integration contract for live OG/civil world ticks.
 * This is NOT proof of the OG match UI or CAR roundtrip (RG-014). */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "expanded_native_timeline.h"

typedef struct TimelineProbe {
    unsigned og,civil,verify;
    int fail;
    int order[3];
    unsigned order_count;
} TimelineProbe;

static int probe_og(ExpandedWorldDate d,void *user)
{
    TimelineProbe *p=(TimelineProbe *)user;
    assert(d.year >= 1996);
    p->og++;
    p->order[p->order_count++]=1;
    return p->fail!=1;
}
static int probe_civil(ExpandedWorldDate d,void *user)
{
    TimelineProbe *p=(TimelineProbe *)user;
    assert(d.month>=1 && d.month<=12);
    p->civil++;
    p->order[p->order_count++]=2;
    return p->fail!=2;
}
static int probe_verify(ExpandedWorldDate d,void *user)
{
    TimelineProbe *p=(TimelineProbe *)user;
    assert(d.day>=1 && d.day<=31);
    p->verify++;
    p->order[p->order_count++]=3;
    return p->fail!=3;
}
static void reset(TimelineProbe *p,int fail)
{
    memset(p,0,sizeof *p);
    p->fail=fail;
}
static ExpandedNativeTimelineResult run(TimelineProbe *probe,
    uint16_t world,ExpandedWorldDate from,ExpandedWorldDate to)
{
    const ExpandedNativeTimelineOps ops={
        probe_og,probe_civil,probe_verify,probe
    };
    return ExpandedNativeTimeline_Advance(world,from,to,&ops);
}
static int32_t day(uint16_t y,uint8_t m,uint8_t d)
{
    int32_t index=-1;
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){y,m,d},&index));
    return index;
}
int main(void)
{
    TimelineProbe p={0};
    const ExpandedWorldDate start={2026,7,1};
    const ExpandedWorldDate next={2026,7,8};
    reset(&p,0);
    assert(run(&p,1,start,next)==EXP_NATIVE_TICK_OK);
    assert(p.og==1 && p.civil==1 && p.verify==1);
    assert(p.order_count==3 && p.order[0]==1 &&
           p.order[1]==2 && p.order[2]==3);
    reset(&p,0);
    assert(run(&p,1,start,start)==EXP_NATIVE_TICK_UNCHANGED);
    assert(p.order_count==0);
    assert(run(&p,1,next,start)==EXP_NATIVE_TICK_INVALID);
    assert(p.order_count==0);
    assert(run(&p,UINT16_MAX,start,next)==EXP_NATIVE_TICK_INVALID);
    assert(p.order_count==0);
    assert(ExpandedNativeTimeline_TickId(1,day(2026,7,8))!=0);
    assert(ExpandedNativeTimeline_TickId(1,day(2026,7,8))!=
           ExpandedNativeTimeline_TickId(0,day(2026,7,8)));
    assert(ExpandedNativeTimeline_TickId(1,day(2026,7,8))!=
           ExpandedNativeTimeline_TickId(1,day(2026,7,9)));
    assert(ExpandedNativeTimeline_TickId(UINT16_MAX,day(2026,7,8))==0);
    for(int failure=1;failure<=3;++failure) {
        reset(&p,failure);
        assert(run(&p,1,start,next)==
               (failure==1?EXP_NATIVE_TICK_OG_FAILED:
                failure==2?EXP_NATIVE_TICK_CIVIL_FAILED:
                           EXP_NATIVE_TICK_VERIFY_FAILED));
        assert(p.og==1);
        assert(p.civil==(unsigned)(failure>=2));
        assert(p.verify==(unsigned)(failure>=3));
        assert(p.order_count==(unsigned)failure);
    }
    reset(&p,0);
    assert(run(&p,0,(ExpandedWorldDate){1996,6,30},
               (ExpandedWorldDate){1996,7,1})==EXP_NATIVE_TICK_OK);
    reset(&p,0);
    assert(run(&p,1,(ExpandedWorldDate){2027,6,30},
               (ExpandedWorldDate){2027,7,1})==EXP_NATIVE_TICK_OK);
    reset(&p,0);
    assert(run(&p,1,(ExpandedWorldDate){2027,11,11},
               (ExpandedWorldDate){2027,11,12})==EXP_NATIVE_TICK_OK);
    puts("NATIVE_TIMELINE_WORLD_TICK=PASS_ISOLATED_ONLY (native/civil/order/faults/July/IDs)");
    puts("NATIVE_TIMELINE_COPA_MATCHDAY=NOT_TESTED");
    puts("NATIVE_TIMELINE_CAR_CROSS_PROCESS=NOT_TESTED");
    return 0;
}
