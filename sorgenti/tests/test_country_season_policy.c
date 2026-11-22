/* RG-T country manager-season inheritance, OG expiry and late KO. */
#include <stdio.h>
#include "expanded_country_season_policy.h"
static unsigned checks;
static unsigned failures;
static void check(const char *label,int got,int expected)
{
    ++checks;
    if(got!=expected) {
        ++failures;
        fprintf(stderr,"FAIL %s got=%d expected=%d\n",
                label,got,expected);
    }
}
int main(void)
{
    /* Same integer unit as the native world's date-index conversion.
     * EU 2026/27 ends on 30 June; Bolivia on 14 January in 2027. */
    const ExpandedCountrySeasonWindow policy[3]={
        {20u,20000,20364},
        {45u,19834,20198},
        {55u,20031,20395}
    };
    ExpandedCountrySeasonWindow effective={0};
    check("Italy manager uses current club country",
        ExpandedCountrySeason_Resolve(policy,3u,20u,-1,-1,&effective),1);
    check("Italy native identity",effective.country==20u,1);
    check("Italy expiry",effective.expiry,20364);
    check("Bolivia manager switch does not copy Italy year",
        ExpandedCountrySeason_Resolve(policy,3u,45u,-1,-1,&effective),1);
    check("Bolivia expiry",effective.expiry,20198);
    check("Bolivia July no automatic reset",
        ExpandedCountrySeason_CanRetire(20000,&effective,78u,78u,0u,1),
        EXP_COMP_EXPIRY_WAIT_DATE);
    check("Bolivia expiry incomplete playoff",
        ExpandedCountrySeason_CanRetire(20198,&effective,78u,77u,1u,0),
        EXP_COMP_EXPIRY_WAIT_OG);
    check("Bolivia late knockout still active",
        ExpandedCountrySeason_CanRetire(20208,&effective,78u,78u,1u,1),
        EXP_COMP_EXPIRY_WAIT_OG);
    check("Bolivia OG all finished on late date",
        ExpandedCountrySeason_CanRetire(20209,&effective,78u,78u,0u,1),
        EXP_COMP_EXPIRY_CAN_RETIRE);
    check("Next Bolivia season cannot open before start",
        ExpandedCountrySeason_CanOpenNext(20209,20210,1,
            EXP_COMP_EXPIRY_CAN_RETIRE),0);
    check("Next cannot open with pending KO",
        ExpandedCountrySeason_CanOpenNext(20210,20210,1,
            EXP_COMP_EXPIRY_WAIT_OG),0);
    check("Next can open after both conditions",
        ExpandedCountrySeason_CanOpenNext(20210,20210,1,
            EXP_COMP_EXPIRY_CAN_RETIRE),1);
    check("New first-ever edition opens at country start",
        ExpandedCountrySeason_CanOpenNext(20210,20210,0,
            EXP_COMP_EXPIRY_WAIT_DATE),1);
    check("Europe expiry still cannot erase unfinished Uefa",
        ExpandedCountrySeason_Resolve(policy,3u,20u,-1,-1,&effective),1);
    check("Europe June 30 OG incomplete",
        ExpandedCountrySeason_CanRetire(20364,&effective,126u,124u,2u,0),
        EXP_COMP_EXPIRY_WAIT_OG);
    check("Europe June 30 complete",
        ExpandedCountrySeason_CanRetire(20364,&effective,126u,126u,0u,1),
        EXP_COMP_EXPIRY_CAN_RETIRE);
    check("Europe before expiry",
        ExpandedCountrySeason_CanRetire(20363,&effective,126u,126u,0u,1),
        EXP_COMP_EXPIRY_WAIT_DATE);
    check("Country override for continental cup",
        ExpandedCountrySeason_Resolve(policy,3u,20u,20100,20400,
                                      &effective),1);
    check("Override expiry distinct from Italy",effective.expiry,20400);
    check("Override before expiry even July",
        ExpandedCountrySeason_CanRetire(20364,&effective,126u,126u,0u,1),
        EXP_COMP_EXPIRY_WAIT_DATE);
    check("No country no silent Europe fallback",
        ExpandedCountrySeason_Resolve(policy,3u,77u,-1,-1,&effective),0);
    check("No half override",
        ExpandedCountrySeason_Resolve(policy,3u,20u,20100,-1,&effective),0);
    check("Inverted override rejected",
        ExpandedCountrySeason_Resolve(policy,3u,20u,20400,20300,
                                      &effective),0);
    check("Negative override start rejected",
        ExpandedCountrySeason_Resolve(policy,3u,20u,-2,20300,
                                      &effective),0);
    /* Regression: failed validation must not partially rebind manager. */
    effective=(ExpandedCountrySeasonWindow){45u,777,888};
    check("Invalid partial override fails without mutation",
        ExpandedCountrySeason_Resolve(policy,3u,20u,20100,-1,
                                      &effective),0);
    check("Invalid override keeps old country",effective.country,45);
    check("Invalid override keeps old start",effective.start,777);
    check("Invalid override keeps old expiry",effective.expiry,888);
    check("Unknown country fails without mutation",
        ExpandedCountrySeason_Resolve(policy,3u,77u,-1,-1,
                                      &effective),0);
    check("Unknown country leaves old expiry",effective.expiry,888);
    const ExpandedCountrySeasonWindow duplicates[2]={
        {20u,20000,20364},{20u,20000,20364}
    };
    check("Duplicate country rejects ambiguity",
        ExpandedCountrySeason_Resolve(duplicates,2u,20u,-1,-1,&effective),0);
    check("Null output",
        ExpandedCountrySeason_Resolve(policy,3u,20u,-1,-1,NULL),0);
    check("Cannot retire before start",
        ExpandedCountrySeason_CanRetire(19999,
            &(ExpandedCountrySeasonWindow){20u,20000,20364},
            126u,126u,0u,1),EXP_COMP_EXPIRY_INVALID);
    check("Never open successor before its own start",
        ExpandedCountrySeason_CanOpenNext(100u,101,0,
            EXP_COMP_EXPIRY_CAN_RETIRE),0);
    check("Reject malformed successor flag",
        ExpandedCountrySeason_CanOpenNext(100u,100,2,
            EXP_COMP_EXPIRY_CAN_RETIRE),0);
    check("Never open successor from invalid previous",
        ExpandedCountrySeason_CanOpenNext(100u,100,1,
            EXP_COMP_EXPIRY_INVALID),0);
    if(failures)return 1;
    printf("PASS %u country season selection / OG expiry / no-July-reset gates (ISOLATED ONLY)\n",checks);
    return 0;
}
