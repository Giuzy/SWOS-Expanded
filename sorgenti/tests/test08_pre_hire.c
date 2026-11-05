#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "menu_system.h"
#include "game_types.h"
uint8_t _expandedUnemployedMenu[8];
uint8_t _teamFileBuffer[2+684*5];
uint16_t _teamsLoaded;
void *_charTables[1]={0};
MenuEntry _menuEntries[8];
static int render,exitCalls;
void FillRectangle(uint16_t x,uint16_t y,uint16_t w,uint16_t h,uint16_t color){assert(x+w<=320 && y+h<=256 && color<=15);}
uint16_t DrawMenuTextCentered(uint16_t x,uint16_t y,uint16_t color,uint8_t *s,uint8_t *table){assert(x<=320 && y<256 && s);render++;(void)color;(void)table;return 0;}
void DrawMenu(void){}
void SetExitMenuFlag(void){exitCalls++;}
int16_t LoadTeamFile(uint16_t ext){
  assert(ext==8);
  *(uint16_t*)_teamFileBuffer=3;
  TeamFileEx *t=(TeamFileEx*)(_teamFileBuffer+2);
  for(int i=0;i<3;i++){t[i].team.countryNumber=8;t[i].team.teamOrdinal=(uint8_t)(i+3);t[i].team.league=(uint8_t)i;}
  memcpy(t[2].team.teamName,"TINY TOWN",10);
  return 0;
}
extern void Expanded_UnemployedWait(void),Expanded_UnemployedAccept(void),Expanded_UnemployedDraw(void);
void ShowMenu(uint8_t *m){assert(m==_expandedUnemployedMenu);for(int i=0;i<4;i++)Expanded_UnemployedWait();Expanded_UnemployedDraw();Expanded_UnemployedAccept();}
extern int Expanded_WaitForFirstOffer(uint16_t *team);
int main(void){uint16_t team=0;assert(Expanded_WaitForFirstOffer(&team));assert(team==0x805);assert(render>0 && exitCalls==1);puts("PASS 0.8: 4-week unemployment, real weak club ID, offer accepted");return 0;}
