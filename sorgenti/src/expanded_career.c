/* SWOS Expanded 0.3 — optional career profile, deliberately isolated from
 * gameplay, Amiga audio and SWOS physics. Windows/Linux native builds only.
 * OpenSWOS remains copyright Ben Baker and contributors (GPL).
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "menu_system.h"
#include "game_types.h"
#include "world.h"
#include "expanded_world_time.h"
#include "expanded_debug.h"
#include "expanded_world_live.h"
#include "swos_io.h"
#include "career_params.h"
#include "string_utils.h"
extern void Currency_Set(const char *name);

static int s_waitAccepted=0;
static int s_unemployedHadClub=0;
static int s_unemployedRecordOpen=0;
uint8_t _expandedManagerStartAge = 35;
uint32_t _expandedAnnualSalary = 30000;
uint32_t _expandedCareerEarnings = 0;
uint16_t _expandedPaidSeasons = 0;
uint8_t _expandedHallRecorded = 0;
char _expandedAgeCaption[] = "AGE (18-70)";
char _expandedAgeText[12] = "35";
char _expandedHallCaption[] = "HALL OF FAME";
char _expandedHallLines[5][64] = {{0}};

extern uint8_t _expandedManagerFame;
extern uint16_t _currentSeasonShowing;
extern uint16_t _winCupBonusCount;
extern uint16_t _minorTrophyBonusCount;
extern uint16_t _majorTrophyBonusCount;
extern uint8_t _managerNameBuffer[];
extern uint8_t _managerSurnameBuffer[];
extern int32_t _jobOfferFunds;
extern uint8_t _expandedHallMenu[];

static uint8_t s_currency = 0; /* 0=GBP, 1=EUR, 2=USD */
char _expandedCurrencyLabel[] = "CURRENCY";
char _expandedCurrencyText[16] = "POUNDS";
static const char *const s_currencyNames[] = {"POUNDS", "EUROS", "DOLLARS"};
static const char *const s_currencyCodes[] = {"pound", "euro", "dollar"};
unsigned char Expanded_CurrencyGlyph(void) {
    return s_currency == 1 ? 0x9D : (s_currency == 2 ? '$' : 0x9C);
}
static void currency_path(char *out, size_t cap) {
    snprintf(out, cap, "%s/Expanded_Currency.txt", swos_user_dir());
}
static void currency_apply(void) {
    strcpy(_expandedCurrencyText, s_currencyNames[s_currency]);
    Currency_Set(s_currencyCodes[s_currency]);
}
void Expanded_CurrencyLoad(void) {
    char path[1200], buf[24] = {0};
    currency_path(path, sizeof path);
    FILE *f = fopen(path, "r");
    if (f) { if (fgets(buf, sizeof buf, f)) {
        if (!strncmp(buf, "euro", 4)) s_currency = 1;
        else if (!strncmp(buf, "dollar", 6)) s_currency = 2;
    } fclose(f); }
    currency_apply();
}
void Expanded_CurrencyMenuInit(void) { currency_apply(); }
void Expanded_CurrencyChange(MenuEntry *entry) {
    s_currency = (uint8_t)((s_currency + 1) % 3);
    currency_apply();
    char path[1200]; currency_path(path, sizeof path);
    FILE *f = fopen(path, "w");
    if (f) { fprintf(f, "%s\n", s_currencyCodes[s_currency]); fclose(f); }
    else fprintf(stderr, "[expanded04] cannot save currency %s\n", path);
    CallDrawMenuItem(entry);
}

void Expanded_SetAgeText(void) {
    snprintf(_expandedAgeText, sizeof _expandedAgeText, "%u",
             (unsigned)_expandedManagerStartAge);
}

void Expanded_StartCareer(void) {
    extern void Expanded_EconomyReset(void);
    extern uint8_t _expandedContractPremiumPct, _expandedSuccessfulRaises;
    uint8_t premium=_expandedContractPremiumPct,raises=_expandedSuccessfulRaises;
    Expanded_EconomyReset();
    if(s_waitAccepted) { /* pending signed offer, saved before InitCareer */
        _expandedContractPremiumPct=premium;
        _expandedSuccessfulRaises=raises;
    }
    _careerParams.num_seasons = (uint16_t)(85u - _expandedManagerStartAge);
    _expandedAnnualSalary = 30000u + (uint32_t)_expandedManagerFame * 5000u;
    _expandedCareerEarnings = 0;
    _expandedPaidSeasons = 0;
    _expandedHallRecorded = 0;
    Expanded_SetAgeText();
    fprintf(stderr,"[expanded03] career started: age=%u fame=%u salary=%lu\n",
            (unsigned)_expandedManagerStartAge,(unsigned)_expandedManagerFame,
            (unsigned long)_expandedAnnualSalary);
}

/* The original JobOffer.amountOffered is CLUB TRANSFER FUNDS, not a wage.
 * We derive a separate fixed annual salary from that budget. */
uint32_t Expanded_ComputeSalary(uint32_t clubFunds) {
    uint32_t budgetPart = clubFunds == UINT32_MAX ? 30000u : clubFunds / 500u;
    if (budgetPart > 250000u) budgetPart = 250000u;
    uint32_t result = 20000u + budgetPart + (uint32_t)_expandedManagerFame * 5000u;
    return result > 350000u ? 350000u : result;
}
void Expanded_AcceptSalary(uint32_t clubFunds) {
    _expandedAnnualSalary = Expanded_ComputeSalary(clubFunds);
    fprintf(stderr,"[expanded03] accepted job salary GBP %lu/year\n",
            (unsigned long)_expandedAnnualSalary);
}
const char *Expanded_OfferSalaryCaption(void) {
    static char b[64];
    snprintf(b,sizeof b,"SALARY %c%lu / YEAR", Expanded_CurrencyGlyph(),
             (unsigned long)Expanded_ComputeSalary((uint32_t)_jobOfferFunds));
    return b;
}
const char *Expanded_CareerProfileCaption(void) {
    static char b[80];
    unsigned age = (unsigned)_expandedManagerStartAge + (unsigned)_currentSeasonShowing;
    snprintf(b,sizeof b,"AGE %u  SALARY %c%lu/YEAR", age,
             Expanded_CurrencyGlyph(), (unsigned long)_expandedAnnualSalary);
    return b;
}
void Expanded_CompleteSeason(void) {
    /* Career Continue is the season-completion gate. Guard against double
     * payment if the same menu is revisited before the season counter moves. */
    uint16_t finished = (uint16_t)(_currentSeasonShowing + 1);
    if (_expandedPaidSeasons >= finished) return;
    uint32_t addition = _expandedAnnualSalary;
    if (UINT32_MAX - _expandedCareerEarnings < addition)
        _expandedCareerEarnings = UINT32_MAX;
    else
        _expandedCareerEarnings += addition;
    _expandedPaidSeasons = finished;
    fprintf(stderr,"[expanded03] season %u salary %lu earnings %lu\n",
            (unsigned)finished,(unsigned long)addition,
            (unsigned long)_expandedCareerEarnings);
}
int Expanded_ShouldRetire(void) {
    return (unsigned)_expandedManagerStartAge + (unsigned)_currentSeasonShowing + 1u >= 85u;
}

static void hall_path(char *buf,size_t n) {
    snprintf(buf,n,"%s/HallOfFame_Expanded.tsv",swos_user_dir());
}
static void sanitize(char *dest,size_t n,const uint8_t *src) {
    size_t j=0;
    for(size_t i=0;src && i<20 && src[i] && j+1<n;i++) {
        unsigned char c=src[i];
        if(c=='\t'||c=='\r'||c=='\n') c=' ';
        if(c<32||c>126) c='?';
        dest[j++]=(char)c;
    }
    dest[j]=0;
}
void Expanded_RecordRetirement(void) {
    if (_expandedHallRecorded) return;
    char path[1200],first[32],last[32];
    hall_path(path,sizeof path);
    sanitize(first,sizeof first,_managerNameBuffer);
    sanitize(last,sizeof last,_managerSurnameBuffer);
    FILE *f=fopen(path,"a");
    if(!f) {
        fprintf(stderr,"[expanded03] cannot open HallOfFame: %s\n",path);
        return;
    }
    unsigned trophies=(unsigned)_winCupBonusCount +
        (unsigned)_minorTrophyBonusCount + (unsigned)_majorTrophyBonusCount;
    fprintf(f,"%s %s\t%u\t%u\t%lu\t%u\n",first,last,
            (unsigned)_expandedManagerFame,
            (unsigned)_expandedManagerStartAge+(unsigned)_expandedPaidSeasons,
            (unsigned long)_expandedCareerEarnings,trophies);
    fclose(f);
    _expandedHallRecorded=1;
    fprintf(stderr,"[expanded03] hall recorded: %s %s earnings=%lu trophies=%u\n",
            first,last,(unsigned long)_expandedCareerEarnings,trophies);
}

typedef struct HallRecord {
    char name[48];
    unsigned age,trophies;
    uint32_t earnings;
} HallRecord;

void Expanded_HallInit(void) {
    HallRecord top[5]={{0}};
    for(int i=0;i<5;i++) strcpy(_expandedHallLines[i],"--");
    char path[1200],line[256];
    hall_path(path,sizeof path);
    FILE *f=fopen(path,"r");
    if(!f) {
        strcpy(_expandedHallLines[0],"NO COMPLETED CAREERS");
        return;
    }
    while(fgets(line,sizeof line,f)) {
        HallRecord cur={{0}};
        unsigned fame=255;
        unsigned long earnings=0;
        if(sscanf(line,"%47[^\t]\t%u\t%u\t%lu\t%u",
                  cur.name,&fame,&cur.age,&earnings,&cur.trophies)!=5 || fame!=0)
            continue; /* Hall only ranks careers begun at fame zero. */
        cur.earnings=(uint32_t)earnings;
        for(int i=0;i<5;i++) {
            if(cur.earnings>top[i].earnings ||
               (cur.earnings==top[i].earnings && cur.trophies>top[i].trophies)) {
                for(int j=4;j>i;j--) top[j]=top[j-1];
                top[i]=cur;break;
            }
        }
    }
    fclose(f);
    for(int i=0;i<5;i++) if(top[i].name[0])
        snprintf(_expandedHallLines[i],sizeof _expandedHallLines[i],
                 "%d %.17s %c%lu T %u",i+1,top[i].name,
                 Expanded_CurrencyGlyph(), (unsigned long)top[i].earnings,top[i].trophies);
}
void Expanded_ShowHall(void) {
    ShowMenu(_expandedHallMenu);
}

/* Expanded 0.10: the unemployed screen uses a cloned copy of the original
 * packed contest menu. No fake custom buttons or alternate panel geometry. */
#include <time.h>
extern uint8_t *Expanded_UnemployedCalendarMenu(void);
extern uint8_t _expandedClubOfferMenu[];
extern uint8_t _expandedOfferPreviewMenu[];
extern uint8_t _teamFileBuffer[];
extern uint16_t _teamsLoaded;
extern int16_t LoadTeamFile(uint16_t extNumber);
extern void DrawMenu(void);
extern void SetExitMenuFlag(void);
extern uint16_t DrawMenuTextCentered(uint16_t,uint16_t,uint16_t,uint8_t *,uint8_t *);
extern void *_charTables[];
extern void *_countriesTable[262];
extern MenuEntry _menuEntries[];
extern uint8_t _currentMenu[];
extern uint16_t _fireResetFlag;
extern int32_t Team_IsOutsideLeague(const TeamFile *team);
extern void Expanded_FormatDate(char *,size_t,unsigned,unsigned);
extern void Expanded_FormatCivilDate(char *,size_t,ExpandedWorldDate);
static unsigned s_waitWeeks=0;
static unsigned s_nextOfferWeek=0;
/* Test-only F12+7: consumed on the NEXT genuine FORWARD TIME menu action.
 * Never run DrawMenu or native OG callbacks from SDL's event pump. */
static unsigned s_debug_wait_jump_weeks=0;
void Expanded_DebugArmFiveYearUnemployment(void) {
    extern uintptr_t _currentMenuPtr;
    if(_currentMenuPtr!=(uintptr_t)Expanded_UnemployedCalendarMenu()) {
        fprintf(stderr,"[debug-unemployed] F12+7 requires UNEMPLOYED CALENDAR\n");
        return;
    }
    s_debug_wait_jump_weeks=260u;
    fprintf(stderr,"[debug-unemployed] armed +260 weeks: press FORWARD TIME "
                   "once; OG competitions are NOT yet verified offscreen\n");
}

static uint16_t s_waitTeam=0;
static uint8_t s_waitCountry=8;
static uint8_t s_waitLeague=0;
static char s_waitTeamName[40]="";
static char s_waitCountryName[40]="";
static TeamFileEx s_offerSnapshot;
static int s_offerSnapshotValid=0;
static char s_previewPlayers[16][28];
static char s_previewTeams[16][24];
static unsigned s_previewCount=0;
static unsigned s_previewMode=0; /* 1=squad, 2=competitions */
static char s_waitRegionName[24]="EUROPE";
static uint32_t s_offerBaseWeekly=0,s_offerDemandWeekly=0,s_offerAgreedWeekly=0;
static uint32_t s_offerStep=100;
static unsigned s_offerRejections=0;
static char s_offerStatus[64]="";
extern uint8_t _expandedManagerFame;
extern QualifyingGroup _europeTable,_africaTable,_southAmericaTable,
    _northAmericaTable,_asiaTable,_oceaniaTable;
const char *Expanded_RegionForCountry(uint8_t country) {
    const QualifyingGroup *groups[]={&_europeTable,&_africaTable,&_southAmericaTable,
        &_northAmericaTable,&_asiaTable,&_oceaniaTable};
    static const char *names[]={"EUROPE","AFRICA","SOUTH AMERICA",
        "NORTH AMERICA","ASIA","OCEANIA"};
    for(unsigned g=0;g<6;g++) {
        const uint8_t *countries=World_ContinentCountries((uint8_t)(80+g));
        if(!countries) countries=groups[g]->countries;
        for(unsigned i=0;i<44 && countries[i]!=255;i++)
            if(countries[i]==country) return names[g];
    }
    return "WORLD";
}
/* Weekly offer uses the real club's average player value, division and the
 * manager's reputation. It is a separate wage, NEVER the transfer budget. */
static void offer_reset_terms(void) {
    unsigned total=0;
    for(unsigned i=0;i<16;i++) total+=(uint8_t)s_offerSnapshot.players[i].price;
    unsigned quality=total/16;
    unsigned leagueBonus=s_waitLeague<5 ? (5-s_waitLeague)*350u : 0;
    uint32_t weekly=250u+quality*20u+leagueBonus+(unsigned)_expandedManagerFame*150u;
    if(weekly<250) weekly=250;
    if(weekly>12000) weekly=12000;
    weekly=((weekly+25u)/50u)*50u;
    s_offerBaseWeekly=s_offerDemandWeekly=weekly;
    s_offerStep=(weekly/10u+25u)/50u*50u;
    if(s_offerStep<50u) s_offerStep=50u;
    s_offerRejections=0;
    s_offerStatus[0]=0;
}
uint32_t Expanded_OfferBaseWeekly(void) {return s_offerBaseWeekly;}
uint32_t Expanded_OfferDemandWeekly(void) {return s_offerDemandWeekly;}
unsigned Expanded_OfferRejections(void) {return s_offerRejections;}
const char *Expanded_OfferRegion(void) {return s_waitRegionName;}
const char *Expanded_OfferClub(void) {return s_waitTeamName;}
const char *Expanded_OfferCountry(void) {return s_waitCountryName;}
unsigned Expanded_OfferDivision(void) {return (unsigned)s_waitLeague+1;}
const char *Expanded_OfferStatus(void) {return s_offerStatus;}
int Expanded_UnemployedOfferAccepted(void) {return s_waitAccepted;}
void Expanded_CommitNegotiatedSalary(void) {
    if(s_waitAccepted && s_offerAgreedWeekly) {
        _expandedAnnualSalary=s_offerAgreedWeekly*52u;
        fprintf(stderr,"[expanded14] negotiated salary %lu/year\n",
                (unsigned long)_expandedAnnualSalary);
    }
}

static unsigned s_randomState=1;
static unsigned next_random(void) {
    s_randomState ^= s_randomState << 13;
    s_randomState ^= s_randomState >> 17;
    s_randomState ^= s_randomState << 5;
    return s_randomState;
}
static unsigned next_delay(void) {return 1u + next_random()%5u;}
/* Do not reload team files from a modal menu callback. LoadTeamFile mutates
 * shared career/team buffers; the offer and its previews must be snapshots.
 * In particular, league=4 often means NON-LEAGUE, not fifth division: such a
 * club crashes InitializeNewSeason when used as a career team. */
static int offer_is_valid(void) {
    const TeamFile *t=&s_offerSnapshot.team;
    return s_offerSnapshotValid && s_waitTeam &&
           t->countryNumber==s_waitCountry &&
           t->teamOrdinal==(uint8_t)s_waitTeam &&
           ((s_waitTeam>>8)==s_waitCountry) &&
           t->league==s_waitLeague &&
           !Team_IsOutsideLeague(t);
}
/* Revalidate against the actual file only AFTER the modal stack has returned.
 * The source file can have changed since an offer was displayed. */
static int verify_offer_in_file(void) {
    if(!offer_is_valid() || LoadTeamFile(s_waitCountry)!=0) return 0;
    unsigned count=*(const uint16_t*)_teamFileBuffer;
    if(!count || count>256) return 0;
    const TeamFileEx *teams=(const TeamFileEx*)(_teamFileBuffer+2);
    for(unsigned i=0;i<count;i++) {
        const TeamFile *t=&teams[i].team;
        if(t->countryNumber==s_waitCountry &&
           t->teamOrdinal==(uint8_t)s_waitTeam &&
           t->league==s_waitLeague && !Team_IsOutsideLeague(t))
            return 1;
    }
    return 0;
}
static void cache_offer_roster(void) {
    for(unsigned i=0;i<16;i++)
        snprintf(s_previewPlayers[i],sizeof s_previewPlayers[i],"%.22s",
                 (const char*)s_offerSnapshot.players[i].playerName);
}
static void cache_offer_league(const TeamFileEx *teams,unsigned count) {
    s_previewCount=0;
    for(unsigned i=0;i<count && s_previewCount<16;i++) {
        const TeamFile *t=&teams[i].team;
        if(t->countryNumber!=s_waitCountry || t->league!=s_waitLeague ||
           Team_IsOutsideLeague(t)) continue;
        snprintf(s_previewTeams[s_previewCount++],sizeof s_previewTeams[0],
                 "%.17s",(const char*)t->teamName);
    }
}
/* After matches have been played, a mid-season vacancy is plausible
 * chiefly at a struggling club. The pilot standings are provisional:
 * this is an offer-selection guard, NOT a second competition engine.
 * Before league play, or without a valid pilot table, preserve the
 * existing small-club fallback rather than inventing standings. */
static int offer_candidate_struggling(uint16_t club,
                                     const ExpandedLiveRow *rows,
                                     uint32_t count,int ranked) {
    if (!ranked) return 1;
    for (uint32_t i=0;i<count;i++)
        if (rows[i].club==club)
            return i>=count/2u;
    return 0; /* never offer a missing club on ranked data */
}
static void choose_small_club(void) {
    uint8_t files[256];
    const uint32_t numFiles=ExpandedWorldLive_OfferFiles(files,256u);
    if (!numFiles) {
        fprintf(stderr,"[offers] no scheduled country file in Living World\n");
        s_waitTeam=0;s_offerSnapshotValid=0;
        return;
    }
    unsigned start=next_random()%numFiles;
    s_waitTeam=0;
    s_offerSnapshotValid=0;
    for(uint32_t attempt=0;attempt<numFiles;attempt++) {
        uint8_t country=files[(start+attempt)%numFiles];
        const uint8_t worldCountry=World_FileCountry(country);
        _teamsLoaded=0;
        if(LoadTeamFile(country)!=0) continue;
        unsigned count=*(const uint16_t*)_teamFileBuffer;
        if(!count || count>256) continue;
        const TeamFileEx *teams=(const TeamFileEx*)(_teamFileBuffer+2);
        unsigned maxLeague=0,candidates=0,divisionCount=0;
        ExpandedLiveRow standings[26];
        uint32_t rankedCount=0;
        /* Count actual scheduled divisions in the canonical WORLD, not
         * just entries in this TEAM file: a country can span TWO files.
         * The highest populated league index is its lowest tier, e.g.
         * division 8 after the database grows to eight divisions. */
        for(unsigned d=0;d<16u;++d) {
            uint32_t n=0;
            if(ExpandedWorldLive_Table(worldCountry,(uint8_t)d,
                                       standings,26u,&n) && n>=2u) {
                ++divisionCount;
                maxLeague=d;
            }
        }
        if(divisionCount<4u) continue;
        int ranked=ExpandedWorldLive_Table(worldCountry,(uint8_t)maxLeague,
                     standings,26u,&rankedCount) &&
                   rankedCount>=2u && standings[0].played>=3u;
        if(!ranked) continue; /* no offer from an unverified table */
        for(unsigned i=0;i<count;i++)
            if(teams[i].team.countryNumber==country && teams[i].team.league==maxLeague &&
               !Team_IsOutsideLeague(&teams[i].team) &&
               offer_candidate_struggling(
                    (uint16_t)((country<<8)|teams[i].team.teamOrdinal),
                    standings,rankedCount,ranked))
                candidates++;
        if(!candidates) continue;
        unsigned selected=next_random()%candidates;
        for(unsigned i=0;i<count;i++) {
            const TeamFile *t=&teams[i].team;
            if(t->countryNumber!=country || t->league!=maxLeague ||
               Team_IsOutsideLeague(t) ||
               !offer_candidate_struggling(
                    (uint16_t)((country<<8)|t->teamOrdinal),
                    standings,rankedCount,ranked)) continue;
            if(selected--==0) {
                s_waitTeam=(uint16_t)((country<<8)|t->teamOrdinal);
                s_waitCountry=country;
                s_waitLeague=(uint8_t)maxLeague;
                snprintf(s_waitTeamName,sizeof s_waitTeamName,"%.17s",(const char*)t->teamName);
                if(_countriesTable[country])
                    snprintf(s_waitCountryName,sizeof s_waitCountryName,"%.28s",
                             (const char*)_countriesTable[country]+1);
                else snprintf(s_waitCountryName,sizeof s_waitCountryName,"COUNTRY %u",country);
                memcpy(&s_offerSnapshot,&teams[i],sizeof s_offerSnapshot);
                s_offerSnapshotValid=1;
                cache_offer_roster();
                cache_offer_league(teams,count);
                snprintf(s_waitRegionName,sizeof s_waitRegionName,"%s",Expanded_RegionForCountry(worldCountry));
                offer_reset_terms();
                fprintf(stderr,"[expanded11] eligible offer team=%04x division=%u country=%u\n",
                        (unsigned)s_waitTeam,(unsigned)s_waitLeague,(unsigned)s_waitCountry);
                return;
            }
        }
    }
}
int Expanded_UnemployedRecordIsEmpty(void) {
    return s_unemployedRecordOpen && !s_unemployedHadClub;
}
void Expanded_UnemployedRecord(MenuEntry *unused) {
    extern uintptr_t _currentMenuPtr;
    (void)unused;
    extern uint8_t _managementRecordMenu[];
    s_unemployedRecordOpen=1;
    ShowMenu(_managementRecordMenu);
    s_unemployedRecordOpen=0;
    /* Redraw the restored unemployment panel even if the modal returns
     * without a new input event (fixes the stale team/squad background). */
    if(_currentMenuPtr==(uintptr_t)Expanded_UnemployedCalendarMenu())
        DrawMenu();
    /* The previous unemployment menu's native entries are restored by
     * ShowMenu, and its live date/offer buttons are repainted as usual. */
}
void Expanded_UnemployedInit(void) {
    /* The unemployed menu is a clone of the ORIGINAL contest screen.
     * Keep its header, fixture-panel background, all nine action buttons
     * and the two scroll arrows in their original positions. The real
     * fixtures/templates (19..43) are hidden because no club is managed. */
    const int offerReady=s_waitTeam && s_waitWeeks>=s_nextOfferWeek;
    for (unsigned i=0;i<44;i++) {
        _menuEntries[i].isInvisible=(i>=19 && i<=43) || (i>=13 && i<=15);
        _menuEntries[i].isDisabled=0;
    }
    _menuEntries[3].isInvisible=1; /* stock career also hides wide header */
    /* The original header is repainted with unemployed text after the
     * entry backgrounds; otherwise it retains the last managed club. */
    for (unsigned i=0;i<4;i++) _menuEntries[i].content=NO_FOREGROUND;
    /* Only real actions are shown: no grey dead buttons in unemployed
     * mode. The underlying contest menu and callbacks stay unchanged. */
    static const unsigned disabled[]={4,5,6,8,11,16,17};
    for (unsigned i=0;i<sizeof(disabled)/sizeof(disabled[0]);i++) {
        MenuEntry *e=&_menuEntries[disabled[i]];
        e->isDisabled=1;
        e->isInvisible=1;
        if (disabled[i]<=12) {
            e->bgColor=0x9;
            e->stringFlags=(e->stringFlags & ~0x0fu) | 6u;
        }
    }
    /* Entry 12 is a REAL, compact RECORD button; it borrows the dormant
     * VIEW WORLD slot only in the unemployed clone, never in career mode. */
    _menuEntries[12].isInvisible=0;
    _menuEntries[12].isDisabled=0;
    _menuEntries[12].onSelect=Expanded_UnemployedRecord;
    _menuEntries[12].fg=(uintptr_t)"RECORD";
    _menuEntries[12].bgColor=0x8;
    _menuEntries[12].stringFlags=(_menuEntries[12].stringFlags & ~0x0fu)|2u;
    _menuEntries[10].isDisabled=!offerReady;
    _menuEntries[10].isInvisible=!offerReady;
    _menuEntries[10].fg=(uintptr_t)(offerReady?"VIEW CLUB OFFER":"NO JOB OFFERS");
    if (!offerReady) {
        _menuEntries[10].bgColor=0x9;
        _menuEntries[10].stringFlags=(_menuEntries[10].stringFlags & ~0x0fu) | 6u;
    }
    /* Explicit links between the ONLY enabled actions. The stock
     * navigation routes through disabled fixture/club buttons, so a
     * controller could get trapped on FORWARD TIME. */
    _menuEntries[12].leftEntry=9;
    _menuEntries[12].rightEntry=offerReady?10:7;
    _menuEntries[12].upEntry=offerReady?10:7;
    _menuEntries[12].downEntry=9;
    _menuEntries[7].leftEntry=9;
    _menuEntries[7].rightEntry=offerReady?10:9;
    _menuEntries[7].upEntry=9;
    _menuEntries[7].downEntry=9;
    _menuEntries[9].leftEntry=7;
    _menuEntries[9].rightEntry=12;
    _menuEntries[9].upEntry=7;
    _menuEntries[9].downEntry=7;
    _menuEntries[10].leftEntry=7;
    _menuEntries[10].rightEntry=9;
    _menuEntries[10].upEntry=7;
    _menuEntries[10].downEntry=12;
    /* Pending offers must not steal the keyboard/controller focus from
     * FORWARD TIME. VIEW CLUB OFFER remains reachable to the right. */
    ((Menu*)_currentMenu)->selectedEntry=&_menuEntries[7];
}
static void contest_at(unsigned x,unsigned y,const char *line,unsigned color) {
    DrawMenuTextCentered((uint16_t)x,(uint16_t)y,(uint16_t)color,
                         (uint8_t*)line,(uint8_t*)_charTables[0]);
}
static void contest_line(unsigned y,const char *line,unsigned color) {
    contest_at(160,y,line,color);
}
/* The unemployed calendar uses the SAME top-right date box as the
 * employed calendar; its independent week clock must be used there. */
static ExpandedWorldDate unemployed_date_at(unsigned weeks) {
    ExpandedWorldDate date={(uint16_t)World_FirstYear(),7u,1u};
    (void)ExpandedWorld_DateAfterWeeks(date,(uint32_t)weeks,&date);
    return date;
}
static unsigned unemployed_season_year(unsigned weeks) {
    ExpandedWorldDate date=unemployed_date_at(weeks);
    return (unsigned)date.year-(date.month<7u ? 1u : 0u);
}
static unsigned unemployed_week_of_season(unsigned weeks) {
    ExpandedWorldDate date=unemployed_date_at(weeks);
    ExpandedWorldDate start={(uint16_t)unemployed_season_year(weeks),7u,1u};
    uint32_t week=1;
    (void)ExpandedWorld_SeasonWeek(start,date,&week);
    return (unsigned)week;
}
void Expanded_UnemployedDate(char *out,size_t cap) {
    Expanded_FormatCivilDate(out,cap,unemployed_date_at(s_waitWeeks));
}
unsigned Expanded_DebugUnemployedWeeks(void) {
    return s_waitWeeks;
}
void Expanded_UnemployedDraw(void) {
    char line[90],date[40],first[32],last[32];
    unsigned season=unemployed_season_year(s_waitWeeks);
    sanitize(first,sizeof first,_managerNameBuffer);
    sanitize(last,sizeof last,_managerSurnameBuffer);
    /* The left header is only 90px wide; abbreviate long manager names
     * instead of letting them overlap the UNEMPLOYED middle header. */
    if (strlen(first)+strlen(last)+1u>14u)
        snprintf(line,sizeof line,"%c. %.10s",first[0]?first[0]:'?',last);
    else
        snprintf(line,sizeof line,"%s %s",first,last);
    extern int Expanded_UI_Active(void);
    int compact=Expanded_UI_Active();
    contest_at(compact?57:69,compact?7:40,line,7);
    contest_at(compact?169:176,compact?7:40,"UNEMPLOYED",7);
    snprintf(line,sizeof line,"%u/%02u SEASON",season,(season+1u)%100u);
    contest_at(compact?270:282,compact?7:40,line,7);
    contest_line(70,"NO FIXTURES - NO CLUB",6);
    if (s_waitTeam && s_waitWeeks>=s_nextOfferWeek) {
        snprintf(line,sizeof line,"WEEK %02u - CLUB OFFER RECEIVED",
                 unemployed_week_of_season(s_waitWeeks));
        contest_line(101,line,2);
        contest_line(127,"SELECT VIEW CLUB OFFER",7);
    } else {
        /* The real calendar's fixture rows become weekly waiting rows.
         * Nothing is simulated by a fake match callback here. */
        /* Five well-spaced weeks instead of seven dense rows; the wait
         * counter and offer scheduling still advance normally. */
        for(unsigned i=0;i<5;i++) {
            snprintf(line,sizeof line,"WEEK %02u - WAIT FOR A CLUB OFFER",
                     unemployed_week_of_season(s_waitWeeks+i));
            contest_line(88u+i*17u,line,i==0?7:6);
        }
    }
    Expanded_UnemployedDate(date,sizeof date);
    if(!compact) contest_line(171,date,2);
}
void Expanded_UnemployedWait(void) {
    const unsigned jump=s_debug_wait_jump_weeks?s_debug_wait_jump_weeks:1u;
    s_debug_wait_jump_weeks=0u;
    /* FORWARD TIME deliberately expires an unanswered offer. Leaving an
     * offer pending must not freeze the world's seasons or competitions.
     * A new offer can arrive later; VIEW CLUB OFFER still accepts normally. */
    if (s_waitTeam && s_waitWeeks>=s_nextOfferWeek) {
        fprintf(stderr,"[offers] week %u: pending offer from %.17s expired "
                       "because manager advanced time\n",
                s_waitWeeks,s_waitTeamName);
        s_waitTeam=0;
        s_offerSnapshotValid=0;
        s_offerAgreedWeekly=0;
        s_nextOfferWeek=s_waitWeeks+next_delay();
    }
    if(s_waitWeeks<4000u) {
        unsigned remaining=4000u-s_waitWeeks;
        s_waitWeeks+=jump<remaining?jump:remaining;
    }
    if(jump!=1u)
        fprintf(stderr,"[debug-unemployed] +%u weeks to absolute week %u "
                       "(club cups still OG-unverified)\n",jump,s_waitWeeks);
    /* The pilot world operates on snapshots of actual club IDs, with no
     * live team-file reload while a modal is open. No game-clock mutation. */
    if (ExpandedWorldLive_Clubs()) {
        if (!ExpandedWorldLive_AdvanceWeeks((uint32_t)s_waitWeeks))
            fprintf(stderr,"[living-world] advance rejected at week %u\n",s_waitWeeks);
        if (s_waitWeeks%4u==0u)
            fprintf(stderr,"[living-world] wait=%u pilot results=%lu\n",
                    s_waitWeeks,(unsigned long)ExpandedWorldLive_Results());
    }
    if(s_waitWeeks>=s_nextOfferWeek && !s_waitTeam) choose_small_club();
    if(!s_waitTeam && s_waitWeeks>=s_nextOfferWeek)
        s_nextOfferWeek=s_waitWeeks+next_delay();
    _fireResetFlag=1;
    Expanded_UnemployedInit();
    DrawMenu();
}
void Expanded_OfferDraw(void) {
    char line[100];
    contest_line(59,"A CLUB WANTS TO HIRE YOU",7);
    snprintf(line,sizeof line,"CLUB: %.25s",s_waitTeamName); contest_line(74,line,2);
    snprintf(line,sizeof line,"COUNTRY: %.25s",s_waitCountryName);contest_line(93,line,7);
    snprintf(line,sizeof line,"REGION: %s",s_waitRegionName);contest_line(105,line,2);
    snprintf(line,sizeof line,"DIVISION: %u",(unsigned)s_waitLeague+1);contest_line(118,line,7);
    snprintf(line,sizeof line,"WAGE %lu / YEAR  REQUEST %lu",
             (unsigned long)(s_offerBaseWeekly*52u),
             (unsigned long)(s_offerDemandWeekly*52u));
    contest_line(138,line,2);
    if(s_offerStatus[0]) contest_line(153,s_offerStatus,7);
}
void Expanded_UnemployedShowOffer(void) {
    if(!s_waitTeam || s_waitWeeks<s_nextOfferWeek) return;
    ShowMenu(_expandedClubOfferMenu);
    if(s_waitAccepted) SetExitMenuFlag();
    else { Expanded_UnemployedInit(); DrawMenu(); }
}
void Expanded_OfferLess(void) {
    if(s_offerDemandWeekly>s_offerBaseWeekly) {
        uint32_t d=s_offerDemandWeekly-s_offerBaseWeekly;
        s_offerDemandWeekly-=d<s_offerStep?d:s_offerStep;
    }
    DrawMenu();
}
void Expanded_OfferMore(void) {
    /* Limit requests to 100% above the original offer; prevents overflow. */
    if(s_offerDemandWeekly+s_offerStep<=s_offerBaseWeekly*2u)
        s_offerDemandWeekly+=s_offerStep;
    DrawMenu();
}
/* Pure, testable decision rule: a zero-fame manager cannot double a
 * small club's wage budget. Better-known managers have more leverage. */
unsigned Expanded_OfferAcceptanceChance(uint32_t base,uint32_t ask,
                                        unsigned fame,unsigned rejected) {
    if(!base || ask<=base) return 100;
    unsigned premium=(unsigned)(((uint64_t)(ask-base)*100u)/base);
    int chance=72+(int)fame*8-(int)premium*3-(int)rejected*8;
    if(fame==0) chance-=18;
    if(fame==0 && premium>=50) chance=0;
    if(chance<0) chance=0;
    if(chance>95) chance=95;
    return (unsigned)chance;
}
void Expanded_UnemployedReject(void);
void Expanded_UnemployedAccept(void) {
    if(!offer_is_valid()) {
        fprintf(stderr,"[expanded14] rejected invalid team id %04x\n",s_waitTeam);
        s_waitTeam=0;
        SetExitMenuFlag();
        return;
    }
    if(s_offerDemandWeekly>s_offerBaseWeekly) {
        unsigned chance=Expanded_OfferAcceptanceChance(s_offerBaseWeekly,
            s_offerDemandWeekly,_expandedManagerFame,s_offerRejections);
        unsigned roll=next_random()%100u;
        if(roll>=chance) {
            s_offerRejections++;
            fprintf(stderr,"[expanded14] rejected counteroffer %lu vs %lu chance=%d roll=%u attempt=%u\n",
                    (unsigned long)s_offerDemandWeekly,(unsigned long)s_offerBaseWeekly,
                    chance,roll,s_offerRejections);
            if(s_offerRejections>=5) {
                Expanded_UnemployedReject();
                return;
            }
            snprintf(s_offerStatus,sizeof s_offerStatus,
                     "COUNTEROFFER REJECTED %u/5",s_offerRejections);
            DrawMenu();
            return;
        }
    }
    s_offerAgreedWeekly=s_offerDemandWeekly;
    { extern void Expanded_SetContractPremium(unsigned);
      unsigned premium=s_offerBaseWeekly?
          (unsigned)(((uint64_t)(s_offerAgreedWeekly-s_offerBaseWeekly)*100u)/s_offerBaseWeekly):0u;
      Expanded_SetContractPremium(premium); }
    s_waitAccepted=1;
    SetExitMenuFlag();
}
void Expanded_UnemployedReject(void) {
    s_waitTeam=0;
    s_offerSnapshotValid=0;
    s_offerAgreedWeekly=0;
    s_nextOfferWeek=s_waitWeeks+next_delay();
    SetExitMenuFlag();
}
void Expanded_OfferPreviewDraw(void) {
    char line[90];
    contest_line(55,s_previewMode==1?"CLUB SQUAD":"VIEW COMPETITIONS",2);
    snprintf(line,sizeof line,"%.23s - %.18s",s_waitTeamName,s_waitCountryName);
    contest_line(69,line,7);
    if(s_previewMode==1) {
        for(unsigned i=0;i<16;i++) {
            snprintf(line,sizeof line,"%02u %.25s",i+1,s_previewPlayers[i]);
            contest_line(81+i*8,line,7);
        }
    } else {
        ExpandedLiveRow rows[26];
        uint32_t count=0;
        /* s_waitCountry is the native TEAM-file index, whereas Living World
         * standings are indexed by the mapped country code. Offer selection
         * already uses World_FileCountry(): the preview must use the same key. */
        const uint8_t worldCountry=World_FileCountry(s_waitCountry);
        if (ExpandedWorldLive_Table(worldCountry,s_waitLeague,rows,26,&count)) {
            snprintf(line,sizeof line,"DIVISION %u - %u/%02u",
                     (unsigned)s_waitLeague+1,
                     (unsigned)ExpandedWorldLive_SeasonYear(),
                     ((unsigned)ExpandedWorldLive_SeasonYear()+1u)%100u);
            contest_line(82,line,2);
            contest_line(93,"RANK CLUB                 PL GD PTS",7);
            unsigned offeredRank=count;
            for(unsigned i=0;i<count;i++)
                if(rows[i].club==s_waitTeam) offeredRank=i;
            for(unsigned i=0;i<count && i<10;i++) {
                int gd=(int)rows[i].scored-(int)rows[i].conceded;
                snprintf(line,sizeof line,"%2u %.15s %2u %+3d %3ld",
                         i+1,rows[i].name,(unsigned)rows[i].played,
                         gd,(long)rows[i].points);
                contest_line(105u+i*10u,line,
                             rows[i].club==s_waitTeam ? 2 : 7);
            }
            if(offeredRank>=10 && offeredRank<count) {
                const ExpandedLiveRow *r=&rows[offeredRank];
                int gd=(int)r->scored-(int)r->conceded;
                snprintf(line,sizeof line,"%2u %.15s %2u %+3d %3ld",
                         offeredRank+1,r->name,(unsigned)r->played,gd,
                         (long)r->points);
                contest_line(205,line,2);
            }
            ExpandedLivePreseason ps;
            if (ExpandedWorldLive_Preseason(s_waitTeam,&ps) &&
                ExpandedWorldLive_Results()==0u) {
                snprintf(line,sizeof line,"PRESEASON: %u FRIENDLIES (PILOT)",
                         (unsigned)ps.friendlies);
                contest_line(219,line,6);
            } else {
                contest_line(219,"LIVING WORLD - OFFER TABLE",6);
            }
        } else {
            snprintf(line,sizeof line,"DIVISION %u - PILOT UNAVAILABLE",
                     (unsigned)s_waitLeague+1);
            contest_line(100,line,2);
            contest_line(150,"RESULTS NOT IN SWOS CAREER YET",7);
        }
    }
}
void Expanded_OfferViewSquad(void) {
    if(!offer_is_valid()) return;
#ifdef SWOS_SDL_BUILD
    /* Real SWOS read-only squad viewer, pointed at the OFFER SNAPSHOT.
     * Never LoadTeamFile inside the modal: that would overwrite career state. */
    extern uint16_t _selTeamNumber;
    extern uint32_t _viewSquadFlags;
    extern int16_t ShowSquad(TeamFile *,uint8_t *,uint32_t,uint32_t,uint16_t,uint16_t);
    static uint8_t title[]="CLUB OFFER - SQUAD";
    uint16_t saved=_selTeamNumber;
    uint32_t savedFlags=_viewSquadFlags;
    _selTeamNumber=0xFFFFu;
    _viewSquadFlags=0;
    ShowSquad(&s_offerSnapshot.team,title,0,0,0,0xFFFFu);
    _selTeamNumber=saved;
    _viewSquadFlags=savedFlags;
    /* Native squad viewer is not an expanded modal; redraw its offer parent. */
    DrawMenu();
#else
    s_previewMode=1;
    ShowMenu(_expandedOfferPreviewMenu);
#endif
}
void Expanded_OfferViewLeague(void) {
    if(!offer_is_valid()) return;
    s_previewMode=2;
    ShowMenu(_expandedOfferPreviewMenu);
}
int Expanded_WaitForFirstOffer(uint16_t *team) {
    if(!team) return 0;
    s_randomState=(unsigned)time(NULL) ^ (unsigned)(uintptr_t)team ^
                  ((unsigned)_expandedManagerStartAge<<16) ^ 0xA529B41Du;
    if(!s_randomState) s_randomState=1;
    s_unemployedHadClub=0;
    s_waitWeeks=0; s_waitAccepted=0; s_waitTeam=0;
    s_debug_wait_jump_weeks=0;
    s_offerSnapshotValid=0; s_offerAgreedWeekly=0;
    s_nextOfferWeek=next_delay();
    /* NEW CAREER START initialized the one shared Living World before
     * opening any modal. Never reset/re-import it at the first contract,
     * otherwise the unemployed calendar and employed league diverge. */
    if (!ExpandedWorldLive_Clubs()) {
        fprintf(stderr,"[living-world] first offer refused: world not "
                       "initialized at NEW CAREER START\n");
        return 0;
    }
    ShowMenu(Expanded_UnemployedCalendarMenu());
    if(!s_waitAccepted || !offer_is_valid()) return 0;
    if(!verify_offer_in_file()) {
        fprintf(stderr,"[expanded11] declined stale/unplayable offer team=%04x\n",s_waitTeam);
        return 0;
    }
    *team=s_waitTeam;
    fprintf(stderr,"[expanded10] accepted club %s (%u) after %u weeks\n",
            s_waitTeamName,(unsigned)s_waitCountry,s_waitWeeks);
    return 1;
}

/* 0.10.16: after dismissal, the unemployment calendar keeps the exact
 * career week (the original first-job flow always starts at 1996 week 1).
 * The new job uses the existing safe InitCareer path; it begins a fresh
 * club season because the engine cannot transplant a mid-season fixture
 * cache into a different club without rebuilding the competition state. */
int Expanded_WaitForRehire(uint16_t *team) {
    if(!team) return 0;
    extern uint8_t _lastFinancialWeekIndex;
    s_unemployedHadClub=1;
    /* Native financial weeks count from August; the unemployed clock now
     * counts from July 1. Convert through civil dates and round forward to
     * the next weekly boundary, avoiding an apparent time reversal. */
    ExpandedWorldDate native={(uint16_t)(World_FirstYear()+
                        (unsigned)_currentSeasonShowing),8u,1u};
    ExpandedWorldDate civil;
    ExpandedWorldDate july={(uint16_t)World_FirstYear(),7u,1u};
    int32_t native_day=0,july_day=0;
    if (ExpandedWorld_DateAfterWeeks(native,
            (uint32_t)_lastFinancialWeekIndex,&civil) &&
        ExpandedWorld_DateToIndex(civil,&native_day) &&
        ExpandedWorld_DateToIndex(july,&july_day) && native_day>=july_day)
        s_waitWeeks=(unsigned)((native_day-july_day+6)/7);
    else
        s_waitWeeks=(unsigned)_currentSeasonShowing*52u+
                    (unsigned)_lastFinancialWeekIndex+5u;
    if (ExpandedWorldLive_Clubs())
        (void)ExpandedWorldLive_AdvanceWeeks((uint32_t)s_waitWeeks);
    s_waitAccepted=0;s_waitTeam=0;s_offerSnapshotValid=0;s_offerAgreedWeekly=0;
    s_debug_wait_jump_weeks=0;
    s_nextOfferWeek=s_waitWeeks+next_delay();
    ShowMenu(Expanded_UnemployedCalendarMenu());
    if(!s_waitAccepted || !offer_is_valid() || !verify_offer_in_file()) return 0;
    *team=s_waitTeam;
    return 1;
}

/* The modal is safe here: this callback runs AFTER the match update, not
 * inside the goal/finance calculation. Continue into the unemployed calendar
 * at the current week. The native SWOS season state is rebuilt if rehired. */
extern uint8_t _expandedSackedMenu[];
void Expanded_SackedReturn(void) { SetExitMenuFlag(); }
int Expanded_HandleSackingAfterMatch(void) {
    extern int Expanded_DismissalPending(void);
    extern void Expanded_DismissalAcknowledged(void);
    extern void Expanded_DebugForceDismissalPending(void);
    extern uint16_t _gameType;
    /* Pre-hire native results must not trigger a post-hire dismissal.
     * The user's F12 test overlay and controls are otherwise unchanged. */
    extern int Expanded_MidseasonJoin_Active(void);
    if (Expanded_MidseasonJoin_Active()) return 0;
    if (_gameType == 4 && ExpandedDebug_ConsumeSack())
        Expanded_DebugForceDismissalPending();
    if(!Expanded_DismissalPending()) return 0;
    Expanded_DismissalAcknowledged();
    ShowMenu(_expandedSackedMenu);
    uint16_t newTeam=0;
    if(Expanded_WaitForRehire(&newTeam)) {
        extern void InitCareer(uint16_t);
        extern uint8_t _contestMenu[];
        extern void ActivateMenu(uint8_t *);
        InitCareer(newTeam);
        { extern int Expanded_MidseasonJoin_AlignSeason(uint32_t);
          if (!Expanded_MidseasonJoin_AlignSeason(s_waitWeeks)) {
              extern uint8_t _swosMainMenu[];
              fprintf(stderr,"[midseason] rehire cannot align world season; "
                             "returning to title without publishing wrong standings\n");
              ActivateMenu(_swosMainMenu);
              return 1;
          }
        }
        Expanded_CommitNegotiatedSalary();
        extern void Expanded_MidseasonJoin_Begin(uint32_t weeks);
        Expanded_MidseasonJoin_Begin(s_waitWeeks);
#ifdef SWOS_SDL_BUILD
        { extern int Expanded_MidseasonJoin_LastResult(void);
          if(Expanded_MidseasonJoin_LastResult()==2) {
              extern uint8_t _swosMainMenu[];
              fprintf(stderr,"[midseason] REHIRE FAILED: no synced OG table; "
                             "preserving previous autosave\n");
              ActivateMenu(_swosMainMenu);
              return 1;
          }
        }
        /* A tester save roundtrip reloads OG competition contexts.
         * Never do it in the middle of the native cup catch-up loop:
         * finish() performs native-join-complete after success. */
        { extern int Expanded_MidseasonJoin_Active(void);
          if (!Expanded_MidseasonJoin_Active()) {
              extern void Expanded_AutoCareer_Save(const char *);
              Expanded_AutoCareer_Save("new-contract");
          } else {
              fprintf(stderr,"[autosave] deferred new-contract until "
                             "native-join-complete (OG catch-up active)\n");
          }
        }
#endif
        ActivateMenu(_contestMenu);
    } else {
        /* The user backed out of the unemployed screen: do not reopen the
         * dismissed club's fixtures. Return to the game's title menu. */
        extern uint8_t _swosMainMenu[];
        extern void ActivateMenu(uint8_t *);
        ActivateMenu(_swosMainMenu);
    }
    return 1;
}

const char *Expanded_CurrentClubName(void) {
    extern TeamFileEx _careerTeam;
    static char name[18];
    memcpy(name,_careerTeam.team.teamName,17);
    name[17]=0;
    return name;
}
