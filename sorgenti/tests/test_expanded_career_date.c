/* Exact date banner source contract; isolated, not Windows gameplay. */
#include <assert.h>
#include <stdio.h>
#include "expanded_career_date.h"
static void expect(ExpandedWorldDate d,unsigned y,unsigned m,unsigned day) {
    assert(d.year==y && d.month==m && d.day==day);
}
int main(void)
{
    ExpandedWorldDate shown={0,0,0};
    const ExpandedWorldDate july2026={2026,7,1},june2027={2027,6,30};
    const ExpandedWorldDate july2027={2027,7,1},july1997={1997,7,1};
    /* August 1 is an OG match, NOT the date a contract becomes effective. */
    assert(Expanded_CareerDate_Select(1,1,july2026,2026,56,&shown));
    expect(shown,2026,7,1);
    assert(Expanded_CareerDate_Select(1,1,june2027,2026,56,&shown));
    expect(shown,2027,6,30);
    assert(Expanded_CareerDate_Select(1,1,july2027,2027,56,&shown));
    expect(shown,2027,7,1);
    assert(Expanded_CareerDate_Select(1,1,july1997,1997,56,&shown));
    expect(shown,1997,7,1);
    /* All non-career modes still show the old native fixture phase. */
    assert(Expanded_CareerDate_Select(0,0,(ExpandedWorldDate){0},2026,56,&shown));
    expect(shown,2026,8,1);
    /* Missing or invalid world clock must never be silently converted. */
    assert(!Expanded_CareerDate_Select(1,0,july2027,2027,56,&shown));
    assert(!Expanded_CareerDate_Select(1,1,(ExpandedWorldDate){2027,2,29},
                                       2027,56,&shown));
    assert(!Expanded_CareerDate_Select(1,1,july2026,2026,56,NULL));
    assert(!Expanded_CareerDate_Select(0,0,july2026,2026,255,&shown));
    puts("CAREER_DATE_AFTER_HIRE_WORLD_TODAY=PASS_ISOLATED");
    puts("CAREER_DATE_NATIVE_NEXT_FIXTURE_UNCHANGED=PASS_ISOLATED");
    puts("CAREER_DATE_WINDOWS_VISUAL_AND_PRESEASON=NOT_TESTED");
    return 0;
}
