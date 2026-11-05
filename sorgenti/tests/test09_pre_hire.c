#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "menu_system.h"
#include "game_types.h"
uint8_t _expandedUnemployedMenu[8],_expandedClubOfferMenu[8];
uint8_t _teamFileBuffer[2+684*5];
uint8_t _currentMenu[sizeof(Menu)];
uint16_t _teamsLoaded,_fireResetFlag;
void *_charTables[1]={0};
void *_countriesTable[262]={0};
MenuEntry _menuEntries[8];
static int offerNumber=0,exitCalls=0,draws=0;
void FillRectangle(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t c){assert(x+w<=320&&y+h<=256&&c<=15);}
uint16_t DrawMenuTextCentered(uint16_t x,uint16_t y,uint16_t c,uint8_t *s,uint8_t *t){assert(x<=320&&y<256&&c<=15&&s);(void)t;draws++;return 0;}
void DrawMenu(void){}
void SetExitMenuFlag(void){exitCalls++;}
int16_t LoadTeamFile(uint16_t ext){
 assert(ext==8||ext==13||ext==14||ext==20||ext==21);
 *(uint16_t*)_teamFileBuffer=3;
 TeamFileEx *t=(TeamFileEx*)(_teamFileBuffer+2);
 for(int i=0;i<3;i++){
   t[i].team.countryNumber=(uint8_t)ext;
   t[i].team.teamOrdinal=(uint8_t)(i+3);
   t[i].team.league=(uint8_t)(i+1);
   memcpy(t[i].team.teamName,"LOWER LEAGUE",12);
 }
 return 0;
}
extern void Expanded_UnemployedWait(void),Expanded_UnemployedAccept(void),Expanded_UnemployedReject(void);
extern void Expanded_UnemployedShowOffer(void),Expanded_UnemployedInit(void),Expanded_UnemployedDraw(void);
extern int Expanded_WaitForFirstOffer(uint16_t *team);
void ShowMenu(uint8_t *m){
 if(m==_expandedClubOfferMenu){
   offerNumber++;
   if(offerNumber==1)Expanded_UnemployedReject();
   else Expanded_UnemployedAccept();
   return;
 }
 assert(m==_expandedUnemployedMenu);
 Expanded_UnemployedInit();
 int previous=0;
 for(int week=0;week<12;week++){
   Expanded_UnemployedDraw();
   if(!_menuEntries[5].isInvisible){
      assert(week-previous<=5);
      Expanded_UnemployedShowOffer();
      if(offerNumber==2) return;
      previous=week;
   }
   Expanded_UnemployedWait();
 }
 assert(!"second offer never arrived");
}
int main(void){
 uint16_t team=0;
 assert(Expanded_WaitForFirstOffer(&team));
 assert((team&255)==5 && (team>>8)>0);
 assert(offerNumber==2 && draws>0 && exitCalls>=2);
 puts("PASS 0.9: random <=5-week offers, reject, country+club, accept, native calendar bounds");
 return 0;
}
