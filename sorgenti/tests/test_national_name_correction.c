#include "team_source.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
int main(void)
{
    int8_t old[17] = "REPUBLIC CONGO", other[17] = "REPUBLIC CONGO";
    int8_t congo[17] = "CONGO", custom[17] = "MY TEAM", want[17] = "ZAMBIA";
    assert(TeamSource_CorrectNationalName(81,32,old));
    assert(memcmp(old,want,sizeof old)==0);
    assert(!TeamSource_CorrectNationalName(81,32,old));
    assert(!TeamSource_CorrectNationalName(81,7,congo));
    assert(strcmp((char *)congo,"CONGO")==0);
    assert(!TeamSource_CorrectNationalName(81,23,other));
    assert(!TeamSource_CorrectNationalName(80,32,other));
    assert(strcmp((char *)other,"REPUBLIC CONGO")==0);
    assert(!TeamSource_CorrectNationalName(81,32,custom));
    assert(!TeamSource_CorrectNationalName(81,32,0));
    puts("PASS TEAM.081 label correction");
    return 0;
}
