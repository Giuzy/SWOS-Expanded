/* 70 seasons, four connected native divisions and 32 stable club IDs.
 * Uses the real production world engine and SWOS fixture pairing tables.
 * It does not substitute an independent competition simulator. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "expanded_world_live.h"
#include "fixture_rounds.h"
typedef struct B {char *p;size_t len,cap;} B;
static int emit(const char *s,int n,void *ctx) {
 B *b=(B*)ctx;if(!s||n<0)return 0;
 size_t need=b->len+(size_t)n+1u;
 if(need>b->cap) {
  size_t cap=b->cap?b->cap:4096u;
  while(cap<need)cap*=2u;
  char *p=(char*)realloc(b->p,cap);
  if(!p)return 0;
  b->p=p;b->cap=cap;
 }
 memcpy(b->p+b->len,s,(size_t)n);b->len+=(size_t)n;b->p[b->len]=0;
 return n;
}
static B saved(void) {
 B b={0};
 assert(emit("{\"living_world\":",16,&b)==16);
 assert(ExpandedWorldLive_WriteJson(emit,&b));
 assert(emit("}",1,&b)==1);
 return b;
}
static void round_trip(void) {
 B a=saved();
 uint32_t u=ExpandedWorldLive_Promotions();
 uint32_t d=ExpandedWorldLive_Relegations();
 assert(a.p && a.len>100);
 ExpandedWorldLive_End();
 assert(ExpandedWorldLive_ReadJson(a.p,(int)a.len));
 assert(ExpandedWorldLive_Promotions()==u);
 assert(ExpandedWorldLive_Relegations()==d);
 B b=saved();
 assert(a.len==b.len && memcmp(a.p,b.p,a.len)==0);
 free(a.p);free(b.p);
}
int main(void) {
 FixtureRounds_Install();
 assert(ExpandedWorldLive_Begin(1996));
 for(unsigned div=0;div<4u;++div) {
  for(unsigned i=0;i<8u;++i) {
   unsigned ord=div*8u+i;
   uint16_t id=(uint16_t)(0x0800u+ord);
   char name[18];
   snprintf(name,sizeof name,"DIV%u CLUB%u",div,i);
   unsigned rating=(ord==31u)?900u:(ord==0u)?1u:(150u+(ord*7u));
   assert(ExpandedWorldLive_AddClub(8u,(uint8_t)div,id,(uint16_t)rating,name,3u));
  }
  assert(ExpandedWorldLive_SetLeagueSchedule(8u,(uint8_t)div,56u,40u,2u));
  assert(ExpandedWorldLive_SetLeagueMovement(8u,(uint8_t)div,
      div?1u:0u,div<3u?1u:0u,0u,0u));
 }
 assert(ExpandedWorldLive_Seal());
 assert(ExpandedWorldLive_Clubs()==32u);
 assert(ExpandedWorldLive_Leagues()==4u);
 uint8_t min_div[32],max_div[32];
 for(unsigned i=0;i<32u;++i)min_div[i]=max_div[i]=(uint8_t)(i/8u);
 unsigned checkpoints=0;
 for(unsigned w=1u;w<=3679u;++w) {
  assert(ExpandedWorldLive_AdvanceWeeks(w));
  if(w%52u==0u) {
   ExpandedLiveCalendarState cal;
   assert(ExpandedWorldLive_CalendarState(&cal));
   assert(cal.overdue==0u);
   assert(cal.pending+ExpandedWorldLive_ResidentResults()==ExpandedWorldLive_Fixtures());
   uint8_t seen[32]={0};
   for(unsigned div=0;div<4u;++div) {
    ExpandedLiveRow rows[32];
    uint32_t total=0;
    assert(ExpandedWorldLive_Table(8u,(uint8_t)div,rows,32u,&total));
    assert(total==8u);
    for(unsigned j=0;j<total;++j) {
        unsigned ord=rows[j].club & 255u;
        assert((rows[j].club>>8)==8u && ord<32u && !seen[ord]);
        seen[ord]=1u;
    }
   }
   for(unsigned i=0;i<32u;++i) {
    uint8_t d=ExpandedWorldLive_ClubDivision((uint16_t)(0x0800u+i));
    assert(d<4u);
    if(d<min_div[i])min_div[i]=d;
    if(d>max_div[i])max_div[i]=d;
   }
   for(unsigned i=0;i<32u;++i)assert(seen[i]==1);
   if(w%(52u*10u)==0u) {round_trip();++checkpoints;}
  }
 }
 assert(ExpandedWorldLive_Promotions()==3u*70u);
 assert(ExpandedWorldLive_Relegations()==3u*70u);
 assert(ExpandedWorldLive_MobilityUnsupported()==0u);
 assert(ExpandedWorldLive_ArchivedLeagues()==4u*70u);
 unsigned full=0;
 for(unsigned i=0;i<32u;++i)
  if((unsigned)max_div[i]-(unsigned)min_div[i]>=3u)++full;
 /* The 1996/97 native fixture pairings and deterministic score resolver
  * must permit the deliberately weakest top-flight club to reach the
  * lowest division and the strongest bottom-flight club to reach the top. */
 assert(min_div[31]==0u && max_div[0]==3u);
 assert(full>=2u);
 printf("REAL_SWOS_FIXTURE_PYRAMID_CLUBS=32 DIVISIONS=4 YEARS=70\n");
 printf("PROMOTIONS=%lu RELEGATIONS=%lu\n",
        (unsigned long)ExpandedWorldLive_Promotions(),
        (unsigned long)ExpandedWorldLive_Relegations());
 printf("FULL_PYRAMID_CROSSINGS_OBSERVED=%u\n",full);
 printf("SAVE_LOAD_CHECKPOINTS=%u\n",checkpoints);
 puts("PYRAMID_MOVEMENT_NATIVE_RULES=PASS");
 puts("PYRAMID_FULL_UP_DOWN=PASS");
 puts("PYRAMID_IDENTITY_SAVE_LOAD=PASS");
 puts("PYRAMID_BOUNDARIES=PASS");
 puts("PYRAMID_STRESS_RESULT=PASS");
 ExpandedWorldLive_End();return 0;
}
