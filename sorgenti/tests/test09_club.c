#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "menu_system.h"
int32_t _newBalance, _gateReceipts, _competitionBonuses;
uint16_t _selTeamNumber, _currentSeasonShowing, _currentDivisionIndex;
uint8_t _expandedInfrastructureMenu[8], _expandedFacilityDetailMenu[8], _expandedClubOverviewMenu[8];
void *_charTables[1]={0};
int32_t _playerWagesBill, _generalRunningCosts, _playerSales, _playerPurchases, _old_balance;
uint8_t _seasonPlaying=96,_lastFinancialWeekIndex=5;
uint16_t _boardConfidenceScore=30;
uint8_t _expandedManagerFame=0;
uint16_t _numWins=3,_numLost=1;
static int rectangles,labels;
void FillRectangle(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t color){assert(x+w<=320 && y+h<=256 && color<=15);rectangles++;}
uint16_t DrawMenuTextCentered(uint16_t x,uint16_t y,uint16_t color,uint8_t *s,uint8_t *table){assert(x<=320 && y<256 && color<=15 && s);labels++;(void)table;return 0;}
unsigned char Expanded_CurrencyGlyph(void){return '$';}
void ShowMenu(uint8_t *m){(void)m;}
void DrawMenu(void){}
extern uint16_t _expandedEconomyClubId, _expandedEconomyLastSeason;
extern uint8_t _expandedStadiumLevel, _expandedFacilityLevels[8], _expandedFacilitySelected;
extern uint32_t _expandedShirtSponsor, _expandedStadiumSponsor, _expandedTvRights;
extern void Expanded_EconomyReset(void), Expanded_EconomyNewSeason(void);
extern uint8_t _expandedBoardMood,_expandedFanMood;
extern uint32_t _expandedSupporters;
extern void Expanded_SentimentMatchResult(int);
extern void Expanded_ClubOverviewDraw(void),Expanded_ClubOverviewIcons(void),Expanded_ClubFacilitySelected(MenuEntry *);
int main(void){
  Expanded_EconomyReset();_selTeamNumber=8;_currentDivisionIndex=2;_currentSeasonShowing=0;
  _newBalance=3000000;_gateReceipts=12345;_competitionBonuses=6789;
  Expanded_EconomyNewSeason();assert(_newBalance==3180000);
  _playerWagesBill=120000;_generalRunningCosts=24000;
  assert(_expandedBoardMood>=50 && _expandedFanMood==40);
  unsigned supporters=_expandedSupporters;
  Expanded_SentimentMatchResult(1);
  assert(_expandedFanMood>40 && _expandedSupporters>supporters);
  Expanded_SentimentMatchResult(-1);
  assert(_expandedFanMood<=47);
  Expanded_ClubOverviewDraw();Expanded_ClubOverviewIcons();
  assert(rectangles>45 && labels>20);
  MenuEntry e={0};e.ordinal=3;Expanded_ClubFacilitySelected(&e);assert(_expandedFacilitySelected==2);
  puts("PASS 0.9: 8 facility menus, native bounds, annual finances, dynamic board/fans");
  return 0;
}
