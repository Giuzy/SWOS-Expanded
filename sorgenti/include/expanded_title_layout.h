/* SWOS Expanded: PNG title buttons use these same logical hitboxes in SDL
 * painting and in input handling. No changes to the SWOS match coordinate space. */
#ifndef EXPANDED_TITLE_LAYOUT_H
#define EXPANDED_TITLE_LAYOUT_H
enum {
    EXP_TITLE_W=1664, EXP_TITLE_H=936,
    EXP_TITLE_MAIN_COUNT=5,      /* approved large/small text PNGs */
    EXP_TITLE_VIDEO_INDEX=5,     /* top-left independent icon */
    EXP_TITLE_DEBUG_INDEX=6,     /* debug start new career, small control */
    EXP_TITLE_WORLD_INDEX=7,     /* same 96/26 debug selector as NEW CAREER */
    EXP_TITLE_SELECTABLE_COUNT=8
};
typedef struct ExpandedTitleRect {int x,y,w,h;} ExpandedTitleRect;
/* 0.10.54_01 — tighten the gaps without moving the bottom of the column:
 * logo still ends at y=244, CAREER now starts at y=426 (182 px lower),
 * the first gap is 8 px and subsequent gaps are 6 px. EXIT still ends
 * at y=871, leaving the build stamp at y=902 undisturbed.
 * All these rectangles are shared by mouse hitboxes and SDL rendering. */
static const ExpandedTitleRect expanded_title_main_rects[EXP_TITLE_MAIN_COUNT] = {
    {1048,426,365,151}, /* CAREER / CONTINUE CAREER */
    {1055,585,350, 67}, /* HALL OF FAME */
    {1055,658,350, 67}, /* CLASSIC MODES */
    {1055,731,350, 67}, /* OPTIONS */
    {1055,804,350, 67}  /* EXIT GAME */
};
/* Left corner stays clear of the logo and floodlight. The same icon
 * hitbox is used whichever video-mode image is currently displayed. */
static const ExpandedTitleRect expanded_title_video_rect = {34,34,92,88};
static const ExpandedTitleRect expanded_title_debug_rect = {1380,300,265,48};
/* The approved logo ends at y=244; DEBUG NEW CAREER ends at y=348.
 * The reused 187x64 blue world toggle fits below both, to the right
 * of the main Career button, with no overlapping hitboxes. */
static const ExpandedTitleRect expanded_title_world_rect = {1440,365,187,64};
static inline ExpandedTitleRect Expanded_TitleMainRect(int i) {
    return i==EXP_TITLE_VIDEO_INDEX ? expanded_title_video_rect :
           i==EXP_TITLE_DEBUG_INDEX ? expanded_title_debug_rect :
           i==EXP_TITLE_WORLD_INDEX ? expanded_title_world_rect :
           expanded_title_main_rects[i];
}
static inline ExpandedTitleRect Expanded_TitleClassicRect(int index) {
    ExpandedTitleRect r = { 422, 172 + index*55, 820, 46 };
    return r;
}
/* A loaded career can leave the TITLE's original gameType at 4 even
 * after returning from gameplay. Allow the TITLE selector only if its
 * just-written, world-specific CAR was verified read-only; the caller
 * detaches the RAM career before installing the other OG static bank.
 * Never abandon an uncheckpointed career or a CLASSIC contest. */
static inline int Expanded_TitleWorldSwitchAllowed(unsigned gameType,
                                                    int checkpointVerified) {
    return gameType==0u || (gameType==4u && checkpointVerified!=0);
}
static inline int Expanded_TitleHit(ExpandedTitleRect r,int x,int y) {
    return x>=r.x && x<r.x+r.w && y>=r.y && y<r.y+r.h;
}
#endif
