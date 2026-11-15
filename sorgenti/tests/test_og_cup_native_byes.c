/* Numeric regression for OG world cup bypass rules; does not play matches. */
#include "expanded_og_cup_validation.h"
#include <stdio.h>
static int expect(int want,unsigned b,unsigned n,unsigned real,
                  unsigned ties,unsigned nb,unsigned nt,unsigned np,
                  const char *name)
{
    int got=ExpandedOGCup_NativeByeLayout(b,n,real,ties,nb,nt,np);
    if(got!=want) {
        printf("OG_CUP_BYE_CONTRACT=FAIL case=%s got=%d expected=%d\n",
               name,got,want);
        return 1;
    }
    return 0;
}
int main(void)
{
    int failures=0;
    /* From real 0.13.53 Windows diagnostics: Albania, Belgium, France. */
    failures+=expect(1,16,10,10,2,16,2,8,"Albania_10_of_16");
    failures+=expect(1,32,17,17,1,32,1,16,"Belgium_17_of_32");
    failures+=expect(1,64,36,36,4,64,4,32,"France_36_of_64");
    failures+=expect(0,16,10,9,2,16,2,8,"wrong_native_entrants");
    failures+=expect(0,16,10,10,3,16,3,8,"invented_first_round_ties");
    failures+=expect(0,16,10,10,2,16,8,8,"native_fixture_count_wrong");
    failures+=expect(0,16,10,10,2,8,2,8,"bracket_changed");
    failures+=expect(0,16,10,10,2,16,2,7,"bracket_pairs_wrong");
    failures+=expect(0,16,10,10,2,17,2,8,"invalid_padded_bracket");
    failures+=expect(0,16,16,16,8,16,8,8,"not_a_bye_cup");
    failures+=expect(0,16,1,1,0,16,0,8,"only_one_club");
    if(failures)return 1;
    puts("OG_CUP_BYE_CONTRACT=PASS_ISOLATED_ONLY");
    puts("OG_CUP_REAL_WINDOWS_FIXTURES=NOT_TESTED");
    return 0;
}
