/* RG-001/RG-014 strict JSON-date + verified OG expiry contract. */
#include <stdio.h>
#include "expanded_competition_expiry_gate.h"
static int failures;
static void check(const char *name,
  ExpandedCompetitionExpiryDecision got,
  ExpandedCompetitionExpiryDecision expected)
{
    if(got!=expected){fprintf(stderr,"FAIL %s got=%d want=%d\n",name,
        (int)got,(int)expected);++failures;}
}
int main(void)
{
    /* Date units are from the SAME shared world day index, not a new clock. */
    const int32_t expiry=47000;
    check("before expiry complete",ExpandedCompetitionExpiry_Check(
        expiry-1,expiry,78u,78u,0u,1),EXP_COMP_EXPIRY_WAIT_DATE);
    check("on expiry incomplete",ExpandedCompetitionExpiry_Check(
        expiry,expiry,78u,77u,1u,0),EXP_COMP_EXPIRY_WAIT_OG);
    check("after expiry incomplete",ExpandedCompetitionExpiry_Check(
        expiry+200,expiry,78u,77u,1u,0),EXP_COMP_EXPIRY_WAIT_OG);
    check("all matches but KO not finalized",ExpandedCompetitionExpiry_Check(
        expiry,expiry,78u,78u,0u,0),EXP_COMP_EXPIRY_WAIT_OG);
    check("verified winner but pending playoff",ExpandedCompetitionExpiry_Check(
        expiry,expiry,78u,78u,1u,1),EXP_COMP_EXPIRY_WAIT_OG);
    check("exact expiry full OG",ExpandedCompetitionExpiry_Check(
        expiry,expiry,78u,78u,0u,1),EXP_COMP_EXPIRY_CAN_RETIRE);
    check("postponed final eventually OG witnessed",ExpandedCompetitionExpiry_Check(
        expiry+30,expiry,78u,78u,0u,1),EXP_COMP_EXPIRY_CAN_RETIRE);
    check("no official schedule",ExpandedCompetitionExpiry_Check(
        expiry,expiry,0u,0u,0u,1),EXP_COMP_EXPIRY_INVALID);
    check("overplayed",ExpandedCompetitionExpiry_Check(
        expiry,expiry,78u,79u,0u,1),EXP_COMP_EXPIRY_INVALID);
    check("bad result flag",ExpandedCompetitionExpiry_Check(
        expiry,expiry,78u,78u,0u,2),EXP_COMP_EXPIRY_INVALID);
    check("bad date",ExpandedCompetitionExpiry_Check(
        -1,expiry,78u,78u,0u,1),EXP_COMP_EXPIRY_INVALID);
    if(failures)return 1;
    puts("PASS 11 lifecycle expiry gates, strict OG witness and JSON civil deadline");
    return 0;
}
