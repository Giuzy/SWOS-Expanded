/* SWOS Expanded 0.10.8 - real SDL framebuffer smoke test, no SDL window needed.
 * Tests actual indexed pixels, focus border, stadium variants and upgrade cost. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "menu_system.h"
#include "display.h"
uint8_t _currentMenu[1024];
uintptr_t _currentMenuPtr=0;
uint8_t _swosMainMenu[4],_careerMenu[4],_contestMenu[4],s_unemployed[4];
uint8_t _expandedClubOfferMenu[4];
uint32_t Expanded_OfferBaseWeekly(void){return 1200;}
uint32_t Expanded_OfferDemandWeekly(void){return 1350;}
unsigned Expanded_OfferRejections(void){return 1;}
unsigned Expanded_OfferDivision(void){return 4;}
const char *Expanded_OfferRegion(void){return "EUROPE";}
const char *Expanded_OfferClub(void){return "MACCLESFIELD";}
const char *Expanded_OfferCountry(void){return "ENGLAND";}
const char *Expanded_OfferStatus(void){return "COUNTEROFFER REJECTED 1/5";}
uint8_t *Expanded_UnemployedCalendarPointer(void){return s_unemployed;}
void Expanded_UnemployedDate(char *out,size_t cap){snprintf(out,cap,"THU 28 AUG 1997");}
uint8_t _managerSexBuffer[4]="MR",_managerNameBuffer[9]="DAVID",_managerSurnameBuffer[13]="WILKIN";
char _expandedAgeText[8]="35",_expandedManagerFameText[24]="UNKNOWN";
uint16_t _coachOrPlayer=1;
MenuEntry _menuEntries[128];
void *_gameListArray[28];
static uint8_t dummyCharTable[32];
uint8_t *_charTables[1]={dummyCharTable};
static int native_text_calls=0,age_label_seen=0;
static int sawWeek=0,sawEur=0,sawLoyalty=0,sawNoOffers=0,sawCompetitions=0,sawTransfers=0;
static int sawDivision=0,sawCup=0,sawHome=0,sawAway=0,sawOfferYear=0,sawOfferWeek=0,sawOfferBase=0,sawOfferAsk=0;
static void record_text(const uint8_t *s) {
    if(!s) return;
    const char *t=(const char*)s;
    if(strstr(t,"FINANCES / WEEK 05")) sawWeek=1;
    if(strstr(t,"EUR") || strstr(t,"GBP")) sawEur=1;
    if(strstr(t,"LOYALTY") || strstr(t,"SUPPORT") || strstr(t,"MIXED")) sawLoyalty=1;
    if(strstr(t,"NO JOB OFFERS")) sawNoOffers=1;
    if(!strcmp(t,"COMPETITIONS")) sawCompetitions=1;
    if(!strcmp(t,"TRANSFERS")) sawTransfers=1;
    if(!strcmp(t,"D. THREE")) sawDivision=1;
    if(!strcmp(t,"L. CUP R2 L1")) sawCup=1;
    if(strstr(t,"SALARY OFFER:") && strstr(t,"/ YEAR")) sawOfferYear=1;
    if(strstr(t,"SALARY OFFER:") && strstr(t,"/ WEEK")) sawOfferWeek=1;
    if(strstr(t,"62,400")) sawOfferBase=1;
    if(strstr(t,"70,200")) sawOfferAsk=1;
    if(strstr(t,"DONCASTER")) sawHome=1;
    if(strstr(t,"MANSFIELD")) sawAway=1;
}

uint8_t _expandedInfrastructureMenu[8],_expandedFacilityDetailMenu[8],_expandedClubOverviewMenu[8];
int32_t _newBalance=1000000,_gateReceipts=0,_competitionBonuses=0;
int32_t _playerWagesBill=0,_generalRunningCosts=0,_playerSales=0,_playerPurchases=0,_old_balance=0;
uint16_t _selTeamNumber=3,_currentSeasonShowing=0,_currentDivisionIndex=2;
uint8_t _expandedManagerFame=0,_seasonPlaying=96,_lastFinancialWeekIndex=4;
uint16_t _boardConfidenceScore=30,_numWins=0,_numLost=0;
volatile uint16_t _youreSacked=0;
static uint8_t framebuffer[320*272+32];
volatile uintptr_t _frontbufferBasePtr=(uintptr_t)framebuffer;
int _menu_draw_x_offset=0,_screen_width=320;
static int detail_opened=0,redraws=0;
void FillRectangle(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t color) {
    assert(x+w<=320 && y+h<=272 && color<=255);
    for(unsigned row=0;row<h;row++)memset(framebuffer+(y+row)*320+x,color,w);
}
uint16_t DrawMenuTextCentered(uint16_t x,uint16_t y,uint16_t color,uint8_t *s,uint8_t *table) {
    (void)x;(void)y;(void)color;(void)table;
    native_text_calls++;
    record_text(s);
    if(s && strcmp((const char*)s,"AGE (18-70)")==0) age_label_seen=1;
    return 0;
}
/* Stand-in for the native SWOS sprite font in this headless smoke test.
 * The game itself uses GetTextWidth/DrawMenuText from text_render.c. */
uint16_t GetTextWidth(uint8_t *s,uint8_t *table) {(void)table;return (uint16_t)(strlen((char*)s)*6);}
uint16_t DrawMenuText(uint16_t x,uint16_t y,uint16_t c,uint8_t *s,uint8_t *t) {
    (void)x;(void)y;(void)c;(void)t;
    native_text_calls++;
    record_text(s);
    if(s && strcmp((const char*)s,"AGE (18-70)")==0) age_label_seen=1;
    return 0;
}
uint16_t DrawMenuTextRightAligned(uint16_t x,uint16_t y,uint16_t c,uint8_t *s,uint8_t *t) {
    (void)x;(void)y;(void)c;(void)s;(void)t;
    native_text_calls++;
    record_text(s);
    return 0;
}
unsigned char Expanded_CurrencyGlyph(void){return 0x9d;}
void ShowMenu(uint8_t *m){assert(m==_expandedFacilityDetailMenu);detail_opened++;}
void DrawMenu(void){redraws++;}
extern void Expanded_EconomyReset(void),Expanded_ClubOverviewInit(void),Expanded_ClubOverviewDraw(void);
extern void Expanded_ClubOverviewIcons(void),Expanded_ClubFacilitySelected(MenuEntry*);
extern void Expanded_UpgradeFacility(MenuEntry*);
extern void Expanded_UI_Prepare(void),Expanded_UI_Before(void),Expanded_UI_After(void);
extern void Expanded_UI_Partial(MenuEntry *);
extern void Expanded_UI_ClearFixtureArea(void);
extern int Expanded_UI_CompactFixtureRow(unsigned,unsigned);
extern uint8_t _expandedFacilityLevels[8],_expandedFacilitySelected;
extern uint8_t _expandedBoardMood,_expandedFanMood;
int main(void){
    memset(framebuffer,0,sizeof framebuffer);
    memset(framebuffer+320*272,0xA5,32);
    Expanded_EconomyReset();
    _newBalance=1000000;
    Menu *menu=(Menu*)_currentMenu;
    _menuEntries[2].ordinal=2;
    menu->selectedEntry=&_menuEntries[2];
    Expanded_ClubOverviewInit();
    assert(_menuEntries[0].isInvisible==1 && _menuEntries[9].isInvisible==1);
    assert(_menuEntries[10].background==NO_BACKGROUND);
    Expanded_ClubOverviewDraw();
    assert(sawWeek && !sawEur && !sawLoyalty);
    _expandedBoardMood=95; _expandedFanMood=95;
    Expanded_ClubOverviewDraw();
    assert(framebuffer[62*320+42]==15 && framebuffer[57*320+278]==15);
    _expandedBoardMood=10; _expandedFanMood=10;
    Expanded_ClubOverviewDraw();
    assert(framebuffer[62*320+42]==10 && framebuffer[57*320+278]==10);
    Expanded_ClubOverviewIcons();
    assert(framebuffer[133*320+49]==15); /* selected TRAINING gold frame */
    assert(framebuffer[33*320+83]!=0);   /* stadium art is present */
    assert(framebuffer[3*320+25]!=0);  /* visible BACK button */
    for(unsigned i=0;i<32;i++)assert(framebuffer[320*272+i]==0xA5);
    uint8_t first_stadium[154*88];
    for(unsigned y=0;y<88;y++)
        memcpy(first_stadium+y*154,framebuffer+(33+y)*320+83,154);
    MenuEntry e={0};e.ordinal=1;
    Expanded_ClubFacilitySelected(&e);
    assert(detail_opened==1 && _expandedFacilitySelected==0);
    Expanded_UpgradeFacility(&e);
    assert(_expandedFacilityLevels[0]==2 && _newBalance==750000 && redraws==1);
    Expanded_ClubOverviewDraw();
    int changed=0;
    for(unsigned y=0;y<88;y++)
        if(memcmp(first_stadium+y*154,framebuffer+(33+y)*320+83,154))changed++;
    assert(changed>10); /* different actual stadium pixels after upgrade */
    /* All three new screens use the actual SWOS MenuEntry selection pointer. */
    _currentMenuPtr=(uintptr_t)_swosMainMenu;
    menu->numEntries=17;
    for(unsigned i=0;i<17;i++) { _menuEntries[i].ordinal=i; _menuEntries[i].isInvisible=0; }
    menu->selectedEntry=&_menuEntries[10];
    Expanded_UI_Prepare();
    assert(_menuEntries[10].x==219 && _menuEntries[10].y==189);
    assert(_menuEntries[10].onSelect==0); /* test stub: real packed callbacks are checked separately */
    assert(_menuEntries[10].upEntry==9 && _menuEntries[10].downEntry==11);
    Expanded_UI_After();
    assert(framebuffer[189*320+219]==15); /* selected NEW CAREER */
    menu->selectedEntry=&_menuEntries[9];
    Expanded_UI_Partial(&_menuEntries[9]);
    assert(framebuffer[_menuEntries[9].y*320+219]==15);
    assert(framebuffer[189*320+219]!=15);
    menu->selectedEntry=&_menuEntries[10];
    Expanded_UI_Partial(&_menuEntries[10]);
    /* Original conditional title entries: Hall of Fame and Load Old stay
     * visible even without a saved competition; hidden slots do not leave
     * empty rows or break up/down keyboard navigation. */
    _menuEntries[11].isInvisible=1;
    _menuEntries[12].isInvisible=1;
    _menuEntries[14].isInvisible=1;
    _menuEntries[16].isInvisible=1;
    Expanded_UI_Prepare();
    assert(_menuEntries[5].isInvisible==0);  /* HALL OF FAME */
    assert(_menuEntries[13].isInvisible==0); /* LOAD OLD COMPETITION */
    assert(_menuEntries[10].downEntry==13);
    assert(_menuEntries[13].upEntry==10);
    assert(_menuEntries[13].x==219 && _menuEntries[13].y==233);
    assert(_menuEntries[13].content==NO_FOREGROUND && _menuEntries[13].onSelect==0);
    _currentMenuPtr=(uintptr_t)_careerMenu;
    menu->numEntries=18;
    _menuEntries[7].fg=(uintptr_t)"ENG";
    menu->selectedEntry=&_menuEntries[4];
    Expanded_UI_Prepare();
    assert(_menuEntries[4].x==204 && _menuEntries[4].y==138);
    assert(_menuEntries[3].downEntry==4 && _menuEntries[4].upEntry==3);
    assert(_menuEntries[4].downEntry==5 && _menuEntries[5].upEntry==4);
    assert(_menuEntries[5].downEntry==7 && _menuEntries[7].upEntry==5);
    assert(_menuEntries[11].downEntry==3 && _menuEntries[3].upEntry==11);
    /* During InputText, entry->fg points at the editing buffer; the
     * overlay must display that buffer instead of the saved old name. */
    _menuEntries[4].fg=(uintptr_t)"CHANGED";
    Expanded_UI_After();
    assert(framebuffer[138*320+204]==15); /* selected manager name */
    assert(native_text_calls>10 && age_label_seen); /* native font + AGE (18-70) */
    _menuEntries[13].isInvisible=1;
    _menuEntries[14].isInvisible=1;
    _menuEntries[15].isInvisible=1;
    _currentMenuPtr=(uintptr_t)_contestMenu;
    menu->numEntries=19;
    menu->selectedEntry=&_menuEntries[7];
    Expanded_UI_Prepare();
    assert(_menuEntries[7].x==115 && _menuEntries[7].y==195);
    /* Exercise actual 45-entry fixture template and original disabled flags. */
    menu->numEntries=45;
    for(unsigned i=19;i<45;i++)_menuEntries[i].ordinal=i;
    _menuEntries[4].isDisabled=1;
    _menuEntries[4].fg=(uintptr_t)"TRANSFERS";
    _menuEntries[5].isDisabled=1;
    _menuEntries[5].fg=(uintptr_t)"NO JOB OFFERS";
    _menuEntries[11].isInvisible=0;
    _menuEntries[11].fg=(uintptr_t)"VIEW COMPETITIONS";
    Expanded_UI_Prepare();
    assert(_menuEntries[4].isDisabled && _menuEntries[5].isDisabled);
    /* 0.10.10: the stock template geometry is deliberately NOT rewritten;
     * ordinal 37 is the highlight, 38/42 are teams, 39..41 score/V. */
    assert(_menuEntries[16].x==12 && _menuEntries[17].x==12);
    assert(_menuEntries[18].x==9 && _menuEntries[18].y==47 &&
           _menuEntries[18].dwWidth==305 && _menuEntries[18].dwHeight==133);
    assert(_menuEntries[16].content==NO_FOREGROUND);
    assert(_menuEntries[17].content==NO_FOREGROUND);
    for(unsigned i=0;i<8;i++)_gameListArray[i*2]=(void*)1;
    Expanded_UI_Before();
    assert(framebuffer[80*320+100]!=0);
    /* 8 actual matches: frame ends just below the last line, not y=183. */
    assert(framebuffer[138*320+100]==7);
    assert(framebuffer[170*320+100]!=7);
    assert(_menuEntries[17].y==125); /* readable dark fixture panel */
    Expanded_UI_After();
    _menuEntries[34].fg=(uintptr_t)"DIVISION THREE";
    _menuEntries[35].isInvisible=1;
    _menuEntries[36].fg=(uintptr_t)"(H)";
    _menuEntries[37].isInvisible=0;
    _menuEntries[38].fg=(uintptr_t)"DONCASTER ROVERS";
    _menuEntries[39].isInvisible=1;
    _menuEntries[40].fg=(uintptr_t)"V";
    _menuEntries[41].isInvisible=1;
    _menuEntries[42].fg=(uintptr_t)"MANSFIELD TOWN";
    Expanded_UI_ClearFixtureArea();
    assert(Expanded_UI_CompactFixtureRow(0,0));
    assert(sawDivision && sawHome && sawAway);
    assert(framebuffer[53*320+305]==11); /* bounded blue highlight */
    assert(framebuffer[53*320+315]!=11); /* no overflow past panel */
    _menuEntries[34].fg=(uintptr_t)"LEAGUE CUP";
    _menuEntries[35].fg=(uintptr_t)"ROUND 2 LEG 1";
    _menuEntries[35].isInvisible=0;
    assert(Expanded_UI_CompactFixtureRow(1,1));
    assert(sawCup); /* competition and round are on ONE line */
    for(unsigned i=8;i<14;i++)_gameListArray[i*2]=(void*)1;
    Expanded_UI_ClearFixtureArea();
    assert(framebuffer[189*320+100]==7); /* 14 matches: full-height frame */
    assert(_menuEntries[17].y==176);
    for(unsigned i=0;i<14;i++)assert(Expanded_UI_CompactFixtureRow(i,0));
    Expanded_UI_After();
    assert(framebuffer[189*320+100]==7); /* 14 painted rows remain visible */
    for(unsigned i=8;i<14;i++)_gameListArray[i*2]=NULL;
    Expanded_UI_ClearFixtureArea();
    for(unsigned i=0;i<8;i++)assert(Expanded_UI_CompactFixtureRow(i,0));
    Expanded_UI_After();
    assert(framebuffer[138*320+100]==7);
    assert(framebuffer[170*320+100]!=7); /* scroll back: no ghost panel */
    /* Even when a cache claims 14, a decoder that draws only 5 rows
     * must not leave a big blank rectangle beneath them. */
    for(unsigned i=8;i<14;i++)_gameListArray[i*2]=(void*)1;
    Expanded_UI_ClearFixtureArea();
    for(unsigned i=0;i<5;i++)assert(Expanded_UI_CompactFixtureRow(i,0));
    Expanded_UI_After();
    assert(_menuEntries[17].y==98); /* 5 rows -> bottom 112, arrow at 98 */
    assert(framebuffer[111*320+100]==7);
    assert(framebuffer[170*320+100]!=7);

    assert(sawNoOffers && sawCompetitions && sawTransfers);
    assert(framebuffer[195*320+115]==15); /* selected PLAY MATCH */
    framebuffer[100*320+80]=137; /* fixture area is owned by native SWOS */
    Expanded_UI_Partial(&_menuEntries[7]);
    assert(framebuffer[100*320+80]==137); /* no fixture list erased */
    /* 0.10.14: the offer screen uses live menu entries and a clean full-width
     * background, including a selectable salary negotiation row. */
    _currentMenuPtr=(uintptr_t)_expandedClubOfferMenu;
    menu->numEntries=8;
    for(unsigned i=0;i<8;i++) {
        _menuEntries[i].ordinal=i;
        _menuEntries[i].isInvisible=0;
        _menuEntries[i].isDisabled=0;
    }
    const int ox[]={65,22,166,22,166,104,43,251};
    const int oy[]={0,199,199,218,218,237,179,179};
    const int ow[]={190,132,132,132,132,112,26,26};
    const int oh[]={15,16,16,16,16,15,17,17};
    for(unsigned i=0;i<8;i++) {
        _menuEntries[i].x=ox[i];_menuEntries[i].y=oy[i];
        _menuEntries[i].dwWidth=ow[i];_menuEntries[i].dwHeight=oh[i];
    }
    menu->selectedEntry=&_menuEntries[3];
    Expanded_UI_Prepare();
    Expanded_UI_Before(); Expanded_UI_After();
    assert(_menuEntries[1].x==42 && _menuEntries[1].y==191);
    assert(_menuEntries[2].x==162 && _menuEntries[2].y==191);
    assert(_menuEntries[3].x==42 && _menuEntries[3].y==212);
    assert(_menuEntries[4].x==162 && _menuEntries[4].y==212);
    assert(_menuEntries[5].x==111 && _menuEntries[5].y==233);
    for(unsigned i=1;i<8;i++)
        assert(_menuEntries[i].x>=0 && _menuEntries[i].y>=0 &&
               _menuEntries[i].x+_menuEntries[i].dwWidth<=320 &&
               _menuEntries[i].y+_menuEntries[i].dwHeight<=256);
    assert(framebuffer[212*320+42]==15); /* green ACCEPT has live focus */
    assert(framebuffer[166*320+92]!=0); /* salary minus is visible */
    assert(framebuffer[166*320+210]!=0); /* salary plus is visible */
    assert(sawOfferYear && !sawOfferWeek && sawOfferBase && sawOfferAsk); /* annual 1200*52 / 1350*52 */
    for(unsigned i=0;i<32;i++)assert(framebuffer[320*272+i]==0xA5);
    puts("PASS 0.10.15: first-concept offer UI, annual wages, 7 live buttons, full scroll clipping, guards");
    return 0;
}
