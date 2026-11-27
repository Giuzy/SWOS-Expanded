#ifndef EXPANDED_CE_RANKING_H
#define EXPANDED_CE_RANKING_H
/* CE 2627 only. Pure calculations over certified inputs, no alternate
 * result writer, no RNG or global state. UEFA values in thousandths.
 * Caller owns fixture identity, effective rules, chronology and CAR. */
#include <stdint.h>
#include <math.h>
#include <limits.h>
typedef enum ExpCeRankResult {
    EXP_RANK_A_WIN=1, EXP_RANK_DRAW, EXP_RANK_B_WIN,
    EXP_RANK_A_PSO, EXP_RANK_B_PSO
} ExpCeRankResult;
typedef enum ExpCeFifaClass {
    EXP_FIFA_FRIENDLY_OUT=5, EXP_FIFA_FRIENDLY_IN=10,
    EXP_FIFA_NATIONS_GROUP=15, EXP_FIFA_NATIONS_FINAL=25,
    EXP_FIFA_QUALIFIER=25, EXP_FIFA_CONTINENTAL_EARLY=35,
    EXP_FIFA_CONTINENTAL_LATE=40, EXP_FIFA_WC_EARLY=50,
    EXP_FIFA_WC_LATE=60
} ExpCeFifaClass;
/* Both teams use their PRE-match points. PSO only for a drawn individual
 * match, not a non-drawn second leg with shoot-out on aggregate.
 * protect: knockout round of a final competition, not a qualifier. */
static inline int ExpCeFifa_Sum(double a,double b,int importance,
    ExpCeRankResult result,int protect,double *next_a,double *next_b)
{
    double wa,wb,ea,da,db,na,nb;
    if(!next_a||!next_b||next_a==next_b||!isfinite(a)||!isfinite(b)||
       (protect!=0&&protect!=1))return 0;
    switch(importance){
    case 5:case 10:case 15:case 25:case 35:case 40:case 50:case 60:break;
    default:return 0;
    }
    switch(result){
    case EXP_RANK_A_WIN:wa=1;wb=0;break;
    case EXP_RANK_DRAW:wa=wb=.5;break;
    case EXP_RANK_B_WIN:wa=0;wb=1;break;
    case EXP_RANK_A_PSO:wa=.75;wb=.5;break;
    case EXP_RANK_B_PSO:wa=.5;wb=.75;break;
    default:return 0;
    }
    ea=1.0/(1.0+pow(10.0,(b-a)/600.0));
    da=importance*(wa-ea);db=importance*(wb-(1.0-ea));
    if(protect){if(da<0)da=0;if(db<0)db=0;}
    na=a+da;nb=b+db;
    if(!isfinite(na)||!isfinite(nb))return 0;
    *next_a=na;*next_b=nb;return 1;
}
typedef enum ExpCeUefaPhase {
    EXP_UEFA_QUALIFYING=1, EXP_UEFA_LEAGUE,
    EXP_UEFA_KO_PLAYOFF, EXP_UEFA_KO, EXP_UEFA_FINAL
} ExpCeUefaPhase;
typedef enum ExpCeUefaCompetition {
    EXP_UEFA_UCL=1, EXP_UEFA_UEL, EXP_UEFA_UECL
} ExpCeUefaCompetition;
typedef struct ExpCeUefaMatchPoints {
    uint32_t club_a,club_b,association_a,association_b;
} ExpCeUefaMatchPoints;
/* Annex D.3/4/7. Qualifying includes preliminary playoffs, NOT the
 * knockout phase playoffs. The latter give association points, but no club
 * points. changed_leg applies only to a converted two-leg round. */
static inline int ExpCeUefa_Match2627(ExpCeUefaPhase phase,int changed_leg,
    ExpCeRankResult result,ExpCeUefaMatchPoints *out)
{
    uint32_t w,d,l,a,b;ExpCeUefaMatchPoints p;
    if(!out||phase<EXP_UEFA_QUALIFYING||phase>EXP_UEFA_FINAL||
       (changed_leg!=0&&changed_leg!=1)||
       (changed_leg&&(phase==EXP_UEFA_LEAGUE||phase==EXP_UEFA_FINAL)))return 0;
    w=changed_leg?3000u:2000u;d=changed_leg?2000u:1000u;l=changed_leg?1000u:0u;
    switch(result){
    case EXP_RANK_A_WIN:a=w;b=l;break;
    case EXP_RANK_B_WIN:a=l;b=w;break;
    case EXP_RANK_DRAW:case EXP_RANK_A_PSO:case EXP_RANK_B_PSO:a=b=d;break;
    default:return 0;
    }
    p.association_a=a;p.association_b=b;p.club_a=a;p.club_b=b;
    if(phase==EXP_UEFA_QUALIFYING){
        p.association_a/=2u;p.association_b/=2u;p.club_a=p.club_b=0;
    }else if(phase==EXP_UEFA_KO_PLAYOFF)p.club_a=p.club_b=0;
    *out=p;return 1;
}
/* D.5: final league rank; both club and association. */
static inline int ExpCeUefa_LeagueBonus2627(ExpCeUefaCompetition comp,
    unsigned rank,uint32_t *points)
{
    uint32_t p;
    if(!points||rank<1||rank>36)return 0;
    switch(comp){
    case EXP_UEFA_UCL:p=rank<=24?12000u-250u*(rank-1u):6000u;break;
    case EXP_UEFA_UEL:p=rank<=24?6000u-250u*(rank-1u):0u;break;
    case EXP_UEFA_UECL:p=rank<=9?4000u-250u*(rank-1u):
        rank<=24?2000u-125u*(rank-9u):0u;break;
    default:return 0;
    }
    *points=p;return 1;
}
static inline int ExpCeUefa_ReachBonus2627(ExpCeUefaCompetition comp,uint32_t *p)
{
    if(!p||comp<EXP_UEFA_UCL||comp>EXP_UEFA_UECL)return 0;
    *p=2000u-500u*(unsigned)comp;return 1;
}
/* Club only; terminal UECL elimination Q1/Q2/Q3/qualifying playoff. */
static inline int ExpCeUefa_Eliminated2627(unsigned round,uint32_t *p)
{
    if(!p||round<1||round>4)return 0;
    *p=500u+500u*round;return 1;
}
/* Club only, minimum not additive; calculate once before league bonuses. */
static inline int ExpCeUefa_LeagueFloor2627(ExpCeUefaCompetition comp,
    uint32_t league_points,uint32_t *adjustment)
{
    uint32_t floor;
    if(!adjustment||comp<EXP_UEFA_UCL||comp>EXP_UEFA_UECL)return 0;
    floor=comp==EXP_UEFA_UEL?3000u:comp==EXP_UEFA_UECL?2500u:0u;
    *adjustment=league_points<floor?floor-league_points:0u;return 1;
}
/* D.3: certified participation/entitlement divisor, never fixture count.
 * D.7 truncate, never round upwards. */
static inline int ExpCeUefa_AssociationSeason(uint64_t points,unsigned clubs,
    uint32_t *coefficient)
{
    if(!clubs||!coefficient||points/clubs>UINT32_MAX)return 0;
    *coefficient=(uint32_t)(points/clubs);return 1;
}
typedef struct ExpCeUefaSeasonValue {
    unsigned year; /* season start year */
    uint32_t points;
    uint8_t known; /* 1 = certified, including explicit zero */
} ExpCeUefaSeasonValue;
/* Unknown/duplicate seasons must not become zero. Reference year is
 * caller-supplied: access list 2026/27 ends 2024, club seeding ends 2025. */
static inline int ExpCeUefa_Five(const ExpCeUefaSeasonValue *v,unsigned n,
    unsigned reference_end,uint64_t *total)
{
    unsigned i,j;uint64_t sum=0;
    if(!v||!total||reference_end<4)return 0;
    for(i=0;i<5;i++){
        unsigned year=reference_end-i,count=0;uint32_t p=0;
        for(j=0;j<n;j++)if(v[j].year==year){
            if(v[j].known!=1||++count!=1)return 0;
            p=v[j].points;
        }
        if(count!=1)return 0;
        sum+=p;
    }
    *total=sum;return 1;
}
static inline uint64_t ExpCeUefa_ClubModern(uint64_t club,uint64_t assoc)
{
    return club>assoc/5u?club:assoc/5u;
}
/* D.8: latest season first; caller resolves remaining association and
 * domestic-position tie breakers. Arrays must contain certified values. */
static inline int ExpCeUefa_RecentCompare(const uint32_t a[5],const uint32_t b[5])
{
    unsigned i;
    for(i=0;i<5;i++)if(a[i]!=b[i])return a[i]>b[i]?1:-1;
    return 0;
}
#endif
