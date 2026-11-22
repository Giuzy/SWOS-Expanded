/* C11 proof using the REAL universal CAR codec, mjson, Gregorian calendar,
 * scheduler and fixture ledger. OG 50Hz is mocked ONLY in this unit test.
 * No disk, gameplay, live save, or extra simulator is touched by this test.
 */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "expanded_universal_car.h"

static WorldId s_world=WORLD_2627;
WorldId World_Current(void) {return s_world;}
void *swos_io_realloc(void *p,uint32_t n) {return realloc(p,n);}
void *swos_io_alloc(uint32_t n) {return malloc(n);}
void swos_io_free(void *p) {free(p);}

static char s_disk[65536];
static unsigned s_writes,s_games;
static int s_fail_completion;
static int32_t s_civil_clock;
int Expanded_AutoCareer_SaveChecked(const char *reason)
{
    struct mjson_fixedbuf dst={s_disk,(int)sizeof s_disk,0};
    ExpandedUniversalRegistry *r=ExpandedUniversal_CarRegistry();
    if(s_fail_completion && strstr(reason,"complete"))return 0;
    assert(r);
    assert(r->scheduler.clock.day_index==s_civil_clock);
    assert(mjson_printf(mjson_print_fixed_buf,&dst,
        "{%Q:{%Q:%d}","living_world","now",
        (int)s_civil_clock)>0);
    assert(ExpandedUniversal_CarEmit(mjson_print_fixed_buf,&dst));
    assert(mjson_printf(mjson_print_fixed_buf,&dst,"}")>0);
    assert(dst.len<(int)sizeof s_disk-1);
    assert(ExpandedUniversal_CarValidate(s_disk,dst.len));
    ++s_writes;
    return 1;
}
static int32_t date(int y,int m,int d)
{
    int32_t out=-1;
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){
        (uint16_t)y,(uint8_t)m,(uint8_t)d},&out));
    return out;
}
static ExpandedUniversalFixture fixture(uint64_t id,uint32_t tie,
                                         uint32_t home,uint32_t away,int32_t day)
{
    ExpandedUniversalFixture f={0};
    f.event_id=id;
    f.day_index=day;
    f.home=(ExpandedClubKey){WORLD_2627,home};
    f.away=(ExpandedClubKey){WORLD_2627,away};
    f.key.edition.competition=(ExpandedCompetitionKey){WORLD_2627,450001u};
    f.key.edition.start_day=day;
    f.key.edition.sequence=1u;
    f.key.stage=1u;f.key.tie=tie;
    f.country=45u;f.slot=0u;f.competition_type=0u;
    return f;
}
static int run_original_mock(void *p,const ExpandedUniversalFixture *f,
                              ExpandedUniversalMatchWitness *w)
{
    (void)p;
    ++s_games;
    *w=(ExpandedUniversalMatchWitness){0};
    w->key=f->key;w->home=f->home;w->away=f->away;
    w->day_index=f->day_index;
    w->score=(ExpandedFixtureScore){2u,1u,0u,0u,0u};
    w->valid=1u;
    return 1;
}
int main(void)
{
    const int32_t d=date(2026,7,1);
    const ExpandedUniversalOps ops={
        ExpandedUniversal_CarSaveClaim,run_original_mock,
        ExpandedUniversal_CarSaveCompletion,NULL};
    ExpandedUniversalRegistry *r;
    ExpandedUniversalFixture f0=fixture(UINT64_C(0x2000000000000001),1u,
                                        (45u<<16)|1u,(45u<<16)|2u,d);
    ExpandedUniversalFixture f1=fixture(UINT64_C(0x2000000000000002),2u,
                                        (45u<<16)|3u,(45u<<16)|4u,d);
    const ExpandedUniversalFixture *next=NULL;
    char corrupted[sizeof s_disk];
    s_civil_clock=d;
    assert(ExpandedUniversal_CarBegin((ExpandedWorldDate){2026,7,1},4u));
    r=ExpandedUniversal_CarRegistry();
    assert(r);
    assert(ExpandedUniversal_Add(r,f0)==EXP_UNIVERSAL_OK);
    assert(ExpandedUniversal_Add(r,f1)==EXP_UNIVERSAL_OK);
    assert(Expanded_AutoCareer_SaveChecked("initial"));
    /* Field checksum must reject any tampering, without mutating runtime. */
    strcpy(corrupted,s_disk);
    {
        char *p=strstr(corrupted,"UE01");
        assert(p);
        p[24]=(p[24]=='0')?'1':'0';
    }
    assert(!ExpandedUniversal_CarValidate(corrupted,(int)strlen(corrupted)));
    assert(ExpandedUniversal_CarRegistry()==r);
    /* Cross-world CAR cannot load or erase the 2627 state. */
    s_world=WORLD_9697;
    assert(!ExpandedUniversal_CarValidate(s_disk,(int)strlen(s_disk)));
    s_world=WORLD_2627;
    /* One real transaction in the test uses mock OG result, but REAL
     * production CAR Claim/Completion callbacks and codec. */
    assert(ExpandedUniversal_RunDue(r,d,&ops)==EXP_UNIVERSAL_OK);
    assert(s_games==1u && s_writes==3u);
    assert(ExpandedUniversal_CarValidate(s_disk,(int)strlen(s_disk)));
    assert(ExpandedUniversal_CarRestore(s_disk,(int)strlen(s_disk)));
    r=ExpandedUniversal_CarRegistry();
    assert(r && ExpandedUniversal_ValidateRestore(r));
    assert(ExpandedUniversal_Peek(r,d,&next)==EXP_UNIVERSAL_DUE);
    assert(next && next->event_id==f1.event_id);
    /* Save completion failure must leave durable CLAIMED and block retry. */
    s_fail_completion=1;
    assert(ExpandedUniversal_RunDue(r,d,&ops)==EXP_UNIVERSAL_RECOVERY);
    assert(s_games==2u);
    assert(ExpandedUniversal_RunDue(r,d,&ops)==EXP_UNIVERSAL_RECOVERY);
    assert(s_games==2u);
    assert(ExpandedUniversal_CarValidate(s_disk,(int)strlen(s_disk)));
    assert(ExpandedUniversal_CarRestore(s_disk,(int)strlen(s_disk)));
    r=ExpandedUniversal_CarRegistry();
    assert(r && r->recovery_required);
    assert(ExpandedUniversal_Peek(r,d,&next)==EXP_UNIVERSAL_RECOVERY);
    /* NULL section restores old-career compatibility without scores. */
    {
        char old[200];
        const int n=snprintf(old,sizeof old,
            "{\"living_world\":{\"now\":%d},\"universal_engine\":null}",
            (int)d);
        assert(n>0 && n<(int)sizeof old);
        assert(ExpandedUniversal_CarValidate(old,n));
        assert(ExpandedUniversal_CarRestore(old,n));
        assert(!ExpandedUniversal_CarRegistry());
    }
    printf("UNIVERSAL_CAR_CODEC_REAL_C11=PASS claims and ACK, CRC, world, rollback, legacy\n");
    printf("OG_50HZ_GAMEPLAY=NOT_TESTED LIVE_CUTOVER=NOT_IMPLEMENTED\n");
    return 0;
}
