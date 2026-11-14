/* Tests the SAME native history match helper called by OG/LWE handoff.
 * Distinct results sharing a native phase must never reuse one fixture.
 * No fabricated games; only verifies deterministic identity selection. */
#include "expanded_native_history.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static void test_repeat_phase(void)
{
    const uint8_t phases[]={56,56,57,57,57,58,59,59};
    const uint8_t expected[]={0,1,2,3,4,5,6,7};
    unsigned next=0;
    uint8_t index=255;
    for(unsigned i=0;i<sizeof(phases);++i) {
        assert(ExpandedNativeHistory_ClaimNext(phases,sizeof phases,&next,
                                               phases[i],&index));
        assert(index==expected[i]);
    }
    assert(next==sizeof(phases));
    assert(!ExpandedNativeHistory_ClaimNext(phases,sizeof phases,&next,59,&index));
}
static void test_reject_mismatch(void)
{
    const uint8_t phases[]={56,56,58};
    unsigned next=0;
    uint8_t index=255;
    assert(ExpandedNativeHistory_ClaimNext(phases,3,&next,56,&index));
    assert(index==0 && next==1);
    assert(!ExpandedNativeHistory_ClaimNext(phases,3,&next,57,&index));
    assert(index==0 && next==1);
    assert(ExpandedNativeHistory_ClaimNext(phases,3,&next,56,&index));
    assert(index==1 && next==2);
    assert(!ExpandedNativeHistory_ClaimNext(phases,3,&next,56,&index));
    assert(index==1 && next==2);
    assert(ExpandedNativeHistory_ClaimNext(phases,3,&next,58,&index));
}
static void test_four_cycles_same_phase(void)
{
    uint8_t phases[92];
    unsigned next=0;
    uint8_t index=255;
    for(unsigned i=0;i<92;++i)phases[i]=(uint8_t)(56+i*72/91);
    for(unsigned i=0;i<92;++i) {
        assert(ExpandedNativeHistory_ClaimNext(phases,92,&next,phases[i],&index));
        assert(index==i);
    }
    assert(next==92);
}
static void test_null_and_bounds(void)
{
    unsigned next=0;
    uint8_t phases[]={56};
    uint8_t index=255;
    assert(!ExpandedNativeHistory_ClaimNext(NULL,1,&next,56,&index));
    assert(!ExpandedNativeHistory_ClaimNext(phases,1,NULL,56,&index));
    assert(!ExpandedNativeHistory_ClaimNext(phases,1,&next,56,NULL));
    assert(!ExpandedNativeHistory_ClaimNext(phases,0,&next,56,&index));
}
int main(void)
{
    test_repeat_phase();
    test_reject_mismatch();
    test_four_cycles_same_phase();
    test_null_and_bounds();
    puts("NATIVE_HISTORY_OCCURRENCE_REPEATED_PHASE=PASS");
    puts("NATIVE_HISTORY_OCCURRENCE_MISMATCH_REJECTED=PASS");
    puts("NATIVE_HISTORY_OCCURRENCE_FOUR_CYCLES_92=PASS");
    return 0;
}
