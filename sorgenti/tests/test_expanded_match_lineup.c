#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "expanded_match_lineup.h"
static void roster(ExpLineupCandidate *c,unsigned n)
{
    unsigned i;
    for(i=0;i<n;++i){c[i].roster_id=(uint16_t)(100+i);c[i].shirt=(uint8_t)(i+1);c[i].is_keeper=(i==0||i==12);c[i].available=1;}
}
int main(void)
{
    static const unsigned sizes[]={0,5,7,9,12,15};
    ExpLineupCandidate c[30];
    ExpLineup l;
    unsigned s,i,j;
    roster(c,30);
    for(s=0;s<sizeof sizes/sizeof *sizes;++s) {
        assert(ExpLineup_Build(&l,c,30,sizes[s]));
        assert(l.xi_filled==11 && l.bench_filled==sizes[s] && l.filled==11+sizes[s]);
        for(i=0;i<l.filled;++i)for(j=i+1;j<l.filled;++j)assert(l.roster_id[i]!=l.roster_id[j]);
        if(sizes[s])assert(l.bench_has_keeper && l.roster_id[11]==112);   /* keeper first on the bench */
    }
    /* players beyond the 16th slot reach the match squad (also among the starters) */
    for(i=0;i<10;++i)c[i].available=0;                               /* 10 unavailable at the top */
    assert(ExpLineup_Build(&l,c,30,15) && l.xi_filled==11 && l.roster_id[0]==110 && l.bench_filled==9);
    {   unsigned maxid=0; for(i=0;i<l.filled;++i)if(l.roster_id[i]>maxid)maxid=l.roster_id[i];
        assert(maxid==129); }                                        /* uses roster ordinal 29 */
    /* incomplete roster: no filler, honest shortage */
    roster(c,30);
    assert(ExpLineup_Build(&l,c,14,15) && l.xi_filled==11 && l.bench_filled==3 && l.filled==14);
    assert(ExpLineup_Build(&l,c,9,5) && l.xi_filled==9 && l.bench_filled==0);   /* never pads the XI */
    /* injured/suspended/red never selected */
    roster(c,30);c[2].available=0;c[13].available=0;
    assert(ExpLineup_Build(&l,c,30,7));
    for(i=0;i<l.filled;++i)assert(l.roster_id[i]!=102 && l.roster_id[i]!=113);
    /* no keeper available: bench still filled, flag false */
    roster(c,30);c[0].is_keeper=0;c[12].is_keeper=0;
    assert(ExpLineup_Build(&l,c,30,5) && !l.bench_has_keeper && l.bench_filled==5);
    /* negative: duplicate identity, too big, bench>15, null; output untouched */
    roster(c,30);c[5].roster_id=c[4].roster_id;
    memset(&l,0x5a,sizeof l);
    assert(!ExpLineup_Build(&l,c,30,5) && l.filled==0x5a);
    roster(c,30);
    assert(!ExpLineup_Build(&l,c,31,5) && !ExpLineup_Build(&l,c,30,16) && !ExpLineup_Build(NULL,c,30,5) && !ExpLineup_Build(&l,NULL,3,5));
    assert(ExpLineup_Build(&l,NULL,0,0) && l.filled==0);
    puts("EXP_MATCH_LINEUP=PASS_ISOLATED_ONLY");
    return 0;
}
