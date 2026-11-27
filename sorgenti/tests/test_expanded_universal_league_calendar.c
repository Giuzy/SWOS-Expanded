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
#include "expanded_international_windows.h"
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
static uint32_t s_extended;
static uint16_t s_year=2026u;   /* edition start year planned */
static int plan(const ExpandedUniversalJsonLeague *r,uint32_t rounds,
                int32_t *days,uint32_t *midweek)
{
    ExpandedDatePlanPolicy p;
    uint32_t i;
    memset(&p,0,sizeof p);
    if(!r->has_calendar ||
       !ExpandedWorld_DateToIndex((ExpandedWorldDate){s_year,r->first_month,
           r->first_day},&p.first_day) ||
       !ExpandedWorld_DateToIndex((ExpandedWorldDate){
           (uint16_t)(s_year+r->last_year_offset),r->last_month,r->last_day},
           &p.last_day))return 0;
    p.match_weekdays=r->match_weekdays;p.extra_weekdays=r->extra_weekdays;
    p.min_rest_days=r->min_rest_days;
    /* Same annual MM-DD mapping as the catalog, including breaks crossing
     * New Year. Do not ignore them in the real-data planner test. */
    for(i=0;i<r->break_count;++i) {
        const ExpandedUniversalJsonLeagueBreak *b=&r->breaks[i];
        ExpandedDatePlanBreak *dst=&p.breaks[p.break_count];
        uint16_t year=s_year;
        if(b->season && b->season!=s_year)continue;
        if(!ExpandedWorld_DateToIndex((ExpandedWorldDate){year,b->from_month,b->from_day},&dst->first_day))return 0;
        if(dst->first_day<p.first_day &&
           !ExpandedWorld_DateToIndex((ExpandedWorldDate){++year,b->from_month,b->from_day},&dst->first_day))return 0;
        if(!ExpandedWorld_DateToIndex((ExpandedWorldDate){year,b->to_month,b->to_day},&dst->last_day))return 0;
        if(dst->last_day<dst->first_day &&
           !ExpandedWorld_DateToIndex((ExpandedWorldDate){(uint16_t)(year+1u),b->to_month,b->to_day},&dst->last_day))return 0;
        ++p.break_count;
    }
    /* v00.13.354: same routine as the game (windows + move after a stop) */
    {
        int32_t ext=0;
        if(!ExpIntl_PlanWithWindows(&p,r->intl_breaks,rounds,days,&ext))return 0;
        if(ext)++s_extended;
    }
    for(i=0;i<rounds;++i) {
        const int wd=ExpandedWorld_Weekday(days[i]);
        uint32_t b;
        assert(days[i]>=p.first_day && days[i]<=p.last_day);
        for(b=0;b<p.break_count;++b)assert(days[i]<p.breaks[b].first_day || days[i]>p.breaks[b].last_day);
        assert((r->match_weekdays|r->extra_weekdays)&(1u<<wd));
        if(!(r->match_weekdays&(1u<<wd)))++*midweek;
        if(i)assert(days[i]-days[i-1u]>=(int32_t)r->min_rest_days);
    }
    return 1;
}

typedef struct HybridAudit {
    uint8_t count[12][12],h[12],a[12],day[26][12];
    uint32_t n;
} HybridAudit;
static int hybrid_emit(void *user,uint32_t cycle,uint32_t round,uint32_t serial,
                       uint32_t h,uint32_t a)
{
    HybridAudit *v=(HybridAudit *)user;
    uint32_t r=cycle*11u+round;
    (void)serial;
    assert(h<12u&&a<12u&&h!=a&&r<26u);
    assert(!v->day[r][h]&&!v->day[r][a]);
    v->day[r][h]=v->day[r][a]=1u;
    ++v->count[h][a];++v->h[h];++v->a[a];++v->n;return 1;
}
static void hybrid_test(const ExpandedUniversalJsonLeague *r)
{
    uint32_t keys[12],i,j,k,y;
    for(i=0u;i<12u;++i)keys[i]=(44u<<16)|(i+1u);
    for(y=2026u;y<2126u;++y) {
        HybridAudit v={0};
        assert(ExpandedUniversal_GenerateLeague(12u,2u,keys,&r->extras,
            ExpPair_Seed(760742145u,(int32_t)y),hybrid_emit,&v));
        assert(v.n==156u);
        for(i=0u;i<12u;++i) {
            uint32_t ex=0u;
            assert(v.h[i]==13u&&v.a[i]==13u);
            for(j=0u;j<12u;++j)if(i!=j) {
                const unsigned pair=v.count[i][j]+v.count[j][i];
                assert(v.count[i][j]>=1u&&(pair==2u||pair==3u));
                if(pair==3u)++ex;
            }
            assert(ex==4u);
        }
        for(k=0u;k<r->extras.nrivalries;++k) {
            i=(r->extras.rivalries[k][0]&65535u)-1u;
            j=(r->extras.rivalries[k][1]&65535u)-1u;
            assert(v.count[i][j]+v.count[j][i]==3u);
        }
    }
    {
        HybridAudit v={0};ExpPairExtras x=r->extras;
        x.rivalries[0][0]=0u;
        assert(!ExpandedUniversal_GenerateLeague(12u,2u,keys,&x,1u,hybrid_emit,&v)&&!v.n);
    }
    puts("HYBRID_ROUND_ROBIN_100_SEEDS=PASS_ISOLATED");
}

int main(void)
{
    static int32_t days[4096];
    uint32_t c,comp,div,divisions=0u,midweek=0u,planned=0u;
    assert(ExpandedUniversalJsonLeagues_Load());
    {   ExpandedUniversalJsonLeague a,n,v,q,it,b;
        assert(ExpandedUniversalJsonLeagues_Find(44,88,0,&a) && a.first_month==10 && a.first_day==16 && a.last_month==5 && a.last_year_offset==1 && a.break_count==2 && a.extras.rounds==4u && a.extras.nrivalries==6u);
        hybrid_test(&a);
        assert(ExpandedUniversalJsonLeagues_Find(44,88,1,&n) && n.first_month==2 && n.first_day==6 && n.last_month==8 && n.last_day==30);
        assert(ExpandedUniversalJsonLeagues_Find(44,88,2,&v) && v.first_month==2 && v.first_day==13 && v.last_day==29);
        assert(ExpandedUniversalJsonLeagues_Find(44,88,3,&q) && q.first_month==2 && q.first_day==20 && q.last_day==14);
        assert(ExpandedUniversalJsonLeagues_Find(20,61,0,&it) && it.first_month==8 && it.first_day==22 && it.last_month==5 && it.last_day==30 && it.break_count==5 && it.breaks[0].season==2026u && it.breaks[3].season==0u &&
               it.intl_breaks==(EXP_INTL_FIFA|EXP_INTL_UEFA) && it.cal.return_order==1u && it.cal.min_return_gap==8u &&
               it.cal.nprofile==4u && !it.cal_invalid[0]);  /* v00.13.354: FIFA windows from the global table */
        assert(ExpandedUniversalJsonLeagues_Find(20,61,1,&b) && b.first_month==9 && b.first_day==1 && b.break_count==0);
    }
    for(c=0;c<256u;++c)for(comp=0;comp<256u;++comp)for(div=0;div<16u;++div) {
        ExpandedUniversalJsonLeague r;
        uint32_t n,rounds;
        if(!ExpandedUniversalJsonLeagues_Find((uint8_t)c,(uint8_t)comp,
                                              (uint8_t)div,&r))continue;
        ++divisions;
        assert(r.has_calendar);
        n=r.declared_clubs;
        rounds=((n&1u)?n:n-1u)*r.meetings+r.extras.rounds;
        if(rounds>4096u) {
            fprintf(stderr,"TOO_MANY_ROUNDS country=%u comp=%u div=%u "
                "rounds=%u\n",c,comp,div,rounds);
            continue;
        }
        if(plan(&r,rounds,days,&midweek))++planned;
        else fprintf(stderr,"PLAN_FAIL country=%u comp=%u div=%u clubs=%u "
                     "rounds=%u\n",c,comp,div,n,rounds);
    }
    printf("LEAGUE_CALENDAR divisions=%u planned=%u midweek-rounds=%u extended-after-intl=%u intl-windows=%u\n",
           divisions,planned,midweek,(unsigned)s_extended,(unsigned)ExpIntl_Count());
    assert(ExpIntl_Count()>0u);
    {   /* official dates only for 2026/27; from 1/7/2027 the calendar
         * generates the windows with the two-year World Cup / EURO cycle */
        ExpIntlWindow w[16];
        int32_t a1,b1;
        uint32_t k,i,mondays;
#define YEAR(y) (assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){(y),1u,1u},&a1)),                  assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){(y),12u,31u},&b1)))
        assert(ExpIntl_GeneratedFrom()>0);
        YEAR(2031u);                      /* odd year: 4 FIFA windows + Gold Cup */
        k=ExpIntl_Overlapping(EXP_INTL_FIFA,a1,b1,w,16u);
        assert(k==4u);
        for(i=0;i<k;++i)assert(ExpandedWorld_Weekday(w[i].first_day)==0);
        assert(w[2].last_day-w[2].first_day==15);   /* autumn double window */
        assert(ExpIntl_Overlapping(EXP_INTL_CONCACAF,a1,b1,w,16u)==1u);
        assert(ExpIntl_Overlapping(EXP_INTL_AFC,a1,b1,w,16u)==1u);    /* Asian Cup 2031 */
        assert(ExpIntl_Overlapping(EXP_INTL_UEFA,a1,b1,w,16u)==0u);
        YEAR(2030u);                      /* World Cup year: + the tournament */
        k=ExpIntl_Overlapping(EXP_INTL_FIFA,a1,b1,w,16u);
        assert(k==5u);
        for(i=0,mondays=0;i<k;++i)mondays+=ExpandedWorld_Weekday(w[i].first_day)==0;
        assert(mondays>=4u);
        assert(ExpIntl_Overlapping(EXP_INTL_UEFA,a1,b1,w,16u)==0u);
        YEAR(2032u);                      /* EURO year (+ Copa America) */
        assert(ExpIntl_Overlapping(EXP_INTL_FIFA,a1,b1,w,16u)==4u);
        assert(ExpIntl_Overlapping(EXP_INTL_UEFA,a1,b1,w,16u)==1u);
        assert(ExpIntl_Overlapping(EXP_INTL_CONMEBOL,a1,b1,w,16u)==1u);
        YEAR(2034u);                      /* World Cup again, no EURO */
        assert(ExpIntl_Overlapping(EXP_INTL_UEFA,a1,b1,w,16u)==0u);
        assert(ExpIntl_Overlapping(EXP_INTL_FIFA,a1,b1,w,16u)==5u);
        YEAR(2027u);                      /* official 2026/27 part + generated autumn */
        k=ExpIntl_Overlapping(EXP_INTL_FIFA,a1,b1,w,16u);
        assert(k==4u);                    /* 22/03, 07/06 official; ~20/09, ~08/11 generated */
        assert(ExpIntl_Overlapping(EXP_INTL_CONCACAF,a1,b1,w,16u)==1u); /* official only, no duplicate */
#undef YEAR
        puts("INTL_WINDOWS_GENERATED_WORLD_CUP_EURO_CYCLE=PASS");
    }
    {   /* every division also fits its generated windows 2027..2036
         * (World Cup and EURO years included), moving after a stop if needed */
        uint32_t fails=0u,ext0=s_extended;
        for(s_year=2027u;s_year<=2036u;++s_year)
            for(c=0;c<256u;++c)for(comp=0;comp<256u;++comp)for(div=0;div<16u;++div) {
                ExpandedUniversalJsonLeague r;
                uint32_t n,rounds,mw=0u;
                if(!ExpandedUniversalJsonLeagues_Find((uint8_t)c,(uint8_t)comp,(uint8_t)div,&r))continue;
                n=r.declared_clubs;rounds=((n&1u)?n:n-1u)*r.meetings+r.extras.rounds;
                if(rounds>4096u)continue;
                if(!plan(&r,rounds,days,&mw)){++fails;
                    fprintf(stderr,"PLAN_FAIL_YEAR year=%u country=%u div=%u\n",(unsigned)s_year,c,div);}
            }
        s_year=2026u;
        printf("MULTI_YEAR_2027_2036 plan-failures=%u extended=%u\n",fails,(unsigned)(s_extended-ext0));
        assert(fails==0u);
    }
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
