/* RG-014: purely presentational 14-row Bolivia contest viewport.
 * The ORIGINAL OG resident and key cache own fixtures and results.
 * Mouse/keyboard scrolling must never mutate a competition cursor. */
#ifndef EXPANDED_BOLIVIA_CALENDAR_VIEWPORT_H
#define EXPANDED_BOLIVIA_CALENDAR_VIEWPORT_H
#include <stdint.h>

enum { BOLIVIA_CALENDAR_VIEW_ROWS=14, BOLIVIA_CALENDAR_CENTER_ROW=7 };
typedef struct {
    unsigned total;
    int focusRow,anchor,page,offset;
    uint64_t focusKey;
} ExpandedBoliviaCalendarViewport;
static inline int ExpandedBoliviaCalendar_Clamp(int v,int low,int high)
{ return v<low?low:v>high?high:v; }
static inline void ExpandedBoliviaCalendar_Update(
    ExpandedBoliviaCalendarViewport *v,unsigned total,int focusRow,
    uint64_t focusKey)
{
    if(!v)return;
    if(v->focusKey!=focusKey) {
        v->focusKey=focusKey;
        v->offset=0; /* a new OG fixture always recentres automatically */
    }
    v->total=total;
    v->focusRow=focusRow>=0 && (unsigned)focusRow<total?focusRow:-1;
    v->anchor=v->focusRow>=0?
        v->focusRow-BOLIVIA_CALENDAR_CENTER_ROW:0;
    if(!total) {v->page=0;v->offset=0;return;}
    const int minPage=v->focusRow>=0?-BOLIVIA_CALENDAR_CENTER_ROW:0;
    const int maxPage=v->focusRow>=0?
        (int)total-1-BOLIVIA_CALENDAR_CENTER_ROW:
        (total>BOLIVIA_CALENDAR_VIEW_ROWS?
            (int)total-BOLIVIA_CALENDAR_VIEW_ROWS:0);
    v->page=ExpandedBoliviaCalendar_Clamp(
        v->anchor+v->offset,minPage,maxPage);
    v->offset=v->page-v->anchor;
}
/* direction -1 = previous chronological fixture; +1 = next.
 * Negative page offsets are legal: blank padding keeps the first/last
 * genuinely actionable OG row at the panel's centre rather than top. */
static inline int ExpandedBoliviaCalendar_Scroll(
    ExpandedBoliviaCalendarViewport *v,int direction)
{
    if(!v || !v->total || (direction!=-1 && direction!=1))return 0;
    const int minPage=v->focusRow>=0?-BOLIVIA_CALENDAR_CENTER_ROW:0;
    const int maxPage=v->focusRow>=0?
        (int)v->total-1-BOLIVIA_CALENDAR_CENTER_ROW:
        (v->total>BOLIVIA_CALENDAR_VIEW_ROWS?
            (int)v->total-BOLIVIA_CALENDAR_VIEW_ROWS:0);
    const int next=ExpandedBoliviaCalendar_Clamp(
        v->page+direction,minPage,maxPage);
    if(next==v->page)return 0;
    v->page=next;
    v->offset=v->page-v->anchor;
    return 1;
}
#endif /* EXPANDED_BOLIVIA_CALENDAR_VIEWPORT_H */
