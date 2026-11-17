#include "expanded_f12_compare.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    static ExpandedF12Snapshot pre, post;
    ExpandedF12Diff d;
    pre.valid=post.valid=1;pre.world=post.world=26;
    pre.year=post.year=2026;
    pre.count=post.count=2;
    pre.rows[0]=(ExpandedF12Context){0,4,0,1,50,2026,12};
    pre.rows[1]=(ExpandedF12Context){2,254,0,1,50,2026,6};
    post.rows[0]=pre.rows[0];post.rows[1]=pre.rows[1];
    ExpandedF12_Compare(&pre,&post,&d);
    assert(d.comparable && d.checked==2 && !d.missing && !d.viewLost &&
           !d.matchesBack && !d.phaseBack && !d.appeared);
    post.rows[1]=(ExpandedF12Context){2,254,1,1,50,2026,6};
    ExpandedF12_Compare(&pre,&post,&d);
    assert(d.comparable && d.checked==1 && d.missing==1 && d.appeared==1);
    post.rows[1]=pre.rows[1];post.rows[1].viewable=0;
    post.rows[1].matches=5;post.rows[1].phase=49;
    ExpandedF12_Compare(&pre,&post,&d);
    assert(d.viewLost==1 && d.matchesBack==1 && d.phaseBack==1);
    post.year=2027;
    ExpandedF12_Compare(&pre,&post,&d);
    assert(!d.comparable && !d.missing && !d.checked);
    post.year=2026;pre.valid=0;
    ExpandedF12_Compare(&pre,&post,&d);
    assert(!d.comparable);
    puts("F12 comparison isolated: PASS (identity swap, viewer, matches, phase, year)");
    return 0;
}
