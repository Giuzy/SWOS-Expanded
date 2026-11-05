/* SWOS Expanded 0.10.5 - real SDL framebuffer smoke test, no SDL window needed.
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
uint8_t *Expanded_UnemployedCalendarPointer(void){return s_unemployed;}
void Expanded_UnemployedDate(char *out,size_t cap){snprintf(out,cap,"THU 28 AUG 1997");}
uint8_t _managerSexBuffer[4]="MR",_managerNameBuffer[9]="DAVID",_managerSurnameBuffer[13]="WILKIN";
char _expandedAgeText[8]="35",_expandedManagerFameText[24]="UNKNOWN";
uint16_t _coachOrPlayer=1;
MenuEntry _menuEntries[128];
static uint8_t dummyCharTable[32];
uint8_t *_charTables[1]={dummyCharTable};
static int native_text_calls=0,age_label_seen=0;
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
    if(s && strcmp((const char*)s,"AGE (18-70)")==0) age_label_seen=1;
    return 0;
}
/* Stand-in for the native SWOS sprite font in this headless smoke test.
 * The game itself uses GetTextWidth/DrawMenuText from text_render.c. */
uint16_t GetTextWidth(uint8_t *s,uint8_t *table) {(void)table;return (uint16_t)(strlen((char*)s)*6);}
uint16_t DrawMenuText(uint16_t x,uint16_t y,uint16_t c,uint8_t *s,uint8_t *t) {
    (void)x;(void)y;(void)c;(void)t;
    native_text_calls++;
    if(s && strcmp((const char*)s,"AGE (18-70)")==0) age_label_seen=1;
    return 0;
}
uint16_t DrawMenuTextRightAligned(uint16_t x,uint16_t y,uint16_t c,uint8_t *s,uint8_t *t) {
    (void)x;(void)y;(void)c;(void)s;(void)t;
    native_text_calls++;
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
extern uint8_t _expandedFacilityLevels[8],_expandedFacilitySelected;
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
    assert(_menuEntries[10].x==233 && _menuEntries[10].y==197);
    assert(_menuEntries[10].onSelect==0); /* test stub: real packed callbacks are checked separately */
    assert(_menuEntries[10].upEntry==9 && _menuEntries[10].downEntry==11);
    Expanded_UI_After();
    assert(framebuffer[197*320+233]==15); /* selected NEW CAREER */
    menu->selectedEntry=&_menuEntries[9];
    Expanded_UI_Partial(&_menuEntries[9]);
    assert(framebuffer[_menuEntries[9].y*320+233]==15);
    assert(framebuffer[197*320+233]!=15);
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
    assert(_menuEntries[13].x==233 && _menuEntries[13].y==237);
    assert(_menuEntries[13].content==NO_FOREGROUND && _menuEntries[13].onSelect==0);
    _currentMenuPtr=(uintptr_t)_careerMenu;
    menu->numEntries=18;
    _menuEntries[7].fg=(uintptr_t)"ENG";
    menu->selectedEntry=&_menuEntries[4];
    Expanded_UI_Prepare();
    assert(_menuEntries[4].x==201 && _menuEntries[4].y==129);
    Expanded_UI_After();
    assert(framebuffer[129*320+201]==15); /* selected manager name */
    assert(native_text_calls>10 && age_label_seen); /* native font + AGE (18-70) */
    _menuEntries[13].isInvisible=1;
    _menuEntries[14].isInvisible=1;
    _menuEntries[15].isInvisible=1;
    _currentMenuPtr=(uintptr_t)_contestMenu;
    menu->numEntries=19;
    menu->selectedEntry=&_menuEntries[7];
    Expanded_UI_Prepare();
    assert(_menuEntries[7].x==115 && _menuEntries[7].y==195);
    Expanded_UI_Before();
    assert(framebuffer[80*320+100]!=0); /* readable dark fixture panel */
    Expanded_UI_After();
    assert(framebuffer[195*320+115]==15); /* selected PLAY MATCH */
    framebuffer[100*320+80]=137; /* fixture area is owned by native SWOS */
    Expanded_UI_Partial(&_menuEntries[7]);
    assert(framebuffer[100*320+80]==137); /* no fixture list erased */
    for(unsigned i=0;i<32;i++)assert(framebuffer[320*272+i]==0xA5);
    puts("PASS 0.10.7: compact native-font menu hitboxes, original business and stadium upgrade, guard bytes");
    return 0;
}
