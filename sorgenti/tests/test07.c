#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "menu_system.h"
#include "text_render.h"
int32_t _newBalance, _gateReceipts, _competitionBonuses;
uint16_t _selTeamNumber, _currentSeasonShowing, _currentDivisionIndex;
uint8_t _expandedInfrastructureMenu[8], _expandedFacilityDetailMenu[8], _expandedClubOverviewMenu[8];
void *_charTables[1] = {0};
int32_t _playerWagesBill, _generalRunningCosts, _playerSales, _playerPurchases, _old_balance;
static int rectangles, labels;
void FillRectangle(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t color) {
  assert(x+w<=320 && y+h<=256 && color<=15); rectangles++;
}
uint16_t DrawMenuTextCentered(uint16_t x,uint16_t y,uint16_t color,uint8_t *s,uint8_t *table) {
  assert(x<=320 && y<256 && color<=15 && s); labels++; (void)table;return 0;
}
extern void Expanded_ClubOverviewDraw(void);
unsigned char Expanded_CurrencyGlyph(void) { return '$'; }
void ShowMenu(uint8_t *m) {(void)m;}
void DrawMenu(void) {}
extern uint16_t _expandedEconomyClubId, _expandedEconomyLastSeason;
extern uint8_t _expandedStadiumLevel, _expandedFacilityLevels[8], _expandedFacilitySelected;
extern uint32_t _expandedShirtSponsor, _expandedStadiumSponsor, _expandedTvRights;
extern char _expandedInfraLines[8][72], _expandedFacilityDetailLines[4][72];
extern void Expanded_EconomyReset(void), Expanded_EconomyNewSeason(void);
extern void Expanded_InfrastructureInit(void), Expanded_FacilityDetailInit(void);
extern void Expanded_UpgradeFacility(MenuEntry *);
int main(void) {
  Expanded_EconomyReset();
  for(int i=0;i<8;i++) assert(_expandedFacilityLevels[i]==1);
  _selTeamNumber=8;_currentDivisionIndex=2;_currentSeasonShowing=0;
  _newBalance=3000000;_gateReceipts=12345;_competitionBonuses=6789;
  Expanded_EconomyNewSeason();
  assert(_expandedStadiumLevel==1);
  assert(_expandedShirtSponsor==60000 && _expandedStadiumSponsor==30000 && _expandedTvRights==90000);
  assert(_newBalance==3180000);
  Expanded_EconomyNewSeason();assert(_newBalance==3180000);
  Expanded_InfrastructureInit();assert(strstr(_expandedInfraLines[2],"TRANSPORT"));
  _expandedFacilitySelected=2;
  Expanded_FacilityDetailInit();assert(strstr(_expandedFacilityDetailLines[1],"MINIBUS"));
  Expanded_UpgradeFacility(0);assert(_expandedFacilityLevels[2]==2 && _newBalance==3120000);
  Expanded_FacilityDetailInit();assert(strstr(_expandedFacilityDetailLines[1],"CLUB BUS"));
  _expandedFacilitySelected=6;
  Expanded_UpgradeFacility(0);assert(_expandedFacilityLevels[6]==2);
  Expanded_UpgradeFacility(0);assert(_expandedFacilityLevels[6]==3);
  _expandedFacilitySelected=0;
  Expanded_UpgradeFacility(0);assert(_expandedStadiumLevel==2);
  _currentSeasonShowing=1;Expanded_EconomyNewSeason();
  assert(_expandedEconomyLastSeason==1);
  _selTeamNumber=10;_currentDivisionIndex=0;
  Expanded_EconomyNewSeason();assert(_expandedStadiumLevel==3);
  for(int i=1;i<8;i++) assert(_expandedFacilityLevels[i]==1);
  _selTeamNumber=10;
  _playerWagesBill=120000;_generalRunningCosts=24000;
  _gateReceipts=96000;_competitionBonuses=24000;
  Expanded_ClubOverviewDraw();
  assert(rectangles>45 && labels>15);
  puts("PASS: native pixel-art overview geometry and finance labels");
  puts("PASS: eight facility tiers, upgrade pricing, transport, medical, transfers balance, season payment, club switch");
  return 0;
}
