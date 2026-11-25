/* v00.13.303: universal round-robin generator: pairs, reversal and the
 * home/away distribution (no long artificial runs). */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "expanded_universal_pairings.h"
#define MAXN 64
typedef struct {
    uint32_t n,meetings,count;
    uint8_t seen[MAXN][MAXN][4]; /* [home][away][cycle] */
    int8_t pat[MAXN][4*MAXN];    /* per club H=1/A=0 in round order */
    uint32_t len[MAXN];
    uint32_t last_round,last_cycle;
    uint8_t in_round[MAXN];
} Ctx;
static int emit(void *u,uint32_t cycle,uint32_t round,uint32_t serial,
                uint32_t h,uint32_t a)
{
    Ctx *c=(Ctx *)u;
    { static uint32_t exp_serial; if(round==0&&cycle==0&&serial==0)exp_serial=0; assert(serial==exp_serial++ || (cycle>0 && serial==0 && round==0 && (exp_serial=1,1))); }
    assert(h<c->n && a<c->n && h!=a);
    assert(!c->seen[h][a][cycle]);
    c->seen[h][a][cycle]=1;
    if(round!=c->last_round || cycle!=c->last_cycle) {
        memset(c->in_round,0,sizeof c->in_round);
        c->last_round=round;c->last_cycle=cycle;
    }
    assert(!c->in_round[h] && !c->in_round[a]); /* one game per round */
    c->in_round[h]=c->in_round[a]=1;
    c->pat[h][c->len[h]++]=1;c->pat[a][c->len[a]++]=0;
    c->count++;
    return 1;
}
int main(void)
{
    uint32_t n,mt,i,j,cy,worst_even=0;
    for(n=2;n<=40;++n)for(mt=1;mt<=4;++mt) {
        static Ctx c;
        const uint32_t pairs=n*(n-1)/2;
        memset(&c,0,sizeof c);c.n=n;c.meetings=mt;c.last_round=~0u;c.last_cycle=~0u;
        assert(ExpandedUniversal_GenerateRoundRobin(n,(uint8_t)mt,emit,&c));
        assert(c.count==pairs*mt);
        for(i=0;i<n;++i)for(j=i+1;j<n;++j)
            for(cy=0;cy<mt;++cy) /* exactly one game of the pair per cycle,
                                    odd cycles are the reverse of even ones */
                assert(c.seen[i][j][cy]+c.seen[j][i][cy]==1 &&
                       (cy==0 || c.seen[i][j][cy]==c.seen[j][i][cy-1]));
        for(i=0;i<n;++i) {
            uint32_t k,run=1,maxrun=1,home=0;
            for(k=0;k<c.len[i];++k)home+=(uint32_t)c.pat[i][k];
            for(k=1;k<c.len[i];++k) {
                run=c.pat[i][k]==c.pat[i][k-1]?run+1:1;
                if(run>maxrun)maxrun=run;
            }
            if(mt==1) { /* one cycle: balanced, short runs */
                const uint32_t g=c.len[i];
                assert(home>=g/2 && home<=(g+1)/2 + (n&1u?1u:0u));
                assert(maxrun<=(n&1u?3u:2u));
            } else
                assert(maxrun<=3u); /* run can cross a cycle boundary */
            if(!(n&1u) && maxrun>worst_even)worst_even=maxrun;
        }
    }
    /* historical bug: 14 clubs gave up to 13 consecutive away games */
    assert(worst_even<=3u);
    printf("ROUND_ROBIN_HOME_AWAY_BALANCED_N2_40_CYCLES1_4=PASS worst_run=%u\n",worst_even);
    return 0;
}
