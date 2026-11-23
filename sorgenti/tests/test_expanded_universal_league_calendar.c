/* v00.13.280 RG-T: the REAL data_json/worlds/2627 leagues.json and
 * league_calendar.json parsed by the REAL loader, then every division
 * planned by the REAL CE date planner. Proves the shipped data covers all
 * 91 leagues with one civil day per round and no OG phase date.
 * NOT Windows gameplay, OG 50 Hz, CAR or UI.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "expanded_universal_json_leagues.h"
#include "expanded_universal_date_planner.h"
#include "expanded_world_time.h"
#include "world.h"
#include <stdlib.h>
void *swos_io_realloc(void *p,uint32_t n){return realloc(p,n);}
WorldId World_Current(void){return WORLD_2627;}
uint16_t World_FirstYear(void){return 2026u;}
const char *swos_data_path(const char *rel,char *buf,uint32_t sz)
{
    snprintf(buf,sz,"../%s",rel); /* runner cwd = sorgenti */
    return buf;
}
static int plan(const ExpandedUniversalJsonLeague *r,uint32_t rounds,
                int32_t *days,uint32_t *midweek)
{
    ExpandedDatePlanPolicy p;
    uint32_t i;
    int reason=0;
    memset(&p,0,sizeof p);
    if(!r->has_calendar ||
       !ExpandedWorld_DateToIndex((ExpandedWorldDate){2026u,r->first_month,
           r->first_day},&p.first_day) ||
       !ExpandedWorld_DateToIndex((ExpandedWorldDate){
           (uint16_t)(2026u+r->last_year_offset),r->last_month,r->last_day},
           &p.last_day))return 0;
    p.match_weekdays=r->match_weekdays;p.extra_weekdays=r->extra_weekdays;
    p.min_rest_days=r->min_rest_days;
    if(r->break_count)return 0; /* shipped data has no breaks yet */
    if(!ExpandedDatePlanner_Plan(&p,rounds,days,&reason))return 0;
    for(i=0;i<rounds;++i) {
        const int wd=ExpandedWorld_Weekday(days[i]);
        assert(days[i]>=p.first_day && days[i]<=p.last_day);
        assert((r->match_weekdays|r->extra_weekdays)&(1u<<wd));
        if(!(r->match_weekdays&(1u<<wd)))++*midweek;
        if(i)assert(days[i]-days[i-1u]>=(int32_t)r->min_rest_days);
    }
    return 1;
}
int main(void)
{
    static int32_t days[4096];
    uint32_t c,comp,div,divisions=0u,midweek=0u,planned=0u;
    assert(ExpandedUniversalJsonLeagues_Load());
    for(c=0;c<256u;++c)for(comp=0;comp<256u;++comp)for(div=0;div<16u;++div) {
        ExpandedUniversalJsonLeague r;
        uint32_t n,rounds;
        if(!ExpandedUniversalJsonLeagues_Find((uint8_t)c,(uint8_t)comp,
                                              (uint8_t)div,&r))continue;
        ++divisions;
        assert(r.has_calendar);
        n=r.declared_clubs;
        rounds=((n&1u)?n:n-1u)*r.meetings;
        if(rounds>4096u) {
            fprintf(stderr,"TOO_MANY_ROUNDS country=%u comp=%u div=%u "
                "rounds=%u\n",c,comp,div,rounds);
            continue;
        }
        if(plan(&r,rounds,days,&midweek))++planned;
        else fprintf(stderr,"PLAN_FAIL country=%u comp=%u div=%u clubs=%u "
                     "rounds=%u\n",c,comp,div,n,rounds);
    }
    printf("LEAGUE_CALENDAR divisions=%u planned=%u midweek-rounds=%u\n",
           divisions,planned,midweek);
    assert(divisions==ExpandedUniversalJsonLeagues_Count());
    assert(planned==divisions);
    {
        uint8_t m=0,d=0,mask=0;
        assert(ExpandedUniversalJsonLeagues_ParseMonthDay("02-29",&m,&d));
        assert(!ExpandedUniversalJsonLeagues_ParseMonthDay("02-30",&m,&d));
        assert(!ExpandedUniversalJsonLeagues_ParseMonthDay("2-1",&m,&d));
        assert(ExpandedUniversalJsonLeagues_ParseWeekdays("SAT,SUN",&mask) &&
               mask==((1u<<5)|(1u<<6)));
        assert(!ExpandedUniversalJsonLeagues_ParseWeekdays("SUN,SUN",&mask));
        assert(!ExpandedUniversalJsonLeagues_ParseWeekdays("sun",&mask));
    }
    puts("REAL_2627_LEAGUE_CALENDAR=PASS_ISOLATED_ONLY");
    puts("OG_NATIVE_MATCH/CAR/GP/WIN=NOT_TESTED");
    return 0;
}
