/* Pure C regression for the identity matcher, not evidence of
 * qualifiers earned in a running Windows OG career. */
#include "expanded_qualification_transition.h"
#include <assert.h>
#include <stdio.h>
int main(void)
{
    const uint16_t champions[]={0x0101,0x0102,0x0103};
    const uint16_t champions_reordered[]={0x0103,0x0101,0x0102};
    const uint16_t europa[]={0x0201,0x0202};
    const uint16_t wrong_cup[]={0x0101,0x0201,0x0103};
    const uint16_t missing[]={0x0101,0x0103};
    const uint16_t duplicate_source[]={0x0101,0x0101,0x0103};
    const uint16_t duplicate_resident[]={0x0101,0x0101,0x0102};
    const uint16_t invalid[]={0xffff,0x0102,0x0103};
    const uint16_t native_zero[]={0x0000,0x0102};
    ExpandedQualDiff d=ExpandedQual_Compare(champions,3,
                                            champions_reordered,3);
    assert(ExpandedQual_IsExact(&d) && d.matched==3);
    assert(ExpandedQual_SameEdition(9697u,1997u,9697u,1997u));
    assert(!ExpandedQual_SameEdition(9697u,1997u,2627u,1997u));
    assert(!ExpandedQual_SameEdition(9697u,1997u,9697u,1996u));
    d=ExpandedQual_Compare(champions,3,wrong_cup,3);
    assert(!ExpandedQual_IsExact(&d) && d.missing==1 && d.extra==1);
    assert(d.first_missing==0x0102 && d.first_extra==0x0201);
    d=ExpandedQual_Compare(champions,3,missing,2);
    assert(!ExpandedQual_IsExact(&d) && d.missing==1 && d.extra==0);
    d=ExpandedQual_Compare(duplicate_source,3,champions,3);
    assert(!ExpandedQual_IsExact(&d) && d.duplicate_source==1);
    d=ExpandedQual_Compare(champions,3,duplicate_resident,3);
    assert(!ExpandedQual_IsExact(&d) && d.duplicate_resident==1);
    d=ExpandedQual_Compare(invalid,3,champions,3);
    assert(!ExpandedQual_IsExact(&d) && d.invalid_source==1);
    d=ExpandedQual_Compare(native_zero,2,native_zero,2);
    assert(ExpandedQual_IsExact(&d)); /* team word 0x0000 is not a sentinel */
    d=ExpandedQual_Compare(europa,2,europa,2);
    assert(ExpandedQual_IsExact(&d));
    d=ExpandedQual_Compare(NULL,2,europa,2);
    assert(!ExpandedQual_IsExact(&d) && d.invalid_source);
    d=ExpandedQual_Compare(champions,65,champions,3);
    assert(!ExpandedQual_IsExact(&d) && d.invalid_source);
    d=ExpandedQual_Compare(NULL,0,NULL,0);
    assert(ExpandedQual_IsExact(&d)); /* caller must still reject absent source */
    d=ExpandedQual_Compare(champions,3,champions_reordered,3);
    assert(ExpandedQual_FirstEditionValid(EXP_QUAL_FIRST_NATIVE,&d,3));
    assert(ExpandedQual_FirstEditionValid(EXP_QUAL_FIRST_MANUAL_APPROVED,&d,3));
    assert(!ExpandedQual_FirstEditionValid(EXP_QUAL_FIRST_MANUAL_UNVERIFIED,&d,3));
    d=ExpandedQual_Compare(champions,3,wrong_cup,3);
    assert(!ExpandedQual_FirstEditionValid(EXP_QUAL_FIRST_NATIVE,&d,3));
    d=ExpandedQual_Compare(NULL,0,NULL,0);
    assert(!ExpandedQual_FirstEditionValid(EXP_QUAL_FIRST_NATIVE,&d,0));
    /* Current native UEFA fields: 36/36/36 modern and 16/32/32
     * historic. Test stable IDs against SAVE/CONTINUE-like reorder,
     * duplicate injection, and an old field used as a new edition.
     * This is NOT a native-world Windows gameplay test. */
    for(unsigned world=0;world<2u;++world) {
        uint16_t original[3][36]={{0}},restored[3][36]={{0}};
        unsigned sizes[3]={world?36u:16u,world?36u:32u,
                           world?36u:32u};
        for(unsigned cup=0;cup<3u;++cup) {
            for(unsigned i=0;i<sizes[cup];++i) {
                const uint16_t id=(uint16_t)
                    (0x1000u+cup*0x100u+i+1u);
                original[cup][i]=id;
                restored[cup][sizes[cup]-1u-i]=id;
            }
            d=ExpandedQual_Compare(original[cup],sizes[cup],
                                   restored[cup],sizes[cup]);
            assert(ExpandedQual_IsExact(&d) &&
                   d.matched==sizes[cup]);
        }
        restored[0][0]=restored[0][1];
        d=ExpandedQual_Compare(original[0],sizes[0],
                               restored[0],sizes[0]);
        assert(!ExpandedQual_IsExact(&d));
        assert(d.duplicate_resident==1 && d.missing==1);
    }
    /* 1996/97 regression: a missing CWC list must not cancel the
     * verified OG Champions and UEFA fields, nor create a phantom CWC. */
    const unsigned independent[3]={16u,0u,32u};
    assert(ExpandedQual_ClassicEditionReady(0u,independent[0]));
    assert(!ExpandedQual_ClassicEditionReady(1u,independent[1]));
    assert(ExpandedQual_ClassicEditionReady(2u,independent[2]));
    assert(ExpandedQual_ClassicEditionReady(1u,32u));
    assert(!ExpandedQual_ClassicEditionReady(0u,0u));
    assert(!ExpandedQual_ClassicEditionReady(0u,32u));
    assert(!ExpandedQual_ClassicEditionReady(1u,16u));
    assert(!ExpandedQual_ClassicEditionReady(2u,0u));
    assert(!ExpandedQual_ClassicEditionReady(3u,32u));
    puts("OG_CLASSIC_INDEPENDENT_QUALIFIER_GATES=PASS_ISOLATED_ONLY");
    puts("OG_UNEMPLOYED_SOURCE_AND_RESTORED_IDS=PASS_ISOLATED_ONLY");
    puts("OG_FIRST_YEAR_REAL_VS_MANUAL_APPROVAL=PASS_ISOLATED_ONLY");
    puts("OG_QUALIFICATION_IDENTITY_MATCHER=PASS_ISOLATED_ONLY");
    puts("OG_QUALIFICATION_NEXT_SEASON_IN_GAME=NOT_TESTED");
    return 0;
}
