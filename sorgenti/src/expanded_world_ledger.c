#include "expanded_world_ledger.h"
#include "expanded_world_time.h"

static int validday(int32_t day) {
    ExpandedWorldDate d;
    return ExpandedWorld_IndexToDate(day, &d);
}
static int scoreok(uint8_t h, uint8_t a) { return h <= 31 && a <= 31; }
static int before(const ExpandedWorldFixture *a, const ExpandedWorldFixture *b) {
    if (a->day != b->day) return a->day < b->day;
    if (a->competition != b->competition) return a->competition < b->competition;
    return a->id < b->id;
}
int ExpandedWorldLedger_Init(ExpandedWorldLedger *l,
                             ExpandedWorldFixture *storage, uint32_t capacity,
                             int32_t start_day) {
    if (!l || !storage || !capacity || !validday(start_day)) return 0;
    l->fixtures=storage; l->count=0; l->capacity=capacity;
    l->played=0; l->now=start_day;
    return 1;
}
int ExpandedWorldLedger_Add(ExpandedWorldLedger *l, ExpandedWorldFixture f) {
    uint32_t i;
    if (!l || !l->fixtures || l->count >= l->capacity || f.played ||
        !validday(f.day) || f.day < l->now || f.home == f.away) return 0;
    for (i=0; i<l->count; ++i)
        if (l->fixtures[i].competition == f.competition &&
            l->fixtures[i].id == f.id) return 0;
    f.goals_home=0; f.goals_away=0;
    i=l->count;
    while (i && before(&f,&l->fixtures[i-1])) {
        l->fixtures[i]=l->fixtures[i-1]; --i;
    }
    l->fixtures[i]=f; ++l->count;
    return 1;
}
int ExpandedWorldLedger_Advance(ExpandedWorldLedger *l, int32_t target,
                                ExpandedWorldResolve resolve, void *context) {
    uint32_t i;
    if (!l || !l->fixtures || !resolve || target < l->now || !validday(target)) return 0;
    for (i=0; i<l->count && l->fixtures[i].day<=target; ++i) {
        ExpandedWorldFixture *f=&l->fixtures[i];
        uint8_t home=0, away=0;
        if (f->played) continue;
        if (!resolve(f,&home,&away,context) || !scoreok(home,away)) {
            if (f->day > l->now) l->now=f->day;
            return 0;
        }
        f->played=1; f->goals_home=home; f->goals_away=away;
        l->played++;
    }
    l->now=target;
    return 1;
}
int ExpandedWorldLedger_Record(ExpandedWorldLedger *l, uint16_t competition,
                               uint32_t id, uint8_t hg, uint8_t ag) {
    uint32_t i;
    if (!l || !scoreok(hg,ag)) return 0;
    for (i=0; i<l->count; ++i) {
        ExpandedWorldFixture *f=&l->fixtures[i];
        if (f->competition==competition && f->id==id &&
            !f->played && f->day <= l->now) {
            f->played=1; f->goals_home=hg; f->goals_away=ag;
            ++l->played; return 1;
        }
    }
    return 0;
}
uint32_t ExpandedWorldLedger_NextPending(const ExpandedWorldLedger *l) {
    uint32_t i;
    if (!l || !l->fixtures) return UINT32_MAX;
    for (i=0; i<l->count; ++i)
        if (!l->fixtures[i].played) return i;
    return UINT32_MAX;
}
int ExpandedWorldLedger_Standings(const ExpandedWorldLedger *l,
                                  uint16_t comp, ExpandedWorldStanding *table,
                                  uint32_t clubs, uint8_t points_for_win) {
    uint32_t i,j;
    if (!l || !table || !clubs || points_for_win<2 || points_for_win>3) return 0;
    for (i=0; i<clubs; ++i) {
        table[i].played=table[i].wins=table[i].drawn=table[i].lost=0;
        table[i].scored=table[i].conceded=0;
        table[i].points=0;
        for (j=0; j<i; ++j) if (table[j].club==table[i].club) return 0;
    }
    for (i=0; i<l->count; ++i) {
        const ExpandedWorldFixture *f=&l->fixtures[i];
        uint32_t h=clubs,a=clubs;
        if (!f->played || f->competition!=comp) continue;
        for (j=0; j<clubs; ++j) {
            if (table[j].club==f->home) h=j;
            if (table[j].club==f->away) a=j;
        }
        if (h==clubs || a==clubs || h==a) return 0;
        table[h].played++; table[a].played++;
        table[h].scored+=f->goals_home; table[h].conceded+=f->goals_away;
        table[a].scored+=f->goals_away; table[a].conceded+=f->goals_home;
        if (f->goals_home > f->goals_away) {
            table[h].wins++; table[h].points+=points_for_win; table[a].lost++;
        } else if (f->goals_home < f->goals_away) {
            table[a].wins++; table[a].points+=points_for_win; table[h].lost++;
        } else {
            table[h].drawn++; table[a].drawn++;
            table[h].points++; table[a].points++;
        }
    }
    return 1;
}
