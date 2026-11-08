/* SWOS Expanded: PNG title buttons use these same logical hitboxes in SDL
 * painting and in input handling. No changes to the SWOS match coordinate space. */
#ifndef EXPANDED_TITLE_LAYOUT_H
#define EXPANDED_TITLE_LAYOUT_H
enum { EXP_TITLE_W=1664, EXP_TITLE_H=936, EXP_TITLE_MAIN_COUNT=5 };
typedef struct ExpandedTitleRect {int x,y,w,h;} ExpandedTitleRect;
/* Center the five-button column on the approved logo center x=1230,
 * underneath its bottom y=244. Keep the source PNG aspect ratios and
 * reserve space for the non-destructive build stamp along the bottom. */
/* Smaller, better-spaced column: logo y=17..244, first button y=308.
 * All five boxes have center x=1230. Keep dynamic hitboxes coupled
 * to the renderer and do not resize/change any approved source PNG. */
static const ExpandedTitleRect expanded_title_main_rects[EXP_TITLE_MAIN_COUNT] = {
    {1005,308,450,187}, /* CAREER, 64 px below logo */
    {1014,507,432, 82}, /* HALL OF FAME */
    {1014,601,432, 82}, /* CLASSIC MODES */
    {1014,695,432, 82}, /* OPTIONS */
    {1014,789,432, 82}  /* EXIT GAME */
};
static inline ExpandedTitleRect Expanded_TitleClassicRect(int index) {
    ExpandedTitleRect r = { 422, 172 + index*55, 820, 46 };
    return r;
}
static inline int Expanded_TitleHit(ExpandedTitleRect r,int x,int y) {
    return x>=r.x && x<r.x+r.w && y>=r.y && y<r.y+r.h;
}
#endif
