#include <assert.h>
#include <stdio.h>
#include "expanded_world_time.h"
int main(void) {
    ExpandedWorldDate d, back;
    int32_t n, max, previous;
    assert(ExpandedWorld_DateToIndex((ExpandedWorldDate){2200,12,31}, &max));
    for (n = 0; n <= max; ++n) {
        assert(ExpandedWorld_IndexToDate(n, &d));
        assert(ExpandedWorld_DateToIndex(d, &previous));
        assert(previous == n && ExpandedWorld_Weekday(n) == n % 7);
    }
    assert(!ExpandedWorld_IndexToDate(-1, &back));
    assert(!ExpandedWorld_IndexToDate(max+1, &back));
    printf("OK: verified %d days (1900-01-01 through 2200-12-31)\n", (int)(max+1));
    return 0;
}
