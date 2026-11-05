#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "menu_system.h"
int32_t _newBalance, _gateReceipts, _competitionBonuses;
uint16_t _selTeamNumber, _currentSeasonShowing, _currentDivisionIndex;
uint8_t _expandedInfrastructureMenu[8], _expandedFacilityDetailMenu[8];
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
  puts("PASS: eight facility tiers, upgrade pricing, transport, medical, transfers balance, season payment, club switch");
  return 0;
}
