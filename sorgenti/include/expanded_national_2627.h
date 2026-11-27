#ifndef EXPANDED_NATIONAL_2627_H
#define EXPANDED_NATIONAL_2627_H
/* Immutable modern identity source. Historic random draw table unchanged.
 * New global IDs extend old 2560 range; source FIFA_RANKING_NATIONAL_ID_MAP_2627.json. */
#include "game_types.h"
#include "world.h"
#include <string.h>
typedef struct ExpandedNationalRow {
    NationalTeamRecord og;
    uint16_t added_global;
    const char *fifa,*name,*native_name;
    uint8_t confederationRegion; /* modern sporting group, independent of OG storage file */
} ExpandedNationalRow;
#define EXP_NATIONAL_COUNT 211u
#define EXP_NATIONAL_ADDED 69u
static const ExpandedNationalRow kExpandedNational2627[EXP_NATIONAL_COUNT]={
    {{80,0,0,0,4864},65535,"ALB","ALBANIA","",80},
    {{80,1,41,52,3584},65535,"ARM","ARMENIA","",80},
    {{80,2,1,1,6144},65535,"AUT","AUSTRIA","",80},
    {{80,3,43,53,3584},65535,"AZE","AZERBAIJAN","",80},
    {{80,4,38,76,4608},65535,"BLR","BELARUS","",80},
    {{80,5,2,2,7680},65535,"BEL","BELGIUM","",80},
    {{80,6,3,3,7168},65535,"BUL","BULGARIA","",80},
    {{80,7,4,4,7424},65535,"CRO","CROATIA","",80},
    {{80,8,5,5,4096},65535,"CYP","CYPRUS","",80},
    {{80,9,6,6,6656},65535,"CZE","CZECHIA","",80},
    {{80,10,7,7,7168},65535,"DEN","DENMARK","",80},
    {{80,11,8,8,8704},65535,"ENG","ENGLAND","",80},
    {{80,12,9,10,3584},65535,"EST","ESTONIA","",80},
    {{80,13,10,11,2816},65535,"FRO","FAROE ISLANDS","",80},
    {{80,14,11,12,5376},65535,"FIN","FINLAND","",80},
    {{80,15,12,13,8192},65535,"FRA","FRANCE","",80},
    {{80,16,44,61,4864},65535,"GEO","GEORGIA","",80},
    {{80,17,13,14,8960},65535,"GER","GERMANY","",80},
    {{80,18,14,15,6144},65535,"GRE","GREECE","",80},
    {{80,19,23,25,8448},65535,"NED","NETHERLANDS","",80},
    {{80,20,15,16,5632},65535,"HUN","HUNGARY","",80},
    {{80,21,16,17,4352},65535,"ISL","ICELAND","",80},
    {{80,22,17,19,4608},65535,"ISR","ISRAEL","",80},
    {{80,23,18,20,9472},65535,"ITA","ITALY","",80},
    {{80,24,19,21,3584},65535,"LVA","LATVIA","",80},
    {{80,25,49,90,2816},65535,"LIE","LIECHTENSTEIN","",80},
    {{80,26,20,22,4608},65535,"LTU","LITHUANIA","",80},
    {{80,27,21,23,3584},65535,"LUX","LUXEMBOURG","",80},
    {{80,28,47,93,5120},65535,"MKD","NORTH MACEDONIA","",80},
    {{80,29,22,24,3840},65535,"MLT","MALTA","",80},
    {{80,30,50,92,4352},65535,"MDA","MOLDOVA","",80},
    {{80,31,24,26,5120},65535,"NIR","NORTHERN IRELAND","",80},
    {{80,32,25,27,6144},65535,"NOR","NORWAY","",80},
    {{80,33,26,28,6144},65535,"POL","POLAND","",80},
    {{80,34,27,29,8192},65535,"POR","PORTUGAL","",80},
    {{80,35,46,18,7680},65535,"IRL","REPUBLIC OF IRELAND","",80},
    {{80,36,28,30,7936},65535,"ROU","ROMANIA","",80},
    {{80,37,29,31,7680},65535,"RUS","RUSSIA","",80},
    {{80,38,30,32,3072},65535,"SMR","SAN MARINO","",80},
    {{80,39,31,33,7424},65535,"SCO","SCOTLAND","",80},
    {{80,40,39,78,5888},65535,"SVK","SLOVAKIA","",80},
    {{80,41,32,34,4864},65535,"SVN","SLOVENIA","",80},
    {{80,42,40,35,8448},65535,"ESP","SPAIN","",80},
    {{80,43,33,36,7680},65535,"SWE","SWEDEN","",80},
    {{80,44,45,37,7168},65535,"SUI","SWITZERLAND","",80},
    {{80,45,34,38,5888},65535,"TUR","TURKEY","",80},
    {{80,46,35,39,5120},65535,"UKR","UKRAINE","",80},
    {{80,47,36,40,7168},65535,"WAL","WALES","",80},
    {{80,48,37,41,0},65535,"SRB","SERBIA","",80},
    {{80,49,158,47,0},2560,"AND","ANDORRA","ANDORRA",80},
    {{80,50,42,54,0},2561,"BIH","BOSNIA AND HERZEGOVINA","BOSNIA-HERZ.",80},
    {{80,51,159,63,0},2562,"GIB","GIBRALTAR","GIBRALTAR",80},
    {{80,52,160,86,0},2563,"KAZ","KAZAKHSTAN","KAZAKHSTAN",80},
    {{80,53,157,88,0},2564,"KOS","KOSOVO","KOSOVO",80},
    {{80,54,156,94,0},2565,"MNE","MONTENEGRO","MONTENEGRO",80},
    {{81,0,80,42,2304},65535,"ALG","ALGERIA","",81},
    {{81,1,102,255,2304},65535,"ANG","ANGOLA","",81},
    {{81,2,111,255,768},65535,"BEN","BENIN","",81},
    {{81,3,82,255,768},65535,"BOT","BOTSWANA","",81},
    {{81,4,83,255,256},65535,"BFA","BURKINA FASO","",81},
    {{81,5,84,255,2816},65535,"BDI","BURUNDI","",81},
    {{81,6,94,255,4352},65535,"CMR","CAMEROON","",81},
    {{81,7,112,255,1536},65535,"CGO","CONGO","",81},
    {{81,8,105,59,4352},65535,"EGY","EGYPT","",81},
    {{81,9,108,255,2304},65535,"ETH","ETHIOPIA","",81},
    {{81,10,109,255,3584},65535,"GAB","GABON","",81},
    {{81,11,88,79,4352},65535,"GHA","GHANA","",81},
    {{81,12,113,255,3584},65535,"GUI","GUINEA","",81},
    {{81,13,90,255,4864},65535,"CIV","IVORY COAST","",81},
    {{81,14,99,255,2048},65535,"KEN","KENYA","",81},
    {{81,15,85,255,1536},65535,"LES","LESOTHO","",81},
    {{81,16,97,255,3584},65535,"LBR","LIBERIA","",81},
    {{81,17,117,255,2816},65535,"MWI","MALAWI","",81},
    {{81,18,92,255,3584},65535,"MLI","MALI","",81},
    {{81,19,115,91,4096},65535,"MAR","MOROCCO","",81},
    {{81,20,98,255,2304},65535,"MOZ","MOZAMBIQUE","",81},
    {{81,21,182,255,256},65535,"NIG","NIGER","",81},
    {{81,22,107,255,4864},65535,"NGA","NIGERIA","",81},
    {{81,23,89,255,5888},65535,"SEN","SENEGAL","",81},
    {{81,24,110,255,1536},65535,"SLE","SIERRA LEONE","",81},
    {{81,25,81,69,4352},65535,"RSA","SOUTH AFRICA","",81},
    {{81,26,100,255,2816},65535,"SDN","SUDAN","",81},
    {{81,27,101,255,1536},65535,"SWZ","ESWATINI","",81},
    {{81,28,106,255,2048},65535,"TAN","TANZANIA","",81},
    {{81,29,103,255,2048},65535,"TOG","TOGO","",81},
    {{81,30,91,97,256},65535,"TUN","TUNISIA","",81},
    {{81,31,96,255,1792},65535,"UGA","UGANDA","",81},
    {{81,32,87,255,4864},65535,"ZAM","ZAMBIA","",81},
    {{81,33,104,255,2816},65535,"ZIM","ZIMBABWE","",81},
    {{81,34,95,255,0},2566,"CHA","CHAD","CHAD",81},
    {{81,35,86,255,0},2567,"COD","DR CONGO","DR CONGO",81},
    {{81,36,184,255,0},2568,"COM","COMOROS","COMOROS",81},
    {{81,37,178,255,0},2569,"CPV","CABO VERDE","CABO VERDE",81},
    {{81,38,185,255,0},2570,"CTA","CENTRAL AFRICAN REPUBLIC","CENTRAL AFR. REP",81},
    {{81,39,191,255,0},2571,"DJI","DJIBOUTI","DJIBOUTI",81},
    {{81,40,180,255,0},2572,"EQG","EQUATORIAL GUINEA","EQUAT. GUINEA",81},
    {{81,41,187,255,0},2573,"ERI","ERITREA","ERITREA",81},
    {{81,42,116,255,0},2574,"GAM","GAMBIA","GAMBIA",81},
    {{81,43,179,255,0},2575,"GNB","GUINEA-BISSAU","GUINEA-BISSAU",81},
    {{81,44,190,255,0},2576,"LBY","LIBYA","LIBYA",81},
    {{81,45,93,255,0},2577,"MAD","MADAGASCAR","MADAGASCAR",81},
    {{81,46,181,255,0},2578,"MTN","MAURITANIA","MAURITANIA",81},
    {{81,47,189,255,0},2579,"NAM","NAMIBIA","NAMIBIA",81},
    {{81,48,183,255,0},2580,"RWA","RWANDA","RWANDA",81},
    {{81,49,219,255,0},2581,"SEY","SEYCHELLES","SEYCHELLES",81},
    {{81,50,188,255,0},2582,"SOM","SOMALIA","SOMALIA",81},
    {{81,51,186,255,0},2583,"SSD","SOUTH SUDAN","SOUTH SUDAN",81},
    {{81,52,192,255,0},2584,"STP","SAO TOME AND PRINCIPE","SAO TOME & PRIN.",81},
    {{82,0,68,43,8192},65535,"ARG","ARGENTINA","",82},
    {{82,1,69,45,6144},65535,"BOL","BOLIVIA","",82},
    {{82,2,70,46,9216},65535,"BRA","BRAZIL","",82},
    {{82,3,71,48,5888},65535,"CHI","CHILE","",82},
    {{82,4,72,49,7168},65535,"COL","COLOMBIA","",82},
    {{82,5,73,50,4352},65535,"ECU","ECUADOR","",82},
    {{82,6,74,64,4096},65535,"PAR","PARAGUAY","",82},
    {{82,7,79,65,4352},65535,"PER","PERU","",82},
    {{82,8,76,71,7168},65535,"URU","URUGUAY","",82},
    {{82,9,77,77,2560},65535,"VEN","VENEZUELA","",82},
    {{83,0,61,255,256},65535,"BER","BERMUDA","",83},
    {{83,1,64,56,4096},65535,"CAN","CANADA","",83},
    {{83,2,51,58,4352},65535,"CRC","COSTA RICA","",83},
    {{83,3,52,51,2816},65535,"SLV","EL SALVADOR","",83},
    {{83,4,53,68,2048},65535,"GUA","GUATEMALA","",83},
    {{83,5,78,255,256},65535,"GUY","GUYANA","",83},
    {{83,6,54,70,768},65535,"HON","HONDURAS","",83},
    {{83,7,62,255,256},65535,"JAM","JAMAICA","",83},
    {{83,8,56,60,4864},65535,"MEX","MEXICO","",83},
    {{83,9,60,255,256},65535,"NCA","NICARAGUA","",83},
    {{83,10,57,95,2304},65535,"PAN","PANAMA","",83},
    {{83,11,67,255,256},65535,"VIN","ST VINCENT AND THE GRENADINES","",83},
    {{83,12,75,66,768},65535,"SUR","SURINAME","",83},
    {{83,13,58,73,5120},65535,"USA","UNITED STATES","",83},
    {{83,14,223,255,0},2585,"AIA","ANGUILLA","ANGUILLA",83},
    {{83,15,165,255,0},2586,"ARU","ARUBA","ARUBA",83},
    {{83,16,173,255,0},2587,"ATG","ANTIGUA AND BARBUDA","ANTIGUA & BARB.",83},
    {{83,17,55,255,0},2588,"BAH","BAHAMAS","BAHAMAS",83},
    {{83,18,162,255,0},2589,"BLZ","BELIZE","BELIZE",83},
    {{83,19,65,255,0},2590,"BRB","BARBADOS","BARBADOS",83},
    {{83,20,177,255,0},2591,"CAY","CAYMAN ISLANDS","CAYMAN ISLANDS",83},
    {{83,21,167,255,0},2592,"CUB","CUBA","CUBA",83},
    {{83,22,164,255,0},2593,"CUW","CURACAO","CURACAO",83},
    {{83,23,215,255,0},2594,"DMA","DOMINICA","DOMINICA",83},
    {{83,24,166,255,0},2595,"DOM","DOMINICAN REPUBLIC","DOMINICAN REP.",83},
    {{83,25,169,255,0},2596,"GRN","GRENADA","GRENADA",83},
    {{83,26,163,255,0},2597,"HAI","HAITI","HAITI",83},
    {{83,27,175,255,0},2598,"LCA","ST LUCIA","ST LUCIA",83},
    {{83,28,176,255,0},2599,"MSR","MONTSERRAT","MONTSERRAT",83},
    {{83,29,168,255,0},2600,"PUR","PUERTO RICO","PUERTO RICO",83},
    {{83,30,174,255,0},2601,"SKN","ST KITTS AND NEVIS","ST KITTS & NEVIS",83},
    {{83,31,220,255,0},2602,"TCA","TURKS AND CAICOS ISLANDS","TURKS & CAICOS",83},
    {{83,32,63,255,0},2603,"TRI","TRINIDAD AND TOBAGO","TRINIDAD & TOB.",83},
    {{83,33,222,255,0},2604,"VGB","BRITISH VIRGIN ISLANDS","BRITISH VIRGIN",83},
    {{83,34,221,255,0},2605,"VIR","US VIRGIN ISLANDS","US VIRGIN IS.",83},
    {{84,0,197,255,1536},65535,"BHR","BAHRAIN","",84},
    {{84,1,121,255,512},65535,"BAN","BANGLADESH","",84},
    {{84,2,122,255,1280},65535,"BRU","BRUNEI","",84},
    {{84,3,139,57,3072},65535,"CHN","CHINA","",84},
    {{84,4,198,255,2304},65535,"HKG","HONG KONG","",84},
    {{84,5,120,75,2304},65535,"IND","INDIA","",84},
    {{84,6,200,255,2048},65535,"IDN","INDONESIA","",84},
    {{84,7,128,74,2816},65535,"IRN","IRAN","",84},
    {{84,8,123,255,2304},65535,"IRQ","IRAQ","",84},
    {{84,9,118,55,3072},65535,"JPN","JAPAN","",84},
    {{84,10,124,255,1536},65535,"JOR","JORDAN","",84},
    {{84,11,133,255,3584},65535,"KUW","KUWAIT","",84},
    {{84,12,134,255,256},65535,"LAO","LAOS","",84},
    {{84,13,130,255,1536},65535,"MAS","MALAYSIA","",84},
    {{84,14,141,255,2304},65535,"MRI","MAURITIUS","",81},
    {{84,15,142,255,1536},65535,"MYA","MYANMAR","",84},
    {{84,16,135,255,2048},65535,"PRK","NORTH KOREA","",84},
    {{84,17,136,255,1536},65535,"OMA","OMAN","",84},
    {{84,18,137,255,1536},65535,"PAK","PAKISTAN","",84},
    {{84,19,143,255,256},65535,"PNG","PAPUA NEW GUINEA","",85},
    {{84,20,138,255,768},65535,"PHI","PHILIPPINES","",84},
    {{84,21,146,96,2304},65535,"QAT","QATAR","",84},
    {{84,22,131,89,3840},65535,"KSA","SAUDI ARABIA","",84},
    {{84,23,140,255,2304},65535,"SGP","SINGAPORE","",84},
    {{84,24,127,87,4352},65535,"KOR","SOUTH KOREA","",84},
    {{84,25,125,255,768},65535,"SRI","SRI LANKA","",84},
    {{84,26,126,255,2816},65535,"SYR","SYRIA","",84},
    {{84,27,119,67,2304},65535,"TPE","CHINESE TAIPEI","",84},
    {{84,28,147,98,3584},65535,"UAE","UNITED ARAB EMIRATES","",84},
    {{84,29,129,255,1536},65535,"VIE","VIETNAM","",84},
    {{84,30,132,255,1280},65535,"YEM","YEMEN","",84},
    {{84,31,202,255,0},2606,"AFG","AFGHANISTAN","AFGHANISTAN",84},
    {{84,32,218,255,0},2607,"BHU","BHUTAN","BHUTAN",84},
    {{84,33,201,255,0},2608,"CAM","CAMBODIA","CAMBODIA",84},
    {{84,34,208,255,0},2609,"GUM","GUAM","GUAM",84},
    {{84,35,161,255,0},2610,"KGZ","KYRGYZSTAN","KYRGYZSTAN",84},
    {{84,36,195,255,0},2611,"LBN","LEBANON","LEBANON",84},
    {{84,37,204,255,0},2612,"MAC","MACAU","MACAU",84},
    {{84,38,206,255,0},2613,"MDV","MALDIVES","MALDIVES",84},
    {{84,39,205,255,0},2614,"MNG","MONGOLIA","MONGOLIA",84},
    {{84,40,203,255,0},2615,"NEP","NEPAL","NEPAL",84},
    {{84,41,196,255,0},2616,"PLE","PALESTINE","PALESTINE",84},
    {{84,42,199,255,0},2617,"THA","THAILAND","THAILAND",84},
    {{84,43,144,255,0},2618,"TJK","TAJIKISTAN","TAJIKISTAN",84},
    {{84,44,214,255,0},2619,"TKM","TURKMENISTAN","TURKMENISTAN",84},
    {{84,45,207,255,0},2620,"TLS","TIMOR-LESTE","TIMOR-LESTE",84},
    {{84,46,145,255,0},2621,"UZB","UZBEKISTAN","UZBEKISTAN",84},
    {{85,0,148,44,4864},65535,"AUS","AUSTRALIA","",84},
    {{85,1,150,255,256},65535,"FIJ","FIJI","",85},
    {{85,2,149,62,3584},65535,"NZL","NEW ZEALAND","",85},
    {{85,3,151,255,256},65535,"SOL","SOLOMON ISLANDS","",85},
    {{85,4,217,255,0},2622,"ASA","AMERICAN SAMOA","AMERICAN SAMOA",85},
    {{85,5,216,255,0},2623,"COK","COOK ISLANDS","COOK ISLANDS",85},
    {{85,6,209,255,0},2624,"NCL","NEW CALEDONIA","NEW CALEDONIA",85},
    {{85,7,212,255,0},2625,"SAM","SAMOA","SAMOA",85},
    {{85,8,210,255,0},2626,"TAH","TAHITI","TAHITI",85},
    {{85,9,213,255,0},2627,"TGA","TONGA","TONGA",85},
    {{85,10,211,255,0},2628,"VAN","VANUATU","VANUATU",85},
};
static inline int ExpandedNational_Active(void)
{
#if SWOS_WORLDS
    return World_Current()==WORLD_2627;
#else
    return 0;
#endif
}
static inline uint16_t ExpandedNational_Raw(const ExpandedNationalRow *r)
{return (uint16_t)(((uint16_t)r->og.countryNumber<<8)|r->og.teamOrdinal);}
static inline const ExpandedNationalRow *ExpandedNational_ByRaw(uint16_t raw)
{
    if((raw>>8)<80u || (raw>>8)>85u || !ExpandedNational_Active())return NULL;
    for(unsigned i=0;i<EXP_NATIONAL_COUNT;i++)
        if(ExpandedNational_Raw(&kExpandedNational2627[i])==raw)return &kExpandedNational2627[i];
    return NULL;
}
static inline const ExpandedNationalRow *ExpandedNational_ByNationality(uint16_t nat)
{
    if(!ExpandedNational_Active() || nat>=255u)return NULL;
    if(nat==59u)nat=197u; /* historic Bahrain code, not Bahamas */
    if(nat==66u)nat=52u; /* ELS/SAL */
    for(unsigned i=0;i<EXP_NATIONAL_COUNT;i++)
        if(kExpandedNational2627[i].og.nationalityCode==nat)return &kExpandedNational2627[i];
    return NULL;
}
static inline const ExpandedNationalRow *ExpandedNational_ByHome(uint16_t home)
{
    if(!ExpandedNational_Active() || home>=255u)return NULL;
    for(unsigned i=0;i<EXP_NATIONAL_COUNT;i++)
        if(kExpandedNational2627[i].og.teamFileId==home)return &kExpandedNational2627[i];
    return NULL;
}
/* WORLD club navigation only. Use the same modern confederation as
 * national WORLD, without changing native home IDs or the source lists.
 * Source lists are the existing sentinel-terminated world_data.c arrays. */
static inline int ExpandedNational_WorldClubCountries(
    uint8_t region,const uint8_t *const sources[6],uint8_t *out,unsigned cap)
{
    if(!ExpandedNational_Active() || region<80u || region>85u || !sources || !out || !cap)return -1;
    unsigned count=0;
    uint8_t seen[256]={0};
    /* Transferred countries first; preserve order of all retained rows. */
    for(unsigned pass=0;pass<2u;pass++){
        for(unsigned i=0;i<6u;i++){
            if((i==region-80u)!=(pass==1u))continue;
            const uint8_t *src=sources[i];
            if(!src)return -1;
            for(;*src!=255u;src++){
                uint8_t home=*src;
                if(seen[home]++)return -1;
                const ExpandedNationalRow *r=ExpandedNational_ByHome(home);
                if((r?r->confederationRegion:i+80u)!=region)continue;
                if(count+1u>=cap)return -1;
                out[count++]=home;
            }
        }
    }
    out[count]=255u;
    return (int)count;
}
/* Historical off-region national-only rows use the existing inactive
 * WORLD club button path, just like the newly catalogued countries. */
static inline int ExpandedNational_WorldInactiveClubRow(const ExpandedNationalRow *r,int includeExisting)
{
    return r && r->og.teamFileId==255u &&
        (includeExisting || r->added_global!=65535u || r->confederationRegion!=r->og.countryNumber);
}
/* One validation gate for the modern JSON source; never synthesize a roster.
 * File/ordinal and nationality are Expanded identities, not upstream ordering.
 * The existing OG global-number assignment still owns the old 142 slots. */
static inline int ExpandedNational_IsModernFile(int file)
{
    return ExpandedNational_Active() && file>=80 && file<=85;
}
static inline int ExpandedNational_Validate(int file,const uint8_t *buf,int count,int cap)
{
    unsigned expected=0;
    uint8_t seen[64]={0};
    if(!ExpandedNational_IsModernFile(file))return count;
    if(!buf || count<=0 || count>cap || count>64)return -1;
    for(unsigned i=0;i<EXP_NATIONAL_COUNT;i++)
        if(kExpandedNational2627[i].og.countryNumber==(unsigned)file)expected++;
    if((unsigned)count!=expected)return -1;
    const TeamFileEx *t=(const TeamFileEx *)(buf+2);
    for(int j=0;j<count;j++){
        unsigned ordinal=t[j].team.teamOrdinal;
        if(t[j].team.countryNumber!=(unsigned)file || ordinal>=64u || seen[ordinal] ||
           !t[j].team.teamName[0] || t[j].team.league!=0u)return -1;
        const ExpandedNationalRow *r=ExpandedNational_ByRaw((uint16_t)(((unsigned)file<<8)|ordinal));
        if(!r)return -1;
        if(r->added_global!=65535u &&
           (uint16_t)t[j].team.globalTeamNumber!=r->added_global)return -1;
        seen[ordinal]=1;
        unsigned lineup=0;
        for(unsigned p=0;p<16u;p++){
            const PlayerFile *v=&t[j].players[p];
            unsigned slot=(uint8_t)t[j].team.playerNumbers[p];
            if(slot>=16u || (lineup&(1u<<slot)) || !v->playerName[0] ||
               (!memcmp(v->playerName,"TEST",4u) && !v->playerName[4]) ||
               (uint8_t)v->nationality!=r->og.nationalityCode)return -1;
            lineup|=1u<<slot;
        }
    }
    return count;
}
#endif
