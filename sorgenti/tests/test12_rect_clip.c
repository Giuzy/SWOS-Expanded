/* 0.10.2 regression: offscreen menu rectangles must never underflow h. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "display.h"
#include "blitter.h"
int _screen_width=SCREEN_WIDTH;
int _menu_draw_x_offset=0;
static uint8_t screen[SCREEN_WIDTH * SCREEN_HEIGHT];
volatile uintptr_t _frontbufferBasePtr=(uintptr_t)screen;
int main(void) {
    memset(screen,0,sizeof screen);
    FillRectangle(134,282,100,1,5); /* old BACK bottom frame: 282>272 */
    for(size_t i=0;i<sizeof screen;i++) assert(screen[i]==0);
    FillRectangle(134,267,100,16,5); /* straddles bottom: bounded wrap */
    size_t n=0;
    for(size_t i=0;i<sizeof screen;i++) if(screen[i]) n++;
    assert(n==1600);
    memset(screen,0,sizeof screen);
    FillRectangle(320,20,100,10,7); /* clip at right edge */
    n=0;
    for(size_t i=0;i<sizeof screen;i++) if(screen[i]) n++;
    assert(n==160);
    puts("PASS 0.10.2: offscreen rectangles clipped, no grey-screen overflow");
    return 0;
}
