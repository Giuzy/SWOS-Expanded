/* SWOS Expanded: PNG title buttons use these same logical hitboxes in SDL
 * painting and in input handling. No changes to the SWOS match coordinate space. */
#ifndef EXPANDED_TITLE_LAYOUT_H
#define EXPANDED_TITLE_LAYOUT_H
enum {
    EXP_TITLE_W=1664, EXP_TITLE_H=936,
    EXP_TITLE_MAIN_COUNT=5,      /* approved large/small text PNGs */
    EXP_TITLE_VIDEO_INDEX=5,     /* top-left independent icon */
    EXP_TITLE_SELECTABLE_COUNT=6
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
static inline ExpandedTitleRect Expanded_TitleMainRect(int i) {
    return i==EXP_TITLE_VIDEO_INDEX ? expanded_title_video_rect :
           expanded_title_main_rects[i];
}
static inline ExpandedTitleRect Expanded_TitleClassicRect(int index) {
    ExpandedTitleRect r = { 422, 172 + index*55, 820, 46 };
    return r;
}
static inline int Expanded_TitleHit(ExpandedTitleRect r,int x,int y) {
    return x>=r.x && x<r.x+r.w && y>=r.y && y<r.y+r.h;
}
#endif
