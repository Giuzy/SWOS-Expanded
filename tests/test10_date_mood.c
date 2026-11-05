#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "menu_system.h"
uint16_t _selTeamNumber=8,_currentSeasonShowing=0,_currentDivisionIndex=2;
uint8_t _expandedManagerFame=0;
int32_t _newBalance=200000,_gateReceipts=0,_competitionBonuses=0;
volatile uint16_t _youreSacked=0;
extern uint8_t _expandedBoardMood,_expandedFanMood,_expandedLowConfidenceMatches;
extern void Expanded_EconomyReset(void),Expanded_SentimentMatchResult(int);
extern void Expanded_FormatDate(char *,size_t,unsigned,unsigned);
int main(void) {
 char date[60];
 Expanded_FormatDate(date,sizeof date,1996,0);
 assert(!strcmp(date,"THU 01 AUG 1996"));
 Expanded_FormatDate(date,sizeof date,1996,4);
 assert(!strcmp(date,"THU 29 AUG 1996"));
 Expanded_FormatDate(date,sizeof date,1999,30);
 assert(strstr(date,"2000")!=NULL);
 Expanded_EconomyReset();
 for(int i=0;i<30 && !_youreSacked;i++) {
    _newBalance=-100000;
    Expanded_SentimentMatchResult(-1);
 }
 assert(_youreSacked==1);
 assert(_expandedBoardMood<=12 && _expandedFanMood<=35);
 assert(_expandedLowConfidenceMatches>=3);
 printf("PASS 0.10: Gregorian dates and dual-confidence dismissal at board %u / fans %u\n",
   (unsigned)_expandedBoardMood,(unsigned)_expandedFanMood);
 return 0;
}
