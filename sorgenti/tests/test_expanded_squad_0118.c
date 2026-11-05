#include <assert.h>
#include <stdio.h>
#include "expanded_squad_rating.h"
static ExpandedSquadSkills mk(unsigned p,unsigned v,unsigned h,unsigned t,
                              unsigned c,unsigned s,unsigned f) {
    ExpandedSquadSkills a={(uint8_t)p,(uint8_t)v,(uint8_t)h,(uint8_t)t,
                           (uint8_t)c,(uint8_t)s,(uint8_t)f};
    return a;
}
int main(void) {
    ExpandedSquadSkills hendon=mk(2,0,0,15,0,12,0);
    assert(ExpandedSquad_Score100(hendon,1,0)==930);
    assert(ExpandedSquad_Stars(hendon,1,0)==4); /* actual JSON defender */
    assert(ExpandedSquad_Stars(mk(0,0,0,0,0,0,0),7,0)==1);
    assert(ExpandedSquad_Stars(mk(15,15,15,15,15,15,15),7,0)==5);
    assert(ExpandedSquad_Stars(mk(15,0,0,0,15,15,0),6,0)>=3);
    assert(ExpandedSquad_Stars(mk(0,0,0,0,0,0,0),0,49)==5);
    assert(ExpandedSquad_Stars(mk(0,0,0,0,0,0,0),0,0)==1);
    assert(ExpandedSquad_Stars(mk(0,0,0,0,0,0,0),0,255)==5);
    for(unsigned pos=0;pos<8;pos++)
      for(unsigned val=0;val<16;val++) {
        ExpandedSquadSkills s=mk(val,val,val,val,val,val,val);
        unsigned stars=ExpandedSquad_Stars(s,pos,20);
        assert(stars>=1 && stars<=5);
      }
    puts("PASS: role-weighted 1..5 stars; SWOS 0..15 nibbles; GK price fallback");
    return 0;
}
