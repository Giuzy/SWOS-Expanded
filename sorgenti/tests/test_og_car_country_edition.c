/* Isolated positive/negative contract for the READONLY native-country
 * edition gate used by CAR prewrite/restore. Does not compile full game. */
#include <assert.h>
#include <stdio.h>
#include "expanded_og_car_country_edition.h"
static ExpandedWorldDate on(unsigned y,unsigned m,unsigned d)
{
    const ExpandedWorldDate date={(uint16_t)y,(uint8_t)m,(uint8_t)d};
    return date;
}
int main(void)
{
    unsigned cases=0u;
#define TEST(expected,modern,global,country,edition,Y,M,D) do { \
  const int actual=ExpandedOGCar_FutureCountryEditionAllowed( \
    (modern),(global),(country),(edition),on((Y),(M),(D))); \
  assert(actual==(expected));++cases; \
} while(0)
    /* July administrator remains in 2026, Bolivia OG 2027 opened Jan15. */
    TEST(0,1,2026,45,2027,2027,1,14);
    TEST(1,1,2026,45,2027,2027,1,15);
    TEST(1,1,2026,45,2027,2027,4,1);
    TEST(1,1,2026,45,2027,2027,6,30);
    TEST(1,1,2027,45,2028,2028,1,15);
    /* Known country policies, but still only +1 year / exact window. */
    TEST(1,1,2026,36,2027,2027,1,1); /* Sweden Jan-Dec */
    TEST(0,1,2026,44,2027,2027,9,30);
    TEST(1,1,2026,44,2027,2027,10,1); /* Australia Oct-Sep */
    /* Fail closed: unknown, transition, cross-world, skipped editions. */
    TEST(0,0,1996,45,1997,1997,1,15); /* world 9697 */
    TEST(0,1,2026,255,2027,2027,1,15);
    TEST(0,1,2026,73,2027,2027,1,15); /* MLS 2027 exception */
    TEST(0,1,2026,45,2028,2028,1,15); /* skipped OG edition */
    TEST(0,1,2026,45,2027,2026,1,15); /* wrong civil year */
    TEST(0,1,2026,45,2027,2027,2,30); /* invalid day */
    TEST(0,1,2026,45,2026,2026,1,15); /* only newer edition */
#undef TEST
    printf("PASS RG-022 isolated CAR country edition guard: %u cases\n",cases);
    return 0;
}
