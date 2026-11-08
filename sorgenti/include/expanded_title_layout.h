/* SWOS Expanded: PNG title buttons use these same logical hitboxes in SDL
 * painting and in input handling. No changes to the SWOS match coordinate space. */
#ifndef EXPANDED_TITLE_LAYOUT_H
#define EXPANDED_TITLE_LAYOUT_H
enum { EXP_TITLE_W=1664, EXP_TITLE_H=936, EXP_TITLE_MAIN_COUNT=5 };
typedef struct ExpandedTitleRect {int x,y,w,h;} ExpandedTitleRect;
/* 0.10.53_04: ~10% smaller again, with 106 px between logo bottom
 * (y=244) and CAREER top (y=350). All centers at x=1230.5.
 * Hitboxes and renderer share these values; title artwork unchanged. */
static const ExpandedTitleRect expanded_title_main_rects[EXP_TITLE_MAIN_COUNT] = {
    {1028,350,405,168}, /* CAREER / CONTINUE CAREER */
    {1036,529,389, 74}, /* HALL OF FAME */
    {1036,611,389, 74}, /* CLASSIC MODES */
    {1036,693,389, 74}, /* OPTIONS */
    {1036,775,389, 74}  /* EXIT GAME */
};
static inline ExpandedTitleRect Expanded_TitleClassicRect(int index) {
    ExpandedTitleRect r = { 422, 172 + index*55, 820, 46 };
    return r;
}
static inline int Expanded_TitleHit(ExpandedTitleRect r,int x,int y) {
    return x>=r.x && x<r.x+r.w && y>=r.y && y<r.y+r.h;
}
#endif
