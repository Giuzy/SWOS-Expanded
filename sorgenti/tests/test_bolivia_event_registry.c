/* Bolivia civil pilot: isolated source-data/scheduler test only.
 * This deliberately does NOT execute OpenSWOS, a Windows build or CAR. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "fixture_data.h"
#include "expanded_bolivia_event_registry.h"

static int32_t idx(uint16_t y,uint8_t m,uint8_t d)
{
    int32_t day=0;
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){y,m,d},&day));
    return day;
}
int main(void)
{
    uint16_t league[16],map[256];
    ExpandedBoliviaEvent program[EXP_BOLIVIA_PILOT_EVENTS];
    ExpandedScheduledEvent storage[EXP_BOLIVIA_PILOT_EVENTS]={0};
    ExpandedEventScheduler schedule={0};
    const ExpandedScheduledEvent *first=0;
    for(unsigned i=0;i<256u;++i)map[i]=0xffffu;
    for(unsigned i=0;i<16u;++i) {
        league[i]=(uint16_t)((45u<<8)|i);
        map[i]=league[i];
    }
    assert(ExpandedBolivia_BuildProgram(1u,2026u,league,
                 _fixtureSchedule16Teams,map,program));
    unsigned countLeague=0u,countGroup=0u,countKO=0u;
    unsigned perClub[16]={0};
    for(unsigned i=0;i<EXP_BOLIVIA_PILOT_EVENTS;++i) {
        for(unsigned j=0;j<i;++j) {
            assert(program[i].id!=program[j].id);
            if(program[i].kind==EXP_BOLIVIA_EVENT_COPA_KO_DATE ||
               program[j].kind==EXP_BOLIVIA_EVENT_COPA_KO_DATE ||
               program[i].day_index!=program[j].day_index)continue;
            const ExpandedBoliviaEvent *a=&program[i],*b=&program[j];
            assert(a->home!=b->home && a->home!=b->away &&
                   a->away!=b->home && a->away!=b->away);
        }
        if(program[i].kind==EXP_BOLIVIA_EVENT_LEAGUE) {
            ++countLeague;
            ++perClub[program[i].home&255u];
            ++perClub[program[i].away&255u];
        } else if(program[i].kind==EXP_BOLIVIA_EVENT_COPA_GROUP) {
            ++countGroup;
            assert(program[i].home!=0xffffu && program[i].away!=0xffffu);
        } else {
            assert(program[i].kind==EXP_BOLIVIA_EVENT_COPA_KO_DATE);
            assert(program[i].home==0xffffu && program[i].away==0xffffu);
            ++countKO;
        }
    }
    assert(countLeague==240u && countGroup==64u && countKO==14u);
    for(unsigned i=0;i<16u;++i)assert(perClub[i]==30u);
    assert(ExpandedBolivia_BuildScheduler(
        &schedule,storage,program,2026u));
    assert(schedule.count==EXP_BOLIVIA_PILOT_EVENTS);
    assert(ExpandedEventScheduler_Next(&schedule,idx(2026,4,1),&first)
           ==EXP_EVENT_DUE);
    assert(first && first->day_index==idx(2026,4,1));
    const ExpandedWorldDate today={2026u,7u,1u};
    const ExpandedWorldDate copa={2026u,8u,18u};
    assert(ExpandedBolivia_ChooseNext(1u,2026u,today,
           8u,1,copa,2u,1)==0); /* Liga 16 Jul precedes Copa */
    assert(ExpandedBolivia_ChooseNext(1u,2026u,today,
           15u,1,copa,2u,1)==1); /* Copa precedes Liga 24 Aug */
    assert(ExpandedBolivia_ChooseNext(1u,2026u,today,
           15u,1,(ExpandedWorldDate){2026u,8u,24u},2u,1)==-1);
    assert(ExpandedBolivia_ChooseNext(1u,2026u,
           (ExpandedWorldDate){2026u,8u,19u},
           15u,1,copa,2u,1)==-2); /* overdue cannot replay */
    assert(ExpandedBolivia_ChooseNext(1u,2026u,today,
           30u,0,copa,65u,1)==1); /* provisional KO date */
    league[0]=league[1];
    assert(!ExpandedBolivia_BuildProgram(1u,2026u,league,
                _fixtureSchedule16Teams,map,program));
    puts("BOLIVIA_EVENT_REGISTRY=PASS_ISOLATED_C_DATA_ONLY");
    puts("OG_GAMEPLAY_WINDOWS_CAR=NOT_TESTED");
    return 0;
}
