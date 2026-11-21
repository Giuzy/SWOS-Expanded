/* RG-014/RG-010: adapt the ORIGINAL OG blink flag to the Expanded
 * CAREER button repaint. No timer, input or gameplay state is owned here.
 * Native SWOS UpdateBlinkingMenuText already advances a 0..31 counter. */
#ifndef EXPANDED_CAREER_JOB_OFFER_FLASH_H
#define EXPANDED_CAREER_JOB_OFFER_FLASH_H

#include <stdint.h>

static inline int ExpandedCareer_JobOfferFlashOn(
    unsigned menuKind,unsigned ordinal,uint16_t stringFlags,
    uint16_t nativeCounter,int disabled)
{
    return menuKind==3u && ordinal==5u && !disabled &&
           (stringFlags & (uint16_t)(1u<<13))!=0u &&
           (nativeCounter & 31u)<16u;
}

#endif /* EXPANDED_CAREER_JOB_OFFER_FLASH_H */
