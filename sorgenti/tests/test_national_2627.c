/* RG-CT-OG-SHARED: isolated source test, no gameplay claim. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "expanded_national_2627.h"
#include "expanded_ce_shared_pool_identity.h"
static WorldId active=WORLD_2627;
WorldId World_Current(void){return active;}
int main(void)
{
    static union { uint16_t align; uint8_t data[2+64*sizeof(TeamFileEx)]; } storage;
    unsigned new_count=0, inactive=0;
    uint16_t slot;
    const unsigned expected[6]={55,53,10,35,47,11};
    assert(sizeof(TeamFileEx)==684u);
    for(unsigned i=0;i<EXP_NATIONAL_COUNT;i++){
        const ExpandedNationalRow *r=&kExpandedNational2627[i];
        assert(ExpandedNational_ByRaw(ExpandedNational_Raw(r))==r);
        assert(ExpandedNational_ByNationality(r->og.nationalityCode)==r);
        for(unsigned j=0;j<i;j++){
            assert(ExpandedNational_Raw(r)!=ExpandedNational_Raw(&kExpandedNational2627[j]));
            assert(r->og.nationalityCode!=kExpandedNational2627[j].og.nationalityCode);
        }
        if(r->added_global!=65535u){
            assert(r->added_global>=2560u && r->added_global<GLOBAL_TEAM_NUMBERS);
            assert(strlen(r->native_name)<=16u);
            for(unsigned j=0;j<i;j++)assert(r->added_global!=kExpandedNational2627[j].added_global);
            ++new_count;if(r->og.teamFileId==255u)++inactive;
        }
        if(r->og.teamFileId!=255u)assert(ExpandedNational_ByHome(r->og.teamFileId)==r);
    }
    assert(new_count==69u && inactive==63u);
    assert(ExpandedNational_ByHome(255u)==NULL);
    assert(ExpCePool_GlobalSlot(2560u,2313u,14u,&slot)&&slot==2546u);
    for(unsigned file=80;file<=85;file++){
        memset(storage.data,0,sizeof storage.data);
        TeamFileEx *t=(TeamFileEx *)(storage.data+2);
        int n=0;
        for(unsigned i=0;i<EXP_NATIONAL_COUNT;i++){
            const ExpandedNationalRow *r=&kExpandedNational2627[i];
            if(r->og.countryNumber==file && r->added_global==65535u){
                t[n].team.countryNumber=(uint8_t)file;
                t[n].team.teamOrdinal=r->og.teamOrdinal;
                t[n].team.globalTeamNumber=(int16_t)(100+n);
                ++n;
            }
        }
        uint16_t count=(uint16_t)n;memcpy(storage.data,&count,2);
        if((unsigned)n<expected[file-80])assert(ExpandedNational_Append((uint8_t)file,storage.data,n,n)<0);
        active=WORLD_9697;
        assert(ExpandedNational_Append((uint8_t)file,storage.data,n,64)==n);
        assert(ExpandedNational_ByRaw((uint16_t)(file<<8))==NULL);
        active=WORLD_2627;
        assert(ExpandedNational_Append((uint8_t)file,storage.data,n,64)==(int)expected[file-80]);
        for(int j=0;j<n;j++)assert(t[j].team.globalTeamNumber==100+j);
        for(unsigned j=(unsigned)n;j<expected[file-80];j++){
            const ExpandedNationalRow *r=ExpandedNational_ByRaw((uint16_t)((file<<8)|t[j].team.teamOrdinal));
            assert(r && t[j].team.globalTeamNumber==r->added_global);
            for(unsigned p=0;p<16;p++){
                const PlayerFile *v=&t[j].players[p];
                assert(!strcmp((const char *)v->playerName,"TEST"));
                assert((uint8_t)v->nationality==r->og.nationalityCode);
                assert(v->price==0 && v->passing==0 && v->velocityHeading==0 && v->tacklingControl==0 && v->speedFinishing==0);
            }
        }
        if((unsigned)n<expected[file-80])assert(ExpandedNational_Append((uint8_t)file,storage.data,(int)expected[file-80],64)<0);
    }
    puts("NATIONAL_2627_IDENTITY_APPEND=PASS");
    return 0;
}
