/* SWOS Expanded 0.5: conservative club-infrastructure and finance extension.
 * All new money goes into SWOS's existing _newBalance, the same balance used
 * by player transfers. Existing gate receipts and competition prizes remain
 * owned by the original engine: they are DISPLAYED here, not added twice.
 * Keep the original packed SWOS pixel-art menu; no SDL widgets.
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "menu_system.h"
#include "build_info.h"
#include "game_types.h"
#include "expanded_squad_rating.h"
#include "teams.h"
#include "expanded_facility_rules.h"
#include "expanded_team_mood.h"
#include "expanded_squad_layout.h"
#include "expanded_business_rules.h"

extern int32_t _newBalance, _gateReceipts, _competitionBonuses;
extern uint16_t _selTeamNumber, _currentSeasonShowing;
extern uint16_t _currentDivisionIndex;
extern uint8_t _expandedInfrastructureMenu[];
extern uint8_t _expandedFacilityDetailMenu[];
extern unsigned char Expanded_CurrencyGlyph(void);
extern void ShowMenu(uint8_t *packedMenu);
extern void CallDrawMenuItem(MenuEntry *entry);

uint16_t _expandedEconomyClubId = 0xFFFFu;
uint8_t _expandedStadiumLevel = 1;
uint8_t _expandedFacilityLevels[8] = {1,1,1,1,1,1,1,1};
uint8_t _expandedFacilitySelected = 0;
uint16_t _expandedScoutingYear = 0xFFFFu;
uint16_t _expandedScoutingProspects = 0;
uint16_t _expandedEconomyLastSeason = 0xFFFFu;
uint32_t _expandedShirtSponsor = 0;
uint32_t _expandedStadiumSponsor = 0;
uint32_t _expandedTvRights = 0;
/* Independent persistent 0..100 club sentiment and actual supporter count.
 * Board responds to results AND cash flow, fans to results and manager fame. */
uint8_t _expandedBoardMood = 50;
uint8_t _expandedFanMood = 50;
uint32_t _expandedSupporters = 0;
uint8_t _expandedLowConfidenceMatches = 0;
/* Persistent terms of the current contract. An accepted counteroffer costs
 * the board confidence, while the manager's fame determines initial fan
 * enthusiasm. Both ratings are the SAME scalars shown in Club Business. */
uint8_t _expandedContractPremiumPct = 0;
uint8_t _expandedSuccessfulRaises = 0;
#include "expanded_scout_near.h"
/* 0.10.19: actual business contracts, accumulated weekly cash flow and
 * team form. These scalars are persisted by save_format_json.c. */
uint8_t _expandedSponsorState=0, _expandedTvState=0; /* 0=not offered,1=signed */
uint8_t _expandedFinanceSeasonView=0;
uint32_t _expandedStaffSeason=0, _expandedSponsorPaidSeason=0, _expandedTvPaidSeason=0;
uint32_t _expandedStaffLastWeek=0, _expandedSponsorLastWeek=0, _expandedTvLastWeek=0;
uint32_t _expandedTravelLastWeek=0, _expandedTravelSeason=0;
uint8_t _expandedBusinessStaffHover=0;
/* 1=open after X, 2=closed. These popup states never auto-open on focus. */
uint8_t _expandedBusinessStaffPopupMode=2;  /* WAGES: existing STAFF / WEEK */
uint8_t _expandedBusinessMaintenancePopupMode=2; /* MAINTENANCE */
uint8_t _expandedResults[5]={0},_expandedResultCount=0,_expandedResultHead=0;
uint8_t _expandedTravelFatigue=0;
uint8_t _expandedCurrentForm=65;
uint8_t _expandedTeamMorale=65;
/* No Expanded player-specific condition: only team form and morale.
 * The original SWOS PlayerFile.form and trialist/reserve semantics remain. */
unsigned Expanded_TeamRecentForm(void) {
    unsigned count=_expandedResultCount>5u?5u:_expandedResultCount;
    if(!count) return 65u;
    unsigned total=0;
    for(unsigned i=0;i<count;i++) total+=_expandedResults[i]>3u?3u:_expandedResults[i];
    unsigned recent=(total*100u)/(3u*count);
    unsigned smoothed=(recent*count+65u*(5u-count)+2u)/5u;
    return smoothed>_expandedTravelFatigue?smoothed-_expandedTravelFatigue:0u;
}
unsigned Expanded_TeamCurrentPerformancePct(void) {
    return Expanded_TeamPerformancePct(_expandedCurrentForm,_expandedTeamMorale);
}
/* The admin scale by club stature is deliberately postponed. */
unsigned Expanded_AdministrationAnnualCost(void) {return 45000u;}
/* Distribute the remainder so 52 charged weeks total exactly 45,000. */
extern uint16_t _seasonWeeksDeducted;
unsigned Expanded_AdministrationWeeklyCost(void) {
    return 45000u/52u + ((_seasonWeeksDeducted%52u)<(45000u%52u));
}
extern uint32_t _expandedAnnualSalary;
unsigned Expanded_ManagerWeeklyCost(void) {
    return _expandedAnnualSalary/52u +
           ((_seasonWeeksDeducted%52u)<(_expandedAnnualSalary%52u));
}

static uint8_t s_businessModalBusy=0;
extern uint8_t _lastFinancialWeekIndex, _careerJobOfferPhaseBias;
extern uint16_t _seasonWeeksDeducted;
extern volatile uint16_t _youreSacked;
extern int32_t _generalRunningCosts,_seasonRunningCostsAccum,_seasonRunningCostsAccum2;
extern uint16_t _careerNumPlayers;
extern uint8_t _playerFormModifiers[32];
extern uint8_t _careerTeam[];
extern uint8_t _currentMatchPlayers[32];
extern uint16_t Rand(void);
static uint32_t s_offerOptions[3];
static uint32_t s_tvProposed=0;
extern uint8_t _expandedSponsorOfferMenu[],_expandedTvOfferMenu[];
extern void SetExitMenuFlag(void);
static uint32_t sat_add(uint32_t a,uint32_t b) {return UINT32_MAX-a<b?UINT32_MAX:a+b;}
static unsigned clamp100(int x) {return x<0?0:x>100?100:(unsigned)x;}
unsigned Expanded_FacilityWeeklyTotal(void) {
    unsigned sum=0; /* ADMINISTRATION and MANAGER are separate weekly costs. */
    for(unsigned i=0;i<8;i++) sum+=Expanded_StaffWeekly(i,_expandedFacilityLevels[i]);
    return sum;
}
unsigned Expanded_ScoutCountryVisible(unsigned country) {
    unsigned home=_selTeamNumber>>8;
    unsigned lv=_expandedFacilityLevels[5];
    if(lv>=5) return 1;
    if(country==home) return 1;
    if(lv<=1) return 0;
    if(lv==2) {
        if(home>=80 || country>=80) return 0;
        for(unsigned i=0;i<5;i++) if(s_nearestFive[home][i]==country) return 1;
        return 0;
    }
    if(lv==3) {
        extern const char *Expanded_RegionForCountry(uint8_t);
        return strcmp(Expanded_RegionForCountry((uint8_t)home),
                      Expanded_RegionForCountry((uint8_t)country))==0;
    }
    /* Almost worldwide: a small fraction of remote markets remains hidden. */
    return (country*17u+home*7u)%20u!=0;
}
void Expanded_BusinessWeeklyStep(void) {
    if(_youreSacked || _expandedEconomyClubId!=_selTeamNumber || _selTeamNumber==0xFFFFu) return;
    /* Recalculate native running costs from the live accepted contract. */
    extern int32_t _weeklyRunningCosts;
    _weeklyRunningCosts=(int32_t)(Expanded_AdministrationWeeklyCost()+
                                   Expanded_ManagerWeeklyCost());
    unsigned staff=Expanded_FacilityWeeklyTotal();
    uint32_t sponsor=_expandedSponsorState?_expandedShirtSponsor/52u:0;
    uint32_t tv=_expandedTvState?_expandedTvRights/52u:0;
    _expandedStaffLastWeek=staff;
    _expandedSponsorLastWeek=sponsor;
    _expandedTvLastWeek=tv;
    _expandedStaffSeason=sat_add(_expandedStaffSeason,staff);
    /* Reflect staff wages in the ORIGINAL SWOS running-cost ledger as well;
     * the custom dashboard separates them back out for clarity. */
    _generalRunningCosts=_generalRunningCosts>INT32_MAX-(int32_t)staff?
        INT32_MAX:_generalRunningCosts+(int32_t)staff;
    _seasonRunningCostsAccum=_seasonRunningCostsAccum>INT32_MAX-(int32_t)staff?
        INT32_MAX:_seasonRunningCostsAccum+(int32_t)staff;
    _seasonRunningCostsAccum2=_seasonRunningCostsAccum2>INT32_MAX-(int32_t)staff?
        INT32_MAX:_seasonRunningCostsAccum2+(int32_t)staff;
    _expandedSponsorPaidSeason=sat_add(_expandedSponsorPaidSeason,sponsor);
    _expandedTvPaidSeason=sat_add(_expandedTvPaidSeason,tv);
    int64_t balance=(int64_t)_newBalance-(int64_t)staff+sponsor+tv;
    _newBalance=balance>INT32_MAX?INT32_MAX:balance<INT32_MIN?INT32_MIN:(int32_t)balance;
    unsigned recover=Expanded_FitnessRecovery(_expandedFacilityLevels[4]);
    _expandedTravelFatigue=_expandedTravelFatigue>recover?
         (uint8_t)(_expandedTravelFatigue-recover):0;
    _expandedCurrentForm=(uint8_t)clamp100((int)_expandedCurrentForm+1);
    /* Morale drifts gently toward 65 between matches. */
    if(_expandedTeamMorale<65u) _expandedTeamMorale++;
    else if(_expandedTeamMorale>65u) _expandedTeamMorale--;
    if(_newBalance<0) _expandedBoardMood=(uint8_t)clamp100((int)_expandedBoardMood-1);
}
void Expanded_TrainingComplete(void) {
    unsigned gain=Expanded_TrainingGain(_expandedFacilityLevels[1],_expandedFacilityLevels[4]);
    _expandedTravelFatigue=_expandedTravelFatigue>gain?
         (uint8_t)(_expandedTravelFatigue-gain):0;
    _expandedCurrentForm=(uint8_t)clamp100((int)_expandedCurrentForm+(int)(gain/3u+1u));
    _expandedTeamMorale=(uint8_t)clamp100((int)_expandedTeamMorale+1);
    /* Training retains SWOS's existing built-in player form mechanics;
     * it does not create a new player-specific condition stat. */
    /* Preserve SWOS's existing form and trialist handling. */
    for(unsigned i=0;i<_careerNumPlayers && i<32;i++) {
        int v=(int8_t)_playerFormModifiers[i];
        if(v>=-16 && v<16) {
            int next=v+(int)(gain/4u);
            _playerFormModifiers[i]=(uint8_t)(int8_t)(next>16?16:next);
        }
    }
}
void Expanded_ToggleFinanceView(void) {
    _expandedFinanceSeasonView^=1;
    extern void DrawMenu(void);
    DrawMenu();
}
void Expanded_SponsorSelect(MenuEntry *e) {
    unsigned option=e&&e->ordinal>=2&&e->ordinal<=4?e->ordinal-2:1;
    if(!_expandedSponsorState) {
        _expandedShirtSponsor=s_offerOptions[option];
        _expandedSponsorState=1;
    }
    SetExitMenuFlag();
}
void Expanded_TvAccept(void) {
    if(!_expandedTvState) {
        _expandedTvRights=s_tvProposed;
        _expandedTvState=1;
    }
    SetExitMenuFlag();
}
static void Expanded_PrepareOffers(void) {
    TeamFileEx *t=(TeamFileEx *)(void *)_careerTeam;
    unsigned quality=0;
    for(unsigned i=0;i<16;i++) quality+=(uint8_t)t->players[i].price;
    quality/=16;
    uint32_t base=22000u+quality*3200u+_expandedSupporters*3u;
    base=base*Expanded_OfficeSponsorPct(_expandedFacilityLevels[3])/100u;
    s_offerOptions[0]=base*75u/100u;
    s_offerOptions[1]=base;
    s_offerOptions[2]=base*125u/100u;
    s_tvProposed=38000u+_expandedSupporters*9u+quality*1700u;
}
void Expanded_BusinessMaybeOffer(void) {
    if(s_businessModalBusy || _youreSacked || _selTeamNumber==0xFFFFu ||
       _expandedEconomyClubId!=_selTeamNumber) return;
    unsigned week=Expanded_FinanceWeek(_lastFinancialWeekIndex,_careerJobOfferPhaseBias);
    if(week<2) return;
    s_businessModalBusy=1;
    Expanded_PrepareOffers();
    if(!_expandedSponsorState) ShowMenu(_expandedSponsorOfferMenu);
    if(week>=4 && !_expandedTvState) ShowMenu(_expandedTvOfferMenu);
    s_businessModalBusy=0;
}
void Expanded_SponsorCaption(unsigned,char *,size_t);
void Expanded_TvCaption(char *,size_t);
extern char _expandedSponsorOptionCaption[3][72],_expandedTvOfferCaption[72];
void Expanded_SponsorMenuInit(void) {
    for(unsigned i=0;i<3;i++) Expanded_SponsorCaption(i,_expandedSponsorOptionCaption[i],72);
}
void Expanded_TvMenuInit(void) {
    Expanded_TvCaption(_expandedTvOfferCaption,sizeof _expandedTvOfferCaption);
}
void Expanded_SponsorCaption(unsigned idx,char *out,size_t cap) {
    static const char *names[3]={"LOCAL", "NATIONAL", "PREMIUM"};
    if(idx>2) idx=2;
    snprintf(out,cap,"%s  %c%lu / YR",names[idx],
             Expanded_CurrencyGlyph(),(unsigned long)s_offerOptions[idx]);
}
void Expanded_TvCaption(char *out,size_t cap) {
    snprintf(out,cap,"TV RIGHTS: %c%lu / YR",Expanded_CurrencyGlyph(),
             (unsigned long)s_tvProposed);
}

static uint8_t s_expandedDismissalPending = 0;
static uint8_t s_demoDismissalArmed = 0; /* test concluded: normal gameplay */
void Expanded_TriggerFirstWelcomeSackingDemo(void) {
    if (!s_demoDismissalArmed) return;
    s_demoDismissalArmed=0;
    _expandedFanMood=19;
    _expandedBoardMood=20;
    s_expandedDismissalPending=1;
    fprintf(stderr,"[expanded17] DEMO sacking armed: FANS 19 + BOARD 20 = 39\n");
}
extern volatile uint16_t _youreSacked;
/* Original SWOS remains in charge of displaying the sacking and job offers.
 * Never invoke a modal menu from inside a match-result callback. */
extern uint8_t _expandedManagerFame;
static void ensure_club(void);
static uint8_t clamp_mood(int value) {
    return (uint8_t)(value<0?0:value>100?100:value);
}
/* The five reputation tiers start at exactly 20, 40, 60, 80, 100. */
unsigned Expanded_InitialFanLiking(unsigned fame) {
    return fame>=4 ? 100u : 20u+20u*fame;
}
/* A board usually welcomes a new manager. An ambitious contract and
 * financial trouble reduce its patience; this can be earned back by wins. */
unsigned Expanded_InitialBoardLiking(unsigned fame,unsigned premium,
                                     unsigned raises,int32_t balance) {
    int value=72+(int)(fame>4?4:fame)*2-(int)premium/2;
    value-=(int)(raises>3?3:raises)*3;
    value+=balance<0?-8:balance<100000?-2:4;
    if(value<8) value=8;
    if(value>95) value=95;
    return (unsigned)value;
}
void Expanded_SetContractPremium(unsigned premium) {
    if(premium>100) premium=100;
    _expandedContractPremiumPct=(uint8_t)premium;
    if(premium && _expandedSuccessfulRaises<255)
        _expandedSuccessfulRaises++;
}
int Expanded_DismissalPending(void) {return s_expandedDismissalPending!=0;}
void Expanded_DismissalAcknowledged(void) {
    s_expandedDismissalPending=0;
    _youreSacked=1;
    /* The dismissal can precede the normal end-of-season statistics pass.
     * Record it now, before InitCareer changes the current club/season. */
    extern uint8_t _seasonInformationsArray[];
    extern uint16_t _currentSeasonNumber;
    SeasonInformations *history=(SeasonInformations*)_seasonInformationsArray;
    if(_currentSeasonNumber<20) history[_currentSeasonNumber].managerStatus=-1;
}
void Expanded_EnsureClubSentiment(void) {ensure_club();}
void Expanded_SentimentMatchResult(int result,int away) {
    ensure_club();
    int money=_newBalance<0?-3:_newBalance<100000?-1:1;
    int board=result>0?5:result<0?-6:0;
    int fans=result>0?6:result<0?-7:1;
    _expandedBoardMood=clamp_mood((int)_expandedBoardMood+board+money);
    _expandedFanMood=clamp_mood((int)_expandedFanMood+fans);
    _expandedResults[_expandedResultHead]=(uint8_t)(result>0?3:result<0?0:1);
    _expandedResultHead=(uint8_t)((_expandedResultHead+1u)%5u);
    if(_expandedResultCount<5) _expandedResultCount++;
    _expandedTeamMorale=(uint8_t)clamp100((int)_expandedTeamMorale+
        (result>0?3:result<0?-4:1));
    /* Travel is charged per AWAY match, not every week.  Home = zero.
     * Original weeklyRunningCosts is club overhead, never mislabeled as travel.
     * All charges enter the same SWOS finance ledger and are saved. */
    _expandedTravelLastWeek=away?Expanded_TravelMatchCost(_expandedFacilityLevels[2]):0;
    if(away) {
        uint32_t cost=_expandedTravelLastWeek;
        _expandedTravelSeason=sat_add(_expandedTravelSeason,cost);
        int64_t nextBalance=(int64_t)_newBalance-(int64_t)cost;
        _newBalance=nextBalance<INT32_MIN?INT32_MIN:(int32_t)nextBalance;
        _generalRunningCosts=_generalRunningCosts>INT32_MAX-(int32_t)cost?
            INT32_MAX:_generalRunningCosts+(int32_t)cost;
        _seasonRunningCostsAccum=_seasonRunningCostsAccum>INT32_MAX-(int32_t)cost?
            INT32_MAX:_seasonRunningCostsAccum+(int32_t)cost;
        _seasonRunningCostsAccum2=_seasonRunningCostsAccum2>INT32_MAX-(int32_t)cost?
            INT32_MAX:_seasonRunningCostsAccum2+(int32_t)cost;
        unsigned fatigue=_expandedTravelFatigue+Expanded_AwayFatigue(_expandedFacilityLevels[2]);
        _expandedTravelFatigue=(uint8_t)(fatigue>60?60:fatigue);
    }
    _expandedCurrentForm=(uint8_t)clamp100((int)_expandedCurrentForm+(result>0?3:result<0?-3:0)-(away?1:0));
    /* Dismissal is requested only here. The actual modal is opened from
     * PlayMatchMenu / NextMatch, never during the result callback. */
    if((unsigned)_expandedBoardMood+(unsigned)_expandedFanMood<40u)
        s_expandedDismissalPending=1;
    int delta=result>0?125:result<0?-90:15;
    if(delta<0 && _expandedSupporters<(uint32_t)(-delta)) _expandedSupporters=0;
    else _expandedSupporters=(uint32_t)((int32_t)_expandedSupporters+delta);
    /* Better club offices make sponsor attraction and fan outreach easier. */
    if(result>0) _expandedSupporters+=(uint32_t)(_expandedFacilityLevels[3]-1)*18u;
}

char _expandedInfrastructureCaption[] = "INFRASTRUCTURE";
char _expandedInfraLines[8][72] = {{0}};
char _expandedFacilityDetailLines[4][72] = {{0}};
static const char *const s_facilityNames[8] = {
    "STADIUM", "TRAINING GROUND", "TRANSPORT", "CLUB OFFICES",
    "FITNESS CENTRE", "SCOUTING", "MEDICAL CENTRE", "YOUTH ACADEMY"
};
static const char *const s_transportNames[5] = {
    "MINIBUS", "CLUB BUS", "LUXURY COACH", "CHARTER PLANE", "CLUB AIRCRAFT"
};
static const uint32_t s_baseCost[8] = {
    250000, 75000, 60000, 90000, 80000, 100000, 120000, 95000
};
static uint32_t facility_cost(unsigned idx) {
    if (idx >= 8 || _expandedFacilityLevels[idx] >= 5) return 0;
    unsigned level = _expandedFacilityLevels[idx];
    uint32_t mult = level == 1 ? 1 : level == 2 ? 4 : level == 3 ? 16 : 64;
    return s_baseCost[idx] * mult;
}
static void sync_stadium(void) { _expandedStadiumLevel = _expandedFacilityLevels[0]; }
static void reset_facilities(unsigned stadium) {
    for (int i=0;i<8;i++) _expandedFacilityLevels[i]=1;
    _expandedFacilityLevels[0] = stadium;
    sync_stadium();
    _expandedScoutingYear = 0xFFFFu;
    _expandedScoutingProspects = 0;
}


static const char *const s_levelName[5] = {
    "SMALL GROUND", "LOCAL STADIUM", "CITY STADIUM",
    "NATIONAL ARENA", "MARACANA CLASS"
};
static const uint32_t s_capacity[5] = {5000, 15000, 35000, 65000, 90000};
/* Cost to upgrade from level N to N+1. */
static const uint32_t s_upgradeCost[5] = {250000, 1000000, 5000000, 25000000, 0};

static void ensure_club(void)
{
    if (_expandedEconomyClubId == _selTeamNumber) return;
    _expandedEconomyClubId = _selTeamNumber;
    reset_facilities(_currentDivisionIndex == 0 ? 3 :
                     _currentDivisionIndex == 1 ? 2 : 1);
    _expandedEconomyLastSeason = 0xFFFFu;
    _expandedShirtSponsor = _expandedStadiumSponsor = _expandedTvRights = 0;
    _expandedSponsorState=_expandedTvState=0;
    _expandedStaffSeason=_expandedSponsorPaidSeason=_expandedTvPaidSeason=0;
    _expandedStaffLastWeek=_expandedSponsorLastWeek=_expandedTvLastWeek=0;
    _expandedTravelLastWeek=_expandedTravelSeason=0;
    _expandedBusinessStaffHover=0;
    _expandedResultCount=_expandedResultHead=_expandedTravelFatigue=0;
    memset(_expandedResults,0,sizeof _expandedResults);
    _expandedCurrentForm=65;
    _expandedTeamMorale=65;
    _expandedFinanceSeasonView=0;
    _expandedBoardMood=(uint8_t)Expanded_InitialBoardLiking(
        _expandedManagerFame,_expandedContractPremiumPct,
        _expandedSuccessfulRaises,_newBalance);
    _expandedFanMood=(uint8_t)Expanded_InitialFanLiking(_expandedManagerFame);
    _expandedLowConfidenceMatches=0;
    s_expandedDismissalPending=0;
    _expandedSupporters = s_capacity[_expandedFacilityLevels[0]-1] *
                          (uint32_t)_expandedFanMood / 100u;
}
void Expanded_EconomyReset(void)
{
    _expandedEconomyClubId = 0xFFFFu;
    reset_facilities(1);
    _expandedEconomyLastSeason = 0xFFFFu;
    _expandedShirtSponsor = _expandedStadiumSponsor = _expandedTvRights = 0;
    _expandedBoardMood=50; _expandedFanMood=50; _expandedSupporters=0;
    _expandedSponsorState=_expandedTvState=_expandedFinanceSeasonView=0;
    _expandedStaffSeason=_expandedSponsorPaidSeason=_expandedTvPaidSeason=0;
    _expandedStaffLastWeek=_expandedSponsorLastWeek=_expandedTvLastWeek=0;
    _expandedTravelLastWeek=_expandedTravelSeason=0;
    _expandedBusinessStaffHover=0;
    _expandedResultCount=_expandedResultHead=_expandedTravelFatigue=0;
    memset(_expandedResults,0,sizeof _expandedResults);
    _expandedCurrentForm=65;
    _expandedTeamMorale=65;
    _expandedLowConfidenceMatches=0;
    _expandedContractPremiumPct=0;_expandedSuccessfulRaises=0;
    s_expandedDismissalPending=0;
}
static int32_t add_saturating(int32_t value, uint32_t gain)
{
    return gain > (uint32_t)(INT32_MAX - (int64_t)value) ? INT32_MAX : value + (int32_t)gain;
}
/* Called ONCE after the original new-season finance calculations. */
void Expanded_EconomyNewSeason(void)
{
    ensure_club();
    if (_expandedEconomyLastSeason == _currentSeasonShowing) return;
    for (int i=0;i<8;i++)
        if (_expandedFacilityLevels[i]<1 || _expandedFacilityLevels[i]>5) _expandedFacilityLevels[i]=1;
    sync_stadium();
    /* New contracts require the player to accept one of three real sponsor
     * proposals and the fixed television proposal. No automatic income. */
    _expandedShirtSponsor=_expandedStadiumSponsor=_expandedTvRights=0;
    _expandedSponsorState=_expandedTvState=0;
    _expandedStaffSeason=_expandedSponsorPaidSeason=_expandedTvPaidSeason=0;
    _expandedStaffLastWeek=_expandedSponsorLastWeek=_expandedTvLastWeek=0;
    _expandedTravelLastWeek=_expandedTravelSeason=0;
    _expandedBusinessStaffHover=0;
    /* The annual scouting report refreshes with the season. It is a report
     * capacity metric, NOT fictitious players added to the database. */
    _expandedScoutingYear = _currentSeasonShowing;
    _expandedScoutingProspects = (uint16_t)(_expandedFacilityLevels[5] * 3);
    /* Actual cash arrives weekly via Expanded_BusinessWeeklyStep. */
    _expandedEconomyLastSeason = _currentSeasonShowing;
    /* Financial review adjusts the board mood; avoids double charging
     * or altering the original transfer-market balance. */
    _expandedBoardMood = clamp_mood((int)_expandedBoardMood +
                        (_newBalance<0 ? -10 : _newBalance>1000000 ? 5 : 1));
}
/* Facility UI: all pages use the original SWOS pixel font and menu engine. */
void Expanded_InfrastructureInit(void) {
    ensure_club();
    for (int i=0;i<8;i++) {
        const char *detail = i==2 ? s_transportNames[_expandedFacilityLevels[i]-1] : "";
        snprintf(_expandedInfraLines[i],sizeof _expandedInfraLines[i],
                 "%s  LV %u/5  %s",s_facilityNames[i],
                 (unsigned)_expandedFacilityLevels[i],detail);
    }
}
void Expanded_FacilityDetailInit(void) {
    ensure_club();
    unsigned idx = _expandedFacilitySelected;
    if (idx>=8) idx=0;
    unsigned lv = _expandedFacilityLevels[idx];
    unsigned char c = Expanded_CurrencyGlyph();
    snprintf(_expandedFacilityDetailLines[0],72,"%s  LEVEL %u/5",s_facilityNames[idx],lv);
    if (idx==0) snprintf(_expandedFacilityDetailLines[1],72,"CAPACITY %lu",(unsigned long)s_capacity[lv-1]);
    else if (idx==1) snprintf(_expandedFacilityDetailLines[1],72,"TRAINING EFFECT +%u",Expanded_TrainingGain(lv,_expandedFacilityLevels[4]));
    else if (idx==2) snprintf(_expandedFacilityDetailLines[1],72,"AWAY TRAVEL: %c%u / MATCH",c,Expanded_TravelMatchCost(lv));
    else if (idx==3) snprintf(_expandedFacilityDetailLines[1],72,"SPONSOR APPEAL %u%%",Expanded_OfficeSponsorPct(lv));
    else if (idx==4) snprintf(_expandedFacilityDetailLines[1],72,"WEEKLY RECOVERY +%u",Expanded_FitnessRecovery(lv));
    else if (idx==5) snprintf(_expandedFacilityDetailLines[1],72,"SCOUT REACH: %s",
             lv==1?"HOME":lv==2?"5 NEIGHBOURS":lv==3?"REGIONAL":lv==4?"MOST COUNTRIES":"WORLDWIDE");
    else if (idx==6) snprintf(_expandedFacilityDetailLines[1],72,"INJURY RECOVERY +%u%%",Expanded_MedicalPct(lv));
    else if (idx==7) snprintf(_expandedFacilityDetailLines[1],72,"YOUTH QUALITY +%u",Expanded_YouthPriceBonus(lv));
    else snprintf(_expandedFacilityDetailLines[1],72,"UPGRADE FOR BETTER CLUB SERVICES");
    snprintf(_expandedFacilityDetailLines[2],72,"%s: %c%u / WEEK",
             "STAFF",c,
             Expanded_StaffWeekly(idx,lv));
    if(lv==5) snprintf(_expandedFacilityDetailLines[3],72,"MAXIMUM LEVEL REACHED");
    else snprintf(_expandedFacilityDetailLines[3],72,"UPGRADE COST: %c%lu",c,(unsigned long)facility_cost(idx));
}
void Expanded_UpgradeFacility(MenuEntry *entry) {
    unsigned idx = _expandedFacilitySelected;
    if (idx>=8) return;
    uint32_t cost = facility_cost(idx);
    if (cost && _newBalance >= 0 && (uint32_t)_newBalance >= cost) {
        _newBalance -= (int32_t)cost;
        _expandedFacilityLevels[idx]++;
        sync_stadium();
    }
    Expanded_FacilityDetailInit();
    (void)entry;
    extern void DrawMenu(void);
    DrawMenu();
}
void Expanded_OpenFacility(MenuEntry *entry) {
    if (!entry || entry->ordinal < 1 || entry->ordinal > 8) return;
    _expandedFacilitySelected = (uint8_t)(entry->ordinal-1);
    ShowMenu(_expandedFacilityDetailMenu);
}
void Expanded_ShowInfrastructure(void) {
    ShowMenu(_expandedInfrastructureMenu);
}
/* Called after the original injury status decay. Medical improvements speed
 * recovery for the managed club only; card suspensions are untouched. */
void Expanded_MedicalRecovery(void *teamPtr, int playerCount, unsigned char *players,
                              size_t stride, size_t cardsOffset) {
    if (!teamPtr || !players || !stride) return;
    if (playerCount < 0 || playerCount > 32) return;
    for (int i=0;i<playerCount;i++) {
        unsigned char *ci = players + (size_t)i*stride + cardsOffset;
        unsigned injury = *ci & 0x1C;
        if ((unsigned)Rand()>=Expanded_MedicalPct(_expandedFacilityLevels[6])*256u/100u) continue;
        if (injury==0x10) *ci = (*ci & 0xE3) | 0x0C;
        else if (injury==0x0C) *ci = (*ci & 0xE3) | 0x08;
        else if (injury==0x08) *ci &= 0xE3;
    }
}

/* Expanded 0.7 club overview: hand-painted rectangles at native 320x256
 * coordinates, rendered with SWOS's own palette and bitmap font. This is
 * genuine in-engine pixel art, not a high-resolution SDL overlay. */
#include "blitter.h"
#include "text_render.h"
extern uint8_t *_charTables[];
extern int32_t _playerWagesBill, _generalRunningCosts, _playerSales;
extern int32_t _playerPurchases, _old_balance;
extern uint8_t _expandedClubOverviewMenu[];
static void rect(unsigned x,unsigned y,unsigned w,unsigned h,unsigned color) {
    FillRectangle((uint16_t)x,(uint16_t)y,(uint16_t)w,(uint16_t)h,(uint16_t)color);
}
static void label(unsigned x,unsigned y,unsigned color,const char *str) {
    DrawMenuTextCentered((uint16_t)x,(uint16_t)y,(uint16_t)color,
                         (uint8_t *)str,(uint8_t *)_charTables[0]);
}
static void draw_stadium(unsigned level) {
    /* Grandstand silhouette and pitch; larger tiers appear on higher levels. */
    rect(75,21,170,48,0);
    rect(79,25,162,42,6);
    for(unsigned tier=0;tier<level;tier++) {
        unsigned xx=83+tier*5, yy=28+tier*5;
        unsigned ww=154-tier*10;
        rect(xx,yy,ww,3,7);         /* concrete ledge */
        rect(xx+3,yy+3,ww-6,3,(tier&1)?2:9); /* supporters */
    }
    rect(91,54,138,11,1);          /* outer green pitch */
    rect(96,56,128,7,3);           /* inner pitch */
    rect(158,56,2,7,7);            /* half-way line */
    rect(98,58,13,3,7); rect(209,58,13,3,7);
    rect(106,58,4,3,3); rect(210,58,4,3,3);
    rect(75,20,170,3,7);          /* roof edge */
    if(level>=2) { rect(72,17,176,3,9); rect(76,20,3,10,7); rect(242,20,3,10,7); }
    if(level>=3) { rect(65,16,6,38,7); rect(249,16,6,38,7); rect(61,16,14,3,2); rect(245,16,14,3,2); }
    if(level>=4) { rect(76,13,168,3,7); rect(87,10,146,3,9); }
    if(level>=5) { rect(98,7,124,3,2); rect(109,5,102,2,7); }
}
static void draw_facility_icon(unsigned idx,unsigned level,unsigned x,unsigned y) {
    /* Eight recognisable tiny sprites, each grows/brights with upgrades. */
    unsigned accent=level>=4?2:level>=2?9:6;
    rect(x,y,44,17,0); rect(x+1,y+1,42,15,8);
    switch(idx) {
    case 0: /* stands */
        rect(x+7,y+4,30,3,7); rect(x+9,y+7,26,3,accent);
        rect(x+13,y+10,18,3,3); break;
    case 1: /* training field */
        rect(x+7,y+4,30,10,3); rect(x+21,y+4,2,10,7);
        rect(x+9,y+6,4,6,7); rect(x+31,y+6,4,6,7); break;
    case 2: /* bus -> plane */
        if(level>=4) {
            rect(x+7,y+8,30,3,7);rect(x+18,y+4,8,11,accent);
            rect(x+12,y+11,20,2,7);
        } else {
            rect(x+7,y+5,30,8,accent); rect(x+11,y+6,6,3,7);
            rect(x+20,y+6,6,3,7);rect(x+10,y+12,5,3,0);
            rect(x+29,y+12,5,3,0);
        } break;
    case 3: /* offices */
        rect(x+11,y+4,22,10,accent);
        for(unsigned i=0;i<3;i++) rect(x+14+i*6,y+6,3,3,7);
        break;
    case 4: /* fitness weights */
        rect(x+8,y+8,28,2,7); rect(x+11,y+5,5,8,accent);
        rect(x+28,y+5,5,8,accent); break;
    case 5: /* scouting magnifier */
        rect(x+12,y+4,12,2,7);rect(x+10,y+6,3,7,7);
        rect(x+23,y+6,3,7,7);rect(x+13,y+12,11,2,7);
        rect(x+26,y+12,10,2,accent);break;
    case 6: /* medical cross */
        rect(x+18,y+4,8,10,7);rect(x+12,y+7,20,4,7);
        rect(x+20,y+5,4,8,accent);rect(x+14,y+8,16,2,accent);break;
    default: /* youth academy */
        rect(x+11,y+5,22,8,3);rect(x+16,y+4,12,3,accent);
        rect(x+20,y+8,4,4,7);break;
    }
    if(level==5) rect(x+38,y+3,3,3,2);
}
static uint32_t nonneg(int32_t n) {return n>0?(uint32_t)n:0;}
/* The native menu engine draws the clickable icon entries after onDraw.
 * Repaint the pixel-art on top of the entry backgrounds, but BEFORE the
 * selection outline, so mouse and keyboard focus remain visible. */
extern MenuEntry _menuEntries[];
/* Reapply transient menu-entry geometry for each full DrawMenu.
 * Do NOT reset the popup here: a controller X press triggers DrawMenu,
 * and clearing popup mode during that redraw immediately hid it again. */
void Expanded_ClubOverviewReapplyLayout(void) {
    /* The legacy generic upgrade row stays in the packed layout to keep
     * ordinals stable, but is not visible or selectable. */
    _menuEntries[0].isInvisible=1; /* artwork replaces stock title */
    /* 0.10.23: compact toggle INSIDE the finance title strip, clear of
     * its top border (y=185) and bottom divider (y=199). The joypad
     * hitbox exactly covers the visible button. */
    _menuEntries[9].isInvisible=0;
    _menuEntries[9].x=168;_menuEntries[9].y=187;
    _menuEntries[9].dwWidth=33;_menuEntries[9].dwHeight=12;
    _menuEntries[9].background=NO_BACKGROUND;
    _menuEntries[9].content=NO_FOREGROUND;
    _menuEntries[9].leftEntry=5;
    _menuEntries[9].rightEntry=8;
    _menuEntries[9].upEntry=3;
    _menuEntries[9].downEntry=12; /* Finance -> WAGES -> MAINTENANCE */
    /* Existing popup focus target is now the MAINTENANCE row (ordinal 11).
     * X opens its own popup; FIRE still has no effect. */
    _menuEntries[11].isInvisible=0;
    _menuEntries[11].isDisabled=0;
    _menuEntries[11].x=111;_menuEntries[11].y=222;
    _menuEntries[11].dwWidth=94;_menuEntries[11].dwHeight=12;
    _menuEntries[11].background=NO_BACKGROUND;
    _menuEntries[11].content=NO_FOREGROUND;
    _menuEntries[11].onSelect=0;
    _menuEntries[11].leftEntry=12;
    _menuEntries[11].rightEntry=12;
    _menuEntries[11].upEntry=12;
    _menuEntries[11].downEntry=10;
    /* New focus-only WAGES entry is appended as ordinal 12 to avoid
     * renumbering existing facilities, BACK, and MAINTENANCE. */
    _menuEntries[12].isInvisible=0;
    _menuEntries[12].isDisabled=0;
    _menuEntries[12].x=111;_menuEntries[12].y=198;
    _menuEntries[12].dwWidth=94;_menuEntries[12].dwHeight=12;
    _menuEntries[12].background=NO_BACKGROUND;
    _menuEntries[12].content=NO_FOREGROUND;
    _menuEntries[12].onSelect=0;
    _menuEntries[12].leftEntry=9;
    _menuEntries[12].rightEntry=9;
    _menuEntries[12].upEntry=9;
    _menuEntries[12].downEntry=11;
    /* Facility cards continue via the finance toggle to both expense rows. */
    for (unsigned i=1; i<=8; ++i) {
        _menuEntries[i].background=NO_BACKGROUND;
        _menuEntries[i].content=NO_FOREGROUND;
        _menuEntries[i].downEntry=9;
    }
    /* BACK stays available by LEFT from the first card and after MAINTENANCE. */
    _menuEntries[10].upEntry=11;
    _menuEntries[10].downEntry=5;
    _menuEntries[10].rightEntry=5;
    _menuEntries[10].background=NO_BACKGROUND;
    _menuEntries[10].content=NO_FOREGROUND;
}
void Expanded_ClubOverviewInit(void) {
    ensure_club();
    /* On each visit, both popups begin closed. Redrawing does not clear
     * either state, so X toggles immediately and reliably. */
    _expandedBusinessStaffHover=0;
    _expandedBusinessStaffPopupMode=2;
    _expandedBusinessMaintenancePopupMode=2;
    Expanded_ClubOverviewReapplyLayout();
}
static int cb_draw_selection(void); /* SDL pixel-art overlay, defined below */
void Expanded_ClubOverviewIcons(void) {
    if (cb_draw_selection()) return;
    static const char *const names[8]={"STADIUM","TRAIN","TRAVEL","OFFICE",
                                        "FITNESS","SCOUT","MEDICAL","YOUTH"};
    for(unsigned i=0;i<8;i++) {
        unsigned x=37+(i%4)*67, y=97+(i/4)*23;
        unsigned f=_expandedFacilityLevels[i]; if(f<1 || f>5) f=1;
        draw_facility_icon(i,f,x,y);
        char t[32]; snprintf(t,sizeof t,"%s %u",names[i],f);
        label(x+22,y+16,7,t);
    }
}
void Expanded_ClubFacilitySelected(MenuEntry *entry) {
    if (!entry || entry->ordinal < 1 || entry->ordinal > 8) return;
    _expandedFacilitySelected=(uint8_t)(entry->ordinal-1);
    ShowMenu(_expandedFacilityDetailMenu);
}

/* Calendar date, same formatter in the match calendar and the club ledger.
 * Career finance week is the best available weekly clock; it is not a
 * simulation of a full day-by-day football fixture schedule. */
static const char *const s_months[]={"JAN","FEB","MAR","APR","MAY","JUN",
                                     "JUL","AUG","SEP","OCT","NOV","DEC"};
static const char *const s_days[]={"SUN","MON","TUE","WED","THU","FRI","SAT"};
static int is_leap(unsigned y) {return (y%4==0 && (y%100!=0 || y%400==0));}
void Expanded_FormatDate(char *out,size_t cap,unsigned season,unsigned week) {
    unsigned year=season>=1900?season:(season>=90?1900+season:2000+season);
    if(year<1990 || year>2200) year=1996;
    unsigned month=7, day=1; /* 1 August, season opening */
    unsigned dayIndex=week*7;
    static const unsigned lengths[12]={31,28,31,30,31,30,31,31,30,31,30,31};
    while(dayIndex) {
        unsigned remaining=lengths[month]+(month==1&&is_leap(year))-day;
        if(dayIndex<=remaining) {day+=dayIndex;break;}
        dayIndex-=remaining+1;
        day=1;
        if(++month==12) {month=0;year++;}
    }
    /* Tomohiko Sakamoto's weekday algorithm; 0=Sunday. */
    static const int offsets[]={0,3,2,5,0,3,5,1,4,6,2,4};
    int y=(int)year-(month<2);
    unsigned wd=(unsigned)((y+y/4-y/100+y/400+offsets[month]+(int)day)%7);
    snprintf(out,cap,"%s %02u %s %u",s_days[wd],day,s_months[month],year);
}
extern uint8_t _seasonPlaying;
extern uint8_t _lastFinancialWeekIndex;
extern uint16_t _boardConfidenceScore;
extern uint16_t _numWins, _numLost;


/* SWOS Expanded 0.10.4: game-native 320x256 pixel-art club dashboard.
 * The indexed artwork uses the game's existing 256-colour menu palette;
 * NO SDL overlay, texture, separate window or palette mutation is needed.
 * Real SWOS packed menu entries remain responsible for input/navigation.
 * The artwork and 5x8 font are written to the chunky framebuffer only on
 * the Windows/Linux SDL build. The original Amiga drawing remains below. */
#ifdef SWOS_SDL_BUILD
#include "display.h"
#include "expanded_club_font.h"
extern volatile uintptr_t _frontbufferBasePtr;
extern int _menu_draw_x_offset;
extern int _screen_width;
extern uint8_t _currentMenu[];
#define CB_W 320
#define CB_H 256
#define CB_SW 154
#define CB_SH 88
static uint8_t cb_bg[CB_W*CB_H];
static uint8_t cb_stadium[5][CB_SW*CB_SH];
static int cb_assets_state = 0; /* 0 untried, 1 loaded, -1 missing */
static int cb_read_file(const char *path, uint8_t *dst, size_t len) {
    FILE *f=fopen(path,"rb");
    if (!f) return 0;
    size_t n=fread(dst,1,len,f);
    int extra=fgetc(f);
    fclose(f);
    return n==len && extra==EOF;
}
static int cb_load_assets(void) {
    if (cb_assets_state) return cb_assets_state>0;
    if (!cb_read_file("data/expanded/club_business_bg.raw",cb_bg,sizeof cb_bg))
        goto fail;
    for (unsigned i=0;i<5;i++) {
        char name[80];
        snprintf(name,sizeof name,"data/expanded/club_stadium_%u.raw",i+1);
        if (!cb_read_file(name,cb_stadium[i],sizeof cb_stadium[i])) goto fail;
    }
    cb_assets_state=1;
    return 1;
fail:
    fprintf(stderr,"[club-business] missing/invalid data/expanded art; using legacy layout\n");
    cb_assets_state=-1;
    return 0;
}
static uint8_t *cb_buffer(void) {
    return (uint8_t *)(uintptr_t)_frontbufferBasePtr + _menu_draw_x_offset;
}
static void cb_blit(int x,int y,int w,int h,const uint8_t *src) {
    if (!src || x<0 || y<0 || x+w>CB_W || y+h>CB_H ||
        !_frontbufferBasePtr || _screen_width<CB_W) return;
    uint8_t *dst=cb_buffer();
    for (int row=0;row<h;row++)
        memcpy(dst+(size_t)(y+row)*FRONTBUFFER_ROW_STRIDE_BYTES+x,
               src+(size_t)row*w,(size_t)w);
}
static void cb_plot(int x,int y,uint8_t color) {
    if ((unsigned)x>=CB_W || (unsigned)y>=CB_H || !_frontbufferBasePtr) return;
    cb_buffer()[(size_t)y*FRONTBUFFER_ROW_STRIDE_BYTES+x]=color;
}
/* Compact 13x13 pixel-art faces. Five expressions are computed from the
 * real 0..100 board/fan sentiment, not from static labels. */
static void cb_mood_face(int cx,int cy,unsigned mood) {
    unsigned tier=Expanded_FaceTier(mood);
    /* Five expressions, two red tiers, one yellow tier, two green tiers. */
    /* Bright AGA menu palette indices: red 154, yellow 15, green 243.
     * Original 10/3 are too dark under the active SWOS menu palette. */
    uint8_t face=tier<2?154:tier==2?15:243,outline=8;
    for(int dy=-6;dy<=6;dy++) for(int dx=-6;dx<=6;dx++) {
        int r2=dx*dx+dy*dy;
        if(r2<=36) cb_plot(cx+dx,cy+dy,r2>=26?outline:face);
    }
    cb_plot(cx-2,cy-2,outline);cb_plot(cx+2,cy-2,outline);
    if(tier==0) { /* deep frown */
        cb_plot(cx-3,cy+4,outline);cb_plot(cx-2,cy+2,outline);
        cb_plot(cx-1,cy+1,outline);cb_plot(cx,cy+1,outline);
        cb_plot(cx+1,cy+1,outline);cb_plot(cx+2,cy+2,outline);
        cb_plot(cx+3,cy+4,outline);
    } else if(tier==1) { /* worried */
        cb_plot(cx-3,cy+3,outline);cb_plot(cx-2,cy+2,outline);
        for(int dx=-1;dx<=1;dx++) cb_plot(cx+dx,cy+2,outline);
        cb_plot(cx+2,cy+2,outline);cb_plot(cx+3,cy+3,outline);
    } else if(tier==2) { /* neutral */
        for(int dx=-3;dx<=3;dx++) cb_plot(cx+dx,cy+3,outline);
    } else if(tier==3) { /* smile */
        cb_plot(cx-3,cy+2,outline);cb_plot(cx-2,cy+3,outline);
        for(int dx=-1;dx<=1;dx++) cb_plot(cx+dx,cy+3,outline);
        cb_plot(cx+2,cy+3,outline);cb_plot(cx+3,cy+2,outline);
    } else { /* wide grin */
        cb_plot(cx-3,cy+1,outline);cb_plot(cx+3,cy+1,outline);
        cb_plot(cx-2,cy+3,outline);cb_plot(cx+2,cy+3,outline);
        for(int dx=-1;dx<=1;dx++) cb_plot(cx+dx,cy+4,outline);
    }
}
static const uint8_t cb_pound[8]={0x06,0x09,0x08,0x1C,0x08,0x1F,0,0};
static const uint8_t cb_euro[8]={0x07,0x08,0x1E,0x08,0x1E,0x08,0x07,0};
static const uint8_t *cb_glyph(unsigned char c) {
    if(c==0x9C) return cb_pound;
    if(c==0x9D) return cb_euro;
    if(c>=32 && c<=126) return cb_glyphs[c-32];
    return cb_glyphs[63-32];
}
/* 0.10.8: render the business statistics with the actual original SWOS
 * sprite font.  The previous autogenerated 5x8 bitmap font had ambiguous
 * letters and made the important finance numbers hard to read.  All seven
 * upgrade hitboxes, stadium sprites and panel coordinates are unchanged. */
/* Use only safe printable characters. Business figures intentionally omit
 * currency prefixes; transfer finances elsewhere keep the original format. */
static void cb_safe_text(char *out,size_t cap,const char *src) {
    size_t n=0;
    if(!cap) return;
    while(src && *src && n+1<cap) {
        unsigned char c=(unsigned char)*src++;
        /* This dashboard deliberately shows figures without currency
         * prefixes; the actual SWOS balance and transfer currency are not
         * modified. */
        if(c==0x9D || c==0x9C) continue;
        out[n++]=(c>=32 && c<=126)?(char)c:'?';
    }
    out[n]=0;
}
static int cb_text_width(const char *s,int step) {
    char safe[100]; cb_safe_text(safe,sizeof safe,s);
    uint8_t *font=_charTables[0];
    if(font) return (int)GetTextWidth((uint8_t *)safe,font);
    return *safe ? (int)strlen(safe)*step-1 : 0;
}
static void cb_native(int x,int y,uint8_t color,const char *s,int align,int maxWidth) {
    uint8_t *font=_charTables[0];
    if(!font || !s || maxWidth<=0) return;
    char text[100];
    cb_safe_text(text,sizeof text,s);
    size_t n=strlen(text);
    while(n && GetTextWidth((uint8_t *)text,font)>(unsigned)maxWidth)
        text[--n]=0;
    if(!n) return;
    if(align<0) DrawMenuText((uint16_t)x,(uint16_t)y,color,(uint8_t *)text,font);
    else if(align>0) DrawMenuTextRightAligned((uint16_t)x,(uint16_t)y,color,(uint8_t *)text,font);
    else DrawMenuTextCentered((uint16_t)x,(uint16_t)y,color,(uint8_t *)text,font);
}
/* WEEK/YEAR uses the SAME original SWOS font as the rest of the screen.
 * Its 33x12 focus box is unchanged: this is only a typeface correction. */
static void cb_finance_toggle_label(int centerX,int y,uint8_t color,const char *s) {
    cb_native(centerX,y,color,s,0,29); /* 2px padding inside 33px box */
}
static void cb_text_step(int x,int y,uint8_t color,const char *str,int step) {
    (void)step;
    cb_native(x,y,color,str,-1,CB_W-x-2);
}
static void cb_text(int x,int y,uint8_t color,const char *s) {
    cb_native(x,y,color,s,-1,CB_W-x-2);
}
static void cb_center(int x,int y,uint8_t color,const char *s) {
    cb_native(x,y,color,s,0,2*(x<CB_W/2?x:CB_W-x)-4);
}
static void cb_center_narrow(int x,int y,uint8_t color,const char *s) {
    int maxWidth=2*(x<CB_W/2?x:CB_W-x)-4;
    if(y==159) maxWidth=42;            /* seven upgrade cards */
    else if(x==42 || x==278) maxWidth=68; /* board, stadium, fans */
    else if(x==29) maxWidth=45;       /* BACK */
    else if(x==263) maxWidth=82;      /* balance column */
    cb_native(x,y,color,s,0,maxWidth);
}
static void cb_right(int x,int y,uint8_t color,const char *s) {
    cb_native(x,y,color,s,1,x-2);
}
static void cb_amount(char *out,size_t cap,uint64_t value,int signedValue) {
    /* Comma-grouped figures with no currency prefix, in the original font. */
    char reverse[32];unsigned n=0;
    do {
        if(n && n%4==3) reverse[n++]=',';
        reverse[n++]=(char)('0'+value%10u);value/=10u;
    } while(value && n<sizeof reverse-2);
    size_t pos=0;
    if(signedValue<0 && pos+1<cap) out[pos++]='-';
    while(n && pos+1<cap) out[pos++]=reverse[--n];
    out[pos]=0;
}
static void cb_compact(char *out,size_t cap,uint64_t value,int sign) {
    char n[24];
    if(value>=10000000u) snprintf(n,sizeof n,"%lluM",(unsigned long long)((value+500000)/1000000));
    else if(value>=1000000u) snprintf(n,sizeof n,"%llu.%01lluM",(unsigned long long)(value/1000000),
                                     (unsigned long long)((value%1000000)/100000));
    else if(value>=100000u) snprintf(n,sizeof n,"%lluK",(unsigned long long)((value+500)/1000));
    else if(value>=10000u) snprintf(n,sizeof n,"%llu.%01lluK",(unsigned long long)(value/1000),
                                    (unsigned long long)((value%1000)/100));
    else snprintf(n,sizeof n,"%llu",(unsigned long long)value);
    snprintf(out,cap,"%s%s",sign<0?"-":sign>0?"+":"",n);
}
static void cb_box_outline(int x,int y,int w,int h,uint8_t color) {
    rect(x,y,w,1,color);rect(x,y+h-1,w,1,color);
    rect(x,y,1,h,color);rect(x+w-1,y,1,h,color);
}
/* The MAINTENANCE caption is 11 characters and would overlap its amount
 * when drawn with the full-size SWOS font in the existing 94px finance box.
 * Draw it in a legible 4x5 pixel font, without moving values or changing
 * the box, so all eleven characters fit within the same panel. */
static void cb_maintenance_label(int x,int y,uint8_t color) {
    static const uint8_t glyph[11][5]={
        {9,15,15,9,9},  /* M */
        {6,9,15,9,9},   /* A */
        {15,6,6,6,15},  /* I */
        {9,13,11,9,9},  /* N */
        {15,6,6,6,6},  /* T */
        {15,8,14,8,15},/* E */
        {9,13,11,9,9}, /* N */
        {6,9,15,9,9},  /* A */
        {9,13,11,9,9}, /* N */
        {7,8,8,8,7},   /* C */
        {15,8,14,8,15}/* E */
    };
    for(int i=0;i<11;i++)
        for(int row=0;row<5;row++)
            for(int col=0;col<4;col++)
                if(glyph[i][row] & (8u>>col))
                    cb_plot(x+i*5+col,y+row+1,color);
}
static int cb_staff_hover_active(void) {
    /* The original STAFF / WEEK breakdown now belongs to WAGES (12).
     * Leaving WAGES closes it; returning never auto-opens the popup. */
    Menu *menu=(Menu *)_currentMenu;
    int focused=menu && menu->selectedEntry &&
                menu->selectedEntry->ordinal==12;
    if (!focused) {
        if (_expandedBusinessStaffPopupMode==1)
            _expandedBusinessStaffPopupMode=2;
        return 0;
    }
    return _expandedBusinessStaffPopupMode==1;
}
static int cb_maintenance_hover_active(void) {
    /* MAINTENANCE (11) has a separate, X-only popup with the same rules. */
    Menu *menu=(Menu *)_currentMenu;
    int focused=menu && menu->selectedEntry &&
                menu->selectedEntry->ordinal==11;
    if (!focused) {
        if (_expandedBusinessMaintenancePopupMode==1)
            _expandedBusinessMaintenancePopupMode=2;
        return 0;
    }
    return _expandedBusinessMaintenancePopupMode==1;
}
static void cb_staff_popup(void) {
    if(!cb_staff_hover_active()) return;
    const uint8_t white=2,gold=15;
    char money[48];
    unsigned admin=Expanded_AdministrationWeeklyCost();
    unsigned manager=Expanded_ManagerWeeklyCost();
    uint64_t weeklyStaff=Expanded_FacilityWeeklyTotal()+admin+manager;
    /* Display only: omit ADMIN and STADIUM without changing the actual
     * weekly cost calculation. MANAGER takes ADMIN's top-left position;
     * OFFICES remains first in the right column. */
    static const char *const sn[9]={"MANAGER","TRAINING","TRAVEL",
        "OFFICES","FITNESS","SCOUTING","MEDICAL","YOUTH","TOTAL"};
    uint64_t amounts[9]={manager,
        Expanded_StaffWeekly(1,_expandedFacilityLevels[1]),
        Expanded_StaffWeekly(2,_expandedFacilityLevels[2]),
        Expanded_StaffWeekly(3,_expandedFacilityLevels[3]),
        Expanded_StaffWeekly(4,_expandedFacilityLevels[4]),
        Expanded_StaffWeekly(5,_expandedFacilityLevels[5]),
        Expanded_StaffWeekly(6,_expandedFacilityLevels[6]),
        Expanded_StaffWeekly(7,_expandedFacilityLevels[7]),weeklyStaff};
    /* Four fixed columns: label / right-aligned amount / label / amount.
     * Both value columns stay vertically aligned regardless of label length. */
    const int px=82,py=104,pw=151,ph=62;
    /* Compact paired columns, following the player-skills tooltip spacing. */
    const int leftName=86,leftAmount=149,rightName=158,rightAmount=228;
    rect(px,py,pw,ph,40);cb_box_outline(px,py,pw,ph,204);
    rect(px+1,py+1,pw-2,11,46);
    cb_native(px+pw/2,py+4,15,"STAFF / WEEK",0,pw-8);
    for(unsigned i=0;i<9;i++) {
        int right=i>=3u;
        int yy=py+14+(int)(right?i-3u:i)*8;
        int nameX=right?rightName:leftName;
        int amountX=right?rightAmount:leftAmount;
        cb_native(nameX,yy,i==8?15:white,sn[i],-1,amountX-nameX-3);
        cb_compact(money,sizeof money,amounts[i],0);
        cb_native(amountX,yy,i==8?15:gold,money,1,24);
    }
}
/* Same two-column pixel-art popup geometry, font and colors as STAFF/WEEK.
 * ADMIN and STADIUM occupy the first row of the left and right columns.
 * TOTAL stays in the right column with the same 8px row spacing used by
 * the final YOUTH -> TOTAL lines of the original popup. */
static void cb_maintenance_popup(void) {
    if(!cb_maintenance_hover_active()) return;
    const uint8_t white=2,gold=15;
    const int px=82,py=104,pw=151,ph=38;
    const int leftName=86,leftAmount=149,rightName=158,rightAmount=228;
    const uint64_t admin=Expanded_AdministrationWeeklyCost();
    const uint64_t stadium=Expanded_StaffWeekly(0,_expandedFacilityLevels[0]);
    char money[48];
    rect(px,py,pw,ph,40);cb_box_outline(px,py,pw,ph,204);
    rect(px+1,py+1,pw-2,11,46);
    cb_native(px+pw/2,py+4,gold,"MAINTENANCE / WEEK",0,pw-8);

    cb_native(leftName,py+14,white,"ADMIN",-1,leftAmount-leftName-3);
    cb_compact(money,sizeof money,admin,0);
    cb_native(leftAmount,py+14,gold,money,1,24);

    cb_native(rightName,py+14,white,"STADIUM",-1,rightAmount-rightName-3);
    cb_compact(money,sizeof money,stadium,0);
    cb_native(rightAmount,py+14,gold,money,1,24);

    cb_native(rightName,py+22,gold,"TOTAL",-1,rightAmount-rightName-3);
    cb_compact(money,sizeof money,admin+stadium,0);
    cb_native(rightAmount,py+22,gold,money,1,24);
}
static void cb_business_popups(void) {
    cb_staff_popup();
    cb_maintenance_popup();
}
static int cb_draw_selection(void) {
    if (!cb_load_assets()) return 0;
    Menu *menu=(Menu *)_currentMenu;
    if (!menu || !menu->selectedEntry) {cb_business_popups();return 1;}
    unsigned ordinal=menu->selectedEntry->ordinal;
    int x,y,w,h;
    if(ordinal==9) {x=168;y=187;w=33;h=12;}
    else if(ordinal==11) {x=111;y=222;w=94;h=12;}
    else if(ordinal==12) {x=111;y=198;w=94;h=12;}
    else if(ordinal==1) {x=243;y=108;w=70;h=18;}
    else if(ordinal==10) {x=5;y=3;w=47;h=17;}
    else if(ordinal>=2 && ordinal<=8) {
        static const uint8_t cardByOrdinal[9]={0,0,1,3,5,0,2,4,6};
        x=5+44*cardByOrdinal[ordinal];y=133;w=43;h=48;
    } else {cb_business_popups();return 1;}
    cb_box_outline(x,y,w,h,15);
    cb_box_outline(x+1,y+1,w-2,h-2,6);
    cb_business_popups(); /* active popup stays above the focus outline */
    return 1;
}
static int cb_draw_full(void) {
    if (!cb_load_assets()) return 0;
    cb_blit(0,0,CB_W,CB_H,cb_bg);
    unsigned level=_expandedFacilityLevels[0];
    if(level<1 || level>5) level=1;
    /* Stadium artwork is selected from the club's real infrastructure level. */
    cb_blit(83,33,CB_SW,CB_SH,cb_stadium[level-1]);
    char t[100],money[48];
    const uint8_t gold=15,white=2;
    cb_center_narrow(160,8,white,"CLUB BUSINESS");
    cb_center_narrow(29,8,white,"< BACK");
    Expanded_FormatDate(t,sizeof t,1996u+(unsigned)_currentSeasonShowing,
                        (unsigned)_lastFinancialWeekIndex);
    /* Short date fits the top-right plaque. */
    cb_center_narrow(278,8,gold,strlen(t)>4?t+4:t);
    /* BOARD: only the actual approval percentage and changing face. */
    cb_center(42,28,white,"BOARD");
    snprintf(t,sizeof t,"%u%%",(unsigned)_expandedBoardMood);
    cb_center_narrow(42,40,gold,t);
    cb_mood_face(42,62,_expandedBoardMood);
    /* FANS: happiness, expression and real supporter count; each occupies
     * its own line so the crowd art and values never overlap. */
    cb_center(278,28,white,"FANS");
    snprintf(t,sizeof t,"%u%%",(unsigned)_expandedFanMood);
    cb_center_narrow(278,40,gold,t);
    cb_mood_face(278,57,_expandedFanMood);
    cb_amount(t,sizeof t,_expandedSupporters,0);
    cb_center_narrow(278,71,white,t);
    cb_center(42,87,white,"STADIUM");
    static const char *const names[5]={"SMALL GROUND","LOCAL STAD.","CITY STAD.","NATL ARENA","WORLD ARENA"};
    cb_center_narrow(42,99,gold,names[level-1]);
    snprintf(t,sizeof t,"%lu SEATS",(unsigned long)s_capacity[level-1]);
    cb_center_narrow(42,109,white,t);
    for(unsigned i=0;i<level;i++) rect(11+i*13,119,9,6,15);
    cb_center_narrow(278,87,white,"COSTS");
    if(level==5) cb_center_narrow(278,99,gold,"MAX LEVEL");
    else { cb_compact(money,sizeof money,facility_cost(0),0);
           cb_center_narrow(278,99,gold,money); }
    cb_center_narrow(278,114,white,level==5?"MAXIMUM":"UPGRADE");
    static const char *const captions[7]={"FITNESS","TRAIN","SCOUT","TRAVEL","MEDICAL","OFFICE","YOUTH"};
    static const unsigned facilityIndex[7]={4,1,5,2,6,3,7};
    for(unsigned card=0;card<7;card++) {
        int x=5+(int)card*44;
        unsigned lv=_expandedFacilityLevels[facilityIndex[card]];
        if(lv>5) lv=5;
        cb_center_narrow(x+21,159,white,captions[card]);
        for(unsigned j=0;j<lv;j++) rect(x+4+j*7,171,6,6,15);
    }
    /* Week: current wages + club overhead and staff, with travel charged
     * only for the most recently played AWAY match. Season: booked totals.
     * The native weekly running cost is overhead, NOT travel. */
    uint64_t gate=nonneg(_gateReceipts)+nonneg(_competitionBonuses);
    extern int32_t _weeklyPlayerWages,_weeklyRunningCosts;
    uint64_t weeklyWages=nonneg(_weeklyPlayerWages);
    uint64_t weeklyStaff=Expanded_FacilityWeeklyTotal()+nonneg(_weeklyRunningCosts);
    uint64_t weeklyTravel=_expandedTravelLastWeek;
    uint64_t sponsor=_expandedFinanceSeasonView?_expandedSponsorPaidSeason:
                     (_expandedSponsorState?_expandedShirtSponsor/52u:0u);
    uint64_t tv=_expandedFinanceSeasonView?_expandedTvPaidSeason:
                (_expandedTvState?_expandedTvRights/52u:0u);
    uint64_t wages=_expandedFinanceSeasonView?nonneg(_playerWagesBill):weeklyWages;
    uint64_t travel=_expandedFinanceSeasonView?_expandedTravelSeason:weeklyTravel;
    uint64_t staff=_expandedFinanceSeasonView?
                   (nonneg(_generalRunningCosts)>_expandedTravelSeason?
                    nonneg(_generalRunningCosts)-_expandedTravelSeason:0u):weeklyStaff;
    uint64_t gateDisplay=_expandedFinanceSeasonView?gate:
                         gate/(_seasonWeeksDeducted?_seasonWeeksDeducted:1u);
    uint64_t income=sponsor+tv+gateDisplay,expense=wages+travel+staff;
    unsigned week=Expanded_FinanceWeek(_lastFinancialWeekIndex,_careerJobOfferPhaseBias);
    snprintf(t,sizeof t,"FINANCES / %s %02u",_expandedFinanceSeasonView?"YEAR":"WEEK",week);
    cb_text(9,189,white,t);
    /* 33x12, 2px below the frame top and above the divider. Even when
     * focused the button never overlaps the title strip's outer border. */
    rect(168,187,33,12,42);cb_box_outline(168,187,33,12,204);
    cb_finance_toggle_label(184,189,gold,_expandedFinanceSeasonView?"WEEK":"YEAR");
    cb_center(264,189,white,"BALANCE");
    static const char *const leftLabel[4]={"SPONSOR","TV","GATE","INCOME"};
    static const char *const rightLabel[4]={"WAGES","TRAVEL","MAINTENANCE","TOTAL"};
    uint64_t lv[4]={sponsor,tv,gateDisplay,income};
    uint64_t rv[4]={wages,travel,staff,expense};
    /* Matching outlined finance rows: WAGES and MAINTENANCE. Paint the
     * slightly lighter blue fills behind the labels and values; STAFF's
     * original focus-only hitbox and popup behavior remain unchanged. */
    rect(113,200,90,8,42);  /* WAGES */
    rect(113,224,90,8,42);  /* MAINTENANCE */
    for(unsigned i=0;i<4;i++) {
        int yy=201+(int)i*12;
        cb_text_step(10,yy,i==3?gold:white,leftLabel[i],5);
        cb_compact(money,sizeof money,lv[i],0);
        cb_text_step(101-cb_text_width(money,5),yy,i==3?gold:white,money,5);
        if(i==2) cb_maintenance_label(115,yy,white);
        else cb_text_step(115,yy,i==3?10:white,rightLabel[i],5);
        cb_compact(money,sizeof money,rv[i],0);
        cb_text_step(201-cb_text_width(money,5),yy,i==3?10:white,money,5);
    }
    /* Both expense rows have permanent cyan outlines and focus-only
     * hitboxes: WAGES (12) and MAINTENANCE (11). */
    cb_box_outline(112,199,92,10,204);
    cb_box_outline(112,223,92,10,204);
    int64_t balance=_newBalance;
    cb_amount(money,sizeof money,(uint64_t)(balance<0?-balance:balance),balance<0?-1:0);
    cb_center_narrow(263,206,gold,money);
    /* Forecast, NOT an extrapolation of an already accumulated season.
     * Each displayed figure is the current weekly expense multiplied by
     * 4 or 52.  A home match contributes zero to weekly travel. */
    uint64_t projected=weeklyWages+weeklyStaff+weeklyTravel;
    cb_compact(money,sizeof money,projected*4u,-1);
    snprintf(t,sizeof t,"MONTH %s",money); cb_center_narrow(263,225,white,t);
    cb_compact(money,sizeof money,Expanded_AnnualExpenses(projected),-1);
    snprintf(t,sizeof t,"YEAR %s",money); cb_center_narrow(263,237,white,t);
    cb_business_popups();
    return 1;
}
#else
static int cb_draw_full(void) {return 0;}
static int cb_draw_selection(void) {return 0;}
#endif

void Expanded_ClubOverviewDraw(void) {
    ensure_club();
    if (cb_draw_full()) return;
    unsigned lv=_expandedFacilityLevels[0];
    if(lv<1 || lv>5) lv=1;
    unsigned char c=Expanded_CurrencyGlyph();
    char t[100];
    unsigned crowd=_expandedSupporters;
    rect(12,23,49,39,0); rect(14,25,45,35,8);
    label(36,29,7,"BOARD");
    snprintf(t,sizeof t,"%u%%",(unsigned)_expandedBoardMood);label(36,39,7,t);
    label(36,49,_expandedBoardMood>=50?3:2,_expandedBoardMood>=50?":-)":":-(");
    rect(259,23,51,39,0); rect(261,25,47,35,8);
    label(284,29,7,"FANS");
    snprintf(t,sizeof t,"%u",crowd);label(284,39,3,t);
    label(284,49,_expandedFanMood>=50?3:2,_expandedFanMood>=50?":-)":":-(");
    draw_stadium(lv);
    snprintf(t,sizeof t,"%s / %lu SEATS",s_levelName[lv-1],(unsigned long)s_capacity[lv-1]);
    label(160,72,7,t);
    label(160,84,7,"SELECT A FACILITY TO UPGRADE");
    uint32_t sponsors=_expandedShirtSponsor+_expandedStadiumSponsor;
    uint32_t income=sponsors+_expandedTvRights+nonneg(_gateReceipts)+nonneg(_competitionBonuses);
    uint32_t expense=nonneg(_playerWagesBill)+nonneg(_generalRunningCosts);
    /* Facility captions end near y=146. BACK is now beside the date,
     * and the horizontal separator stops before the clickable button. */
    rect(20,151,220,1,7);
    Expanded_FormatDate(t,sizeof t,1996u+(unsigned)_currentSeasonShowing,
                        (unsigned)_lastFinancialWeekIndex);
    label(160,154,2,t);
    snprintf(t,sizeof t,"WEEKLY: +%c%lu / -%c%lu",c,(unsigned long)(income/52),
             c,(unsigned long)(expense/52));
    label(160,165,7,t);
    label(82,178,7,"INCOME"); label(238,178,7,"EXPENSES");
    snprintf(t,sizeof t,"SPONSOR %c%lu",c,(unsigned long)(sponsors/12)); label(84,189,7,t);
    snprintf(t,sizeof t,"WAGES %c%lu",c,(unsigned long)(nonneg(_playerWagesBill)/12)); label(237,189,7,t);
    snprintf(t,sizeof t,"TV %c%lu",c,(unsigned long)(_expandedTvRights/12)); label(84,200,7,t);
    { extern int32_t _weeklyRunningCosts;
      snprintf(t,sizeof t,"STAFF %c%lu",c,(unsigned long)(Expanded_FacilityWeeklyTotal()+nonneg(_weeklyRunningCosts)));
      label(237,200,7,t); }
    snprintf(t,sizeof t,"GATE/PRIZE %c%lu",c,(unsigned long)((nonneg(_gateReceipts)+nonneg(_competitionBonuses))/12));label(84,211,7,t);
    snprintf(t,sizeof t,"MONTHLY: +%c%lu / -%c%lu",c,(unsigned long)(income/12),c,(unsigned long)(expense/12));
    label(160,222,7,t);
    snprintf(t,sizeof t,"ANNUAL: +%c%lu / -%c%lu",c,(unsigned long)income,c,(unsigned long)expense);
    label(160,233,7,t);
    snprintf(t,sizeof t,"BALANCE %c%ld",c,(long)_newBalance);label(160,244,2,t);
}


/* SWOS Expanded 0.10.5: the other three approved pixel-art menu screens.
 * Real packed menu callbacks, navigation and editable buffers remain untouched.
 * Only screen painting and clickable geometry change. */
#ifdef SWOS_SDL_BUILD
extern uint8_t _swosMainMenu[], _careerMenu[], _contestMenu[],_expandedClubOfferMenu[];
extern uint8_t _managementRecordMenu[],_squadMenu[];
extern uint8_t _seasonInformationsArray[];
extern uint16_t _currentSeasonNumber;
extern uint32_t _expandedAnnualSalary,_expandedCareerEarnings;
extern uint8_t _managerNameBuffer[];
extern void *_countriesTable[];
extern const char *Expanded_RegionForCountry(uint8_t country);
extern int Expanded_UnemployedRecordIsEmpty(void);
extern uint8_t _chairmanMemosMenu[], _expandedSackedMenu[];
extern volatile uint16_t _currentMessage;
extern uint8_t _managerSurnameBuffer[];
extern uint8_t _careerTeam[];
extern uint32_t Expanded_OfferBaseWeekly(void),Expanded_OfferDemandWeekly(void);
extern unsigned Expanded_OfferRejections(void),Expanded_OfferDivision(void);
extern const char *Expanded_OfferRegion(void),*Expanded_OfferClub(void),
                  *Expanded_OfferCountry(void),*Expanded_OfferStatus(void);
extern uintptr_t _currentMenuPtr;
extern uint8_t *Expanded_UnemployedCalendarPointer(void);
extern uint8_t _managerSexBuffer[], _managerNameBuffer[], _managerSurnameBuffer[];
extern char _expandedAgeText[], _expandedManagerFameText[];
extern uint16_t _coachOrPlayer;
static uint8_t ui_title[CB_W*CB_H],ui_career[CB_W*CB_H],
               ui_calendar[CB_W*CB_H],ui_unemployed[CB_W*CB_H],ui_offer[CB_W*CB_H],
               ui_welcome[CB_W*CB_H],ui_sacked[CB_W*CB_H],
               ui_record[CB_W*CB_H],ui_squad[CB_W*CB_H];
static int ui_state=0;
/* The contest panel is drawn to the number of available fixtures.  The
 * underlying art contains only the pitch, not a permanently tall box. */
static unsigned ui_fixture_rows_drawn=0;
static void ui_draw_fixture_panel(void);
static void ui_fixture_finish_panel(void);
static int ui_load(void) {
    if(ui_state) return ui_state>0;
    if(!cb_read_file("data/expanded/expanded_title_bg.raw",ui_title,sizeof ui_title) ||
       !cb_read_file("data/expanded/expanded_career_bg.raw",ui_career,sizeof ui_career) ||
       !cb_read_file("data/expanded/expanded_calendar_bg.raw",ui_calendar,sizeof ui_calendar) ||
       !cb_read_file("data/expanded/expanded_unemployed_bg.raw",ui_unemployed,sizeof ui_unemployed) ||
       !cb_read_file("data/expanded/expanded_offer_bg.raw",ui_offer,sizeof ui_offer) ||
       !cb_read_file("data/expanded/expanded_welcome_bg.raw",ui_welcome,sizeof ui_welcome) ||
       !cb_read_file("data/expanded/expanded_sacked_popup.raw",ui_sacked,sizeof ui_sacked) ||
       !cb_read_file("data/expanded/expanded_record_bg.raw",ui_record,sizeof ui_record) ||
       !cb_read_file("data/expanded/expanded_squad_bg.raw",ui_squad,sizeof ui_squad)) {
        fprintf(stderr,"[expanded-ui] art missing: keeping original SWOS menus\n");
        ui_state=-1;return 0;
    }
    ui_state=1;return 1;
}
static int ui_kind(void) {
    if(_currentMenuPtr==(uintptr_t)_chairmanMemosMenu && _currentMessage==19) return 5;
    if(_currentMenuPtr==(uintptr_t)_expandedSackedMenu) return 6;
    if(_currentMenuPtr==(uintptr_t)_managementRecordMenu) return 7;
    if(_currentMenuPtr==(uintptr_t)_squadMenu) return 8;
    if(_currentMenuPtr==(uintptr_t)_expandedClubOfferMenu) return 4;
    if(_currentMenuPtr==(uintptr_t)_swosMainMenu) return 1;
    if(_currentMenuPtr==(uintptr_t)_careerMenu) return 2;
    if(_currentMenuPtr==(uintptr_t)_contestMenu ||
       _currentMenuPtr==(uintptr_t)Expanded_UnemployedCalendarPointer()) return 3;
    return 0;
}
/* 0.10.7: compact native-font menu geometry.  All packed callbacks, live
 * captions and original conditional entries remain owned by SWOS. */
static void ui_move(unsigned ordinal,int x,int y,int w,int h) {
    Menu *m=(Menu *)_currentMenu;
    if(!m || ordinal>=m->numEntries) return;
    MenuEntry *e=&_menuEntries[ordinal];
    e->x=x;e->y=y;e->dwWidth=w;e->dwHeight=h;
}
static const char *const ui_main_labels[16]={
    "TACTICS", "CUSTOM TEAM", "HIGHLIGHTS", "OPTIONS",
    "HALL OF FAME", "FRIENDLY", "DIY COMP", "PRESET COMP",
    "SEASON", "CAREER", "CONTINUE", "SAVE GAME",
    "LOAD OLD COMP", "MULTIPLAYER", "CREDITS", "HALL OF FAME"
};
static void ui_main_prepare(void) {
    Menu *m=(Menu *)_currentMenu;
    if(!m || m->numEntries<17) return;
    /* No duplicated/cropped original title sprite; approved logo is in art. */
    _menuEntries[0].background=NO_BACKGROUND;
    _menuEntries[0].content=NO_FOREGROUND;
    _menuEntries[16].isInvisible=1; /* legacy duplicate Hall of Fame */
    unsigned order[16],n=0;
    for(unsigned i=1;i<=16 && i<m->numEntries;i++)
        if(!_menuEntries[i].isInvisible) order[n++]=i;
    /* Readable 99x11 buttons, bottom-anchored even with all 15 actions.
     * All original packed callbacks and conditional visibility remain intact. */
    int top=255-(int)n*11;
    for(unsigned i=0;i<n;i++) {
        unsigned ordinal=order[i];
        MenuEntry *e=&_menuEntries[ordinal];
        ui_move(ordinal,219,top+(int)i*11,99,11);
        e->background=NO_BACKGROUND;
        e->content=NO_FOREGROUND;
        e->leftEntry=e->rightEntry=-1;
        int up=-1,down=-1;
        for(int j=(int)i-1;j>=0;j--)
            if(!_menuEntries[order[j]].isDisabled) {up=(int)order[j];break;}
        for(unsigned j=i+1;j<n;j++)
            if(!_menuEntries[order[j]].isDisabled) {down=(int)order[j];break;}
        e->upEntry=(int8_t)up;e->downEntry=(int8_t)down;
        e->directionLeft=e->directionRight=0;
        e->directionUp=e->directionDown=0;
        e->skipLeftEntry=e->skipRightEntry=-1;
        e->skipUpEntry=e->skipDownEntry=-1;
    }
}
static void ui_career_prepare(void) {
    Menu *m=(Menu *)_currentMenu;
    if(!m || m->numEntries<18) return;
    /* 0.10.6's 142x240 form hid too much of the manager and stretched him.
     * This 116x157 form uses half the area and never crosses x=320. */
    static const unsigned fields[]={3,4,5,7,9,15,13};
    /* 0.10.11: panel is flush right and with the bottom edge; no fake grey
     * button. The original packed callbacks and keyboard path are retained. */
    static const int rows[]={124,138,152,166,180,194,208};
    for(unsigned i=0;i<7;i++) ui_move(fields[i],204,rows[i],114,12);
    ui_move(10,204,223,114,16);
    ui_move(11,204,241,114,15);
    ui_move(1,204,241,114,15); /* stock LOAD LIVERPOOL remains invisible */
    /* The original form placed NAME and SURNAME on the same horizontal
     * line, so its left/right/up/down graph no longer matches our vertical
     * layout.  Rewire ONLY navigation, keeping every original onSelect,
     * InputText buffer, and nationality/fame callbacks intact. */
    static const unsigned path[]={3,4,5,7,9,15,13,10,11};
    for(unsigned i=0;i<sizeof path/sizeof path[0];i++) {
        MenuEntry *e=&_menuEntries[path[i]];
        e->leftEntry=e->rightEntry=-1;
        e->upEntry=(int8_t)(i?path[i-1]:11);
        e->downEntry=(int8_t)(i+1<sizeof path/sizeof path[0]?path[i+1]:3);
        e->skipLeftEntry=e->skipRightEntry=-1;
        e->skipUpEntry=e->skipDownEntry=-1;
        e->directionLeft=e->directionRight=0;
        e->directionUp=e->directionDown=0;
    }
    /* The packed title/field labels and the old hints must not paint over
     * the compact native-font form; their interactive fields stay live. */
    static const unsigned decoration[]={0,2,6,8,12,14,16,17};
    for(unsigned i=0;i<sizeof decoration/sizeof decoration[0];i++) {
        MenuEntry *e=&_menuEntries[decoration[i]];
        e->background=NO_BACKGROUND;e->content=NO_FOREGROUND;
    }
    for(unsigned i=0;i<7;i++) {
        MenuEntry *e=&_menuEntries[fields[i]];
        e->background=NO_BACKGROUND; /* preserve native editable text */
    }
    for(unsigned i=0;i<3;i++) {
        MenuEntry *e=&_menuEntries[i==0?10:i==1?11:1];
        e->background=NO_BACKGROUND;e->content=NO_FOREGROUND;
    }
}
static void ui_calendar_prepare(void) {
    Menu *m=(Menu *)_currentMenu;
    if(!m || m->numEntries<19) return;
    /* The huge native season bar and fixture panel caused white/blue
     * horizontal overlays. Keep the REAL fixture entries 19+ untouched. */
    _menuEntries[3].background=NO_BACKGROUND;
    _menuEntries[3].content=NO_FOREGROUND;
    _menuEntries[18].background=NO_BACKGROUND;
    _menuEntries[18].content=NO_FOREGROUND;
    /* The scroll callback copies entry 18's rectangle to the visible
     * buffer. Cover our complete 14-row panel (not the old 116px box),
     * otherwise rows at the bottom stay stale after clicking an arrow.
     * ContestMenuOnReturn has already computed its original game-list
     * data limits before this visual-only adjustment. */
    ui_move(18,9,47,305,133);
    /* Three real dynamic headers, in the three centered artwork boxes. */
    static const int xs[]={5,112,227};
    static const int ws[]={105,113,88};
    for(unsigned i=0;i<3;i++) {
        ui_move(i,xs[i],4,ws[i],13);
        _menuEntries[i].background=NO_BACKGROUND;
        /* Keep native content: manager, team, season update every redraw. */
    }
    static const unsigned ord[]={4,5,6,7,8,9,10,11,12,13,14,15};
    static const int x[]={11,11,11,115,115,115,219,219,219,115,219,115};
    static const int y[]={195,213,231,195,213,231,195,213,231,195,213,231};
    for(unsigned i=0;i<12;i++) {
        ui_move(ord[i],x[i],y[i],x[i]==219?97:90,14);
        _menuEntries[ord[i]].background=NO_BACKGROUND;
        _menuEntries[ord[i]].content=NO_FOREGROUND;
    }
    /* The original fixture template reuses entries 33..43 for EVERY row.
     * Do not relocate those ordinals here: 37 is the highlight, 38/42
     * are team names, and 39..41 are scores / V.  A separate compact
     * renderer handles all rows after CareerGameListCommon has filled them.
     * The stock scroll callbacks on entries 16/17 remain unchanged. */
    if(_currentMenuPtr==(uintptr_t)_contestMenu) {
        ui_move(16,12,49,14,14);
        ui_move(17,12,162,14,14);
        for(unsigned i=16;i<=17;i++) {
            _menuEntries[i].background=NO_BACKGROUND;
            _menuEntries[i].content=NO_FOREGROUND;
        }
    }
}
/* 0.10.15: the first approved concept's buttons are all inside the native
 * 320x256 screen. Unlike the previous version, do not rely on the packed
 * menu's byte-coordinate transforms: they shifted the bottom buttons below
 * the viewport on some displays. Hitboxes and visible pixel-art buttons now
 * use exactly the same absolute coordinates. */
static void ui_welcome_prepare(void) {
    Menu *m=(Menu *)_currentMenu;
    if(!m || m->numEntries<7) return;
    for(unsigned i=0;i<6;i++) {
        _menuEntries[i].background=NO_BACKGROUND;
        _menuEntries[i].content=NO_FOREGROUND;
    }
    ui_move(6,93,229,134,18);
    _menuEntries[6].background=NO_BACKGROUND;
    _menuEntries[6].content=NO_FOREGROUND;
}
static void ui_record_prepare(void) {
    Menu *m=(Menu *)_currentMenu;
    if(!m || m->numEntries<4) return;
    /* Native prev/next/exit callbacks are preserved, only their hitboxes
     * are aligned with the visible pixel-art controls. */
    ui_move(1,14,234,66,18);
    ui_move(2,88,234,66,18);
    ui_move(3,232,234,74,18);
    for(unsigned i=1;i<=3;i++) {
        _menuEntries[i].background=NO_BACKGROUND;
        _menuEntries[i].content=NO_FOREGROUND;
    }
}
extern uint16_t _isSquadViewMode;
static void ui_squad_prepare(void) {
    extern void Expanded_SquadClearRowCache(void);
    Expanded_SquadClearRowCache();
    Menu *m=(Menu *)_currentMenu;
    if(!m || m->numEntries<64) return;
    /* Preserve the real packed onSelect callbacks and up/down graph.
     * NO_BACKGROUND on row entries makes joypad movement request a full
     * redraw, keeping the live player tooltip in sync with focus. */
    for(unsigned i=28;i<=44;i++) {
        /* Native selection/drag rectangle matches the custom 8px row. */
        ui_move(i,10,Expanded_SquadRowY(i-28u)-2,300,EXP_SQUAD_ROW_H);
        MenuEntry *e=&_menuEntries[i];
        e->background=NO_BACKGROUND;
        e->content=NO_FOREGROUND;
        e->leftEntry=-1;
        e->rightEntry=(int8_t)(_isSquadViewMode?11:8);
        e->upEntry=(int8_t)(i==28?(_isSquadViewMode?11:9):i-1);
        e->downEntry=(int8_t)(i==44?(_isSquadViewMode?11:9):i+1);
        e->directionLeft=e->directionRight=e->directionUp=e->directionDown=0;
        e->skipLeftEntry=e->skipRightEntry=e->skipUpEntry=e->skipDownEntry=-1;
    }
    for(unsigned i=6;i<=11;i++) {
        _menuEntries[i].background=NO_BACKGROUND;
        _menuEntries[i].content=NO_FOREGROUND;
    }
    /* Preserve all original callbacks. The normal bottom row is
     * GOALS <-> TRAINING <-> EXIT.  In goals view, BACK <-> EXIT. */
    ui_move(8,146,236,52,18);  /* GOALS */
    ui_move(6,201,236,59,18);  /* TRAINING */
    ui_move(9,263,236,52,18);  /* EXIT (edit) */
    ui_move(10,263,236,52,18); /* EXIT (goals view) */
    ui_move(11,146,236,52,18); /* GOALS -> BACK */
    _menuEntries[10].isInvisible=_isSquadViewMode?0:1;
    _menuEntries[11].isInvisible=_isSquadViewMode?0:1;
    static const unsigned buttons[]={8,6,9,10,11};
    for(unsigned j=0;j<sizeof buttons/sizeof buttons[0];j++) {
        MenuEntry *e=&_menuEntries[buttons[j]];
        e->directionLeft=e->directionRight=e->directionUp=e->directionDown=0;
        e->skipLeftEntry=e->skipRightEntry=e->skipUpEntry=e->skipDownEntry=-1;
        e->upEntry=44;
        e->downEntry=28; /* one DOWN from EXIT returns to the roster */
    }
    _menuEntries[8].leftEntry=44;
    _menuEntries[8].rightEntry=6;
    _menuEntries[6].leftEntry=8;
    _menuEntries[6].rightEntry=9;
    _menuEntries[9].leftEntry=6;
    _menuEntries[9].rightEntry=8;
    _menuEntries[11].leftEntry=44;
    _menuEntries[11].rightEntry=10;
    _menuEntries[10].leftEntry=11;
    _menuEntries[10].rightEntry=11;
}
static void ui_sacked_prepare(void) {
    Menu *m=(Menu *)_currentMenu;
    if(!m || m->numEntries<1) return;
    ui_move(0,63,222,194,23);
    _menuEntries[0].background=NO_BACKGROUND;
    _menuEntries[0].content=NO_FOREGROUND;
}
static void ui_offer_prepare(void) {
    Menu *m=(Menu *)_currentMenu;
    if(!m || m->numEntries<8) return;
    static const int x[]={89,42,162,42,162,111,92,210};
    static const int y[]={3,191,191,212,212,233,166,166};
    static const int w[]={142,116,116,116,116,98,18,18};
    static const int h[]={18,17,17,17,17,17,17,17};
    for(unsigned i=0;i<8;i++) {
        ui_move(i,x[i],y[i],w[i],h[i]);
        _menuEntries[i].background=NO_BACKGROUND;
        _menuEntries[i].content=NO_FOREGROUND;
    }
}
/* The negotiation remains weekly internally (same rules, same five-refusal
 * cap), but every number on the offer screen is now annual, consistent with
 * the contract that Expanded_CommitNegotiatedSalary() stores. */
static void ui_offer_annual(char *dst,size_t cap,uint32_t weekly) {
    uint32_t amount=weekly*52u;
    unsigned char currency=Expanded_CurrencyGlyph();
    if(amount>=1000000u)
        snprintf(dst,cap,"%c%lu,%03lu,%03lu",currency,
                 (unsigned long)(amount/1000000u),
                 (unsigned long)((amount/1000u)%1000u),
                 (unsigned long)(amount%1000u));
    else if(amount>=1000u)
        snprintf(dst,cap,"%c%lu,%03lu",currency,
                 (unsigned long)(amount/1000u),
                 (unsigned long)(amount%1000u));
    else snprintf(dst,cap,"%c%lu",currency,(unsigned long)amount);
}
int Expanded_UI_Active(void) {return ui_load() && ui_kind()!=0;}
void Expanded_UI_Prepare(void) {
    if(!ui_load()) return;
    switch(ui_kind()) {
        case 1: ui_main_prepare();break;
        case 2: ui_career_prepare();break;
        case 3: ui_calendar_prepare();break;
        case 4: ui_offer_prepare();break;
        case 5: ui_welcome_prepare();break;
        case 6: ui_sacked_prepare();break;
        case 7: ui_record_prepare();break;
        case 8: ui_squad_prepare();break;
        default:break;
    }
}
void Expanded_UI_Before(void) {
    if(!ui_load()) return;
    /* All three backgrounds are blitted before native entries. Their old
     * opaque overlays are suppressed in Prepare; real fixture text survives. */
    int kind=ui_kind();
    if(kind==5) cb_blit(0,0,CB_W,CB_H,ui_welcome);
    else if(kind==6) cb_blit(0,0,CB_W,CB_H,ui_unemployed);
    else if(kind==7) cb_blit(0,0,CB_W,CB_H,ui_record);
    else if(kind==8) cb_blit(0,0,CB_W,CB_H,ui_squad);
    else if(kind==4) cb_blit(0,0,CB_W,CB_H,ui_offer);
    else if(kind==1) cb_blit(0,0,CB_W,CB_H,ui_title);
    else if(kind==2) cb_blit(0,0,CB_W,CB_H,ui_career);
    else if(kind==3) {
        cb_blit(0,0,CB_W,CB_H,
            _currentMenuPtr==(uintptr_t)Expanded_UnemployedCalendarPointer()?ui_unemployed:ui_calendar);
        if(_currentMenuPtr==(uintptr_t)_contestMenu) {
            ui_fixture_rows_drawn=0;
            ui_draw_fixture_panel();
        }
    }
}
/* Restore only pixels owned by the custom art; native fixture text is
 * never erased by a partial menu redraw. */
static void ui_restore_rect(const uint8_t *art,int x,int y,int w,int h) {
    if(!art || !_frontbufferBasePtr || _screen_width<CB_W ||
       x<0 || y<0 || x+w>CB_W || y+h>CB_H) return;
    uint8_t *dst=cb_buffer();
    for(int row=0;row<h;row++)
        memcpy(dst+(size_t)(y+row)*FRONTBUFFER_ROW_STRIDE_BYTES+x,
               art+(size_t)(y+row)*CB_W+x,(size_t)w);
}
static void ui_focus(int x,int y,int w,int h) {
    cb_box_outline(x,y,w,h,15);
    if(w>3 && h>3) cb_box_outline(x+1,y+1,w-2,h-2,6);
}
/* Use the ORIGINAL SWOS menu font, not the custom 5x8 club art font.
 * GetTextWidth respects the actual sprite font; shorten text only in a
 * temporary buffer to avoid overflowing its clickable rectangle. */
static void ui_native_text(int x,int y,int maxWidth,uint8_t color,
                           const char *caption,int align) {
    uint8_t *table=_charTables[0];
    if(!table || !caption || maxWidth<=0) return;
    char tmp[64];
    snprintf(tmp,sizeof tmp,"%.62s",caption);
    /* InputText inserts 0xFF while editing; display it as a fixed-width
     * caret so the active NAME/SURNAME field visibly follows typing. */
    for(size_t i=0;tmp[i];i++) if((unsigned char)tmp[i]==0xFF) tmp[i]='|';
    size_t len=strlen(tmp);
    while(len && GetTextWidth((uint8_t *)tmp,table)>(unsigned)maxWidth)
        tmp[--len]=0;
    if(!len) return;
    if(align<0) DrawMenuText((uint16_t)x,(uint16_t)y,color,(uint8_t *)tmp,table);
    else if(align>0) DrawMenuTextRightAligned((uint16_t)x,(uint16_t)y,color,(uint8_t *)tmp,table);
    else DrawMenuTextCentered((uint16_t)x,(uint16_t)y,color,(uint8_t *)tmp,table);
}
/* 0.10.10 calendar: the packed SWOS game-list template is reused for
 * every fixture, so its text cannot be safely "fixed" by moving a handful
 * of ordinal numbers. Draw one bounded, native-font row at a time instead.
 * This is visual-only: scrolling, match data and original callbacks stay in
 * CareerGameListCommon and the packed menu. */
static int ui_prefix(const char *s,const char *prefix) {
    while(*prefix) {
        unsigned char a=(unsigned char)*s++,b=(unsigned char)*prefix++;
        if(a>='a' && a<='z') a=(unsigned char)(a-'a'+'A');
        if(b>='a' && b<='z') b=(unsigned char)(b-'a'+'A');
        if(a!=b) return 0;
    }
    return 1;
}
static void ui_short_comp(char *out,size_t cap,const char *name,
                          const char *round,unsigned cup) {
    if(!cap) return;
    if(!name) name="";
    if(ui_prefix(name,"DIVISION ")) snprintf(out,cap,"D. %s",name+9);
    else if(ui_prefix(name,"LEAGUE CUP")) snprintf(out,cap,"L. CUP");
    else if(ui_prefix(name,"FOOTBALL LEAGUE CUP")) snprintf(out,cap,"L. CUP");
    else if(ui_prefix(name,"FA CUP")) snprintf(out,cap,"FA CUP");
    else snprintf(out,cap,"%.30s",name);
    if(!cup || !round || !*round) return;
    /* A single compact label replaces the two stacked cup lines. */
    char suffix[24]="";
    const char *r=round;
    while(*r && *r!='0' && (*r<'1' || *r>'9')) ++r;
    if(*r>='0' && *r<='9') {
        size_t n=strlen(suffix);
        snprintf(suffix+n,sizeof suffix-n," R%c",*r);
    } else if(ui_prefix(r,"SEMI")) snprintf(suffix,sizeof suffix," SF");
    else if(ui_prefix(r,"QUARTER")) snprintf(suffix,sizeof suffix," QF");
    else if(ui_prefix(r,"FINAL")) snprintf(suffix,sizeof suffix," F");
    const char *leg=strstr(round,"LEG ");
    if(leg && leg[4]>='1' && leg[4]<='9') {
        size_t n=strlen(suffix);
        snprintf(suffix+n,sizeof suffix-n," L%c",leg[4]);
    } else if(strstr(round,"REPLAY")) {
        size_t n=strlen(suffix);
        snprintf(suffix+n,sizeof suffix-n," RP");
    }
    size_t used=strlen(out);
    if(used<cap-1) snprintf(out+used,cap-used,"%s",suffix);
}
static void ui_arrow(int cx,int cy,int down) {
    /* Inside the fixture panel, not outside its left border. */
    for(int j=0;j<8;j++) {
        int yy=cy+(down?j:7-j);
        cb_plot(cx,yy,10);
        cb_plot(cx+1,yy,10);
    }
    for(int j=0;j<4;j++) {
        int yy=cy+(down?5+j:2-j);
        for(int dx=-j;dx<=j+1;dx++) cb_plot(cx+dx,yy,10);
    }
}
/* The native 129px list-height check used 12-20 logical pixels per match,
 * despite this renderer needing just 9. The original logic therefore stopped
 * after ~8 fixtures even when 14 were cached. We allow the native fixture
 * decoder to prepare all 14, and make the panel follow the actual row count.
 * The schedule, scrolling and original menu actions are untouched. */
extern void *_gameListArray[];
static unsigned ui_fixture_count(void) {
    unsigned n=0;
    while(n<14 && _gameListArray[n*2]) n++;
    return n;
}
static int ui_fixture_panel_bottom=0;
static int ui_fixture_bottom_for_count(unsigned n) {
    if(n<1) n=1;
    if(n>14) n=14;
    /* 14px top margin, 12-15px below the last glyph. 14 rows still fit
     * above the action buttons at y=195. */
    int bottom=67+(int)n*9; /* 8 fixtures -> 139; 14 -> 190 */
    return bottom>190?190:bottom;
}
static void ui_draw_fixture_panel(void) {
    int bottom=ui_fixture_bottom_for_count(ui_fixture_count());
    ui_fixture_panel_bottom=bottom;
    /* x=5..314: symmetric side margins in the 320px SWOS framebuffer. */
    rect(5,39,310,(unsigned)(bottom-39),42);
    for(int y=46;y<bottom-3;y+=14) {
        int h=bottom-3-y;
        if(h>12) h=12;
        if(h>0) rect(8,(unsigned)y,304,(unsigned)h,40);
    }
    cb_box_outline(5,39,310,bottom-39,7);
    rect(7,41,306,1,2);
    rect(7,(unsigned)(bottom-3),306,1,2);
    ui_move(17,12,bottom-22,14,14);
    ui_arrow(19,53,0);
    ui_arrow(19,bottom-21,1);
}
void Expanded_UI_ClearFixtureArea(void) {
    if(!ui_load() || _currentMenuPtr!=(uintptr_t)_contestMenu) return;
    /* Restore the original stadium beneath any previous, taller frame.
     * Also clear stale team names when using the red scroll arrows. */
    ui_restore_rect(ui_calendar,5,39,310,151);
    ui_fixture_rows_drawn=0;
    ui_draw_fixture_panel();
}
static void ui_fixture_finish_panel(void) {
    if(_currentMenuPtr!=(uintptr_t)_contestMenu || !ui_fixture_panel_bottom)
        return;
    /* A cache slot can be skipped by native rules. Count painted rows, not
     * just non-NULL pointers, and shrink only the empty lower section. */
    int bottom=ui_fixture_bottom_for_count(ui_fixture_rows_drawn);
    int oldBottom=ui_fixture_panel_bottom;
    if(bottom>=oldBottom) return;
    ui_restore_rect(ui_calendar,5,bottom,310,oldBottom-bottom);
    /* Remove any part of the old down arrow still inside the new panel.
     * This narrow strip never touches the fixture text (starts at x=29). */
    int oldArrow=oldBottom-22;
    if(oldArrow<bottom) rect(11,(unsigned)oldArrow,17,(unsigned)(bottom-oldArrow),42);
    cb_box_outline(5,39,310,bottom-39,7);
    rect(7,(unsigned)(bottom-3),306,1,2);
    ui_move(17,12,bottom-22,14,14);
    ui_arrow(19,bottom-21,1);
    ui_fixture_panel_bottom=bottom;
}
static void ui_native_ellipsis(int x,int y,int maxWidth,uint8_t color,
                               const char *name,int align) {
    uint8_t *font=_charTables[0];
    if(!font || !name || maxWidth<=0) return;
    char text[90];
    snprintf(text,sizeof text,"%.86s",name);
    size_t n=strlen(text);
    int cut=0;
    while(n && GetTextWidth((uint8_t *)text,font)>(unsigned)maxWidth) {
        text[--n]=0;cut=1;
    }
    if(cut && n>2) {
        while(n>2) {
            text[n]='.';text[n+1]='.';text[n+2]=0;
            if(GetTextWidth((uint8_t *)text,font)<=(unsigned)maxWidth) break;
            text[--n]=0;
        }
    }
    ui_native_text(x,y,maxWidth,color,text,align);
}
int Expanded_UI_CompactFixtureRow(unsigned row,unsigned slotType) {
    if(!ui_load() || _currentMenuPtr!=(uintptr_t)_contestMenu) return 0;
    if(row>=14 || row>=ui_fixture_count()) return 1;
    if(row+1>ui_fixture_rows_drawn) ui_fixture_rows_drawn=row+1;
    const int y=53+(int)row*9;
    /* Dark 8-pixel rows never extend beyond the panel's right border. */
    rect(28,y-1,282,8,(row&1)?40:42);
    if(!_menuEntries[37].isInvisible)
        rect(81,y-1,229,8,11);
    char comp[80];
    const char *name=_menuEntries[34].fg?
        (const char *)(uintptr_t)_menuEntries[34].fg:"";
    const char *round=(!_menuEntries[35].isInvisible && _menuEntries[35].fg)?
        (const char *)(uintptr_t)_menuEntries[35].fg:"";
    ui_short_comp(comp,sizeof comp,name,round,slotType!=0);
    ui_native_text(29,y,73,slotType?15:2,comp,-1);
    if(!_menuEntries[38].isInvisible && _menuEntries[38].fg) {
        ui_native_ellipsis(102,y,68,2,
            (const char *)(uintptr_t)_menuEntries[38].fg,-1);
    }
    if(!_menuEntries[36].isInvisible && _menuEntries[36].fg)
        ui_native_text(178,y,18,2,
            (const char *)(uintptr_t)_menuEntries[36].fg,0);
    if(!_menuEntries[40].isInvisible && _menuEntries[40].fg) {
        ui_native_text(196,y,12,2,
            (const char *)(uintptr_t)_menuEntries[40].fg,0);
    } else if(!_menuEntries[39].isInvisible && !_menuEntries[41].isInvisible) {
        char score[16];
        snprintf(score,sizeof score,"%d-%d",
                 (int)(int16_t)_menuEntries[39].fgWord,
                 (int)(int16_t)_menuEntries[41].fgWord);
        ui_native_text(196,y,24,2,score,0);
    }
    if(!_menuEntries[42].isInvisible && _menuEntries[42].fg)
        ui_native_ellipsis(210,y,100,2,
            (const char *)(uintptr_t)_menuEntries[42].fg,-1);
    return 1;
}
static void ui_button(unsigned ordinal,const char *caption) {
    Menu *m=(Menu *)_currentMenu;
    if(!m || ordinal>=m->numEntries) return;
    MenuEntry *e=&_menuEntries[ordinal];
    if(e->isInvisible) return;
    int x=e->x,y=e->y,w=e->dwWidth,h=e->dwHeight;
    if(x<0 || y<0 || x+w>320 || y+h>256 || w<5 || h<8) return;
    int kind=ui_kind();
    uint8_t fill=46;
    if((kind==1 && ordinal==10) || (kind==2 && ordinal==10) ||
       (kind==3 && ordinal==7)) fill=230;
    if((kind==2 && ordinal==11) || (kind==3 && ordinal==9)) fill=72;
    if(kind==5 || kind==6) fill=14;
    if(kind==4 && ordinal==3) fill=14; /* ACCEPT: green */
    if(kind==4 && ordinal==4) fill=88; /* DECLINE: red */
    if(kind==4 && ordinal==5) fill=72; /* BACK: brown */
    if(kind==4 && ordinal==6) fill=72; /* less: brown */
    if(kind==4 && ordinal==7) fill=14; /* more: green */
    if(e->isDisabled) fill=46; /* readable, muted; callback stays disabled */
    rect(x+1,y+1,w-2,h-2,fill);
    cb_box_outline(x,y,w,h,e->isDisabled?7:204);
    if(m->selectedEntry==e) ui_focus(x,y,w,h);
    uint8_t *font=_charTables[0];
    int fontHeight=font?*(uint16_t *)(void *)(font+CHARTABLE_CHARHEIGHT):8;
    if(fontHeight<5 || fontHeight>14) fontHeight=8;
    /* Center on the real original font metrics, not a hard-coded 8px.
     * The selection border never changes font, size, or text position. */
    int ty=y+(h-fontHeight)/2;
    if(ty<y+1) ty=y+1;
    ui_native_text(x+w/2,ty,w-10,e->isDisabled?7:2,caption,0);
}
static void ui_record_status(char *out,size_t cap,int code) {
    if(code==0) snprintf(out,cap,"-");
    else if(code==1) snprintf(out,cap,"WINNERS");
    else if(code==2) snprintf(out,cap,"RUNNER-UP");
    else if(code==-100) snprintf(out,cap,"FINALIST");
    else if(code==-101) snprintf(out,cap,"SEMI-FINAL");
    else if(code==-102) snprintf(out,cap,"QUARTER-FINAL");
    else if(code==-103) snprintf(out,cap,"THIRD PLACE");
    else if(code==-104) snprintf(out,cap,"IN COMPETITION");
    else if(code==-105) snprintf(out,cap,"FOURTH PLACE");
    else if(code==-106) snprintf(out,cap,"-");
    else if(code>0) snprintf(out,cap,"POSITION %d",code);
    else if(code<=-110) snprintf(out,cap,"OUT ROUND %d",-code-110);
    else snprintf(out,cap,"POSITION %d",-code);
}
static void ui_record_render(void) {
    cb_blit(0,0,CB_W,CB_H,ui_record);
    /* Leave the manager and touchline fully visible on the upper left.
     * Every textual datum is drawn using SWOS's original sharp font. */
    rect(5,4,310,19,40);cb_box_outline(5,4,310,19,204);
    char line[96];
    snprintf(line,sizeof line,"MANAGEMENT RECORD - %.15s %.15s",
             (const char*)_managerNameBuffer,(const char*)_managerSurnameBuffer);
    ui_native_text(160,10,296,2,line,0);
    rect(123,27,192,78,40);cb_box_outline(123,27,192,78,204);
    rect(125,29,188,13,42);
    int empty=Expanded_UnemployedRecordIsEmpty();
    const SeasonInformations *si=&((const SeasonInformations*)_seasonInformationsArray)
                                  [_currentSeasonNumber<20?_currentSeasonNumber:0];
    unsigned season=1996u+(_currentSeasonNumber<20?_currentSeasonNumber:0);
    snprintf(line,sizeof line,"%u/%02u SEASON",season,(season+1u)%100u);
    ui_native_text(218,32,175,15,line,0);
    const char *country="-",*region="-";
    unsigned countryNo=si->clubNo>>8;
    if(!empty && countryNo<80 && _countriesTable[countryNo]) {
        country=(const char*)_countriesTable[countryNo]+1;
        region=Expanded_RegionForCountry((uint8_t)countryNo);
    } else if(!empty && countryNo>=80 && countryNo<=85) {
        country="NATIONAL";
        region=Expanded_RegionForCountry((uint8_t)countryNo);
    }
    const char *club=empty?"NO CLUB YET":(const char*)si->clubName;
    if(!club || !club[0]) club="NO CLUB YET";
    ui_native_text(129,45,53,15,"CLUB",-1);
    ui_native_text(307,45,119,2,club,1);
    ui_native_text(129,56,53,15,"COUNTRY",-1);
    ui_native_text(307,56,119,2,country,1);
    ui_native_text(129,67,53,6,"REGION",-1);
    ui_native_text(307,67,119,2,region,1);
    ui_native_text(129,78,53,15,"SALARY",-1);
    snprintf(line,sizeof line,"%c%lu / YR",Expanded_CurrencyGlyph(),
             (unsigned long)_expandedAnnualSalary);
    ui_native_text(307,78,119,2,empty?"-":line,1);
    snprintf(line,sizeof line,"SEASON %u OF %u",(unsigned)_currentSeasonNumber+1,
             (unsigned)_currentSeasonShowing+1);
    ui_native_text(218,92,176,15,line,0);
    rect(13,110,294,118,40);cb_box_outline(13,110,294,118,204);
    rect(15,112,290,14,42);
    ui_native_text(160,115,280,15,"SEASON PERFORMANCE",0);
    if(empty) {
        ui_native_text(160,153,275,2,"NO PREVIOUS CLUB RECORD",0);
        ui_native_text(160,169,275,15,"ACCEPT AN OFFER TO START",0);
    } else {
        const uint8_t *comps[]={si->leagueStrPtr,si->strPtr1,si->strPtr2,si->strPtr3};
        const int results[]={si->comp1Result,si->comp2Result,si->comp3Result,si->comp4Result};
        unsigned lines=0;
        for(unsigned i=0;i<4;i++) if(comps[i] && comps[i][0] && lines<4) {
            int y=130+(int)lines*11;
            if(lines&1) rect(16,y-1,288,10,42);
            ui_native_ellipsis(21,y,150,2,(const char*)comps[i],-1);
            if(si->managerStatus<0 && results[i]==-104)
                snprintf(line,sizeof line,"SACKED");
            else ui_record_status(line,sizeof line,results[i]);
            ui_native_text(300,y,120,(si->managerStatus<0 && results[i]==-104)?10:
                           results[i]==1?15:2,line,1);
            lines++;
        }
        if(!lines) ui_native_text(160,139,280,7,"COMPETITIONS NOT STARTED",0);
        rect(16,175,288,1,204);
        snprintf(line,sizeof line,"MATCHES %u   W %u   D %u   L %u",
                 (unsigned)si->totalGames,(unsigned)si->totalWins,
                 (unsigned)si->totalDrawn,(unsigned)si->totalLost);
        ui_native_text(160,181,280,2,line,0);
        if(si->bestScorerName[0]) {
            snprintf(line,sizeof line,"TOP SCORER %.18s - %u",
                     (const char*)si->bestScorerName,(unsigned)si->bestScorerNumGoals);
            ui_native_text(160,193,280,15,line,0);
        } else ui_native_text(160,193,280,7,"TOP SCORER -",0);
        snprintf(line,sizeof line,"CAREER TOTAL  %c%lu",Expanded_CurrencyGlyph(),
                 (unsigned long)_expandedCareerEarnings);
        ui_native_text(160,208,280,15,line,0);
        if(si->managerStatus<0) ui_native_text(160,218,280,10,"SACKED",0);
    }
    ui_button(1,"PREV");ui_button(2,"NEXT");ui_button(3,"EXIT");
}
/* SWOS 0.10.18: squad UI is a live view of the same PlayerFile objects
 * used by the original SquadAfterDrawCommon. No sample attributes from the
 * generated concept art, no JSON reread after the career has begun. */
extern uintptr_t _squadTeamFilePtr;
extern uint8_t _shortCountryNames[];
extern int32_t _newBalance;
extern const PlayerFile *Expanded_SquadRowPlayer(unsigned row);
extern unsigned Expanded_SquadRowFileIndex(unsigned row);
extern uint16_t _squadPrevSelectedPlayerIndex;
static ExpandedSquadSkills ui_player_skills(const PlayerFile *p) {
    ExpandedSquadSkills s={0};
    if(!p) return s;
    s.passing=(uint8_t)p->passing & 15u;
    s.velocity=((uint8_t)p->velocityHeading >> 4) & 15u;
    s.heading=(uint8_t)p->velocityHeading & 15u;
    s.tackling=((uint8_t)p->tacklingControl >> 4) & 15u;
    s.control=(uint8_t)p->tacklingControl & 15u;
    s.speed=((uint8_t)p->speedFinishing >> 4) & 15u;
    s.finishing=(uint8_t)p->speedFinishing & 15u;
    return s;
}
static void ui_star(int x,int y,uint8_t color) {
    static const uint8_t pixels[7]={0x04,0x15,0x0E,0x1F,0x0E,0x15,0x11};
    for(int dy=0;dy<7;dy++) for(int dx=0;dx<5;dx++)
        if(pixels[dy]&(1u<<(4-dx))) cb_plot(x+dx,y+dy,color);
}
static int s_squadStatsPopupOpen=0;
int Expanded_SquadPopupOpen(void) { return s_squadStatsPopupOpen; }
void Expanded_SquadPopupToggle(void) { s_squadStatsPopupOpen=!s_squadStatsPopupOpen; }
void Expanded_SquadPopupClose(void) { s_squadStatsPopupOpen=0; }
static void ui_squad_tooltip(unsigned row,const PlayerFile *p) {
    if(!p) return;
    ExpandedSquadSkills s=ui_player_skills(p);
    static const char *const names[7]={"PASSING","VELOCITY","HEADING","TACKLING",
                                      "CONTROL","SPEED","FINISHING"};
    unsigned values[7]={s.passing,s.velocity,s.heading,s.tackling,
                        s.control,s.speed,s.finishing};
    /* Two close columns (4+4), fully opaque right up to the outline.
     * Both keyboard/joypad focus and mouse hover use this same geometry. */
    const int x=EXP_SQUAD_TOOLTIP_X,w=EXP_SQUAD_TOOLTIP_W,
              h=EXP_SQUAD_TOOLTIP_H;
    const int y=Expanded_SquadTooltipY(row);
    rect(x,y,w,h,40); /* opaque border: no background visible through edges */
    cb_box_outline(x,y,w,h,204);
    rect(x+1,y+1,w-2,11,46);
    ui_native_ellipsis(x+4,y+4,w-8,15,(const char*)p->playerName,-1);
    char line[32];
    for(unsigned i=0;i<7;i++) {
        int col=(int)(i/4u), yy=y+16+(int)(i%4u)*9;
        int xx=x+4+col*82;
        if(i%4u & 1u) rect(xx-1,yy,w/2-2,9,42);
        ui_native_text(xx,yy,53,2,names[i],-1);
        snprintf(line,sizeof line,"%u",values[i]);
        ui_native_text(xx+57,yy,12,15,line,1);
        /* Display an estimated rounded change based on the same team-wide
         * multiplier used at match setup. Never mutate PlayerFile attributes.
         * Half points round away from zero, for both positive and negative. */
        int perf=(int)Expanded_TeamPerformancePct(_expandedCurrentForm,_expandedTeamMorale)-100;
        int delta=(int)((values[i]*(unsigned)(perf<0?-perf:perf)+50u)/100u);
        if(perf<0) delta=-delta;
        snprintf(line,sizeof line,"%+d",delta);
        ui_native_text(xx+61,yy,21,delta>0?3:(delta<0?10:2),line,-1);
    }
}
static void ui_squad_render(void) {
    cb_blit(0,0,CB_W,CB_H,ui_squad);
    /* Two independent layers: complete new stadium art below; SWOS-native
     * interactive UI frames, player data and popups drawn above each frame.
     * No generated lettering is baked into the playable UI. */
    rect(3,79,314,152,40);cb_box_outline(3,79,314,152,204);
    rect(5,81,310,12,46);
    for(int row=0;row<17;row++)
        rect(6,96+row*8,308,8,(row&1)?40:42);
    rect(3,234,314,20,40);cb_box_outline(3,234,314,20,204);
    TeamFileEx *team=(TeamFileEx*)(uintptr_t)_squadTeamFilePtr;
    if(!team) return;
    char line[96];
    rect(92,3,136,18,40);cb_box_outline(92,3,136,18,204);
    ui_native_text(160,8,128,15,"VIEW SQUAD",0);
    rect(94,30,132,17,40);cb_box_outline(94,30,132,17,204);
    ui_native_text(160,35,126,15,(const char*)team->team.teamName,0);
    unsigned country=team->team.countryNumber;
    if(country<96) {
        extern void *_countriesTable[];
        if(_countriesTable[country]) {
            snprintf(line,sizeof line,"(%.24s)",(const char*)_countriesTable[country]+1);
            /* Country shares the same baseline and height as FORM/MORALE. */
            rect(94,49,132,19,40);cb_box_outline(94,49,132,19,204);
            ui_native_text(160,55,128,15,line,0);
        }
    }
    rect(231,3,85,18,40);cb_box_outline(231,3,85,18,204);
    /* The actual balance and chosen game currency, never a fixed mockup. */
    int64_t bank=_newBalance;
    uint64_t absBank=(uint64_t)(bank<0?-bank:bank);
    if(absBank<1000000u)
        snprintf(line,sizeof line,"BANK %s%c%lu,%03lu",bank<0?"-":"",
                 Expanded_CurrencyGlyph(),(unsigned long)(absBank/1000u),
                 (unsigned long)(absBank%1000u));
    else snprintf(line,sizeof line,"BANK %s%c%lu.%01luM",bank<0?"-":"",
                  Expanded_CurrencyGlyph(),(unsigned long)(absBank/1000000u),
                  (unsigned long)((absBank/100000u)%10u));
    ui_native_text(273,8,79,15,line,0);
    /* Header: star column replaces the old top-skills string. */
    ui_native_text(11,81,15,15,"#",-1);
    ui_native_text(31,81,108,15,"NAME",-1);
    ui_native_text(141,81,22,15,"POS",-1);
    ui_native_text(166,81,33,15,_isSquadViewMode?"GOALS":"SKILL",-1);
    ui_native_text(200,81,30,15,"NAT",-1);
    ui_native_text(237,81,20,15,"G",-1);
    ui_native_text(265,81,45,15,"VALUE",-1);
    Menu *m=(Menu *)_currentMenu;
    unsigned selected=99;
    if(m && m->selectedEntry && m->selectedEntry->ordinal>=28 &&
       m->selectedEntry->ordinal<=44)
        selected=(unsigned)m->selectedEntry->ordinal-28u;
    const PlayerFile *selectedPlayer=NULL;
    static const char *const posNames[]={"G","RB","LB","D","RW","LW","M","A"};
    for(unsigned i=0;i<17;i++) {
        const PlayerFile *p=Expanded_SquadRowPlayer(i);
        if(!p) continue;
        int y=Expanded_SquadRowY(i);
        /* Pending swap is orange-red, even when the cursor moves away.
         * The focused row is blue with white SIDE markers only: no white
         * horizontal line is allowed to slice through player-name glyphs. */
        unsigned fileIndex=Expanded_SquadRowFileIndex(i);
        int swapping=_squadPrevSelectedPlayerIndex!=0xFFFFu &&
                     fileIndex==_squadPrevSelectedPlayerIndex;
        if(swapping) {
            rect(10,y-2,300,8,72);
        } else if(selected==i) {
            rect(10,y-2,300,8,11);
        } else if ((((unsigned)p->positionAndFace & PF_POSITION_MASK)>>PF_POSITION_SHIFT)==0) {
            rect(10,y-2,300,8,48); /* muted teal: goalkeeper */
        }
        if(((((unsigned)p->positionAndFace & PF_POSITION_MASK)>>PF_POSITION_SHIFT)==0)) {
            rect(11,y-1,3,6,15); /* keeper marker survives selected-row highlighting */
        }
        if(selected==i) selectedPlayer=p;
        snprintf(line,sizeof line,"%u",(unsigned)(uint8_t)p->shirtNumber);
        ui_native_text(24,y,17,2,line,1);
        ui_native_ellipsis(31,y,108,2,(const char*)p->playerName,-1);
        unsigned pos=((unsigned)p->positionAndFace & PF_POSITION_MASK)>>PF_POSITION_SHIFT;
        ui_native_text(145,y,20,2,posNames[pos<8?pos:0],-1);
        ExpandedSquadSkills skills=ui_player_skills(p);
        unsigned stars=ExpandedSquad_Stars(skills,pos,(uint8_t)p->price);
        if(_isSquadViewMode) {
            unsigned sum=0;
            for(unsigned j=0;j<4;j++) sum+=(unsigned)p->goalStats[j];
            snprintf(line,sizeof line,"%u",sum);
            ui_native_text(179,y,31,15,line,0);
        } else for(unsigned j=0;j<5;j++) ui_star(164+(int)j*6,y,j<stars?15:0);
        unsigned nat=(uint8_t)p->nationality;
        if(nat<96) {
            char shortNat[8];
            snprintf(shortNat,sizeof shortNat,"%.3s",(char*)_shortCountryNames+nat*3u);
            ui_native_text(201,y,30,2,shortNat,-1);
        }
        unsigned goals=(unsigned)p->goalStats[0]+p->goalStats[1]+
                       p->goalStats[2]+p->goalStats[3];
        snprintf(line,sizeof line,"%u",goals);
        ui_native_text(244,y,22,2,line,0);
        /* The original formatter applies loans, reserves, trialists and
         * market prices; no hand-written replacement money calculations. */
        uint8_t value[80]={0};
        FormatPlayerValueOrStatusText((uint8_t*)p-sizeof(TeamFile),value);
        ui_native_text(311,y,48,2,(const char*)value,1);
    }
    /* Team-wide form and morale, both percentage + shared five-face scale. */
    rect(5,49,84,19,40);cb_box_outline(5,49,84,19,204);
    snprintf(line,sizeof line,"FORM %u%%",(unsigned)_expandedCurrentForm);
    ui_native_text(37,55,65,15,line,0);
    cb_mood_face(80,58,_expandedCurrentForm);
    rect(231,49,85,19,40);cb_box_outline(231,49,85,19,204);
    snprintf(line,sizeof line,"MORALE %u%%",(unsigned)_expandedTeamMorale);
    ui_native_text(265,55,72,15,line,0);
    cb_mood_face(307,58,_expandedTeamMorale);
    if(team->team.coachName[0]) {
        snprintf(line,sizeof line,"COACH: %.22s",(const char*)team->team.coachName);
        ui_native_ellipsis(9,242,134,2,line,-1);
    }
    if(_isSquadViewMode) ui_button(11,"BACK");
    else if(!_menuEntries[8].isInvisible) ui_button(8,"GOALS");
    if(!_menuEntries[6].isInvisible) ui_button(6,"TRAINING");
    if(!_menuEntries[9].isInvisible) ui_button(9,"EXIT");
    if(!_menuEntries[10].isInvisible) ui_button(10,"EXIT");
    if(selectedPlayer && !_isSquadViewMode && s_squadStatsPopupOpen) ui_squad_tooltip(selected,selectedPlayer);
}
void Expanded_UI_After(void) {
    if(!ui_load()) return;
    int kind=ui_kind();
    if(kind!=8) s_squadStatsPopupOpen=0;
    if(kind==8) {
        ui_squad_render();
    } else if(kind==7) {
        ui_record_render();
    } else if(kind==5) {
        /* Exact first approved welcome composition: full-bleed arrival
         * illustration, central message, side-by-side live approval boxes,
         * and a single centred CONTINUE button. */
        cb_blit(0,0,CB_W,CB_H,ui_welcome);
        /* Crisp original SWOS font, same approved title rectangle. */
        rect(81,4,158,18,40);
        cb_box_outline(81,4,158,18,204);
        ui_native_text(160,9,148,2,"WELCOME TO THE CLUB",0);
        rect(23,156,274,67,40);
        cb_box_outline(21,154,278,70,204);
        char line[96],name[28],club[40];
        snprintf(name,sizeof name,"%.22s",(const char *)_managerSurnameBuffer);
        /* The actual managed club, never the example club in the artwork. */
        /* TeamFileEx starts with TeamFile; use the typed name accessor. */
        extern const char *Expanded_CurrentClubName(void);
        snprintf(club,sizeof club,"%.28s",Expanded_CurrentClubName());
        snprintf(line,sizeof line,"DEAR MR %.19s,",name);
        ui_native_text(160,160,260,6,line,0);
        ui_native_text(160,172,260,15,"WE ARE DELIGHTED TO WELCOME YOU AS",0);
        snprintf(line,sizeof line,"THE NEW MANAGER OF %.25s",club);
        ui_native_text(160,182,260,15,line,0);
        ui_native_text(160,192,260,15,"WE TRUST YOU WILL LEAD US TO SUCCESS.",0);
        rect(28,203,124,16,42);cb_box_outline(28,203,124,16,204);
        rect(168,203,124,16,42);cb_box_outline(168,203,124,16,204);
        snprintf(line,sizeof line,"FANS LIKING %u%%",(unsigned)_expandedFanMood);
        ui_native_text(90,207,116,2,line,0);
        snprintf(line,sizeof line,"BOARD LIKING %u%%",(unsigned)_expandedBoardMood);
        ui_native_text(230,207,116,2,line,0);
        ui_button(6,"CONTINUE");
    } else if(kind==6) {
        /* Full-screen dismissal: no previous career/unemployed frame visible
         * behind the art, including while the menu is dismissed. */
        cb_blit(0,0,CB_W,CB_H,ui_sacked);
        ui_button(0,"RETURN UNEMPLOYED");
    } else if(kind==4) {
        /* The *first* office/contract concept now covers ALL 320x256 pixels,
         * including the desk, windows and background behind the buttons.
         * Repaint its baked-in sample values with live native-font data.
         * Preserve its top title, window, chairman, contract, crest, trophy
         * and table composition. All seven actions remain real MenuEntries. */
        cb_blit(0,0,CB_W,CB_H,ui_offer);
        rect(81,3,158,18,40);
        cb_box_outline(81,3,158,18,204);
        ui_native_text(160,8,148,2,"CLUB OFFER",0);
        rect(81,21,158,10,40);
        ui_native_text(160,22,150,15,"A CLUB WANTS TO HIRE YOU",0);
        /* Same compact top-center information card as the first concept,
         * enlarged slightly to accommodate actual long club names. */
        rect(81,31,158,53,40);
        rect(83,33,154,11,40);
        rect(83,45,154,11,42);
        rect(83,57,154,11,40);
        rect(83,69,154,12,42);
        cb_box_outline(81,31,158,53,204);
        rect(83,44,154,1,191);
        rect(83,56,154,1,191);
        rect(83,68,154,1,191);
        ui_native_text(89,35,54,15,"CLUB:",-1);
        ui_native_text(148,35,84,2,Expanded_OfferClub(),-1);
        ui_native_text(89,47,54,15,"COUNTRY:",-1);
        ui_native_text(148,47,84,2,Expanded_OfferCountry(),-1);
        ui_native_text(89,59,54,6,"REGION:",-1);
        ui_native_text(148,59,84,2,Expanded_OfferRegion(),-1);
        ui_native_text(89,71,54,15,"DIVISION:",-1);
        char info[96],base[32],ask[32];
        snprintf(info,sizeof info,"%u",Expanded_OfferDivision());
        ui_native_text(148,71,84,2,info,-1);
        /* A rejection is visible without hiding the original club offer. */
        if(Expanded_OfferStatus()[0]) {
            rect(69,136,182,11,40);
            cb_box_outline(69,136,182,11,204);
            ui_native_text(160,138,174,6,Expanded_OfferStatus(),0);
        }
        ui_offer_annual(base,sizeof base,Expanded_OfferBaseWeekly());
        ui_offer_annual(ask,sizeof ask,Expanded_OfferDemandWeekly());
        /* Match the original screenshot's salary card and +/- selector,
         * but use yearly figures. No fixed EUR/GBP label is baked in. */
        rect(56,148,206,39,40);
        cb_box_outline(56,148,206,39,204);
        rect(58,162,202,1,6);
        snprintf(info,sizeof info,"SALARY OFFER: %s / YEAR",base);
        ui_native_text(160,152,194,2,info,0);
        rect(112,166,96,17,14);
        cb_box_outline(112,166,96,17,204);
        ui_native_text(160,170,88,2,ask,0);
        /* Buttons follow the FIRST concept's two-column, two-row layout,
         * with the BACK button centered at the bottom. Native focus frames
         * and onSelect callbacks are unchanged, including keyboard +/-.
         */
        ui_button(6,"-"); ui_button(7,"+");
        ui_button(1,"VIEW SQUAD");
        ui_button(2,"VIEW COMPETITIONS");
        ui_button(3,"ACCEPT"); ui_button(4,"DECLINE");
        ui_button(5,"BACK");
    } else if(kind==1) {
        Menu *m=(Menu *)_currentMenu;
        if(!m) return;
        /* The stock menu draws an old masked panel and highlight shadow.
         * Repaint the whole approved artwork after the native menu pass,
         * then paint our actual clickable entries on top. */
        cb_blit(0,0,CB_W,CB_H,ui_title);
        for(unsigned i=1;i<=16 && i<m->numEntries;i++) {
            MenuEntry *e=&_menuEntries[i];
            if(e->isInvisible) continue;
            const char *name=ui_main_labels[i-1];
            /* Continue/Replay and Save are still conditional, not fake buttons. */
            if((i==11 || i==12) && e->fg &&
               e->fg!=(uintptr_t)0xFFFFFFFFu)
                name=(const char *)(uintptr_t)e->fg;
            ui_button(i,name);
        }
        ui_native_text(43,246,110,15,Build_DisplayVersion(),0);
    } else if(kind==2) {
        Menu *m=(Menu *)_currentMenu;
        if(!m) return;
        /* Repaint approved art to remove stock selection/font ghosts.
         * The InputText edit buffer is read below, including the caret. */
        cb_blit(0,0,CB_W,CB_H,ui_career);
        /* Compact form over undistorted manager-arrival illustration. */
        ui_native_text(261,110,108,2,"NEW CAREER",0);
        static const char *const fields[]={
            "TITLE","NAME","SURNAME","NATION","ROLE","AGE (18-70)","FAME"};
        static const unsigned ord[]={3,4,5,7,9,15,13};
        const char *values[]={
            (const char *)_managerSexBuffer,
            (_menuEntries[4].fg && _menuEntries[4].fg!=(uintptr_t)0xFFFFFFFFu)?
                (const char *)(uintptr_t)_menuEntries[4].fg:(const char *)_managerNameBuffer,
            (_menuEntries[5].fg && _menuEntries[5].fg!=(uintptr_t)0xFFFFFFFFu)?
                (const char *)(uintptr_t)_menuEntries[5].fg:(const char *)_managerSurnameBuffer,
            (_menuEntries[7].fg && _menuEntries[7].fg!=(uintptr_t)0xFFFFFFFFu)?
                (const char *)(uintptr_t)_menuEntries[7].fg:"---",
            _coachOrPlayer?"COACH":"PL/COACH",_expandedAgeText,_expandedManagerFameText
        };
        for(unsigned i=0;i<7;i++) {
            MenuEntry *e=&_menuEntries[ord[i]];
            ui_restore_rect(ui_career,e->x,e->y,e->dwWidth,e->dwHeight);
            /* Native SWOS glyphs sit a little above their nominal sprite box.
             * Use a 3px top inset in the 12px fields to visually center
             * both labels and editable values without altering the font. */
            int y=e->y+3;
            static const int labelWidth[]={38,34,49,44,34,84,32};
            static const int valueWidth[]={66,74,58,66,74,20,78};
            ui_native_text(207,y,labelWidth[i],2,fields[i],-1);
            ui_native_text(314,y,valueWidth[i],15,values[i]?values[i]:"",1);
            if(m->selectedEntry==e) ui_focus(e->x,e->y,e->dwWidth,e->dwHeight);
        }
        ui_button(10,_expandedManagerFameText[0]=='U'?"START UNEMPLOYED":"CHOOSE TEAM");
        ui_button(11,"BACK");
        ui_button(1,"LOAD LIVERPOOL");
    } else if(kind==3) {
        if(_currentMenuPtr==(uintptr_t)_contestMenu) ui_fixture_finish_panel();
        int unemployed=_currentMenuPtr==(uintptr_t)Expanded_UnemployedCalendarPointer();
        /* Fixture rows are now rendered once per entry by the compact
         * calendar renderer, with bounded columns and no stock overpaint. */
        static const char *const defaultLabels[12]={
            "TRANSFERS","JOB OFFERS","CLUB BUSINESS","PLAY MATCH",
            "SQUAD","EXIT","NEXT MATCH","COMPETITIONS","VIEW WORLD",
            "NEXT MATCH","COMPETITIONS","EXIT"};
        for(unsigned i=4;i<=15;i++) {
            MenuEntry *e=&_menuEntries[i];
            if(e->isInvisible || (unemployed && i!=7 && i!=9 && i!=10 && i!=12)) continue;
            const char *name=defaultLabels[i-4];
            if(unemployed && i==7)
                name="FORWARD TIME";
            else if(unemployed && i==12)
                name="RECORD";
            /* The live game can change captions without altering the callback. */
            else if(e->fg && e->fg!=(uintptr_t)0xFFFFFFFFu)
                name=(const char *)(uintptr_t)e->fg;
            /* The full native text is too long for a 90px pixel-art key.
             * Only shorten its VISUAL label, not the original onSelect. */
            if(!unemployed && (i==11 || i==14)) name="COMPETITIONS";
            ui_button(i,name);
        }
        char date[48];
        if(_currentMenuPtr==(uintptr_t)Expanded_UnemployedCalendarPointer()) {
            extern void Expanded_UnemployedDate(char *,size_t);
            Expanded_UnemployedDate(date,sizeof date);
        } else
            Expanded_FormatDate(date,sizeof date,1996u+(unsigned)_currentSeasonShowing,
                                (unsigned)_lastFinancialWeekIndex);
        /* Native font, inside the top-right 126px date textbox, away from
         * all fixture rows (which begin below y=39). */
        ui_native_text(252,24,118,15,date,0);
    }
}


/* Called after an original SWOS partial DrawMenuItem. Without this hook,
 * LEFT/RIGHT nationality/fame changes and moving the focus would leave
 * stale pixel-art text or a ghost selection frame until a full redraw. */
void Expanded_UI_Partial(MenuEntry *entry) {
    (void)entry;
    if(!ui_load()) return;
    int kind=ui_kind();
    if(kind==1 || kind==2 || kind==4 || kind==5 || kind==6 || kind==7 || kind==8) {
        Expanded_UI_Before();
        Expanded_UI_After();
    } else if(kind==3) {
        const uint8_t *art=_currentMenuPtr==(uintptr_t)Expanded_UnemployedCalendarPointer()?ui_unemployed:ui_calendar;
        ui_restore_rect(art,5,191,310,65);
        ui_restore_rect(art,190,20,127,15);
        Expanded_UI_After();
    }
}

#else
int Expanded_UI_Active(void) {return 0;}
void Expanded_UI_Prepare(void) {}
void Expanded_UI_Before(void) {}
void Expanded_UI_After(void) {}
void Expanded_UI_Partial(MenuEntry *entry) {(void)entry;}
#endif

/* Squad popup input: only active while the Expanded squad screen is open. */
int Expanded_UI_IsSquadScreen(void) { return ui_kind()==8; }
/* Business WAGES and MAINTENANCE details: immediate X toggle, like SQUAD. */
int Expanded_UI_IsBusinessScreen(void) {
    return _currentMenuPtr==(uintptr_t)_expandedClubOverviewMenu;
}
void Expanded_BusinessPopupToggle(void) {
    if(!Expanded_UI_IsBusinessScreen()) return;
    Menu *menu=(Menu *)_currentMenu;
    unsigned focused=menu && menu->selectedEntry ?
                     menu->selectedEntry->ordinal : 0u;
    /* Only an explicitly selected finance row responds to X. The other
     * popup closes as focus changes, and neither opens just by moving. */
    if(focused==12) {
        _expandedBusinessMaintenancePopupMode=2;
        _expandedBusinessStaffPopupMode=
            _expandedBusinessStaffPopupMode==1?2:1;
    } else if(focused==11) {
        _expandedBusinessStaffPopupMode=2;
        _expandedBusinessMaintenancePopupMode=
            _expandedBusinessMaintenancePopupMode==1?2:1;
    } else return;
    /* Repaint on this controller or keyboard X event, not a later tick. */
    extern void DrawMenu(void);
    DrawMenu();
}

