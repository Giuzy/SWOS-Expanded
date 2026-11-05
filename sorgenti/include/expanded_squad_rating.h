#ifndef EXPANDED_SQUAD_RATING_H
#define EXPANDED_SQUAD_RATING_H
#include <stdint.h>
/* Exactly the seven 0..15 fields in the team JSON / SWOS PlayerFile. */
typedef struct ExpandedSquadSkills {
    uint8_t passing, velocity, heading, tackling, control, speed, finishing;
} ExpandedSquadSkills;
/* Positions: 0=GK, 1=RB, 2=LB, 3=CD, 4=RW, 5=LW, 6=MF, 7=ST.
 * Weighted result is 0..1500 (score * 100). Price ID is only a GK fallback
 * because old SWOS goalkeepers have all seven outfield skills zero. */
static inline unsigned ExpandedSquad_Score100(ExpandedSquadSkills s,
                                                unsigned pos,unsigned priceId) {
    if(pos==0) return (priceId>49?49:priceId)*1500u/49u;
    if(pos>=1 && pos<=3)
        return 40u*s.tackling+25u*s.speed+20u*s.heading+15u*s.passing;
    if(pos>=4 && pos<=6)
        return 35u*s.passing+30u*s.control+20u*s.speed+15u*s.tackling;
    return 40u*s.finishing+25u*s.speed+20u*s.control+15u*s.heading;
}
static inline unsigned ExpandedSquad_Stars(ExpandedSquadSkills s,
                                            unsigned pos,unsigned priceId) {
    unsigned score=ExpandedSquad_Score100(s,pos,priceId);
    return score>1200?5:score>900?4:score>600?3:score>300?2:1;
}
#endif
