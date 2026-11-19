/* Exercises the exact pure hash used by career_status.c across >65536 OG
 * fixtures. This cannot certify a Windows CAR write or gameplay. */
#include "expanded_og_fixture_hash.h"
#include <stdio.h>
#include <assert.h>
int main(void)
{
    const size_t count=110000u;
    uint8_t *wire=(uint8_t *)calloc(count,EXP_OG_HASH_RECORD_SIZE);
    assert(wire);
    for(size_t i=0u;i<count;++i) {
        uint8_t *p=wire+i*EXP_OG_HASH_RECORD_SIZE;
        p[0]=0u;
        p[1]=(uint8_t)i;
        p[2]=(uint8_t)(i>>8u);
        p[3]=(uint8_t)(i>>16u);
        p[4]=(uint8_t)(i>>24u);
        p[14]=(uint8_t)(i&1u);
    }
    uint32_t *index=NULL;size_t capacity=0u;
    assert(ExpandedOGFixture_MakeIndex(wire,count,&index,&capacity));
    assert(capacity>=count*2u);
    for(size_t i=0u;i<count;++i) {
        size_t pos=0u;
        assert(ExpandedOGFixture_Locate(
            wire,index,capacity,wire+i*EXP_OG_HASH_RECORD_SIZE,&pos)==1);
        assert(index[pos]==(uint32_t)(i+1u));
    }
    uint8_t replay[EXP_OG_HASH_RECORD_SIZE];
    memcpy(replay,wire+70000u*EXP_OG_HASH_RECORD_SIZE,sizeof replay);
    replay[12]=5u;replay[13]=1u; /* score changed, same native fixture identity */
    size_t pos=0u;
    assert(ExpandedOGFixture_Locate(wire,index,capacity,replay,&pos)==1);
    replay[14]^=1u; /* leg is part of the native stable identity */
    assert(ExpandedOGFixture_Locate(wire,index,capacity,replay,&pos)==0);
    assert(!ExpandedOGFixture_BuildIndex(wire,count,index,capacity/3u));
    free(index);free(wire);
    puts("OG_FIXTURE_DYNAMIC_INDEX_110K_DUPLICATE_REJECT=PASS_ISOLATED_ONLY");
    return 0;
}
