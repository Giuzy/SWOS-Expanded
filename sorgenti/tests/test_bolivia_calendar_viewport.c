/* Original OG Copa/Liga visual calendar viewport regression (isolated).
 * This tests only presentation arithmetic, not match simulation or Windows. */
#include <assert.h>
#include <stdio.h>
#include "expanded_bolivia_calendar_viewport.h"

int main(void)
{
    ExpandedBoliviaCalendarViewport v={0};
    /* BEFORE: renderer always showed index 0..13, even if the next OG
     * fixture was index 17; AFTER: row 17 is visibly centered at slot7. */
    ExpandedBoliviaCalendar_Update(&v,38u,17,0x1234u);
    assert(v.page==10 && 17-v.page==BOLIVIA_CALENDAR_CENTER_ROW);
    assert(ExpandedBoliviaCalendar_Scroll(&v,1));
    assert(v.page==11 && v.offset==1);
    ExpandedBoliviaCalendar_Update(&v,38u,17,0x1234u);
    assert(v.page==11 && v.offset==1); /* redraw must retain arrow scroll */
    assert(ExpandedBoliviaCalendar_Scroll(&v,-1));
    assert(v.page==10);
    /* New actionable fixture recentres even when already in first14. */
    ExpandedBoliviaCalendar_Update(&v,38u,3,0x9876u);
    assert(v.page==-4 && 3-v.page==BOLIVIA_CALENDAR_CENTER_ROW);
    assert(v.offset==0);
    ExpandedBoliviaCalendar_Update(&v,38u,0,0xabceu);
    assert(v.page==-7 && 0-v.page==BOLIVIA_CALENDAR_CENTER_ROW);
    assert(!ExpandedBoliviaCalendar_Scroll(&v,-1)); /* first bound */
    assert(ExpandedBoliviaCalendar_Scroll(&v,1));
    assert(v.page==-6);
    ExpandedBoliviaCalendar_Update(&v,38u,37,0xabcdu);
    assert(v.page==30 && 37-v.page==BOLIVIA_CALENDAR_CENTER_ROW);
    assert(!ExpandedBoliviaCalendar_Scroll(&v,1)); /* last bound */
    assert(ExpandedBoliviaCalendar_Scroll(&v,-1));
    assert(v.page==29);
    /* No selectable OG fixture: browsing may reveal the entire page,
     * but must never manufacture a blue match highlight. */
    ExpandedBoliviaCalendar_Update(&v,38u,-1,0);
    assert(v.page==0 && v.focusRow==-1);
    for(unsigned i=0;i<25;i++)(void)ExpandedBoliviaCalendar_Scroll(&v,1);
    assert(v.page==24);
    assert(!ExpandedBoliviaCalendar_Scroll(&v,1));
    ExpandedBoliviaCalendar_Update(&v,38u,-1,0);
    assert(v.page==24); /* preserve manual scroll without a fixture */
    /* Empty/short pages and corrupted focus index never dereference rows. */
    ExpandedBoliviaCalendar_Update(&v,0u,19,123);
    assert(v.total==0 && v.page==0 && v.focusRow==-1);
    assert(!ExpandedBoliviaCalendar_Scroll(&v,1));
    ExpandedBoliviaCalendar_Update(&v,5u,2,124);
    assert(v.page==-5 && 2-v.page==BOLIVIA_CALENDAR_CENTER_ROW);
    ExpandedBoliviaCalendar_Update(&v,5u,99,125);
    assert(v.focusRow==-1 && v.page==0);
    assert(!ExpandedBoliviaCalendar_Scroll(&v,0));
    puts("RG-014 CALENDAR_VIEWPORT_ISO PASS: center, arrows, bounds, reset");
    puts("WINDOWS_MOUSE_FOCUS_OG_CAR: NOT_TESTED");
    return 0;
}
