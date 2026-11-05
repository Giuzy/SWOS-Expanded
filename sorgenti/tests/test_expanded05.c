#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "menu_system.h"
int32_t _newBalance, _gateReceipts, _competitionBonuses;
uint16_t _selTeamNumber, _currentSeasonShowing, _currentDivisionIndex;
uint8_t _expandedInfrastructureMenu[8];
unsigned char Expanded_CurrencyGlyph(void) { return '$'; }
void ShowMenu(uint8_t *m) {(void)m;}
void DrawMenu(void) {}
extern uint16_t _expandedEconomyClubId, _expandedEconomyLastSeason;
extern uint8_t _expandedStadiumLevel;
extern uint32_t _expandedShirtSponsor, _expandedStadiumSponsor, _expandedTvRights;
extern char _expandedInfraLines[8][72];
extern void Expanded_EconomyReset(void);
extern void Expanded_EconomyNewSeason(void);
extern void Expanded_InfrastructureInit(void);
extern void Expanded_UpgradeStadium(MenuEntry *);
int main(void) {
    Expanded_EconomyReset();
    _selTeamNumber=8;_currentDivisionIndex=2;_currentSeasonShowing=0;
    _newBalance=2000000;_gateReceipts=12345;_competitionBonuses=6789;
    Expanded_EconomyNewSeason();
    assert(_expandedStadiumLevel==1);
    assert(_expandedShirtSponsor==50000 && _expandedStadiumSponsor==30000 && _expandedTvRights==75000);
    assert(_newBalance==2155000);
    Expanded_EconomyNewSeason();assert(_newBalance==2155000); /* no double payment */
    Expanded_InfrastructureInit();assert(_expandedInfraLines[5][0]=='M');
    Expanded_UpgradeStadium(0);assert(_expandedStadiumLevel==2 && _newBalance==1905000);
    Expanded_UpgradeStadium(0);assert(_expandedStadiumLevel==3 && _newBalance==905000);
    Expanded_UpgradeStadium(0);assert(_expandedStadiumLevel==3 && _newBalance==905000); /* cannot afford */
    _currentSeasonShowing=1;Expanded_EconomyNewSeason();
    assert(_newBalance==905000+9*155000);
    _selTeamNumber=10;_currentDivisionIndex=0;
    Expanded_EconomyNewSeason();assert(_expandedStadiumLevel==3);
    assert(_expandedEconomyClubId==10 && _expandedEconomyLastSeason==1);
    puts("PASS: no double payment, transfers share balance, upgrade costs, new club and seasonal revenue");
    return 0;
}
