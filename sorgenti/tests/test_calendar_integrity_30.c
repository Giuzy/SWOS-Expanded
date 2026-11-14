/* Exact 30-year calendar transition regression, using the production
 * Living World ledger, native fixture generator and .CAR serializer.
 * This does NOT exercise SDL, OG cup callbacks or Windows saves.
 * Never label its isolated PASS a complete Windows career PASS. */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "expanded_world_live.h"
#include "expanded_world_time.h"
#include "fixture_rounds.h"

typedef struct Buffer {
    char *s;
    size_t len, cap;
} Buffer;
static int append(const char *data,int len,void *arg) {
    Buffer *b=(Buffer*)arg;
    size_t needed;
    if(!b || !data || len<0) return 0;
    needed=b->len+(size_t)len+1u;
    if(needed>b->cap) {
        size_t size=b->cap?b->cap:4096u;
        while(size<needed) {
            if(size>SIZE_MAX/2u) return 0;
            size*=2u;
        }
        char *p=(char*)realloc(b->s,size);
        if(!p) return 0;
        b->s=p;b->cap=size;
    }
    memcpy(b->s+b->len,data,(size_t)len);
    b->len+=(size_t)len;b->s[b->len]=0;
    return len;
}
static int capture(Buffer *b) {
    static const char open[]="{\"living_world\":";
    if(append(open,(int)(sizeof(open)-1u),b)!=(int)(sizeof(open)-1u))
        return 0;
    if(!ExpandedWorldLive_WriteJson(append,b)) return 0;
    return append("}",1,b)==1;
}
static int same_date(ExpandedWorldDate a,ExpandedWorldDate b) {
    return a.year==b.year && a.month==b.month && a.day==b.day;
}
static int check_state(ExpandedWorldDate wanted,const char *event) {
    ExpandedLiveCalendarState state={0};
    int32_t now=0,next=0,anchor=0;
    const ExpandedWorldDate july96={1996u,7u,1u};
    if(!ExpandedWorldLive_CalendarState(&state) ||
       !same_date(state.today,wanted) ||
       !ExpandedWorld_DateToIndex(state.today,&now) ||
       !ExpandedWorld_DateToIndex(july96,&anchor) ||
       now<anchor || ExpandedWorldLive_Weeks()!=(uint32_t)(now-anchor)/7u ||
       state.overdue!=0u ||
       (uint64_t)state.pending+(uint64_t)ExpandedWorldLive_ResidentResults()!=
           (uint64_t)ExpandedWorldLive_Fixtures() ||
       ExpandedWorldLive_ArchivedLeagues()!=
           (uint32_t)(ExpandedWorldLive_WindowFirstYear()-1996u)*2u ||
       ExpandedWorldLive_ArchivedResults()!=
           (uint32_t)(ExpandedWorldLive_WindowFirstYear()-1996u)*512u ||
       ExpandedWorldLive_Results()!=
           ExpandedWorldLive_ArchivedResults()+
           ExpandedWorldLive_ResidentResults() ||
       (state.has_next &&
        (!ExpandedWorld_DateToIndex(state.next_date,&next) ||
         next<now || state.pending==0u || state.next_matches==0u)) ||
       (!state.has_next && state.pending!=0u)) {
        fprintf(stderr,"FIRST_CALENDAR_FAILURE event=%s target=%04u-%02u-%02u "
                "actual=%04u-%02u-%02u window=%u archives=%lu "
                "archived_matches=%lu results=%lu pending=%lu overdue=%lu "
                "week=%lu\n",
                event,(unsigned)wanted.year,(unsigned)wanted.month,
                (unsigned)wanted.day,(unsigned)state.today.year,
                (unsigned)state.today.month,(unsigned)state.today.day,
                (unsigned)ExpandedWorldLive_WindowFirstYear(),
                (unsigned long)ExpandedWorldLive_ArchivedLeagues(),
                (unsigned long)ExpandedWorldLive_ArchivedResults(),
                (unsigned long)ExpandedWorldLive_Results(),
                (unsigned long)state.pending,(unsigned long)state.overdue,
                (unsigned long)ExpandedWorldLive_Weeks());
        return 0;
    }
    return 1;
}
static int save_restore_exact(ExpandedWorldDate wanted) {
    Buffer before={0},after={0};
    int result=0;
    const uint32_t matches=ExpandedWorldLive_Results();
    const uint32_t fixtures=ExpandedWorldLive_Fixtures();
    const uint32_t archives=ExpandedWorldLive_ArchivedResults();
    const uint16_t window=ExpandedWorldLive_WindowFirstYear();
    if(!capture(&before)) goto done;
    ExpandedWorldLive_End();
    if(!ExpandedWorldLive_ReadJson(before.s,(int)before.len) ||
       !check_state(wanted,"AFTER_CAR_RELOAD") ||
       matches!=ExpandedWorldLive_Results() ||
       fixtures!=ExpandedWorldLive_Fixtures() ||
       archives!=ExpandedWorldLive_ArchivedResults() ||
       window!=ExpandedWorldLive_WindowFirstYear() ||
       !capture(&after) || before.len!=after.len ||
       memcmp(before.s,after.s,before.len)!=0)goto done;
    result=1;
done:
    if(!result) fprintf(stderr,
        "FIRST_CALENDAR_FAILURE event=SAVE_RESTORE target=%04u-%02u-%02u "
        "pre_bytes=%lu post_bytes=%lu\n",
        (unsigned)wanted.year,(unsigned)wanted.month,(unsigned)wanted.day,
        (unsigned long)before.len,(unsigned long)after.len);
    free(before.s);free(after.s);
    return result;
}
static int checkpoint(ExpandedWorldDate next,int *last_index,
                      unsigned *checks,unsigned *reloads,
                      int reload) {
    int32_t current=0;
    const uint32_t old_matches=ExpandedWorldLive_Results();
    if(!ExpandedWorld_DateToIndex(next,&current) ||
       current<=*last_index ||
       !ExpandedWorldLive_AdvanceToDate(next) ||
       !check_state(next,"ADVANCE") ||
       old_matches>ExpandedWorldLive_Results()) {
        fprintf(stderr,"FIRST_CALENDAR_FAILURE event=CHECKPOINT "
            "target=%04u-%02u-%02u previous_index=%d\n",
            (unsigned)next.year,(unsigned)next.month,(unsigned)next.day,
            *last_index);
        return 0;
    }
    ++*checks;
    /* Advancing twice to the same day must not replay any match. */
    const uint32_t played=ExpandedWorldLive_Results();
    const uint32_t resident=ExpandedWorldLive_ResidentResults();
    if(!ExpandedWorldLive_AdvanceToDate(next) ||
       played!=ExpandedWorldLive_Results() ||
       resident!=ExpandedWorldLive_ResidentResults()) {
        fprintf(stderr,"FIRST_CALENDAR_FAILURE event=DUPLICATE_ON_SAME_DAY "
            "target=%04u-%02u-%02u\n",
            (unsigned)next.year,(unsigned)next.month,(unsigned)next.day);
        return 0;
    }
    if(reload) {
        if(!save_restore_exact(next))return 0;
        ++*reloads;
        if(!ExpandedWorldLive_AdvanceToDate(next) ||
           played!=ExpandedWorldLive_Results()) {
            fprintf(stderr,"FIRST_CALENDAR_FAILURE event=DUPLICATE_AFTER_LOAD "
                "target=%04u-%02u-%02u\n",
                (unsigned)next.year,(unsigned)next.month,
                (unsigned)next.day);
            return 0;
        }
    }
    *last_index=current;
    return 1;
}
int main(void) {
    int32_t last=0;
    unsigned checks=0,reloads=0;
    const ExpandedWorldDate first={1996u,7u,1u};
    FixtureRounds_Install();
    if(!ExpandedWorld_DateToIndex(first,&last) ||
       !ExpandedWorldLive_Begin(1996u)) {
        fprintf(stderr,"FIRST_CALENDAR_FAILURE event=INITIALIZE_YEAR_1996\n");
        goto fail;
    }
    /* Northern August-May and southern March-October schedules; both
     * use the OG native fixture table, without any extra simulator. */
    for(unsigned group=0;group<2u;++group) {
        const uint8_t country=group?44u:8u;
        const unsigned clubs=group?12u:20u;
        for(unsigned i=0;i<clubs;++i) {
            char name[18];
            snprintf(name,sizeof(name),"CAL %02u/%02u",group,i);
            if(!ExpandedWorldLive_AddClub(country,0u,
                   (uint16_t)(((uint16_t)country<<8)|i),
                   (uint16_t)(180u+i),name,3u)) {
                fprintf(stderr,"FIRST_CALENDAR_FAILURE event=ADD_CLUB "
                               "country=%u index=%u\n",country,i);
                goto fail;
            }
        }
        if(!ExpandedWorldLive_SetLeagueSchedule(country,0u,
                   group?16u:56u,group?80u:32u,2u)) {
            fprintf(stderr,"FIRST_CALENDAR_FAILURE event=SET_NATIVE_SCHEDULE "
                           "country=%u\n",country);
            goto fail;
        }
    }
    if(!ExpandedWorldLive_Seal()) {
        fprintf(stderr,"FIRST_CALENDAR_FAILURE event=SEAL_WORLD\n");
        goto fail;
    }
    if(!check_state(first,"INITIAL_JULY_1996") ||
       !save_restore_exact(first))goto fail;
    ++reloads;
    for(unsigned year=1996u;year<2026u;++year) {
        const uint16_t y=(uint16_t)year, n=(uint16_t)(year+1u);
        /* Check every season boundary on its actual day, never week*7.
         * Jan 1 rolls the resident ledger; July 1 starts OG career
         * year. Feb 29 / Mar 1 catch leap-year boundary errors. */
        const ExpandedWorldDate steps[]={
            {y,7u,2u},{y,10u,1u},{y,12u,31u},
            {n,1u,1u},{n,2u,28u},
            {n,3u,1u},{n,6u,30u},{n,7u,1u}
        };
        for(unsigned i=0;i<sizeof(steps)/sizeof(steps[0]);++i) {
            if(i==5u && n%4u==0u &&
               (n%100u!=0u || n%400u==0u)) {
                const ExpandedWorldDate leap={n,2u,29u};
                if(!checkpoint(leap,&last,&checks,&reloads,1))goto fail;
            }
            if(!checkpoint(steps[i],&last,&checks,&reloads,
                           i==3u || i==7u))goto fail;
            if(i==7u) {
                /* The OG July boundary must never round the exact
                 * date to a financial or weekly cursor. */
                const uint32_t results=ExpandedWorldLive_Results();
                if(!ExpandedWorldLive_AdvanceToSeason(n) ||
                   !check_state(steps[i],"OG_JULY_SEASON_BOUNDARY") ||
                   results!=ExpandedWorldLive_Results()) {
                    fprintf(stderr,"FIRST_CALENDAR_FAILURE event=OG_JULY_SEASON "
                                   "year=%u\n",n);
                    goto fail;
                }
                for(unsigned k=1996u;
                    k<ExpandedWorldLive_WindowFirstYear();++k) {
                    uint16_t champion=0u;
                    if(!ExpandedWorldLive_HistoryChampion((uint16_t)k,8u,0u,
                                                         &champion) ||
                       (champion>>8)!=8u ||
                       !ExpandedWorldLive_HistoryChampion((uint16_t)k,44u,
                                                         0u,&champion) ||
                       (champion>>8)!=44u) {
                        fprintf(stderr,"FIRST_CALENDAR_FAILURE "
                            "event=MISSING_OR_WRONG_CHAMPION "
                            "archived_year=%u window=%u\n",
                            k,(unsigned)ExpandedWorldLive_WindowFirstYear());
                        goto fail;
                    }
                }
            }
        }
    }
    {
        ExpandedLiveCalendarState state;
        const ExpandedWorldDate expected={2026u,7u,1u};
        if(!ExpandedWorldLive_CalendarState(&state) ||
           !same_date(state.today,expected) ||
           checks<240u || reloads<60u) {
            fprintf(stderr,"FIRST_CALENDAR_FAILURE event=30_YEAR_COMPLETION "
                           "checks=%u reloads=%u\n",checks,reloads);
            goto fail;
        }
    }
    puts("CALENDAR_30Y_BOUNDARIES=PASS");
    puts("CALENDAR_30Y_SAVE_LOAD_EXACT=PASS");
    puts("CALENDAR_30Y_NO_DUPLICATE_FIXTURES=PASS");
    puts("CALENDAR_30Y_E2E=PASS");
    printf("CALENDAR_30Y_CHECKPOINTS=%u SNAPSHOTS=%u "
           "SCOPE=ISOLATED_C_WORLD_ONLY_WINDOWS_OG_NOT_PROVEN\n",
           checks,reloads);
    ExpandedWorldLive_End();
    return 0;
fail:
    fprintf(stderr,"CALENDAR_30Y_E2E=FAIL checks=%u snapshots=%u\n",
            checks,reloads);
    ExpandedWorldLive_End();
    return 1;
}
