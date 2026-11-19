/* Resident-only restoration of a real OG career club skipped by the
 * original team cache. This is an ID validation contract, not a selector,
 * a promotion algorithm or a fixture simulator. */
#ifndef EXPANDED_OG_RESIDENT_SELECTION_H
#define EXPANDED_OG_RESIDENT_SELECTION_H
#include <stdint.h>

static inline int ExpandedOGResidentSelection_ManagerRow(
    uint16_t managerId, const uint16_t *entrants, unsigned entrantsCount,
    const uint16_t *selected, unsigned selectedCount,
    const int32_t *identities, unsigned identityCount,
    unsigned *managerRow)
{
    unsigned managerSeen=0u, managerIdentity=0u, index=0u;
    unsigned referenced[64]={0};
    if(!entrants || !selected || !identities || !managerRow ||
       managerId==0u || managerId==0xffffu ||
       entrantsCount<2u || entrantsCount>64u ||
       selectedCount+1u!=entrantsCount ||
       identityCount!=entrantsCount)return 0;
    for(unsigned i=0u;i<entrantsCount;++i) {
        const uint16_t id=entrants[i];
        if(!id || id==0xffffu)return 0;
        if(id==managerId)++managerSeen;
        for(unsigned j=0u;j<i;++j)
            if(entrants[j]==id)return 0;
    }
    if(managerSeen!=1u)return 0;
    for(unsigned i=0u;i<selectedCount;++i) {
        const uint16_t id=selected[i];
        unsigned found=0u;
        if(!id || id==0xffffu || id==managerId)return 0;
        for(unsigned j=0u;j<entrantsCount;++j)
            if(entrants[j]==id)++found;
        if(found!=1u)return 0;
        for(unsigned j=0u;j<i;++j)
            if(selected[j]==id)return 0;
    }
    for(unsigned i=0u;i<identityCount;++i) {
        const int32_t id=identities[i];
        if(id==-1) {
            ++managerIdentity;
            index=i;
        } else {
            if(id<0 || (unsigned)id>=selectedCount)return 0;
            if(++referenced[(unsigned)id]!=1u)return 0;
        }
    }
    if(managerIdentity!=1u)return 0;
    for(unsigned i=0u;i<selectedCount;++i)
        if(referenced[i]!=1u)return 0;
    *managerRow=index;
    return 1;
}
#endif
