/* Isolated phase-3 test, not a SWOS gameplay or CAR roundtrip claim. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "expanded_event_scheduler.h"
static int32_t day(uint16_t y,uint8_t m,uint8_t d)
{
    int32_t result=-1;
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){y,m,d},&result));
    return result;
}
int main(void)
{
    ExpandedScheduledEvent storage[8]={0};
    ExpandedEventScheduler s={0};
    const ExpandedScheduledEvent *due=0;
    const int32_t aug=day(2026,8,1),next=day(2026,8,2);
    ExpandedScheduledEvent qualifier={10u,0u,aug,5u,EXP_EVENT_QUALIFICATION,EXP_EVENT_PENDING};
    ExpandedScheduledEvent fixture={20u,10u,aug,10u,EXP_EVENT_FIXTURE,EXP_EVENT_PENDING};
    ExpandedScheduledEvent cup={30u,20u,next,1u,EXP_EVENT_CUP_ADVANCE,EXP_EVENT_PENDING};
    ExpandedScheduledEvent broken={40u,999u,next,2u,EXP_EVENT_SEASON_CLOSE,EXP_EVENT_PENDING};
    ExpandedScheduledEvent wrong;
    assert(ExpandedEventScheduler_Init(&s,storage,8u,(ExpandedWorldDate){2026,7,1})==EXP_EVENT_OK);
    assert(ExpandedEventScheduler_Schedule(&s,fixture)==EXP_EVENT_OK);
    assert(ExpandedEventScheduler_Schedule(&s,qualifier)==EXP_EVENT_OK);
    assert(ExpandedEventScheduler_Schedule(&s,cup)==EXP_EVENT_OK);
    assert(ExpandedEventScheduler_Schedule(&s,fixture)==EXP_EVENT_ALREADY);
    wrong=fixture;wrong.priority++;
    assert(ExpandedEventScheduler_Schedule(&s,wrong)==EXP_EVENT_CONFLICT);
    assert(ExpandedEventScheduler_Schedule(&s,(ExpandedScheduledEvent){50u,0u,day(2026,6,1),1u,EXP_EVENT_FIXTURE,EXP_EVENT_PENDING})==EXP_EVENT_INVALID);
    assert(ExpandedEventScheduler_ValidateRestore(&s));
    assert(ExpandedEventScheduler_Next(&s,day(2026,7,31),&due)==EXP_EVENT_EMPTY);
    assert(s.clock.day_index==day(2026,7,31));
    assert(ExpandedEventScheduler_Next(&s,day(2026,8,3),&due)==EXP_EVENT_DUE && due->id==10u);
    assert(s.clock.day_index==aug);
    assert(ExpandedEventScheduler_Complete(&s,20u)==EXP_EVENT_CONFLICT);
    assert(ExpandedEventScheduler_Complete(&s,10u)==EXP_EVENT_OK);
    assert(ExpandedEventScheduler_Complete(&s,10u)==EXP_EVENT_ALREADY);
    assert(ExpandedEventScheduler_Schedule(&s,qualifier)==EXP_EVENT_ALREADY);
    assert(ExpandedEventScheduler_Next(&s,next,&due)==EXP_EVENT_DUE && due->id==20u);
    assert(ExpandedEventScheduler_Complete(&s,20u)==EXP_EVENT_OK);
    assert(ExpandedEventScheduler_Next(&s,next,&due)==EXP_EVENT_DUE && due->id==30u);
    assert(ExpandedEventScheduler_Complete(&s,30u)==EXP_EVENT_OK);
    assert(ExpandedEventScheduler_Schedule(&s,broken)==EXP_EVENT_OK);
    assert(ExpandedEventScheduler_Next(&s,next,&due)==EXP_EVENT_BLOCKED && due==0);
    assert(ExpandedEventScheduler_ValidateRestore(&s));
    {
        ExpandedScheduledEvent saved[8];
        ExpandedEventScheduler restored=s;
        memcpy(saved,storage,sizeof saved);
        restored.events=saved;
        assert(ExpandedEventScheduler_ValidateRestore(&restored));
        assert(ExpandedEventScheduler_Next(&restored,next,&due)==EXP_EVENT_BLOCKED);
        assert(restored.clock.day_index==next);
        saved[0].id=saved[1].id;
        assert(!ExpandedEventScheduler_ValidateRestore(&restored));
    }
    {
        ExpandedScheduledEvent small[3]={0};
        ExpandedEventScheduler t={0};
        assert(ExpandedEventScheduler_Init(&t,small,3u,(ExpandedWorldDate){1996,7,1})==EXP_EVENT_OK);
        assert(ExpandedEventScheduler_Schedule(&t,(ExpandedScheduledEvent){9u,0u,day(1996,8,1),2u,EXP_EVENT_FIXTURE,0u})==EXP_EVENT_OK);
        assert(ExpandedEventScheduler_Schedule(&t,(ExpandedScheduledEvent){2u,0u,day(1996,8,1),2u,EXP_EVENT_FIXTURE,0u})==EXP_EVENT_OK);
        assert(ExpandedEventScheduler_Next(&t,day(1996,8,1),&due)==EXP_EVENT_DUE && due->id==2u);
        assert(ExpandedEventScheduler_Complete(&t,2u)==EXP_EVENT_OK);
        assert(ExpandedEventScheduler_Next(&t,day(1996,8,1),&due)==EXP_EVENT_DUE && due->id==9u);
        assert(ExpandedEventScheduler_Complete(&t,9u)==EXP_EVENT_OK);
        assert(ExpandedEventScheduler_Next(&t,day(1996,8,1),&due)==EXP_EVENT_EMPTY);
    }
    /* A still-active cross-year cup must keep its event identity and order.
     * July 1 is not a reset or an implicit competition finish. */
    {
        ExpandedScheduledEvent cross[4] = {0};
        ExpandedEventScheduler t = {0};
        const int32_t june = day(2027,6,30);
        const int32_t july = day(2027,7,1);
        const int32_t november = day(2027,11,12);
        assert(ExpandedEventScheduler_Init(&t,cross,4u,
               (ExpandedWorldDate){2027,2,1}) == EXP_EVENT_OK);
        assert(ExpandedEventScheduler_Schedule(&t,
               (ExpandedScheduledEvent){100u,0u,june,1u,
               EXP_EVENT_FIXTURE,EXP_EVENT_PENDING}) == EXP_EVENT_OK);
        assert(ExpandedEventScheduler_Schedule(&t,
               (ExpandedScheduledEvent){101u,100u,november,1u,
               EXP_EVENT_CUP_ADVANCE,EXP_EVENT_PENDING}) == EXP_EVENT_OK);
        assert(ExpandedEventScheduler_Next(&t,july,&due) ==
               EXP_EVENT_DUE && due->id == 100u);
        assert(ExpandedEventScheduler_Complete(&t,100u) == EXP_EVENT_OK);
        assert(ExpandedEventScheduler_Next(&t,july,&due) ==
               EXP_EVENT_EMPTY && t.clock.day_index == july);
        assert(ExpandedEventScheduler_ValidateRestore(&t));
        assert(ExpandedEventScheduler_Next(&t,november,&due) ==
               EXP_EVENT_DUE && due->id == 101u);
        assert(ExpandedEventScheduler_Complete(&t,101u) == EXP_EVENT_OK);
        assert(ExpandedEventScheduler_Next(&t,november,&due) == EXP_EVENT_EMPTY);
        assert(ExpandedEventScheduler_ValidateRestore(&t));
    }
    puts("POINT3_EVENT_SCHEDULER=PASS_ISOLATED_ONLY");
    return 0;
}
