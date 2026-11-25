#include "expanded_ce_admin.h"
#include "mjson.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct AdminState {
    uint32_t *clubs;          /* sorted stable_club keys */
    uint32_t count;
    uint16_t world;
    uint8_t rv_win,rv_lose;   /* TEST vs real: real club goals / TEST goals */
    uint8_t tt_win,tt_lose;   /* TEST vs TEST */
    uint8_t loaded;
} AdminState;
static AdminState s_admin;

void ExpCeAdmin_Reset(void)
{
    free(s_admin.clubs);
    memset(&s_admin,0,sizeof s_admin);
}
static int cmp_u32(const void *a,const void *b)
{
    const uint32_t x=*(const uint32_t *)a,y=*(const uint32_t *)b;
    return x<y?-1:x>y?1:0;
}
static int goals(const char *j,int l,const char *path,uint8_t *out)
{
    double v=0;
    if(mjson_get_number(j,l,path,&v)<=0 || v<0 || v>9 || v!=(double)(int)v)
        return 0;
    *out=(uint8_t)v;
    return 1;
}
int ExpCeAdmin_LoadJson(const char *json,int len,uint16_t world)
{
    const char *arr=NULL;
    int alen=0,off=0,ko,kl,vo,vl,vt;
    double schema=0;
    AdminState n;
    char sel[32];
    uint32_t cap=0u,i;
    memset(&n,0,sizeof n);
    n.world=world;
    if(!json || len<=0 || mjson_get_number(json,len,"$.schema",&schema)<=0 ||
       schema!=1 || mjson_find(json,len,"$.clubs",&arr,&alen)!=MJSON_TOK_ARRAY ||
       !goals(json,len,"$.policy.test_vs_real.winner_goals",&n.rv_win) ||
       !goals(json,len,"$.policy.test_vs_real.loser_goals",&n.rv_lose) ||
       !goals(json,len,"$.policy.test_vs_test.winner_goals",&n.tt_win) ||
       !goals(json,len,"$.policy.test_vs_test.loser_goals",&n.tt_lose) ||
       n.rv_win<=n.rv_lose || n.tt_win<=n.tt_lose ||
       mjson_get_string(json,len,"$.policy.test_vs_test.selection",sel,
                        (int)sizeof sel)<=0 || strcmp(sel,"PAIR_HASH")!=0)
        return -1;
    while((off=mjson_next(arr,alen,off,&ko,&kl,&vo,&vl,&vt))>0) {
        unsigned c=0,o=0;
        char t[24];
        if(vt!=MJSON_TOK_STRING || vl<5 || vl>=(int)sizeof t+2)goto bad;
        memcpy(t,arr+vo+1,(size_t)vl-2u);t[vl-2]=0;
        if(sscanf(t,"%u:%u",&c,&o)!=2 || c>255u || !o || o>65535u)goto bad;
        if(n.count==cap) {
            uint32_t nc=cap?cap*2u:16u;
            uint32_t *g=(uint32_t *)realloc(n.clubs,(size_t)nc*sizeof *g);
            if(!g)goto bad;
            n.clubs=g;cap=nc;
        }
        n.clubs[n.count++]=((uint32_t)c<<16)|o;
    }
    if(off<0)goto bad;
    qsort(n.clubs,n.count,sizeof *n.clubs,cmp_u32);
    for(i=1u;i<n.count;++i)if(n.clubs[i]==n.clubs[i-1u])goto bad; /* duplicate */
    n.loaded=1u;
    ExpCeAdmin_Reset();
    s_admin=n;
    return (int)n.count;
bad:
    free(n.clubs);
    return -1;
}
uint32_t ExpCeAdmin_Count(void){return s_admin.loaded?s_admin.count:0u;}
int ExpCeAdmin_IsTest(ExpandedClubKey club)
{
    uint32_t lo=0u,hi=s_admin.count;
    if(!s_admin.loaded || club.world!=s_admin.world)return 0;
    while(lo<hi) {
        const uint32_t mid=lo+(hi-lo)/2u;
        if(s_admin.clubs[mid]<club.stable_club)lo=mid+1u;
        else hi=mid;
    }
    return lo<s_admin.count && s_admin.clubs[lo]==club.stable_club;
}
static uint64_t fnv64(const uint32_t *w,unsigned n)
{
    uint64_t h=UINT64_C(14695981039346656037);
    unsigned i,b;
    for(i=0u;i<n;++i)
        for(b=0u;b<4u;++b) {
            h^=(uint8_t)(w[i]>>(8u*b));
            h*=UINT64_C(1099511628211);
        }
    return h;
}
int ExpCeAdmin_Decide(const ExpandedUniversalFixture *f,ExpandedFixtureScore *score,
                      const char **reason)
{
    int th,ta;
    if(!f || !score || !s_admin.loaded)return 0;
    th=ExpCeAdmin_IsTest(f->home);
    ta=ExpCeAdmin_IsTest(f->away);
    if(!th && !ta)return 0;
    memset(score,0,sizeof *score);
    score->flags=EXP_CE_SCORE_ADMIN_TEST;
    if(th!=ta) {
        /* TEST vs real: the real club wins, every time */
        if(th){score->home=s_admin.rv_lose;score->away=s_admin.rv_win;}
        else  {score->home=s_admin.rv_win; score->away=s_admin.rv_lose;}
        if(reason)*reason="TEST_VS_REAL_REAL_WINS";
        return 1;
    }
    {
        /* TEST vs TEST: the winner depends on the PAIR and the edition, not
         * on who is listed first, so both legs agree. */
        const uint32_t a=f->home.stable_club<f->away.stable_club?
                         f->home.stable_club:f->away.stable_club;
        const uint32_t b=f->home.stable_club<f->away.stable_club?
                         f->away.stable_club:f->home.stable_club;
        const uint32_t w[5]={f->key.edition.competition.stable_competition,
            (uint32_t)f->key.edition.start_day,f->key.edition.sequence,a,b};
        const uint32_t winner=(fnv64(w,5u)>>32)&1u?b:a;
        const int home_wins=f->home.stable_club==winner;
        score->home=home_wins?s_admin.tt_win:s_admin.tt_lose;
        score->away=home_wins?s_admin.tt_lose:s_admin.tt_win;
        if(reason)*reason="TEST_VS_TEST_PAIR_WINNER";
    }
    return 1;
}
