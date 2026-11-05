/* 0.10.12: Enter must COMMIT Name/Surname and consume its held fire,
 * not immediately re-open the editor on the next menu frame. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "menu_system.h"
#include "string_utils.h"

volatile uint16_t _panicReasonCode;
uint8_t _inMenuKeyboardBuffer[100];
uint16_t _currentKey;
volatile uint16_t _shortFire;
volatile uint16_t _fire;
volatile uint8_t _expandedAgeInputActive;
uint16_t _fireResetFlag;
uint16_t _deadVar02,_deadInputTextVar,_allowExtraCharsFlag;
uint8_t _inputAllowedControlKeys[]={13,27,127,126,32,255};
static int tick=0;
void MenuCheckControls(void) {
    /* First fire opens the editor, then it is released, then user types
     * one letter, and finally presses Enter to save the field. */
    if(tick==0){_currentKey=0;_shortFire=1;_fire=1;}
    else if(tick==1){_currentKey=0;_shortFire=0;_fire=0;}
    else if(tick==2){_currentKey='B';_shortFire=0;_fire=0;}
    else if(tick==3){_currentKey=13;_shortFire=1;_fire=1;}
    else assert(!"editor did not exit on Enter");
    tick++;
}
void CallDrawMenuItem(MenuEntry *e){(void)e;}
void DrawMenuItemBackgroundMasked(uint16_t x,uint16_t y,uint16_t w,uint16_t h){
    (void)x;(void)y;(void)w;(void)h;
}
void PanicHaltLoop(void){assert(!"unexpected panic");}
void InputSDL_RawText(int on){(void)on;}
int InputSDL_RawChar(void){return -1;}
uint16_t GetStringLength(const uint8_t *s){return (uint16_t)strlen((const char*)s);}
uint8_t *StringCopy(const uint8_t *src,uint8_t *dst){
    size_t n=strlen((const char*)src);memmove(dst,src,n+1);return dst+n;
}
extern void InputText_IgnoreCareerOpeningFire(void);
extern uint16_t InputText(uint16_t,uint8_t*);
int main(void){
    uint8_t name[9]="CHRIS";
    MenuEntry e={0};e.fg=(uintptr_t)name;
    e.background=NO_BACKGROUND;e.dwWidth=90;e.dwHeight=12;
    InputText_IgnoreCareerOpeningFire();
    assert(InputText(9,(uint8_t*)&e)==0);
    assert(!strcmp((char*)name,"CHRISB"));
    assert(e.fg==(uintptr_t)name);
    assert(_fireResetFlag==1);
    /* Surname uses the same editor with a 13-byte buffer. */
    tick=0;_fireResetFlag=0;
    uint8_t surname[13]="CHAPMAN";
    e.fg=(uintptr_t)surname;
    InputText_IgnoreCareerOpeningFire();
    assert(InputText(13,(uint8_t*)&e)==0);
    assert(!strcmp((char*)surname,"CHAPMANB"));
    assert(_fireResetFlag==1);
    puts("PASS 0.10.12: Name and Surname Enter commit without held-fire reopening");
    return 0;
}
