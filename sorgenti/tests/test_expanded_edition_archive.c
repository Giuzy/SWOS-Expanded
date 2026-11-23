/* v00.13.290 RG-T: synthetic edition archive (pure module). Isolated test:
 * sorted unique insert, ALREADY/CONFLICT/INVALID, the closure gate against a
 * registry-shaped fixture/ledger array, anti-replay audit, UA01 codec round
 * trip, every-byte corruption, truncation, wrong world and forged (but
 * hash-correct) unsorted/duplicate/invalid payloads. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "expanded_edition_archive.h"

static ExpandedEditionKey ed(uint32_t comp,int32_t start,uint16_t seq)
{
    ExpandedEditionKey e={{2627,comp},start,seq};
    return e;
}
static ExpandedEditionSummary sum(ExpandedEditionKey e,int32_t closed,
                                  uint32_t ch,uint32_t ru,uint32_t n)
{
    ExpandedEditionSummary s;
    memset(&s,0,sizeof s);
    s.edition=e;s.closed_day=closed;s.champion=ch;s.runner_up=ru;s.match_count=n;
    return s;
}
static uint64_t fnv(const uint8_t *p,size_t n)
{
    uint64_t h=1469598103934665603ull;size_t i;
    for(i=0;i<n;++i){h^=p[i];h*=1099511628211ull;}
    return h;
}
static void refix(uint8_t *b,size_t len)
{
    const uint64_t h=fnv(b,len-8u);size_t k;
    for(k=0;k<8;++k)b[len-8u+k]=(uint8_t)(h>>(8*k));
}
int main(void)
{
    ExpandedEditionArchive a={0},b={0};
    ExpandedEditionSummary s1=sum(ed(10,100,1),200,1001,1002,38);
    ExpandedEditionSummary s2=sum(ed(5,50,1),150,2001,0,10);
    ExpandedEditionSummary s3=sum(ed(10,100,2),210,3001,3002,6);
    size_t len=0,k;uint8_t *buf,*mut;
    /* insert keeps the array sorted and unique */
    assert(ExpArchive_Insert(&a,&s1)==EXP_ARCHIVE_OK);
    assert(ExpArchive_Insert(&a,&s2)==EXP_ARCHIVE_OK);
    assert(ExpArchive_Insert(&a,&s3)==EXP_ARCHIVE_OK);
    assert(a.count==3 && a.items[0].edition.competition.stable_competition==5 &&
           a.items[1].edition.sequence==1 && a.items[2].edition.sequence==2);
    assert(ExpArchive_Insert(&a,&s1)==EXP_ARCHIVE_ALREADY);
    { ExpandedEditionSummary x=s1;x.champion=1003;
      assert(ExpArchive_Insert(&a,&x)==EXP_ARCHIVE_CONFLICT); }
    { ExpandedEditionSummary x=sum(ed(11,100,1),200,0,0,3);          /* no champion */
      assert(ExpArchive_Insert(&a,&x)==EXP_ARCHIVE_INVALID);
      x=sum(ed(11,100,1),99,5,0,3);                                  /* closes before start */
      assert(ExpArchive_Insert(&a,&x)==EXP_ARCHIVE_INVALID);
      x=sum(ed(11,100,1),200,5,5,3);                                 /* runner-up == champion */
      assert(ExpArchive_Insert(&a,&x)==EXP_ARCHIVE_INVALID);
      x=sum(ed(11,100,1),200,5,6,0);                                 /* no matches */
      assert(ExpArchive_Insert(&a,&x)==EXP_ARCHIVE_INVALID); }
    assert(a.count==3 && ExpArchive_Validate(&a,2627) && !ExpArchive_Validate(&a,9697));
    assert(ExpArchive_Find(&a,ed(10,100,2))==&a.items[2] && !ExpArchive_Find(&a,ed(10,100,3)));
    /* codec round trip + every byte flipped is rejected */
    buf=ExpArchive_Encode(&a,2627,&len);assert(buf && len==10+3*27+8);
    assert(ExpArchive_Decode(&b,buf,len,2627) && b.count==3 &&
           !memcmp(b.items,a.items,3*sizeof a.items[0]));
    mut=(uint8_t *)malloc(len);assert(mut);
    for(k=0;k<len;++k) {
        ExpandedEditionArchive t={0};
        memcpy(mut,buf,len);mut[k]^=0x41;
        assert(!ExpArchive_Decode(&t,mut,len,2627) && t.count==0);
    }
    for(k=1;k<30;++k) {
        ExpandedEditionArchive t={0};
        assert(!ExpArchive_Decode(&t,buf,len-k,2627));
    }
    { ExpandedEditionArchive t={0};assert(!ExpArchive_Decode(&t,buf,len,9697)); }
    /* forged payloads with a CORRECT hash must still fail validation */
    memcpy(mut,buf,len);                       /* swap record 0 and 1 -> unsorted */
    memcpy(mut+10,buf+10+27,27);memcpy(mut+10+27,buf+10,27);refix(mut,len);
    { ExpandedEditionArchive t={0};assert(!ExpArchive_Decode(&t,mut,len,2627)); }
    memcpy(mut,buf,len);                       /* duplicate edition */
    memcpy(mut+10+27,buf+10,27);refix(mut,len);
    { ExpandedEditionArchive t={0};assert(!ExpArchive_Decode(&t,mut,len,2627)); }
    memcpy(mut,buf,len);mut[10+26]=7;refix(mut,len);   /* facts flag out of range */
    { ExpandedEditionArchive t={0};assert(!ExpArchive_Decode(&t,mut,len,2627)); }
    free(mut);free(buf);
    /* empty archive round-trips */
    { ExpandedEditionArchive e={0},t={0};size_t l=0;uint8_t *x=ExpArchive_Encode(&e,2627,&l);
      assert(x && ExpArchive_Decode(&t,x,l,2627) && t.count==0);free(x); }
    /* copy is independent */
    { ExpandedEditionArchive c={0};
      assert(ExpArchive_Copy(&c,&a) && c.count==3);
      c.items[0].champion=9999u;assert(a.items[0].champion!=9999u);
      ExpArchive_Free(&c); }
    /* closure gate against a registry-shaped array */
    {
        ExpandedUniversalFixture fx[3];ExpandedFixtureLedgerEntry led[3];
        ExpandedEditionArchive g={0};
        ExpandedEditionKey e=ed(40,300,1);
        int i;
        memset(fx,0,sizeof fx);memset(led,0,sizeof led);
        for(i=0;i<3;++i) {
            fx[i].key.edition=e;fx[i].key.stage=1u;fx[i].key.tie=(uint32_t)i+1u;
            fx[i].home.world=2627;fx[i].away.world=2627;
            fx[i].home.stable_club=100u+(uint32_t)i;
            fx[i].away.stable_club=200u+(uint32_t)i;
            fx[i].day_index=300+i*7;
            led[i].key=fx[i].key;led[i].state=EXP_FIXTURE_PLAYED;
        }
        led[2].state=EXP_FIXTURE_SCHEDULED;
        assert(ExpArchive_CloseFromRegistry(&g,fx,led,3,e,100u,201u,330)==EXP_ARCHIVE_OPEN);
        led[2].state=EXP_FIXTURE_CLAIMED;
        assert(ExpArchive_CloseFromRegistry(&g,fx,led,3,e,100u,201u,330)==EXP_ARCHIVE_OPEN);
        led[2].state=EXP_FIXTURE_PLAYED;
        assert(ExpArchive_CloseFromRegistry(&g,fx,led,3,ed(41,300,1),100u,0u,330)==EXP_ARCHIVE_UNKNOWN);
        assert(ExpArchive_CloseFromRegistry(&g,fx,led,3,e,555u,0u,330)==EXP_ARCHIVE_INVALID);
        assert(ExpArchive_CloseFromRegistry(&g,fx,led,3,e,100u,555u,330)==EXP_ARCHIVE_INVALID);
        assert(ExpArchive_CloseFromRegistry(&g,fx,led,3,e,100u,201u,313)==EXP_ARCHIVE_INVALID); /* last fixture 314 */
        assert(ExpArchive_CloseFromRegistry(&g,fx,led,3,e,100u,100u,330)==EXP_ARCHIVE_INVALID);
        assert(g.count==0);
        assert(ExpArchive_CloseFromRegistry(&g,fx,led,3,e,100u,201u,330)==EXP_ARCHIVE_OK &&
               g.count==1 && g.items[0].match_count==3);
        assert(ExpArchive_AuditFixtures(&g,fx,led,3));
        led[1].state=EXP_FIXTURE_SCHEDULED;                       /* replay hole */
        assert(!ExpArchive_AuditFixtures(&g,fx,led,3));
        ExpArchive_Free(&g);
    }
    ExpArchive_Free(&a);ExpArchive_Free(&b);
    puts("EXP_EDITION_ARCHIVE=PASS_ISOLATED_ONLY");
    puts("EDITION_CLOSING_LOGIC=NOT_IMPLEMENTED_BY_DESIGN GAME_CAR_AND_WINDOWS=NOT_TESTED_HERE");
    return 0;
}
