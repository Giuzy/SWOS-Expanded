/* RG-CT-OG-SHARED: isolated source test, no gameplay claim. */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "expanded_national_2627.h"
#include "expanded_nominal_world_2627.h"
#include "expanded_ce_shared_pool_identity.h"
static WorldId active=WORLD_2627;
WorldId World_Current(void){return active;}
int main(void)
{
    static union { uint16_t align; uint8_t data[2+64*sizeof(TeamFileEx)]; } storage;
    unsigned new_count=0, inactive=0;
    uint16_t slot;
    const unsigned expected[6]={55,53,10,35,47,11}; /* storage */
    const unsigned expected_regions[6]={55,54,10,35,46,11}; /* FIFA confederations, independent of storage */
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
        }else if(!strcmp(r->fifa,"AUS")){
            assert(r->og.countryNumber==85u && r->og.teamOrdinal==0u);
            assert(r->og.nationalityCode==148u && r->og.teamFileId==44u && r->added_global==65535u);
            assert(r->confederationRegion==84u && ExpandedNational_Raw(r)==0x5500u);
        }else if(!strcmp(r->fifa,"PNG")){
            assert(r->og.countryNumber==84u && r->og.teamOrdinal==19u);
            assert(r->og.nationalityCode==143u && r->og.teamFileId==255u && r->added_global==65535u);
            assert(r->confederationRegion==85u && ExpandedNational_Raw(r)==0x5413u);
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
    /* START and WORLD use one gate, with no leakage into ordinary pickers.
     * START overrides stale national/career globals, never an offer. */
    assert(ExpandedNational_ClubPickerEnabled(1,0,0,0,0));
    assert(ExpandedNational_ClubPickerEnabled(1,1,0,1,0));
    assert(ExpandedNational_ClubPickerEnabled(0,1,1,0,0));
    assert(!ExpandedNational_ClubPickerEnabled(0,1,1,1,0));
    assert(!ExpandedNational_ClubPickerEnabled(0,1,0,0,0));
    assert(!ExpandedNational_ClubPickerEnabled(0,0,1,0,0));
    assert(!ExpandedNational_ClubPickerEnabled(1,1,0,0,1));
    assert(!ExpandedNational_ClubPickerEnabled(0,1,1,0,1));
    active=WORLD_9697;
    assert(!ExpandedNational_ClubPickerEnabled(1,1,0,0,0));
    assert(!ExpandedNational_ClubPickerEnabled(0,1,1,0,0));
    active=WORLD_2627;
    /* Exact WORLD/START coverage: 91 native country IDs, 120 no-home rows. */
    {
        const uint8_t r0[]={0,47,52,1,53,76,2,54,3,4,5,6,7,8,10,11,12,13,61,14,63,15,25,16,17,19,20,86,88,21,90,22,23,24,92,94,93,26,27,28,29,18,30,31,32,33,41,78,34,35,36,37,38,39,40,255};
        const uint8_t r1[]={42,59,79,91,69,97,255};
        const uint8_t r2[]={43,45,46,48,49,50,64,65,66,71,77,255};
        const uint8_t r3[]={56,58,51,68,70,60,95,73,255};
        const uint8_t r4[]={57,75,74,55,96,89,87,67,98,255};
        const uint8_t r5[]={44,62,255};
        const uint8_t *sources[]={r0,r1,r2,r3,r4,r5};
        const unsigned nativeExpected[]={55,6,10,9,10,1};
        const unsigned inactiveExpected[]={0,48,0,26,36,10};
        unsigned all=0;
        uint8_t seen[EXP_NATIONAL_COUNT]={0};
        for(unsigned region=80;region<=85;region++){
            uint8_t out[65];
            int n=ExpandedNational_WorldClubCountries((uint8_t)region,sources,out,sizeof out);
            assert(n==(int)nativeExpected[region-80u]);
            for(int j=0;j<n;j++){
                const ExpandedNationalRow *r=ExpandedNational_ByHome(out[j]);
                assert(r && r->confederationRegion==region);
                assert(!seen[r-kExpandedNational2627]++);
            }
            unsigned inactive=0;
            for(unsigned j=0;j<EXP_NATIONAL_COUNT;j++){
                const ExpandedNationalRow *r=&kExpandedNational2627[j];
                if(r->confederationRegion!=region || !ExpandedNational_WorldInactiveClubRow(r,1))continue;
                assert(!seen[j]++);
                ++inactive;
            }
            assert(inactive==inactiveExpected[region-80u]);
            assert((unsigned)n+inactive==expected_regions[region-80u]);
            all+=(unsigned)n+inactive;
        }
        assert(all==EXP_NATIONAL_COUNT);
        for(unsigned j=0;j<EXP_NATIONAL_COUNT;j++)assert(seen[j]==1u);
    }
    assert(!strcmp(ExpandedNational_ByRaw(0x540eu)->fifa,"MRI"));
    assert(ExpandedNational_ByNationality(141u)==ExpandedNational_ByRaw(0x540eu));
    assert(!strcmp(ExpandedNational_ByRaw(0x5500u)->fifa,"AUS"));
    assert(!strcmp(ExpandedNational_ByRaw(0x5413u)->fifa,"PNG"));
    assert(ExpandedNational_ByNationality(148u)==ExpandedNational_ByRaw(0x5500u));
    assert(ExpandedNational_ByNationality(143u)==ExpandedNational_ByRaw(0x5413u));
    active=WORLD_9697;
    assert(ExpandedNational_ByRaw(0x540eu)==NULL);
    assert(ExpandedNational_ByRaw(0x5500u)==NULL);
    assert(ExpandedNational_ByRaw(0x5413u)==NULL);
    active=WORLD_2627;
    assert(new_count==69u && inactive==63u);
    {
        const uint8_t eu[]={0,255},af[]={42,59,79,91,69,97,255};
        const uint8_t sa[]={43,45,46,48,49,50,64,65,66,71,77,255},na[]={56,58,51,68,70,60,95,73,255};
        const uint8_t as[]={57,75,74,55,96,89,87,67,98,255},oc[]={44,62,255};
        const uint8_t *sources[]={eu,af,sa,na,as,oc};
        const uint8_t asiaExpected[]={44,57,75,74,55,96,89,87,67,98,255};
        uint8_t out[65];
        assert(ExpandedNational_WorldClubCountries(81u,sources,out,sizeof out)==6);
        assert(!memcmp(out,af,sizeof af));
        const uint8_t northExpected[]={66,56,58,51,68,70,60,95,73,255};
        assert(ExpandedNational_WorldClubCountries(83u,sources,out,sizeof out)==9);
        assert(!memcmp(out,northExpected,sizeof northExpected));
        const uint8_t southExpected[]={43,45,46,48,49,50,64,65,71,77,255};
        const char *const southCodes[]={"ARG","BOL","BRA","CHI","COL","ECU","PAR","PER","URU","VEN"};
        assert(ExpandedNational_WorldClubCountries(82u,sources,out,sizeof out)==10);
        assert(!memcmp(out,southExpected,sizeof southExpected));
        for(unsigned i=0;i<10u;i++){
            const ExpandedNationalRow *r=ExpandedNational_ByHome(out[i]);
            assert(r && !strcmp(r->fifa,southCodes[i]) && r->confederationRegion==82u);
            assert(ExpandedNational_Raw(r)==(uint16_t)(0x5200u+i));
            assert(!ExpandedNational_WorldInactiveClubRow(r,1));
        }
        const ExpandedNationalRow *sur=ExpandedNational_ByHome(66u);
        assert(sur && !strcmp(sur->fifa,"SUR") && sur->confederationRegion==83u);
        assert(ExpandedNational_Raw(sur)==0x530cu && sur->og.nationalityCode==75u);
        unsigned northInactive=0;
        for(unsigned i=0;i<EXP_NATIONAL_COUNT;i++){
            const ExpandedNationalRow *r=&kExpandedNational2627[i];
            if(r->confederationRegion==83u && ExpandedNational_WorldInactiveClubRow(r,1))++northInactive;
        }
        assert(northInactive==26u);
        assert(ExpandedNational_WorldInactiveClubRow(ExpandedNational_ByRaw(0x5305u),1));
        assert(!ExpandedNational_WorldInactiveClubRow(ExpandedNational_ByRaw(0x5305u),0));
        assert(ExpandedNational_WorldClubCountries(84u,sources,out,sizeof out)==10);
        assert(!memcmp(out,asiaExpected,sizeof asiaExpected));
        assert(ExpandedNational_WorldClubCountries(85u,sources,out,sizeof out)==1);
        assert(out[0]==62u && out[1]==255u);
        const ExpandedNationalRow *nzl=ExpandedNational_ByHome(62u);
        assert(nzl && !strcmp(nzl->fifa,"NZL") && nzl->confederationRegion==85u);
        assert(ExpandedNational_Raw(nzl)==0x5502u && nzl->og.nationalityCode==149u);
        unsigned ofcInactive=0;
        for(unsigned i=0;i<EXP_NATIONAL_COUNT;i++){
            const ExpandedNationalRow *r=&kExpandedNational2627[i];
            if(r->confederationRegion==85u && ExpandedNational_WorldInactiveClubRow(r,1))++ofcInactive;
        }
        assert(ofcInactive==10u);
        const ExpandedNationalRow *fiji=ExpandedNational_ByRaw(0x5501u);
        const ExpandedNationalRow *sol=ExpandedNational_ByRaw(0x5503u);
        assert(fiji && !strcmp(fiji->fifa,"FIJ") && fiji->confederationRegion==85u);
        assert(sol && !strcmp(sol->fifa,"SOL") && sol->confederationRegion==85u);
        assert(ExpandedNational_WorldInactiveClubRow(fiji,1));
        assert(ExpandedNational_WorldInactiveClubRow(sol,1));
        assert(!ExpandedNational_WorldInactiveClubRow(fiji,0));
        assert(!ExpandedNational_WorldInactiveClubRow(sol,0));
        assert(!ExpandedNational_WorldInactiveClubRow(nzl,1));
        assert(ExpandedNational_WorldClubCountries(84u,sources,out,10u)<0);
        assert(ExpandedNational_WorldInactiveClubRow(ExpandedNational_ByRaw(0x5413u),0));
        assert(ExpandedNational_WorldInactiveClubRow(ExpandedNational_ByRaw(0x540eu),0));
        assert(!ExpandedNational_WorldInactiveClubRow(ExpandedNational_ByRaw(0x5500u),0));
        const uint8_t dup[]={44,62,44,255};sources[5]=dup;
        assert(ExpandedNational_WorldClubCountries(84u,sources,out,sizeof out)<0);
        sources[5]=oc;
        active=WORLD_9697;
        assert(ExpandedNational_WorldClubCountries(84u,sources,out,sizeof out)<0);
        active=WORLD_2627;
    }
    /* Catalog-only pilot: no native home, no fake PlayerFile/TEAM records.
     * All names remain available, including source rosters above 30. */
    {
        const unsigned expectedClubs[]={7,11,8,12,28,10,9,18,16,13,13,18,11,18,20,14,10,12,10,20,14,10,8,18,14,16,12,16,10},expectedNames[]={168,328,190,350,908,312,231,538,506,442,369,671,340,563,554,372,279,359,250,879,505,315,203,639,429,439,274,431,355};
        const uint16_t raw[]={0x5312u,0x5421u,0x5318u,0x5424u,0x512cu,0x5426u,0x531du,0x5130u,0x542au,0x5401u,0x5402u,0x5109u,0x5404u,0x5406u,0x5408u,0x5307u,0x540au,0x540du,0x5309u,0x5116u,0x5411u,0x5414u,0x5417u,0x511fu,0x541du,0x5423u,0x542bu,0x542eu,0x5501u};
        assert(EXP_NOMINAL_LEN(kExpNominalWorld2627)==EXP_NOMINAL_LEN(raw));
        for(unsigned k=0;k<EXP_NOMINAL_LEN(raw);++k){
            const ExpNominalCountry *c=ExpNominalWorld_Find(raw[k]);
            const ExpandedNationalRow *r=ExpandedNational_ByRaw(raw[k]);
            assert(c && r && r->og.teamFileId==255u);
            assert(ExpandedNational_WorldInactiveClubRow(r,1));
            assert(c->count==expectedClubs[k]);
            unsigned names=0;
            for(unsigned i=0;i<c->count;++i){
                const ExpNominalClub *club=&c->clubs[i];
                assert(club->name[0] && club->id[0] && club->count>=16u && club->count<=64u);
                for(unsigned j=0;j<i;++j)assert(strcmp(club->id,c->clubs[j].id));
                for(unsigned j=0;j<club->count;++j){
                    assert(club->players[j].id[0] && club->players[j].name[0]);
                    for(unsigned m=0;m<j;++m)assert(strcmp(club->players[j].id,club->players[m].id));
                }
                names+=club->count;
                /* The native 17 visible rows reach every nominal member,
                 * up to the user-approved 32 names, with no fake slots. */
                unsigned visible=ExpNominalSquad_Count(club);
                assert(visible==(club->count>32u?32u:club->count));
                unsigned offset=0,max=visible>17u?visible-17u:0u;
                for(unsigned j=0;j<max;++j){
                    assert(ExpNominalSquad_Offset(visible,offset,16u,4u)==offset+1u);
                    offset=ExpNominalSquad_Offset(visible,offset,16u,4u);
                }
                assert(ExpNominalSquad_Offset(visible,offset,16u,4u)==max);
                assert(offset+17u>=visible);
                while(offset){
                    assert(ExpNominalSquad_Offset(visible,offset,0u,0u)==offset-1u);
                    --offset;
                }
                assert(ExpNominalSquad_Offset(visible,0u,0u,0u)==0u);
            }
            assert(names==expectedNames[k]);
        }
        for(unsigned i=0;i<EXP_NATIONAL_COUNT;++i){
            uint16_t id=ExpandedNational_Raw(&kExpandedNational2627[i]);
            int expected=0;
            for(unsigned k=0;k<EXP_NOMINAL_LEN(raw);++k)if(id==raw[k])expected=1;
            assert((ExpNominalWorld_Find(id)!=NULL)==expected);
        }
    }
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
