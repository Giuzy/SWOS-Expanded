/* Competition Engine civil-day planner (v00.13.280). See header.
 * Data in, round days out. No OG native phase, clock, result or CAR.
 */
#ifdef SWOS_SDL_BUILD
#include "expanded_universal_date_planner.h"
#include "expanded_world_time.h"
#include <stdlib.h>
#include <string.h>

/* A season window longer than this is a data error, not a long season. */
#define EXP_DATE_PLAN_MAX_WINDOW 800

static int plan_in_break(const ExpandedDatePlanPolicy *p, int32_t day)
{
    uint32_t i;
    for(i=0;i<p->break_count;++i)
        if(day>=p->breaks[i].first_day && day<=p->breaks[i].last_day)
            return 1;
    return 0;
}
static int plan_day_mask(int32_t day)
{
    const int weekday=ExpandedWorld_Weekday(day);
    return weekday<0 || weekday>6 ? 0 : 1<<weekday;
}
/* Distance from day to the nearest element of a sorted array. */
static int32_t plan_nearest(const int32_t *sorted, uint32_t n, int32_t day)
{
    uint32_t lo=0u,hi=n;
    int32_t best=INT32_MAX;
    while(lo<hi) {
        const uint32_t mid=lo+(hi-lo)/2u;
        if(sorted[mid]<day)lo=mid+1u; else hi=mid;
    }
    if(lo<n && sorted[lo]-day<best)best=sorted[lo]-day;
    if(lo>0u && day-sorted[lo-1u]<best)best=day-sorted[lo-1u];
    return best;
}
/* Strictly increasing even selection of k items out of n (k<=n). */
static void plan_pick_even(const int32_t *from, uint32_t n,
                           uint32_t k, int32_t *out)
{
    uint32_t i;
    for(i=0;i<k;++i) {
        const uint64_t idx=k==1u?0u:
            (uint64_t)i*(uint64_t)(n-1u)/(uint64_t)(k-1u);
        out[i]=from[idx];
    }
}
static int plan_cmp(const void *aa, const void *bb)
{
    const int32_t a=*(const int32_t *)aa,b=*(const int32_t *)bb;
    return a<b?-1:a>b?1:0;
}
int ExpandedDatePlanner_Plan(const ExpandedDatePlanPolicy *policy,
                             uint32_t rounds, int32_t *out_days,
                             int *out_reason)
{
    int32_t *preferred=NULL,*extra=NULL;
    uint32_t np=0u,nx=0u,i,span,need;
    int32_t d;
    int reason=EXP_DATE_PLAN_BAD_INPUT;
    const int32_t gap=policy && policy->min_rest_days>1u?
        (int32_t)policy->min_rest_days:1;
    if(!policy || !out_days || !rounds ||
       policy->first_day<0 || policy->last_day<policy->first_day ||
       policy->last_day-policy->first_day>=EXP_DATE_PLAN_MAX_WINDOW ||
       !(policy->match_weekdays&0x7fu) ||
       policy->break_count>EXP_DATE_PLAN_MAX_BREAKS)goto done;
    span=(uint32_t)(policy->last_day-policy->first_day)+1u;
    preferred=(int32_t *)malloc((size_t)span*sizeof *preferred);
    extra=(int32_t *)malloc((size_t)span*sizeof *extra);
    if(!preferred || !extra)goto done;
    for(d=policy->first_day;d<=policy->last_day;++d) {
        const int mask=plan_day_mask(d);
        if(!mask)goto done;
        if(plan_in_break(policy,d))continue;
        if(policy->match_weekdays&mask)preferred[np++]=d;
    }
    reason=EXP_DATE_PLAN_NOT_ENOUGH_DAYS;
    if(np>=rounds) {
        plan_pick_even(preferred,np,rounds,out_days);
    } else {
        /* Midweek days only fill the shortfall, each far enough from every
         * preferred round; preferred days are all used. */
        need=rounds-np;
        for(d=policy->first_day;d<=policy->last_day;++d) {
            const int mask=plan_day_mask(d);
            if(!(policy->extra_weekdays&mask) ||
               (policy->match_weekdays&mask) || plan_in_break(policy,d))
                continue;
            if(np && plan_nearest(preferred,np,d)<gap)continue;
            extra[nx++]=d;
        }
        if(nx<need)goto done;
        memcpy(out_days,preferred,(size_t)np*sizeof *out_days);
        plan_pick_even(extra,nx,need,out_days+np);
        qsort(out_days,(size_t)rounds,sizeof *out_days,plan_cmp);
    }
    reason=EXP_DATE_PLAN_REST_VIOLATION;
    for(i=1;i<rounds;++i)
        if(out_days[i]-out_days[i-1u]<gap)goto done;
    reason=EXP_DATE_PLAN_OK;
done:
    free(preferred);
    free(extra);
    if(out_reason)*out_reason=reason;
    return reason==EXP_DATE_PLAN_OK;
}
#endif
