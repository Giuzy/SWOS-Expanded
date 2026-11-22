/* RG-T: country edition expiry year must roll EVERY edition, not be
 * hardcoded as 2027 or bound to 30 June for all countries. Pure C11 test. */
#include <stdio.h>
#include "expanded_country_season_year.h"
static unsigned checks,failures;
static void eq(const char *why,int got,int want)
{
    ++checks;
    if(got!=want) {
        ++failures;
        fprintf(stderr,"FAIL %s got=%d want=%d\n",why,got,want);
    }
}
static void date(const char *why,ExpandedWorldDate d,
    unsigned y,unsigned m,unsigned day)
{
    eq(why,d.year==y && d.month==m && d.day==day,1);
}
int main(void)
{
    ExpandedWorldDate a={1u,2u,3u},b={4u,5u,6u};
    ExpandedCountrySeasonCycle europe={7u,1u,6u,30u};
    ExpandedCountrySeasonCycle bolivia={1u,15u,1u,14u};
    ExpandedCountrySeasonCycle solar={1u,1u,12u,31u};
    ExpandedCountrySeasonCycle japan={8u,1u,7u,31u};
    eq("EU first 2026",ExpandedCountrySeason_AnnualDates(
        2026,europe,&a,&b),1);
    date("EU start 2026",a,2026,7,1);
    date("EU expiry 2027",b,2027,6,30);
    eq("EU next 2027",ExpandedCountrySeason_AnnualDates(
        2027,europe,&a,&b),1);
    date("EU expiry 2028",b,2028,6,30);
    eq("EU 2028",ExpandedCountrySeason_AnnualDates(
        2028,europe,&a,&b),1);
    date("EU expiry 2029",b,2029,6,30);
    eq("Bol first 2027",ExpandedCountrySeason_AnnualDates(
        2027,bolivia,&a,&b),1);
    date("Bol start 2027",a,2027,1,15);
    date("Bol expiry 2028",b,2028,1,14);
    eq("Bol next 2028",ExpandedCountrySeason_AnnualDates(
        2028,bolivia,&a,&b),1);
    date("Bol expiry 2029",b,2029,1,14);
    eq("solar 2026",ExpandedCountrySeason_AnnualDates(
        2026,solar,&a,&b),1);
    date("solar ends 2026",b,2026,12,31);
    eq("solar 2027",ExpandedCountrySeason_AnnualDates(
        2027,solar,&a,&b),1);
    date("solar ends 2027",b,2027,12,31);
    eq("Japan 2026",ExpandedCountrySeason_AnnualDates(
        2026,japan,&a,&b),1);
    date("Japan admin end 2027",b,2027,7,31);
    eq("Japan next",ExpandedCountrySeason_AnnualDates(
        2027,japan,&a,&b),1);
    date("Japan admin end 2028",b,2028,7,31);
    eq("MLS sprint edition override",ExpandedCountrySeason_SpecialDates(
        (ExpandedWorldDate){2027u,1u,1u},
        (ExpandedWorldDate){2027u,6u,30u},&a,&b),1);
    date("MLS sprint expires June 2027",b,2027,6,30);
    eq("MLS next special edition",ExpandedCountrySeason_SpecialDates(
        (ExpandedWorldDate){2027u,7u,1u},
        (ExpandedWorldDate){2028u,6u,30u},&a,&b),1);
    date("MLS next ends 2028",b,2028,6,30);
    eq("Bol 2026 special historical",ExpandedCountrySeason_SpecialDates(
        (ExpandedWorldDate){2026u,4u,3u},
        (ExpandedWorldDate){2027u,1u,14u},&a,&b),1);
    date("Bol 2026 historic end",b,2027,1,14);
    eq("Leap 2028",ExpandedCountrySeason_LeapYear(2028),1);
    eq("Century 2100 is not leap",ExpandedCountrySeason_LeapYear(2100),0);
    eq("400th 2000 leap",ExpandedCountrySeason_LeapYear(2000),1);
    eq("Feb 2028",ExpandedCountrySeason_DaysInMonth(2028,2),29);
    eq("Feb 2027",ExpandedCountrySeason_DaysInMonth(2027,2),28);
    eq("Reject expired same start day",ExpandedCountrySeason_AnnualDates(
        2028,(ExpandedCountrySeasonCycle){1u,1u,1u,1u},&a,&b),0);
    a=(ExpandedWorldDate){777u,7u,7u};
    b=(ExpandedWorldDate){778u,8u,8u};
    eq("Reject invalid February",ExpandedCountrySeason_AnnualDates(
        2027,(ExpandedCountrySeasonCycle){2u,29u,12u,31u},&a,&b),0);
    date("Invalid keeps start",a,777,7,7);
    date("Invalid keeps expiry",b,778,8,8);
    eq("Feb29 start in leap",ExpandedCountrySeason_AnnualDates(
        2028,(ExpandedCountrySeasonCycle){2u,29u,2u,28u},&a,&b),1);
    date("Leap cycle next expiry",b,2029,2,28);
    eq("Feb29 next nonleap invalid",ExpandedCountrySeason_AnnualDates(
        2029,(ExpandedCountrySeasonCycle){2u,29u,2u,28u},&a,&b),0);
    eq("Reject absent start pointer",ExpandedCountrySeason_AnnualDates(
        2027,europe,NULL,&b),0);
    eq("Reject same pointers",ExpandedCountrySeason_AnnualDates(
        2027,europe,&b,&b),0);
    eq("Reject invalid year",ExpandedCountrySeason_AnnualDates(
        1899,europe,&a,&b),0);
    eq("Reject 2200 edition",ExpandedCountrySeason_AnnualDates(
        2200,europe,&a,&b),0);
    eq("Reject malformed override",ExpandedCountrySeason_SpecialDates(
        (ExpandedWorldDate){2027u,2u,29u},
        (ExpandedWorldDate){2027u,6u,30u},&a,&b),0);
    eq("Reject inverted override",ExpandedCountrySeason_SpecialDates(
        (ExpandedWorldDate){2028u,6u,30u},
        (ExpandedWorldDate){2027u,6u,30u},&a,&b),0);
    if(failures)return 1;
    printf("PASS %u annual country expiry recurrence + overrides tests (ISO ONLY)\n",
        checks);
    return 0;
}
