/* Isolated native-selection contract: no OG match, CAR, or Windows claim. */
#include <assert.h>
#include <stdio.h>
#include "expanded_og_resident_selection.h"

int main(void)
{
    /* Actual Scotland 1997 diagnostic (00.13.200): manager East Stirling
     * is in the original 10 entrants but absent from the 9 selected records. */
    const uint16_t entrants[10]={
        0x2103u,0x2104u,0x2108u,0x210cu,0x210du,
        0x2112u,0x2114u,0x211bu,0x211fu,0x2125u
    };
    const uint16_t selected[9]={
        0x2103u,0x2104u,0x2108u,0x210cu,0x210du,
        0x2114u,0x211bu,0x211fu,0x2125u
    };
    const int32_t rows[10]={0,1,2,3,4,-1,5,6,7,8};
    unsigned row=99u;
    assert(ExpandedOGResidentSelection_ManagerRow(0x2112u,
        entrants,10u,selected,9u,rows,10u,&row));
    assert(row==5u);
    /* Same rule in a 4-team native cup with NO byes. */
    {
        const uint16_t cup[4]={0x2101u,0x2112u,0x2105u,0x2107u};
        const uint16_t cache[3]={0x2101u,0x2105u,0x2107u};
        const int32_t bracket[4]={2,-1,0,1};
        assert(ExpandedOGResidentSelection_ManagerRow(0x2112u,
            cup,4u,cache,3u,bracket,4u,&row) && row==1u);
        /* A padded/bye bracket must NOT be treated as a missing manager. */
        assert(!ExpandedOGResidentSelection_ManagerRow(0x2112u,
            cup,4u,cache,3u,bracket,3u,&row));
    }
    assert(!ExpandedOGResidentSelection_ManagerRow(0x2111u,
        entrants,10u,selected,9u,rows,10u,&row));
    assert(!ExpandedOGResidentSelection_ManagerRow(0xffffu,
        entrants,10u,selected,9u,rows,10u,&row));
    {
        uint16_t bad[9];
        int32_t badRows[10];
        for(unsigned i=0;i<9u;++i)bad[i]=selected[i];
        bad[3]=bad[2]; /* never fix a duplicate by fabricating a club */
        assert(!ExpandedOGResidentSelection_ManagerRow(0x2112u,
            entrants,10u,bad,9u,rows,10u,&row));
        for(unsigned i=0;i<10u;++i)badRows[i]=rows[i];
        badRows[5]=1; /* the manager must have the original -1 identity */
        assert(!ExpandedOGResidentSelection_ManagerRow(0x2112u,
            entrants,10u,selected,9u,badRows,10u,&row));
        badRows[5]=-1;badRows[6]=-1;
        assert(!ExpandedOGResidentSelection_ManagerRow(0x2112u,
            entrants,10u,selected,9u,badRows,10u,&row));
        badRows[6]=9; /* selected index outside 0..8 */
        assert(!ExpandedOGResidentSelection_ManagerRow(0x2112u,
            entrants,10u,selected,9u,badRows,10u,&row));
    }
    puts("OG_RESIDENT_CACHED_MANAGER_REAL_ID=PASS_ISOLATED");
    puts("OG_RESIDENT_MANAGER_DUPLICATE_AND_BYE_GUARD=PASS_ISOLATED");
    puts("OG_RESIDENT_MANAGER_WINDOWS_ROLLOVER=NOT_TESTED");
    return 0;
}
