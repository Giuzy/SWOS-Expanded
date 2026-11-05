/* Headless 320x256 smoke test: real indexed assets, modal geometry and focus. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "menu_system.h"
#include "display.h"
uint8_t _currentMenu[2048];
MenuEntry _menuEntries[128];
uintptr_t _currentMenuPtr=0;
uint8_t _swosMainMenu[4],_careerMenu[4],_contestMenu[4],_expandedClubOfferMenu[4];
uint8_t _chairmanMemosMenu[4],_expandedSackedMenu[4];
uint8_t s_unemployed[4];
uint8_t *Expanded_UnemployedCalendarPointer(void){return s_unemployed;}
uint8_t _managerSexBuffer[4]="MR",_managerNameBuffer[8]="CHRIS",_managerSurnameBuffer[16]="CHAPMAN";
char _expandedAgeText[8]="35",_expandedManagerFameText[24]="UNKNOWN";
uint16_t _coachOrPlayer=1;
uint8_t _expandedManagerFame=0;
uint8_t _lastFinancialWeekIndex=3;
uint16_t _currentSeasonShowing=0;
volatile uint16_t _currentMessage=19;
uint8_t _expandedInfrastructureMenu[4],_expandedFacilityDetailMenu[4];
static uint8_t font[32]={0};
void *_charTables[1]={font};
static uint8_t framebuffer[320*272+32];
volatile uintptr_t _frontbufferBasePtr=(uintptr_t)framebuffer;
int _menu_draw_x_offset=0,_screen_width=320;
int32_t _newBalance=200000,_gateReceipts=0,_competitionBonuses=0;
uint16_t _selTeamNumber=1,_currentDivisionIndex=3;
volatile uint16_t _youreSacked=0;
void FillRectangle(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t color){
    assert(x+w<=320 && y+h<=272 && color<=255);
    for(unsigned row=0;row<h;row++)memset(framebuffer+(y+row)*320+x,color,w);
}
uint16_t GetTextWidth(uint8_t *s,uint8_t *t){(void)t;return (uint16_t)strlen((char*)s)*6;}
uint16_t DrawMenuTextCentered(uint16_t x,uint16_t y,uint16_t c,uint8_t *s,uint8_t *t){
    (void)x;(void)y;(void)c;(void)s;(void)t;return 0;
}
uint16_t DrawMenuText(uint16_t x,uint16_t y,uint16_t c,uint8_t *s,uint8_t *t){
    (void)x;(void)y;(void)c;(void)s;(void)t;return 0;
}
uint16_t DrawMenuTextRightAligned(uint16_t x,uint16_t y,uint16_t c,uint8_t *s,uint8_t *t){
    (void)x;(void)y;(void)c;(void)s;(void)t;return 0;
}
unsigned char Expanded_CurrencyGlyph(void){return '$';}
const char *Expanded_CurrentClubName(void){return "NOTTS COUNTY";}
uint32_t Expanded_OfferBaseWeekly(void){return 500;}
uint32_t Expanded_OfferDemandWeekly(void){return 550;}
unsigned Expanded_OfferDivision(void){return 4;}
const char *Expanded_OfferRegion(void){return "EUROPE";}
const char *Expanded_OfferClub(void){return "NOTTS COUNTY";}
const char *Expanded_OfferCountry(void){return "ENGLAND";}
const char *Expanded_OfferStatus(void){return "";}
void Expanded_UnemployedDate(char *out,size_t n){snprintf(out,n,"THU 28 AUG 1997");}
void *_gameListArray[40]={0};
void ShowMenu(uint8_t *menu){(void)menu;}
void DrawMenu(void){}

/* Standalone headless mock data for other menus reachable through UI_After. */
#include "game_types.h"
uint8_t _managementRecordMenu[4],_squadMenu[4];
uint8_t _seasonInformationsArray[20*sizeof(SeasonInformations)];
uint16_t _currentSeasonNumber=0;
uint32_t _expandedAnnualSalary=10000,_expandedCareerEarnings=0;
static TeamFileEx mockSquad={0};
uintptr_t _squadTeamFilePtr=0;
uint8_t _shortCountryNames[300]={0};
void *_countriesTable[100]={0};
const char *Expanded_RegionForCountry(uint8_t c){(void)c;return "EUROPE";}
int Expanded_UnemployedRecordIsEmpty(void){return 1;}
void Expanded_SquadClearRowCache(void){}
const PlayerFile *Expanded_SquadRowPlayer(unsigned r){return r==0?&mockSquad.players[0]:NULL;}
void FormatPlayerValueOrStatusText(uint8_t *base,uint8_t *dst){(void)base;dst[0]=0;}

int main(void){
    memset(framebuffer,0,sizeof framebuffer);
    memset(framebuffer+320*272,0xA5,32);
    Menu *m=(Menu*)_currentMenu;
    m->numEntries=7;m->selectedEntry=&_menuEntries[6];
    _menuEntries[6].isInvisible=0;
    _currentMenuPtr=(uintptr_t)_chairmanMemosMenu;
    extern void Expanded_UI_Prepare(void),Expanded_UI_Before(void),Expanded_UI_After(void);
    extern uint8_t _expandedBoardMood,_expandedFanMood;
    _expandedBoardMood=72;_expandedFanMood=20;
    Expanded_UI_Prepare();Expanded_UI_Before();Expanded_UI_After();
    assert(_menuEntries[6].x==93 && _menuEntries[6].y==229);
    assert(framebuffer[229*320+93]==15); /* visible focused Continue */
    assert(framebuffer[50*320+150]!=0); /* welcome art */
    _currentMenuPtr=(uintptr_t)_expandedSackedMenu;
    m->numEntries=1;m->selectedEntry=&_menuEntries[0];
    _menuEntries[0].isInvisible=0;
    Expanded_UI_Prepare();Expanded_UI_Before();Expanded_UI_After();
    assert(_menuEntries[0].x==63 && _menuEntries[0].y==222);
    assert(framebuffer[222*320+63]==15); /* focused Return Unemployed */
    assert(framebuffer[80*320+160]!=0); /* sacked illustration */
    /* Squad hover is driven by the same ordinal the joypad selects. */
    strcpy((char*)mockSquad.team.teamName,"TEST CLUB");
    strcpy((char*)mockSquad.players[0].playerName,"IAN HENDON");
    mockSquad.players[0].positionAndFace=1u<<5;
    mockSquad.players[0].passing=2;
    mockSquad.players[0].tacklingControl=(15u<<4);
    mockSquad.players[0].speedFinishing=(12u<<4);
    mockSquad.players[0].shirtNumber=2;
    _squadTeamFilePtr=(uintptr_t)&mockSquad;
    m->numEntries=64;
    m->selectedEntry=&_menuEntries[28];
    _menuEntries[28].isInvisible=0;
    _menuEntries[28].ordinal=28;
    _currentMenuPtr=(uintptr_t)_squadMenu;
    Expanded_UI_Prepare();Expanded_UI_Before();Expanded_UI_After();
    assert(framebuffer[90*320+157]==15); /* first yellow star */
    assert(framebuffer[113*320+174]==204); /* tooltip frame */
    for(unsigned i=0;i<32;i++) assert(framebuffer[320*272+i]==0xA5);
    puts("PASS: welcome, full-screen sacked, live squad 4-star hover, joypad focus, no overflow");
    return 0;
}
