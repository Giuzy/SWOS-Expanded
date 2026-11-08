/* SWOS Expanded: PNG title buttons use these same logical hitboxes in SDL
 * painting and in input handling. No changes to the SWOS match coordinate space. */
#ifndef EXPANDED_TITLE_LAYOUT_H
#define EXPANDED_TITLE_LAYOUT_H
enum { EXP_TITLE_W=1664, EXP_TITLE_H=936, EXP_TITLE_MAIN_COUNT=5 };
typedef struct ExpandedTitleRect {int x,y,w,h;} ExpandedTitleRect;
static const ExpandedTitleRect expanded_title_main_rects[EXP_TITLE_MAIN_COUNT] = {
    {128,265,540,224}, /* CAREER: intentionally prominent */
    {148,511,500, 96}, /* HALL OF FAME */
    {148,609,500, 96}, /* CLASSIC MODES */
    {148,707,500, 96}, /* OPTIONS */
    {148,805,500, 96}  /* EXIT GAME */
};
static inline ExpandedTitleRect Expanded_TitleClassicRect(int index) {
    ExpandedTitleRect r = { 422, 172 + index*55, 820, 46 };
    return r;
}
static inline int Expanded_TitleHit(ExpandedTitleRect r,int x,int y) {
    return x>=r.x && x<r.x+r.w && y>=r.y && y<r.y+r.h;
}
#endif
