/* Isolated title-world selector hitbox and active-game guard test.
 * Real SDL render, platform world banks and SWOS.ini I/O need Windows. */
#include "expanded_title_layout.h"
#include <assert.h>
#include <stdio.h>
static int overlaps(ExpandedTitleRect a,ExpandedTitleRect b)
{
    return a.x<b.x+b.w && b.x<a.x+a.w &&
           a.y<b.y+b.h && b.y<a.y+a.h;
}
int main(void)
{
    assert(EXP_TITLE_MAIN_COUNT==5);
    assert(EXP_TITLE_VIDEO_INDEX==5);
    assert(EXP_TITLE_DEBUG_INDEX==6);
    assert(EXP_TITLE_WORLD_INDEX==7);
    assert(EXP_TITLE_SELECTABLE_COUNT==8);
    ExpandedTitleRect era=Expanded_TitleMainRect(EXP_TITLE_WORLD_INDEX);
    assert(era.x==1440 && era.y==365 && era.w==187 && era.h==64);
    assert(Expanded_TitleHit(era,era.x,era.y));
    assert(Expanded_TitleHit(era,era.x+era.w-1,era.y+era.h-1));
    assert(!Expanded_TitleHit(era,era.x-1,era.y));
    assert(!Expanded_TitleHit(era,era.x+era.w,era.y));
    assert(!Expanded_TitleHit(era,era.x,era.y+era.h));
    for(int i=0;i<EXP_TITLE_WORLD_INDEX;i++)
        assert(!overlaps(era,Expanded_TitleMainRect(i)));
    /* Approved Title logo occupies (920,17,620,227); no overlap. */
    assert(!overlaps(era,(ExpandedTitleRect){920,17,620,227}));
    assert(Expanded_TitleWorldSwitchAllowed(0u));
    for(unsigned gameType=1;gameType<=4;++gameType)
        assert(!Expanded_TitleWorldSwitchAllowed(gameType));
    puts("PASS: title world 96/26 hitbox, separation and no-active-contest switch");
    return 0;
}
