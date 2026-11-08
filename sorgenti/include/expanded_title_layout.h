/* SWOS Expanded: PNG title buttons use these same logical hitboxes in SDL
 * painting and in input handling. No changes to the SWOS match coordinate space. */
#ifndef EXPANDED_TITLE_LAYOUT_H
#define EXPANDED_TITLE_LAYOUT_H
enum { EXP_TITLE_W=1664, EXP_TITLE_H=936, EXP_TITLE_MAIN_COUNT=5 };
typedef struct ExpandedTitleRect {int x,y,w,h;} ExpandedTitleRect;
/* Center the five-button column on the approved logo center x=1230,
 * underneath its bottom y=244. Keep the source PNG aspect ratios and
 * reserve space for the non-destructive build stamp along the bottom. */
static const ExpandedTitleRect expanded_title_main_rects[EXP_TITLE_MAIN_COUNT] = {
    {982,261,496,206}, /* CAREER: 1066x442 source aspect 2.412 */
    {990,479,480, 91}, /* HALL OF FAME: 981x186 */
    {990,581,480, 91}, /* CLASSIC MODES: 981x185 */
    {990,683,480, 91}, /* OPTIONS: 981x185 */
    {990,785,480, 91}  /* EXIT GAME: 981x187 */
};
static inline ExpandedTitleRect Expanded_TitleClassicRect(int index) {
    ExpandedTitleRect r = { 422, 172 + index*55, 820, 46 };
    return r;
}
static inline int Expanded_TitleHit(ExpandedTitleRect r,int x,int y) {
    return x>=r.x && x<r.x+r.w && y>=r.y && y<r.y+r.h;
}
#endif
