/* Regression: do not offer a non-league team; do not load team files
 * while offer/preview menus are active; accept and return a playable ID. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "menu_system.h"
#include "game_types.h"

uint8_t _expandedClubOfferMenu[8],_expandedOfferPreviewMenu[8];
uint8_t _expandedManagerFame=0;
static unsigned savedPremium=0;
void Expanded_SetContractPremium(unsigned premium){savedPremium=premium;}
QualifyingGroup _europeTable={0},_africaTable={0},_southAmericaTable={0},_northAmericaTable={0},_asiaTable={0},_oceaniaTable={0};
const uint8_t *World_ContinentCountries(uint8_t region){(void)region;return NULL;}
static uint8_t calendar[8];
uint8_t _teamFileBuffer[2+684*8];
uint8_t _managerNameBuffer[32]="TEST", _managerSurnameBuffer[32]="MANAGER";
uint8_t _currentMenu[sizeof(Menu)];
uint16_t _teamsLoaded,_fireResetFlag;
MenuEntry _menuEntries[44];
void *_charTables[1]={0};
static const char country[]="\0TEST COUNTRY";
void *_countriesTable[262]={0};
static unsigned loads=0,offers=0,previews=0,exits=0,date_y=0;
static uint16_t resultTeam=0;
extern void Expanded_UnemployedInit(void),Expanded_UnemployedWait(void);
extern void Expanded_UnemployedShowOffer(void),Expanded_UnemployedAccept(void),Expanded_UnemployedReject(void);
extern void Expanded_OfferViewSquad(void),Expanded_OfferViewLeague(void);
extern void Expanded_OfferMore(void),Expanded_OfferLess(void);
extern uint32_t Expanded_OfferBaseWeekly(void),Expanded_OfferDemandWeekly(void);
extern unsigned Expanded_OfferRejections(void);
extern unsigned Expanded_OfferAcceptanceChance(uint32_t,uint32_t,unsigned,unsigned);
extern const char *Expanded_OfferRegion(void);
extern void Expanded_OfferPreviewDraw(void),Expanded_UnemployedDraw(void);
int Expanded_UI_Active(void){return 0;}
extern int Expanded_WaitForFirstOffer(uint16_t *team);
extern uint32_t _expandedAnnualSalary;
extern void Expanded_CommitNegotiatedSalary(void);

uint8_t *Expanded_UnemployedCalendarMenu(void){return calendar;}
void DrawMenu(void){}
void SetExitMenuFlag(void){exits++;}
void Expanded_FormatDate(char *out,size_t n,unsigned y,unsigned w){snprintf(out,n,"TEST %u %u",y,w);}
uint16_t DrawMenuTextCentered(uint16_t x,uint16_t y,uint16_t color,uint8_t *s,uint8_t *table){
    (void)x;(void)color;(void)table;
    if(strncmp((char*)s,"TEST 1996",9)==0) date_y=y;
    return 0;
}
/* The real game has a country-specific playable league table: league 4 is
 * 'non-league', not a valid fifth division. */
int32_t Team_IsOutsideLeague(const TeamFile *t){return t->league>=4;}
int16_t LoadTeamFile(uint16_t ext){
    assert(ext==8||ext==13||ext==14||ext==20||ext==21);
    loads++;
    *(uint16_t*)_teamFileBuffer=5;
    TeamFileEx *teams=(TeamFileEx*)(_teamFileBuffer+2);
    memset(teams,0,sizeof(TeamFileEx)*5);
    for(unsigned i=0;i<5;i++){
        teams[i].team.countryNumber=(uint8_t)ext;
        teams[i].team.teamOrdinal=(uint8_t)(i+1);
        teams[i].team.league=(uint8_t)i;
        snprintf((char*)teams[i].team.teamName,17,"CLUB%u",i+1);
        for(unsigned j=0;j<16;j++)
            snprintf((char*)teams[i].players[j].playerName,23,"PLAYER%u",j+1);
    }
    return 0;
}
void ShowMenu(uint8_t *menu){
    if(menu==calendar){
        for(int week=0;week<12;week++){
            Expanded_UnemployedInit();
            Expanded_UnemployedDraw();
            if(!_menuEntries[10].isDisabled){
                Expanded_UnemployedShowOffer();
                if(offers==2) return;
            }
            Expanded_UnemployedWait();
        }
        assert(!"offer did not arrive");
    }else if(menu==_expandedClubOfferMenu){
        offers++;
        unsigned before=loads;
        uint32_t base=Expanded_OfferBaseWeekly();
        assert(base>=250 && Expanded_OfferDemandWeekly()==base);
        assert(!strcmp(Expanded_OfferRegion(),"EUROPE"));
        Expanded_OfferMore();
        assert(Expanded_OfferDemandWeekly()>base);
        for(int i=0;i<100;i++)Expanded_OfferMore();
        assert(Expanded_OfferDemandWeekly()<=base*2);
        for(int i=0;i<100;i++)Expanded_OfferLess();
        assert(Expanded_OfferDemandWeekly()==base);
        Expanded_OfferViewSquad();
        Expanded_OfferViewLeague();
        assert(previews==offers*2);
        assert(loads==before); /* the critical modal reentrancy guard */
        if(offers==1) {
            for(int i=0;i<100;i++)Expanded_OfferMore();
            assert(Expanded_OfferAcceptanceChance(base,Expanded_OfferDemandWeekly(),0,0)==0);
            assert(Expanded_OfferAcceptanceChance(base,base+base/10,4,0)>
                   Expanded_OfferAcceptanceChance(base,base+base/10,0,0));
            for(unsigned attempt=1;attempt<=5;attempt++) {
                Expanded_UnemployedAccept();
                assert(Expanded_OfferRejections()==attempt);
            }
        } else Expanded_UnemployedAccept();
        assert(loads==before);
    }else if(menu==_expandedOfferPreviewMenu){
        previews++;
        Expanded_OfferPreviewDraw();
    }else assert(!"unexpected menu");
}
int main(void){
    for(unsigned i=0;i<262;i++)_countriesTable[i]=(void*)country;
    memset(_europeTable.countries,255,sizeof _europeTable.countries);
    _europeTable.countries[0]=8; _europeTable.countries[1]=13;
    _europeTable.countries[2]=14; _europeTable.countries[3]=20;
    _europeTable.countries[4]=21;
    assert(Expanded_WaitForFirstOffer(&resultTeam));
    Expanded_CommitNegotiatedSalary();
    assert(_expandedAnnualSalary==Expanded_OfferDemandWeekly()*52u);
    assert((resultTeam>>8)==8 || (resultTeam>>8)==13 || (resultTeam>>8)==14 ||
           (resultTeam>>8)==20 || (resultTeam>>8)==21);
    assert((resultTeam&255)==4); /* playable last division, never club #5 */
    assert(offers==2 && previews==4 && exits>=2);
    /* Unused buttons must disappear completely, not remain grey.  Only
     * FORWARD TIME, BACK and a READY club offer remain selectable. */
    for(unsigned i=4;i<=12;i++) {
        if(i==7 || i==9 || i==10) assert(!_menuEntries[i].isInvisible);
        else assert(_menuEntries[i].isInvisible);
    }
    assert(_menuEntries[4].isDisabled && _menuEntries[5].isDisabled &&
           _menuEntries[6].isDisabled && _menuEntries[8].isDisabled &&
           _menuEntries[11].isDisabled && _menuEntries[12].isDisabled);
    assert(!_menuEntries[7].isDisabled && !_menuEntries[9].isDisabled);
    assert(_menuEntries[7].downEntry==9 && _menuEntries[9].upEntry==7);
    assert(_menuEntries[10].isDisabled==0);
    assert(date_y==171); /* date is outside fixture text and buttons */
    printf("PASS 0.10.16: playable team %04x, region, salary +/- and cap, reject/reoffer, 4 previews\n",resultTeam);
    return 0;
}
