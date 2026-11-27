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
    const unsigned expected[6]={55,53,10,35,47,11}; /* storage */
    const unsigned expected_regions[6]={55,54,10,35,46,11}; /* interim grouping */
    unsigned regions[6]={0};
    assert(sizeof(TeamFileEx)==684u);
    for(unsigned i=0;i<EXP_NATIONAL_COUNT;i++){
        const ExpandedNationalRow *r=&kExpandedNational2627[i];
        assert(r->confederationRegion>=80u && r->confederationRegion<=85u);
        ++regions[r->confederationRegion-80u];
        if(!strcmp(r->fifa,"MRI")){
            assert(r->og.countryNumber==84u && r->og.teamOrdinal==14u);
            assert(r->og.nationalityCode==141u && r->added_global==65535u);
            assert(r->confederationRegion==81u && ExpandedNational_Raw(r)==0x540eu);
        }else assert(r->confederationRegion==r->og.countryNumber);
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
    for(unsigned i=0;i<6u;i++)assert(regions[i]==expected_regions[i]);
    assert(!strcmp(ExpandedNational_ByRaw(0x540eu)->fifa,"MRI"));
    assert(ExpandedNational_ByNationality(141u)==ExpandedNational_ByRaw(0x540eu));
    active=WORLD_9697;
    assert(ExpandedNational_ByRaw(0x540eu)==NULL);
    active=WORLD_2627;
    assert(new_count==69u && inactive==63u);
    assert(ExpandedNational_ByHome(255u)==NULL);
    assert(ExpCePool_GlobalSlot(2560u,2313u,14u,&slot)&&slot==2546u);
    for(unsigned file=80;file<=85;file++){
        memset(storage.data,0,sizeof storage.data);
        TeamFileEx *t=(TeamFileEx *)(storage.data+2);
        int n=0;
        for(unsigned i=0;i<EXP_NATIONAL_COUNT;i++){
            const ExpandedNationalRow *r=&kExpandedNational2627[i];
            if(r->og.countryNumber!=file)continue;
            t[n].team.countryNumber=(uint8_t)file;
            t[n].team.teamOrdinal=r->og.teamOrdinal;
            t[n].team.globalTeamNumber=(int16_t)(r->added_global==65535u?100+n:r->added_global);
            memcpy(t[n].team.teamName,"SQUAD",5u);
            for(unsigned p=0;p<16u;p++){
                t[n].team.playerNumbers[p]=(uint8_t)p;
                t[n].players[p].nationality=(int8_t)r->og.nationalityCode;
                memcpy(t[n].players[p].playerName,"REAL",4u);
            }
            ++n;
        }
        uint16_t count=(uint16_t)n;memcpy(storage.data,&count,2);
        assert((unsigned)n==expected[file-80]);
        assert(ExpandedNational_Validate((int)file,storage.data,n,64)==n);
        assert(ExpandedNational_Validate((int)file,storage.data,n-1,64)<0);
        assert(ExpandedNational_Validate((int)file,storage.data,n,n-1)<0);
        uint8_t saved=t[1].team.teamOrdinal;t[1].team.teamOrdinal=t[0].team.teamOrdinal;
        assert(ExpandedNational_Validate((int)file,storage.data,n,64)<0);t[1].team.teamOrdinal=saved;
        memcpy(t[0].players[0].playerName,"TEST",4u);
        assert(ExpandedNational_Validate((int)file,storage.data,n,64)<0);
        memcpy(t[0].players[0].playerName,"REAL",4u);
        int8_t nat=t[0].players[0].nationality;t[0].players[0].nationality=(int8_t)((uint8_t)nat^1u);
        assert(ExpandedNational_Validate((int)file,storage.data,n,64)<0);
        t[0].players[0].nationality=nat;
        t[0].team.playerNumbers[1]=0;
        assert(ExpandedNational_Validate((int)file,storage.data,n,64)<0);
        t[0].team.playerNumbers[1]=1;
        if(file!=82u){
            int16_t global=t[n-1].team.globalTeamNumber;t[n-1].team.globalTeamNumber=0;
            assert(ExpandedNational_Validate((int)file,storage.data,n,64)<0);
            t[n-1].team.globalTeamNumber=global;
        }
        active=WORLD_9697;
        assert(ExpandedNational_Validate((int)file,storage.data,n-1,64)==n-1);
        assert(ExpandedNational_ByRaw((uint16_t)(file<<8))==NULL);
        active=WORLD_2627;
        assert(ExpandedNational_Validate((int)file,storage.data,n,64)==n);
    }
    puts("NATIONAL_2627_REAL_ROSTER_GATE=PASS");
    return 0;
}
