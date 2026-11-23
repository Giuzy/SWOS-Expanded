/* World-switch register banks: one OG world installed at a time.
 * Both source datasets are prepared at loading; this module stores the static
 * OG tables and in-place competition records BEFORE/AFTER modern installation.
 *
 * Same-process namespace principle reviewed against aminta/swos-9697-mod
 * tools/careerworld.py::w_ns_set and tools/swe.c::reg_enter/reg_leave.
 * No MOD executable/assembly patch or new gameplay simulator is used.
 * This bank is STATIC DEFINITIONS ONLY: it never snapshots an active career.
 */
#ifdef SWOS_SDL_BUILD
#include "world.h"
#include "competition_data.h"
#include "swos_io.h"
#include "swos_settings.h" /* existing [general] world preference */
#if defined(SWOS_SDL_BUILD) && defined(SWOS_CAR_ZSTD)
#include "car_writer.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

extern uint16_t _teamsCountryNumbers[256];
extern void *_competitionsTable[256];
extern void *_countriesTable[256];
extern uint16_t _teamsLoaded, _teamFileTeamCount;
extern uint8_t _lastLoadedExtNumber;
extern uint32_t _teamFileDataSize;
extern void DataJson_InstallCompetitionGraph(void);
extern void WorldCups_Install(void);
extern void Career_InstallCopaLibertadoresPresetEntry(void);
/* main.c owns the active SWOS.ini options for the full menu lifetime.
 * Reuse its non-destructive writer instead of creating a second settings
 * file or writing any .CAR during a debug world selection. */
extern PlatformInitOptions *g_arcadeOpts;
extern const char *g_arcadeIniPath;
static void remember_selected_world(void)
{
    if(!g_arcadeOpts || !g_arcadeIniPath || !*g_arcadeIniPath) {
        fprintf(stderr,"[world-preference] NOT_SAVED world=%s reason=NO_SETTINGS_CONTEXT\n",
                World_Tag());
        return;
    }
    snprintf(g_arcadeOpts->world,sizeof g_arcadeOpts->world,"%s",World_Tag());
    Settings_Save(g_arcadeOpts,g_arcadeIniPath);
    /* Never report a successful write merely because Settings_Save was
     * called (it has a void API and may fail on a read-only game folder). */
    PlatformInitOptions verify={0};
    Settings_Load(&verify,g_arcadeIniPath);
    fprintf(stderr,"[world-preference] %s world=%s file=%s\n",
            strcmp(verify.world,World_Tag())==0?"STORED":"WRITE_UNCONFIRMED",
            World_Tag(),g_arcadeIniPath);
}


/* Registry entries live in competition_data.c; snapshots are by compId.
 * Dynamic JSON templates remain allocated and resident in either namespace. */
extern void Comp_ExportWorldRegistry(Competition **out);
extern void Comp_ImportWorldRegistry(Competition *const *in);

/* Static OG records and pointer tables whose contents are replaced by
 * World_InstallBaked, DataJson_InstallCompetitionGraph, WorldCups_Install
 * and LeaguePhase_InstallCups. Generated from competition_data.h external
 * definitions, excluding active career *Copy records. */
typedef struct {
    void *dst;
    size_t size;
} WorldBankArea;
#define WORLD_BANK_AREA(x) { &(x), sizeof(x) }
static const WorldBankArea kAreas[] = {
    WORLD_BANK_AREA(_teamsCountryNumbers),
    WORLD_BANK_AREA(_countriesTable),
    WORLD_BANK_AREA(_competitionsTable),
    WORLD_BANK_AREA(_careerContests),
    WORLD_BANK_AREA(_customsLeague),
    WORLD_BANK_AREA(_customsTable),
    WORLD_BANK_AREA(_euroCup),
    WORLD_BANK_AREA(_cupWinnersCup),
    WORLD_BANK_AREA(_uefaCup),
    WORLD_BANK_AREA(_worldCup2),
    WORLD_BANK_AREA(_worldCup),
    WORLD_BANK_AREA(_worldCupQualifyingGroup1),
    WORLD_BANK_AREA(_worldCupQualifyingGroup2),
    WORLD_BANK_AREA(_worldCupQualifyingGroup3),
    WORLD_BANK_AREA(_worldCupQualifyingGroup4),
    WORLD_BANK_AREA(_worldCupQualifyingGroup5),
    WORLD_BANK_AREA(_worldCupQualifyingGroup6),
    WORLD_BANK_AREA(_worldCupQualifyingGroup7),
    WORLD_BANK_AREA(_worldCupQualifyingGroup8),
    WORLD_BANK_AREA(_northAmericaWorldCupQualification),
    WORLD_BANK_AREA(_southAmericaWorldCupQualification),
    WORLD_BANK_AREA(_africaWorldCupQualification),
    WORLD_BANK_AREA(_asiaWorldCupQualification),
    WORLD_BANK_AREA(_oceaniaWorldCupQualification),
    WORLD_BANK_AREA(_invitationTournament),
    WORLD_BANK_AREA(_tour),
    WORLD_BANK_AREA(_friendly),
    WORLD_BANK_AREA(_europeanChampionships2),
    WORLD_BANK_AREA(_europeanChampionships),
    WORLD_BANK_AREA(_ecQualifyingGroup1),
    WORLD_BANK_AREA(_ecQualifyingGroup2),
    WORLD_BANK_AREA(_ecQualifyingGroup3),
    WORLD_BANK_AREA(_ecQualifyingGroup4),
    WORLD_BANK_AREA(_ecQualifyingGroup5),
    WORLD_BANK_AREA(_ecQualifyingGroup6),
    WORLD_BANK_AREA(_ecQualifyingGroup7),
    WORLD_BANK_AREA(_ecQualifyingGroup8),
    WORLD_BANK_AREA(_copaAmerica),
    WORLD_BANK_AREA(_africanNationsCup),
    WORLD_BANK_AREA(_northAmericanCup),
    WORLD_BANK_AREA(_asianCup),
    WORLD_BANK_AREA(_oceaniaCup),
    WORLD_BANK_AREA(_albanianLeague),
    WORLD_BANK_AREA(_austrianLeague),
    WORLD_BANK_AREA(_belarussiaLleague),
    WORLD_BANK_AREA(_belgianLeague),
    WORLD_BANK_AREA(_bulgarianLeague),
    WORLD_BANK_AREA(_croatianLeague),
    WORLD_BANK_AREA(_cypriotLeague),
    WORLD_BANK_AREA(_czechLeague),
    WORLD_BANK_AREA(_danishLeague),
    WORLD_BANK_AREA(_estonianLeague),
    WORLD_BANK_AREA(_faroeIslesLeague),
    WORLD_BANK_AREA(_finnishLeague),
    WORLD_BANK_AREA(_greekLeague),
    WORLD_BANK_AREA(_hungarianLeague),
    WORLD_BANK_AREA(_icelandicLeague),
    WORLD_BANK_AREA(_israeliLeague),
    WORLD_BANK_AREA(_latvianLeague),
    WORLD_BANK_AREA(_lithuanianLeague),
    WORLD_BANK_AREA(_luxembourgLeague),
    WORLD_BANK_AREA(_malteseLeague),
    WORLD_BANK_AREA(_northernIrishLeague),
    WORLD_BANK_AREA(_norwegianLeague),
    WORLD_BANK_AREA(_polishLeague),
    WORLD_BANK_AREA(_portugeseLeague),
    WORLD_BANK_AREA(_irishLeague),
    WORLD_BANK_AREA(_romanianLeague),
    WORLD_BANK_AREA(_russianLeague),
    WORLD_BANK_AREA(_sanMarinoLeague),
    WORLD_BANK_AREA(_slovakianLeague),
    WORLD_BANK_AREA(_slovenianLeague),
    WORLD_BANK_AREA(_swedishLeague),
    WORLD_BANK_AREA(_swissLeague),
    WORLD_BANK_AREA(_turkishLeague),
    WORLD_BANK_AREA(_ukrainianLeague),
    WORLD_BANK_AREA(_welshLeague),
    WORLD_BANK_AREA(_yugoslavianLeague),
    WORLD_BANK_AREA(_algerianLeague),
    WORLD_BANK_AREA(_bolivianLeague),
    WORLD_BANK_AREA(_brasilianLeague),
    WORLD_BANK_AREA(_chileanLeague),
    WORLD_BANK_AREA(_colombianLeague),
    WORLD_BANK_AREA(_ecuadorLeague),
    WORLD_BANK_AREA(_indianLeague),
    WORLD_BANK_AREA(_paraguayanLeague),
    WORLD_BANK_AREA(_peruvianLeague),
    WORLD_BANK_AREA(_southAfricanLeague),
    WORLD_BANK_AREA(_surinamLeague),
    WORLD_BANK_AREA(_taiwaneseLeague),
    WORLD_BANK_AREA(_americanLeague),
    WORLD_BANK_AREA(_venezuelanLeague),
    WORLD_BANK_AREA(_ghanaLeague),
    WORLD_BANK_AREA(_frenchLeague),
    WORLD_BANK_AREA(_germanLeague),
    WORLD_BANK_AREA(_italianLeague),
    WORLD_BANK_AREA(_japaneseLeague),
    WORLD_BANK_AREA(_argentinianLeague),
    WORLD_BANK_AREA(_mexicanLeague),
    WORLD_BANK_AREA(_elSalvadorLeague),
    WORLD_BANK_AREA(_australianLeague),
    WORLD_BANK_AREA(_newZealandLeague),
    WORLD_BANK_AREA(_div1Playoffs),
    WORLD_BANK_AREA(_div2Playoffs),
    WORLD_BANK_AREA(_div3Playoffs),
    WORLD_BANK_AREA(_englishLeague),
    WORLD_BANK_AREA(_englishLeagues1),
    WORLD_BANK_AREA(_englishLeagues2),
    WORLD_BANK_AREA(_englishLeagues3),
    WORLD_BANK_AREA(_dutchLeague),
    WORLD_BANK_AREA(_dutchLeagues1),
    WORLD_BANK_AREA(_dutchPlayoffs),
    WORLD_BANK_AREA(_scottishLeague),
    WORLD_BANK_AREA(_scottishLeague1),
    WORLD_BANK_AREA(_scottishPlayoffs),
    WORLD_BANK_AREA(_spanishLeague),
    WORLD_BANK_AREA(_spanishLeague1),
    WORLD_BANK_AREA(_spanishPlayoffs),
    WORLD_BANK_AREA(_uruguayanLeague),
    WORLD_BANK_AREA(_uruguayanLeague1),
    WORLD_BANK_AREA(_uruguayanPlayoffs),
    WORLD_BANK_AREA(_albanianCup),
    WORLD_BANK_AREA(_austrianCup),
    WORLD_BANK_AREA(_belarusianCup),
    WORLD_BANK_AREA(_belgianCup),
    WORLD_BANK_AREA(_bulgarianCup),
    WORLD_BANK_AREA(_croatianCup),
    WORLD_BANK_AREA(_cypriotCup),
    WORLD_BANK_AREA(_czechCup),
    WORLD_BANK_AREA(_danishCup),
    WORLD_BANK_AREA(_estonianCup),
    WORLD_BANK_AREA(_faroeIslesCup),
    WORLD_BANK_AREA(_finnishCup),
    WORLD_BANK_AREA(_greekCup),
    WORLD_BANK_AREA(_hungarianCup),
    WORLD_BANK_AREA(_icelandicCup),
    WORLD_BANK_AREA(_irishCup),
    WORLD_BANK_AREA(_israeliCup),
    WORLD_BANK_AREA(_latvianCup),
    WORLD_BANK_AREA(_lithuanianCup),
    WORLD_BANK_AREA(_luxembourgCup),
    WORLD_BANK_AREA(_malteseCup),
    WORLD_BANK_AREA(_northernIrishCup),
    WORLD_BANK_AREA(_norwegianCup),
    WORLD_BANK_AREA(_polishCup),
    WORLD_BANK_AREA(_portugeseCup),
    WORLD_BANK_AREA(_romanianCup),
    WORLD_BANK_AREA(_russianCup),
    WORLD_BANK_AREA(_sanMarinoCup),
    WORLD_BANK_AREA(_slovakianCup),
    WORLD_BANK_AREA(_slovenianCup),
    WORLD_BANK_AREA(_swedishCup),
    WORLD_BANK_AREA(_swissCup),
    WORLD_BANK_AREA(_turkishCup),
    WORLD_BANK_AREA(_ukrainianCup),
    WORLD_BANK_AREA(_welshCup),
    WORLD_BANK_AREA(_yugoslavianCup),
    WORLD_BANK_AREA(_algerianCup),
    WORLD_BANK_AREA(_southAfricanCup),
    WORLD_BANK_AREA(_taiwaneseCup),
    WORLD_BANK_AREA(_englishCup),
    WORLD_BANK_AREA(_englishLeagueCup),
    WORLD_BANK_AREA(_englishCup_1),
    WORLD_BANK_AREA(_englishLeagueCup_1),
    WORLD_BANK_AREA(_frenchCup),
    WORLD_BANK_AREA(_germanCup),
    WORLD_BANK_AREA(_italianCup),
    WORLD_BANK_AREA(_scottishCup),
    WORLD_BANK_AREA(_scottishLeagueCup),
    WORLD_BANK_AREA(_spanishCup),
    WORLD_BANK_AREA(_dutchCup),
    WORLD_BANK_AREA(_australianCup),
    WORLD_BANK_AREA(_japaneseCup),
    WORLD_BANK_AREA(_japaneseEmperorsCup),
    WORLD_BANK_AREA(_nationalTeamCompetitionsAndQualifications),
    WORLD_BANK_AREA(_continentalCups),
    WORLD_BANK_AREA(_worldCupQualifyingGroups),
    WORLD_BANK_AREA(_ecQualifyingGroups),
    WORLD_BANK_AREA(_worldTable),
    WORLD_BANK_AREA(_europeTable),
    WORLD_BANK_AREA(_africaTable),
    WORLD_BANK_AREA(_southAmericaTable),
    WORLD_BANK_AREA(_northAmericaTable),
    WORLD_BANK_AREA(_asiaTable),
    WORLD_BANK_AREA(_oceaniaTable),
    WORLD_BANK_AREA(_albaniaTable),
    WORLD_BANK_AREA(_austriaTable),
    WORLD_BANK_AREA(_belgiumTable),
    WORLD_BANK_AREA(_belarusTable),
    WORLD_BANK_AREA(_bulgariaTable),
    WORLD_BANK_AREA(_croatiaTable),
    WORLD_BANK_AREA(_cyprusTable),
    WORLD_BANK_AREA(_czechRepublicTable),
    WORLD_BANK_AREA(_denmarkTable),
    WORLD_BANK_AREA(_englandTable),
    WORLD_BANK_AREA(_estoniaTable),
    WORLD_BANK_AREA(_faroeIslandsTable),
    WORLD_BANK_AREA(_finlandTable),
    WORLD_BANK_AREA(_franceTable),
    WORLD_BANK_AREA(_germanyTable),
    WORLD_BANK_AREA(_greeceTable),
    WORLD_BANK_AREA(_hungaryTable),
    WORLD_BANK_AREA(_icelandTable),
    WORLD_BANK_AREA(_republicIrelandTable),
    WORLD_BANK_AREA(_israelTable),
    WORLD_BANK_AREA(_italyTable),
    WORLD_BANK_AREA(_latviaTable),
    WORLD_BANK_AREA(_lithuaniaTable),
    WORLD_BANK_AREA(_luxembourgTable),
    WORLD_BANK_AREA(_maltaTable),
    WORLD_BANK_AREA(_hollandTable),
    WORLD_BANK_AREA(_northernIrelandTable),
    WORLD_BANK_AREA(_norwayTable),
    WORLD_BANK_AREA(_polandTable),
    WORLD_BANK_AREA(_portugalTable),
    WORLD_BANK_AREA(_romaniaTable),
    WORLD_BANK_AREA(_russiaTable),
    WORLD_BANK_AREA(_sanMarinoTable),
    WORLD_BANK_AREA(_scotlandTable),
    WORLD_BANK_AREA(_slovakiaTable),
    WORLD_BANK_AREA(_sloveniaTable),
    WORLD_BANK_AREA(_spainTable),
    WORLD_BANK_AREA(_swedenTable),
    WORLD_BANK_AREA(_switzerlandTable),
    WORLD_BANK_AREA(_turkeyTable),
    WORLD_BANK_AREA(_ukraineTable),
    WORLD_BANK_AREA(_walesTable),
    WORLD_BANK_AREA(_yugoslaviaTable),
    WORLD_BANK_AREA(_algeriaTable),
    WORLD_BANK_AREA(_argentinaTable),
    WORLD_BANK_AREA(_australiaTable),
    WORLD_BANK_AREA(_boliviaTable),
    WORLD_BANK_AREA(_brasilTable),
    WORLD_BANK_AREA(_chileTable),
    WORLD_BANK_AREA(_colombiaTable),
    WORLD_BANK_AREA(_ecuadorTable),
    WORLD_BANK_AREA(_elSalvadorTable),
    WORLD_BANK_AREA(_indiaTable),
    WORLD_BANK_AREA(_japanTable),
    WORLD_BANK_AREA(_mexicoTable),
    WORLD_BANK_AREA(_newZealandTable),
    WORLD_BANK_AREA(_paraguayTable),
    WORLD_BANK_AREA(_peruTable),
    WORLD_BANK_AREA(_southAfricaTable),
    WORLD_BANK_AREA(_surinamTable),
    WORLD_BANK_AREA(_taiwanTable),
    WORLD_BANK_AREA(_uruguayTable),
    WORLD_BANK_AREA(_usaTable),
    WORLD_BANK_AREA(_venezuelaTable),
    WORLD_BANK_AREA(_ghanaTable),
    WORLD_BANK_AREA(_cupPickOceaniaInviteFriendly),
    WORLD_BANK_AREA(_cupPickOceaniaOceaniaCupFriendly),
    WORLD_BANK_AREA(_cupPickOceaniaWorldCupFriendly),
    WORLD_BANK_AREA(_cupPickOceaniaWorldCupInvite),
    WORLD_BANK_AREA(_cupPickOceaniaOceaniaCupNonFinalist),
    WORLD_BANK_AREA(_cupPickOceaniaOceaniaCupFinalist),
    WORLD_BANK_AREA(_cupPickNAmericaInviteFriendly),
    WORLD_BANK_AREA(_cupPickNAmericaNaCupFriendly),
    WORLD_BANK_AREA(_cupPickNAmericaWorldCupFriendly),
    WORLD_BANK_AREA(_cupPickNAmericaWorldCupInvite),
    WORLD_BANK_AREA(_cupPickNAmericaNaCupNonFinalist),
    WORLD_BANK_AREA(_cupPickNAmericaNaCupFinalist),
    WORLD_BANK_AREA(_cupPickAsiaInviteFriendly),
    WORLD_BANK_AREA(_cupPickAsiaAsianCupFriendly),
    WORLD_BANK_AREA(_cupPickAsiaWorldCupFriendly),
    WORLD_BANK_AREA(_cupPickAsiaWorldCupInvite),
    WORLD_BANK_AREA(_cupPickAsiaAsianCupNonFinalist),
    WORLD_BANK_AREA(_cupPickAsiaAsianCupFinalist),
    WORLD_BANK_AREA(_cupPickAfricaInviteFriendly),
    WORLD_BANK_AREA(_cupPickAfricaAfricanCupFriendly),
    WORLD_BANK_AREA(_cupPickAfricaWorldCupFriendly),
    WORLD_BANK_AREA(_cupPickAfricaWorldCupInvite),
    WORLD_BANK_AREA(_cupPickAfricaAfricanCupNonFinalist),
    WORLD_BANK_AREA(_cupPickAfricaAfricanCupFinalist),
    WORLD_BANK_AREA(_cupPickSAmericaInviteFriendly),
    WORLD_BANK_AREA(_cupPickSAmericaCopaAmericaFriendly),
    WORLD_BANK_AREA(_cupPickSAmericaWorldCupFriendly),
    WORLD_BANK_AREA(_cupPickSAmericaWorldCupInvite),
    WORLD_BANK_AREA(_cupPickSAmericaCopaAmericaNonFinalist),
    WORLD_BANK_AREA(_cupPickSAmericaCopaAmericaFinalist),
    WORLD_BANK_AREA(_ecQualifyingGroupsTable),
    WORLD_BANK_AREA(_cupPickEuropeNoCupFriendly),
    WORLD_BANK_AREA(_cupPickListEuropeInviteTournamentFriendly),
    WORLD_BANK_AREA(_cupPickLisEuropeEuroChampsFriendly),
    WORLD_BANK_AREA(_cupPickListEuropeEuroChampsInviteFriendly),
    WORLD_BANK_AREA(_cupPickListEuropeWorldCupFriendly),
    WORLD_BANK_AREA(_cupPickLisEuropeWorldCupInviteFriendly),
    WORLD_BANK_AREA(_cupPickEuropeNoCupFriendlyAlt),
    WORLD_BANK_AREA(_cupPickListEuropeInviteTournamentFriendlyAlt),
    WORLD_BANK_AREA(_worldCupQualifyingGroupsTable),
};
#undef WORLD_BANK_AREA

#define WORLD_AREA_COUNT (sizeof(kAreas)/sizeof(kAreas[0]))
static unsigned char *s_bank[2];
static size_t s_areaOffset[WORLD_AREA_COUNT];
static size_t s_bankSize;
static Competition *s_registry[2][256];
static int s_prepared;
static int s_modernReady;
static unsigned s_switches;

static int set_json_mode(int modern)
{
#ifdef _WIN32
    return _putenv(modern ? "SWOS_DATA_JSON=1" : "SWOS_DATA_JSON=0") == 0;
#else
    return setenv("SWOS_DATA_JSON", modern ? "1" : "0", 1) == 0;
#endif
}
static void save_bank(int n)
{
    for(size_t i=0;i<WORLD_AREA_COUNT;++i)
        memcpy(s_bank[n]+s_areaOffset[i],kAreas[i].dst,kAreas[i].size);
    Comp_ExportWorldRegistry(s_registry[n]);
}
static void restore_bank(int n)
{
    for(size_t i=0;i<WORLD_AREA_COUNT;++i)
        memcpy(kAreas[i].dst,s_bank[n]+s_areaOffset[i],kAreas[i].size);
    Comp_ImportWorldRegistry(s_registry[n]);
}
static int modern_data_valid(void)
{
    char file[128], resolved[1024], line[160], fingerprint[40]="";
    const char *expected=World_DataFingerprint();
    snprintf(file,sizeof file,"data_json/worlds/2627/WORLD.ini");
    FILE *f=fopen(swos_data_path(file,resolved,sizeof resolved),"rb");
    if(!f) {
        fprintf(stderr,"[world-switch] 2627 UNAVAILABLE: WORLD.ini not installed\n");
        return 0;
    }
    while(fgets(line,sizeof line,f))
        if(sscanf(line," fingerprint = %39s",fingerprint)==1)break;
    fclose(f);
    if(expected[0] && strcmp(expected,fingerprint)!=0){
        fprintf(stderr,"[world-switch] 2627 UNAVAILABLE: fingerprint=%s expected=%s\n",
            fingerprint[0]?fingerprint:"missing",expected);
        return 0;
    }
    return 1;
}

/* Called once by FrontEndPreLoopSetup before any team/competition menu.
 * Capture the compiled OG baseline, then prepare modern overlays exactly once
 * and restore the startup user's world; no second OG world is simultaneously
 * installed. Modern's persistent JSON heap and name pools stay allocated.
 * If the package is missing, preserve the historical world without a hot path. */
void WorldSwitch_PrepareAtLoading(void)
{
    const WorldId wanted=World_Current();
    size_t total=0;
    if(s_prepared)return;
    s_prepared=1;
    for(size_t i=0;i<WORLD_AREA_COUNT;++i){
        s_areaOffset[i]=total;
        if(kAreas[i].size>SIZE_MAX-total){
            fprintf(stderr,"[world-switch] ERROR: overflow in static definitions\n");
            return;
        }
        total+=kAreas[i].size;
    }
    s_bankSize=total;
    s_bank[0]=(unsigned char*)malloc(total);
    s_bank[1]=(unsigned char*)malloc(total);
    if(!s_bank[0]||!s_bank[1]){
        fprintf(stderr,"[world-switch] ERROR: failed to allocate banks; keeping startup world\n");
        free(s_bank[0]);free(s_bank[1]);s_bank[0]=s_bank[1]=NULL;
        World_InstallBaked();
        DataJson_InstallCompetitionGraph();
        WorldCups_Install();
        Career_InstallCopaLibertadoresPresetEntry();
        return;
    }
    save_bank(0);
    if(World_Set("2627")==0 && modern_data_valid() && !getenv("SWOS_DATA_JSON_NO_COMP")) {
        if(!set_json_mode(1)) {
            fprintf(stderr,"[world-switch] 2627 UNAVAILABLE: cannot enable JSON teams\n");
        } else {
            World_InstallBaked();
            DataJson_InstallCompetitionGraph();
            WorldCups_Install();
            Career_InstallCopaLibertadoresPresetEntry();
            save_bank(1);
            s_modernReady=1;
        }
    } else {
        fprintf(stderr,"[world-switch] 2627 unavailable or JSON graph disabled\n");
    }
    /* Historic world: install its original Copa only AFTER capturing the
     * modern namespace, so 1996 Copa cannot leak into the 2627 registry.
     * Every restore is of static records, never an active career or fixture. */
    World_Set("9697");
    restore_bank(0);
    {
        extern void CopaBaked_Install(void);
        CopaBaked_Install();
        Career_InstallCopaLibertadoresPresetEntry();
        save_bank(0);
        fprintf(stderr,"[og-libertadores-96] historic-record=INSTALLED "
                       "entrants=20 groups=5 holder-bye=NO_NATIVE_RULE\n");
    }
    if(wanted==WORLD_2627 && s_modernReady){
        World_Set("2627");
        restore_bank(1);
        set_json_mode(1);
    } else {
        World_Set("9697");
        restore_bank(0);
        set_json_mode(0);
    }
    _teamsLoaded=0;
    _lastLoadedExtNumber=0xffu;
    _teamFileTeamCount=0;
    _teamFileDataSize=0;
    fprintf(stderr,"[world-switch] LOADING STATIC_BANKS prepared=9697:%d 2627:%d "
                   "active=%s areas=%zu bytes-per-bank=%zu\n",
            s_bank[0]!=NULL,s_modernReady,World_Tag(),
            (size_t)WORLD_AREA_COUNT,s_bankSize);
    fflush(stderr);
}

/* The NEW CAREER selector can report invalid/missing modern data without
 * changing the active world or entering a partially initialized namespace. */
int WorldSwitch_IsAvailable(const char *tag)
{
    return s_prepared && s_bank[0] &&
           ((tag && strcmp(tag,"9697")==0) ||
            (tag && strcmp(tag,"2627")==0 && s_modernReady));
}
/* NEW CAREER selector only. No live career state, save, matches, clocks,
 * runtime JSON pools or native callbacks are created/destroyed here. */
int WorldSwitch_SelectBeforeCareer(const char *tag)
{
    int modern=tag&&strcmp(tag,"2627")==0;
    const char *requested=modern?"2627":"9697";
    if(!tag || (strcmp(tag,"2627")&&strcmp(tag,"9697")) ||
       !s_bank[0] || !s_prepared || (modern && !s_modernReady)){
        fprintf(stderr,"[world-switch] REFUSED target=%s reason=NOT_PREPARED_OR_INVALID\n",
                tag?tag:"(null)");
        return 0;
    }
    if(World_Current()==(modern?WORLD_2627:WORLD_9697))
        return 1;
#if defined(SWOS_SDL_BUILD) && defined(SWOS_CAR_ZSTD)
    /* v288: the CarWriter worker reads World_Current() while it validates a
     * snapshot image; never switch the world under a job in flight. */
    (void)CarWriter_Wait();
#endif
    if(World_Set(requested)!=0)return 0;
    restore_bank(modern);
    if(!set_json_mode(modern))
        fprintf(stderr,"[world-switch] WARNING: JSON environment update failed\n");
    /* MOD w_ns_set invalidates its file cache for identical country numbers;
     * do the same here, including the cached team-count metadata. */
    _teamsLoaded=0;
    _lastLoadedExtNumber=0xffu;
    _teamFileTeamCount=0;
    _teamFileDataSize=0;
    ++s_switches;
    fprintf(stderr,"[world-switch] IN_PROCESS world=%s switches=%u "
                   "teams-cache=INVALIDATED static-registry=RESTORED "
                   "other-world=INACTIVE save=UNTOUCHED\n",
            World_Tag(),s_switches);
    /* Write the selected world immediately: waiting until normal shutdown
     * loses the user's choice after an interrupted debug session.
     * On the next boot main.c already loads [general] world automatically. */
    remember_selected_world();
    fflush(stderr);
    return 1;
}
#endif /* SWOS_SDL_BUILD */
