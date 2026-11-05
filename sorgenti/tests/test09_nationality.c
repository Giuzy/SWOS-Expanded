#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "menu_system.h"
extern uint8_t _expandedNationalityPickerMenu[];
extern uint8_t _expandedNationalityPage,_expandedNationalityAction;
extern int _expandedNationalityAbsoluteChoice;
extern void Expanded_NationalityPickerPopulate(const uint8_t*,int);
extern int Expanded_NationalityRowFromEntry(const MenuEntry*);
extern uint8_t Expanded_NationalityCodeAtRow(int);
extern const char *Expanded_NationalityLabelAtRow(int);
extern void Expanded_NationalitySelect(MenuEntry*);
void *_countriesTable[262];
static int exits=0;
uint16_t _fireResetFlag=0;
uint8_t _nationalityOrdinalsArray[64]={0};
MenuEntry _menuEntries[20];
uint8_t _currentMenu[sizeof(Menu)];
int World_Current(void){return 0;}
void DrawMenu(void){}
void SetExitMenuFlag(void){exits++;}
int main(void){
 static const uint8_t nats[68]={
    8,31,36,24,46,0,1,2,38,3,4,5,7,40,9,10,11,12,13,14,23,
    15,16,17,18,19,20,21,22,25,26,27,28,29,32,30,45,39,33,6,34,35,
    56,52,68,69,70,71,72,73,74,79,75,76,77,80,88,81,120,118,119,148,149,
    153,154,155,37,255
 };
 static const char name[]="\0COUNTRY";
 for(int i=0;i<262;i++)_countriesTable[i]=(void*)name;
 for(int page=0;page<7;page++){
  _expandedNationalityPage=(uint8_t)page;
  Expanded_NationalityPickerPopulate(nats,68);
  for(int row=0;row<10 && page*10+row<68;row++){
   /* Packed MenuHeader 3 pointers + u32; marker=2; title=22;
    * row stride=36; row string pointer at offset 18. */
   size_t pos=3*sizeof(void*)+4+2+22+row*(4+6+4+12+10+2)+18;
   void *label=0;
   memcpy(&label,_expandedNationalityPickerMenu+pos,sizeof label);
   MenuEntry e={0};e.ordinal=(uint16_t)(row+1);e.fg=(uintptr_t)label;
   assert(Expanded_NationalityRowFromEntry(&e)==row);
   assert(Expanded_NationalityCodeAtRow(row)==nats[page*10+row]);
   assert(Expanded_NationalityLabelAtRow(row)[0]);
   assert(strncmp(Expanded_NationalityLabelAtRow(row),"COUNTRY ",8)!=0);
   if(nats[page*10+row]==9) assert(!strcmp(Expanded_NationalityLabelAtRow(row),"ESTONIA"));
   if(nats[page*10+row]==46) assert(!strcmp(Expanded_NationalityLabelAtRow(row),"REPUBLIC OF IRELAND"));
   if(nats[page*10+row]==70) assert(!strcmp(Expanded_NationalityLabelAtRow(row),"BRAZIL"));
   if(nats[page*10+row]==148) assert(!strcmp(Expanded_NationalityLabelAtRow(row),"AUSTRALIA"));
   if(nats[page*10+row]==255) assert(!strcmp(Expanded_NationalityLabelAtRow(row),"OTHER"));
   _expandedNationalityAbsoluteChoice=-1;
   Expanded_NationalitySelect(&e);
   assert(_expandedNationalityAbsoluteChoice==page*10+row);
  }
 }
 assert(exits==68);
 puts("PASS 0.10.2: all 68 nationality choices preserve displayed label, country code and row");
}
