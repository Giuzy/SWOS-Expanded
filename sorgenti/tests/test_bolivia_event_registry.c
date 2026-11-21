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
/* Native OG advances only for currentPhase > viewRandomPhase.
 * Verify that every civil due round is INCLUDED by the exclusive cutoff,
 * and that the following round is not played prematurely. Static/ISO
 * contract only: no simulation, manager menu, Windows or CAR evidence. */
static void verify_exclusive_native_cutoffs(void)
{
    uint8_t cutoff=0xffu;
    for(unsigned r=0u;r<EXP_BOLIVIA_LEAGUE_ROUNDS;++r) {
        ExpandedWorldDate due,before;
        assert(ExpandedBolivia_LeagueRoundDate(2026u,r,&due));
        assert(ExpandedBolivia_LeagueExecutionCutoff(2026u,due,&cutoff));
        assert(cutoff==(uint8_t)(kBoliviaLeague2026NativePhases[r]+1u));
        assert(cutoff>kBoliviaLeague2026NativePhases[r]);
        if(r+1u<EXP_BOLIVIA_LEAGUE_ROUNDS)
            assert(cutoff<=kBoliviaLeague2026NativePhases[r+1u]);
        assert(ExpandedWorld_IndexToDate(idx(due.year,due.month,due.day)-1,
                                         &before));
        assert(ExpandedBolivia_LeagueExecutionCutoff(2026u,before,&cutoff));
        assert(cutoff==(uint8_t)(r==0u?0u:
                         kBoliviaLeague2026NativePhases[r-1u]+1u));
    }
    assert(ExpandedBolivia_LeagueExecutionCutoff(2026u,
             (ExpandedWorldDate){2026u,7u,1u},&cutoff));
    assert(cutoff==37u); /* 8 pre-July native matchdays, last phase 36 */
    assert(ExpandedBolivia_LeagueExecutionCutoff(2026u,
             (ExpandedWorldDate){2026u,3u,31u},&cutoff));
    assert(cutoff==0u); /* START phase0 cannot simulate fixtures */
    puts("BOLIVIA_OG_EXCLUSIVE_CUTOFF=PASS_ISOLATED 30_rounds july=37");
}

int main(void)
{
    verify_exclusive_native_cutoffs();
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
