/* World-wide standings and knockout decisions from the ONE fixture ledger.
 * No result simulation, no old OG league-table callback and no separate clock.
 * Recomputed from the exact stored OG scores, so CAR replay is unnecessary.
 */
#ifndef EXPANDED_UNIVERSAL_STANDINGS_H
#define EXPANDED_UNIVERSAL_STANDINGS_H
#include "expanded_universal_fixture_registry.h"
#include <stdint.h>

typedef struct ExpandedUniversalStanding {
    ExpandedClubKey club;
    uint32_t played,wins,draws,losses,goals_for,goals_against,points;
} ExpandedUniversalStanding;

static inline int ExpandedUniversal_Standings(
    const ExpandedUniversalRegistry *r,ExpandedEditionKey edition,
    uint32_t stage,uint8_t points_for_win,
    ExpandedUniversalStanding *rows,uint32_t capacity,uint32_t *used)
{
    uint32_t i,n=0;
    if(used)*used=0u;
    if(!r || !rows || !used || !capacity || !stage ||
       (points_for_win!=2u && points_for_win!=3u) ||
       !r->fixtures || !r->ledger.entries ||
       r->fixture_count!=r->ledger.count ||
       r->fixture_count>r->fixture_capacity || r->world!=edition.competition.world)
        return 0;
    for(i=0;i<r->fixture_count;++i) {
        const ExpandedUniversalFixture *f=&r->fixtures[i];
        const ExpandedFixtureLedgerEntry *l=&r->ledger.entries[i];
        uint32_t ih=0,ia=0;
        if(!ExpandedFixtureKey_Equal(f->key,l->key))return 0;
        if(!ExpandedEditionKey_Equal(f->key.edition,edition) ||
           f->key.stage!=stage)continue;
        /* A standings competition must not silently absorb KO/replays.
         * The caller selects only the actual group/league stage. */
        if(f->key.replay!=0u)return 0;
        while(ih<n && !ExpandedClubKey_Equal(rows[ih].club,f->home))++ih;
        if(ih==n) {
            if(n==capacity)return 0;
            rows[n]=(ExpandedUniversalStanding){0};
            rows[n++].club=f->home;
        }
        while(ia<n && !ExpandedClubKey_Equal(rows[ia].club,f->away))++ia;
        if(ia==n) {
            if(n==capacity)return 0;
            rows[n]=(ExpandedUniversalStanding){0};
            rows[n++].club=f->away;
        }
        if(l->state==EXP_FIXTURE_CLAIMED)return 0; /* recovery is unresolved */
        if(l->state==EXP_FIXTURE_SCHEDULED)continue;
        if(l->state!=EXP_FIXTURE_PLAYED)return 0;
        ++rows[ih].played;++rows[ia].played;
        rows[ih].goals_for+=l->score.home;
        rows[ih].goals_against+=l->score.away;
        rows[ia].goals_for+=l->score.away;
        rows[ia].goals_against+=l->score.home;
        if(l->score.home>l->score.away) {
            ++rows[ih].wins;++rows[ia].losses;
            rows[ih].points+=points_for_win;
        } else if(l->score.home<l->score.away) {
            ++rows[ia].wins;++rows[ih].losses;
            rows[ia].points+=points_for_win;
        } else {
            ++rows[ih].draws;++rows[ia].draws;
            ++rows[ih].points;++rows[ia].points;
        }
    }
    *used=n;
    return 1;
}
/* 1 = winner from played original OG goals (or explicit real penalties).
 * 0 = pending or equal without a verified tiebreaker.
 * No away-goal or random draw rule is silently introduced.
 */
static inline int ExpandedUniversal_KnockoutWinner(
    const ExpandedUniversalRegistry *r,ExpandedEditionKey edition,
    uint32_t stage,uint32_t tie,ExpandedClubKey *winner)
{
    ExpandedClubKey first={0},other={0};
    uint32_t first_goals=0,other_goals=0;
    uint32_t i,seen=0;
    uint16_t pen_first=0,pen_other=0;
    if(winner)*winner=(ExpandedClubKey){0};
    if(!winner || !r || !r->fixtures || !r->ledger.entries ||
       !stage || !tie || r->world!=edition.competition.world ||
       r->fixture_count!=r->ledger.count)return 0;
    for(i=0;i<r->fixture_count;++i) {
        const ExpandedUniversalFixture *f=&r->fixtures[i];
        const ExpandedFixtureLedgerEntry *l=&r->ledger.entries[i];
        int normal,reverse;
        if(!ExpandedFixtureKey_Equal(f->key,l->key))return 0;
        if(!ExpandedEditionKey_Equal(f->key.edition,edition) ||
           f->key.stage!=stage || f->key.tie!=tie)continue;
        if(l->state!=EXP_FIXTURE_PLAYED)return 0;
        if(!seen) {first=f->home;other=f->away;}
        normal=ExpandedClubKey_Equal(f->home,first) &&
               ExpandedClubKey_Equal(f->away,other);
        reverse=ExpandedClubKey_Equal(f->home,other) &&
                ExpandedClubKey_Equal(f->away,first);
        if(!normal && !reverse)return 0;
        first_goals+=normal?l->score.home:l->score.away;
        other_goals+=normal?l->score.away:l->score.home;
        if(l->score.penalties_home || l->score.penalties_away) {
            if(l->score.penalties_home==l->score.penalties_away)return 0;
            pen_first=(uint16_t)(normal?l->score.penalties_home:
                                         l->score.penalties_away);
            pen_other=(uint16_t)(normal?l->score.penalties_away:
                                         l->score.penalties_home);
        }
        ++seen;
    }
    if(!seen)return 0;
    if(first_goals==other_goals && pen_first==pen_other)return 0;
    *winner=first_goals>other_goals?first:
            first_goals<other_goals?other:
            pen_first>pen_other?first:other;
    return 1;
}
#endif
