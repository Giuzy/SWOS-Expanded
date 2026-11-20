/* RG-014 isolated ST only: canonical resident-fixture projection and date
 * ordering. This does not test live manager gameplay, CAR or Windows. */
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include "bolivia_cup_sequence.h"
#include "bolivia_cup_knockout.h"
#include "bolivia_manager_calendar_gate.h"

int main(void)
{
    BoliviaCupFixture fx[BOLIVIA_CUP_GROUP_FIXTURES];
    assert(BoliviaCup_ChronologicalFixtures(fx)==64u);
    assert(BoliviaKO_Validate());
    for(unsigned team=0u;team<16u;++team) {
        unsigned games=0u;
        uint16_t firstId=0u;
        for(unsigned i=0u;i<64u;++i) {
            const BoliviaCupFixture *f=&fx[i];
            if(f->home!=team && f->away!=team)continue;
            if(!games)firstId=f->fixtureId;
            ++games;
            for(unsigned j=0u;j<i;++j)
                assert(fx[j].month!=f->month || fx[j].day!=f->day ||
                       (fx[j].home!=team && fx[j].away!=team));
        }
        assert(games==8u);
        assert(firstId>0u);
    }
    /* Nacional Potosi ordinal 13: real first group fixture, Aug 18,
     * Guabira (7) vs Nacional (13). No invented first-round result. */
    unsigned seen=0u;
    for(unsigned i=0u;i<64u;++i) {
        if(fx[i].home!=13u && fx[i].away!=13u)continue;
        assert(fx[i].fixtureId==2u);
        assert(fx[i].month==8u && fx[i].day==18u);
        assert(fx[i].home==7u && fx[i].away==13u);
        seen=1u;break;
    }
    assert(seen);
    /* Positive: cup is strictly before manager's next native league key. */
    assert(BoliviaManagerCalendar_Gate(100,105,1,110)==1);
    /* Negative: native league first; Copa does NOT steal PLAY MATCH. */
    assert(BoliviaManagerCalendar_Gate(100,110,1,105)==0);
    /* Negative: same civil date is unverified double booking: fail closed. */
    assert(BoliviaManagerCalendar_Gate(100,105,1,105)==-1);
    /* Negative: historical overdue fixture cannot silently replay. */
    assert(BoliviaManagerCalendar_Gate(106,105,1,110)==-2);
    /* No local match left: manager's actual cup event still exists. */
    assert(BoliviaManagerCalendar_Gate(100,105,0,0)==1);
    puts("RG-014 ISOLATED ST PASS: 16x8 group schedule, next real pair, "
         "KO template, calendar ordering and negative gates; GP/Windows NP");
    return 0;
}
